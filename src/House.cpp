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

#include <House.h>

#include <globals.h>
#include <sand.h>

#include <FileClasses/TextManager.h>

#include <players/PlayerFactory.h>
#include <players/AIPlayer.h>
#include <players/QuantBot.h>
#include <players/HumanPlayer.h>


#include <Game.h>
#include <GameInterface.h>
#include <Map.h>
#include <SoundPlayer.h>

#include <structures/StructureBase.h>
#include <structures/BuilderBase.h>
#include <structures/Refinery.h>
#include <structures/ConstructionYard.h>
#include <units/Carryall.h>
#include <units/Harvester.h>

#include <algorithm>


House::House(int newHouse, int newCredits, Uint8 team, int quota) : choam(this) {
    House::init();

    houseID = ((newHouse >= 0) && (newHouse < NUM_HOUSES)) ? newHouse :  0;
    this->team = team;

	storedCredits = 0;
    startingCredits = newCredits;
    oldCredits = lround(storedCredits+startingCredits);

    this->quota = quota;

    unitBuiltValue = 0;
    structureBuiltValue = 0;
    militaryValue = 0;
    killValue = 0;
    lossValue = 0;
    numBuiltUnits = 0;
    numBuiltStructures = 0;
    destroyedValue = 0;
    numDestroyedUnits = 0;
    numDestroyedStructures = 0;
    harvestedSpice = 0;
    producedPower = 0;
    powerUsageTimer = 0;
}




House::House(InputStream& stream) : choam(this) {
    House::init();

    houseID = stream.readUint8();
	team = stream.readUint8();

	storedCredits = stream.readFixPoint();
	startingCredits = stream.readFixPoint();
    oldCredits = lround(storedCredits+startingCredits);
    quota = stream.readSint32();

    unitBuiltValue = stream.readUint32();
    structureBuiltValue = stream.readUint32();
    militaryValue = stream.readUint32();
    killValue = stream.readUint32();
    lossValue = stream.readUint32();
    numBuiltUnits = stream.readUint32();
    numBuiltStructures = stream.readUint32();
    destroyedValue = stream.readUint32();
    numDestroyedUnits = stream.readUint32();
    numDestroyedStructures = stream.readUint32();
    harvestedSpice = stream.readFixPoint();
    producedPower = stream.readSint32();
    powerUsageTimer = stream.readSint32();

    choam.load(stream);

	Uint32 numPlayers = stream.readUint32();
	for(Uint32 i = 0; i < numPlayers; i++) {
        std::string playerclass = stream.readString();
        const PlayerFactory::PlayerData* pPlayerData = PlayerFactory::getByPlayerClass(playerclass);
        if(pPlayerData == NULL) {
            fprintf(stderr, "Cannot load player \"%s\"", playerclass.c_str());
        } else {
            addPlayer(std::shared_ptr<Player>(pPlayerData->load(stream,this)));
        }
	}
}




void House::init() {
    ai = true;

    numUnits = 0;
	numStructures = 0;
	for(int i=0;i<Num_ItemID;i++) {
        numItem[i] = 0;
        numItemBuilt[i] = 0;
        numItemKills[i] = 0;
        numItemLosses[i] = 0;
	}

	capacity = 0;
	powerRequirement = 0;
}




House::~House() {
}




void House::save(OutputStream& stream) const {
	stream.writeUint8(houseID);
	stream.writeUint8(team);

	stream.writeFixPoint(storedCredits);
	stream.writeFixPoint(startingCredits);
	stream.writeSint32(quota);

    stream.writeUint32(unitBuiltValue);
    stream.writeUint32(structureBuiltValue);
    stream.writeUint32(militaryValue);
    stream.writeUint32(killValue);
    stream.writeUint32(lossValue);
    stream.writeUint32(numBuiltUnits);
    stream.writeUint32(numBuiltStructures);
    stream.writeUint32(destroyedValue);
    stream.writeUint32(numDestroyedUnits);
    stream.writeUint32(numDestroyedStructures);
    stream.writeFixPoint(harvestedSpice);
    stream.writeSint32(producedPower);
    stream.writeSint32(powerUsageTimer);

    choam.save(stream);

	stream.writeUint32(players.size());
    std::list<std::shared_ptr<Player> >::const_iterator iter;
    for(iter = players.begin(); iter != players.end(); ++iter) {
        stream.writeString((*iter)->getPlayerclass());
        (*iter)->save(stream);
    }
}




