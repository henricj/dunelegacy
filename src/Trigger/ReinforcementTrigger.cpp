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


ReinforcementTrigger::ReinforcementTrigger(HOUSETYPE houseID, Uint32 itemID, DropLocation location, bool bRepeat,
                                           Uint32 triggerCycleNumber)
 : Trigger(triggerCycleNumber), dropLocation(location), houseID(houseID), repeatCycle((bRepeat == true) ? triggerCycleNumber : 0) {
    droppedUnits.push_back(itemID);
}

ReinforcementTrigger::ReinforcementTrigger(InputStream& stream) : Trigger(stream)
{
    droppedUnits = stream.readUint32Vector();
    dropLocation = static_cast<DropLocation>(stream.readUint32());
    houseID = static_cast<HOUSETYPE>(stream.readSint32());
    repeatCycle = stream.readUint32();
}

ReinforcementTrigger::~ReinforcementTrigger() = default;

void ReinforcementTrigger::save(OutputStream& stream) const
{
    Trigger::save(stream);

    stream.writeUint32Vector(droppedUnits);
    stream.writeUint32(static_cast<const Uint32>(dropLocation));
    stream.writeSint32(static_cast<const Sint32>(houseID));
    stream.writeUint32(repeatCycle);
}

void ReinforcementTrigger::trigger()
{
    auto dropHouse = currentGame->getHouse(houseID);

    if(dropHouse == nullptr) {
        return;
    }

    switch(dropLocation) {
        case DropLocation::Drop_North:
        case DropLocation::Drop_East:
        case DropLocation::Drop_South:
        case DropLocation::Drop_West: {

            Coord placeCoord = Coord::Invalid();

            switch(dropLocation) {

                case DropLocation::Drop_North: {
                    placeCoord = Coord(currentGame->randomGen.rand(0,currentGameMap->getSizeX()-1), 0);
                } break;

                case DropLocation::Drop_East: {
                    placeCoord = Coord(currentGameMap->getSizeX()-1, currentGame->randomGen.rand(0,currentGameMap->getSizeY()-1));
                } break;

                case DropLocation::Drop_South: {
                    placeCoord = Coord(currentGame->randomGen.rand(0,currentGameMap->getSizeX()-1), currentGameMap->getSizeY()-1);
                } break;

                case DropLocation::Drop_West: {
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
                auto newCoord = placeCoord;
                if(dropLocation == DropLocation::Drop_North || dropLocation == DropLocation::Drop_South) {
                    newCoord += Coord(currentGame->randomGen.rand(-r,r), 0);
                } else {
                    newCoord += Coord(0, currentGame->randomGen.rand(-r,r));
                }

                if(currentGameMap->tileExists(newCoord)
                    && (currentGameMap->getTile(newCoord)->hasAGroundObject() == false)
                    && ((units2Drop.front() != Unit_Sandworm) || (currentGameMap->getTile(newCoord)->isSand()))) {
                    auto pUnit2Drop = dropHouse->createUnit(units2Drop.front());
                    units2Drop.erase(units2Drop.begin());

                    pUnit2Drop->deploy(newCoord);

                    if (newCoord.x == 0) {
                        pUnit2Drop->setAngle(ANGLETYPE::RIGHT);
                        pUnit2Drop->setDestination(newCoord + Coord(1,0));
                    } else if (newCoord.x == currentGameMap->getSizeX()-1) {
                        pUnit2Drop->setAngle(ANGLETYPE::LEFT);
                        pUnit2Drop->setDestination(newCoord + Coord(-1,0));
                    } else if (newCoord.y == 0) {
                        pUnit2Drop->setAngle(ANGLETYPE::DOWN);
                        pUnit2Drop->setDestination(newCoord + Coord(0,1));
                    } else if (newCoord.y == currentGameMap->getSizeY()-1) {
                        pUnit2Drop->setAngle(ANGLETYPE::UP);
                        pUnit2Drop->setDestination(newCoord + Coord(0,-1));
                    }
                }
            }

        } break;

        case DropLocation::Drop_Air:
        case DropLocation::Drop_Visible:
        case DropLocation::Drop_Enemybase:
        case DropLocation::Drop_Homebase: {
            auto dropCoord = Coord::Invalid();

            switch(dropLocation) {
                case DropLocation::Drop_Air: {
                    int x = currentGame->randomGen.rand(0,currentGameMap->getSizeX()-1);
                    int y = currentGame->randomGen.rand(0,currentGameMap->getSizeY()-1);
                    dropCoord = Coord(x, y);
                } break;

                case DropLocation::Drop_Visible: {
                    dropCoord = Coord(currentGameMap->getSizeX() / 2, currentGameMap->getSizeY() / 2);
                } break;

                case DropLocation::Drop_Enemybase: {
                    for(int i = 0; i < static_cast<int>(HOUSETYPE::NUM_HOUSES); i++) {
                        auto pHouse = currentGame->getHouse(static_cast<HOUSETYPE>(i));
                        if(pHouse != nullptr && pHouse->getNumStructures() != 0 && pHouse->getTeamID() != 0 && pHouse->getTeamID() != dropHouse->getTeamID()) {
                            dropCoord = pHouse->getCenterOfMainBase();
                            break;
                        }
                    }

                    if(dropCoord.isInvalid()) {
                        // no house with structures found => search for units
                        for(int i = 0; i < static_cast<int>(HOUSETYPE::NUM_HOUSES); i++) {
                            auto pHouse = currentGame->getHouse(static_cast<HOUSETYPE>(i));
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

                case DropLocation::Drop_Homebase : {
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
            for(auto i=0;i<32;i++) {
                const auto r = currentGame->randomGen.rand(0,7);
                const auto angle = 2*FixPt_PI*currentGame->randomGen.randFixPoint();

                dropCoord += Coord( lround(r*FixPoint::sin(angle)), lround(-r*FixPoint::cos(angle)));

                if(currentGameMap->tileExists(dropCoord) && currentGameMap->getTile(dropCoord)->hasAGroundObject() == false) {
                    // found the an empty drop location => drop here

                    auto carryall = static_cast<Carryall*>(dropHouse->createUnit(Unit_Carryall));
                    carryall->setOwned(false);

                    for(auto itemID2Drop : droppedUnits) {
                        auto pUnit2Drop = dropHouse->createUnit(itemID2Drop);
                        pUnit2Drop->setActive(false);
                        carryall->giveCargo(pUnit2Drop);
                    }

                    const auto closestPos = currentGameMap->findClosestEdgePoint(dropCoord, Coord(1,1));
                    carryall->deploy(closestPos);
                    carryall->setDropOfferer(true);

                    if (closestPos.x == 0)
                        carryall->setAngle(ANGLETYPE::RIGHT);
                    else if (closestPos.x == currentGameMap->getSizeX()-1)
                        carryall->setAngle(ANGLETYPE::LEFT);
                    else if (closestPos.y == 0)
                        carryall->setAngle(ANGLETYPE::DOWN);
                    else if (closestPos.y == currentGameMap->getSizeY()-1)
                        carryall->setAngle(ANGLETYPE::UP);

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
