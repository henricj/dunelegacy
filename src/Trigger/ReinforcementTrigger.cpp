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


ReinforcementTrigger::ReinforcementTrigger(HOUSETYPE houseID, ItemID_enum itemID, DropLocation location, bool bRepeat,
                                           Uint32 triggerCycleNumber)
 : Trigger(triggerCycleNumber), dropLocation(location), houseID(houseID), repeatCycle((bRepeat) ? triggerCycleNumber : 0) {
    droppedUnits.push_back(itemID);
}

ReinforcementTrigger::ReinforcementTrigger(InputStream& stream) : Trigger(stream)
{
    droppedUnits = stream.readUint32Vector<ItemID_enum>();
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

void ReinforcementTrigger::trigger(const GameContext& context) {
    auto& [game, map, objectManager] = context;

    auto *dropHouse = game.getHouse(houseID);

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
                    placeCoord = Coord(game.randomGen.rand(0, map.getSizeX() - 1), 0);
                } break;

                case DropLocation::Drop_East: {
                    placeCoord = Coord(map.getSizeX() - 1, game.randomGen.rand(0, map.getSizeY() - 1));
                } break;

                case DropLocation::Drop_South: {
                    placeCoord = Coord(game.randomGen.rand(0, map.getSizeX() - 1), map.getSizeY() - 1);
                } break;

                case DropLocation::Drop_West: {
                    placeCoord = Coord(0, game.randomGen.rand(0, map.getSizeY() - 1));
                } break;

                default: {
                } break;
            }


            if(placeCoord.isInvalid()) {
                break;
            }

            auto it = droppedUnits.begin();

            // try 30 times
            int r = 1;
            while(it != droppedUnits.end() && ++r < 64) {
                auto newCoord = placeCoord;
                if(dropLocation == DropLocation::Drop_North || dropLocation == DropLocation::Drop_South) {
                    newCoord += Coord(game.randomGen.rand(-r,r), 0);
                } else {
                    newCoord += Coord(0, game.randomGen.rand(-r,r));
                }

                auto* const tile = map.tryGetTile(newCoord.x, newCoord.y);

                if(tile
                    && (!tile->hasAGroundObject())
                    && ((*it != Unit_Sandworm) || (tile->isSand()))) {
                    auto *pUnit2Drop = dropHouse->createUnit(*it);
                    ++it;

                    pUnit2Drop->deploy(context, newCoord);

                    if (newCoord.x == 0) {
                        pUnit2Drop->setAngle(ANGLETYPE::RIGHT);
                        pUnit2Drop->setDestination(newCoord + Coord(1,0));
                    } else if (newCoord.x == map.getSizeX()-1) {
                        pUnit2Drop->setAngle(ANGLETYPE::LEFT);
                        pUnit2Drop->setDestination(newCoord + Coord(-1,0));
                    } else if (newCoord.y == 0) {
                        pUnit2Drop->setAngle(ANGLETYPE::DOWN);
                        pUnit2Drop->setDestination(newCoord + Coord(0,1));
                    } else if (newCoord.y == map.getSizeY()-1) {
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
                    int x = game.randomGen.rand(0,map.getSizeX()-1);
                    int y = game.randomGen.rand(0,map.getSizeY()-1);
                    dropCoord = Coord(x, y);
                } break;

                case DropLocation::Drop_Visible: {
                    dropCoord = Coord(map.getSizeX() / 2, map.getSizeY() / 2);
                } break;

                case DropLocation::Drop_Enemybase: {
                    if(auto* pHouse = game.house_find_if([&](auto& house) {
                           return house.getNumStructures() != 0 && house.getTeamID() != 0 &&
                                  house.getTeamID() != dropHouse->getTeamID();
                       })) {
                        dropCoord = pHouse->getCenterOfMainBase();
                    }

                    if(dropCoord.isInvalid()) {
                        // no house with structures found => search for units
                        if(auto* pHouse = game.house_find_if([&](auto& house) {
                               return house.getNumUnits() != 0 && house.getTeamID() != 0 &&
                                      house.getTeamID() != dropHouse->getTeamID();
                           })) {
                            dropCoord = pHouse->getStrongestUnitPosition();
                        }
                    }

                    if(dropCoord.isInvalid()) {
                        // no house with units or structures found => random position
                        int x     = game.randomGen.rand(0, map.getSizeX() - 1);
                        int y     = game.randomGen.rand(0, map.getSizeY() - 1);
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
                            int x = game.randomGen.rand(0,currentGameMap->getSizeX()-1);
                            int y = game.randomGen.rand(0,currentGameMap->getSizeY()-1);
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
            for(auto i = 0; i < 32; i++) {
                const auto r     = game.randomGen.rand(0, 7);
                const auto angle = 2 * FixPt_PI * game.randomGen.randFixPoint();

                dropCoord += Coord(lround(r * FixPoint::sin(angle)), lround(-r * FixPoint::cos(angle)));

                if(auto* tile = map.tryGetTile(dropCoord.x, dropCoord.y); tile && tile->hasAGroundObject()) {
                    // found the an empty drop location => drop here

                    auto* carryall = dropHouse->createUnit<Carryall>();
                    carryall->setOwned(false);

                    for(auto itemID2Drop : droppedUnits) {
                        auto* pUnit2Drop = dropHouse->createUnit(itemID2Drop);
                        pUnit2Drop->setActive(false);
                        carryall->giveCargo(context, pUnit2Drop);
                    }

                    const auto closestPos = map.findClosestEdgePoint(dropCoord, Coord(1, 1));
                    carryall->deploy(context, closestPos);
                    carryall->setDropOfferer(true);

                    if(closestPos.x == 0) carryall->setAngle(ANGLETYPE::RIGHT);
                    else if(closestPos.x == map.getSizeX() - 1)
                        carryall->setAngle(ANGLETYPE::LEFT);
                    else if(closestPos.y == 0)
                        carryall->setAngle(ANGLETYPE::DOWN);
                    else if(closestPos.y == map.getSizeY() - 1)
                        carryall->setAngle(ANGLETYPE::UP);

                    carryall->setDestination(dropCoord);

                    break;
                }
            }

        } break;


        default: {
            sdl2::log_info("ReinforcementTrigger::trigger(): Invalid drop location!");
        } break;
    }

    if(isRepeat()) {
        auto pReinforcementTrigger =  std::make_unique<ReinforcementTrigger>(*this);
        pReinforcementTrigger->cycleNumber += repeatCycle;
        game.getTriggerManager().addTrigger(std::move(pReinforcementTrigger));
    }
}