void House::addPlayer(std::shared_ptr<Player> newPlayer) {
    if(dynamic_cast<HumanPlayer*>(newPlayer.get()) != NULL && players.empty()) {
        ai = false;
    } else {
        ai = true;
    }

    players.push_back(newPlayer);

    Uint8 newPlayerID = (houseID << 4) | players.size();
    newPlayer->playerID = newPlayerID;

    currentGame->registerPlayer(newPlayer.get());
}


void House::setProducedPower(int newPower) {
	producedPower = newPower;
}


void House::addCredits(FixPoint newCredits, bool wasRefined) {
	if(newCredits > 0) {
	    if(wasRefined == true) {
            harvestedSpice += newCredits;
	    }

		storedCredits += newCredits;
		if(this == pLocalHouse)	{
			if(((currentGame->winFlags & WINLOSEFLAGS_QUOTA) != 0) && (quota != 0))	{
				if(storedCredits >= quota) {
					win();
				}
			}
		}
	}
}




void House::returnCredits(FixPoint newCredits) {
	if(newCredits > 0) {
        FixPoint leftCapacity = capacity - storedCredits;
        if(newCredits <= leftCapacity) {
            addCredits(newCredits, false);
        } else {
            addCredits(leftCapacity, false);
            startingCredits += (newCredits - leftCapacity);
        }
	}
}




FixPoint House::takeCredits(FixPoint amount) {
	FixPoint taken = 0;

	if(getCredits() >= 1) {
		if(storedCredits > amount) {
			taken = amount;
			storedCredits -= amount;
		} else {
			taken = storedCredits;
			storedCredits = 0;

			if(startingCredits > (amount - taken)) {
				startingCredits -= (amount - taken);
				taken = amount;
			} else {
				taken += startingCredits;
				startingCredits = 0;
			}
		}
	}

	return taken;	//the amount that was actually withdrawn
}




void House::printStat() const {
	fprintf(stderr,"House %s: (Number of Units: %d, Number of Structures: %d)\n",getHouseNameByNumber( (HOUSETYPE) getHouseID()).c_str(),numUnits,numStructures);
	fprintf(stderr,"Barracks: %d\t\tWORs: %d\n", numItem[Structure_Barracks],numItem[Structure_WOR]);
	fprintf(stderr,"Light Factories: %d\tHeavy Factories: %d\n",numItem[Structure_LightFactory],numItem[Structure_HeavyFactory]);
	fprintf(stderr,"IXs: %d\t\t\tPalaces: %d\n",numItem[Structure_IX],numItem[Structure_Palace]);
	fprintf(stderr,"Repair Yards: %d\t\tHigh-Tech Factories: %d\n",numItem[Structure_RepairYard],numItem[Structure_HighTechFactory]);
	fprintf(stderr,"Refineries: %d\t\tStarports: %d\n",numItem[Structure_Refinery],numItem[Structure_StarPort]);
	fprintf(stderr,"Walls: %d\t\tRocket Turrets: %d\n",numItem[Structure_Wall],numItem[Structure_RocketTurret]);
	fprintf(stderr,"Gun Turrets: %d\t\tConstruction Yards: %d\n",numItem[Structure_GunTurret],numItem[Structure_ConstructionYard]);
	fprintf(stderr,"Windtraps: %d\t\tRadars: %d\n",numItem[Structure_WindTrap],numItem[Structure_Radar]);
	fprintf(stderr,"Silos: %d\n",numItem[Structure_Silo]);
	fprintf(stderr,"Carryalls: %d\t\tFrigates: %d\n",numItem[Unit_Carryall],numItem[Unit_Frigate]);
	fprintf(stderr,"Devastators: %d\t\tDeviators: %d\n",numItem[Unit_Devastator],numItem[Unit_Deviator]);
	fprintf(stderr,"Soldiers: %d\t\tTrooper: %d\n",numItem[Unit_Soldier],numItem[Unit_Trooper]);
	fprintf(stderr,"Saboteur: %d\t\tSandworms: %d\n",numItem[Unit_Saboteur],numItem[Unit_Sandworm]);
	fprintf(stderr,"Quads: %d\t\tTrikes: %d\n",numItem[Unit_Quad],numItem[Unit_Trike]);
	fprintf(stderr,"Raiders: %d\t\tTanks: %d\n",numItem[Unit_RaiderTrike],numItem[Unit_Tank]);
	fprintf(stderr,"Siege Tanks : %d\t\tSonic Tanks: %d\n",numItem[Unit_SiegeTank],numItem[Unit_SonicTank]);
	fprintf(stderr,"Harvesters: %d\t\tMCVs: %d\n",numItem[Unit_Harvester],numItem[Unit_MCV]);
	fprintf(stderr,"Ornithopters: %d\t\tRocket Launchers: %d\n",numItem[Unit_Ornithopter],numItem[Unit_Launcher]);
}




