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
#include <misc/SDL2pp.h>

#include <string>
#include <memory>


class INIMap {
public:
    typedef unique_or_nonowning_ptr<INIFile> inifile_ptr;

    explicit INIMap(inifile_ptr pINIFile)
     : inifile(std::move(pINIFile)) {

    }

    INIMap(GameType gameType, const std::string& mapname, const std::string& mapdata = "") : mapname(mapname) {

        if(gameType == GameType::Campaign || gameType == GameType::Skirmish) {
            // load from PAK-File
            inifile = std::make_unique<INIFile>(pFileManager->openFile(this->mapname).get());
        } else if(gameType == GameType::CustomGame || gameType == GameType::CustomMultiplayer) {
            SDL_RWops* RWops = SDL_RWFromConstMem(mapdata.c_str(), mapdata.size());
            inifile = std::make_unique<INIFile>(RWops);
            SDL_RWclose(RWops);
        } else {
            inifile = std::make_unique<INIFile>(mapname);
        }
    }

    ~INIMap() = default;

protected:

    /**
        Log a warning while reading the scenario file.
        \param  warning the warning message
    */
    void logWarning(const std::string& warning) {
        SDL_Log("%s: %s", mapname.c_str(), warning.c_str());
    }


    /**
        Log a warning while reading the scenario file.
        \param  line    the line number the warning occurs
        \param  warning the warning message
    */
    void logWarning(int line, const std::string& warning) {
        SDL_Log("%s:%d: %s", mapname.c_str(), line, warning.c_str());
    }


    /**
        Log an error while reading the scenario file. This method throws an std::runtime_error exception
        with error as the exception message
        \param  error the error message
    */
    void logError(const std::string& error) {
        THROW(std::runtime_error, mapname + ": " + error);
    }


    /**
        Log an error while reading the scenario file. This method throws an std::runtime_error exception
        with error as the exception message
        \param  line    the line number the error occurs
        \param  error the error message
    */
    void logError(int line, const std::string& error) {
        THROW(std::runtime_error, mapname + ":" + std::to_string(line) + ": " + error);
    }


    /**
    Checks if all map features of this map are supported.
    */
    void checkFeatures() {
        if(!inifile->hasSection("FEATURES")) {
            return;
        }

        for(const INIFile::Key& key : inifile->getSection("FEATURES")) {
            if(key.getBoolValue(true) == true) {
                logError(key.getLineNumber(), "Unsupported feature \"" + key.getKeyName() + "\"!");
                return;
            }
        }
    }


    inline int getXPos(int pos) const { return (pos % logicalSizeX) - logicalOffsetX; };
    inline int getYPos(int pos) const { return (pos / logicalSizeX) - logicalOffsetY; };

    std::string mapname;
    inifile_ptr inifile;

    int version = 0;

    int sizeX = 0;
    int sizeY = 0;
    int logicalSizeX = 0;
    int logicalSizeY = 0;
    int logicalOffsetX = 0;
    int logicalOffsetY = 0;
};

#endif //INIMAP_H
