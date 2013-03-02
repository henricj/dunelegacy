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


#include <players/OldAIPlayer.h>

#include <Game.h>
#include <GameInitSettings.h>
#include <Map.h>
#include <sand.h>
#include <House.h>

#include <structures/StructureBase.h>
#include <structures/BuilderBase.h>
#include <structures/StarPort.h>
#include <structures/ConstructionYard.h>
#include <units/UnitBase.h>



OldAIPlayer::OldAIPlayer(House* associatedHouse, std::string playername, Uint8 difficulty)
 : Player(associatedHouse, playername), difficulty(difficulty) {
	attackTimer = ((2-difficulty) * MILLI2CYCLES(2*60*1000)) + getRandomGen().rand(MILLI2CYCLES(8*60*1000), MILLI2CYCLES(11*60*1000));
	buildTimer = 0;
}

OldAIPlayer::OldAIPlayer(InputStream& stream, House* associatedHouse) : Player(stream, associatedHouse) {
    OldAIPlayer::init();

	difficulty = stream.readUint8();
	attackTimer = stream.readSint32();
	buildTimer = stream.readSint32();

	Uint32 NumPlaceLocations = stream.readUint32();
	for(Uint32 i = 0; i < NumPlaceLocations; i++) {
        Sint32 x = stream.readSint32();
        Sint32 y = stream.readSint32();

		placeLocations.push_back(Coord(x,y));
	}
}

void OldAIPlayer::init() {
}


OldAIPlayer::~OldAIPlayer() {
}

void OldAIPlayer::save(OutputStream& stream) const {
    Player::save(stream);

    stream.writeUint8(difficulty);
	stream.writeSint32(attackTimer);
    stream.writeSint32(buildTimer);

	stream.writeUint32(placeLocations.size());
	std::list<Coord>::const_iterator iter;
	for(iter = placeLocations.begin(); iter != placeLocations.end(); ++iter) {
		stream.writeSint32(iter->x);
		stream.writeSint32(iter->y);
	}
}