void House::updateBuildLists() {
    RobustList<StructureBase*>::const_iterator iter;
    for(iter = structureList.begin(); iter != structureList.end(); ++iter) {
		StructureBase* tempStructure = *iter;
        if(tempStructure->isABuilder() && (tempStructure->getOwner() == this)) {
			((BuilderBase*) tempStructure)->updateBuildList();
        }
    }
}




void House::update() {
	if (oldCredits != getCredits()) {
		if((this == pLocalHouse) && (getCredits() > 0)) {
			soundPlayer->playSound(CreditsTick);
		}
		oldCredits = getCredits();
	}

	if(storedCredits > capacity) {
		storedCredits--;
		if(storedCredits < 0) {
		 storedCredits = 0;
		}

		if(this == pLocalHouse) {
			currentGame->addToNewsTicker(_("@DUNE.ENG|145#As insufficient spice storage is available, spice is lost."));
		}
	}

	powerUsageTimer--;
	if(powerUsageTimer <= 0) {
	    powerUsageTimer = MILLI2CYCLES(15*1000);
        takeCredits(FixPoint(getPowerRequirement()) / 32);
	}

	choam.update();

    std::list<std::shared_ptr<Player> >::iterator iter;
    for(iter = players.begin(); iter != players.end(); ++iter) {
        (*iter)->update();
    }
}




void House::incrementUnits(int itemID) {
    numUnits++;
    numItem[itemID]++;

    if(itemID != Unit_Saboteur
       && itemID != Unit_Frigate
       && itemID != Unit_Carryall
       && itemID != Unit_MCV
       && itemID != Unit_Harvester
       && itemID != Unit_Sandworm) {

            militaryValue += currentGame->objectData.data[itemID][houseID].price;
    }

}



void House::decrementUnits(int itemID) {
	numUnits--;
	numItemLosses[itemID]++;

	if(itemID == Unit_Harvester) {
        decrementHarvesters();
	} else {
        numItem[itemID]--;
	}

    std::list<std::shared_ptr<Player> >::iterator iter;
    for(iter = players.begin(); iter != players.end(); ++iter) {
        (*iter)->onDecrementUnits(itemID);
    }



    if(itemID != Unit_Saboteur
       && itemID != Unit_Frigate
       && itemID != Unit_Carryall
       && itemID != Unit_MCV
       && itemID != Unit_Harvester
       && itemID != Unit_Sandworm) {

            lossValue += currentGame->objectData.data[itemID][houseID].price;
    }

	if (!isAlive())
		lose();


}




