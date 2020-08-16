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

#include "engine_mmath.h"

#include <units/GroundUnit.h>

#include <Game.h>
#include <House.h>
#include <Map.h>

#include <players/HumanPlayer.h>

#include <structures/RepairYard.h>
#include <units/Carryall.h>

namespace Dune::Engine {

GroundUnit::GroundUnit(const GroundUnitConstants& constants, uint32_t objectID, const ObjectInitializer& initializer)
    : UnitBase(constants, objectID, initializer) {

    awaitingPickup = false;
}

GroundUnit::GroundUnit(const GroundUnitConstants& constants, uint32_t objectID,
                       const ObjectStreamInitializer& initializer)
    : UnitBase(constants, objectID, initializer) {

    auto& stream = initializer.stream();

    awaitingPickup = stream.readBool();
    bookedCarrier.load(stream);
}

GroundUnit::~GroundUnit() = default;

void GroundUnit::save(const Game& game, OutputStream& stream) const {
    UnitBase::save(game, stream);

    stream.writeBool(awaitingPickup);
    bookedCarrier.save(stream);
}

void GroundUnit::assignToMap(const GameContext& context, const Coord& pos) {
    auto* const tile = context.map.tryGetTile(pos.x, pos.y);

    if(tile) {
        tile->assignNonInfantryGroundObject(getObjectID());
        context.map.viewMap(owner->getHouseID(), pos, getViewRange(context.game));
    }
}

void GroundUnit::checkPos(const GameContext& context) {
    auto* pTile = context.map.getTile(location);
    if(!moving && !justStoppedMoving && !isInfantry()) {
        pTile->setTrack(drawnAngle, context.game.getGameCycleCount());
    }

    if(justStoppedMoving) {
        realX = location.x * TILESIZE + TILESIZE / 2;
        realY = location.y * TILESIZE + TILESIZE / 2;
        // findTargetTimer = 0;  //allow a scan for new targets now

        if(pTile->isSpiceBloom()) {
            setHealth(context.game, 0);
            setVisible(VIS_ALL, false);
            pTile->triggerSpiceBloom(context, getOwner());
        } else if(pTile->isSpecialBloom()) {
            pTile->triggerSpecialBloom(context, getOwner());
        }
    }

    /*
        Go to repair yard if low on health
    */
    if(active && (getHealth() < getMaxHealth(context.game) / 2) && !goingToRepairYard && owner->hasRepairYard() &&
       !pickedUp && owner->hasCarryalls() &&
       owner->getHouseID() == originalHouseID // stop deviated units from being repaired
       && !isInfantry() && !forced) {         // Stefan - Allow units with targets to be picked up for repairs

        doRepair(context);
    }

    if(goingToRepairYard) {
        auto* const pTarget = target.getObjPointer(context.objectManager);

        if(pTarget == nullptr) {
            goingToRepairYard = false;
            awaitingPickup    = false;
            bookedCarrier.reset();

            clearPath();
        } else {
            auto* const pObject = pTile->getGroundObject(context.objectManager);

            if(justStoppedMoving && (pObject != nullptr) && (pObject->getObjectID() == target.getObjectID())) {
                if(auto* const pRepairYard = dune_cast<RepairYard>(pTarget)) {
                    if(pRepairYard->isFree()) {
                        setGettingRepaired(context);
                    } else {
                        // the repair yard is already in use by some other unit => move out
                        const auto newDestination = context.map.findDeploySpot(
                            this, pRepairYard->getLocation(), getLocation(), pRepairYard->getStructureSize());
                        doMove2Pos(context, newDestination, true);
                    }
                }
            }
        }
    }

    // If we are awaiting a pickup try book a carryall if we have one
    if(!pickedUp && attackMode == CARRYALLREQUESTED && !bookedCarrier) {
        if(getOwner()->hasCarryalls() && (target || (destination != location))) {
            requestCarryall(context);
        } else {
            if(getItemID() == Unit_Harvester) {
                doSetAttackMode(context, HARVEST);
            } else {
                doSetAttackMode(context, GUARD);
            }
        }
    }
}

/**
    Request a Carryall to drop at target location
**/

void GroundUnit::doRequestCarryallDrop(const GameContext& context, int xPos, int yPos) {
    if(getOwner()->hasCarryalls() && !awaitingPickup && context.map.tileExists(xPos, yPos)) {
        doMove2Pos(context, xPos, yPos, true);
        requestCarryall(context);
    }
}

bool GroundUnit::requestCarryall(const GameContext& context) {
    if(getOwner()->hasCarryalls() && !awaitingPickup) {

        // This allows a unit to keep requesting a carryall even if one isn't available right now
        doSetAttackMode(context, CARRYALLREQUESTED);

        for(auto* pUnit : context.game.unitList) {
            if(pUnit->getOwner() != owner) continue;

            auto* carryall = dune_cast<Carryall>(pUnit);
            if(!carryall) continue;

            if(!carryall->isBooked()) {
                carryall->setTarget(context.objectManager, this);
                carryall->clearPath();
                bookCarrier(carryall);

                // setDestination(&location);    //stop moving, and wait for carryall to arrive

                return true;
            }
        }
    }

    return false;
}

void GroundUnit::setPickedUp(const GameContext& context, UnitBase* newCarrier) {
    UnitBase::setPickedUp(context, newCarrier);
    awaitingPickup = false;
    bookedCarrier.reset();

    clearPath(); // Stefan: I don't think this is right
                 // but there is definitely something to it
                 // <try removing this to keep tanks moving even when a carryall is coming>
}

void GroundUnit::bookCarrier(UnitBase* newCarrier) {
    if(newCarrier == nullptr) {
        bookedCarrier.reset();
        awaitingPickup = false;
    } else {
        bookedCarrier.pointTo(newCarrier->getObjectID());
        awaitingPickup = true;
    }
}

const UnitBase* GroundUnit::getCarrier(const ObjectManager& objectManager) const {
    return bookedCarrier.getUnitPointer(objectManager);
}

FixPoint GroundUnit::getTerrainDifficulty(TERRAINTYPE terrainType) const {
    switch(terrainType) {
        case Terrain_Slab: return 1.0_fix;
        case Terrain_Sand: return 1.375_fix;
        case Terrain_Rock: return 1.5625_fix;
        case Terrain_Dunes: return 1.375_fix;
        case Terrain_Mountain: return 1.0_fix;
        case Terrain_Spice: return 1.375_fix;
        case Terrain_ThickSpice: return 1.375_fix;
        case Terrain_SpiceBloom: return 1.375_fix;
        case Terrain_SpecialBloom: return 1.375_fix;
        default: return 1.0_fix;
    }
}

void GroundUnit::move(const GameContext& context) {
    if(!moving && !justStoppedMoving && (((context.game.getGameCycleCount() + getObjectID()) % 512) == 0)) {
        context.map.viewMap(owner->getHouseID(), location, getViewRange(context.game));
    }

    parent::move(context);
}

void GroundUnit::navigate(const GameContext& context) {
    // Lets keep units moving even if they are awaiting a pickup
    // Could potentially make this distance based depending on how
    // far away the booked carrier is
    if(!awaitingPickup) { parent::navigate(context); }
}

void GroundUnit::doRepair(const GameContext& context) noexcept {
    if(getHealth() >= getMaxHealth(context.game)) return;

    // find a repair yard to return to

    FixPoint    closestLeastBookedRepairYardDistance = 1000000;
    RepairYard* pBestRepairYard                      = nullptr;

    for(auto* pStructure : context.game.structureList) {
        auto* const pRepairYard = dune_cast<RepairYard>(pStructure);
        if(pRepairYard && (pStructure->getOwner() == owner)) {

            if(pRepairYard->getNumBookings() == 0) {
                const auto tempDistance = blockDistance(location, pRepairYard->getClosestPoint(location));
                if(tempDistance < closestLeastBookedRepairYardDistance) {
                    closestLeastBookedRepairYardDistance = tempDistance;
                    pBestRepairYard                      = pRepairYard;
                }
            }
        }
    }

    if(pBestRepairYard) {
        requestCarryall(context);
        doMove2Object(context, pBestRepairYard);
    }
}

} // namespace Dune::Engine
