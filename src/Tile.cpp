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

#include <Tile.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>

#include <sand.h>
#include <Game.h>
#include <Map.h>
#include <House.h>
#include <SoundPlayer.h>
#include <ScreenBorder.h>
#include <ConcatIterator.h>
#include <Explosion.h>

#include <structures/StructureBase.h>
#include <units/InfantryBase.h>
#include <units/AirUnit.h>

Tile::Tile() {
	type = Terrain_Sand;

	for(int i = 0; i < NUM_HOUSES; i++) {
		explored[i] = currentGame->getGameInitSettings().getGameOptions().startWithExploredMap;
		lastAccess[i] = 0;
	}

	fogColor = PALCOLOR_BLACK;

	owner = INVALID;
	sandRegion = NONE;

	spice = 0;

	sprite = pGFXManager->getObjPic(ObjPic_Terrain);

	for(int i=0; i < NUM_ANGLES; i++) {
        tracksCounter[i] = 0;
	}

	location.x = 0;
	location.y = 0;

	destroyedStructureTile = DestroyedStructure_None;
}


Tile::~Tile() {
}

void Tile::load(InputStream& stream) {
	type = stream.readUint32();

    stream.readBools(&explored[0], &explored[1], &explored[2], &explored[3], &explored[4], &explored[5]);

    bool bLastAccess[NUM_HOUSES];
    stream.readBools(&bLastAccess[0], &bLastAccess[1], &bLastAccess[2], &bLastAccess[3], &bLastAccess[4], &bLastAccess[5]);

    for(int i=0;i<NUM_HOUSES;i++) {
        if(bLastAccess[i] == true) {
            lastAccess[i] = stream.readUint32();
        }
	}

	fogColor = stream.readUint32();

	owner = stream.readSint32();
	sandRegion = stream.readUint32();

	spice = stream.readFixPoint();

	bool bHasDamage, bHasDeadUnits, bHasAirUnits, bHasInfantry, bHasUndergroundUnits, bHasNonInfantryGroundObjects;
	stream.readBools(&bHasDamage, &bHasDeadUnits, &bHasAirUnits, &bHasInfantry, &bHasUndergroundUnits, &bHasNonInfantryGroundObjects);

    if(bHasDamage) {
        Uint32 numDamage = stream.readUint32();
        for(Uint32 i=0; i<numDamage; i++) {
            DAMAGETYPE newDamage;
            newDamage.damageType = stream.readUint32();
            newDamage.tile = stream.readSint32();
            newDamage.realPos.x = stream.readSint32();
            newDamage.realPos.y = stream.readSint32();

            damage.push_back(newDamage);
        }
    }

    if(bHasDeadUnits) {
        Uint32 numDeadUnits = stream.readUint32();
        for(Uint32 i=0; i<numDeadUnits; i++) {
            DEADUNITTYPE newDeadUnit;
            newDeadUnit.type = stream.readUint8();
            newDeadUnit.house = stream.readUint8();
            newDeadUnit.onSand = stream.readBool();
            newDeadUnit.realPos.x = stream.readSint32();
            newDeadUnit.realPos.y = stream.readSint32();
            newDeadUnit.timer = stream.readSint16();

            deadUnits.push_back(newDeadUnit);
        }
    }

	destroyedStructureTile = stream.readSint32();

    bool bTrackCounter[NUM_ANGLES];
    stream.readBools(&bTrackCounter[0], &bTrackCounter[1], &bTrackCounter[2], &bTrackCounter[3], &bTrackCounter[4], &bTrackCounter[5], &bTrackCounter[6], &bTrackCounter[7]);

    for(int i=0; i < NUM_ANGLES; i++) {
        if(bTrackCounter[i] == true) {
            tracksCounter[i] = stream.readSint16();
        }
    }

    if(bHasAirUnits) {
        assignedAirUnitList = stream.readUint32List();
    }

    if(bHasInfantry) {
        assignedInfantryList = stream.readUint32List();
    }

	if(bHasUndergroundUnits) {
	    assignedUndergroundUnitList = stream.readUint32List();
	}

	if(bHasNonInfantryGroundObjects) {
	    assignedNonInfantryGroundObjectList = stream.readUint32List();
	}
}