void OldAIPlayer::update() {
	bool bConstructionYardChecked = false;
	if(buildTimer == 0) {

		RobustList<const StructureBase*>::const_iterator iter;
		for(iter = getStructureList().begin(); iter != getStructureList().end(); ++iter) {
            const StructureBase* pStructure = *iter;

            //if this players structure, and its a heavy factory, build something
            if(pStructure->getOwner() == getHouse()) {

                if((pStructure->isRepairing() == false) && (pStructure->getHealth() < pStructure->getMaxHealth())) {
                    doRepair(pStructure);
                }

                const BuilderBase* pBuilder = dynamic_cast<const BuilderBase*>(pStructure);
                if(pBuilder != NULL) {

                    if((pBuilder->getHealth() >= pBuilder->getMaxHealth()) && (pBuilder->isUpgrading() == false) && (pBuilder->getCurrentUpgradeLevel() < pBuilder->getMaxUpgradeLevel())) {
                        doUpgrade(pBuilder);
                    }

                    switch (pStructure->getItemID()) {

                        case Structure_Barracks: {
                            if((getHouse()->hasLightFactory() == false) && (getHouse()->hasHeavyFactory() == false)) {
                                if((getHouse()->getCredits() > 100) && (pBuilder->getProductionQueueSize() < 1) && (pBuilder->getBuildListSize() > 0)) {
                                    doBuildRandom(pBuilder);
                                }
                            }
                        } break;

                        case Structure_LightFactory: {
                            if(getHouse()->hasHeavyFactory() == false) {
                                if((getHouse()->getCredits() > 200) && (pBuilder->getProductionQueueSize() < 1) && (pBuilder->getBuildListSize() > 0)) {
                                    doBuildRandom(pBuilder);
                                }
                            }
                        } break;

                        case Structure_WOR: {
                            if(getHouse()->hasHeavyFactory() == false) {
                                if((getHouse()->getCredits() > 100) && (pBuilder->getProductionQueueSize() < 1) && (pBuilder->getBuildListSize() > 0)) {
                                    doBuildRandom(pBuilder);
                                }
                            }
                        } break;

                        case Structure_ConstructionYard: {
                            if(bConstructionYardChecked == false) {
                                bConstructionYardChecked = true;
                                if(getHouse()->getCredits() > 100) {
                                    if((pBuilder->getProductionQueueSize() < 1) && (pBuilder->getBuildListSize() > 0)) {
                                        Uint32 itemID = NONE;
                                        if(getHouse()->getProducedPower() < 50 && pBuilder->isAvailableToBuild(Structure_WindTrap)) {
                                            itemID = Structure_WindTrap;
                                        } else if(getHouse()->getNumItems(Structure_Refinery) < 2 && pBuilder->isAvailableToBuild(Structure_Refinery)) {
                                            itemID = Structure_Refinery;
                                        } else if(getHouse()->getProducedPower() < 150 && pBuilder->isAvailableToBuild(Structure_WindTrap)) {
                                            itemID = Structure_WindTrap;
                                        } else if((getHouse()->hasRadar() == false) && (currentGame->techLevel >= 2) && pBuilder->isAvailableToBuild(Structure_Radar)) {
                                            itemID = Structure_Radar;
                                        } else if((getHouse()->getNumItems(Structure_WOR) <= 0) && (currentGame->techLevel >= 2) && pBuilder->isAvailableToBuild(Structure_WOR)) {
                                            itemID = Structure_WOR;
                                        } else if((getHouse()->getNumItems(Structure_RocketTurret) < 2) && (currentGame->techLevel >= 6) && pBuilder->isAvailableToBuild(Structure_RocketTurret)) {
                                            itemID = Structure_RocketTurret;
                                        } else if(getHouse()->getProducedPower() < 250 && pBuilder->isAvailableToBuild(Structure_WindTrap)) {
                                            itemID = Structure_WindTrap;
                                        } else if((getHouse()->hasLightFactory() == false) && (currentGame->techLevel >= 3) && pBuilder->isAvailableToBuild(Structure_LightFactory)) {
                                            itemID = Structure_LightFactory;
                                        } else if(getHouse()->getNumItems(Structure_Refinery) < 3 && pBuilder->isAvailableToBuild(Structure_Refinery)) {
                                            itemID = Structure_Refinery;
                                        } else if((getHouse()->getNumItems(Structure_RocketTurret) < 4) && (currentGame->techLevel >= 6) && pBuilder->isAvailableToBuild(Structure_RocketTurret)) {
                                            itemID = Structure_RocketTurret;
                                        } else if((getHouse()->getNumItems(Structure_HeavyFactory) <= 0) && (currentGame->techLevel >= 4) && pBuilder->isAvailableToBuild(Structure_HeavyFactory)) {
                                            itemID = Structure_HeavyFactory;
                                        } else if(getHouse()->getProducedPower() < 350 && pBuilder->isAvailableToBuild(Structure_WindTrap)) {
                                            itemID = Structure_WindTrap;
                                        } else if((getHouse()->getNumItems(Structure_HighTechFactory) <= 0) && (currentGame->techLevel >= 5) && pBuilder->isAvailableToBuild(Structure_HighTechFactory)) {
                                            itemID = Structure_HighTechFactory;
                                        } else if((getHouse()->getNumItems(Structure_RocketTurret) < 6) && (currentGame->techLevel >= 6) && pBuilder->isAvailableToBuild(Structure_RocketTurret)) {
                                            itemID = Structure_RocketTurret;
                                        } else if((getHouse()->getNumItems(Structure_RepairYard) <= 0) && (currentGame->techLevel >= 5) && pBuilder->isAvailableToBuild(Structure_RepairYard)) {
                                            itemID = Structure_RepairYard;
                                        } else if(getHouse()->getProducedPower() < 450 && pBuilder->isAvailableToBuild(Structure_WindTrap)) {
                                            itemID = Structure_WindTrap;
                                        } else if((getHouse()->getNumItems(Structure_StarPort) <= 0) && (currentGame->techLevel >= 6) && pBuilder->isAvailableToBuild(Structure_StarPort)) {
                                            itemID = Structure_StarPort;
                                        } else if((getHouse()->getNumItems(Structure_IX) <= 0) && (currentGame->techLevel >= 7) && pBuilder->isAvailableToBuild(Structure_IX)) {
                                            itemID = Structure_IX;
                                        } else if((getHouse()->getNumItems(Structure_Palace) <= 0) && (currentGame->techLevel >= 8) && pBuilder->isAvailableToBuild(Structure_Palace)) {
                                            itemID = Structure_Palace;
                                        } else if((getHouse()->getNumItems(Structure_RocketTurret) < 10) && (currentGame->techLevel >= 6) && pBuilder->isAvailableToBuild(Structure_RocketTurret)) {
                                            itemID = Structure_RocketTurret;
                                        } else if(getHouse()->getProducedPower() < 550 && pBuilder->isAvailableToBuild(Structure_WindTrap)) {
                                            itemID = Structure_WindTrap;
                                        } else if((getHouse()->getNumItems(Structure_WOR) < 2) && (currentGame->techLevel >= 2) && pBuilder->isAvailableToBuild(Structure_WOR)) {
                                            itemID = Structure_WOR;
                                        } else if((getHouse()->getNumItems(Structure_LightFactory) < 2) && (currentGame->techLevel >= 3) && pBuilder->isAvailableToBuild(Structure_LightFactory)) {
                                            itemID = Structure_LightFactory;
                                        } else if((getHouse()->getNumItems(Structure_HeavyFactory) < 2) && (currentGame->techLevel >= 4) && pBuilder->isAvailableToBuild(Structure_HeavyFactory)) {
                                            itemID = Structure_HeavyFactory;
                                        } else if((getHouse()->getNumItems(Structure_Palace) < 2) && (currentGame->techLevel >= 8) && pBuilder->isAvailableToBuild(Structure_Palace)) {
                                            itemID = Structure_Palace;
                                        } else if(getHouse()->getProducedPower() < 650 && pBuilder->isAvailableToBuild(Structure_WindTrap)) {
                                            itemID = Structure_WindTrap;
                                        } else if((getHouse()->getNumItems(Structure_RocketTurret) < 20) && (currentGame->techLevel >= 6) && pBuilder->isAvailableToBuild(Structure_RocketTurret)) {
                                            itemID = Structure_RocketTurret;
                                        }

                                        if(itemID != NONE) {
                                            Coord	placeLocation, location;
                                            location = findPlaceLocation(itemID);

                                            if(location.x >= 0) {
                                                placeLocation = location;
                                                if(getGameInitSettings().getGameOptions().concreteRequired) {
                                                    int i, j,
                                                        incI, incJ,
                                                        startI, startJ;

                                                    if(getMap().isWithinBuildRange(location.x, location.y, getHouse())) {
                                                        startI = location.x, startJ = location.y, incI = 1, incJ = 1;
                                                    } else if(getMap().isWithinBuildRange(location.x + getStructureSize(itemID).x - 1, location.y, getHouse())) {
                                                        startI = location.x + getStructureSize(itemID).x - 1, startJ = location.y, incI = -1, incJ = 1;
                                                    } else if(getMap().isWithinBuildRange(location.x, location.y + getStructureSize(itemID).y - 1, getHouse())) {
                                                        startI = location.x, startJ = location.y + getStructureSize(itemID).y - 1, incI = 1, incJ = -1;
                                                    } else {
                                                        startI = location.x + getStructureSize(itemID).x - 1, startJ = location.y + getStructureSize(itemID).y - 1, incI = -1, incJ = -1;
                                                    }

                                                    for(i = startI; abs(i - startI) < getStructureSize(itemID).x; i += incI) {
                                                        for(j = startJ; abs(j - startJ) < getStructureSize(itemID).y; j += incJ) {
                                                            const Tile *pTile = getMap().getTile(i, j);

                                                            if(pTile == NULL) {
                                                                fprintf(stderr,"%s - Line %d:getTile(%d,%d) returned NULL\n", __FILE__, __LINE__,i,j);
                                                                fflush(stderr);
                                                            }

                                                            if(pTile->getType() != Terrain_Slab) {
                                                                placeLocations.push_back(Coord(i,j));
                                                                doProduceItem(pBuilder, Structure_Slab1);
                                                            }
                                                        }
                                                    }
                                                }

                                                placeLocations.push_back(placeLocation);
                                                doProduceItem(pBuilder, itemID);
                                            }
                                        }
                                    }
                                }
                            }

                            if(pBuilder->isWaitingToPlace()) {
                                //find total region of possible placement and place in random ok position
                                int itemID = pBuilder->getCurrentProducedItem();
                                Coord itemsize = getStructureSize(itemID);

                                //see if there is already a spot to put it stored
                                if(!placeLocations.empty()) {
                                    Coord location = placeLocations.front();
                                    const ConstructionYard* pConstYard = dynamic_cast<const ConstructionYard*>(pBuilder);
                                    if(getMap().okayToPlaceStructure(location.x, location.y, itemsize.x, itemsize.y, false, pConstYard->getOwner())) {
                                        doPlaceStructure(pConstYard, location.x, location.y);
                                        placeLocations.pop_front();
                                    } else if(itemID == Structure_Slab1) {
                                        //forget about concrete
                                        doCancelItem(pConstYard, Structure_Slab1);
                                        placeLocations.pop_front();
                                    } else {
                                        //cancel item
                                        doCancelItem(pConstYard, itemID);
                                        placeLocations.pop_front();
                                    }
                                }
                            }

                        } break;

                        case Structure_HeavyFactory: {
                            if((getHouse()->getCredits() > 600) && (pBuilder->getProductionQueueSize() < 1) && (pBuilder->getBuildListSize() > 0)) {

                                if(getHouse()->getNumItems(Unit_Harvester) < 2*getHouse()->getNumItems(Structure_Refinery)) {
                                    doProduceItem(pBuilder, Unit_Harvester);
                                } else {
                                    doBuildRandom(pBuilder);
                                }
                            }
                        } break;

                        case Structure_HighTechFactory: {
                            if((getHouse()->getCredits() > 800) && (pBuilder->getProductionQueueSize() < 1)) {

                                if(getHouse()->getNumItems(Unit_Carryall) < getHouse()->getNumItems(Unit_Harvester)) {
                                    doProduceItem(pBuilder, Unit_Carryall);
                                } else {
                                    doProduceItem(pBuilder, Unit_Ornithopter);
                                }
                            }
                        } break;

                        case Structure_StarPort: {
                            const StarPort* pStarPort = dynamic_cast<const StarPort*>(pBuilder);
                            if(pStarPort->okToOrder())	{
                                // order max 6 units
                                int num = 6;
                                while((num > 0) && (getHouse()->getCredits() > 2000)) {
                                    doBuildRandom(pStarPort);
                                    num--;
                                }
                                doPlaceOrder(pStarPort);
                            }
                        } break;

                        default: {
                            break;
                        }
                    }
                }
            }

		}

		buildTimer = ((2-difficulty) * MILLI2CYCLES(1000)) + getRandomGen().rand(300, 400) * (  getHouse()->getNumItems(Structure_HeavyFactory)
                                                                                                + getHouse()->getNumItems(Structure_LightFactory)
                                                                                                + getHouse()->getNumItems(Structure_WOR) );

	}


	if(attackTimer > 0) {
	    attackTimer--;
	} else {
        Coord destination;
        const UnitBase* pLeaderUnit = NULL;
        RobustList<const UnitBase*>::const_iterator iter;
	    for(iter = getUnitList().begin(); iter != getUnitList().end(); ++iter) {
            const UnitBase *pUnit = *iter;
            if (pUnit->isRespondable()
                && (pUnit->getOwner() == getHouse())
                && pUnit->isActive()
                && !(pUnit->getAttackMode() == HUNT)
                && (pUnit->getAttackMode() == AREAGUARD)
                && (pUnit->getItemID() != Unit_Harvester)
                && (pUnit->getItemID() != Unit_MCV)
                && (pUnit->getItemID() != Unit_Carryall)
                && (pUnit->getItemID() != Unit_Saboteur)) {

                if(pLeaderUnit == NULL) {
                    pLeaderUnit = pUnit;

                    //default destination
                    destination.x = pLeaderUnit->getX();
                    destination.y = pLeaderUnit->getY();

                    const StructureBase* closestStructure = pLeaderUnit->findClosestTargetStructure();
                    if(closestStructure) {
                        destination = closestStructure->getClosestPoint(pLeaderUnit->getLocation());
                    } else {
                        const UnitBase* closestUnit = pLeaderUnit->findClosestTargetUnit();
                        if(closestUnit) {
                            destination.x = closestUnit->getX();
                            destination.y = closestUnit->getY();
                        }
                    }
                }

                doMove2Pos(pUnit, destination.x, destination.y, false);
                doSetAttackMode(pUnit, HUNT);
            }
        }

		//reset timer for next attack
		attackTimer = getRandomGen().rand(10000, 20000);
	}

	if(buildTimer > 0) {
		buildTimer--;
	}
}

