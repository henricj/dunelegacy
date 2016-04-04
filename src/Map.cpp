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

#include <Map.h>

#include <globals.h>

#include <Game.h>
#include <House.h>
#include <ScreenBorder.h>
#include <ConcatIterator.h>
#include <sand.h>

#include <units/UnitBase.h>
#include <units/InfantryBase.h>
#include <units/AirUnit.h>
#include <structures/StructureBase.h>

#include <limits.h>
#include <stack>
#include <set>

Map::Map(int xSize, int ySize)
 : sizeX(xSize), sizeY(ySize), tiles(nullptr), lastSinglySelectedObject(nullptr) {

	tiles = new Tile[sizeX*sizeY];

	for(int i=0; i<sizeX; i++) {
		for(int j=0; j<sizeY; j++) {
			tiles[i+j*sizeX].location.x = i;
			tiles[i+j*sizeX].location.y = j;
		}
	}
}


Map::~Map() {
	delete[] tiles;
}

void Map::load(InputStream& stream) {
	sizeX = stream.readSint32();
	sizeY = stream.readSint32();

	for (int i = 0; i < sizeX; i++) {
		for (int j = 0; j < sizeY; j++) {
			getTile(i,j)->load(stream);
			getTile(i,j)->location.x = i;
			getTile(i,j)->location.y = j;
		}
	}
}

void Map::save(OutputStream& stream) const {
	stream.writeSint32(sizeX);
	stream.writeSint32(sizeY);

	for (int i = 0; i < sizeX; i++) {
		for (int j = 0; j < sizeY; j++) {
			getTile(i,j)->save(stream);
		}
	}
}

void Map::createSandRegions() {
	std::stack<Tile*> tileQueue;
	std::vector<bool> visited(sizeX * sizeY);

	for(int i = 0; i < sizeX; i++) {
		for(int j = 0; j < sizeY; j++)	{
			getTile(i,j)->setSandRegion(NONE);
		}
	}

    int	region = 0;
	for(int i = 0; i < sizeX; i++) {
		for(int j = 0; j < sizeY; j++) {
			if(!getTile(i,j)->isRock() && !visited[j*sizeX+i]) {
				tileQueue.push(getTile(i,j));

				while(!tileQueue.empty()) {
					Tile* pTile = tileQueue.top();
					tileQueue.pop();

					pTile->setSandRegion(region);
					for(int angle = 0; angle < NUM_ANGLES; angle++) {
						Coord pos = getMapPos(angle, pTile->location);
						if(tileExists(pos) && !getTile(pos)->isRock() && !visited[pos.y*sizeX + pos.x]) {
							tileQueue.push(getTile(pos));
							visited[pos.y*sizeX + pos.x] = true;
						}
					}
				}
				region++;
			}
		}
	}
}

