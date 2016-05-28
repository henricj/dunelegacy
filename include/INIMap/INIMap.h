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

#ifndef INIMAP_H
#define INIMAP_H

#include <FileClasses/INIFile.h>
#include <FileClasses/FileManager.h>

#include <DataTypes.h>

#include <globals.h>

#include <misc/string_util.h>

#include <SDL.h>

#include <string>
#include <memory>


class INIMap {
public:
    explicit INIMap(std::shared_ptr<INIFile>& pINIFile)
     : inifile(pINIFile), version(0), sizeX(0), sizeY(0), logicalSizeX(0), logicalSizeY(0), logicalOffsetX(0), logicalOffsetY(0) {

    }

    INIMap(GAMETYPE gameType, std::string mapname, std::string mapdata = "") : mapname(mapname) {

        if(gameType == GAMETYPE_CAMPAIGN || gameType == GAMETYPE_SKIRMISH) {
            // load from PAK-File
            SDL_RWops* mapiniFile = nullptr;
            try {

                mapiniFile = pFileManager->openFile(this->mapname);
                inifile = std::shared_ptr<INIFile>(new INIFile(mapiniFile));
                SDL_RWclose(mapiniFile);
            } catch (...) {
                if(mapiniFile != nullptr) {
                    SDL_RWclose(mapiniFile);
                }
                throw;
            }
        } else if(gameType == GAMETYPE_CUSTOM || gameType == GAMETYPE_CUSTOM_MULTIPLAYER) {
            SDL_RWops* RWops = SDL_RWFromConstMem(mapdata.c_str(), mapdata.size());
            inifile = std::shared_ptr<INIFile>(new INIFile(RWops));
            SDL_RWclose(RWops);
        } else {
            inifile = std::shared_ptr<INIFile>(new INIFile(mapname));
        }
    }

    ~INIMap() {

    }

protected:

    /**
        Log a warning while reading the scenario file.
        \param  warning the warning message
    */
    void logWarning(std::string warning) {
        fprintf(stderr, "%s: %s\n", mapname.c_str(), warning.c_str());
    }


    /**
        Log a warning while reading the scenario file.
        \param  line    the line number the warning occurs
        \param  warning the warning message
    */
    void logWarning(int line, std::string warning) {
        fprintf(stderr, "%s:%d: %s\n", mapname.c_str(), line, warning.c_str());
    }


    /**
        Log an error while reading the scenario file. This method throws an std::runtime_error exception
        with error as the exception message
        \param  error the error message
    */
    void logError(std::string error) {
        throw std::runtime_error(mapname + ": " + error);
    }


    /**
        Log an error while reading the scenario file. This method throws an std::runtime_error exception
        with error as the exception message
        \param  line    the line number the error occurs
        \param  error the error message
    */
    void logError(int line, std::string error) {
        throw std::runtime_error(mapname + ":" + stringify(line) + ": " + error);
    }


    /**
    Checks if all map features of this map are supported.
    */
    void checkFeatures() {
        if(inifile->hasSection("FEATURES") == false) {
            return;
        }

        INIFile::KeyIterator iter;

        for(iter = inifile->begin("FEATURES"); iter != inifile->end("FEATURES"); ++iter) {
            if(iter->getBoolValue(true) == true) {
                logError(iter->getLineNumber(), "Unsupported feature \"" + iter->getKeyName() + "\"!");
                return; // never reached
            }
        }
    }


    inline int getXPos(int pos) const { return (pos % logicalSizeX) - logicalOffsetX; };
    inline int getYPos(int pos) const { return (pos / logicalSizeX) - logicalOffsetY; };

    std::string mapname;
    std::shared_ptr<INIFile> inifile;

    int version;

    int sizeX;
    int sizeY;
    int logicalSizeX;
    int logicalSizeY;
    int logicalOffsetX;
    int logicalOffsetY;
};

#endif //INIMAP_H