void OldAIPlayer::onIncrementStructures(int itemID) {
}

void OldAIPlayer::onDecrementStructures(int itemID, const Coord& location) {
	/* // no good idea to rebuild everything
	//rebuild the structure if its the original gameType

	if (((currentGame->gameType == GAMETYPE_CAMPAIGN) || (currentGame->gameType == GAMETYPE_SKIRMISH)) && !structureList.empty()) {
		RobustList<StructureBase*>::const_iterator iter;
		for(iter = structureList.begin(); iter != structureList.end(); ++iter) {
			StructureBase* structure = *iter;
			if ((structure->getItemID() == Structure_ConstructionYard) && (structure->getOwner() == this)) {
				Coord *newLocation;

				if (getGameInitSettings().isConcreteRequired()) {
					int i, j,
						incI, incJ,
						startI, startJ;

					if (getMap().isWithinBuildRange(location->x, location->y, this))
						startI = location->x, startJ = location->y, incI = 1, incJ = 1;
					else if (getMap().isWithinBuildRange(location->x + getStructureSize(itemID).x - 1, location->y, this))
						startI = location->x + getStructureSize(itemID).x - 1, startJ = location->y, incI = -1, incJ = 1;
					else if (getMap().isWithinBuildRange(location->x, location->y + getStructureSize(itemID).y - 1, this))
						startI = location->x, startJ = location->y + getStructureSize(itemID).y - 1, incI = 1, incJ = -1;
					else
						startI = location->x + getStructureSize(itemID).x - 1, startJ = location->y + getStructureSize(itemID).y - 1, incI = -1, incJ = -1;

					for (i = startI; abs(i - startI) < getStructureSize(itemID).x; i += incI) {
						for (j = startJ; abs(j - startJ) < getStructureSize(itemID).y; j += incJ) {
							const Tile* pTile = getMap().getTile(i, j);

							if(pTile == NULL) {
								fprintf(stderr,"%s - Line %d:getTile(%d,%d) returned NULL\n", __FILE__, __LINE__,i,j);
								fflush(stderr);
							}

							if(pTile->getType() != Terrain_Slab) {
								placeLocations.push_back(Coord(i,j));
								((ConstructionYard*)structure)->ProduceItem(Structure_Slab1);
							}
						}
					}
				}

				((ConstructionYard*)structure)->ProduceItem(itemID);
				placeLocations.push_back(*location);
				break;
			}
		}
	}*/
}

