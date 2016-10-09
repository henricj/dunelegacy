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

#include <units/UnitBase.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>

#include <SoundPlayer.h>
#include <Map.h>
#include <Bullet.h>
#include <ScreenBorder.h>
#include <House.h>

#include <players/HumanPlayer.h>

#include <misc/draw_util.h>

#include <AStarSearch.h>

#include <GUI/ObjectInterfaces/UnitInterface.h>

#include <structures/Refinery.h>
#include <structures/RepairYard.h>
#include <units/Harvester.h>

#define SMOKEDELAY 30
#define UNITIDLETIMER (GAMESPEED_DEFAULT *  315)  // about every 5s

UnitBase::UnitBase(House* newOwner) : ObjectBase(newOwner) {

    UnitBase::init();

    drawnAngle = currentGame->randomGen.rand(0, 7);
    angle = drawnAngle;

    goingToRepairYard = false;
    pickedUp = false;
    bFollow = false;
    guardPoint = Coord::Invalid();
    attackPos = Coord::Invalid();

    moving = false;
    turning = false;
    justStoppedMoving = false;
    xSpeed = 0;
    ySpeed = 0;
    bumpyOffsetX = 0;
    bumpyOffsetY = 0;

    targetDistance = 0;
    targetAngle = INVALID;

    noCloserPointCount = 0;
    nextSpotFound = false;
    nextSpotAngle = drawnAngle;
    recalculatePathTimer = 0;
    nextSpot = Coord::Invalid();

    findTargetTimer = 0;
    primaryWeaponTimer = 0;
    secondaryWeaponTimer = INVALID;

    deviationTimer = INVALID;
}

UnitBase::UnitBase(InputStream& stream) : ObjectBase(stream) {

    UnitBase::init();

    stream.readBools(&goingToRepairYard, &pickedUp, &bFollow);
    guardPoint.x = stream.readSint32();
    guardPoint.y = stream.readSint32();
    attackPos.x = stream.readSint32();
    attackPos.y = stream.readSint32();

    stream.readBools(&moving, &turning, &justStoppedMoving);
    xSpeed = stream.readFixPoint();
    ySpeed = stream.readFixPoint();
    bumpyOffsetX = stream.readFixPoint();
    bumpyOffsetY = stream.readFixPoint();

    targetDistance = stream.readFixPoint();
    targetAngle = stream.readSint8();

    noCloserPointCount = stream.readUint8();
    nextSpotFound = stream.readBool();
    nextSpotAngle = stream.readSint8();
    recalculatePathTimer = stream.readSint32();
    nextSpot.x = stream.readSint32();
    nextSpot.y = stream.readSint32();
    int numPathNodes = stream.readUint32();
    for(int i=0;i<numPathNodes; i++) {
        Sint32 x = stream.readSint32();
        Sint32 y = stream.readSint32();
        pathList.push_back(Coord(x,y));
    }

    findTargetTimer = stream.readSint32();
    primaryWeaponTimer = stream.readSint32();
    secondaryWeaponTimer = stream.readSint32();

    deviationTimer = stream.readSint32();
}

void UnitBase::init() {
    aUnit = true;
    canAttackStuff = true;

    tracked = false;
    turreted = false;
    numWeapons = 0;

    drawnFrame = 0;

    unitList.push_back(this);
}

UnitBase::~UnitBase() {
    pathList.clear();
    removeFromSelectionLists();

    currentGame->getObjectManager().removeObject(objectID);
}


void UnitBase::save(OutputStream& stream) const {

    ObjectBase::save(stream);

    stream.writeBools(goingToRepairYard, pickedUp, bFollow);
    stream.writeSint32(guardPoint.x);
    stream.writeSint32(guardPoint.y);
    stream.writeSint32(attackPos.x);
    stream.writeSint32(attackPos.y);

    stream.writeBools(moving, turning, justStoppedMoving);
    stream.writeFixPoint(xSpeed);
    stream.writeFixPoint(ySpeed);
    stream.writeFixPoint(bumpyOffsetX);
    stream.writeFixPoint(bumpyOffsetY);

    stream.writeFixPoint(targetDistance);
    stream.writeSint8(targetAngle);

    stream.writeUint8(noCloserPointCount);
    stream.writeBool(nextSpotFound);
    stream.writeSint8(nextSpotAngle);
    stream.writeSint32(recalculatePathTimer);
    stream.writeSint32(nextSpot.x);
    stream.writeSint32(nextSpot.y);
    stream.writeUint32(pathList.size());
    for(std::list<Coord>::const_iterator iter = pathList.begin(); iter != pathList.end(); ++iter) {
        stream.writeSint32(iter->x);
        stream.writeSint32(iter->y);
    }

    stream.writeSint32(findTargetTimer);
    stream.writeSint32(primaryWeaponTimer);
    stream.writeSint32(secondaryWeaponTimer);

    stream.writeSint32(deviationTimer);
}

