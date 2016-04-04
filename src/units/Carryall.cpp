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

	booked = false;
    idle = true;
	firstRun = true;
    owned = true;

    aDropOfferer = false;
    droppedOffCargo = false;
    respondable = false;

    currentMaxSpeed = 2;

	curFlyPoint = 0;
	for(int i=0; i < 8; i++) {
		flyPoints[i].invalidate();
	}
	constYardPoint.invalidate();
}

Carryall::Carryall(InputStream& stream) : AirUnit(stream)
{
    Carryall::init();

	pickedUpUnitList = stream.readUint32List();
	if(!pickedUpUnitList.empty()) {
		drawnFrame = 1;
	}

    stream.readBools(&booked, &idle, &firstRun, &owned, &aDropOfferer, &droppedOffCargo);

	currentMaxSpeed = stream.readFixPoint();

	curFlyPoint = stream.readUint8();
	for(int i=0; i < 8; i++) {
		flyPoints[i].x = stream.readSint32();
		flyPoints[i].y = stream.readSint32();
	}
	constYardPoint.x = stream.readSint32();
	constYardPoint.y = stream.readSint32();
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

    stream.writeBools(booked, idle, firstRun, owned, aDropOfferer, droppedOffCargo);

	stream.writeFixPoint(currentMaxSpeed);

	stream.writeUint8(curFlyPoint);
	for(int i=0; i < 8; i++) {
		stream.writeSint32(flyPoints[i].x);
		stream.writeSint32(flyPoints[i].y);
	}
	stream.writeSint32(constYardPoint.x);
	stream.writeSint32(constYardPoint.y);
}

bool Carryall::update() {

    if(AirUnit::update() == false) {
        return false;
    }

    FixPoint dist = distanceFrom(location.x*TILESIZE + TILESIZE/2, location.y*TILESIZE + TILESIZE/2,
                                destination.x*TILESIZE + TILESIZE/2, destination.y*TILESIZE + TILESIZE/2);

    if((target || hasCargo()) && dist < 256) {
        currentMaxSpeed = (((2 - currentGame->objectData.data[itemID][originalHouseID].maxspeed)/256) * (256 - dist)) + currentGame->objectData.data[itemID][originalHouseID].maxspeed;
        setSpeeds();
    } else {
        currentMaxSpeed = std::min(currentMaxSpeed + FixPt(0,2), currentGame->objectData.data[itemID][originalHouseID].maxspeed);
        setSpeeds();
    }

	// check if this carryall has to be removed because it has just brought something
	// to the map (e.g. new harvester)
	if (active)	{
		if(aDropOfferer && droppedOffCargo && (hasCargo() == false)
            && (    (location.x == 0) || (location.x == currentGameMap->getSizeX()-1)
                    || (location.y == 0) || (location.y == currentGameMap->getSizeY()-1))
            && !moving)	{

            setVisible(VIS_ALL, false);
            destroy();
            return false;
		}
	}
	return true;
}

FixPoint Carryall::getMaxSpeed() const {
    return currentMaxSpeed;
}

void Carryall::deploy(const Coord& newLocation) {
	AirUnit::deploy(newLocation);

	respondable = false;
}

void Carryall::checkPos()
{
	AirUnit::checkPos();

	if (active)	{
		if (hasCargo() && (location == destination) && (distanceFrom(realX, realY, destination.x * TILESIZE + (TILESIZE/2), destination.y * TILESIZE + (TILESIZE/2)) < TILESIZE/8) ) {
		    // drop up to 3 infantry units at once or one other unit
            int droppedUnits = 0;
            do {
                Uint32 unitID = pickedUpUnitList.front();
                UnitBase* pUnit = (UnitBase*) (currentGame->getObjectManager().getObject(unitID));

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

                idle = true;
            }
		} else if((isBooked() == false) && idle && !firstRun) {
			//fly around const yard
			Coord point = this->getClosestPoint(location);

			if(point == guardPoint) {
				//arrived at point, move to next
				curFlyPoint++;

				if(curFlyPoint >= 8) {
					curFlyPoint = 0;
				}

                int looped = 0;
				while(!(currentGameMap->tileExists(flyPoints[curFlyPoint].x, flyPoints[curFlyPoint].y)) && looped <= 2) {
					curFlyPoint++;

					if(curFlyPoint >= 8) {
						curFlyPoint = 0;
						looped++;
					}
				}

				setGuardPoint(flyPoints[curFlyPoint]);
				setDestination(guardPoint);
			}
		} else if(firstRun && owned) {
			findConstYard();
			setGuardPoint(constYardPoint);
			setDestination(guardPoint);
			firstRun = false;
		}
	}

}

