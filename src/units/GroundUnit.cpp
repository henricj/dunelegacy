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

#include <players/HumanPlayer.h>

#include <structures/RepairYard.h>
#include <units/Carryall.h>

GroundUnit::GroundUnit(House* newOwner) : UnitBase(newOwner) {

    GroundUnit::init();

    awaitingPickup = false;
    bookedCarrier = NONE_ID;
}

GroundUnit::GroundUnit(InputStream& stream) : UnitBase(stream) {

    GroundUnit::init();

    awaitingPickup = stream.readBool();
    bookedCarrier = stream.readUint32();
}

void GroundUnit::init() {
    aGroundUnit = true;
}

GroundUnit::~GroundUnit() = default;

void GroundUnit::save(OutputStream& stream) const {
    UnitBase::save(stream);

    stream.writeBool(awaitingPickup);
    stream.writeUint32(bookedCarrier);
}

void GroundUnit::assignToMap(const Coord& pos) {
    if (currentGameMap->tileExists(pos)) {
        currentGameMap->getTile(pos)->assignNonInfantryGroundObject(getObjectID());
        currentGameMap->viewMap(owner->getHouseID(), pos, getViewRange());
    }
}

void GroundUnit::checkPos() {
    auto* pTile = currentGameMap->getTile(location);
    if(!moving && !justStoppedMoving && !isInfantry()) {
        pTile->setTrack(drawnAngle);
    }

    if(justStoppedMoving)
    {
        realX = location.x*TILESIZE + TILESIZE/2;
        realY = location.y*TILESIZE + TILESIZE/2;
        //findTargetTimer = 0;  //allow a scan for new targets now

        if(pTile->isSpiceBloom()) {
            setHealth(0);
            setVisible(VIS_ALL, false);
            pTile->triggerSpiceBloom(getOwner());
        } else if(pTile->isSpecialBloom()){
            pTile->triggerSpecialBloom(getOwner());
        }
    }

    /*
        Go to repair yard if low on health
    */
    if(active && (getHealth() < getMaxHealth()/2)
            && !goingToRepairYard
            && owner->hasRepairYard()
            && !pickedUp
            && owner->hasCarryalls()
            && owner->getHouseID() == originalHouseID // stop deviated units from being repaired
            && !isInfantry()
            && !forced ) { // Stefan - Allow units with targets to be picked up for repairs

        doRepair();

    }


    if(goingToRepairYard) {
        if(target.getObjPointer() == nullptr) {
            goingToRepairYard = false;
            awaitingPickup = false;
            bookedCarrier = NONE_ID;

            clearPath();
        } else {
            ObjectBase *pObject = pTile->getGroundObject();

            if( justStoppedMoving
                && (pObject != nullptr)
                && (pObject->getObjectID() == target.getObjectID())
                && (target.getObjPointer()->getItemID() == Structure_RepairYard))
            {
                RepairYard* pRepairYard = static_cast<RepairYard*>(target.getObjPointer());
                if(pRepairYard->isFree()) {
                    setGettingRepaired();
                } else {
                    // the repair yard is already in use by some other unit => move out
                    Coord newDestination = currentGameMap->findDeploySpot(this, target.getObjPointer()->getLocation(), currentGame->randomGen, getLocation(), pRepairYard->getStructureSize());
                    doMove2Pos(newDestination, true);
                }
            }
        }
    }

    // If we are awaiting a pickup try book a carryall if we have one
    if(!pickedUp && attackMode == CARRYALLREQUESTED && bookedCarrier == NONE_ID) {
        if(getOwner()->hasCarryalls() && (target || (destination != location))) {
            requestCarryall();
        } else {
            if(getItemID() == Unit_Harvester) {
                doSetAttackMode(HARVEST);
            } else {
                doSetAttackMode(GUARD);
            }
        }
    }
}


