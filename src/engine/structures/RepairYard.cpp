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

#include <structures/RepairYard.h>

#include <House.h>
#include <Map.h>

#include <units/Carryall.h>
#include <units/GroundUnit.h>
#include <units/Harvester.h>

namespace {
using namespace Dune::Engine;

constexpr StructureBaseConstants repair_yard_constants{RepairYard::item_id, Coord{3, 2}};
}

namespace Dune::Engine {

RepairYard::RepairYard(uint32_t objectID, const ObjectInitializer& initializer)
    : StructureBase(repair_yard_constants, objectID, initializer) {
    RepairYard::init();

    RepairYard::setHealth(initializer.game(), getMaxHealth(initializer.game()));
    bookings       = 0;
    repairingAUnit = false;
}

RepairYard::RepairYard(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : StructureBase(repair_yard_constants, objectID, initializer) {
    RepairYard::init();

    auto& stream = initializer.stream();

    repairingAUnit = stream.readBool();
    repairUnit.load(stream);
    bookings = stream.readUint32();
}

void RepairYard::init() {
    assert(itemID == Structure_RepairYard);
    owner->incrementStructures(itemID);
}

RepairYard::~RepairYard() = default;

void RepairYard::cleanup(const GameContext& context, HumanPlayer* humanPlayer) {
    if(repairingAUnit) {
        unBook();
        auto* const unit = repairUnit.getUnitPointer(context.objectManager);
        if(unit) unit->destroy(context);
    }

    parent::cleanup(context, humanPlayer);
}

void RepairYard::save(const Game& game, OutputStream& stream) const {
    StructureBase::save(game, stream);

    stream.writeBool(repairingAUnit);
    repairUnit.save(stream);
    stream.writeUint32(bookings);
}

void RepairYard::deployRepairUnit(const GameContext& context, Carryall* pCarryall) {
    unBook();
    repairingAUnit = false;

    if(auto* const pRepairUnit = repairUnit.getUnitPointer(context.objectManager)) {
        if(pCarryall != nullptr) {
            pCarryall->giveCargo(context, pRepairUnit);
            pCarryall->setTarget(context.objectManager, nullptr);
            pCarryall->setDestination(context, pRepairUnit->getGuardPoint());
        } else {
            const auto deployPos = context.map.findDeploySpot(pRepairUnit, location, destination, getStructureSize());

            pRepairUnit->setForced(false);
            pRepairUnit->doSetAttackMode(context, (pRepairUnit->getItemID() == Unit_Harvester) ? HARVEST : GUARD);
            pRepairUnit->deploy(context, deployPos);
            pRepairUnit->setTarget(context.objectManager, nullptr);
            pRepairUnit->setDestination(context, pRepairUnit->getLocation());
        }
    }

    repairUnit.pointTo(NONE_ID);
}

void RepairYard::updateStructureSpecificStuff(const GameContext& context) {
    if(!repairingAUnit) return;

    auto* const pRepairUnit = dune_cast<GroundUnit>(repairUnit.getUnitPointer(context.objectManager));

    if(!pRepairUnit) {
        Dune::Logger.log("RepairYard trying to repair phantom unit");
        unBook();
        repairingAUnit = false;
        repairUnit.reset();
        return;
    }

    if(pRepairUnit->getHealth() * 100 / pRepairUnit->getMaxHealth(context.game) < 100) {
        if(owner->takeCredits(UNIT_REPAIRCOST) > 0) { pRepairUnit->addHealth(context.game); }

    } else if(!pRepairUnit->isAwaitingPickup() &&
              blockDistance(location, pRepairUnit->getGuardPoint()) >= MIN_CARRYALL_LIFT_DISTANCE) {
        // find carryall
        Carryall* pCarryall = nullptr;
        if((pRepairUnit->getGuardPoint().isValid()) && getOwner()->hasCarryalls()) {
            for(auto* const pUnit : context.game.unitList) {
                if((pUnit->getOwner() == owner) && (pUnit->getItemID() == Unit_Carryall)) {
                    auto* pTmpCarryall = dune_cast<Carryall>(pUnit);
                    if(pTmpCarryall && !pTmpCarryall->isBooked()) { pCarryall = pTmpCarryall; }
                }
            }
        }

        if(pCarryall != nullptr) {
            pCarryall->setTarget(context.objectManager, this);
            pCarryall->clearPath();
            if(auto* const ground_unit = dune_cast<GroundUnit>(pRepairUnit)) ground_unit->bookCarrier(pCarryall);
            pRepairUnit->setTarget(context.objectManager, nullptr);
            pRepairUnit->setDestination(context, pRepairUnit->getGuardPoint());
        } else {
            deployRepairUnit(context);
        }
    } else if(!pRepairUnit->hasBookedCarrier(context.objectManager)) {
        deployRepairUnit(context);
    }
}

} // namespace Dune::Engine