void OldAIPlayer::onDamage(const ObjectBase* pObject, int damage, Uint32 damagerID) {
    if(pObject->isAStructure()) {
	    //scramble some free units to defend
        scrambleUnitsAndDefend(damagerID);
    } else if(pObject->getItemID() == Unit_Harvester) {
	    //scramble some free units to defend
        scrambleUnitsAndDefend(damagerID);

        const ObjectBase* pDamager = getObject(damagerID);
        if((pDamager != NULL) && pDamager->isInfantry()) {
            const UnitBase* pUnit = dynamic_cast<const UnitBase*>(pObject);
            doAttackObject(pUnit, pDamager, false);
        }
	} else if(pObject->isAUnit()) {
        const UnitBase* pUnit = dynamic_cast<const UnitBase*>(pObject);
        const ObjectBase* pDamager = getObject(damagerID);

        if(pUnit->getAttackMode() == GUARD || pUnit->getAttackMode() == AMBUSH) {
            doSetAttackMode(pUnit, HUNT);
            doAttackObject(pUnit, pDamager, false);
        } else if(pUnit->getAttackMode() == AREAGUARD) {
            doAttackObject(pUnit, pDamager, false);
        }
	}
}

void OldAIPlayer::setAttackTimer(int newAttackTimer) {
	if(newAttackTimer >= 0) {
		attackTimer = newAttackTimer;
	}
}