void Tile::save(OutputStream& stream) const {
	stream.writeUint32(type);

	stream.writeBools(explored[0], explored[1], explored[2], explored[3], explored[4], explored[5]);

    stream.writeBools((lastAccess[0] != 0), (lastAccess[1] != 0), (lastAccess[2] != 0), (lastAccess[3] != 0), (lastAccess[4] != 0), (lastAccess[5] != 0));
    for(int i=0;i<NUM_HOUSES;i++) {
        if(lastAccess[i] != 0) {
            stream.writeUint32(lastAccess[i]);
        }
	}

	stream.writeUint32(fogColor);

	stream.writeUint32(owner);
	stream.writeUint32(sandRegion);

	stream.writeFixPoint(spice);

	stream.writeBools(  !damage.empty(), !deadUnits.empty(), !assignedAirUnitList.empty(),
                        !assignedInfantryList.empty(), !assignedUndergroundUnitList.empty(), !assignedNonInfantryGroundObjectList.empty());

    if(!damage.empty()) {
        stream.writeUint32(damage.size());
        for(std::vector<DAMAGETYPE>::const_iterator iter = damage.begin(); iter != damage.end(); ++iter) {
            stream.writeUint32(iter->damageType);
            stream.writeSint32(iter->tile);
            stream.writeSint32(iter->realPos.x);
            stream.writeSint32(iter->realPos.y);
        }
    }

    if(!deadUnits.empty()) {
        stream.writeUint32(deadUnits.size());
        for(std::vector<DEADUNITTYPE>::const_iterator iter = deadUnits.begin(); iter != deadUnits.end(); ++iter) {
            stream.writeUint8(iter->type);
            stream.writeUint8(iter->house);
            stream.writeBool(iter->onSand);
            stream.writeSint32(iter->realPos.x);
            stream.writeSint32(iter->realPos.y);
            stream.writeSint16(iter->timer);
        }
    }

    stream.writeSint32(destroyedStructureTile);


    stream.writeBools(  (tracksCounter[0] != 0), (tracksCounter[1] != 0), (tracksCounter[2] != 0), (tracksCounter[3] != 0),
                        (tracksCounter[4] != 0), (tracksCounter[5] != 0), (tracksCounter[6] != 0), (tracksCounter[7] != 0));
    for(int i=0; i < NUM_ANGLES; i++) {
        if(tracksCounter[i] != 0) {
            stream.writeSint16(tracksCounter[i]);
        }
    }

    if(!assignedAirUnitList.empty()) {
        stream.writeUint32List(assignedAirUnitList);
    }

	if(!assignedInfantryList.empty()) {
        stream.writeUint32List(assignedInfantryList);
	}

	if(!assignedUndergroundUnitList.empty()) {
	    stream.writeUint32List(assignedUndergroundUnitList);
	}

	if(!assignedNonInfantryGroundObjectList.empty()) {
	    stream.writeUint32List(assignedNonInfantryGroundObjectList);
	}
}

void Tile::assignAirUnit(Uint32 newObjectID) {
	assignedAirUnitList.push_back(newObjectID);
}

void Tile::assignNonInfantryGroundObject(Uint32 newObjectID) {
	assignedNonInfantryGroundObjectList.push_back(newObjectID);
}

int Tile::assignInfantry(Uint32 newObjectID, Sint8 currentPosition) {
	Sint8 i = currentPosition;

	if(currentPosition == -1) {
		bool used[NUM_INFANTRY_PER_TILE];
		int pos;
		for (i = 0; i < NUM_INFANTRY_PER_TILE; i++)
			used[i] = false;


		std::list<Uint32>::const_iterator iter;
		for(iter = assignedInfantryList.begin(); iter != assignedInfantryList.end() ;++iter) {
			InfantryBase* infant = (InfantryBase*) currentGame->getObjectManager().getObject(*iter);
			if(infant == NULL) {
				continue;
			}

			pos = infant->getTilePosition();
			if ((pos >= 0) && (pos < NUM_INFANTRY_PER_TILE))
				used[pos] = true;
		}

		for (i = 0; i < NUM_INFANTRY_PER_TILE; i++) {
			if (used[i] == false) {
				break;
			}
		}

		if ((i < 0) || (i >= NUM_INFANTRY_PER_TILE))
			i = 0;
	}

	assignedInfantryList.push_back(newObjectID);
	return i;
}


void Tile::assignUndergroundUnit(Uint32 newObjectID) {
	assignedUndergroundUnitList.push_back(newObjectID);
}

void Tile::blitGround(int xPos, int yPos) {
	SDL_Rect	source = { static_cast<Sint16>(getTerrainTile()*world2zoomedWorld(TILESIZE)), 0, static_cast<Uint16>(world2zoomedWorld(TILESIZE)), static_cast<Uint16>(world2zoomedWorld(TILESIZE)) };
	SDL_Rect    drawLocation = { static_cast<Sint16>(xPos), static_cast<Sint16>(yPos), static_cast<Uint16>(world2zoomedWorld(TILESIZE)), static_cast<Uint16>(world2zoomedWorld(TILESIZE)) };

	if((hasANonInfantryGroundObject() == false) || (getNonInfantryGroundObject()->isAStructure() == false)) {

		//draw terrain
		if(destroyedStructureTile == DestroyedStructure_None || destroyedStructureTile == DestroyedStructure_Wall) {
            SDL_BlitSurface(sprite[currentZoomlevel], &source, screen, &drawLocation);
		}

		if(destroyedStructureTile != DestroyedStructure_None) {
		    SDL_Surface** pDestroyedStructureSurface = pGFXManager->getObjPic(ObjPic_DestroyedStructure);
		    SDL_Rect source2 = { static_cast<Sint16>(destroyedStructureTile*world2zoomedWorld(TILESIZE)), 0, static_cast<Uint16>(world2zoomedWorld(TILESIZE)), static_cast<Uint16>(world2zoomedWorld(TILESIZE)) };
            SDL_BlitSurface(pDestroyedStructureSurface[currentZoomlevel], &source2, screen, &drawLocation);
		}

		if(!isFogged(pLocalHouse->getHouseID())) {
		    // tracks
		    for(int i=0;i<NUM_ANGLES;i++) {
                if(tracksCounter[i] > 0) {
                    source.x = ((10-i)%8)*world2zoomedWorld(TILESIZE);
                    SDL_BlitSurface(pGFXManager->getObjPic(ObjPic_Terrain_Tracks)[currentZoomlevel], &source, screen, &drawLocation);
                }
		    }

            // damage
		    for(std::vector<DAMAGETYPE>::const_iterator iter = damage.begin(); iter != damage.end(); ++iter) {
                source.x = iter->tile*world2zoomedWorld(TILESIZE);
                SDL_Rect dest = {   static_cast<Sint16>(screenborder->world2screenX(iter->realPos.x) - world2zoomedWorld(TILESIZE)/2),
                                    static_cast<Sint16>(screenborder->world2screenY(iter->realPos.y) - world2zoomedWorld(TILESIZE)/2),
                                    static_cast<Uint16>(world2zoomedWorld(TILESIZE)),
                                    static_cast<Uint16>(world2zoomedWorld(TILESIZE)) };

                if(iter->damageType == Terrain_RockDamage) {
                    SDL_BlitSurface(pGFXManager->getObjPic(ObjPic_RockDamage)[currentZoomlevel], &source, screen, &dest);
                } else {
                    SDL_BlitSurface(pGFXManager->getObjPic(ObjPic_SandDamage)[currentZoomlevel], &source, screen, &drawLocation);
                }
		    }
		}
	}

}