void UnitBase::attack() {

    if(numWeapons) {
        Coord targetCenterPoint;
        Coord centerPoint = getCenterPoint();
        bool bAirBullet;

        if(target.getObjPointer() != nullptr) {
            targetCenterPoint = target.getObjPointer()->getClosestCenterPoint(location);
            bAirBullet = target.getObjPointer()->isAFlyingUnit();
        } else {
            targetCenterPoint = currentGameMap->getTile(attackPos)->getCenterPoint();
            bAirBullet = false;
        }

        int currentBulletType = bulletType;
        Sint32 currentWeaponDamage = currentGame->objectData.data[itemID][originalHouseID].weapondamage;

        if(getItemID() == Unit_Trooper) {
            // Troopers change weapon type depending on distance

            FixPoint distance = distanceFrom(centerPoint, targetCenterPoint);
            if(distance > 2*TILESIZE) {
                currentBulletType = Bullet_SmallRocket;
            }
        } else if(getItemID() == Unit_Launcher && bAirBullet){
            // Launchers change weapon type when targeting flying units
            currentBulletType = Bullet_TurretRocket;

        }

        if(primaryWeaponTimer == 0) {
            bulletList.push_back( new Bullet( objectID, &centerPoint, &targetCenterPoint, currentBulletType, currentWeaponDamage, bAirBullet) );
            playAttackSound();
            primaryWeaponTimer = getWeaponReloadTime();

            secondaryWeaponTimer = 15;

            if(attackPos && getItemID() != Unit_SonicTank && currentGameMap->getTile(attackPos)->isSpiceBloom()) {
                setDestination(location);
                forced = false;
                attackPos.invalidate();
            }

            // shorten deviation time
            if(deviationTimer > 0) {
                deviationTimer = std::max(0,deviationTimer - MILLI2CYCLES(20*1000));
            }
        }

        if((numWeapons == 2) && (secondaryWeaponTimer == 0) && (isBadlyDamaged() == false)) {
            bulletList.push_back( new Bullet( objectID, &centerPoint, &targetCenterPoint, currentBulletType, currentWeaponDamage, bAirBullet) );
            playAttackSound();
            secondaryWeaponTimer = -1;

            if(attackPos && getItemID() != Unit_SonicTank && currentGameMap->getTile(attackPos)->isSpiceBloom()) {
                setDestination(location);
                forced = false;
                attackPos.invalidate();
            }

            // shorten deviation time
            if(deviationTimer > 0) {
                deviationTimer = std::max(0,deviationTimer - MILLI2CYCLES(20*1000));
            }
        }
    }
}

void UnitBase::blitToScreen() {
    int x = screenborder->world2screenX(realX);
    int y = screenborder->world2screenY(realY);

    SDL_Texture* pUnitGraphic = graphic[currentZoomlevel];
    SDL_Rect source = calcSpriteSourceRect(pUnitGraphic, drawnAngle, numImagesX, drawnFrame, numImagesY);
    SDL_Rect dest = calcSpriteDrawingRect( pUnitGraphic, x, y, numImagesX, numImagesY, HAlign::Center, VAlign::Center);

    SDL_RenderCopy(renderer, pUnitGraphic, &source, &dest);

    if(isBadlyDamaged()) {
        drawSmoke(x, y);
    }
}

ObjectInterface* UnitBase::getInterfaceContainer() {
    if((pLocalHouse == owner && isRespondable()) || (debug == true)) {
        return UnitInterface::create(objectID);
    } else {
        return DefaultObjectInterface::create(objectID);
    }
}

int UnitBase::getCurrentAttackAngle() const {
    return drawnAngle;
}

void UnitBase::deploy(const Coord& newLocation) {

    if(currentGameMap->tileExists(newLocation)) {
        setLocation(newLocation);

        if (guardPoint.isInvalid())
            guardPoint = location;
        setDestination(guardPoint);
        pickedUp = false;
        setRespondable(true);
        setActive(true);
        setVisible(VIS_ALL, true);
        setForced(false);

        // Deployment logic to hopefully stop units freezing
        if (getAttackMode() == CARRYALLREQUESTED || getAttackMode() == HUNT) {
            if(getItemID() == Unit_Harvester) {
                doSetAttackMode(HARVEST);
            } else {
                doSetAttackMode(GUARD);
            }
        }

        if(isAGroundUnit() && (getItemID() != Unit_Sandworm)) {
            if(currentGameMap->getTile(location)->isSpiceBloom()) {
                setHealth(0);
                setVisible(VIS_ALL, false);
                currentGameMap->getTile(location)->triggerSpiceBloom(getOwner());
            } else if(currentGameMap->getTile(location)->isSpecialBloom()){
                currentGameMap->getTile(location)->triggerSpecialBloom(getOwner());
            }
        }
    }
}

void UnitBase::destroy() {

    setTarget(nullptr);
    currentGameMap->removeObjectFromMap(getObjectID()); //no map point will reference now
    currentGame->getObjectManager().removeObject(objectID);

    currentGame->getHouse(originalHouseID)->decrementUnits(itemID);

    unitList.remove(this);

    if(isVisible()) {
        if(currentGame->randomGen.rand(1,100) <= getInfSpawnProp()) {
            UnitBase* pNewUnit = currentGame->getHouse(originalHouseID)->createUnit(Unit_Soldier);
            pNewUnit->setHealth(pNewUnit->getMaxHealth()/2);
            pNewUnit->deploy(location);

            if(owner->getHouseID() != originalHouseID) {
                // deviation is inherited
                pNewUnit->owner = owner;
                pNewUnit->graphic = pGFXManager->getObjPic(pNewUnit->graphicID,owner->getHouseID());
                pNewUnit->deviationTimer = deviationTimer;
            }
        }
    }

    delete this;
}

void UnitBase::deviate(House* newOwner) {

    if(newOwner->getHouseID() == originalHouseID) {
        quitDeviation();
    } else {
        removeFromSelectionLists();
        setTarget(nullptr);
        setGuardPoint(location);
        setDestination(location);
        clearPath();
        doSetAttackMode(GUARD);
        owner = newOwner;

        graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
        deviationTimer = DEVIATIONTIME;
    }

    // Adding this in as a surrogate for damage inflicted upon deviation.. Still not sure what the best value
    // should be... going in with a 25% of the units value unless its a devastator which we can destruct or an ornithoper
    // which is likely to get killed
    if(getItemID() == Unit_Devastator || getItemID() == Unit_Ornithopter){
        newOwner->informHasDamaged(Unit_Deviator, currentGame->objectData.data[getItemID()][newOwner->getHouseID()].price);
    } else{
        newOwner->informHasDamaged(Unit_Deviator, currentGame->objectData.data[getItemID()][newOwner->getHouseID()].price / 5);
    }



}

