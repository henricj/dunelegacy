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

#include <FileClasses/FileManager.h>
#include <FileClasses/INIFile.h>

#include <DataTypes.h>

#include <globals.h>

#include <misc/SDL2pp.h>
#include <misc/string_util.h>

#include <memory>
#include <string>

class INIMap {
protected:
    typedef unique_or_nonowning_ptr<INIFile> inifile_ptr;

    explicit INIMap(inifile_ptr pINIFile) : inifile(std::move(pINIFile)) { }

    INIMap(GameType gameType, const std::filesystem::path& mapname, const std::string& mapdata = "");

public:
    virtual ~INIMap();

protected:
    /**
        Log a warning while reading the scenario file.
        \param  warning the warning message
    */
    void logWarning(std::string_view warning) const { sdl2::log_info("%s: %s", mapname.c_str(), warning); }

    /**
        Log a warning while reading the scenario file.
        \param  line    the line number the warning occurs
        \param  format  the warning message format
        \param  args    any arguments for the warning message format
    */
    template<typename... Args>
    void logWarning(size_t line, std::string_view format, Args&&... args) const {
        sdl2::log_info("%s:%d: %s", mapname, line, fmt::sprintf(format, std::forward<Args>(args)...));
    }

    /**
        Log an error while reading the scenario file. This method throws an std::runtime_error exception
        with error as the exception message
        \param  error the error message
    */
    void logError(const std::string& error) const { THROW(std::runtime_error, "%s: %s", mapname, error); }

    /**
        Log an error while reading the scenario file. This method throws an std::runtime_error exception
        with error as the exception message
        \param  line    the line number the error occurs
        \param  format  the error message format
        \param  args    any arguments for the error message format
    */
    template<typename... Args>
    void logError(size_t line, std::string_view format, Args&&... args) const {
        THROW(std::runtime_error, "%s:%d: %s", mapname, line, fmt::sprintf(format, std::forward<Args>(args)...));
    }

    /**
    Checks if all map features of this map are supported.
    */
    void checkFeatures() const;

    [[nodiscard]] int getXPos(int pos) const {
        return (version < 2 ? (pos & 0x3f) : (pos % logicalSizeX)) - logicalOffsetX;
    }
    [[nodiscard]] int getYPos(int pos) const {
        return (version < 2 ? ((pos >> 6) & 0x3f) : (pos / logicalSizeX)) - logicalOffsetY;
    }

    std::string mapname;
    inifile_ptr inifile;

    int version = 0;

    int sizeX          = 0;
    int sizeY          = 0;
    int logicalSizeX   = 0;
    int logicalSizeY   = 0;
    int logicalOffsetX = 0;
    int logicalOffsetY = 0;
};

#endif // INIMAP_H