void House::incrementStructures(int itemID) {
	numStructures++;
	numItem[itemID]++;

    // change power requirements
	int currentItemPower = currentGame->objectData.data[itemID][houseID].power;
	if(currentItemPower >= 0) {
        powerRequirement += currentItemPower;
	}

	// change spice capacity
	capacity += currentGame->objectData.data[itemID][houseID].capacity;

    if(currentGame->gameState != LOADING) {
        // do not check selection lists if we are loading
        updateBuildLists();
    }

    std::list<std::shared_ptr<Player> >::iterator iter;
    for(iter = players.begin(); iter != players.end(); ++iter) {
        (*iter)->onIncrementStructures(itemID);
    }
}




void House::decrementStructures(int itemID, const Coord& location) {
	numStructures--;
    numItem[itemID]--;
    numItemLosses[itemID]++;

	// change power requirements
	int currentItemPower = currentGame->objectData.data[itemID][houseID].power;
	if(currentItemPower >= 0) {
        powerRequirement -= currentItemPower;
	}

    // change spice capacity
	capacity -= currentGame->objectData.data[itemID][houseID].capacity;

    if(currentGame->gameState != LOADING) {
        // do not check selection lists if we are loading
        updateBuildLists();
    }

	if (!isAlive())
		lose();

    std::list<std::shared_ptr<Player> >::iterator iter;
    for(iter = players.begin(); iter != players.end(); ++iter) {
        (*iter)->onDecrementStructures(itemID, location);
    }
}




void House::noteDamageLocation(ObjectBase* pObject, int damage, Uint32 damagerID) {
    std::list<std::shared_ptr<Player> >::iterator iter;
    for(iter = players.begin(); iter != players.end(); ++iter) {
        (*iter)->onDamage(pObject, damage, damagerID);
    }
}


/**
    This method informs this house that a new unit or structure was built
    \param itemID   the ID of the enemy unit or structure
*/
void House::informWasBuilt(Uint32 itemID) {
    if(isStructure(itemID)) {
        structureBuiltValue += currentGame->objectData.data[itemID][houseID].price; // Removed divide by 100
        numBuiltStructures++;
    } else {
        unitBuiltValue += currentGame->objectData.data[itemID][houseID].price; // Removed divide by 100
        numBuiltUnits++;
    }

    numItemBuilt[itemID]++;
}



/**
    This method informs this house that one of its units has killed an enemy unit or structure
    \param itemID   the ID of the enemy unit or structure
*/
void House::informHasKilled(Uint32 itemID) {
    destroyedValue += std::max(currentGame->objectData.data[itemID][houseID].price/100, 1);
    if(isStructure(itemID)) {
        numDestroyedStructures++;
    } else {
        numDestroyedUnits++;

        if(itemID != Unit_Saboteur
           && itemID != Unit_Frigate
           && itemID != Unit_Carryall
           && itemID != Unit_MCV
           && itemID != Unit_Harvester
           && itemID != Unit_Sandworm) {

                killValue += currentGame->objectData.data[itemID][houseID].price;

        }

    }

    numItemKills[itemID]++;


    std::list<std::shared_ptr<Player> >::iterator iter;
    for(iter = players.begin(); iter != players.end(); ++iter) {
        (*iter)->onIncrementUnitKills(itemID);
    }
}




void House::win() {
	if(getTeam() == pLocalHouse->getTeam()) {
		currentGame->setGameWon();
	} else {
		currentGame->setGameLost();
	}
}