void UnitBase::drawSelectionBox() {

    SDL_Texture* selectionBox = nullptr;

    switch(currentZoomlevel) {
        case 0:     selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel0);   break;
        case 1:     selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel1);   break;
        case 2:
        default:    selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel2);   break;
    }

    SDL_Rect dest = calcDrawingRect(selectionBox, screenborder->world2screenX(realX), screenborder->world2screenY(realY), HAlign::Center, VAlign::Center);
    SDL_RenderCopy(renderer, selectionBox, nullptr, &dest);

    int x = screenborder->world2screenX(realX) - getWidth(selectionBox)/2;
    int y = screenborder->world2screenY(realY) - getHeight(selectionBox)/2;
    for(int i=1;i<=currentZoomlevel+1;i++) {
        renderDrawHLine(renderer, x+1, y-i, x+1 + (lround((getHealth()/getMaxHealth())*(getWidth(selectionBox)-3))), getHealthColor());
    }
}

void UnitBase::drawOtherPlayerSelectionBox() {
    SDL_Texture* selectionBox = nullptr;

    switch(currentZoomlevel) {
        case 0:     selectionBox = pGFXManager->getUIGraphic(UI_OtherPlayerSelectionBox_Zoomlevel0);   break;
        case 1:     selectionBox = pGFXManager->getUIGraphic(UI_OtherPlayerSelectionBox_Zoomlevel1);   break;
        case 2:
        default:    selectionBox = pGFXManager->getUIGraphic(UI_OtherPlayerSelectionBox_Zoomlevel2);   break;
    }

    SDL_Rect dest = calcDrawingRect(selectionBox, screenborder->world2screenX(realX), screenborder->world2screenY(realY), HAlign::Center, VAlign::Center);
    SDL_RenderCopy(renderer, selectionBox, nullptr, &dest);
}


void UnitBase::releaseTarget() {
    if(forced == true) {
        guardPoint = location;
    }
    setDestination(guardPoint);

    findTargetTimer = 0;
    setForced(false);
    setTarget(nullptr);
}

void UnitBase::engageTarget() {

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

    if(target && !targetFriendly && !canAttack(target.getObjPointer())) {
        // the (non-friendly) target cannot be attacked anymore
        releaseTarget();
        return;
    }

    if(target && !targetFriendly && !forced && !isInAttackRange(target.getObjPointer())) {
        // the (non-friendly) target left the attack mode range (and we were not forced to attack it)
        releaseTarget();
        return;
    }

    if(target) {
        // we have a target unit or structure

        Coord targetLocation = target.getObjPointer()->getClosestPoint(location);

        if(destination != targetLocation) {
            // the location of the target has moved
            // => recalculate path
            clearPath();
        }

        targetDistance = blockDistance(location, targetLocation);

        Sint8 newTargetAngle = destinationDrawnAngle(location, targetLocation);

        if(bFollow) {
            // we are following someone
            setDestination(targetLocation);
            return;
        }

        if(targetDistance > getWeaponRange()) {
            // we are not in attack range
            // => follow the target
            setDestination(targetLocation);
            return;
        }

        // we are in attack range

        if(targetFriendly && !forced) {
            // the target is friendly and we only attack these if were forced to do so
            return;
        }

        if(goingToRepairYard) {
            // we are going to the repair yard
            // => we do not need to change the destination
            targetAngle = INVALID;
        } else if(attackMode == CAPTURE) {
            // we want to capture the target building
            setDestination(targetLocation);
            targetAngle = INVALID;
        } else if(isTracked() && target.getObjPointer()->isInfantry() && currentGameMap->tileExists(targetLocation) && !currentGameMap->getTile(targetLocation)->isMountain() && forced) {
            // we squash the infantry unit because we are forced to
            setDestination(targetLocation);
            targetAngle = INVALID;
        } else {
            // we decide to fire on the target thus we can stop moving
            setDestination(location);
            targetAngle = newTargetAngle;
        }

        if(getCurrentAttackAngle() == newTargetAngle) {
            attack();
        }

    } else if(attackPos) {
        // we attack a position

        targetDistance = blockDistance(location, attackPos);

        Sint8 newTargetAngle = destinationDrawnAngle(location, attackPos);

        if(targetDistance <= getWeaponRange()) {
            // we are in weapon range thus we can stop moving
            setDestination(location);
            targetAngle = newTargetAngle;

            if(getCurrentAttackAngle() == newTargetAngle) {
                attack();
            }
        } else {
            targetAngle = INVALID;
        }
    }
}

