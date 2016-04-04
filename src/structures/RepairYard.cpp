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

RepairYard::RepairYard(House* newOwner) : StructureBase(newOwner) {
    RepairYard::init();

    setHealth(getMaxHealth());
    bookings = 0;
	repairing = false;
}

RepairYard::RepairYard(InputStream& stream) : StructureBase(stream) {
    RepairYard::init();

	repairing = stream.readBool();
	repairUnit.load(stream);
	bookings = stream.readUint32();
}

void RepairYard::init() {
    itemID = Structure_RepairYard;
	owner->incrementStructures(itemID);

	structureSize.x = 3;
	structureSize.y = 2;

	graphicID = ObjPic_RepairYard;
	graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
	numImagesX = 10;
	numImagesY = 1;
	firstAnimFrame = 2;
	lastAnimFrame = 5;
}

RepairYard::~RepairYard() {
    if(repairing) {
		unBook();
		repairUnit.getUnitPointer()->destroy();
	}
}

void RepairYard::save(OutputStream& stream) const {
	StructureBase::save(stream);

	stream.writeBool(repairing);
	repairUnit.save(stream);
	stream.writeUint32(bookings);
}


ObjectInterface* RepairYard::getInterfaceContainer() {
	if((pLocalHouse == owner) || (debug == true)) {
		return RepairYardInterface::create(objectID);
	} else {
		return DefaultObjectInterface::create(objectID);
	}
}

void RepairYard::deployRepairUnit(Carryall* pCarryall) {
	unBook();
	repairing = false;
	firstAnimFrame = 2;
	lastAnimFrame = 5;

    UnitBase* pRepairUnit = repairUnit.getUnitPointer();
	if(pCarryall != nullptr) {
	    pCarryall->giveCargo(pRepairUnit);
	    pCarryall->setTarget(nullptr);
	    pCarryall->setDestination(pRepairUnit->getGuardPoint());
	} else {
        Coord deployPos = currentGameMap->findDeploySpot(pRepairUnit, location, destination, structureSize);

        if(pRepairUnit->getItemID() != Unit_Harvester){
            pRepairUnit->setForced(false);
            pRepairUnit->setTarget(nullptr);
            //pRepairUnit->setDestination(nullptr);
            pRepairUnit->doSetAttackMode(GUARD);

        }else{
            // If we need additional harvester logic
            pRepairUnit->doSetAttackMode(HARVEST);
        }
        pRepairUnit->deploy(deployPos);
        /**
            Need to fix at some point in a balanced way
        **/
        if(pRepairUnit->getAttackMode() == HUNT){
            pRepairUnit->doSetAttackMode(GUARD);
        }
        pRepairUnit->setTarget(nullptr);
        pRepairUnit->setDestination(pRepairUnit->getLocation());

	}

	repairUnit.pointTo(NONE);

	if(getOwner() == pLocalHouse) {
		soundPlayer->playVoice(VehicleRepaired,getOwner()->getHouseID());
	}
}

void RepairYard::updateStructureSpecificStuff() {
	if(repairing) {
		if(curAnimFrame < 6) {
			firstAnimFrame = 6;
			lastAnimFrame = 9;
			curAnimFrame = 6;
		}
	} else {
		if(curAnimFrame > 5) {
			firstAnimFrame = 2;
			lastAnimFrame = 5;
			curAnimFrame = 2;
		}
	}

	if(repairing == true) {
	    UnitBase* pRepairUnit = repairUnit.getUnitPointer();

		if (pRepairUnit->getHealth()*100/pRepairUnit->getMaxHealth() < 100) {
			if (owner->takeCredits(UNIT_REPAIRCOST) > 0) {
				pRepairUnit->addHealth();
			}

		} else if(((GroundUnit*)pRepairUnit)->isawaitingPickup() == false) {
		    // find carryall
		    Carryall* pCarryall = nullptr;
            if((pRepairUnit->getGuardPoint().isValid()) && getOwner()->hasCarryalls())	{
                RobustList<UnitBase*>::const_iterator iter;
                for(iter = unitList.begin(); iter != unitList.end(); ++iter) {
                    UnitBase* unit = *iter;
                    if ((unit->getOwner() == owner) && (unit->getItemID() == Unit_Carryall)) {
                        if (((Carryall*)unit)->isRespondable() && !((Carryall*)unit)->isBooked()) {
                            pCarryall = (Carryall*)unit;
                        }
                    }
                }
            }

            if(pCarryall != nullptr) {
                /*
                pCarryall->setTarget(this);
                pCarryall->clearPath();
                ((GroundUnit*)pRepairUnit)->bookCarrier(pCarryall);
                pRepairUnit->setTarget(nullptr);
                pRepairUnit->setDestination(pRepairUnit->getGuardPoint());
                */
                deployRepairUnit(pCarryall);
            } else {
                deployRepairUnit();
            }
		} else if(((GroundUnit*)pRepairUnit)->hasBookedCarrier() == false) {
            deployRepairUnit();
		}
	}
}
