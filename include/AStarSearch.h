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

#include <list>
#include <vector>

class UnitBase;
class Map;

class AStarSearch {
public:
    AStarSearch(Map* pMap, UnitBase* pUnit, Coord start, Coord destination);
    ~AStarSearch();

    AStarSearch(const AStarSearch &) = delete;
    AStarSearch(AStarSearch &&) = delete;
    AStarSearch& operator=(const AStarSearch &) = delete;
    AStarSearch& operator=(AStarSearch &&) = delete;

    std::list<Coord> getFoundPath() {
        std::list<Coord> path;

        if(bestCoord.isInvalid()) {
            return false;
        }

        auto currentCoord = bestCoord;
        while(true) {
            const auto nextCoord = getMapData(currentCoord).parentCoord;

            if(nextCoord.isInvalid()) {
                break;
            }

            path.push_front(currentCoord);
            currentCoord = nextCoord;
        }

        return true;
    };

private:
    struct TileData {
        Coord    parentCoord;
        size_t   openListIndex{};
        FixPoint g;
        FixPoint h;
        FixPoint f;
        bool     bInOpenList{};
        bool     bClosed{};
    };


    inline TileData& getMapData(const Coord& coord) noexcept { return mapData[coord.y * sizeX + coord.x]; };
    inline const TileData& getMapData(const Coord& coord) const noexcept { return mapData[coord.y * sizeX + coord.x]; };

    void trickleUp(size_t openListIndex) {
        const Coord bottom = openList[openListIndex];
        const FixPoint newf = getMapData(bottom).f;

        size_t current = openListIndex;
        size_t parent = (openListIndex - 1)/2;
        while (current > 0 && getMapData(openList[parent]).f > newf) {

            // copy parent to position of current
            openList[current] = openList[parent];
            getMapData(openList[current]).openListIndex = current;

            // go up one level in the tree
            current = parent;
            parent = (parent - 1)/2;
        }

        openList[current] = bottom;
        getMapData(openList[current]).openListIndex = current;
    };

    void putOnOpenListIfBetter(const Coord& coord, const Coord& parentCoord, FixPoint g, FixPoint h) {
        const FixPoint f = g + h;

        auto& map_data = getMapData(coord);

        if(map_data.bInOpenList == false) {
            // not yet in openlist => add at the end of the open list
            map_data.g = g;
            map_data.h = h;
            map_data.f = f;
            map_data.parentCoord = parentCoord;
            map_data.bInOpenList = true;
            openList.push_back(coord);
            map_data.openListIndex = openList.size() - 1;

            trickleUp(openList.size() - 1);
        } else {
            // already on openlist
            if(f >= map_data.f) {
                // new item is worse => don't change anything
                return;
            } else {
                // new item is better => replace
                map_data.g = g;
                map_data.h = h;
                map_data.f = f;
                map_data.parentCoord = parentCoord;
                trickleUp(map_data.openListIndex);
            }
        }
    };

    Coord extractMin() {
        const auto ret = openList[0];
        getMapData(ret).bInOpenList = false;

        openList[0] = openList.back();
        getMapData(openList[0]).openListIndex = 0;
        openList.pop_back();

        if (openList.empty())
            return ret;

        size_t current = 0;
        const auto top = openList[current];  // save root
        const auto topf = getMapData(top).f;
        while(current < openList.size()/2) {

            const auto leftChild = 2*current+1;
            const auto rightChild = leftChild+1;

            // find smaller child
            size_t smallerChild;
            FixPoint smallerChildf;
            if(rightChild < openList.size()) {
                const auto leftf = getMapData(openList[leftChild]).f;
                const auto rightf = getMapData(openList[rightChild]).f;

                if(leftf < rightf) {
                    smallerChild = leftChild;
                    smallerChildf = leftf;
                } else {
                    smallerChild = rightChild;
                    smallerChildf = rightf;
                }
            } else {
                // there is only a left child
                smallerChild = leftChild;
                smallerChildf = getMapData(openList[leftChild]).f;
            }

            // top >= largerChild?
            if(topf <= smallerChildf)
                break;

            // shift child up
            openList[current] = openList[smallerChild];
            getMapData(openList[current]).openListIndex = current;

            // go down one level in the tree
            current = smallerChild;
        }

        openList[current] = top;
        getMapData(openList[current]).openListIndex = current;

        return ret;
    };

    int sizeX;
    int sizeY;
    Coord bestCoord;
    std::vector<TileData> mapData;
    std::vector<Coord> openList;
};

#endif //ASTARSEARCH_H