void Tile::blitStructures(int xPos, int yPos) {
	if (hasANonInfantryGroundObject() && getNonInfantryGroundObject()->isAStructure()) {
		//if got a structure, draw the structure, and dont draw any terrain because wont be seen
		bool	done = false;	//only draw it once
		StructureBase* structure = (StructureBase*) getNonInfantryGroundObject();

		for(int i = structure->getX(); (i < structure->getX() + structure->getStructureSizeX()) && !done;  i++) {
            for(int j = structure->getY(); (j < structure->getY() + structure->getStructureSizeY()) && !done;  j++) {
                if(screenborder->isTileInsideScreen(Coord(i,j))
                    && currentGameMap->tileExists(i, j) && (currentGameMap->getTile(i, j)->isExplored(pLocalHouse->getHouseID()) || debug))
                {

                    structure->setFogged(isFogged(pLocalHouse->getHouseID()));

                    if ((i == location.x) && (j == location.y)) {
                        //only this tile will draw it, so will be drawn only once
                        structure->blitToScreen();
                    }

                    done = true;
                }
            }
        }
	}
}

void Tile::blitUndergroundUnits(int xPos, int yPos) {
	if(hasAnUndergroundUnit() && !isFogged(pLocalHouse->getHouseID())) {
	    UnitBase* current = getUndergroundUnit();

	    if(current->isVisible(pLocalHouse->getTeam())) {
		    if(location == current->getLocation()) {
                current->blitToScreen();
		    }
		}
	}
}

void Tile::blitDeadUnits(int xPos, int yPos) {
	if(!isFogged(pLocalHouse->getHouseID())) {
	    for(std::vector<DEADUNITTYPE>::const_iterator iter = deadUnits.begin(); iter != deadUnits.end(); ++iter) {
	        SDL_Rect source = { 0, 0, static_cast<Uint16>(world2zoomedWorld(TILESIZE)), static_cast<Uint16>(world2zoomedWorld(TILESIZE))};
	        SDL_Surface** pSurface = NULL;
	        switch(iter->type) {
                case DeadUnit_Infantry: {
                    pSurface = pGFXManager->getObjPic(ObjPic_DeadInfantry, iter->house);
                    source.x = (iter->timer < 1000 && iter->onSand) ? world2zoomedWorld(TILESIZE) : 0;
                } break;

                case DeadUnit_Infantry_Squashed1: {
                    pSurface = pGFXManager->getObjPic(ObjPic_DeadInfantry, iter->house);
                    source.x = 4 * world2zoomedWorld(TILESIZE);
                } break;

                case DeadUnit_Infantry_Squashed2: {
                    pSurface = pGFXManager->getObjPic(ObjPic_DeadInfantry, iter->house);
                    source.x = 5 * world2zoomedWorld(TILESIZE);
                } break;

                case DeadUnit_Carrall: {
                    pSurface = pGFXManager->getObjPic(ObjPic_DeadAirUnit, iter->house);
                    if(iter->onSand) {
                        source.x = (iter->timer < 1000) ? 5*world2zoomedWorld(TILESIZE) : 4*world2zoomedWorld(TILESIZE);
                    } else {
                        source.x = 3*world2zoomedWorld(TILESIZE);
                    }
                } break;

                case DeadUnit_Ornithopter: {
                    pSurface = pGFXManager->getObjPic(ObjPic_DeadAirUnit, iter->house);
                    if(iter->onSand) {
                        source.x = (iter->timer < 1000) ? 2*world2zoomedWorld(TILESIZE) : world2zoomedWorld(TILESIZE);
                    } else {
                        source.x = 0;
                    }
                } break;

                default: {
                    pSurface = NULL;
                } break;
	        }

	        if(pSurface != NULL) {
                SDL_Rect dest = {   static_cast<Sint16>(screenborder->world2screenX(iter->realPos.x) - world2zoomedWorld(TILESIZE)/2),
                                    static_cast<Sint16>(screenborder->world2screenY(iter->realPos.y) - world2zoomedWorld(TILESIZE)/2),
                                    static_cast<Uint16>(pSurface[currentZoomlevel]->w),
                                    static_cast<Uint16>(pSurface[currentZoomlevel]->h) };
                SDL_BlitSurface(pSurface[currentZoomlevel], &source, screen, &dest);
	        }
	    }
	}
}

void Tile::blitInfantry(int xPos, int yPos) {
	if(hasInfantry() && !isFogged(pLocalHouse->getHouseID())) {
		std::list<Uint32>::const_iterator iter;
		for(iter = assignedInfantryList.begin(); iter != assignedInfantryList.end() ;++iter) {
			InfantryBase* current = (InfantryBase*) currentGame->getObjectManager().getObject(*iter);

			if(current == NULL) {
				continue;
			}

			if(current->isVisible(pLocalHouse->getTeam())) {
			    if(location == current->getLocation()) {
                    current->blitToScreen();
			    }
			}
		}
	}
}

