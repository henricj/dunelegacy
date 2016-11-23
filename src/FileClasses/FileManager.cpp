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

#include <config.h>
#include <misc/fnkdat.h>
#include <misc/md5.h>
#include <misc/string_util.h>
#include <misc/exceptions.h>

#include <algorithm>
#include <sstream>
#include <iomanip>

FileManager::FileManager() {
    SDL_Log("\nFileManager is loading PAK-Files...");
    SDL_Log("\nMD5-Checksum                      Filename");

    for(const std::string& filename : getNeededFiles()) {
        for(const std::string& searchPath : getSearchPath()) {
            std::string filepath = searchPath + "/" + filename;
            if(getCaseInsensitiveFilename(filepath) == true) {
                try {
                    SDL_Log("%s  %s", md5FromFilename(filepath).c_str(), filepath.c_str());
                    pakFiles.push_back(new Pakfile(filepath));
                } catch (std::exception &e) {
                    while(pakFiles.empty()) {
                        delete pakFiles.back();
                        pakFiles.pop_back();
                    }

                    THROW(io_error, "Error while opening '%s': %s!", filepath, e.what());
                }

                // break out of searchPath-loop because we have opened the file in one directory
                break;
            }
        }

    }

    SDL_Log("%s", "");
}

FileManager::~FileManager() {
    for(Pakfile* pPakFile : pakFiles) {
        delete pPakFile;
    }
}

std::vector<std::string> FileManager::getSearchPath() {
    std::vector<std::string> searchPath;

    searchPath.push_back(getDuneLegacyDataDir());
    char tmp[FILENAME_MAX];
    fnkdat("data", tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
    searchPath.push_back(tmp);

    return searchPath;
}

std::vector<std::string> FileManager::getNeededFiles() {
    std::vector<std::string> fileList;

    fileList.push_back("LEGACY.PAK");
    fileList.push_back("OPENSD2.PAK");
    fileList.push_back("GFXHD.PAK");
    fileList.push_back("DUNE.PAK");
    fileList.push_back("SCENARIO.PAK");
    fileList.push_back("MENTAT.PAK");
    fileList.push_back("VOC.PAK");
    fileList.push_back("MERC.PAK");
    fileList.push_back("FINALE.PAK");
    fileList.push_back("INTRO.PAK");
    fileList.push_back("INTROVOC.PAK");
    fileList.push_back("SOUND.PAK");

    std::string LanguagePakFiles = (pTextManager != nullptr) ? _("LanguagePakFiles") : "";

    if(LanguagePakFiles.empty()) {
        LanguagePakFiles = "ENGLISH.PAK,HARK.PAK,ATRE.PAK,ORDOS.PAK";
    }

    std::vector<std::string> additionalPakFiles = splitString(LanguagePakFiles);
    fileList.insert(std::end(fileList), std::begin(additionalPakFiles), std::end(additionalPakFiles));

    std::sort(fileList.begin(), fileList.end());

    return fileList;
}

std::vector<std::string> FileManager::getMissingFiles() {
    std::vector<std::string> MissingFiles;
    std::vector<std::string> searchPath = getSearchPath();

    for(const std::string& fileName : getNeededFiles()) {
        bool bFound = false;
        for(const std::string& searchPath : getSearchPath()) {
            std::string filepath = searchPath + "/" + fileName;
            if(getCaseInsensitiveFilename(filepath) == true) {
                bFound = true;
                break;
            }
        }

        if(bFound == false) {
            MissingFiles.push_back(fileName);
        }
    }

    return MissingFiles;
}

SDL_RWops* FileManager::openFile(std::string filename) {
    SDL_RWops* ret;

    // try loading external file
    for(const std::string& searchPath : getSearchPath()) {
        std::string externalFilename = searchPath + "/" + filename;
        if(getCaseInsensitiveFilename(externalFilename) == true) {
            if((ret = SDL_RWFromFile(externalFilename.c_str(), "rb")) != nullptr) {
                return ret;
            }
        }
    }

    // now try loading from pak file
    for(Pakfile* pPakFile : pakFiles) {
        ret = pPakFile->openFile(filename);
        if(ret != nullptr) {
            return ret;
        }
    }

    THROW(io_error, "Cannot find '%s'!", filename);
}

bool FileManager::exists(std::string filename) const {

    // try finding external file
    for(const std::string& searchPath : getSearchPath()) {
        std::string externalFilename = searchPath + "/" + filename;
        if(getCaseInsensitiveFilename(externalFilename) == true) {
            return true;
        }
    }

    // now try finding in one pak file
    for(const Pakfile* pPakFile : pakFiles) {
        if(pPakFile->exists(filename)) {
            return true;
        }
    }

    return false;
}


std::string FileManager::md5FromFilename(std::string filename) {
    unsigned char md5sum[16];

    if(md5_file(filename.c_str(), md5sum) != 0) {
        THROW(io_error, "Cannot open or read '%s'!", filename);
    } else {

        std::stringstream stream;
        stream << std::setfill('0') << std::hex;
        for(int i=0;i<16;i++) {
            stream << std::setw(2) << (int) md5sum[i];
        }
        return stream.str();
    }
}
