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

#include <Map.h>
#include <Game.h>
#include <units/UnitBase.h>

#include <stdlib.h>

#define MAX_NODES_CHECKED   (128*128)

AStarSearch::AStarSearch(Map* pMap, UnitBase* pUnit, Coord start, Coord destination) {
    FixPoint rotationSpeed = 1.0_fix/(currentGame->objectData.data[pUnit->getItemID()][pUnit->getOriginalHouseID()].turnspeed * TILESIZE);

    sizeX = pMap->getSizeX();
    sizeY = pMap->getSizeY();

    mapData = static_cast<TileData*>(calloc(sizeX*sizeY, sizeof(TileData)));
    if(mapData == nullptr) {
        throw std::bad_alloc();
    }

    FixPoint heuristic = blockDistance(start, destination);
    FixPoint smallestHeuristic = FixPt_MAX;
    bestCoord = Coord::Invalid();

    //if the unit is not directly next to its destination or it is and the destination is unblocked
    if ((heuristic > 1.5_fix) || (pUnit->canPass(destination.x, destination.y) == true)) {

        putOnOpenListIfBetter(start, Coord::Invalid(), 0 , heuristic);

        std::vector<short> depthCheckCount(std::min(sizeX, sizeY));

        int numNodesChecked = 0;
        while(openList.empty() == false) {
            Coord currentCoord = extractMin();

            if (getMapData(currentCoord).h < smallestHeuristic) {
                smallestHeuristic = getMapData(currentCoord).h;
                bestCoord = currentCoord;
            }

            if(currentCoord == destination) {
                // destination found
                smallestHeuristic = getMapData(currentCoord).h;
                bestCoord = currentCoord;
                break;
            }

            if (numNodesChecked < MAX_NODES_CHECKED) {
                //push a node for each direction we could go
                for (int angle=0; angle<=7; angle++) {
                    Coord nextCoord = pMap->getMapPos(angle, currentCoord);
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
                            int posAngle = currentGameMap->getPosAngle(getMapData(currentCoord).parentCoord, currentCoord);
                            g += angleDiff(angle,posAngle) * rotationSpeed;
                        }

                        FixPoint h = blockDistance(nextCoord, destination);

                        if(getMapData(nextCoord).bClosed == false) {
                            putOnOpenListIfBetter(nextCoord, currentCoord, g, h);
                        }
                    }

                }
            }

            if (getMapData(currentCoord).bClosed == false) {

                int depth = std::max(abs(currentCoord.x - destination.x), abs(currentCoord.y - destination.y));

                if(depth < std::min(sizeX,sizeY)) {

                    // calculate maximum number of tiles in a square shape
                    // you could look at without success around a destination x,y
                    // with a specific k distance before knowing that it is
                    // imposible to get to the destination.  Each time the astar
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


                    int x = destination.x;
                    int y = destination.y;
                    int k = depth;
                    int horizontal = std::min(sizeX-1, x+(k-1)) - std::max(0, x-(k-1)) + 1;
                    int vertical = std::min(sizeY-1, y+k) - std::max(0, y-k) + 1;
                    int depthCheckMax = ((x-k >= 0) ? vertical : 0) +  ((x+k < sizeX) ? vertical : 0) + ((y-k >= 0) ? horizontal : 0) +  ((y+k < sizeY) ? horizontal : 0);


                    if (++depthCheckCount[k] >= depthCheckMax) {
                        // we have searched a whole square around destination, it can't be reached
                        break;
                    }
                }

                getMapData(currentCoord).bClosed = true;
                numNodesChecked++;
            }
        }

    }


}

AStarSearch::~AStarSearch() {
    free(mapData);
}