void Tile::blitNonInfantryGroundUnits(int xPos, int yPos) {
	if(hasANonInfantryGroundObject() && !isFogged(pLocalHouse->getHouseID())) {
        std::list<Uint32>::const_iterator iter;
		for(iter = assignedNonInfantryGroundObjectList.begin(); iter != assignedNonInfantryGroundObjectList.end() ;++iter) {
			ObjectBase* current =  currentGame->getObjectManager().getObject(*iter);

            if(current->isAUnit() && current->isVisible(pLocalHouse->getTeam())) {
                if(location == current->getLocation()) {
                    current->blitToScreen();
                }
            }
		}
	}
}


void Tile::blitAirUnits(int xPos, int yPos) {
	if(hasAnAirUnit()) {
		std::list<Uint32>::const_iterator iter;
		for(iter = assignedAirUnitList.begin(); iter != assignedAirUnitList.end() ;++iter) {
			AirUnit* airUnit = (AirUnit*) currentGame->getObjectManager().getObject(*iter);

			if(airUnit == NULL) {
				continue;
			}

			if(!isFogged(pLocalHouse->getHouseID()) || airUnit->getOwner() == pLocalHouse) {
                if(airUnit->isVisible(pLocalHouse->getTeam())) {
                    if(location == airUnit->getLocation()) {
                        airUnit->blitToScreen();
                    }
                }
			}
		}
	}
}

void Tile::blitSelectionRects(int xPos, int yPos) {
    // draw underground selection rectangles
    if(hasAnUndergroundUnit() && !isFogged(pLocalHouse->getHouseID())) {
	    UnitBase* current = getUndergroundUnit();

        if(current != NULL) {
            if(current->isVisible(pLocalHouse->getTeam()) && (location == current->getLocation())) {
                if(current->isSelected()) {
                    current->drawSelectionBox();
                }

                if(current->isSelectedByOtherPlayer()) {
                    current->drawOtherPlayerSelectionBox();
                }
            }
        }
	}


    // draw infantry selection rectangles
    if(hasInfantry() && !isFogged(pLocalHouse->getHouseID())) {
		std::list<Uint32>::const_iterator iter;
		for(iter = assignedInfantryList.begin(); iter != assignedInfantryList.end() ;++iter) {
			InfantryBase* current = dynamic_cast<InfantryBase*>(currentGame->getObjectManager().getObject(*iter));

			if(current == NULL) {
				continue;
			}

            if(current->isVisible(pLocalHouse->getTeam()) && (location == current->getLocation())) {
                if(current->isSelected()) {
                    current->drawSelectionBox();
                }

                if(current->isSelectedByOtherPlayer()) {
                    current->drawOtherPlayerSelectionBox();
                }
            }
		}
	}

    // draw non infantry ground object selection rectangles
	if(hasANonInfantryGroundObject() && !isFogged(pLocalHouse->getHouseID())) {
	    std::list<Uint32>::const_iterator iter;
		for(iter = assignedNonInfantryGroundObjectList.begin(); iter != assignedNonInfantryGroundObjectList.end() ;++iter) {
            ObjectBase* current = currentGame->getObjectManager().getObject(*iter);

            if(current == NULL) {
				continue;
			}

            if(current->isVisible(pLocalHouse->getTeam()) && (location == current->getLocation())) {
                if(current->isSelected()) {
                    current->drawSelectionBox();
                }

                if(current->isSelectedByOtherPlayer()) {
                    current->drawOtherPlayerSelectionBox();
                }
            }
		}
	}

    // draw air unit selection rectangles
	if(hasAnAirUnit() && !isFogged(pLocalHouse->getHouseID())) {
		std::list<Uint32>::const_iterator iter;
		for(iter = assignedAirUnitList.begin(); iter != assignedAirUnitList.end() ;++iter) {
			AirUnit* airUnit = dynamic_cast<AirUnit*>(currentGame->getObjectManager().getObject(*iter));

			if(airUnit == NULL) {
				continue;
			}

            if(airUnit->isVisible(pLocalHouse->getTeam()) && (location == airUnit->getLocation())) {
                if(airUnit->isSelected()) {
                    airUnit->drawSelectionBox();
                }

                if(airUnit->isSelectedByOtherPlayer()) {
                    airUnit->drawOtherPlayerSelectionBox();
                }
            }
		}
	}
}


void Tile::clearTerrain() {
    damage.clear();
    deadUnits.clear();
}


void Tile::selectAllPlayersUnits(int houseID, ObjectBase** lastCheckedObject, ObjectBase** lastSelectedObject) {
	ConcatIterator<Uint32> iterator;
	iterator.addList(assignedInfantryList);
	iterator.addList(assignedNonInfantryGroundObjectList);
	iterator.addList(assignedUndergroundUnitList);
	iterator.addList(assignedAirUnitList);

	while(!iterator.isIterationFinished()) {
		*lastCheckedObject = currentGame->getObjectManager().getObject(*iterator);
		if (((*lastCheckedObject)->getOwner()->getHouseID() == houseID)
			&& !(*lastCheckedObject)->isSelected()
			&& (*lastCheckedObject)->isAUnit()
			&& ((*lastCheckedObject)->isRespondable()))	{

			(*lastCheckedObject)->setSelected(true);
			currentGame->getSelectedList().insert((*lastCheckedObject)->getObjectID());
			currentGame->selectionChanged();
			*lastSelectedObject = *lastCheckedObject;
		}
		++iterator;
	}
}


