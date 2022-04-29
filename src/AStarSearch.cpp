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

#include <AStarSearch.h>

#include <globals.h>

#include <Game.h>
#include <Map.h>
#include <units/UnitBase.h>

#include <cstdlib>

inline constexpr auto MAX_NODES_CHECKED = 128 * 128;

AStarSearch::AStarSearch(Map* pMap)
    : sizeX(pMap->getSizeX()), sizeY(pMap->getSizeY()), bestCoord{} { //, heap_compare_(mapData) {

    mapData.resize(sizeX * sizeY);

    depthCheckCount.resize(std::min(sizeX, sizeY));
}

void AStarSearch::Search(Map* pMap, UnitBase* pUnit, Coord start, Coord destination) {
#if 1
    std::ranges::fill(mapData, TileData{});
    std::ranges::fill(depthCheckCount, 0);
#else
    // Clobber PODs the evil way...
    memset(&mapData[0], 0, sizeof(mapData[0]) * mapData.size());
    memset(&depthCheckCount[0], 0, sizeof(depthCheckCount[0]) * depthCheckCount.size());
#endif
    openList.clear();
    openList.reserve(2 * std::max(sizeX, sizeY));

    const auto* const destinationTile = pMap->getTile(destination);

    const auto& object_data = dune::globals::currentGame->objectData;
    const auto& item_data   = object_data.data[pUnit->getItemID()];
    const auto turnspeed    = item_data[static_cast<int>(pUnit->getOriginalHouseID())].turnspeed;

    const FixPoint rotationSpeed = 1_fix / (turnspeed * TILESIZE);

    const auto heuristic   = blockDistance(start, destination);
    auto smallestHeuristic = FixPt_MAX;
    bestCoord              = nullptr;

    // if the unit is not directly next to its destination or it is and the destination is unblocked
    if (heuristic <= 1.5_fix && !pUnit->canPassTile(destinationTile))
        return;

    putOnOpenListIfBetter(pMap->getKey(start.x, start.y), start, nullptr, 0, heuristic);

    int numNodesChecked = 0;
    while (auto* const currentTileData = extractMin()) {
        auto& map_data = *currentTileData; // getMapData(currentKey);

        const auto& currentCoord = map_data.coord;

        // assert(currentKey == pMap->getKey(currentCoord.x, currentCoord.y));

        if (map_data.h < smallestHeuristic) {
            smallestHeuristic = map_data.h;
            bestCoord         = currentTileData;
        }

        if (currentCoord == destination) {
            // destination found
            smallestHeuristic = map_data.h;
            bestCoord         = currentTileData;
            break;
        }

        if (numNodesChecked < MAX_NODES_CHECKED) {
            // push a node for each direction we could go
            pMap->for_each_angle(currentCoord.x, currentCoord.y, [&](ANGLETYPE angle, Tile& nextTile) {
                if (!pUnit->canPassTile(&nextTile))
                    return;

                const auto& nextCoord = nextTile.location;
                const auto nextKey    = pMap->getKey(nextTile);

                assert(nextKey == pMap->getKey(nextCoord.x, nextCoord.y));

                auto g = map_data.g;

                auto difficulty =
                    (pUnit->isAFlyingUnit() ? FixPoint(1) : pUnit->getTerrainDifficulty(nextTile.getType()));

                if ((nextCoord.x != currentCoord.x) && (nextCoord.y != currentCoord.y)) {
                    // add diagonal movement cost
                    difficulty *= FixPt_SQRT2;
                }

                g += difficulty;

                if (map_data.parentKey) {
                    // add cost of turning time
                    // assert(map_data.parentKey == pMap->getKey(mapData[map_data.parentKey].coord.x,
                    // mapData[map_data.parentKey].coord.y));
                    const auto posAngle = Map::getPosAngle(map_data.parentKey->coord, currentCoord);
                    g += angleDiff(angle, posAngle) * rotationSpeed;
                }

                const auto& next_map_data = getMapData(nextKey);
                if (!next_map_data.bClosed) {
                    const auto h = blockDistance(nextCoord, destination);

                    putOnOpenListIfBetter(nextKey, nextCoord, &map_data, g, h);
                }
            });
#if 0
                for (int angle=0; angle<=7; angle++) {
                    const Coord nextCoord = pMap->getMapPos(angle, currentCoord);
                    if(pUnit->canPass(nextCoord.x, nextCoord.y)) {
                        Tile& nextTile = *(pMap->getTile(nextCoord));
                        FixPoint g = getMapData(currentCoord).g;

                        if((nextCoord.x != currentCoord.x) && (nextCoord.y != currentCoord.y)) {
                            //add diagonal movement cost
                            g += FixPt_SQRT2*(pUnit->isAFlyingUnit() ? 1.0_fix : pUnit->getTerrainDifficulty((TERRAINTYPE) nextTile.getType()));
                        } else {
                            g += (pUnit->isAFlyingUnit() ? 1.0_fix : pUnit->getTerrainDifficulty((TERRAINTYPE) nextTile.getType()));
                        }

                        if(getMapData(currentCoord).parentCoord.isValid())  {
                            //add cost of turning time
                            const int posAngle = currentGameMap->getPosAngle(getMapData(currentCoord).parentCoord, currentCoord);
                            g += angleDiff(angle,posAngle) * rotationSpeed;
                        }

                        FixPoint h = blockDistance(nextCoord, destination);

                        if(getMapData(nextCoord).bClosed == false) {
                            putOnOpenListIfBetter(nextCoord, currentCoord, g, h);
                        }
                    }
                }
#endif // 0
        }

        if (!map_data.bClosed) {
            const int depth = std::max(abs(currentCoord.x - destination.x), abs(currentCoord.y - destination.y));

            if (depth < std::min(sizeX, sizeY)) {

                // calculate maximum number of tiles in a square shape
                // you could look at without success around a destination x,y
                // with a specific k distance before knowing that it is
                // impossible to get to the destination.  Each time the astar
                // algorithm pushes a node with a max diff of k,
                // depthcheckcount(k) is incremented, if it reaches the
                // value in depthcheckmax(x,y,k), we know we have done a full
                // square around target, and thus it is impossible to reach
                // the target, so we should try and get closer if possible,
                // but otherwise stop
                //
                // Examples on 6x4 map:
                //
                //  ......
                //  ..###.     - k=1 => 3x3 Square
                //  ..# #.     - (x,y)=(3,2) => Square completely inside map
                //  ..###.     => depthcheckmax(3,2,1) = 8
                //
                //  .#....
                //  ##....     - k=1 => 3x3 Square
                //  ......     - (x,y)=(0,0) => Square only partly inside map
                //  ......     => depthcheckmax(0,0,1) = 3
                //
                //  ...#..
                //  ...#..     - k=2 => 5x5 Square
                //  ...#..     - (x,y)=(0,1) => Square only partly inside map
                //  ####..     => depthcheckmax(0,1,2) = 7

                const auto x             = destination.x;
                const auto y             = destination.y;
                const auto k             = depth;
                const auto horizontal    = std::min(sizeX - 1, x + (k - 1)) - std::max(0, x - (k - 1)) + 1;
                const auto vertical      = std::min(sizeY - 1, y + k) - std::max(0, y - k) + 1;
                const auto depthCheckMax = ((x - k >= 0) ? vertical : 0) + ((x + k < sizeX) ? vertical : 0)
                                         + ((y - k >= 0) ? horizontal : 0) + ((y + k < sizeY) ? horizontal : 0);

                if (++depthCheckCount[k] >= depthCheckMax) {
                    // we have searched a whole square around destination, it can't be reached
                    break;
                }
            }

            map_data.bClosed = true;
            numNodesChecked++;
        }
    }
}