void UnitBase::move() {

    if(!moving && !justStoppedMoving && (isAFlyingUnit() == false) && currentGame->randomGen.rand(0,40) == 0 && itemID != Unit_Sandworm) {
        currentGameMap->viewMap(owner->getTeam(), location, getViewRange() );
    }

    if(moving && !justStoppedMoving) {
        if((isBadlyDamaged() == false) || isAFlyingUnit()) {
            realX += xSpeed;
            realY += ySpeed;
        } else {
            realX += xSpeed/2;
            realY += ySpeed/2;
        }

        // check if vehicle is on the first half of the way
        FixPoint fromDistanceX;
        FixPoint fromDistanceY;
        FixPoint toDistanceX;
        FixPoint toDistanceY;
        if(location != nextSpot) {
            // check if vehicle is half way out of old tile

            fromDistanceX = FixPoint::abs(location.x*TILESIZE - (realX-bumpyOffsetX) + TILESIZE/2);
            fromDistanceY = FixPoint::abs(location.y*TILESIZE - (realY-bumpyOffsetY) + TILESIZE/2);
            toDistanceX = FixPoint::abs(nextSpot.x*TILESIZE - (realX-bumpyOffsetX) + TILESIZE/2);
            toDistanceY = FixPoint::abs(nextSpot.y*TILESIZE - (realY-bumpyOffsetY) + TILESIZE/2);

            if((fromDistanceX >= TILESIZE/2) || (fromDistanceY >= TILESIZE/2)) {
                // let something else go in
                unassignFromMap(location);
                oldLocation = location;
                location = nextSpot;

                if(isAFlyingUnit() == false && itemID != Unit_Sandworm) {
                    currentGameMap->viewMap(owner->getTeam(), location, getViewRange());
                }
            }

        } else {
            // if vehicle is out of old tile

            fromDistanceX = FixPoint::abs(oldLocation.x*TILESIZE - (realX-bumpyOffsetX) + TILESIZE/2);
            fromDistanceY = FixPoint::abs(oldLocation.y*TILESIZE - (realY-bumpyOffsetY) + TILESIZE/2);
            toDistanceX = FixPoint::abs(location.x*TILESIZE - (realX-bumpyOffsetX) + TILESIZE/2);
            toDistanceY = FixPoint::abs(location.y*TILESIZE - (realY-bumpyOffsetY) + TILESIZE/2);

            if ((fromDistanceX >= TILESIZE) || (fromDistanceY >= TILESIZE)) {

                if(forced && (location == destination) && !target) {
                    setForced(false);
                    if(getAttackMode() == CARRYALLREQUESTED) {
                        doSetAttackMode(GUARD);
                    }
                }

                moving = false;
                justStoppedMoving = true;
                realX = location.x * TILESIZE + TILESIZE/2;
                realY = location.y * TILESIZE + TILESIZE/2;
                bumpyOffsetX = 0;
                bumpyOffsetY = 0;

                oldLocation.invalidate();
            }

        }

        bumpyMovementOnRock(fromDistanceX, fromDistanceY, toDistanceX, toDistanceY);

    } else {
        justStoppedMoving = false;
    }

    checkPos();
}

void UnitBase::bumpyMovementOnRock(FixPoint fromDistanceX, FixPoint fromDistanceY, FixPoint toDistanceX, FixPoint toDistanceY) {

    if(hasBumpyMovementOnRock() && ((currentGameMap->getTile(location)->getType() == Terrain_Rock)
                                    || (currentGameMap->getTile(location)->getType() == Terrain_Mountain)
                                    || (currentGameMap->getTile(location)->getType() == Terrain_ThickSpice))) {
        // bumping effect

        const FixPoint epsilon = FixPt(0,005);
        const FixPoint bumpyOffset = FixPt(2,5);
        const FixPoint absXSpeed = FixPoint::abs(xSpeed);
        const FixPoint absYSpeed = FixPoint::abs(ySpeed);


        if((FixPoint::abs(xSpeed) >= epsilon) && (FixPoint::abs(fromDistanceX - absXSpeed) < absXSpeed/2)) { realY -= bumpyOffset; bumpyOffsetY -= bumpyOffset; }
        if((FixPoint::abs(ySpeed) >= epsilon) && (FixPoint::abs(fromDistanceY - absYSpeed) < absYSpeed/2)) { realX += bumpyOffset; bumpyOffsetX += bumpyOffset; }

        if((FixPoint::abs(xSpeed) >= epsilon) && (FixPoint::abs(fromDistanceX - 4*absXSpeed) < absXSpeed/2)) { realY += bumpyOffset; bumpyOffsetY += bumpyOffset; }
        if((FixPoint::abs(ySpeed) >= epsilon) && (FixPoint::abs(fromDistanceY - 4*absYSpeed) < absYSpeed/2)) { realX -= bumpyOffset; bumpyOffsetX -= bumpyOffset; }


        if((FixPoint::abs(xSpeed) >= epsilon) && (FixPoint::abs(fromDistanceX - 10*absXSpeed) < absXSpeed/2)) { realY -= bumpyOffset; bumpyOffsetY -= bumpyOffset; }
        if((FixPoint::abs(ySpeed) >= epsilon) && (FixPoint::abs(fromDistanceY - 20*absYSpeed) < absYSpeed/2)) { realX += bumpyOffset; bumpyOffsetX += bumpyOffset; }

        if((FixPoint::abs(xSpeed) >= epsilon) && (FixPoint::abs(fromDistanceX - 14*absXSpeed) < absXSpeed/2)) { realY += bumpyOffset; bumpyOffsetY += bumpyOffset; }
        if((FixPoint::abs(ySpeed) >= epsilon) && (FixPoint::abs(fromDistanceY - 14*absYSpeed) < absYSpeed/2)) { realX -= bumpyOffset; bumpyOffsetX -= bumpyOffset; }


        if((FixPoint::abs(xSpeed) >= epsilon) && (FixPoint::abs(toDistanceX - absXSpeed) < absXSpeed/2)) { realY -= bumpyOffset; bumpyOffsetY -= bumpyOffset; }
        if((FixPoint::abs(ySpeed) >= epsilon) && (FixPoint::abs(toDistanceY - absYSpeed) < absYSpeed/2)) { realX += bumpyOffset; bumpyOffsetX += bumpyOffset; }

        if((FixPoint::abs(xSpeed) >= epsilon) && (FixPoint::abs(toDistanceX - 4*absXSpeed) < absXSpeed/2)) { realY += bumpyOffset; bumpyOffsetY += bumpyOffset; }
        if((FixPoint::abs(ySpeed) >= epsilon) && (FixPoint::abs(toDistanceY - 4*absYSpeed) < absYSpeed/2)) { realX -= bumpyOffset; bumpyOffsetX -= bumpyOffset; }

        if((FixPoint::abs(xSpeed) >= epsilon) && (FixPoint::abs(toDistanceX - 10*absXSpeed) < absXSpeed/2)) { realY -= bumpyOffset; bumpyOffsetY -= bumpyOffset; }
        if((FixPoint::abs(ySpeed) >= epsilon) && (FixPoint::abs(toDistanceY - 10*absYSpeed) < absYSpeed/2)) { realX += bumpyOffset; bumpyOffsetX += bumpyOffset; }

        if((FixPoint::abs(xSpeed) >= epsilon) && (FixPoint::abs(toDistanceX - 14*absXSpeed) < absXSpeed/2)) { realY += bumpyOffset; bumpyOffsetY += bumpyOffset; }
        if((FixPoint::abs(ySpeed) >= epsilon) && (FixPoint::abs(toDistanceY - 14*absYSpeed) < absYSpeed/2)) { realX -= bumpyOffset; bumpyOffsetX -= bumpyOffset; }

    }
}

