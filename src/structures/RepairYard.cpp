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

#include <structures/RepairYard.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Map.h>
#include <SoundPlayer.h>

#include <units/Carryall.h>
#include <units/GroundUnit.h>
#include <units/Harvester.h>

#include <GUI/ObjectInterfaces/RepairYardInterface.h>

namespace {
constexpr StructureBaseConstants repair_yard_constants{RepairYard::item_id, Coord{3, 2}};
}

RepairYard::RepairYard(uint32_t objectID, const ObjectInitializer& initializer)
    : StructureBase(repair_yard_constants, objectID, initializer) {
    RepairYard::init();

    RepairYard::setHealth(getMaxHealth());
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

    graphicID      = ObjPic_RepairYard;
    graphic        = dune::globals::pGFXManager->getObjPic(graphicID, getOwner()->getHouseID());
    numImagesX     = 10;
    numImagesY     = 1;
    firstAnimFrame = 2;
    lastAnimFrame  = 3;
}

RepairYard::~RepairYard() = default;

void RepairYard::cleanup(const GameContext& context, HumanPlayer* humanPlayer) {
    if (repairingAUnit) {
        unBook();
        if (auto* const unit = repairUnit.getUnitPointer())
            unit->destroy(context);
    }

    parent::cleanup(context, humanPlayer);
}

void RepairYard::save(OutputStream& stream) const {
    StructureBase::save(stream);

    stream.writeBool(repairingAUnit);
    repairUnit.save(stream);
    stream.writeUint32(bookings);
}

std::unique_ptr<ObjectInterface> RepairYard::getInterfaceContainer(const GameContext& context) {
    if ((dune::globals::pLocalHouse == owner) || (dune::globals::debug)) {
        return RepairYardInterface::create(context, objectID);
    }
    return DefaultObjectInterface::create(context, objectID);
}

void RepairYard::deployRepairUnit(const GameContext& context, Carryall* pCarryall) {
    unBook();
    repairingAUnit = false;
    firstAnimFrame = 2;
    lastAnimFrame  = 3;

    UnitBase* pRepairUnit = repairUnit.getUnitPointer();

    if (nullptr == pRepairUnit)
        return;

    if (pCarryall != nullptr) {
        pCarryall->giveCargo(context, pRepairUnit);
        pCarryall->setTarget(nullptr);
        pCarryall->setDestination(pRepairUnit->getGuardPoint());
    } else {
        const Coord deployPos = context.map.findDeploySpot(pRepairUnit, location, destination, getStructureSize());

        pRepairUnit->setForced(false);
        pRepairUnit->doSetAttackMode(context, (pRepairUnit->getItemID() == Unit_Harvester) ? HARVEST : GUARD);
        pRepairUnit->deploy(context, deployPos);
        pRepairUnit->setTarget(nullptr);
        pRepairUnit->setDestination(pRepairUnit->getLocation());
    }

    repairUnit.pointTo(NONE_ID);

    if (getOwner() == dune::globals::pLocalHouse) {
        dune::globals::soundPlayer->playVoice(Voice_enum::VehicleRepaired, getOwner()->getHouseID());
    }
}

void RepairYard::updateStructureSpecificStuff(const GameContext& context) {
    if (repairingAUnit) {
        if (curAnimFrame < 6) {
            firstAnimFrame = 6;
            lastAnimFrame  = 9;
            curAnimFrame   = 6;
        }
    } else {
        if (curAnimFrame > 3) {
            firstAnimFrame = 2;
            lastAnimFrame  = 3;
            curAnimFrame   = 2;
        }
    }

    if (repairingAUnit != true)
        return;

    auto* pRepairUnit = dune_cast<GroundUnit>(repairUnit.getUnitPointer());

    if (nullptr == pRepairUnit)
        return;

    if (pRepairUnit->getHealth() * 100 / pRepairUnit->getMaxHealth() < 100) {
        if (owner->takeCredits(UNIT_REPAIRCOST) > 0)
            pRepairUnit->addHealth();

        /*
            Will turn this code into an option that's only on when manual carryall is enabled.
            While this is fixing the original code, it can cause imbalances in combat as tanks
            can be kept at the frontline letting an initial successful attack snowball
        */
    } else if (!pRepairUnit->isAwaitingPickup()
               && blockDistance(location, pRepairUnit->getGuardPoint()) >= MIN_CARRYALL_LIFT_DISTANCE
               && context.game.getGameInitSettings().getGameOptions().manualCarryallDrops) {
        // find carryall
        Carryall* pCarryall = nullptr;
        if ((pRepairUnit->getGuardPoint().isValid()) && getOwner()->hasCarryalls()) {
            for (auto* pUnit : dune::globals::unitList) {
                if (pUnit->getOwner() != owner)
                    continue;

                if (auto* const pTmpCarryall = dune_cast<Carryall>(pUnit)) {
                    if (!pTmpCarryall->isBooked())
                        pCarryall = pTmpCarryall;
                }
            }
        }

        if (pCarryall != nullptr) {
            pCarryall->setTarget(this);
            pCarryall->clearPath();
            pRepairUnit->bookCarrier(pCarryall);
            pRepairUnit->setTarget(nullptr);
            pRepairUnit->setDestination(pRepairUnit->getGuardPoint());
        } else
            deployRepairUnit(context);
    } else
        deployRepairUnit(context);
}
