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

#include <units/Carryall.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/SFXManager.h>
#include <House.h>
#include <Map.h>
#include <Game.h>
#include <SoundPlayer.h>

#include <structures/RepairYard.h>
#include <structures/Refinery.h>
#include <structures/ConstructionYard.h>
#include <units/Harvester.h>

Carryall::Carryall(House* newOwner) : AirUnit(newOwner)
{
    Carryall::init();

    setHealth(getMaxHealth());

    owned = true;

    aDropOfferer = false;
    droppedOffCargo = false;
    respondable = false;
}

Carryall::Carryall(InputStream& stream) : AirUnit(stream)
{
    Carryall::init();

    pickedUpUnitList = stream.readUint32List();
    if(!pickedUpUnitList.empty()) {
        drawnFrame = 1;
    }

    stream.readBools(&owned, &aDropOfferer, &droppedOffCargo);
}

void Carryall::init()
{
    itemID = Unit_Carryall;
    owner->incrementUnits(itemID);

    canAttackStuff = false;

    graphicID = ObjPic_Carryall;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    shadowGraphic = pGFXManager->getObjPic(ObjPic_CarryallShadow,getOwner()->getHouseID());

    numImagesX = NUM_ANGLES;
    numImagesY = 2;
}

Carryall::~Carryall()
{
    ;
}

void Carryall::save(OutputStream& stream) const
{
    AirUnit::save(stream);

    stream.writeUint32List(pickedUpUnitList);

    stream.writeBools(owned, aDropOfferer, droppedOffCargo);
}

bool Carryall::update() {
    const auto& maxSpeed = currentGame->objectData.data[itemID][originalHouseID].maxspeed;

    FixPoint dist = -1;
    const auto pTarget = target.getObjPointer();
    if(pTarget != nullptr && pTarget->isAUnit()) {
        dist = distanceFrom(realX, realY, pTarget->getRealX(), pTarget->getRealY());
    } else if((pTarget != nullptr) || hasCargo()) {
        dist = distanceFrom(realX, realY, destination.x*TILESIZE + TILESIZE/2, destination.y*TILESIZE + TILESIZE/2);
    }

    if(dist >= 0) {
        static const FixPoint minSpeed = FixPoint32(TILESIZE/32);
        if(dist < TILESIZE/2) {
            currentMaxSpeed = std::min(dist, minSpeed);
        } else if(dist >= 10*TILESIZE) {
            currentMaxSpeed = maxSpeed;
        } else {
            FixPoint m = (maxSpeed-minSpeed) / ((10*TILESIZE)-(TILESIZE/2));
            FixPoint t = minSpeed-(TILESIZE/2)*m;
            currentMaxSpeed = dist*m+t;
        }
    } else {
        currentMaxSpeed = std::min(currentMaxSpeed + 0.2_fix, maxSpeed);
    }

    if(AirUnit::update() == false) {
        return false;
    }

    // check if this carryall has to be removed because it has just brought something
    // to the map (e.g. new harvester)
    if (active) {
        if(aDropOfferer && droppedOffCargo && (hasCargo() == false)
            && ((getRealX() < -TILESIZE) || (getRealX() > (currentGameMap->getSizeX()+1)*TILESIZE)
                || (getRealY() < -TILESIZE) || (getRealY() > (currentGameMap->getSizeY()+1)*TILESIZE))) {
            setVisible(VIS_ALL, false);
            destroy();
            return false;
        }
    }
    return true;
}

void Carryall::deploy(const Coord& newLocation) {
    AirUnit::deploy(newLocation);

    respondable = false;
}

void Carryall::checkPos()
{
    AirUnit::checkPos();

    if (active) {
        if (hasCargo()) {
            if((location == destination) && (currentMaxSpeed <= 0.5_fix) ) {
                // drop up to 3 infantry units at once or one other unit
                int droppedUnits = 0;
                do {
                    Uint32 unitID = pickedUpUnitList.front();
                    UnitBase* pUnit = static_cast<UnitBase*>(currentGame->getObjectManager().getObject(unitID));

                    if(pUnit == nullptr) {
                        return;
                    }

                    if((pUnit != nullptr) && (pUnit->isInfantry() == false) && (droppedUnits > 0)) {
                        // we already dropped infantry and this is no infantry
                        // => do not drop this here
                        break;
                    }

                    deployUnit(unitID);
                    droppedUnits++;

                    if((pUnit != nullptr) && (pUnit->isInfantry() == false)) {
                        // we dropped a non infantry unit
                        // => do not drop another unit
                        break;
                    }
                } while(hasCargo() && (droppedUnits < 3));

                if(pickedUpUnitList.empty() == false) {
                    // find next place to drop
                    for(int i=8;i<18;i++) {
                        int r = currentGame->randomGen.rand(3,i/2);
                        FixPoint angle = 2 * FixPt_PI * currentGame->randomGen.randFixPoint();

                        Coord dropCoord = location + Coord( lround(r*FixPoint::sin(angle)), lround(-r*FixPoint::cos(angle)));
                        if(currentGameMap->tileExists(dropCoord) && currentGameMap->getTile(dropCoord)->hasAGroundObject() == false) {
                            setDestination(dropCoord);
                            break;
                        }
                    }
                } else {
                    setTarget(nullptr);
                    setDestination(guardPoint);
                }
            }
        } else if(isBooked() == false) {
            if(destination.isValid()) {
                if(blockDistance(location, destination) <= 2) {
                    destination.invalidate();
                }
            } else {
                if(blockDistance(location, guardPoint) > 17) {
                    setDestination(guardPoint);
                }
            }
        }
    }

}