void UnitBase::navigate() {

    if(isAFlyingUnit() || (((currentGame->getGameCycleCount() + getObjectID()*1337) % 5) == 0)) {
        // navigation is only performed every 5th frame

        if(!moving && !justStoppedMoving) {
            if(location != destination) {
                if(nextSpotFound == false)  {

                    if(pathList.empty() && (recalculatePathTimer == 0)) {
                        recalculatePathTimer = 100;

                        if(!SearchPathWithAStar() && (++noCloserPointCount >= 3)
                            && (location != oldLocation))
                        {   //try searching for a path a number of times then give up
                            if (target.getObjPointer() != nullptr && targetFriendly
                                && (target.getObjPointer()->getItemID() != Structure_RepairYard)
                                && ((target.getObjPointer()->getItemID() != Structure_Refinery)
                                || (getItemID() != Unit_Harvester))) {
                                setTarget(nullptr);
                            }

                            /// This method will transport units if they get stuck inside a base
                            /// This often happens after an AI get nuked and has a hole in their base
                            if(getOwner()->hasCarryalls()
                               && this->isAGroundUnit()
                               && blockDistance(location, destination) >= 6
                               && (currentGame->getGameInitSettings().getGameOptions().manualCarryallDrops
                                   || getOwner()->isAI())){
                               static_cast<GroundUnit*>(this)->requestCarryall();
                            } else {
                                setDestination(location);   //can't get any closer, give up
                                forced = false;
                            }
                        }
                    }

                    if(!pathList.empty()) {
                        nextSpot = pathList.front();
                        pathList.pop_front();
                        nextSpotFound = true;
                        recalculatePathTimer = 0;
                        noCloserPointCount = 0;
                    }
                } else {
                    int tempAngle = currentGameMap->getPosAngle(location, nextSpot);
                    if(tempAngle != INVALID) {
                        nextSpotAngle = tempAngle;
                    }

                    if(!canPass(nextSpot.x, nextSpot.y)) {
                        clearPath();
                    } else {
                        if (drawnAngle == nextSpotAngle)    {
                            moving = true;
                            nextSpotFound = false;

                            assignToMap(nextSpot);
                            angle = drawnAngle;
                            setSpeeds();
                        }
                    }
                }
            } else if(!target && attackPos.isInvalid()) {
                if(((currentGame->getGameCycleCount() + getObjectID()*1337) % MILLI2CYCLES(UNITIDLETIMER)) == 0) {
                    idleAction();
                }
            }
        }
    }
}

void UnitBase::idleAction() {
    //not moving and not wanting to go anywhere, do some random turning
    if(isAGroundUnit() && (getItemID() != Unit_Harvester) && (getAttackMode() == GUARD)) {
        // we might turn this cylce with 20% chance
        if(currentGame->randomGen.rand(0, 4) == 0) {
            // choose a random one of the eight possible angles
            nextSpotAngle = currentGame->randomGen.rand(0, 7);
        }
    }
}

void UnitBase::handleActionClick(int xPos, int yPos) {
    if(respondable) {
        if(currentGameMap->tileExists(xPos, yPos)) {
            if(currentGameMap->getTile(xPos,yPos)->hasAnObject()) {
                // attack unit/structure or move to structure
                ObjectBase* tempTarget = currentGameMap->getTile(xPos,yPos)->getObject();

                if(tempTarget->getOwner()->getTeam() != getOwner()->getTeam()) {
                    // attack
                    currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_UNIT_ATTACKOBJECT,objectID,tempTarget->getObjectID()));
                } else {
                    // move to object/structure
                    currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_UNIT_MOVE2OBJECT,objectID,tempTarget->getObjectID()));
                }
            } else {
                // move this unit
                currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_UNIT_MOVE2POS,objectID,(Uint32) xPos, (Uint32) yPos, (Uint32) true));
            }
        }
    }
}

void UnitBase::handleAttackClick(int xPos, int yPos) {
    if(respondable) {
        if(currentGameMap->tileExists(xPos, yPos)) {
            if(currentGameMap->getTile(xPos,yPos)->hasAnObject()) {
                // attack unit/structure or move to structure
                ObjectBase* tempTarget = currentGameMap->getTile(xPos,yPos)->getObject();

                currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_UNIT_ATTACKOBJECT,objectID,tempTarget->getObjectID()));
            } else {
                // attack pos
                currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_UNIT_ATTACKPOS,objectID,(Uint32) xPos, (Uint32) yPos, (Uint32) true));
            }
        }
    }

}

void UnitBase::handleMoveClick(int xPos, int yPos) {
    if(respondable) {
        if(currentGameMap->tileExists(xPos, yPos)) {
            // move to pos
            currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_UNIT_MOVE2POS,objectID,(Uint32) xPos, (Uint32) yPos, (Uint32) true));
        }
    }
}

void UnitBase::handleSetAttackModeClick(ATTACKMODE newAttackMode) {
    currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_UNIT_SETMODE,objectID,(Uint32) newAttackMode));
}

/**
    User action
    Request a Carryall to drop at target location
**/
void UnitBase::handleRequestCarryallDropClick(int xPos, int yPos) {
    if(respondable) {
        if(currentGameMap->tileExists(xPos, yPos)) {
            currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_UNIT_REQUESTCARRYALLDROP, objectID, (Uint32) xPos, (Uint32) yPos));
        }
    }
}



