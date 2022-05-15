/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <FileClasses/FileManager.h>

#include <globals.h>

#include <FileClasses/TextManager.h>

#include <misc/FileSystem.h>

#include <misc/exceptions.h>
#include <misc/fnkdat.h>
#include <misc/md5.h>
#include <misc/string_util.h>

#include <algorithm>
#include <filesystem>
#include <mutex>

std::vector<std::unique_ptr<Pakfile>>
PakFileManager::loadPakFiles(const CaseInsensitiveFilesystemCache& cache, std::span<const std::string> files) {
    sdl2::log_info("FileManager is loading PAK-Files...");
    sdl2::log_info("MD5-Checksum                      Filename");

    std::vector<std::unique_ptr<Pakfile>> pakFiles;
    pakFiles.reserve(files.size());

    for (const auto& filename : files) {
        auto filepath0 = cache.find(std::u8string{reinterpret_cast<const char8_t*>(filename.data()), filename.size()});

        if (!filepath0.has_value())
            continue;

        auto& filepath = filepath0.value();

        try {
            sdl2::log_info("%s  %s", md5FromFilename(filepath),
                           reinterpret_cast<const char*>(filepath.u8string().c_str()));
            pakFiles.emplace_back(std::make_unique<Pakfile>(filepath));
        } catch (std::exception& e) {
            THROW(io_error, "Error while opening '%s': %s!", reinterpret_cast<const char*>(filepath.u8string().c_str()),
                  e.what());
        }
    }

    sdl2::log_info("");

    return pakFiles;
}

PakFileManager::pak_directory_type PakFileManager::createPakDirectory() const {
    pak_directory_type directory;

    for (auto& pf : pakFiles_) {
        for (auto i = 0; i < pf->getNumFiles(); ++i)
            directory[pf->getFilename(i)] = std::make_tuple(pf.get(), i);
    }

    return directory;
}

CaseInsensitiveFilesystemCache::CaseInsensitiveFilesystemCache(std::span<const std::filesystem::path> paths)
    : files_{createFilesystemDirectory(paths)} { }

std::optional<std::filesystem::path> CaseInsensitiveFilesystemCache::find(const std::u8string& filename) const {
    const auto it = files_.find(filename);
    if (it == files_.end())
        return std::nullopt;

    return it->second;
}

CaseInsensitiveFilesystemCache::filesystem_directory_type
CaseInsensitiveFilesystemCache::createFilesystemDirectory(std::span<const std::filesystem::path> paths) const {
    filesystem_directory_type files;

    for (const auto& path : paths) {
        for (const auto& de : std::filesystem::directory_iterator(path)) {
            if (!de.is_regular_file())
                continue;

            auto file_path      = de.path().lexically_normal().make_preferred();
            const auto filename = file_path.filename();

            auto& file = files[filename.u8string()];

            if (file.empty())
                file = std::move(file_path);
        }
    }

    return files;
}

FileManager::FileManager()
    : filesystem_cache_{getSearchPath()}, pak_files_{filesystem_cache_, PakFileConfiguration ::getNeededFiles()} { }

FileManager::~FileManager() = default;

const std::vector<std::filesystem::path>& FileManager::getSearchPath() {
    static std::vector<std::filesystem::path> search_path;
    static std::once_flag flag;

    std::call_once(flag, [] {
        search_path.push_back(getDuneLegacyDataDir());
        auto [ok, tmp] = fnkdat("data/", FNKDAT_USER | FNKDAT_CREAT);
        if (ok)
            search_path.push_back(tmp);
    });

    return search_path;
}

std::vector<std::string> PakFileConfiguration::getNeededFiles() {
    std::vector<std::string> fileList = {
        "LEGACY.PAK", "OPENSD2.PAK", "GFXHD.PAK",  "DUNE.PAK",  "SCENARIO.PAK", "MENTAT.PAK",
        "VOC.PAK",    "MERC.PAK",    "FINALE.PAK", "INTRO.PAK", "INTROVOC.PAK", "SOUND.PAK",
    };

    auto LanguagePakFiles = dune::globals::pTextManager != nullptr ? _("LanguagePakFiles") : "";

    if (LanguagePakFiles.empty()) {
        LanguagePakFiles = "ENGLISH.PAK,HARK.PAK,ATRE.PAK,ORDOS.PAK";
    }

    std::vector<std::string> additionalPakFiles = splitStringToStringVector(LanguagePakFiles);
    fileList.insert(std::end(fileList), std::begin(additionalPakFiles), std::end(additionalPakFiles));

    std::ranges::sort(fileList);

    return fileList;
}

