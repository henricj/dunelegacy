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

#include <units/GroundUnit.h>

#include <globals.h>

#include <Game.h>
#include <House.h>
#include <Map.h>
#include <SoundPlayer.h>

#include <structures/RepairYard.h>
#include <units/Carryall.h>

GroundUnit::GroundUnit(House* newOwner) : UnitBase(newOwner) {

    GroundUnit::init();

    awaitingPickup = false;
	bookedCarrier = NONE;
}

GroundUnit::GroundUnit(InputStream& stream) : UnitBase(stream) {

    GroundUnit::init();

    awaitingPickup = stream.readBool();
    bookedCarrier = stream.readUint32();
}

void GroundUnit::init() {
    aGroundUnit = true;
}

GroundUnit::~GroundUnit() {
}

void GroundUnit::save(OutputStream& stream) const {
	UnitBase::save(stream);

	stream.writeBool(awaitingPickup);
	stream.writeUint32(bookedCarrier);
}

void GroundUnit::assignToMap(const Coord& pos) {
	if (currentGameMap->tileExists(pos)) {
		currentGameMap->getTile(pos)->assignNonInfantryGroundObject(getObjectID());
		currentGameMap->viewMap(owner->getTeam(), location, getViewRange());
	}
}

void GroundUnit::checkPos() {
    if(!moving && !justStoppedMoving && !isInfantry()) {
        currentGameMap->getTile(location.x,location.y)->setTrack(drawnAngle);
    }

	if(justStoppedMoving)
	{
		realX = location.x*TILESIZE + TILESIZE/2;
		realY = location.y*TILESIZE + TILESIZE/2;
		//findTargetTimer = 0;	//allow a scan for new targets now

		if(currentGameMap->getTile(location)->isSpiceBloom()) {
		    setHealth(0.0f);
		    setVisible(VIS_ALL, false);
			currentGameMap->getTile(location)->triggerSpiceBloom(getOwner());
		} else if(currentGameMap->getTile(location)->isSpecialBloom()){
            currentGameMap->getTile(location)->triggerSpecialBloom(getOwner());
		}
	}

    /*
        Go to repair yard if low on health
    */
    if(active && (getHealthColor() != COLOR_LIGHTGREEN)
            && !goingToRepairYard
            && owner->hasRepairYard()
            && !pickedUp
            && owner->hasCarryalls()
            && !isInfantry()
            && !forced ) { // Stefan - Allow units with targets to be picked up for repairs

        doRepair();

    }


	if(goingToRepairYard) {
	    if(target.getObjPointer() == NULL) {
            goingToRepairYard = false;
            awaitingPickup = false;
            bookedCarrier = NONE;

            clearPath();
	    } else{
            Coord closestPoint = target.getObjPointer()->getClosestPoint(location);
            if (!moving && !justStoppedMoving && (blockDistance(location, closestPoint) <= 1.5f)
                && ((RepairYard*)target.getObjPointer())->isFree())
            {
                if (getHealth()*100.0f/getMaxHealth() < 100.0f) {
                    setGettingRepaired();
                } else {
                    setTarget(NULL);
                    setDestination(guardPoint);
                }
            }
	    }
	}


	/*
        If we are awaiting a pickup try book a carryall if we have one
	*/
	if( attackMode == CARRYALLREQUESTED
        && bookedCarrier == NONE){

        if(getOwner()->hasCarryalls()){
            requestCarryall();
        } else{
            if(getItemID() == Unit_Harvester){
                doSetAttackMode(HARVEST);
            } else{
                doSetAttackMode(GUARD);
            }
        }

	}
}


void GroundUnit::playConfirmSound() {
	soundPlayer->playSound((Sound_enum) getRandomOf(2,Acknowledged,Affirmative));
}

void GroundUnit::playSelectSound() {
	soundPlayer->playSound(Reporting);
}

/**
    Request a Carryall to drop at target location
**/

void GroundUnit::doRequestCarryallDrop(int xPos, int yPos) {
    if(getOwner()->hasCarryalls() && !awaitingPickup){
        doMove2Pos(xPos, yPos, true);
        requestCarryall();
    }
}

bool GroundUnit::requestCarryall() {
	if (getOwner()->hasCarryalls() && !awaitingPickup)	{
		Carryall* carryall = NULL;

		/*
            This allows a unit to keep requesting a carryall even if one isn't
            available right now
		*/
		doSetAttackMode(CARRYALLREQUESTED);

        RobustList<UnitBase*>::const_iterator iter;
	    for(iter = unitList.begin(); iter != unitList.end(); ++iter) {
			UnitBase* unit = *iter;
			if ((unit->getOwner() == owner) && (unit->getItemID() == Unit_Carryall)) {
				if(!((Carryall*)unit)->isBooked()) {
					carryall = (Carryall*)unit;
					carryall->setTarget(this);
					carryall->clearPath();
					bookCarrier(carryall);

					//setDestination(&location);	//stop moving, and wait for carryall to arrive

					return true;
				}
			}
		}
	}

	return false;
}

void GroundUnit::setPickedUp(UnitBase* newCarrier) {
	UnitBase::setPickedUp(newCarrier);
	awaitingPickup = false;
	bookedCarrier = NONE;

	clearPath(); // Stefan: I don't think this is right
                 // but there is definitely something to it
                 // <try removing this to keep tanks moving even when a carryall is coming>
}

void GroundUnit::bookCarrier(UnitBase* newCarrier) {
    if(newCarrier == NULL) {
        bookedCarrier = NONE;
        awaitingPickup = false;
    } else {
        bookedCarrier = newCarrier->getObjectID();
        awaitingPickup = true;
    }
}

bool GroundUnit::hasBookedCarrier() const {
    if(bookedCarrier == NONE) {
        return false;
    } else {
        return (currentGame->getObjectManager().getObject(bookedCarrier) != NULL);
    }
}

const UnitBase* GroundUnit::getCarrier() const {
    return (UnitBase*) currentGame->getObjectManager().getObject(bookedCarrier);
}

void GroundUnit::navigate() {
	// Lets keep units moving even if they are awaiting a pickup
	// Could potentially make this distance based depending on how
	// far away the booked carrier is
	if(!awaitingPickup) {
		UnitBase::navigate();
    }
}



void GroundUnit::doRepair() {
	if(getHealth() < getMaxHealth()) {
		//find a repair yard to return to

		float	closestLeastBookedRepairYardDistance = 1000000.0f;
        RepairYard* bestRepairYard = NULL;

        RobustList<StructureBase*>::const_iterator iter;
        for(iter = structureList.begin(); iter != structureList.end(); ++iter) {
            StructureBase* tempStructure = *iter;

			if ((tempStructure->getItemID() == Structure_RepairYard) && (tempStructure->getOwner() == owner)) {
                RepairYard* tempRepairYard = ((RepairYard*)tempStructure);

                if(tempRepairYard->getNumBookings() == 0) {
                    float tempDistance = distanceFrom(location, tempRepairYard->getClosestPoint(location));
					if(tempDistance < closestLeastBookedRepairYardDistance) {
                        closestLeastBookedRepairYardDistance = tempDistance;
                        bestRepairYard = tempRepairYard;
                    }
                }
            }
        }

        if(bestRepairYard) {
            if((requestCarryall())) {
                doMove2Object(bestRepairYard);
            } else {
                doMove2Object(bestRepairYard);
            }
        }
	}
}