void House::lose(bool bSilent) {
    if(!bSilent) {
        currentGame->addToNewsTicker(strprintf(_("House '%s' has been defeated."), getHouseNameByNumber( (HOUSETYPE) getHouseID()).c_str()));
    }

	if((getTeam() == pLocalHouse->getTeam()) && ((currentGame->winFlags & WINLOSEFLAGS_HUMAN_HAS_BUILDINGS) != 0)) {

        bool finished = true;

		for(int i=0; i < NUM_HOUSES; i++) {
            House* pHouse = currentGame->getHouse(i);
			if(pHouse != NULL && pHouse->isAlive() && pHouse->getTeam() == pLocalHouse->getTeam()) {
				finished = false;
            }
		}

		if(finished) {
            // pLocalHouse is destroyed and this is a game finish condition
            if((currentGame->loseFlags & WINLOSEFLAGS_HUMAN_HAS_BUILDINGS) != 0) {
                // house has won
                currentGame->setGameWon();
            } else {
                // house has lost
                currentGame->setGameLost();
            }
	    }

	} else if((currentGame->winFlags & WINLOSEFLAGS_AI_NO_BUILDINGS) != 0) {
		//if the only players left are on the thisPlayers team, pLocalHouse has won
		bool finished = true;

		for(int i=0; i < NUM_HOUSES; i++) {
            House* pHouse = currentGame->getHouse(i);
			if(pHouse != NULL && pHouse->isAlive() && pHouse->getTeam() != 0 && pHouse->getTeam() != pLocalHouse->getTeam()) {
				finished = false;
            }
		}

		if(finished) {
		    // all AI players are destroyed and this is a game finish condition
            if((currentGame->loseFlags & WINLOSEFLAGS_AI_NO_BUILDINGS) != 0) {
                // house has won
                currentGame->setGameWon();
            } else {
                // house has lost
                currentGame->setGameLost();
            }
		}
	}
}




void House::freeHarvester(int xPos, int yPos) {
	if(currentGameMap->tileExists(xPos, yPos)
		&& currentGameMap->getTile(xPos, yPos)->hasAGroundObject()
		&& (currentGameMap->getTile(xPos, yPos)->getGroundObject()->getItemID() == Structure_Refinery))
	{
		Refinery* refinery = (Refinery*)currentGameMap->getTile(xPos, yPos)->getGroundObject();
		Coord closestPos = currentGameMap->findClosestEdgePoint(refinery->getLocation() + Coord(2,0), Coord(1,1));

		Carryall* carryall = (Carryall*)createUnit(Unit_Carryall);
		Harvester* harvester = (Harvester*)createUnit(Unit_Harvester);
		harvester->setAmountOfSpice(5);
		carryall->setOwned(false);
		carryall->giveCargo(harvester);
		carryall->deploy(closestPos);
		carryall->setDropOfferer(true);

		if (closestPos.x == 0)
			carryall->setAngle(RIGHT);
		else if (closestPos.x == currentGameMap->getSizeX()-1)
			carryall->setAngle(LEFT);
		else if (closestPos.y == 0)
			carryall->setAngle(DOWN);
		else if (closestPos.y == currentGameMap->getSizeY()-1)
			carryall->setAngle(UP);

		harvester->setTarget(refinery);
		harvester->setActive(false);
		carryall->setTarget(refinery);
	}
}