bool AStarSearch::getFoundPath(Map* pMap, std::vector<Coord>& path) const {
    path.clear();

    if (!bestCoord) {
        return false;
    }

    for (const auto* p = bestCoord; p->parentKey; p = p->parentKey) {
        path.push_back(p->coord);
    }

    return true;
}

void AStarSearch::putOnOpenListIfBetter(int key, const Coord& coord, TileData* parentKey, FixPoint g, FixPoint h) {
    const FixPoint f = g + h;

    auto& map_data = getMapData(key);

    if (map_data.bInOpenList) {
        if (map_data.f <= f) {
            return;
        }
    }

    map_data.g           = g;
    map_data.h           = h;
    map_data.f           = f;
    map_data.coord       = coord;
    map_data.parentKey   = parentKey;
    map_data.bInOpenList = true;
    // This will result in duplicate entries for the same location, but the
    // newest one will always be found first and subsequent closed ones will
    // be ignored.
    openList.emplace_back(open_list{f, &map_data});
    std::push_heap(std::begin(openList), std::end(openList));
}

AStarSearch::TileData* AStarSearch::extractMin() {
    while (!openList.empty()) {
        auto* const ret = openList.front().key;

        std::pop_heap(std::begin(openList), std::end(openList));
        openList.pop_back();

        if (!ret->bClosed)
            return ret;
    }

    return nullptr;
}

AStarSearch::~AStarSearch() = default;