void GroundUnit::playConfirmSound() {
    soundPlayer->playVoice(getRandomOf({Acknowledged,Affirmative}), getOwner()->getHouseID());
}

void GroundUnit::playSelectSound() {
    soundPlayer->playVoice(Reporting, getOwner()->getHouseID());
}

/**
    Request a Carryall to drop at target location
**/

void GroundUnit::doRequestCarryallDrop(int xPos, int yPos) {
    if(getOwner()->hasCarryalls() && !awaitingPickup && currentGameMap->tileExists(xPos, yPos)){
        doMove2Pos(xPos, yPos, true);
        requestCarryall();
    }
}

bool GroundUnit::requestCarryall() {
    if (getOwner()->hasCarryalls() && !awaitingPickup)  {
        Carryall* carryall = nullptr;

        // This allows a unit to keep requesting a carryall even if one isn't available right now
        doSetAttackMode(CARRYALLREQUESTED);

        for(UnitBase* pUnit : unitList) {
            if ((pUnit->getOwner() == owner) && (pUnit->getItemID() == Unit_Carryall)) {
                if(!static_cast<Carryall*>(pUnit)->isBooked()) {
                    carryall = static_cast<Carryall*>(pUnit);
                    carryall->setTarget(this);
                    carryall->clearPath();
                    bookCarrier(carryall);

                    //setDestination(&location);    //stop moving, and wait for carryall to arrive

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
    bookedCarrier = NONE_ID;

    clearPath(); // Stefan: I don't think this is right
                 // but there is definitely something to it
                 // <try removing this to keep tanks moving even when a carryall is coming>
}

void GroundUnit::bookCarrier(UnitBase* newCarrier) {
    if(newCarrier == nullptr) {
        bookedCarrier = NONE_ID;
        awaitingPickup = false;
    } else {
        bookedCarrier = newCarrier->getObjectID();
        awaitingPickup = true;
    }
}

bool GroundUnit::hasBookedCarrier() const {
    if(bookedCarrier == NONE_ID) {
        return false;
    } else {
        return (currentGame->getObjectManager().getObject(bookedCarrier) != nullptr);
    }
}

const UnitBase* GroundUnit::getCarrier() const {
    return static_cast<UnitBase*>(currentGame->getObjectManager().getObject(bookedCarrier));
}

void GroundUnit::move() {
    if(!moving && !justStoppedMoving && (((currentGame->getGameCycleCount() + getObjectID()) % 512) == 0)) {
        currentGameMap->viewMap(owner->getHouseID(), location, getViewRange());
    }

    UnitBase::move();
}

void GroundUnit::navigate() {
    // Lets keep units moving even if they are awaiting a pickup
    // Could potentially make this distance based depending on how
    // far away the booked carrier is
    if(!awaitingPickup) {
        UnitBase::navigate();
    }
}

void GroundUnit::handleSendToRepairClick() {
    currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_UNIT_SENDTOREPAIR,objectID));
}

void GroundUnit::doRepair() {
    if(getHealth() < getMaxHealth()) {
        //find a repair yard to return to

        FixPoint closestLeastBookedRepairYardDistance = 1000000;
        RepairYard* pBestRepairYard = nullptr;

        for(StructureBase* pStructure : structureList) {
            if ((pStructure->getItemID() == Structure_RepairYard) && (pStructure->getOwner() == owner)) {
                RepairYard* pRepairYard = static_cast<RepairYard*>(pStructure);

                if(pRepairYard->getNumBookings() == 0) {
                    FixPoint tempDistance = blockDistance(location, pRepairYard->getClosestPoint(location));
                    if(tempDistance < closestLeastBookedRepairYardDistance) {
                        closestLeastBookedRepairYardDistance = tempDistance;
                        pBestRepairYard = pRepairYard;
                    }
                }
            }
        }

        if(pBestRepairYard) {
            requestCarryall();
            doMove2Object(pBestRepairYard);
        }
    }
}