std::vector<std::filesystem::path> PakFileConfiguration::getMissingFiles(const CaseInsensitiveFilesystemCache& cache) {
    std::vector<std::filesystem::path> missing;

    const auto files = getNeededFiles();

    for (const auto& fileName : files) {
        const auto path =
            cache.find(std::u8string{reinterpret_cast<const char8_t*>(fileName.c_str()), fileName.size()});
        if (!path.has_value())
            missing.emplace_back(path.value());
    }

    return missing;
}

sdl2::RWops_ptr FileManager::openFile(std::filesystem::path filename) const {
    if (filename.is_absolute()) {
        if (sdl2::RWops_ptr ret{SDL_RWFromFile(filename.lexically_normal().make_preferred().u8string(), "rb")})
            return ret;
    } else {
        // try loading external file
        const auto fit = filesystem_cache_.find(filename.u8string());
        if (fit.has_value()) {
            if (sdl2::RWops_ptr ret{SDL_RWFromFile(fit.value().u8string(), "rb")})
                return ret;
        }

        // now try loading from pak file
        const auto [pPakFile, index] = pak_files_.find(filename.string());
        if (pPakFile)
            return pPakFile->openFile(index);
    }

    THROW(io_error, "Cannot find '%s'!", filename.string());
}

bool FileManager::exists(std::filesystem::path filename) const {
    if (filename.is_absolute()) {
        std::error_code ec;
        if (is_regular_file(filename, ec))
            return ec == std::error_code{};

        return false;
    }

    // try finding external file
    {
        // Scope
        const auto fit = filesystem_cache_.find(filename.u8string());
        if (fit.has_value())
            return true;
    }

    // now try finding in one pak file
    {
        // scope
        const auto [pPakFile, index] = pak_files_.find(filename.string());
        if (pPakFile)
            return true;
    }

    return false;
}

void CaseInsensitiveFilesystemCache::refresh(std::span<const std::filesystem::path> paths) {
    files_ = createFilesystemDirectory(paths);
}

bool CaseInsensitiveFilesystemCache::CaseInsensitiveEqualTo::operator()(const std::u8string& lhs,
                                                                        const std::u8string& rhs) const {
    if (&lhs == &rhs)
        return true;

    if (lhs.length() != rhs.length())
        return false;

    return std::ranges::equal(lhs, rhs, [](auto a, auto b) {
        // Force values to be "unsigned char" or std::tolower()'s behavior is undefined.
        return a == b || std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
    });
}

size_t CaseInsensitiveFilesystemCache::CaseInsensitiveHash::operator()(const std::u8string& value) const {
    auto sum = value.size();

    // Force "c" to be "unsigned char" or std::tolower()'s behavior is undefined.
    for (unsigned char c : value)
        sum = sum * 101 + std::tolower(c);

    return sum;
}

std::string PakFileManager::md5FromFilename(std::filesystem::path filename) {
    std::array<unsigned char, 16> md5sum{};

    const auto path = filename.lexically_normal().make_preferred().u8string();

    if (md5_file(reinterpret_cast<const char*>(path.c_str()), md5sum.data()) != 0)
        THROW(io_error, "Cannot open or read '%s'!", filename.string());

    return to_hex(md5sum, 0);
}

PakFileManager::PakFileManager(const CaseInsensitiveFilesystemCache& cache, std::span<const std::string> files)
    : pakFiles_{loadPakFiles(cache, files)}, pak_directory_(createPakDirectory()) { }

std::tuple<const Pakfile*, int> PakFileManager::find(const std::string& filename) const {
    const auto it = pak_directory_.find(filename);

    if (it == pak_directory_.end())
        return {nullptr, 0};

    const auto [pPakFile, index] = it->second;

    return {pPakFile, index};
}

bool PakFileManager::exists(const std::string& filename) const {
    return pak_directory_.contains(filename);
}