void Map::damage(Uint32 damagerID, House* damagerOwner, const Coord& realPos, Uint32 bulletID, FixPoint damage, int damageRadius, bool air) {
	Coord location = Coord(realPos.x/TILESIZE, realPos.y/TILESIZE);

    std::set<Uint32>	affectedAirUnits;
    std::set<Uint32>    affectedGroundAndUndergroundUnits;

	for(int i = location.x-2; i <= location.x+2; i++) {
		for(int j = location.y-2; j <= location.y+2; j++) {
			if(tileExists(i, j)) {
			    Tile* pTile = getTile(i,j);

                affectedAirUnits.insert(pTile->getAirUnitList().begin(), pTile->getAirUnitList().end());
                affectedGroundAndUndergroundUnits.insert(pTile->getInfantryList().begin(), pTile->getInfantryList().end());
                affectedGroundAndUndergroundUnits.insert(pTile->getUndergroundUnitList().begin(), pTile->getUndergroundUnitList().end());
                affectedGroundAndUndergroundUnits.insert(pTile->getNonInfantryGroundObjectList().begin(), pTile->getNonInfantryGroundObjectList().end());
			}
		}
	}

    if(bulletID == Bullet_Sandworm) {
        std::set<Uint32>::const_iterator iter;
        for(iter = affectedGroundAndUndergroundUnits.begin(); iter != affectedGroundAndUndergroundUnits.end() ;++iter) {
            ObjectBase* pObject = currentGame->getObjectManager().getObject(*iter);
            if((pObject->getItemID() != Unit_Sandworm) && (pObject->isAGroundUnit() || pObject->isInfantry()) && (pObject->getLocation() == location)) {
                pObject->setVisible(VIS_ALL, false);
                pObject->handleDamage( lround(damage), damagerID, damagerOwner);
            }
        }
    } else {

        if(air == true) {
            // air damage
            if((bulletID == Bullet_DRocket) || (bulletID == Bullet_Rocket) || (bulletID == Bullet_TurretRocket)|| (bulletID == Bullet_SmallRocket)) {
                std::set<Uint32>::const_iterator iter;
                for(iter = affectedAirUnits.begin(); iter != affectedAirUnits.end() ;++iter) {
                    AirUnit* pAirUnit = dynamic_cast<AirUnit*>(currentGame->getObjectManager().getObject(*iter));

                    if(pAirUnit == nullptr)
                        continue;


                    Coord centerPoint = pAirUnit->getCenterPoint();
                    int distance = lround(distanceFrom(centerPoint, realPos));

                    if(distance <= damageRadius) {
                        if(bulletID == Bullet_DRocket) {
                            if((pAirUnit->getItemID() != Unit_Carryall) && (pAirUnit->getItemID() != Unit_Sandworm) && (pAirUnit->getItemID() != Unit_Frigate)) {
                                // try to deviate
                                if(currentGame->randomGen.randFixPoint() < getDeviateWeakness((HOUSETYPE) pAirUnit->getOriginalHouseID())) {
                                    pAirUnit->deviate(damagerOwner);
                                }
                            }
                        } else {
                            int scaledDamage = lround(damage) >> (distance/4 + 1);
                            pAirUnit->handleDamage(scaledDamage, damagerID, damagerOwner);
                        }
                    }

                }
            }
        } else {
            // non air damage
            std::set<Uint32>::const_iterator iter;
            for(iter = affectedGroundAndUndergroundUnits.begin(); iter != affectedGroundAndUndergroundUnits.end() ;++iter) {
                ObjectBase* pObject = currentGame->getObjectManager().getObject(*iter);

                if(pObject->isAStructure()) {
                    StructureBase* pStructure = dynamic_cast<StructureBase*>(pObject);

                    Coord topLeftCorner = pStructure->getLocation()*TILESIZE;
                    Coord bottomRightCorner = topLeftCorner + pStructure->getStructureSize()*TILESIZE;

                    if(realPos.x >= topLeftCorner.x && realPos.y >= topLeftCorner.y && realPos.x < bottomRightCorner.x && realPos.y < bottomRightCorner.y) {
                        pStructure->handleDamage(lround(damage), damagerID, damagerOwner);

                        if( (bulletID == Bullet_LargeRocket || bulletID == Bullet_Rocket || bulletID == Bullet_TurretRocket || bulletID == Bullet_SmallRocket)
                            && (pStructure->getHealth() < pStructure->getMaxHealth()/2)) {
                            if(pStructure->getNumSmoke() < 5) {
                                pStructure->addSmoke(realPos, currentGame->getGameCycleCount());
                            }
                        }
                    }


                } else if(pObject->isAUnit()) {
                    UnitBase* pUnit = dynamic_cast<UnitBase*>(pObject);

                    Coord centerPoint = pUnit->getCenterPoint();
                    int distance = lround(distanceFrom(centerPoint, realPos));

                    if(distance <= damageRadius) {
                        if(bulletID == Bullet_DRocket) {
                            if((pUnit->getItemID() != Unit_Carryall) && (pUnit->getItemID() != Unit_Sandworm) && (pUnit->getItemID() != Unit_Frigate)) {
                                // try to deviate
                                if(currentGame->randomGen.randFixPoint() < getDeviateWeakness((HOUSETYPE) pUnit->getOriginalHouseID())) {
                                    pUnit->deviate(damagerOwner);
                                }
                            }
                        } else if(bulletID == Bullet_Sonic) {
                            pUnit->handleDamage(lround(damage), damagerID, damagerOwner);
                        } else {
                            int scaledDamage = lround(damage) >> (distance/16 + 1);
                            pUnit->handleDamage(scaledDamage, damagerID, damagerOwner);
                        }
                    }
                }
            }


            if(currentGameMap->tileExists(location)) {

                Tile* pTile = currentGameMap->getTile(location);

                if(((bulletID == Bullet_Rocket) || (bulletID == Bullet_TurretRocket) || (bulletID == Bullet_SmallRocket) || (bulletID == Bullet_LargeRocket))
                    && (!pTile->hasAGroundObject() || !pTile->getGroundObject()->isAStructure()) )
                {

                    if(((pTile->getType() == Terrain_Rock) && (pTile->getTerrainTile() == Tile::TerrainTile_RockFull)) || (pTile->getType() == Terrain_Slab)) {
                        if(pTile->getType() == Terrain_Slab) {
                            pTile->setType(Terrain_Rock);
                            pTile->setDestroyedStructureTile(Destroyed1x1Structure);
                            pTile->setOwner(NONE);
                        }

                        pTile->addDamage(Tile::Terrain_RockDamage, (bulletID==Bullet_SmallRocket) ? Tile::RockDamage1 : Tile::RockDamage2, realPos);

                    } else if((pTile->getType() == Terrain_Sand) || (pTile->getType() == Terrain_Spice)) {
                        if(bulletID==Bullet_SmallRocket) {
                            pTile->addDamage(Tile::Terrain_SandDamage, currentGame->randomGen.rand(Tile::SandDamage1, Tile::SandDamage2), realPos);
                        } else {
                            pTile->addDamage(Tile::Terrain_SandDamage, currentGame->randomGen.rand(Tile::SandDamage3, Tile::SandDamage4), realPos);
                        }
                    }
                }
            }

        }
    }

	if((bulletID != Bullet_Sonic) && (bulletID != Bullet_Sandworm) && tileExists(location) && getTile(location)->isSpiceBloom()) {
        getTile(location)->triggerSpiceBloom(damagerOwner);
	}
}