void Tile::selectAllPlayersUnitsOfType(int houseID, int itemID, ObjectBase** lastCheckedObject, ObjectBase** lastSelectedObject) {
	ConcatIterator<Uint32> iterator;
	iterator.addList(assignedInfantryList);
	iterator.addList(assignedNonInfantryGroundObjectList);
	iterator.addList(assignedUndergroundUnitList);
	iterator.addList(assignedAirUnitList);

	while(!iterator.isIterationFinished()) {
		*lastCheckedObject = currentGame->getObjectManager().getObject(*iterator);
		if (((*lastCheckedObject)->getOwner()->getHouseID() == houseID)
			&& !(*lastCheckedObject)->isSelected()
			&& ((*lastCheckedObject)->getItemID() == itemID)) {

			(*lastCheckedObject)->setSelected(true);
			currentGame->getSelectedList().insert((*lastCheckedObject)->getObjectID());
			currentGame->selectionChanged();
			*lastSelectedObject = *lastCheckedObject;
		}
		++iterator;
	}
}


void Tile::unassignAirUnit(Uint32 objectID) {
	assignedAirUnitList.remove(objectID);
}


void Tile::unassignNonInfantryGroundObject(Uint32 objectID) {
	assignedNonInfantryGroundObjectList.remove(objectID);
}

void Tile::unassignUndergroundUnit(Uint32 objectID) {
	assignedUndergroundUnitList.remove(objectID);
}

void Tile::unassignInfantry(Uint32 objectID, int currentPosition) {
	assignedInfantryList.remove(objectID);
}

void Tile::unassignObject(Uint32 objectID) {
	unassignInfantry(objectID,-1);
	unassignUndergroundUnit(objectID);
	unassignNonInfantryGroundObject(objectID);
	unassignAirUnit(objectID);
}


void Tile::setType(int newType) {
	type = newType;
	destroyedStructureTile = DestroyedStructure_None;

	if (type == Terrain_Spice) {
		spice = currentGame->randomGen.rand(RANDOMSPICEMIN, RANDOMSPICEMAX);
	} else if (type == Terrain_ThickSpice) {
		spice = currentGame->randomGen.rand(RANDOMTHICKSPICEMIN, RANDOMTHICKSPICEMAX);
	} else if (type == Terrain_Dunes) {
	} else {
		spice = 0;
		if (isRock()) {
			sandRegion = NONE;
			if (hasAnUndergroundUnit())	{
				ObjectBase* current;
				std::list<Uint32>::const_iterator iter;
				iter = assignedUndergroundUnitList.begin();

				do {
					current = currentGame->getObjectManager().getObject(*iter);
					++iter;

					if(current == NULL)
						continue;

					unassignUndergroundUnit(current->getObjectID());
					current->destroy();
				} while(iter != assignedUndergroundUnitList.end());
			}

			if(type == Terrain_Mountain) {
				if(hasANonInfantryGroundObject()) {
					ObjectBase* current;
					std::list<Uint32>::const_iterator iter;
					iter = assignedNonInfantryGroundObjectList.begin();

					do {
						current = currentGame->getObjectManager().getObject(*iter);
						++iter;

						if(current == NULL)
							continue;

						unassignNonInfantryGroundObject(current->getObjectID());
						current->destroy();
					} while(iter != assignedNonInfantryGroundObjectList.end());
				}
			}
		}
	}

	for (int i=location.x; i <= location.x+3; i++) {
		for (int j=location.y; j <= location.y+3; j++) {
			if (currentGameMap->tileExists(i, j)) {
				currentGameMap->getTile(i, j)->clearTerrain();
			}
		}
	}
}


void Tile::squash() {
	if(hasInfantry()) {
		InfantryBase* current;
		std::list<Uint32>::const_iterator iter;
		iter = assignedInfantryList.begin();

		do {
			current = (InfantryBase*) currentGame->getObjectManager().getObject(*iter);
			++iter;

			if(current == NULL)
				continue;

			current->squash();
		} while(iter != assignedInfantryList.end());
	}
}


int Tile::getInfantryTeam() {
	int team = NONE;
	if (hasInfantry())
		team = getInfantry()->getOwner()->getTeam();
	return team;
}


FixPoint Tile::harvestSpice() {
	FixPoint oldSpice = spice;

	if((spice - HARVESTSPEED) >= 0) {
		spice -= HARVESTSPEED;
	} else {
		spice = 0;
	}

    if(oldSpice >= RANDOMTHICKSPICEMIN && spice < RANDOMTHICKSPICEMIN) {
        setType(Terrain_Spice);
    }

    if(oldSpice > 0 && spice == 0) {
        setType(Terrain_Sand);
    }

	return (oldSpice - spice);
}


void Tile::setSpice(FixPoint newSpice) {
	if(newSpice <= 0) {
		type = Terrain_Sand;
	} else if(newSpice >= RANDOMTHICKSPICEMIN) {
		type = Terrain_ThickSpice;
	} else {
		type = Terrain_Spice;
	}
	spice = newSpice;
}


AirUnit* Tile::getAirUnit() {
	return dynamic_cast<AirUnit*>(currentGame->getObjectManager().getObject(assignedAirUnitList.front()));
}

ObjectBase* Tile::getGroundObject() {
	if (hasANonInfantryGroundObject())
		return getNonInfantryGroundObject();
	else if (hasInfantry())
		return getInfantry();
	else
		return NULL;
}

InfantryBase* Tile::getInfantry() {
	return dynamic_cast<InfantryBase*>(currentGame->getObjectManager().getObject(assignedInfantryList.front()));
}

ObjectBase* Tile::getNonInfantryGroundObject() {
	return currentGame->getObjectManager().getObject(assignedNonInfantryGroundObjectList.front());
}

UnitBase* Tile::getUndergroundUnit() {
	return dynamic_cast<UnitBase*>(currentGame->getObjectManager().getObject(assignedUndergroundUnitList.front()));
}