void UnitBase::doMove2Pos(int xPos, int yPos, bool bForced) {
    if(attackMode == CAPTURE || attackMode == HUNT) {
        doSetAttackMode(GUARD);
    }

    if((xPos != destination.x) || (yPos != destination.y)) {
        clearPath();
        findTargetTimer = 0;
    }

    setTarget(nullptr);
    setDestination(xPos,yPos);
    setForced(bForced);
    setGuardPoint(xPos,yPos);
}

void UnitBase::doMove2Pos(const Coord& coord, bool bForced) {
    doMove2Pos(coord.x, coord.y, bForced);
}

void UnitBase::doMove2Object(const ObjectBase* pTargetObject) {
    if(pTargetObject->getObjectID() == getObjectID()) {
        return;
    }

    if(attackMode == CAPTURE || attackMode == HUNT) {
        doSetAttackMode(GUARD);
    }

    setDestination(INVALID_POS,INVALID_POS);
    setTarget(pTargetObject);
    setForced(true);

    bFollow = true;

    clearPath();
    findTargetTimer = 0;
}

void UnitBase::doMove2Object(Uint32 targetObjectID) {
    ObjectBase* pObject = currentGame->getObjectManager().getObject(targetObjectID);

    if(pObject == nullptr) {
        return;
    }

    doMove2Object(pObject);
}

void UnitBase::doAttackPos(int xPos, int yPos, bool bForced) {
    if(attackMode == CAPTURE) {
        doSetAttackMode(GUARD);
    }

    setDestination(xPos,yPos);
    setTarget(nullptr);
    setForced(bForced);
    attackPos.x = xPos;
    attackPos.y = yPos;

    clearPath();
    findTargetTimer = 0;
}

void UnitBase::doAttackObject(const ObjectBase* pTargetObject, bool bForced) {
    if(pTargetObject->getObjectID() == getObjectID() || (!canAttack() && getItemID() != Unit_Harvester)) {
        return;
    }

    if(attackMode == CAPTURE) {
        doSetAttackMode(GUARD);
    }

    setDestination(INVALID_POS,INVALID_POS);

    setTarget(pTargetObject);
    // hack to make it possible to attack own repair yard
    if(goingToRepairYard && target && (target.getObjPointer()->getItemID() == Structure_RepairYard)) {
        static_cast<RepairYard*>(target.getObjPointer())->unBook();
        goingToRepairYard = false;
    }


    setForced(bForced);

    clearPath();
    findTargetTimer = 0;
}

void UnitBase::doAttackObject(Uint32 TargetObjectID, bool bForced) {
    ObjectBase* pObject = currentGame->getObjectManager().getObject(TargetObjectID);

    if(pObject == nullptr) {
        return;
    }

    doAttackObject(pObject, bForced);
}

void UnitBase::doSetAttackMode(ATTACKMODE newAttackMode) {
    if((newAttackMode >= 0) && (newAttackMode < ATTACKMODE_MAX)) {
        attackMode = newAttackMode;
    }

    if(attackMode == GUARD || attackMode == STOP) {
        if(moving && !justStoppedMoving) {
            doMove2Pos(nextSpot, false);
        } else {
            doMove2Pos(location, false);
        }
    }
}

void UnitBase::handleDamage(int damage, Uint32 damagerID, House* damagerOwner) {
    // shorten deviation time
    if(deviationTimer > 0) {
        deviationTimer = std::max(0,deviationTimer - MILLI2CYCLES(damage*20*1000));
    }

    ObjectBase::handleDamage(damage, damagerID, damagerOwner);

    ObjectBase* pDamager = currentGame->getObjectManager().getObject(damagerID);

    if(pDamager != nullptr){

        if(attackMode == HUNT && !forced) {
            ObjectBase* pDamager = currentGame->getObjectManager().getObject(damagerID);
            if(canAttack(pDamager)) {
                if(!target || target.getObjPointer() == nullptr || !isInWeaponRange(target.getObjPointer())) {
                    // no target or target not on weapon range => switch target
                    doAttackObject(pDamager, false);
                }

            }
        }

        /*
         This method records the damage taken so that QuantBot can use it to know how effective different unit
         classes are during the current game so that it can adjust its unit build ratios
        */

        // If you damaged your own unit then, the damage should be treated as negative.
        if(damagerOwner == getOwner()){
            damage *= -1;
        }

        damagerOwner->informHasDamaged(pDamager->getItemID(), damage);
    }
}

bool UnitBase::isInGuardRange(const ObjectBase* pObject) const  {
    int checkRange;
    switch(attackMode) {
        case GUARD: {
            checkRange = getWeaponRange();
        } break;

        case AREAGUARD: {
            checkRange = getAreaGuardRange();
        } break;

        case AMBUSH: {
            checkRange = getViewRange();
        } break;

        case HUNT: {
            return true;
        } break;

        case CARRYALLREQUESTED: {
            return false;
        } break;

        case RETREAT: {
            return false;
        } break;

        case STOP:
        default: {
            return false;
        } break;
    }

    if(getItemID() == Unit_Sandworm) {
        checkRange = getViewRange();
    }

    return (blockDistance(guardPoint*TILESIZE + Coord(TILESIZE/2, TILESIZE/2), pObject->getCenterPoint()) <= checkRange*TILESIZE);
}

bool UnitBase::isInAttackRange(const ObjectBase* pObject) const {
    int checkRange;
    switch(attackMode) {
        case GUARD: {
            checkRange = getWeaponRange();
        } break;

        case AREAGUARD: {
            checkRange = getAreaGuardRange() + getWeaponRange() + 1;
        } break;

        case AMBUSH: {
            checkRange = getViewRange() + 1;
        } break;

        case HUNT: {
            return true;
        } break;

        case CARRYALLREQUESTED: {
            return false;
        } break;

        case RETREAT: {
            return false;
        } break;

        case STOP:
        default: {
            return false;
        } break;
    }

    if(getItemID() == Unit_Sandworm) {
        checkRange = getViewRange() + 1;
    }

    return (blockDistance(guardPoint*TILESIZE + Coord(TILESIZE/2, TILESIZE/2), pObject->getCenterPoint()) <= checkRange*TILESIZE);
}