/**
    Check each tile which surrounds the building location to make sure there
    is no building. We want to ensure we don't block units in and have
    traffic lanes for our troops
**/

bool Map::isAStructureGap(int x, int y, int buildingSizeX, int buildingSizeY) const {

	bool hasAGap = true;

    // Spacing rues don't apply for rocket turrets
    if(buildingSizeX == 1){
        return hasAGap;
    }

    int xMin = x - 1;
    int xMax = x + buildingSizeX + 1;
    int yMin = y - 1;
    int yMax = y + buildingSizeY + 1;

	for(int i = xMin; i < xMax; i++) {
		for(int j = yMin; j < yMax; j++) {
            if(currentGameMap->tileExists(i,j)
               && !((i == xMin || i == xMax) && (j == yMin || j == yMax))){ //Corners are ok as units can get through

                const Tile* pTile = getTile(i,j);

                if((pTile->hasAStructure() && !pTile->isConcrete())) { // I need some more conditions to make it ignore units

                        hasAGap = false;
                }
            }
        }
    }

	return hasAGap;
}

bool Map::okayToPlaceStructure(int x, int y, int buildingSizeX, int buildingSizeY, bool tilesRequired, const House* pHouse, bool bIgnoreUnits) const {
	bool withinBuildRange = false;

	for(int i = x; i < x + buildingSizeX; i++) {
		for(int j = y; j < y + buildingSizeY; j++) {
            if(!currentGameMap->tileExists(i,j)) {
                return false;
            }

            const Tile* pTile = getTile(i,j);

            if(!pTile->isRock() || (tilesRequired && !pTile->isConcrete()) || (!bIgnoreUnits && pTile->isBlocked())) {
				return false;
			}

			if((pHouse == nullptr) || isWithinBuildRange(i, j, pHouse)) {
				withinBuildRange = true;
			}
		}
	}
	return withinBuildRange;
}


bool Map::isWithinBuildRange(int x, int y, const House* pHouse) const {
	bool withinBuildRange = false;

	for (int i = x - BUILDRANGE; i <= x + BUILDRANGE; i++)
		for (int j = y - BUILDRANGE; j <= y + BUILDRANGE; j++)
			if (tileExists(i, j) && (getTile(i,j)->getOwner() == pHouse->getHouseID()))
				withinBuildRange = true;

	return withinBuildRange;
}

/**
    This method figures out the direction of tile pos relative to tile source.
    \param  source  the starting point
    \param  pos     the destination
    \return one of RIGHT, RIGHTUP, UP, LEFTUP, LEFT, LEFTDOWN, DOWN, RIGHTDOWN or INVALID
*/
int Map::getPosAngle(const Coord& source, const Coord& pos) const {
	if(pos.x > source.x) {
		if(pos.y > source.y) {
			return RIGHTDOWN;
        } else if(pos.y < source.y) {
			return RIGHTUP;
		} else {
			return RIGHT;
		}
	} else if(pos.x < source.x) {
		if(pos.y > source.y) {
			return LEFTDOWN;
		} else if(pos.y < source.y) {
			return LEFTUP;
		} else {
			return LEFT;
		}
	} else {
		if(pos.y > source.y) {
			return DOWN;
		} else if(pos.y < source.y) {
			return UP;
		} else {
			return INVALID;
		}
	}
}