/*ObjectBase* Tile::getInfantry(int i)
{
	int count;
	InfantryBase* infantry;
	assignedInfantry.reset();
	while (assignedInfantry.currentNotNull())
	{
		((InfantryBase*)assignedInfantry.getCurrent())->squash();
		assignedInfantry.nextLink();
	}
	return assignedInfantry.removeElement();
}*/


ObjectBase* Tile::getObject() {
	ObjectBase* temp = NULL;
	if (hasAnAirUnit())
		temp = getAirUnit();
	else if (hasANonInfantryGroundObject())
		temp = getNonInfantryGroundObject();
	else if (hasInfantry())
		temp = getInfantry();
	else if (hasAnUndergroundUnit())
		temp = getUndergroundUnit();
	return temp;
}


ObjectBase* Tile::getObjectAt(int x, int y) {
	ObjectBase* temp = NULL;
	if (hasAnAirUnit())
		temp = getAirUnit();
	else if (hasANonInfantryGroundObject())
		temp = getNonInfantryGroundObject();
	else if (hasInfantry())	{
		FixPoint closestDistance = FixPt_MAX;
		Coord atPos, centerPoint;
		InfantryBase* infantry;
		atPos.x = x;
		atPos.y = y;

		std::list<Uint32>::const_iterator iter;
		for(iter = assignedInfantryList.begin(); iter != assignedInfantryList.end() ;++iter) {
			infantry = (InfantryBase*) currentGame->getObjectManager().getObject(*iter);
			if(infantry == NULL)
				continue;

			centerPoint = infantry->getCenterPoint();
			if(distanceFrom(atPos, centerPoint) < closestDistance) {
				closestDistance = distanceFrom(atPos, centerPoint);
				temp = infantry;
			}
		}
	}
	else if (hasAnUndergroundUnit())
		temp = getUndergroundUnit();

	return temp;
}


ObjectBase* Tile::getObjectWithID(Uint32 objectID) {
	ConcatIterator<Uint32> iterator;
	iterator.addList(assignedInfantryList);
	iterator.addList(assignedNonInfantryGroundObjectList);
	iterator.addList(assignedUndergroundUnitList);
	iterator.addList(assignedAirUnitList);

	while(!iterator.isIterationFinished()) {
		if(*iterator == objectID) {
			return currentGame->getObjectManager().getObject(*iterator);
		}
		++iterator;
	}

	return NULL;
}

void Tile::triggerSpiceBloom(House* pTrigger) {
    if (isSpiceBloom()) {
        //a spice bloom
        soundPlayer->playSoundAt(Sound_Bloom, getLocation());
        screenborder->shakeScreen(18);
        if(pTrigger == pLocalHouse) {
            soundPlayer->playVoice(BloomLocated, pLocalHouse->getHouseID());
        }

        setType(Terrain_Spice); // Set this tile to spice first
        currentGameMap->createSpiceField(location, 5);

        Coord realLocation = location*TILESIZE + Coord(TILESIZE/2, TILESIZE/2);

        if(damage.size() < DAMAGE_PER_TILE) {
            DAMAGETYPE newDamage;
            newDamage.tile = SandDamage1;
            newDamage.damageType = Terrain_SandDamage;
            newDamage.realPos = realLocation;

            damage.push_back(newDamage);
        }

        currentGame->getExplosionList().push_back(new Explosion(Explosion_SpiceBloom,realLocation,pTrigger->getHouseID()));
    }
}

void Tile::triggerSpecialBloom(House* pTrigger) {
    if(isSpecialBloom()) {
        setType(Terrain_Sand);

        switch(currentGame->randomGen.rand(0,3)) {
            case 0: {
                // the player gets an randomly choosen amount of credits between 150 and 400
                pTrigger->addCredits(currentGame->randomGen.rand(150, 400),false);
            } break;

            case 1: {
                // The house gets a Trike for free. It spawns beside the special bloom.
                UnitBase* pNewUnit = pTrigger->createUnit(Unit_Trike);
                if(pNewUnit != NULL) {
                    Coord spot = currentGameMap->findDeploySpot(pNewUnit, location);
                    pNewUnit->deploy(spot);
                }
            } break;

            case 2: {
                // One of the AI players on the map (one that has at least one unit) gets a Trike for free. It spawns beside the special bloom.
                int numCandidates = 0;
                for(int i=0;i<NUM_HOUSES;i++) {
                    House* pHouse = currentGame->getHouse(i);
                    if(pHouse != NULL && pHouse->getTeam() != pTrigger->getTeam() && pHouse->getNumUnits() > 0) {
                        numCandidates++;
                    }
                }

                if(numCandidates == 0) {
                    break;
                }

                House* pEnemyHouse = NULL;
                for(int i=0;i<NUM_HOUSES;i++) {
                    House* pHouse = currentGame->getHouse(i);
                    if(pHouse != NULL && pHouse->getTeam() != pTrigger->getTeam() && pHouse->getNumUnits() > 0) {
                        numCandidates--;
                        if(numCandidates == 0) {
                            pEnemyHouse = pHouse;
                            break;
                        }
                    }
                }

                UnitBase* pNewUnit = pEnemyHouse->createUnit(Unit_Trike);
                if(pNewUnit != NULL) {
                    Coord spot = currentGameMap->findDeploySpot(pNewUnit, location);
                    pNewUnit->deploy(spot);
                }

            } break;

            case 3:
            default: {
                // One of the AI players on the map (one that has at least one unit) gets an Infantry unit (3 Soldiers) for free. The spawn beside the special bloom.
                int numCandidates = 0;
                for(int i=0;i<NUM_HOUSES;i++) {
                    House* pHouse = currentGame->getHouse(i);
                    if(pHouse != NULL && pHouse->getTeam() != pTrigger->getTeam() && pHouse->getNumUnits() > 0) {
                        numCandidates++;
                    }
                }

                if(numCandidates == 0) {
                    break;
                }

                House* pEnemyHouse = NULL;
                for(int i=0;i<NUM_HOUSES;i++) {
                    House* pHouse = currentGame->getHouse(i);
                    if(pHouse != NULL && pHouse->getTeam() != pTrigger->getTeam() && pHouse->getNumUnits() > 0) {
                        numCandidates--;
                        if(numCandidates == 0) {
                            pEnemyHouse = pHouse;
                            break;
                        }
                    }
                }

                for(int i=0;i<3;i++) {
                    UnitBase* pNewUnit = pEnemyHouse->createUnit(Unit_Soldier);
                    if(pNewUnit != NULL) {
                        Coord spot = currentGameMap->findDeploySpot(pNewUnit, location);
                        pNewUnit->deploy(spot);
                    }
                }
            } break;
        }
    }
}

