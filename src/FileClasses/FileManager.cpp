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
#include <iomanip>
#include <mutex>
#include <sstream>

FileManager::FileManager() {
    sdl2::log_info("FileManager is loading PAK-Files...");
    sdl2::log_info("MD5-Checksum                      Filename");

    const auto search_path = getSearchPath();

    for (const auto& filename : getNeededFiles()) {
        for (const auto& sp : search_path) {
            auto filepath = sp / filename;
            if (getCaseInsensitiveFilename(filepath)) {
                try {
                    sdl2::log_info("%s  %s", md5FromFilename(filepath).c_str(), filepath.u8string().c_str());
                    pakFiles.push_back(std::make_unique<Pakfile>(filepath));
                } catch (std::exception& e) {
                    pakFiles.clear();

                    THROW(io_error, "Error while opening '%s': %s!", filepath.u8string().c_str(), e.what());
                }

                // break out of searchPath-loop because we have opened the file in one directory
                break;
            }
        }
    }

    sdl2::log_info("%s", "");
}

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

std::vector<std::filesystem::path> FileManager::getNeededFiles() {
    std::vector<std::filesystem::path> fileList = {
        "LEGACY.PAK", "OPENSD2.PAK", "GFXHD.PAK",  "DUNE.PAK",  "SCENARIO.PAK", "MENTAT.PAK",
        "VOC.PAK",    "MERC.PAK",    "FINALE.PAK", "INTRO.PAK", "INTROVOC.PAK", "SOUND.PAK",
    };

    std::string LanguagePakFiles = pTextManager != nullptr ? _("LanguagePakFiles") : "";

    if (LanguagePakFiles.empty()) {
        LanguagePakFiles = "ENGLISH.PAK,HARK.PAK,ATRE.PAK,ORDOS.PAK";
    }

    std::vector<std::string> additionalPakFiles = splitStringToStringVector(LanguagePakFiles);
    fileList.insert(std::end(fileList), std::begin(additionalPakFiles), std::end(additionalPakFiles));

    std::sort(fileList.begin(), fileList.end());

    return fileList;
}

std::vector<std::filesystem::path> FileManager::getMissingFiles() {
    std::vector<std::filesystem::path> MissingFiles;
    const auto searchPath = getSearchPath();

    for (const auto& fileName : getNeededFiles()) {
        auto bFound = false;
        for (const auto& sp : searchPath) {
            auto filepath = sp / fileName;
            if (getCaseInsensitiveFilename(filepath)) {
                bFound = true;
                break;
            }
        }

        if (!bFound) {
            MissingFiles.push_back(fileName);
        }
    }

    return MissingFiles;
}

sdl2::RWops_ptr FileManager::openFile(const std::filesystem::path& filename) const {

    if (filename.is_absolute()) {
        if (sdl2::RWops_ptr ret{SDL_RWFromFile(filename.u8string().c_str(), "rb")})
            return ret;
    } else {
        // try loading external file
        for (const auto& searchPath : getSearchPath()) {
            auto externalFilename = searchPath / filename;
            if (getCaseInsensitiveFilename(externalFilename)) {
                if (sdl2::RWops_ptr ret{SDL_RWFromFile(externalFilename.u8string().c_str(), "rb")})
                    return ret;
            }
        }

        // now try loading from pak file
        for (const auto& pPakFile : pakFiles) {
            if (pPakFile->exists(filename)) {
                return pPakFile->openFile(filename);
            }
        }
    }

    THROW(io_error, "Cannot find '%s'!", filename.string());
}

bool FileManager::exists(const std::filesystem::path& filename) const {

    // try finding external file
    for (const auto& searchPath : getSearchPath()) {
        auto externalFilename = searchPath / filename;
        if (getCaseInsensitiveFilename(externalFilename)) {
            return true;
        }
    }

    // now try finding in one pak file
    for (const auto& pPakFile : pakFiles) {
        if (pPakFile->exists(filename)) {
            return true;
        }
    }

    return false;
}

std::string FileManager::md5FromFilename(const std::filesystem::path& filename) {
    unsigned char md5sum[16];

    if (md5_file(filename.u8string().c_str(), md5sum) != 0) {
        THROW(io_error, "Cannot open or read '%s'!", filename.string());
    }
    std::stringstream stream;
    stream << std::setfill('0') << std::hex;
    for (const int i : md5sum) {
        stream << std::setw(2) << i;
    }
    return stream.str();
}