bool UnitBase::isInWeaponRange(const ObjectBase* object) const {
    if(object == nullptr) {
        return false;
    }

    Coord targetLocation = target.getObjPointer()->getClosestPoint(location);

    return (blockDistance(location, targetLocation) <= getWeaponRange());
}


void UnitBase::setAngle(int newAngle) {
    if(!moving && !justStoppedMoving) {
        newAngle = newAngle % NUM_ANGLES;
        angle = drawnAngle = newAngle;
        clearPath();
    }
}

void UnitBase::setGettingRepaired() {
    if(target.getObjPointer() != nullptr && (target.getObjPointer()->getItemID() == Structure_RepairYard)) {
        if(selected) {
            removeFromSelectionLists();
        }

        currentGameMap->removeObjectFromMap(getObjectID());

        static_cast<RepairYard*>(target.getObjPointer())->assignUnit(this);

        respondable = false;
        setActive(false);
        setVisible(VIS_ALL, false);
        goingToRepairYard = false;
        badlyDamaged = false;

        setTarget(nullptr);
        //setLocation(NONE, NONE);
        setDestination(location);
        nextSpotAngle = DOWN;
    }
}

void UnitBase::setGuardPoint(int newX, int newY) {
    if(currentGameMap->tileExists(newX, newY) || ((newX == INVALID_POS) && (newY == INVALID_POS))) {
        guardPoint.x = newX;
        guardPoint.y = newY;

        if((getItemID() == Unit_Harvester) && guardPoint.isValid()) {
            if(currentGameMap->getTile(newX, newY)->hasSpice()) {
                if(attackMode == STOP) {
                    attackMode = GUARD;
                }
            } else {
                if(attackMode != STOP) {
                    attackMode = STOP;
                }
            }
        }
    }
}

void UnitBase::setLocation(int xPos, int yPos) {

    if((xPos == INVALID_POS) && (yPos == INVALID_POS)) {
        ObjectBase::setLocation(xPos, yPos);
    } else if (currentGameMap->tileExists(xPos, yPos)) {
        ObjectBase::setLocation(xPos, yPos);
        realX += TILESIZE/2;
        realY += TILESIZE/2;
        bumpyOffsetX = 0;
        bumpyOffsetY = 0;
    }

    moving = false;
    pickedUp = false;
    setTarget(nullptr);

    clearPath();
}

void UnitBase::setPickedUp(UnitBase* newCarrier) {
    if(selected) {
        removeFromSelectionLists();
    }

    currentGameMap->removeObjectFromMap(getObjectID());

    if(goingToRepairYard) {
        static_cast<RepairYard*>(target.getObjPointer())->unBook();
    }

    if(getItemID() == Unit_Harvester) {
        Harvester* harvester = static_cast<Harvester*>(this);
        if(harvester->isReturning() && target && (target.getObjPointer()!= nullptr) && (target.getObjPointer()->getItemID() == Structure_Refinery)) {
            static_cast<Refinery*>(target.getObjPointer())->unBook();
        }
    }

    target.pointTo(newCarrier);

    goingToRepairYard = false;
    forced = false;
    moving = false;
    pickedUp = true;
    respondable = false;
    setActive(false);
    setVisible(VIS_ALL, false);

    clearPath();
}

FixPoint UnitBase::getMaxSpeed() const {
    return currentGame->objectData.data[itemID][originalHouseID].maxspeed;
}

void UnitBase::setSpeeds() {
    FixPoint speed = getMaxSpeed();

    if(!isAFlyingUnit()) {
        speed += speed*(1 - getTerrainDifficulty((TERRAINTYPE) currentGameMap->getTile(location)->getType()));
        if(isBadlyDamaged()) {
            speed *= HEAVILYDAMAGEDSPEEDMULTIPLIER;
        }
    }

    switch(drawnAngle){
        case LEFT:      xSpeed = -speed;                    ySpeed = 0;         break;
        case LEFTUP:    xSpeed = -speed*DIAGONALSPEEDCONST; ySpeed = xSpeed;    break;
        case UP:        xSpeed = 0;                         ySpeed = -speed;    break;
        case RIGHTUP:   xSpeed = speed*DIAGONALSPEEDCONST;  ySpeed = -xSpeed;   break;
        case RIGHT:     xSpeed = speed;                     ySpeed = 0;         break;
        case RIGHTDOWN: xSpeed = speed*DIAGONALSPEEDCONST;  ySpeed = xSpeed;    break;
        case DOWN:      xSpeed = 0;                         ySpeed = speed;     break;
        case LEFTDOWN:  xSpeed = -speed*DIAGONALSPEEDCONST; ySpeed = -xSpeed;   break;
    }
}

void UnitBase::setTarget(const ObjectBase* newTarget) {
    attackPos.invalidate();
    bFollow = false;
    targetAngle = INVALID;

    if(goingToRepairYard && target && (target.getObjPointer()->getItemID() == Structure_RepairYard)) {
        static_cast<RepairYard*>(target.getObjPointer())->unBook();
        goingToRepairYard = false;
    }

    ObjectBase::setTarget(newTarget);

    if(target.getObjPointer() != nullptr
        && (target.getObjPointer()->getOwner() == getOwner())
        && (target.getObjPointer()->getItemID() == Structure_RepairYard)
        && (itemID != Unit_Carryall) && (itemID != Unit_Frigate)
        && (itemID != Unit_Ornithopter)) {
        static_cast<RepairYard*>(target.getObjPointer())->book();
        goingToRepairYard = true;
    }
}