bool Tile::hasAStructure() const {
    if(!hasANonInfantryGroundObject()) {
        return false;
    }

    ObjectBase* pObject = currentGame->getObjectManager().getObject(assignedNonInfantryGroundObjectList.front());
    return ( (pObject != NULL) && pObject->isAStructure() );
}

bool Tile::isFogged(int houseID) {
	if(debug)
		return false;

	if(currentGame->getGameInitSettings().getGameOptions().fogOfWar == false) {
		return false;
	} else if((currentGame->getGameCycleCount() - lastAccess[houseID]) >= MILLI2CYCLES(10*1000)) {
		return true;
	} else {
		return false;
	}
}

Uint32 Tile::getRadarColor(House* pHouse, bool radar) {
	if(isExplored(pHouse->getHouseID()) || debug) {
		if(isFogged(pHouse->getHouseID()) && radar) {
			return fogColor;
		} else {
			ObjectBase* pObject = getObject();
			if(pObject != NULL) {
			    int color;

			    if(pObject->getItemID() == Unit_Sandworm) {
					color = PALCOLOR_WHITE;
				} else {
                    switch(pObject->getOwner()->getHouseID()) {
                        case HOUSE_HARKONNEN:   color = PALCOLOR_HARKONNEN;  break;
                        case HOUSE_ATREIDES:    color = PALCOLOR_ATREIDES;   break;
                        case HOUSE_ORDOS:       color = PALCOLOR_ORDOS;      break;
                        case HOUSE_FREMEN:      color = PALCOLOR_FREMEN;     break;
                        case HOUSE_SARDAUKAR:   color = PALCOLOR_SARDAUKAR;  break;
                        case HOUSE_MERCENARY:   color = PALCOLOR_MERCENARY;  break;
                        default:                color = PALCOLOR_BLACK;      break;
                    }
				}

				if(pObject->isAUnit()) {
                    fogColor = getColorByTerrainType(getType());
				} else {
                    fogColor = color;
				}

				// units and structures of the enemy are not visible if no radar
				if(!radar && !debug && (pObject->getOwner()->getTeam() != pHouse->getTeam())) {
					return PALCOLOR_BLACK;
				} else {
                    return color;
				}
			} else {
			    fogColor = getColorByTerrainType(getType());

				if(!radar && !debug) {
					return PALCOLOR_BLACK;
				} else {
                    return fogColor;
				}
			}
		}
	} else {
		return PALCOLOR_BLACK;
	}
}

