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

#ifndef MAPDATA_H
#define MAPDATA_H

#include <data.h>

#include <vector>

class MapData {

public:
    MapData() {
        sizeX = 0;
        sizeY = 0;
    }

    MapData(int sizeX, int sizeY, TERRAINTYPE terrainType = Terrain_Sand) {
        this->sizeX = sizeX;
        this->sizeY = sizeY;
        data.resize(sizeX*sizeY,terrainType);
    }

    const TERRAINTYPE& operator()(int x, int y) const {
        return data.at(y*sizeX+x);
    }

    TERRAINTYPE& operator()(int x, int y) {
        return data.at(y*sizeX+x);
    }

    int getSizeX() const {
        return sizeX;
    }

    int getSizeY() const {
        return sizeY;
    }

    bool isInsideMap(int x, int y) const {
        return ( (x >= 0) && (x < sizeX) && (y >= 0) && (y < sizeY) );
    }

private:
    std::vector<TERRAINTYPE> data;

    int sizeX;
    int sizeY;
};

#endif // MAPDATA_H