void Carryall::deployUnit(Uint32 unitID)
{
    bool found = false;

    for(const Uint32& pickedUpUnitID : pickedUpUnitList) {
        if(pickedUpUnitID == unitID) {
            found = true;
            break;
        }
    }

    if(found == false) {
        return;
    }

    pickedUpUnitList.remove(unitID);

    soundPlayer->playSoundAt(Sound_Drop, location);

    UnitBase* pUnit = static_cast<UnitBase*>(currentGame->getObjectManager().getObject(unitID));

    if(pUnit == nullptr) {
        return;
    }

    if (found) {
        currentMaxSpeed = 0;
        setSpeeds();

        if (currentGameMap->getTile(location)->hasANonInfantryGroundObject()) {
            ObjectBase* object = currentGameMap->getTile(location)->getNonInfantryGroundObject();
            if (object->getOwner() == getOwner()) {
                if (object->getItemID() == Structure_RepairYard) {
                    if (static_cast<RepairYard*>(object)->isFree()) {
                        pUnit->setTarget(object);   // unit books repair yard again
                        pUnit->setGettingRepaired();
                        pUnit = nullptr;
                    } else {
                        // unit is still going to repair yard but was unbooked from repair yard at pickup => book now
                        static_cast<RepairYard*>(object)->book();
                    }
                } else if ((object->getItemID() == Structure_Refinery) && (pUnit->getItemID() == Unit_Harvester)) {
                    if (static_cast<Refinery*>(object)->isFree()) {
                        static_cast<Harvester*>(pUnit)->setTarget(object);
                        static_cast<Harvester*>(pUnit)->setReturned();
                        pUnit = nullptr;
                        goingToRepairYard = false;
                    }
                }
            }
        }

        if(pUnit != nullptr) {
            pUnit->setAngle(drawnAngle);
            Coord deployPos = currentGameMap->findDeploySpot(pUnit, location, currentGame->randomGen);
            pUnit->setForced(false); // Stop units being forced if they are deployed
            pUnit->deploy(deployPos);
            if(pUnit->getItemID() == Unit_Saboteur) {
                pUnit->doSetAttackMode(HUNT);
            } else if(pUnit->getItemID() != Unit_Harvester) {
                pUnit->doSetAttackMode(AREAGUARD);
            } else {
                pUnit->doSetAttackMode(HARVEST);
            }
        }

        if (pickedUpUnitList.empty()) {
            if(!aDropOfferer) {
                setTarget(nullptr);
                setDestination(guardPoint);
            }
            droppedOffCargo = true;
            drawnFrame = 0;

            clearPath();
        }
    }
}

void Carryall::destroy()
{
    // destroy cargo
    for(const Uint32& pickedUpUnitID : pickedUpUnitList) {
        UnitBase* pPickedUpUnit = static_cast<UnitBase*>(currentGame->getObjectManager().getObject(pickedUpUnitID));
        if(pPickedUpUnit != nullptr) {
            pPickedUpUnit->destroy();
        }
    }
    pickedUpUnitList.clear();

    // place wreck
    if(isVisible() && currentGameMap->tileExists(location)) {
        Tile* pTile = currentGameMap->getTile(location);
        pTile->assignDeadUnit(DeadUnit_Carrall, owner->getHouseID(), Coord(lround(realX), lround(realY)));
    }

    AirUnit::destroy();
}

void Carryall::releaseTarget() {
    setTarget(nullptr);

    if(!hasCargo()) {
        setDestination(guardPoint);
    }
}

void Carryall::engageTarget()
{
    if(target && (target.getObjPointer() == nullptr)) {
        // the target does not exist anymore
        releaseTarget();
        return;
    }

    if(target && (target.getObjPointer()->isActive() == false)) {
        // the target changed its state to inactive
        releaseTarget();
        return;
    }

    if(target && target.getObjPointer()->isAGroundUnit() && !static_cast<GroundUnit*>(target.getObjPointer())->isAwaitingPickup()) {
        // the target changed its state to not awaiting pickup anymore
        releaseTarget();
        return;
    }

    if(target && (target.getObjPointer()->getOwner()->getTeamID() != owner->getTeamID())) {
        // the target changed its owner (e.g. was deviated)
        releaseTarget();
        return;
    }

    Coord targetLocation;
    if(target.getObjPointer()->getItemID() == Structure_Refinery) {
        targetLocation = target.getObjPointer()->getLocation() + Coord(2,0);
    } else {
        targetLocation = target.getObjPointer()->getClosestPoint(location);
    }

    Coord realLocation = Coord(lround(realX), lround(realY));
    Coord realDestination = targetLocation * TILESIZE + Coord(TILESIZE/2,TILESIZE/2);

    targetDistance = distanceFrom(realLocation, realDestination);

    if (targetDistance <= TILESIZE/32) {
        if(hasCargo()) {
            if(target.getObjPointer()->isAStructure()) {
                while(pickedUpUnitList.begin() != pickedUpUnitList.end()) {
                    deployUnit(*(pickedUpUnitList.begin()) );
                }

                setTarget(nullptr);
                setDestination(guardPoint);
            }
        } else {
            pickupTarget();
        }
    } else {
        setDestination(targetLocation);
    }
}