StructureBase* House::placeStructure(Uint32 builderID, int itemID, int xPos, int yPos, bool bForcePlacing) {
	if(!currentGameMap->tileExists(xPos,yPos)) {
		return NULL;
	}

	BuilderBase* pBuilder = (builderID == NONE) ? NULL : dynamic_cast<BuilderBase*>(currentGame->getObjectManager().getObject(builderID));

	if(currentGame->getGameInitSettings().getGameOptions().onlyOnePalace && pBuilder != NULL && itemID == Structure_Palace && getNumItems(Structure_Palace) > 0) {
        if(this == pLocalHouse && pBuilder->isSelected()) {
            currentGame->currentCursorMode = Game::CursorMode_Normal;
        }
        return NULL;
	}

	StructureBase* tempStructure = NULL;

	switch (itemID) {
		case (Structure_Slab1): {
			// Slabs are no normal buildings
			currentGameMap->getTile(xPos, yPos)->setType(Terrain_Slab);
			currentGameMap->getTile(xPos, yPos)->setOwner(getHouseID());
			currentGameMap->viewMap(getTeam(), xPos, yPos, currentGame->objectData.data[Structure_Slab1][houseID].viewrange);
	//		currentGameMap->getTile(xPos, yPos)->clearTerrain();

			if(pBuilder != NULL) {
				pBuilder->unSetWaitingToPlace();

				if(this == pLocalHouse && pBuilder->isSelected()) {
					currentGame->currentCursorMode = Game::CursorMode_Normal;
				}
			}

		} break;

		case (Structure_Slab4): {
			// Slabs are no normal buildings
			int i,j;
			for(i = xPos; i < xPos + 2; i++) {
				for(j = yPos; j < yPos + 2; j++) {
					if (currentGameMap->tileExists(i, j)) {
						Tile* pTile = currentGameMap->getTile(i, j);

						if (!pTile->hasAGroundObject() && pTile->isRock() && !pTile->isMountain()) {
							pTile->setType(Terrain_Slab);
							pTile->setOwner(getHouseID());
							currentGameMap->viewMap(getTeam(), i, j, currentGame->objectData.data[Structure_Slab4][houseID].viewrange);
							//pTile->clearTerrain();
						}
					}
				}
			}

			if(pBuilder != NULL) {
				pBuilder->unSetWaitingToPlace();

				if(this == pLocalHouse && pBuilder->isSelected()) {
					currentGame->currentCursorMode = Game::CursorMode_Normal;
				}
			}

		} break;

		default: {
			tempStructure = (StructureBase*) ObjectBase::createObject(itemID,this);
			if(tempStructure == NULL) {
				fprintf(stderr,"House::placeStructure(): Cannot create Object with itemID %d\n",itemID);
				fflush(stderr);
				exit(EXIT_FAILURE);
			}

            if(bForcePlacing == false) {
                // check if there is already something on this tile
                for(int i=0;i<tempStructure->getStructureSizeX();i++) {
                    for(int j=0;j<tempStructure->getStructureSizeY();j++) {
                        if((currentGameMap->tileExists(xPos+i, yPos+j) == false) || (currentGameMap->getTile(xPos+i, yPos+j)->hasAGroundObject() == true)) {
                            delete tempStructure;
                            return NULL;
                        }
                    }
                }
            }

			for(int i=0;i<tempStructure->getStructureSizeX();i++) {
				for(int j=0;j<tempStructure->getStructureSizeY();j++) {
					if(currentGameMap->tileExists(xPos+i, yPos+j)) {
						currentGameMap->getTile(xPos+i, yPos+j)->clearTerrain();
					}
				}
			}

			tempStructure->setLocation(xPos, yPos);

			if ((builderID != NONE) && (itemID != Structure_Wall)) {
				tempStructure->setJustPlaced();
			}

			// at the beginning of the game the first refinery gets a harvester for free (brought by a carryall)
			if((itemID == Structure_Refinery) && ( ((currentGame->gameState == START) && (numItem[Unit_Harvester] <= 0)) || (builderID != NONE)) ) {
				freeHarvester(xPos, yPos);
			}

			// if this structure was built by a construction yard this construction yard must be informed
			if(pBuilder != NULL) {
                pBuilder->unSetWaitingToPlace();

                if(itemID == Structure_Palace) {
                    // cancel all other palaces
                    for(RobustList<StructureBase*>::iterator iter = structureList.begin(); iter != structureList.end(); ++iter) {
                        if((*iter)->getOwner() == this && (*iter)->getItemID() == Structure_ConstructionYard) {
                            ConstructionYard* pConstructionYard = (ConstructionYard*) *iter;

                            if(pBuilder != pConstructionYard) {
                                pConstructionYard->doCancelItem(Structure_Palace, false);
                            }
                        }
                    }
                }

                if (this == pLocalHouse && pBuilder->isSelected()) {
                    currentGame->currentCursorMode = Game::CursorMode_Normal;
                }

                // only if we were constructed by construction yard
                // => inform house of the building
                pBuilder->getOwner()->informWasBuilt(itemID);
			}

			if(tempStructure->isABuilder()) {
                ((BuilderBase*) tempStructure)->updateBuildList();
			}


		} break;
	}

	return tempStructure;
}




