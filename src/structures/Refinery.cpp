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

Refinery::Refinery(House* newOwner) : StructureBase(newOwner) {
    Refinery::init();

    setHealth(getMaxHealth());

    extractingSpice = false;
    bookings = 0;

    firstRun = true;

    firstAnimFrame = 2;
    lastAnimFrame = 3;
}

Refinery::Refinery(InputStream& stream) : StructureBase(stream) {
    Refinery::init();

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
    itemID = Structure_Refinery;
    owner->incrementStructures(itemID);

    structureSize.x = 3;
    structureSize.y = 2;

    graphicID = ObjPic_Refinery;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    numImagesX = 10;
    numImagesY = 1;
}

Refinery::~Refinery() {
    if(extractingSpice && harvester) {
        if(harvester.getUnitPointer() != nullptr)
            harvester.getUnitPointer()->destroy();
        harvester.pointTo(NONE_ID);
    }
}

void Refinery::save(OutputStream& stream) const {
    StructureBase::save(stream);

    stream.writeBool(extractingSpice);
    harvester.save(stream);
    stream.writeUint32(bookings);
}

ObjectInterface* Refinery::getInterfaceContainer() {
    if((pLocalHouse == owner) || (debug == true)) {
        return RefineryAndSiloInterface::create(objectID);
    } else {
        return DefaultObjectInterface::create(objectID);
    }
}

void Refinery::assignHarvester(Harvester* newHarvester) {
    extractingSpice = true;
    harvester.pointTo(newHarvester);
    drawnAngle = 1;
    firstAnimFrame = 8;
    lastAnimFrame = 9;
    curAnimFrame = 8;
}

void Refinery::deployHarvester(Carryall* pCarryall) {
    unBook();
    drawnAngle = 0;
    extractingSpice = false;

    if(firstRun) {
        if(getOwner() == pLocalHouse) {
            soundPlayer->playVoice(HarvesterDeployed,getOwner()->getHouseID());
        }
    }

    firstRun = false;

    Harvester* pHarvester = static_cast<Harvester*>(harvester.getObjPointer());
    if((pCarryall != nullptr) && pHarvester->getGuardPoint().isValid()) {
        pCarryall->giveCargo(pHarvester);
        pCarryall->setTarget(nullptr);
        pCarryall->setDestination(pHarvester->getGuardPoint());
    } else {
        Coord deployPos = currentGameMap->findDeploySpot(pHarvester, location, currentGame->randomGen, destination, structureSize);
        pHarvester->deploy(deployPos);
    }

    if(bookings == 0) {
        stopAnimate();
    } else {
        startAnimate();
    }
}

void Refinery::startAnimate() {
    if(extractingSpice == false) {
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

void Refinery::updateStructureSpecificStuff() {
    if(extractingSpice) {
        Harvester* pHarvester = static_cast<Harvester*>(harvester.getObjPointer());

        if(pHarvester->getAmountOfSpice() > 0) {
            FixPoint extractionSpeed = MAXIMUMHARVESTEREXTRACTSPEED;

            int scale = floor(5*getHealth()/getMaxHealth());
            if(scale == 0) {
                scale = 1;
            }

            extractionSpeed = (extractionSpeed * scale) / 5;


            owner->addCredits(pHarvester->extractSpice(extractionSpeed), true);
        } else if((pHarvester->isAwaitingPickup() == false) && (pHarvester->getGuardPoint().isValid())) {
            // find carryall
            Carryall* pCarryall = nullptr;
            if((pHarvester->getGuardPoint().isValid()) && getOwner()->hasCarryalls())   {
                for(UnitBase* pUnit : unitList) {
                    if ((pUnit->getOwner() == owner) && (pUnit->getItemID() == Unit_Carryall)) {
                        Carryall* pTmpCarryall = static_cast<Carryall*>(pUnit);
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
                deployHarvester();
            }
        } else if(!pHarvester->hasBookedCarrier()) {
            deployHarvester();
        }
    }
}