void Carryall::deployUnit(Uint32 unitID)
{
	bool found = false;

	std::list<Uint32>::iterator iter;
	for(iter = pickedUpUnitList.begin() ; iter != pickedUpUnitList.end(); ++iter) {
		if(*iter == unitID) {
			found = true;
			break;
		}
	}

	if(found == false) {
        return;
	}

	pickedUpUnitList.remove(unitID);

	soundPlayer->playSoundAt(Sound_Drop, location);

	UnitBase* pUnit = (UnitBase*) (currentGame->getObjectManager().getObject(unitID));

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
					if (((RepairYard*)object)->isFree()) {
						pUnit->setTarget(object);
						pUnit->setGettingRepaired();
						pUnit = nullptr;
					} else {
					    // carryall has booked this repair yard but now will not go there => unbook
						((RepairYard*)object)->unBook();

						// unit is still going to repair yard but was unbooked from repair yard at pickup => book now
						((RepairYard*)object)->book();
					}
				} else if ((object->getItemID() == Structure_Refinery) && (pUnit->getItemID() == Unit_Harvester)) {
					if (((Refinery*)object)->isFree()) {
						((Harvester*)pUnit)->setTarget(object);
						((Harvester*)pUnit)->setReturned();
						pUnit = nullptr;
						goingToRepairYard = false;
					}
				}
			}
		}

		if(pUnit != nullptr) {
			pUnit->setAngle(drawnAngle);
			Coord deployPos = currentGameMap->findDeploySpot(pUnit, location);
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
				booked = false;
                idle = true;
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
	std::list<Uint32>::const_iterator iter;
	for(iter = pickedUpUnitList.begin() ; iter != pickedUpUnitList.end(); ++iter) {
		UnitBase* pUnit = (UnitBase*) (currentGame->getObjectManager().getObject(*iter));
		if(pUnit != nullptr) {
			pUnit->destroy();
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
        booked = false;
        idle = true;
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

    if(target && target.getObjPointer()->isAGroundUnit() && !((GroundUnit*)target.getObjPointer())->isawaitingPickup()) {
        // the target changed its state to not awaiting pickup anymore
        releaseTarget();
        return;
    }

    if(target && (target.getObjPointer()->getOwner()->getTeam() != owner->getTeam())) {
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

    targetAngle = destinationDrawnAngle(location, destination);

    targetDistance = distanceFrom(realLocation, realDestination);

    if (targetDistance <= TILESIZE/8) {
        if (target.getObjPointer()->isAUnit()) {
            targetAngle = ((GroundUnit*)target.getObjPointer())->getAngle();
        }

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

	booked = true;
	pickedUpUnitList.push_back(newUnit->getObjectID());

	newUnit->setPickedUp(this);

	if (getItemID() != Unit_Frigate)
		drawnFrame = 1;

	droppedOffCargo = false;
}

void Carryall::pickupTarget()
{
    currentMaxSpeed = 0;
    setSpeeds();

    ObjectBase* pTarget = target.getObjPointer();

	if(pTarget->isAGroundUnit()) {
        GroundUnit* pGroundUnitTarget = dynamic_cast<GroundUnit*>(pTarget);

        if(pTarget->getHealth() <= 0) {
            // unit died just in the moment we tried to pick it up => carryall also crushes
            setHealth(0);
            return;
        }

		if (  pTarget->hasATarget()
			|| ( pGroundUnitTarget->getGuardPoint() != pTarget->getLocation())
			|| pGroundUnitTarget->isBadlyDamaged())	{

			if(pGroundUnitTarget->isBadlyDamaged() || (pTarget->hasATarget() == false && pTarget->getItemID() != Unit_Harvester))	{
				pGroundUnitTarget->doRepair();
			}

			ObjectBase* newTarget = pGroundUnitTarget->hasATarget() ? pGroundUnitTarget->getTarget() : nullptr;

			pickedUpUnitList.push_back(target.getObjectID());
			pGroundUnitTarget->setPickedUp(this);

			drawnFrame = 1;
			booked = true;

            if(newTarget && ((newTarget->getItemID() == Structure_Refinery)
                              || (newTarget->getItemID() == Structure_RepairYard)))
            {
                setTarget(newTarget);
                if(newTarget->getItemID() == Structure_Refinery) {
                    setDestination(target.getObjPointer()->getLocation() + Coord(2,0));
                } else {
                    setDestination(target.getObjPointer()->getClosestPoint(location));
                }
            } else if (pGroundUnitTarget->getDestination().isValid()) {
                setDestination(pGroundUnitTarget->getDestination());
            }

            clearPath();

		} else {
			pGroundUnitTarget->setawaitingPickup(false);
			releaseTarget();
		}
	} else {
        // get unit from structure
        ObjectBase* pObject = target.getObjPointer();
        if(pObject->getItemID() == Structure_Refinery) {
            // get harvester
            ((Refinery*) pObject)->deployHarvester(this);
        } else if(pObject->getItemID() == Structure_RepairYard) {
            // get repaired unit
            ((RepairYard*) pObject)->deployRepairUnit(this);
        }
	}
}

void Carryall::setTarget(const ObjectBase* newTarget) {
	if(target.getObjPointer() != nullptr
		&& targetFriendly
		&& target.getObjPointer()->isAGroundUnit()
		&& (((GroundUnit*)target.getObjPointer())->getCarrier() == this))
	{
		((GroundUnit*)target.getObjPointer())->bookCarrier(nullptr);
	}

	UnitBase::setTarget(newTarget);

	if(target && targetFriendly && target.getObjPointer()->isAGroundUnit()) {
		((GroundUnit*)target.getObjPointer())->setawaitingPickup(true);
	}

	booked = target;
}

void Carryall::targeting() {
	if(target) {
		engageTarget();
	}
}


void Carryall::findConstYard() {
    FixPoint closestYardDistance = 1000000;
    ConstructionYard* bestYard = nullptr;

    RobustList<StructureBase*>::const_iterator iter;
    for(iter = structureList.begin(); iter != structureList.end(); ++iter) {
        StructureBase* tempStructure = *iter;

        if((tempStructure->getItemID() == Structure_ConstructionYard) && (tempStructure->getOwner() == owner)) {
            ConstructionYard* tempYard = ((ConstructionYard*) tempStructure);
            Coord closestPoint = tempYard->getClosestPoint(location);
            FixPoint tempDistance = distanceFrom(location, closestPoint);

            if(tempDistance < closestYardDistance) {
                closestYardDistance = tempDistance;
                bestYard = tempYard;
            }
        }
    }

    if(bestYard) {
        constYardPoint = bestYard->getClosestPoint(location);
    } else {
        constYardPoint = guardPoint;
    }

    // make all circles a bit different
    constYardPoint.x += (getObjectID() % 5) - 2;
    constYardPoint.y += ((getObjectID() >> 2) % 5) - 2;

    // stay on map
    constYardPoint.x = std::min(currentGameMap->getSizeX()-11, std::max(constYardPoint.x, 9));
    constYardPoint.y = std::min(currentGameMap->getSizeY()-11, std::max(constYardPoint.y, 9));

    static const Coord circles[][8] = {
                                        { Coord(-2,-6), Coord(3,-6), Coord(7,-2), Coord(7,3), Coord(3,7), Coord(-2,7), Coord(-6,3), Coord(-6,-2) },
                                        { Coord(-2,-6), Coord(-6,-2), Coord(-6,3), Coord(-2,7), Coord(3,7), Coord(7,3), Coord(7,-2), Coord(3,-6) },
                                        { Coord(-3,-8), Coord(4,-8), Coord(9,-3), Coord(9,4), Coord(4,9), Coord(-3,9), Coord(-8,4), Coord(-8,-3) },
                                        { Coord(-3,-8), Coord(-8,-3), Coord(-8,4), Coord(-3,9), Coord(4,9), Coord(9,4), Coord(9,-3), Coord(4,-8) }
                                      };

    const Coord* pUsedCircle = circles[currentGame->randomGen.rand(0,3)];


    for(int i=0;i<8; i++) {
        flyPoints[i] = constYardPoint + pUsedCircle[i];
    }
}