UnitBase* House::createUnit(int itemID) {
	UnitBase* newUnit = NULL;

	newUnit = (UnitBase*) ObjectBase::createObject(itemID,this);

	if(newUnit == NULL) {
		fprintf(stderr,"House::createUnit(): Cannot create Object with itemID %d\n",itemID);
		fflush(stderr);
		exit(EXIT_FAILURE);
	}

	return newUnit;
}




UnitBase* House::placeUnit(int itemID, int xPos, int yPos) {
	UnitBase* newUnit = NULL;
	if(currentGameMap->tileExists(xPos, yPos) == true) {
	    Tile* pTile = currentGameMap->getTile(xPos,yPos);

	    if(itemID == Unit_Saboteur || itemID == Unit_Soldier || itemID == Unit_Trooper) {
            if((pTile->hasANonInfantryGroundObject() == true) || (pTile->infantryNotFull() == false)) {
                // infantry units can not placed on non-infantry units or structures (or the tile is already full of infantry units)
                return NULL;
            }
	    } else {
	        if(pTile->hasAGroundObject() == true) {
                // non-infantry units can not placed on a tile where already some other unit or structure is placed on
                return NULL;
	        }
	    }

        newUnit = createUnit(itemID);
	}

	if (newUnit) {
		Coord pos = Coord(xPos, yPos);
		if (newUnit->canPass(xPos, yPos)) {
			newUnit->deploy(pos);
		} else {
			newUnit->setVisible(VIS_ALL, false);
			newUnit->destroy();
			newUnit = NULL;
		}
	}

	return newUnit;
}


/**
    This method returns the center of the base of this house.
    \return the coordinate of the center in tile coordinates
*/
Coord House::getCenterOfMainBase() const {
    Coord center;
    int numStructures = 0;

    RobustList<StructureBase*>::const_iterator iter;
    for(iter = structureList.begin(); iter != structureList.end(); ++iter) {
        StructureBase* tempStructure = *iter;

        if(tempStructure->getOwner() == this) {
            center += tempStructure->getLocation();
            numStructures++;
        }
    }

    center /= numStructures;

    return center;
}


/**
    This method returns the position of the strongest unit
    \return the coordinate of the strongest unit
*/
Coord House::getStrongestUnitPosition() const {
    Coord position = Coord::Invalid();
    Sint32 highestCost = 0;

    RobustList<UnitBase*>::const_iterator iter;
    for(iter = unitList.begin(); iter != unitList.end(); ++iter) {
        UnitBase* tempUnit = *iter;

        if(tempUnit->getOwner() == this) {
            Sint32 currentCost = currentGame->objectData.data[tempUnit->getItemID()][houseID].price;

            if(currentCost > highestCost) {
                position = tempUnit->getLocation();
            }
        }
    }

    return position;
}




void House::decrementHarvesters() {
    numItem[Unit_Harvester]--;

    if(numItem[Unit_Harvester] <= 0) {
        numItem[Unit_Harvester] = 0;

        if(numItem[Structure_Refinery]) {
            Coord	closestPos;
            Coord	pos = Coord(0,0);
            FixPoint	closestDistance = FixPt_MAX;
            StructureBase *closestRefinery = NULL;

            RobustList<StructureBase*>::const_iterator iter;
            for(iter = structureList.begin(); iter != structureList.end(); ++iter) {
                StructureBase* tempStructure = *iter;

                if((tempStructure->getItemID() == Structure_Refinery) && (tempStructure->getOwner() == this) && (tempStructure->getHealth() > 0)) {
                    pos = tempStructure->getLocation();

                    Coord closestPoint = tempStructure->getClosestPoint(pos);
                    FixPoint refineryDistance = blockDistance(pos, closestPoint);
                    if(!closestRefinery || (refineryDistance < closestDistance)) {
                            closestDistance = refineryDistance;
                            closestRefinery = tempStructure;
                            closestPos = pos;
                    }
                }
            }

            if(closestRefinery && (currentGame->gameState == BEGUN)) {
                freeHarvester(closestRefinery->getLocation().x, closestRefinery->getLocation().y);
            }
        }
    }
}