void UnitBase::targeting() {
    if(findTargetTimer == 0) {

        if(attackMode != STOP && attackMode != CARRYALLREQUESTED) {

            // lets add a bit of logic to make units recalibrate their nearest target if the target isn't in weapon range
            if(target && !attackPos && !forced &&(attackMode == GUARD || attackMode == AREAGUARD || attackMode == HUNT)){
                if(!isInWeaponRange(target.getObjPointer())){
                    const ObjectBase* pNewTarget = findTarget();

                    if(pNewTarget != nullptr) {

                        doAttackObject(pNewTarget, false);

                        findTargetTimer = 500;
                    }
                }
            }


            if(!target && !attackPos && !moving && !justStoppedMoving && !forced) {
                // we have no target, we have stopped moving and we weren't forced to do anything else

                const ObjectBase* pNewTarget = findTarget();

                if(pNewTarget != nullptr && isInGuardRange(pNewTarget)) {
                    // we have found a new target => attack it
                    if(attackMode == AMBUSH) {
                        doSetAttackMode(HUNT);
                    }
                    doAttackObject(pNewTarget, false);

                    if(getItemID() == Unit_Sandworm) {
                        if(pNewTarget->getOwner() == pLocalHouse) {
                            soundPlayer->playVoice(WarningWormSign, pLocalHouse->getHouseID());
                        }
                        doSetAttackMode(HUNT);
                    }
                } else if(attackMode == HUNT) {
                    setGuardPoint(location);
                    doSetAttackMode(GUARD);
                }

                // reset target timer
                findTargetTimer = 200;
            }
        }

    }

    engageTarget();
}

void UnitBase::turn() {
    if(!moving && !justStoppedMoving) {
        int wantedAngle = INVALID;

        // if we have to decide between moving and shooting we opt for moving
        if(nextSpotAngle != INVALID) {
            wantedAngle = nextSpotAngle;
        } else if(targetAngle != INVALID) {
            wantedAngle = targetAngle;
        }

        if(wantedAngle != INVALID) {
            FixPoint angleLeft = 0;
            FixPoint angleRight = 0;

            if(angle > wantedAngle) {
                angleRight = angle - wantedAngle;
                angleLeft = FixPoint::abs(8-angle)+wantedAngle;
            } else if (angle < wantedAngle) {
                angleRight = FixPoint::abs(8-wantedAngle) + angle;
                angleLeft = wantedAngle - angle;
            }

            if(angleLeft <= angleRight) {
                turnLeft();
            } else {
                turnRight();
            }
        }
    }
}

void UnitBase::turnLeft() {
    angle += currentGame->objectData.data[itemID][originalHouseID].turnspeed;
    if(angle >= FixPt(7,5)) {
        drawnAngle = lround(angle) - NUM_ANGLES;
        angle -= NUM_ANGLES;
    } else {
        drawnAngle = lround(angle);
    }
}

void UnitBase::turnRight() {
    angle -= currentGame->objectData.data[itemID][originalHouseID].turnspeed;
    if(angle <= FixPt(-0,5)) {
        drawnAngle = lround(angle) + NUM_ANGLES;
        angle += NUM_ANGLES;
    } else {
        drawnAngle = lround(angle);
    }
}

void UnitBase::quitDeviation() {
    if(wasDeviated()) {
        // revert back to real owner
        removeFromSelectionLists();
        setTarget(nullptr);
        setGuardPoint(location);
        setDestination(location);
        owner = currentGame->getHouse(originalHouseID);
        graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
        deviationTimer = INVALID;
    }
}

bool UnitBase::update() {
    if(active) {
        targeting();
        navigate();
        move();
        if(active) {
            turn();
        }
    }

    if(getHealth() <= 0) {
        destroy();
        return false;
    }




    if(recalculatePathTimer > 0) recalculatePathTimer--;
    if(findTargetTimer > 0) findTargetTimer--;
    if(primaryWeaponTimer > 0) primaryWeaponTimer--;
    if(secondaryWeaponTimer > 0) secondaryWeaponTimer--;
    if(deviationTimer != INVALID) {
        if(--deviationTimer <= 0) {
            quitDeviation();
        }
    }

    return true;
}

bool UnitBase::canPass(int xPos, int yPos) const {
    return (currentGameMap->tileExists(xPos, yPos) && !currentGameMap->getTile(xPos, yPos)->hasAGroundObject() && !currentGameMap->getTile(xPos, yPos)->isMountain());
}

bool UnitBase::SearchPathWithAStar() {
    Coord destinationCoord;

    if(target && target.getObjPointer() != nullptr) {
        if(itemID == Unit_Carryall && target.getObjPointer()->getItemID() == Structure_Refinery) {
            destinationCoord = target.getObjPointer()->getLocation() + Coord(2,0);
        } else if(itemID == Unit_Frigate && target.getObjPointer()->getItemID() == Structure_StarPort) {
            destinationCoord = target.getObjPointer()->getLocation() + Coord(1,1);
        } else {
            destinationCoord = target.getObjPointer()->getClosestPoint(location);
        }
    } else {
        destinationCoord = destination;
    }

    AStarSearch pathfinder(currentGameMap, this, location, destinationCoord);
    pathList = pathfinder.getFoundPath();

    if(pathList.empty() == true) {
        nextSpotFound = false;
        return false;
    } else {
        return true;
    }
}

void UnitBase::drawSmoke(int x, int y) {
    int frame = ((currentGame->getGameCycleCount() + (getObjectID() * 10)) / SMOKEDELAY) % (2*2);
    if(frame == 3) {
        frame = 1;
    }

    SDL_Texture** smoke = pGFXManager->getObjPic(ObjPic_Smoke,getOwner()->getHouseID());

    SDL_Rect dest = calcSpriteDrawingRect(smoke[currentZoomlevel], x, y, 3, 1, HAlign::Center, VAlign::Bottom);
    SDL_Rect source = calcSpriteSourceRect(smoke[currentZoomlevel], frame, 3);

    SDL_RenderCopy(renderer, smoke[currentZoomlevel], &source, &dest);
}

void UnitBase::playAttackSound() {
}
