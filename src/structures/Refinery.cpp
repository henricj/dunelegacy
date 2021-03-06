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
#include <SoundPlayer.h>
#include <Map.h>

#include <units/UnitBase.h>
#include <units/Harvester.h>
#include <units/Carryall.h>

#include <GUI/ObjectInterfaces/RefineryAndSiloInterface.h>

/* how fast is spice extracted */
#define MAXIMUMHARVESTEREXTRACTSPEED (0.625_fix)

namespace {
constexpr StructureBaseConstants refinery_constants{Refinery::item_id, Coord{3, 2}};
}

Refinery::Refinery(Uint32 objectID, const ObjectInitializer& initializer) : StructureBase(refinery_constants, objectID, initializer) {
    Refinery::init();

    ObjectBase::setHealth(getMaxHealth());

    extractingSpice = false;
    bookings = 0;

    firstRun = true;

    firstAnimFrame = 2;
    lastAnimFrame = 3;
}

Refinery::Refinery(Uint32 objectID, const ObjectStreamInitializer& initializer) : StructureBase(refinery_constants, objectID, initializer) {
    Refinery::init();

    auto& stream = initializer.stream();

    extractingSpice = stream.readBool();
    harvester.load(stream);
    bookings = stream.readUint32();

    if(extractingSpice) {
        firstAnimFrame = 8;
        lastAnimFrame = 9;
        curAnimFrame = 8;
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

    graphicID = ObjPic_Refinery;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    numImagesX = 10;
    numImagesY = 1;
}

Refinery::~Refinery() = default;

void Refinery::cleanup(const GameContext& context, HumanPlayer* humanPlayer) {
    if(extractingSpice && harvester) {
        auto* pHarvester = harvester.getUnitPointer();
        if(pHarvester) pHarvester->destroy(context);
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
    if((pLocalHouse == owner) || (debug)) { return RefineryAndSiloInterface::create(context, objectID); }
    return DefaultObjectInterface::create(context, objectID);
}

void Refinery::assignHarvester(Harvester* newHarvester) {
    extractingSpice = true;
    harvester.pointTo(newHarvester);
    drawnAngle = static_cast<ANGLETYPE>(1);
    firstAnimFrame = 8;
    lastAnimFrame = 9;
    curAnimFrame = 8;
}

void Refinery::deployHarvester(const GameContext& context, Carryall* pCarryall) {
    unBook();
    drawnAngle = static_cast<ANGLETYPE>(0);
    extractingSpice = false;

    if(firstRun) {
        if(getOwner() == pLocalHouse) {
            soundPlayer->playVoice(HarvesterDeployed,getOwner()->getHouseID());
        }
    }

    firstRun = false;

    auto* pHarvester = static_cast<Harvester*>(harvester.getObjPointer());
    if((pCarryall != nullptr) && pHarvester->getGuardPoint().isValid()) {
        pCarryall->giveCargo(context, pHarvester);
        pCarryall->setTarget(nullptr);
        pCarryall->setDestination(pHarvester->getGuardPoint());
    } else {
        Coord deployPos = currentGameMap->findDeploySpot(pHarvester, location, destination, getStructureSize());
        pHarvester->deploy(context, deployPos);
    }

    if(bookings == 0) {
        stopAnimate();
    } else {
        startAnimate();
    }
}

void Refinery::startAnimate() {
    if(!extractingSpice) {
        firstAnimFrame = 2;
        lastAnimFrame = 7;
        curAnimFrame = 2;
        justPlacedTimer = 0;
        animationCounter = 0;
    }
}

void Refinery::stopAnimate() {
    firstAnimFrame = 2;
    lastAnimFrame = 3;
    curAnimFrame = 2;
}

void Refinery::updateStructureSpecificStuff(const GameContext& context) {
    if(extractingSpice) {
        auto* pHarvester = static_cast<Harvester*>(harvester.getObjPointer());

        if(pHarvester->getAmountOfSpice() > 0) {
            FixPoint extractionSpeed = MAXIMUMHARVESTEREXTRACTSPEED;

            int scale = floor(5*getHealth()/getMaxHealth());
            if(scale == 0) {
                scale = 1;
            }

            extractionSpeed = (extractionSpeed * scale) / 5;


            owner->addCredits(pHarvester->extractSpice(extractionSpeed), true);
        } else if((!pHarvester->isAwaitingPickup()) && (pHarvester->getGuardPoint().isValid())) {
            // find carryall
            Carryall* pCarryall = nullptr;
            if((pHarvester->getGuardPoint().isValid()) && getOwner()->hasCarryalls())   {
                for(UnitBase* pUnit : unitList) {
                    if ((pUnit->getOwner() == owner) && (pUnit->getItemID() == Unit_Carryall)) {
                        auto* pTmpCarryall = static_cast<Carryall*>(pUnit);
                        if (!pTmpCarryall->isBooked()) {
                            pCarryall = pTmpCarryall;
                            break;
                        }
                    }
                }
            }

            if(pCarryall != nullptr) {
                pCarryall->setTarget(this);
                pCarryall->clearPath();
                pHarvester->bookCarrier(pCarryall);
                pHarvester->setTarget(nullptr);
                pHarvester->setDestination(pHarvester->getGuardPoint());
            } else {
                deployHarvester(context);
            }
        } else if(!pHarvester->hasBookedCarrier()) {
            deployHarvester(context);
        }
    }
}