void OldAIPlayer::scrambleUnitsAndDefend(Uint32 intruderID) {
    RobustList<const UnitBase*>::const_iterator iter;
    for(iter = getUnitList().begin(); iter != getUnitList().end(); ++iter) {
        const UnitBase* pUnit = *iter;
        if(pUnit->isRespondable() && (pUnit->getOwner() == getHouse())) {

            if((pUnit->getAttackMode() != HUNT) && !pUnit->hasATarget()) {
                Uint32 itemID = pUnit->getItemID();
                if((itemID != Unit_Harvester) && (pUnit->getItemID() != Unit_MCV) && (pUnit->getItemID() != Unit_Carryall)
                    && (pUnit->getItemID() != Unit_Frigate) && (pUnit->getItemID() != Unit_Saboteur) && (pUnit->getItemID() != Unit_Sandworm)) {
                    const ObjectBase* pIntruder = getObject(intruderID);
                    doAttackObject(pUnit, pIntruder, false);
                }
            }
        }
    }
}

Coord OldAIPlayer::findPlaceLocation(Uint32 itemID) {
    int structureSizeX = getStructureSize(itemID).x;
    int structureSizeY = getStructureSize(itemID).y;

	Coord bestLocation = Coord::Invalid();
	int	count;
	int minX = getMap().getSizeX();
	int maxX = -1;
    int minY = getMap().getSizeY();
    int maxY = -1;

    RobustList<const StructureBase*>::const_iterator iter;
    for(iter = getStructureList().begin(); iter != getStructureList().end(); ++iter) {
		const StructureBase* structure = *iter;
		if (structure->getOwner() == getHouse()) {
			if (structure->getX() < minX)
				minX = structure->getX();
			if (structure->getX() > maxX)
				maxX = structure->getX();
			if (structure->getY() < minY)
				minY = structure->getY();
			if (structure->getY() > maxY)
				maxY = structure->getY();
		}
	}

	minX -= structureSizeX + 1;
	maxX += 2;
	minY -= structureSizeY + 1;
	maxY += 2;
	if (minX < 0) minX = 0;
	if (maxX >= getMap().getSizeX()) maxX = getMap().getSizeX() - structureSizeX;
	if (minY < 0) minY = 0;
	if (maxY >= getMap().getSizeY()) maxY = getMap().getSizeY() - structureSizeY;

    float bestrating = 0.0f;
	count = 0;
	do {
	    int x = getRandomGen().rand(minX, maxX);
        int y = getRandomGen().rand(minY, maxY);
	    Coord pos = Coord(x, y);

		count++;

		if(getMap().okayToPlaceStructure(pos.x, pos.y, structureSizeX, structureSizeY, false, getHouse())) {
            float rating;

		    switch(itemID) {
                case Structure_Refinery: {
                    // place near spice
                    Coord spicePos;
                    if(getMap().findSpice(spicePos, pos)) {
                        rating = 10000000.0f - blockDistance(pos, spicePos);
                    } else {
                        rating = 10000000.0f;
                    }
                } break;

                case Structure_Barracks:
                case Structure_ConstructionYard:
                case Structure_HeavyFactory:
                case Structure_LightFactory:
                case Structure_RepairYard:
                case Structure_StarPort:
                case Structure_WOR: {
                    // place near sand

                    float nearestSand = 10000000.0f;

                    for(int y = 0 ; y < currentGameMap->getSizeY(); y++) {
                        for(int x = 0; x < currentGameMap->getSizeX(); x++) {
                            int type = currentGameMap->getTile(x,y)->getType();

                            if(type != Terrain_Rock || type != Terrain_Slab || type != Terrain_Mountain) {
                                float tmp = blockDistance(pos, Coord(x,y));
                                if(tmp < nearestSand) {
                                    nearestSand = tmp;
                                }
                            }
                        }
                    }

                    rating = 10000000.0f - nearestSand;
                } break;

                case Structure_Wall:
                case Structure_GunTurret:
                case Structure_RocketTurret: {
                    // place towards enemy
                    float nearestEnemy = 10000000.0f;

                    RobustList<const StructureBase*>::const_iterator iter2;
                    for(iter2 = getStructureList().begin(); iter2 != getStructureList().end(); ++iter2) {
                        const StructureBase* pStructure = *iter2;
                        if(pStructure->getOwner()->getTeam() != getHouse()->getTeam()) {

                            float tmp = blockDistance(pos, pStructure->getLocation());
                            if(tmp < nearestEnemy) {
                                nearestEnemy = tmp;
                            }
                        }
                    }

                    rating = 10000000.0f - nearestEnemy;
                } break;

                case Structure_HighTechFactory:
                case Structure_IX:
                case Structure_Palace:
                case Structure_Radar:
                case Structure_Silo:
                case Structure_WindTrap:
                default: {
                    // place at a save place
                    float nearestEnemy = 10000000.0f;

                    RobustList<const StructureBase*>::const_iterator iter2;
                    for(iter2 = getStructureList().begin(); iter2 != getStructureList().end(); ++iter2) {
                        const StructureBase* pStructure = *iter2;
                        if(pStructure->getOwner()->getTeam() != getHouse()->getTeam()) {

                            float tmp = blockDistance(pos, pStructure->getLocation());
                            if(tmp < nearestEnemy) {
                                nearestEnemy = tmp;
                            }
                        }
                    }

                    rating = nearestEnemy;
                } break;
		    }

		    if(rating > bestrating) {
                bestLocation = pos;
                bestrating = rating;
		    }
		}

	} while(count <= 100);

	return bestLocation;
}
