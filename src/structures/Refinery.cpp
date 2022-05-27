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

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Map.h>
#include <SoundPlayer.h>

#include <units/Carryall.h>
#include <units/Harvester.h>
#include <units/UnitBase.h>

#include <GUI/ObjectInterfaces/RefineryAndSiloInterface.h>

namespace {
/* how fast is spice extracted */
constexpr auto MAXIMUMHARVESTEREXTRACTSPEED = 0.625_fix;

constexpr StructureBaseConstants refinery_constants{Refinery::item_id, Coord{3, 2}};
} // namespace

Refinery::Refinery(uint32_t objectID, const ObjectInitializer& initializer)
    : StructureBase(refinery_constants, objectID, initializer) {
    Refinery::init();

    ObjectBase::setHealth(getMaxHealth());

    firstAnimFrame = 2;
    lastAnimFrame  = 3;
}

Refinery::Refinery(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : StructureBase(refinery_constants, objectID, initializer) {
    Refinery::init();

    auto& stream = initializer.stream();

    extractingSpice = stream.readBool();
    harvester.load(stream);
    bookings = stream.readUint32();

    if (extractingSpice) {
        firstAnimFrame = 8;
        lastAnimFrame  = 9;
        curAnimFrame   = 8;
    } else if (bookings == 0) {
        stopAnimate();
    } else {
        startAnimate();
    }

    firstRun = false;
}

void Refinery::init() {
    assert(itemID_ == Structure_Refinery);
    owner_->incrementStructures(itemID_);

    graphicID_  = ObjPic_Refinery;
    graphic_    = dune::globals::pGFXManager->getObjPic(graphicID_, getOwner()->getHouseID());
    numImagesX_ = 10;
    numImagesY_ = 1;
}

Refinery::~Refinery() = default;

void Refinery::cleanup(const GameContext& context, HumanPlayer* humanPlayer) {
    if (extractingSpice && harvester) {
        if (auto* pHarvester = harvester.getUnitPointer())
            pHarvester->destroy(context);
        harvester.pointTo(NONE_ID);
    }

    parent::cleanup(context, humanPlayer);
}

void Refinery::save(OutputStream& stream) const {
    StructureBase::save(stream);

    stream.writeBool(extractingSpice);
    harvester.save(stream);
    stream.writeUint32(bookings);
}

std::unique_ptr<ObjectInterface> Refinery::getInterfaceContainer(const GameContext& context) {
    if (dune::globals::pLocalHouse == owner_ || dune::globals::debug) {
        return RefineryAndSiloInterface::create(context, objectID_);
    }
    return DefaultObjectInterface::create(context, objectID_);
}

void Refinery::assignHarvester(Harvester* newHarvester) {
    extractingSpice = true;
    harvester.pointTo(newHarvester);
    drawnAngle_    = static_cast<ANGLETYPE>(1);
    firstAnimFrame = 8;
    lastAnimFrame  = 9;
    curAnimFrame   = 8;
}

void Refinery::deployHarvester(const GameContext& context, Carryall* pCarryall) {
    unBook();
    drawnAngle_     = static_cast<ANGLETYPE>(0);
    extractingSpice = false;

    if (firstRun) {
        if (getOwner() == dune::globals::pLocalHouse)
            dune::globals::soundPlayer->playVoice(Voice_enum::HarvesterDeployed, getOwner()->getHouseID());
    }

    firstRun = false;

    if (auto* const pHarvester = getHarvester()) {
        if (pCarryall != nullptr && pHarvester->getGuardPoint().isValid()) {
            pCarryall->giveCargo(context, pHarvester);
            pCarryall->setTarget(nullptr);
            pCarryall->setDestination(pHarvester->getGuardPoint());
        } else {
            const auto deployPos =
                dune::globals::currentGameMap->findDeploySpot(pHarvester, location_, destination_, getStructureSize());

            if (deployPos.isInvalid()) {
                sdl2::log_error("Unable to locate deployment location for harvester!");
                pHarvester->setHealth(0_fix); // TODO: Just blow it up?
            } else
                pHarvester->deploy(context, deployPos);
        }
    } else
        sdl2::log_error("A refinery is trying to deploy a non-existent harvester!");

    if (bookings == 0)
        stopAnimate();
    else
        startAnimate();
}

void Refinery::startAnimate() {
    if (!extractingSpice) {
        firstAnimFrame   = 2;
        lastAnimFrame    = 7;
        curAnimFrame     = 2;
        justPlacedTimer  = 0;
        animationCounter = 0;
    }
}

void Refinery::stopAnimate() {
    firstAnimFrame = 2;
    lastAnimFrame  = 3;
    curAnimFrame   = 2;
}

void Refinery::updateStructureSpecificStuff(const GameContext& context) {
    if (!extractingSpice)
        return;

    auto* const unit       = harvester.getObjPointer();
    auto* const pHarvester = dune_cast<Harvester>(unit);

    if (!pHarvester) {
        if (unit) {
            sdl2::log_error(
                "A refinery is extracting spice from object %d, but it is type %d, which is not a harvester!",
                unit->getObjectID(), unit->getItemID());

            deployHarvester(context);
        } else {
            sdl2::log_error("A refinery is extracting spice but has no harvester!");

            // We don't know how we got here, so leave the bookings alone.
            extractingSpice = false;
        }

        return;
    }

    if (pHarvester->getAmountOfSpice() > 0) {
        FixPoint extractionSpeed = MAXIMUMHARVESTEREXTRACTSPEED;

        int scale = floor(5 * getHealth() / getMaxHealth());
        if (scale == 0)
            scale = 1;

        extractionSpeed = extractionSpeed * scale / 5;

        owner_->addCredits(pHarvester->extractSpice(extractionSpeed), true);
    } else if (!pHarvester->isAwaitingPickup() && pHarvester->getGuardPoint().isValid()) {
        // find carryall
        Carryall* pCarryall = nullptr;
        if (pHarvester->getGuardPoint().isValid() && getOwner()->hasCarryalls()) {
            for (auto* const pUnit : dune::globals::unitList) {
                if (pUnit->getOwner() == owner_)
                    continue;

                if (auto* const pTmpCarryall = dune_cast<Carryall>(pUnit)) {
                    if (!pTmpCarryall->isBooked()) {
                        pCarryall = pTmpCarryall;
                        break;
                    }
                }
            }
        }

        if (pCarryall != nullptr) {
            pCarryall->setTarget(this);
            pCarryall->clearPath();
            pHarvester->bookCarrier(pCarryall);
            pHarvester->setTarget(nullptr);
            pHarvester->setDestination(pHarvester->getGuardPoint());
        } else {
            deployHarvester(context);
        }
    } else if (!pHarvester->hasBookedCarrier()) {
        deployHarvester(context);
    }
}
