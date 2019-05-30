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

#include <Trigger/ReinforcementTrigger.h>

#include <globals.h>
#include <Game.h>
#include <Map.h>
#include <House.h>

#include <DataTypes.h>

#include <fixmath/FixPoint.h>

#include <units/UnitBase.h>
#include <units/Carryall.h>


ReinforcementTrigger::ReinforcementTrigger(int houseID, Uint32 itemID, DropLocation location, bool bRepeat, Uint32 triggerCycleNumber)
 : Trigger(triggerCycleNumber), dropLocation(location), houseID(houseID), repeatCycle((bRepeat == true) ? triggerCycleNumber : 0) {
    droppedUnits.push_back(itemID);
}

ReinforcementTrigger::ReinforcementTrigger(InputStream& stream) : Trigger(stream)
{
    droppedUnits = stream.readUint32Vector();
    dropLocation = (DropLocation) stream.readUint32();
    houseID = stream.readSint32();
    repeatCycle = stream.readUint32();
}

ReinforcementTrigger::~ReinforcementTrigger() = default;

void ReinforcementTrigger::save(OutputStream& stream) const
{
    Trigger::save(stream);

    stream.writeUint32Vector(droppedUnits);
    stream.writeUint32(dropLocation);
    stream.writeSint32(houseID);
    stream.writeUint32(repeatCycle);
}