/**
    This method calculates the coordinate of one of the neighbor tiles of source
    \param  angle   one of RIGHT, RIGHTUP, UP, LEFTUP, LEFT, LEFTDOWN, DOWN, RIGHTDOWN
    \param  source  the tile to calculate neighbor tiles from
*/
Coord Map::getMapPos(int angle, const Coord& source) const {
	switch (angle)
	{
		case (RIGHT):       return Coord(source.x + 1 , source.y);       break;
		case (RIGHTUP):     return Coord(source.x + 1 , source.y - 1);   break;
		case (UP):          return Coord(source.x     , source.y - 1);   break;
		case (LEFTUP):      return Coord(source.x - 1 , source.y - 1);   break;
		case (LEFT):        return Coord(source.x - 1 , source.y);       break;
		case (LEFTDOWN):    return Coord(source.x - 1 , source.y + 1);   break;
		case (DOWN):        return Coord(source.x     , source.y + 1);   break;
		case (RIGHTDOWN):   return Coord(source.x + 1 , source.y + 1);   break;
		default:            return Coord(source.x     , source.y);       break;
	}
}

//building size is num squares
Coord Map::findDeploySpot(UnitBase* pUnit, const Coord origin, const Coord gatherPoint, const Coord buildingSize) const {
	FixPoint	closestDistance = FixPt_MAX;
	Coord       closestPoint;
	Coord		size;

	bool	found = false;
    bool    foundClosest = false;

	int	counter = 0;
	int	depth = 0;
	int edge;

    if(pUnit->isAFlyingUnit()) {
        return origin;
    }

	int ranX = origin.x;
	int ranY = origin.y;

	do {
		edge = currentGame->randomGen.rand(0, 3);
		switch(edge) {
            case 0:	//right edge
                ranX = origin.x + buildingSize.x + depth;
                ranY = currentGame->randomGen.rand(origin.y - depth, origin.y + buildingSize.y + depth);
                break;
            case 1:	//top edge
                ranX = currentGame->randomGen.rand(origin.x - depth, origin.x + buildingSize.x + depth);
                ranY = origin.y - depth - ((buildingSize.y == 0) ? 0 : 1);
                break;
            case 2:	//left edge
                ranX = origin.x - depth - ((buildingSize.x == 0) ? 0 : 1);
                ranY = currentGame->randomGen.rand(origin.y - depth, origin.y + buildingSize.y + depth);
                break;
            case 3: //bottom edge
                ranX = currentGame->randomGen.rand(origin.x - depth, origin.x + buildingSize.x + depth);
                ranY = origin.y + buildingSize.y + depth;
                break;
            default:
                break;
		}

		bool bOK2Deploy = pUnit->canPass(ranX, ranY);

		if(pUnit->isTracked() && tileExists(ranX, ranY) && getTile(ranX, ranY)->hasInfantry()) {
		    // we do not deploy on enemy infantry
		    bOK2Deploy = false;
		}

		if(bOK2Deploy) {
			if(gatherPoint.isInvalid()) {
				closestPoint.x = ranX;
				closestPoint.y = ranY;
				found = true;
			} else {
				Coord temp = Coord(ranX, ranY);
				if(blockDistance(temp, gatherPoint) < closestDistance) {
					closestDistance = blockDistance(temp, gatherPoint);
					closestPoint.x = ranX;
					closestPoint.y = ranY;
					foundClosest = true;
				}
			}
		}

		if(counter++ >= 100) {
		    //if hasn't found a spot on tempObject layer in 100 tries, goto next

			counter = 0;
			if(++depth > (std::max(currentGameMap->getSizeX(), currentGameMap->getSizeY()))) {
				closestPoint.invalidate();
				found = true;
				fprintf(stderr, "Cannot find deploy position because the map is full!\n"); fflush(stderr);
			}
		}
	} while (!found && (!foundClosest || (counter > 0)));

	return closestPoint;
}

