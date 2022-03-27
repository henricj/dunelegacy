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

#ifndef ICNFILE_H
#define ICNFILE_H

#include <misc/SDL2pp.h>

#include <vector>

/// A class for loading a *.ICN-File and the corresponding *.MAP-File.
/**
    This class can read icn-Files and return the contained pictures as a SDL_Surface. An icn-File contains
    small 16x16 pixel tiles. The map-file contains the information how to build up a complete picture with
    this small tiles.
*/
class Icnfile {
private:
    /// Internal structure for the MAP-File.
    struct MapfileEntry {
        uint32_t numTiles{};
        std::vector<uint16_t> tileIndices;
    };

public:
    Icnfile(SDL_RWops* icnRWop, SDL_RWops* mapRWop);
    Icnfile(const Icnfile& o) = delete;
    Icnfile(Icnfile&& o)      = delete;
    ~Icnfile();

    [[nodiscard]] sdl2::surface_ptr getPicture(uint32_t indexOfFile) const;
    sdl2::surface_ptr getPictureArray(uint32_t mapfileIndex, int tilesX = 0, int tilesY = 0, int tilesN = 0);
    [[nodiscard]] sdl2::surface_ptr getPictureRow(uint32_t startIndex, uint32_t endIndex,
                                                  uint32_t maxRowLength = 0) const;
    sdl2::surface_ptr getPictureRow2(unsigned int numTiles, ...) const;

    /// Returns the number of tiles
    /**
        Returns the number of tiles in the icn-File.
        \return Number of tiles
    */
    [[nodiscard]] int getNumFiles() const noexcept { return numFiles; }

    /// Returns the number of tilesets
    /**
        Returns the number of tilesets in the map-File.
        \return Number of tilesets
    */
    [[nodiscard]] int getNumTilesets() const noexcept { return tilesets.size(); }

private:
    std::unique_ptr<uint8_t[]> pIcnFiledata;
    uint32_t numFiles;

    std::vector<MapfileEntry> tilesets;

    const uint8_t* SSET;
    uint32_t SSET_Length;
    const uint8_t* RPAL;
    uint32_t RPAL_Length;
    const uint8_t* RTBL;
    uint32_t RTBL_Length;
};

#endif // ICNFILE_H
