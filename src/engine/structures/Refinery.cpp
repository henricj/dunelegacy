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

#include <structures/Refinery.h>

#include <House.h>
#include <Map.h>

#include <units/UnitBase.h>
#include <units/Harvester.h>
#include <units/Carryall.h>

namespace {
using namespace Dune::Engine;

/* how fast is spice extracted */
constexpr auto MAXIMUMHARVESTEREXTRACTSPEED = 0.625_fix;

constexpr StructureBaseConstants refinery_constants{Refinery::item_id, Coord{3, 2}};
}

namespace Dune::Engine {

Refinery::Refinery(uint32_t objectID, const ObjectInitializer& initializer)
    : StructureBase(refinery_constants, objectID, initializer) {
    Refinery::init();

    ObjectBase::setHealth(initializer.game(), getMaxHealth(initializer.game()));

    extractingSpice = false;
    bookings        = 0;

    firstRun = true;
}

Refinery::Refinery(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : StructureBase(refinery_constants, objectID, initializer) {
    Refinery::init();

    auto& stream = initializer.stream();

    extractingSpice = stream.readBool();
    harvester.load(stream);
    bookings = stream.readUint32();

    if(extractingSpice) {
    } else if(bookings == 0) {
        stopAnimate();
    } else {
        startAnimate();
    }

    firstRun = false;
}

void Refinery::init() {
    assert(itemID == Structure_Refinery);
    owner->incrementStructures(itemID);
}

Refinery::~Refinery() = default;

void Refinery::cleanup(const GameContext& context, HumanPlayer* humanPlayer) {
    if(extractingSpice && harvester) {
        auto* pHarvester = harvester.getUnitPointer(context.objectManager);
        if(pHarvester) pHarvester->destroy(context);
        harvester.pointTo(NONE_ID);
    }

    parent::cleanup(context, humanPlayer);
}

void Refinery::save(const Game& game, OutputStream& stream) const {
    StructureBase::save(game, stream);

    stream.writeBool(extractingSpice);
    harvester.save(stream);
    stream.writeUint32(bookings);
}

void Refinery::assignHarvester(Harvester* newHarvester) {
    extractingSpice = true;
    harvester.pointTo(newHarvester);
    drawnAngle = static_cast<ANGLETYPE>(1);
}

void Refinery::deployHarvester(const GameContext& context, Carryall* pCarryall) {
    unBook();
    drawnAngle      = static_cast<ANGLETYPE>(0);
    extractingSpice = false;

    if(firstRun) { }

    firstRun = false;

    auto* pHarvester = dune_cast<Harvester>(harvester.getObjPointer(context.objectManager));
    if((pCarryall != nullptr) && pHarvester->getGuardPoint().isValid()) {
        pCarryall->giveCargo(context, pHarvester);
        pCarryall->setTarget(context.objectManager, nullptr);
        pCarryall->setDestination(context, pHarvester->getGuardPoint());
    } else {
        const Coord deployPos = context.map.findDeploySpot(pHarvester, location, destination, getStructureSize());
        pHarvester->deploy(context, deployPos);
    }

    if(bookings == 0) {
        stopAnimate();
    } else {
        startAnimate();
    }
}

void Refinery::startAnimate() {
    if(!extractingSpice) { justPlacedTimer = 0; }
}

void Refinery::stopAnimate() { }

void Refinery::updateStructureSpecificStuff(const GameContext& context) {
    if(extractingSpice) {
        auto* pHarvester = dune_cast<Harvester>(harvester.getObjPointer(context.objectManager));

        if(!pHarvester) {
            harvester.reset();
            return;
        }

        if(pHarvester->getAmountOfSpice() > 0) {
            FixPoint extractionSpeed = MAXIMUMHARVESTEREXTRACTSPEED;

            int scale = floor(5 * getHealth() / getMaxHealth(context.game));
            if(scale == 0) { scale = 1; }

            extractionSpeed = (extractionSpeed * scale) / 5;

            owner->addCredits(pHarvester->extractSpice(extractionSpeed), true);
        } else if((!pHarvester->isAwaitingPickup()) && (pHarvester->getGuardPoint().isValid())) {
            // find carryall
            Carryall* pCarryall = nullptr;
            if((pHarvester->getGuardPoint().isValid()) && getOwner()->hasCarryalls()) {
                for(auto* pUnit : context.game.unitList) {
                    if((pUnit->getOwner() == owner) && (pUnit->getItemID() == Unit_Carryall)) {
                        auto* const pTmpCarryall = dune_cast<Carryall>(pUnit);
                        if(pTmpCarryall && !pTmpCarryall->isBooked()) {
                            pCarryall = pTmpCarryall;
                            break;
                        }
                    }
                }
            }

            if(pCarryall != nullptr) {
                pCarryall->setTarget(context.objectManager, this);
                pCarryall->clearPath();
                pHarvester->bookCarrier(pCarryall);
                pHarvester->setTarget(context.objectManager, nullptr);
                pHarvester->setDestination(context, pHarvester->getGuardPoint());
            } else {
                deployHarvester(context);
            }
        } else if(!pHarvester->hasBookedCarrier(context.objectManager)) {
            deployHarvester(context);
        }
    }
}

} // namespace Dune::Engine
