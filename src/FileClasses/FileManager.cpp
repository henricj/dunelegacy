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

#include <misc/fnkdat.h>
#include <misc/md5.h>
#include <misc/string_util.h>
#include <misc/exceptions.h>

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <mutex>

FileManager::FileManager() {
    SDL_Log("\nFileManager is loading PAK-Files...");
    SDL_Log("\nMD5-Checksum                      Filename");

    const auto search_path = getSearchPath();

    for(const auto& filename : getNeededFiles()) {
        for(const auto& sp : search_path) {
            auto filepath = sp + "/";
            filepath += filename;
            if(getCaseInsensitiveFilename(filepath)) {
                try {
                    SDL_Log("%s  %s", md5FromFilename(filepath).c_str(), filepath.c_str());
                    pakFiles.push_back(std::make_unique<Pakfile>(filepath));
                } catch (std::exception &e) {
                    pakFiles.clear();

                    THROW(io_error, "Error while opening '%s': %s!", filepath, e.what());
                }

                // break out of searchPath-loop because we have opened the file in one directory
                break;
            }
        }

    }

    SDL_Log("%s", "");
}

FileManager::~FileManager() = default;

const std::vector<std::string>& FileManager::getSearchPath() {
    static std::vector<std::string> search_path;
    static std::once_flag flag;

    std::call_once(flag, [] {
        search_path.push_back(getDuneLegacyDataDir());
        char tmp[FILENAME_MAX];
        fnkdat("data", tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
        search_path.emplace_back(tmp);
        });

    return search_path;
}

std::vector<std::string> FileManager::getNeededFiles() {
    std::vector<std::string> fileList = {
        "LEGACY.PAK",
        "OPENSD2.PAK",
        "GFXHD.PAK",
        "DUNE.PAK",
        "SCENARIO.PAK",
        "MENTAT.PAK",
        "VOC.PAK",
        "MERC.PAK",
        "FINALE.PAK",
        "INTRO.PAK",
        "INTROVOC.PAK",
        "SOUND.PAK",
    };

    std::string LanguagePakFiles = (pTextManager != nullptr) ? _("LanguagePakFiles") : "";

    if(LanguagePakFiles.empty()) {
        LanguagePakFiles = "ENGLISH.PAK,HARK.PAK,ATRE.PAK,ORDOS.PAK";
    }

    std::vector<std::string> additionalPakFiles = splitStringToStringVector(LanguagePakFiles);
    fileList.insert(std::end(fileList), std::begin(additionalPakFiles), std::end(additionalPakFiles));

    std::sort(fileList.begin(), fileList.end());

    return fileList;
}

std::vector<std::string> FileManager::getMissingFiles() {
    std::vector<std::string> MissingFiles;
    const auto searchPath = getSearchPath();

    for(const auto& fileName : getNeededFiles()) {
        auto bFound = false;
        for(const auto& sp : searchPath) {
            auto filepath = sp + "/";
            filepath += fileName;
            if(getCaseInsensitiveFilename(filepath)) {
                bFound = true;
                break;
            }
        }

        if(!bFound) {
            MissingFiles.push_back(fileName);
        }
    }

    return MissingFiles;
}

sdl2::RWops_ptr FileManager::openFile(const std::string& filename) {
    sdl2::RWops_ptr ret;

    // try loading external file
    for(const auto& searchPath : getSearchPath()) {
        auto externalFilename = searchPath + "/";
        externalFilename += filename;
        if(getCaseInsensitiveFilename(externalFilename)) {
            ret = sdl2::RWops_ptr{SDL_RWFromFile(externalFilename.c_str(), "rb")};
            if(ret) {
                return ret;
            }
        }
    }

    // now try loading from pak file
    for(const auto& pPakFile : pakFiles) {
        if(pPakFile->exists(filename)) {
            return pPakFile->openFile(filename);
        }
    }

    THROW(io_error, "Cannot find '%s'!", filename);
}

bool FileManager::exists(const std::string& filename) const {

    // try finding external file
    for(const std::string& searchPath : getSearchPath()) {
        auto externalFilename = searchPath + "/";
        externalFilename += filename;
        if(getCaseInsensitiveFilename(externalFilename)) {
            return true;
        }
    }

    // now try finding in one pak file
    for(const auto& pPakFile : pakFiles) {
        if(pPakFile->exists(filename)) {
            return true;
        }
    }

    return false;
}


std::string FileManager::md5FromFilename(const std::string& filename) const {
    unsigned char md5sum[16];

    if(md5_file(filename.c_str(), md5sum) != 0) {
        THROW(io_error, "Cannot open or read '%s'!", filename);
    } else {

        std::stringstream stream;
        stream << std::setfill('0') << std::hex;
        for(int i : md5sum) {
            stream << std::setw(2) << i;
        }
        return stream.str();
    }
}