int Tile::getTerrainTile() const {
    switch(type) {
        case Terrain_Slab: {
            return TerrainTile_Slab;
        } break;

        case Terrain_Sand: {
            return TerrainTile_Sand;
        } break;

        case Terrain_Rock: {
            //determine which surounding tiles are rock
            bool up = (currentGameMap->tileExists(location.x,location.y-1) == false) || (currentGameMap->getTile(location.x, location.y-1)->isRock() == true);
            bool right = (currentGameMap->tileExists(location.x+1,location.y) == false) || (currentGameMap->getTile(location.x+1, location.y)->isRock() == true);
            bool down = (currentGameMap->tileExists(location.x,location.y+1) == false) || (currentGameMap->getTile(location.x, location.y+1)->isRock() == true);
            bool left = (currentGameMap->tileExists(location.x-1,location.y) == false) || (currentGameMap->getTile(location.x-1, location.y)->isRock() == true);

            return TerrainTile_Rock + (up | (right << 1) | (down << 2) | (left << 3));
        } break;

        case Terrain_Dunes: {
            //determine which surounding tiles are dunes
            bool up = (currentGameMap->tileExists(location.x,location.y-1) == false) || (currentGameMap->getTile(location.x, location.y-1)->getType() == Terrain_Dunes);
            bool right = (currentGameMap->tileExists(location.x+1,location.y) == false) || (currentGameMap->getTile(location.x+1, location.y)->getType() == Terrain_Dunes);
            bool down = (currentGameMap->tileExists(location.x,location.y+1) == false) || (currentGameMap->getTile(location.x, location.y+1)->getType() == Terrain_Dunes);
            bool left = (currentGameMap->tileExists(location.x-1,location.y) == false) || (currentGameMap->getTile(location.x-1, location.y)->getType() == Terrain_Dunes);

            return TerrainTile_Dunes + (up | (right << 1) | (down << 2) | (left << 3));
        } break;

        case Terrain_Mountain: {
            //determine which surounding tiles are mountains
            bool up = (currentGameMap->tileExists(location.x,location.y-1) == false) || (currentGameMap->getTile(location.x, location.y-1)->isMountain() == true);
            bool right = (currentGameMap->tileExists(location.x+1,location.y) == false) || (currentGameMap->getTile(location.x+1, location.y)->isMountain() == true);
            bool down = (currentGameMap->tileExists(location.x,location.y+1) == false) || (currentGameMap->getTile(location.x, location.y+1)->isMountain() == true);
            bool left = (currentGameMap->tileExists(location.x-1,location.y) == false) || (currentGameMap->getTile(location.x-1, location.y)->isMountain() == true);

            return TerrainTile_Mountain + (up | (right << 1) | (down << 2) | (left << 3));
        } break;

        case Terrain_Spice: {
            //determine which surounding tiles are spice
            bool up = (currentGameMap->tileExists(location.x,location.y-1) == false) || (currentGameMap->getTile(location.x, location.y-1)->isSpice() == true);
            bool right = (currentGameMap->tileExists(location.x+1,location.y) == false) || (currentGameMap->getTile(location.x+1, location.y)->isSpice() == true);
            bool down = (currentGameMap->tileExists(location.x,location.y+1) == false) || (currentGameMap->getTile(location.x, location.y+1)->isSpice() == true);
            bool left = (currentGameMap->tileExists(location.x-1,location.y) == false) || (currentGameMap->getTile(location.x-1, location.y)->isSpice() == true);

            return TerrainTile_Spice + (up | (right << 1) | (down << 2) | (left << 3));
        } break;

        case Terrain_ThickSpice: {
            //determine which surounding tiles are thick spice
            bool up = (currentGameMap->tileExists(location.x,location.y-1) == false) || (currentGameMap->getTile(location.x, location.y-1)->getType() == Terrain_ThickSpice);
            bool right = (currentGameMap->tileExists(location.x+1,location.y) == false) || (currentGameMap->getTile(location.x+1, location.y)->getType() == Terrain_ThickSpice);
            bool down = (currentGameMap->tileExists(location.x,location.y+1) == false) || (currentGameMap->getTile(location.x, location.y+1)->getType() == Terrain_ThickSpice);
            bool left = (currentGameMap->tileExists(location.x-1,location.y) == false) || (currentGameMap->getTile(location.x-1, location.y)->getType() == Terrain_ThickSpice);

            return TerrainTile_ThickSpice + (up | (right << 1) | (down << 2) | (left << 3));
        } break;

        case Terrain_SpiceBloom: {
            return TerrainTile_SpiceBloom;
        } break;

        case Terrain_SpecialBloom: {
            return TerrainTile_SpecialBloom;
        } break;

        default: {
            throw std::runtime_error("Tile::getTerrainTile(): Invalid terrain type");
        } break;
    }
}

int Tile::getHideTile(int houseID) const {

    // are all surounding tiles explored?
    if( ((currentGameMap->tileExists(location.x,location.y-1) == false) || (currentGameMap->getTile(location.x, location.y-1)->isExplored(houseID) == true))
        && ((currentGameMap->tileExists(location.x+1,location.y) == false) || (currentGameMap->getTile(location.x+1, location.y)->isExplored(houseID) == true))
        && ((currentGameMap->tileExists(location.x,location.y+1) == false) || (currentGameMap->getTile(location.x, location.y+1)->isExplored(houseID) == true))
        && ((currentGameMap->tileExists(location.x-1,location.y) == false) || (currentGameMap->getTile(location.x-1, location.y)->isExplored(houseID) == true))) {
        return 0;
    }

    //determine what tiles are unexplored
    bool up = (currentGameMap->tileExists(location.x,location.y-1) == false) || (currentGameMap->getTile(location.x, location.y-1)->isExplored(houseID) == false);
    bool right = (currentGameMap->tileExists(location.x+1,location.y) == false) || (currentGameMap->getTile(location.x+1, location.y)->isExplored(houseID) == false);
    bool down = (currentGameMap->tileExists(location.x,location.y+1) == false) || (currentGameMap->getTile(location.x, location.y+1)->isExplored(houseID) == false);
    bool left = (currentGameMap->tileExists(location.x-1,location.y) == false) || (currentGameMap->getTile(location.x-1, location.y)->isExplored(houseID) == false);

    return (up | (right << 1) | (down << 2) | (left << 3));
}

int Tile::getFogTile(int houseID) const {

    // are all surounding tiles fogged?
    if( ((currentGameMap->tileExists(location.x,location.y-1) == false) || (currentGameMap->getTile(location.x, location.y-1)->isFogged(houseID) == false))
        && ((currentGameMap->tileExists(location.x+1,location.y) == false) || (currentGameMap->getTile(location.x+1, location.y)->isFogged(houseID) == false))
        && ((currentGameMap->tileExists(location.x,location.y+1) == false) || (currentGameMap->getTile(location.x, location.y+1)->isFogged(houseID) == false))
        && ((currentGameMap->tileExists(location.x-1,location.y) == false) || (currentGameMap->getTile(location.x-1, location.y)->isFogged(houseID) == false))) {
        return 0;
    }

    //determine what tiles are fogged
    bool up = (currentGameMap->tileExists(location.x,location.y-1) == false) || (currentGameMap->getTile(location.x, location.y-1)->isFogged(houseID) == true);
    bool right = (currentGameMap->tileExists(location.x+1,location.y) == false) || (currentGameMap->getTile(location.x+1, location.y)->isFogged(houseID) == true);
    bool down = (currentGameMap->tileExists(location.x,location.y+1) == false) || (currentGameMap->getTile(location.x, location.y+1)->isFogged(houseID) == true);
    bool left = (currentGameMap->tileExists(location.x-1,location.y) == false) || (currentGameMap->getTile(location.x-1, location.y)->isFogged(houseID) == true);

    return (up | (right << 1) | (down << 2) | (left << 3));
}