void Carryall::giveCargo(UnitBase* newUnit)
{
    if(newUnit == nullptr) {
        return;
    }

    pickedUpUnitList.push_back(newUnit->getObjectID());

    newUnit->setPickedUp(this);

    drawnFrame = 1;

    droppedOffCargo = false;
}

void Carryall::pickupTarget()
{
    currentMaxSpeed = 0;
    setSpeeds();

    ObjectBase* pTarget = target.getObjPointer();

    if(pTarget->isAGroundUnit()) {
        GroundUnit* pGroundUnitTarget = static_cast<GroundUnit*>(pTarget);

        if(pTarget->getHealth() <= 0) {
            // unit died just in the moment we tried to pick it up => carryall also crushes
            setHealth(0);
            return;
        }

        if (  pGroundUnitTarget->hasATarget()
            || ( pGroundUnitTarget->getDestination() != pGroundUnitTarget->getLocation())
            || pGroundUnitTarget->isBadlyDamaged()) {

            if(pGroundUnitTarget->isBadlyDamaged() || (pGroundUnitTarget->hasATarget() == false && pGroundUnitTarget->getItemID() != Unit_Harvester))   {
                pGroundUnitTarget->doRepair();
            }

            ObjectBase* newTarget = pGroundUnitTarget->hasATarget() ? pGroundUnitTarget->getTarget() : nullptr;

            pickedUpUnitList.push_back(target.getObjectID());
            pGroundUnitTarget->setPickedUp(this);

            drawnFrame = 1;

            if(newTarget && (newTarget->getItemID() == Structure_Refinery)) {
                pGroundUnitTarget->setGuardPoint(pGroundUnitTarget->getLocation());
                setTarget(newTarget);
                setDestination(target.getObjPointer()->getLocation() + Coord(2,0));
            } else if(newTarget && (newTarget->getItemID() == Structure_RepairYard)) {
                pGroundUnitTarget->setGuardPoint(pGroundUnitTarget->getLocation());
                setTarget(newTarget);
                setDestination(target.getObjPointer()->getClosestPoint(location));
            } else if (pGroundUnitTarget->getDestination().isValid()) {
                setDestination(pGroundUnitTarget->getDestination());
            }

            clearPath();

        } else {
            pGroundUnitTarget->setAwaitingPickup(false);
            if(pGroundUnitTarget->getAttackMode() == CARRYALLREQUESTED) {
                pGroundUnitTarget->doSetAttackMode(STOP);
            }
            releaseTarget();
        }
    } else {
        // get unit from structure
        ObjectBase* pObject = target.getObjPointer();
        if(pObject->getItemID() == Structure_Refinery) {
            // get harvester
            static_cast<Refinery*>(pObject)->deployHarvester(this);
        } else if(pObject->getItemID() == Structure_RepairYard) {
            // get repaired unit
            static_cast<RepairYard*>(pObject)->deployRepairUnit(this);
        }
    }
}

void Carryall::setTarget(const ObjectBase* newTarget) {
    if(target.getObjPointer() != nullptr
        && targetFriendly
        && target.getObjPointer()->isAGroundUnit()
        && (static_cast<GroundUnit*>(target.getObjPointer())->getCarrier() == this))
    {
        static_cast<GroundUnit*>(target.getObjPointer())->bookCarrier(nullptr);
    }

    if(target.getObjPointer() != nullptr && (target.getObjPointer()->getItemID() == Structure_Refinery)) {
        static_cast<Refinery*>(target.getObjPointer())->unBook();
    }

    UnitBase::setTarget(newTarget);

    if(target.getObjPointer() != nullptr && (target.getObjPointer()->getItemID() == Structure_Refinery))
    {
        static_cast<Refinery*>(target.getObjPointer())->book();
    }

    if(target && targetFriendly && target.getObjPointer()->isAGroundUnit()) {
        static_cast<GroundUnit*>(target.getObjPointer())->setAwaitingPickup(true);
    }
}

void Carryall::targeting() {
    if(target) {
        engageTarget();
    }
}

void Carryall::turn() {
    if (active && aDropOfferer && droppedOffCargo && (hasCargo() == false)
        && ((getRealX() < TILESIZE/2) || (getRealX() > currentGameMap->getSizeX()*TILESIZE - TILESIZE/2)
            || (getRealY() < TILESIZE/2) || (getRealY() > currentGameMap->getSizeY()*TILESIZE - TILESIZE/2))) {
        // already partially outside the map => do not turn
        return;
    }

    AirUnit::turn();
}
