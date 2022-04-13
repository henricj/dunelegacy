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

#ifndef ASTARSEARCH_H
#define ASTARSEARCH_H

#include <DataTypes.h>
#include <fixmath/FixPoint.h>

#include <vector>

class UnitBase;
class Map;

class AStarSearch final {
public:
    AStarSearch(Map* pMap);
    ~AStarSearch();

    AStarSearch(const AStarSearch&)            = delete;
    AStarSearch(AStarSearch&&)                 = delete;
    AStarSearch& operator=(const AStarSearch&) = delete;
    AStarSearch& operator=(AStarSearch&&)      = delete;

    void Search(Map* pMap, UnitBase* pUnit, Coord start, Coord destination);

    bool getFoundPath(Map* pMap, std::vector<Coord>& path) const;

private:
    struct TileData {
        TileData* parentKey{};
        Coord coord;
        FixPoint g;
        FixPoint h;
        FixPoint f;
        bool bInOpenList{};
        bool bClosed{};
    };

    TileData& getMapData(const Coord& coord) noexcept { return mapData[coord.y * sizeX + coord.x]; }
    [[nodiscard]] const TileData& getMapData(const Coord& coord) const noexcept {
        return mapData[coord.y * sizeX + coord.x];
    }

    TileData& getMapData(int key) noexcept { return mapData[key]; }
    [[nodiscard]] const TileData& getMapData(int key) const noexcept { return mapData[key]; }

    // void trickleUp(size_t openListIndex);
    void putOnOpenListIfBetter(int key, const Coord& coord, TileData* parentKey, FixPoint g, FixPoint h);
    TileData* extractMin();

    struct open_list final {
        FixPoint f;
        TileData* key;
        bool operator<(const open_list& other) const noexcept { return f > other.f; }
    };

    const int sizeX;
    const int sizeY;
    TileData* bestCoord;
    std::vector<TileData> mapData;
    std::vector<open_list> openList;
    std::vector<short> depthCheckCount;
};

#endif // ASTARSEARCH_H