void ReinforcementTrigger::trigger()
{
    House* dropHouse = currentGame->getHouse(houseID);

    if(dropHouse == nullptr) {
        return;
    }

    switch(dropLocation) {
        case Drop_North:
        case Drop_East:
        case Drop_South:
        case Drop_West: {

            Coord placeCoord = Coord::Invalid();

            switch(dropLocation) {

                case Drop_North: {
                    placeCoord = Coord(currentGame->randomGen.rand(0,currentGameMap->getSizeX()-1), 0);
                } break;

                case Drop_East: {
                    placeCoord = Coord(currentGameMap->getSizeX()-1, currentGame->randomGen.rand(0,currentGameMap->getSizeY()-1));
                } break;

                case Drop_South: {
                    placeCoord = Coord(currentGame->randomGen.rand(0,currentGameMap->getSizeX()-1), currentGameMap->getSizeY()-1);
                } break;

                case Drop_West: {
                    placeCoord = Coord(0, currentGame->randomGen.rand(0,currentGameMap->getSizeY()-1));
                } break;

                default: {
                } break;
            }


            if(placeCoord.isInvalid()) {
                break;
            }

            std::vector<Uint32> units2Drop = droppedUnits;

            // try 30 times
            int r = 1;
            while(units2Drop.empty() == false && ++r < 64) {

                Coord newCoord = placeCoord;
                if(dropLocation == Drop_North || dropLocation == Drop_South) {
                    newCoord += Coord(currentGame->randomGen.rand(-r,r), 0);
                } else {
                    newCoord += Coord(0, currentGame->randomGen.rand(-r,r));
                }

                if(currentGameMap->tileExists(newCoord)
                    && (currentGameMap->getTile(newCoord)->hasAGroundObject() == false)
                    && ((units2Drop.front() != Unit_Sandworm) || (currentGameMap->getTile(newCoord)->isSand()))) {
                    UnitBase* pUnit2Drop = dropHouse->createUnit(units2Drop.front());
                    units2Drop.erase(units2Drop.begin());

                    pUnit2Drop->deploy(newCoord);

                    if (newCoord.x == 0) {
                        pUnit2Drop->setAngle(RIGHT);
                        pUnit2Drop->setDestination(newCoord + Coord(1,0));
                    } else if (newCoord.x == currentGameMap->getSizeX()-1) {
                        pUnit2Drop->setAngle(LEFT);
                        pUnit2Drop->setDestination(newCoord + Coord(-1,0));
                    } else if (newCoord.y == 0) {
                        pUnit2Drop->setAngle(DOWN);
                        pUnit2Drop->setDestination(newCoord + Coord(0,1));
                    } else if (newCoord.y == currentGameMap->getSizeY()-1) {
                        pUnit2Drop->setAngle(UP);
                        pUnit2Drop->setDestination(newCoord + Coord(0,-1));
                    }
                }
            }

        } break;

        case Drop_Air:
        case Drop_Visible:
        case Drop_Enemybase:
        case Drop_Homebase: {
            Coord dropCoord = Coord::Invalid();

            switch(dropLocation) {
                case Drop_Air: {
                    int x = currentGame->randomGen.rand(0,currentGameMap->getSizeX()-1);
                    int y = currentGame->randomGen.rand(0,currentGameMap->getSizeY()-1);
                    dropCoord = Coord(x, y);
                } break;

                case Drop_Visible: {
                    dropCoord = Coord(currentGameMap->getSizeX() / 2, currentGameMap->getSizeY() / 2);
                } break;

                case Drop_Enemybase: {
                    for(int i=0;i<NUM_HOUSES;i++) {
                        House* pHouse = currentGame->getHouse(i);
                        if(pHouse != nullptr && pHouse->getNumStructures() != 0 && pHouse->getTeamID() != 0 && pHouse->getTeamID() != dropHouse->getTeamID()) {
                            dropCoord = pHouse->getCenterOfMainBase();
                            break;
                        }
                    }

                    if(dropCoord.isInvalid()) {
                        // no house with structures found => search for units
                        for(int i=0;i<NUM_HOUSES;i++) {
                            House* pHouse = currentGame->getHouse(i);
                            if(pHouse != nullptr && pHouse->getNumUnits() != 0 && pHouse->getTeamID() != 0 && pHouse->getTeamID() != dropHouse->getTeamID()) {
                                dropCoord = pHouse->getStrongestUnitPosition();
                                break;
                            }
                        }
                    }

                    if(dropCoord.isInvalid()) {
                        // no house with units or structures found => random position
                        int x = currentGame->randomGen.rand(0,currentGameMap->getSizeX()-1);
                        int y = currentGame->randomGen.rand(0,currentGameMap->getSizeY()-1);
                        dropCoord = Coord(x, y);
                    }

                } break;

                case Drop_Homebase: {
                    if(dropHouse->getNumStructures() != 0) {
                        dropCoord = dropHouse->getCenterOfMainBase();
                    } else {
                        // house has no structures => find unit

                        if(dropHouse->getNumUnits() != 0) {
                            dropCoord = dropHouse->getStrongestUnitPosition();
                        } else {
                            // house has no units => random position
                            int x = currentGame->randomGen.rand(0,currentGameMap->getSizeX()-1);
                            int y = currentGame->randomGen.rand(0,currentGameMap->getSizeY()-1);
                            dropCoord = Coord(x, y);
                        }
                    }
                } break;

                default: {
                } break;
            }

            if(dropCoord.isInvalid()) {
                break;
            }

            // try 32 times
            for(int i=0;i<32;i++) {
                int r = currentGame->randomGen.rand(0,7);
                FixPoint angle = 2*FixPt_PI*currentGame->randomGen.randFixPoint();

                dropCoord += Coord( lround(r*FixPoint::sin(angle)), lround(-r*FixPoint::cos(angle)));

                if(currentGameMap->tileExists(dropCoord) && currentGameMap->getTile(dropCoord)->hasAGroundObject() == false) {
                    // found the an empty drop location => drop here

                    Carryall* carryall = static_cast<Carryall*>(dropHouse->createUnit(Unit_Carryall));
                    carryall->setOwned(false);

                    for(Uint32 itemID2Drop : droppedUnits) {
                        UnitBase* pUnit2Drop = dropHouse->createUnit(itemID2Drop);
                        pUnit2Drop->setActive(false);
                        carryall->giveCargo(pUnit2Drop);
                    }

                    Coord closestPos = currentGameMap->findClosestEdgePoint(dropCoord, Coord(1,1));
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

                    carryall->setDestination(dropCoord);

                    break;
                }
            }

        } break;


        default: {
            SDL_Log("ReinforcementTrigger::trigger(): Invalid drop location!");
        } break;
    }

    if(isRepeat()) {
        auto pReinforcementTrigger =  std::make_unique<ReinforcementTrigger>(*this);
        pReinforcementTrigger->cycleNumber += repeatCycle;
        currentGame->getTriggerManager().addTrigger(std::move(pReinforcementTrigger));
    }
}