/**
    This method finds the tile which is at a map border and is at minimal distance to the structure
    specified by origin and buildingsSize. This method is especcially useful for Carryalls and Frigates
    that have to enter the map to deliver units.
    \param origin           the position of the structure in map coordinates
    \param buildingSize    the number of tiles occupied by the building (e.g. 3x2 for refinery)
*/
Coord Map::findClosestEdgePoint(const Coord& origin, const Coord& buildingSize) const {
	int closestDistance = INT_MAX;
	Coord closestPoint;

	if(origin.x < (sizeX - (origin.x + buildingSize.x))) {
		closestPoint.x = 0;
		closestDistance = origin.x;
	} else {
		closestPoint.x = sizeX - 1;
		closestDistance = sizeX - (origin.x + buildingSize.x);
	}
	closestPoint.y = origin.y;

	if(origin.y < closestDistance) {
		closestPoint.x = origin.x;
		closestPoint.y = 0;
		closestDistance = origin.y;
	}

	if((sizeY - (origin.y + buildingSize.y)) < closestDistance) {
		closestPoint.x = origin.x;
		closestPoint.y = sizeY - 1;
		// closestDistance = origin.y;  //< Not needed anymore
	}

	return closestPoint;
}


void Map::removeObjectFromMap(Uint32 objectID) {
	for(int y = 0; y < sizeY ; y++) {
		for(int x = 0 ; x < sizeX ; x++) {
			getTile(x,y)->unassignObject(objectID);
		}
	}
}

void Map::selectObjects(int houseID, int x1, int y1, int x2, int y2, int realX, int realY, bool objectARGMode) {

	ObjectBase	*lastCheckedObject = nullptr;
	ObjectBase *lastSelectedObject = nullptr;

	//if selection rectangle is checking only one tile and has shift selected we want to add/ remove that unit from the selected group of units
	if(!objectARGMode) {
		currentGame->unselectAll(currentGame->getSelectedList());
		currentGame->getSelectedList().clear();
		currentGame->selectionChanged();
	}

	if((x1 == x2) && (y1 == y2) && tileExists(x1, y1)) {

        if(getTile(x1,y1)->isExplored(houseID) || debug) {
            lastCheckedObject = getTile(x1,y1)->getObjectAt(realX, realY);
        } else {
		    lastCheckedObject = nullptr;
		}

		if((lastCheckedObject != nullptr) && (lastCheckedObject->getOwner()->getHouseID() == houseID)) {
			if((lastCheckedObject == lastSinglySelectedObject) && ( !lastCheckedObject->isAStructure())) {
                for(int i = screenborder->getTopLeftTile().x; i <= screenborder->getBottomRightTile().x; i++) {
                    for(int j = screenborder->getTopLeftTile().y; j <= screenborder->getBottomRightTile().y; j++) {
                        if(tileExists(i,j) && getTile(i,j)->hasAnObject()) {
                            getTile(i,j)->selectAllPlayersUnitsOfType(houseID, lastSinglySelectedObject->getItemID(), &lastCheckedObject, &lastSelectedObject);
                        }
                    }
				}
				lastSinglySelectedObject = nullptr;

			} else if(!lastCheckedObject->isSelected())	{

				lastCheckedObject->setSelected(true);
				currentGame->getSelectedList().insert(lastCheckedObject->getObjectID());
				currentGame->selectionChanged();
				lastSelectedObject = lastCheckedObject;
				lastSinglySelectedObject = lastSelectedObject;

			} else if(objectARGMode) {
			    //holding down shift, unselect this unit
				lastCheckedObject->setSelected(false);
				currentGame->getSelectedList().erase(lastCheckedObject->getObjectID());
				currentGame->selectionChanged();
			}

		} else {
			lastSinglySelectedObject = nullptr;
		}

	} else {
		lastSinglySelectedObject = nullptr;
		for(int i = std::min(x1, x2); i <= std::max(x1, x2); i++) {
            for(int j = std::min(y1, y2); j <= std::max(y1, y2); j++) {
                if(tileExists(i,j) && getTile(i,j)->hasAnObject() && getTile(i,j)->isExplored(houseID) && !getTile(i,j)->isFogged(houseID)) {
                    getTile(i,j)->selectAllPlayersUnits(houseID, &lastCheckedObject, &lastSelectedObject);
                }
            }
        }
	}

	//select an enemy unit if none of your units found
	if(currentGame->getSelectedList().empty() && (lastCheckedObject != nullptr) && !lastCheckedObject->isSelected()) {
		lastCheckedObject->setSelected(true);
		lastSelectedObject = lastCheckedObject;
		currentGame->getSelectedList().insert(lastCheckedObject->getObjectID());
		currentGame->selectionChanged();
	} else if (lastSelectedObject != nullptr) {
		lastSelectedObject->playSelectSound();	//we only want one unit responding
	}
}


bool Map::findSpice(Coord& destination, const Coord& origin) const {
	bool found = false;

	int	counter = 0;
	int depth = 1;

	do {
        int ranX;
        int ranY;
		do {
			ranX = currentGame->randomGen.rand(origin.x-depth, origin.x + depth);
			ranY = currentGame->randomGen.rand(origin.y-depth, origin.y + depth);
		} while(((ranX >= (origin.x+1 - depth)) && (ranX < (origin.x + depth))) && ((ranY >= (origin.y+1 - depth)) && (ranY < (origin.y + depth))));

		if(tileExists(ranX,ranY) && !getTile(ranX,ranY)->hasAGroundObject() && getTile(ranX,ranY)->hasSpice()) {
			found = true;
			destination.x = ranX;
			destination.y = ranY;
		}

		counter++;
		if(counter >= 100) {
            //if hasn't found a spot on tempObject layer in 100 tries, goto next
			counter = 0;
			depth++;
		}

		if(depth > std::max(sizeX, sizeY)) {
			return false;	//there is possibly no spice left anywhere on map
		}
	} while (!found);

	if((depth > 1) && (getTile(origin)->hasSpice())) {
		destination = origin;
	}

	return true;
}

/**
    This method fixes surounding thick spice tiles after spice gone to make things look smooth.
    \param coord    the coordinate where spice was removed from
*/
void Map::spiceRemoved(const Coord& coord) {

	if(tileExists(coord)) {	//this is the center tile
		if(getTile(coord)->getType() == Terrain_Sand) {
			//thickspice tiles can't handle non-(thick)spice tiles next to them, if this happens after changes, make it non thick
			for(int i = coord.x-1; i <= coord.x+1; i++) {
                for(int j = coord.y-1; j <= coord.y+1; j++) {
                    if (tileExists(i, j) && (((i==coord.x) && (j!=coord.y)) || ((i!=coord.x) && (j==coord.y))) && getTile(i,j)->isThickSpice()) {
                        //only check tile right, up, left and down of this one
                        getTile(i,j)->setType(Terrain_Spice);
                    }
                }
			}
		}
	}
}

void Map::viewMap(int playerTeam, const Coord& location, int maxViewRange) {

//makes map viewable in an area like as shown below

//				       *****
//                   *********
//                  *****T*****
//                   *********
//                     *****

    Coord   check;
	check.x = location.x - maxViewRange;
	if(check.x < 0) {
		check.x = 0;
	}

	while((check.x < sizeX) && ((check.x - location.x) <=  maxViewRange)) {
		check.y = (location.y - lookDist[abs(check.x - location.x)]);
		if (check.y < 0) check.y = 0;

		while((check.y < sizeY) && ((check.y - location.y) <= lookDist[abs(check.x - location.x)])) {
			if(distanceFrom(location, check) <= maxViewRange) {
                for(int i = 0; i < NUM_HOUSES; i++) {
                    House* pHouse = currentGame->getHouse(i);
                    if((pHouse != nullptr) && (pHouse->getTeam() == playerTeam)) {
                        getTile(check)->setExplored(i,currentGame->getGameCycleCount());
                    }
                }
			}

			check.y++;
		}

		check.x++;
		check.y = location.y;
	}
}

/**
    Creates a spice field of the given radius at the given location.
    \param  location            the location in tile coordinates
    \param  radius              the radius in tiles (0 = only one tile is filled)
    \param  centerIsThickSpice  if set the center is filled with thick spice
*/
void Map::createSpiceField(Coord location, int radius, bool centerIsThickSpice) {
    Coord offset;
    for(offset.x = -radius; offset.x <= radius; offset.x++) {
        for(offset.y = -radius; offset.y <= radius; offset.y++) {
            if(currentGameMap->tileExists(location + offset)) {
                Tile* pTile = currentGameMap->getTile(location + offset);

                if(pTile->isSand() && (distanceFrom(location, location + offset) <= radius)) {
                    if(centerIsThickSpice && (offset.x == 0) && (offset.y == 0)) {
                        pTile->setType(Terrain_ThickSpice);
                    } else {
                        pTile->setType(Terrain_Spice);
                    }
                }
            }
        }
    }
}
