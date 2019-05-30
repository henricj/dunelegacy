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

#include <structures/TurretBase.h>

#include <globals.h>

#include <Game.h>
#include <Map.h>
#include <House.h>
#include <Bullet.h>
#include <SoundPlayer.h>

#include <players/HumanPlayer.h>

TurretBase::TurretBase(House* newOwner) : StructureBase(newOwner)
{
    TurretBase::init();

    angle = currentGame->randomGen.rand(0, 7);
    drawnAngle = lround(angle);

    findTargetTimer = 0;
    weaponTimer = 0;
}

TurretBase::TurretBase(InputStream& stream) : StructureBase(stream) {
    TurretBase::init();

    findTargetTimer = stream.readSint32();
    weaponTimer = stream.readSint32();
}

void TurretBase::init() {
    attackSound = Sound_Gun;
    bulletType = Bullet_ShellTurret;

    structureSize.x = 1;
    structureSize.y = 1;

    canAttackStuff = true;
    attackMode = AREAGUARD;
}

TurretBase::~TurretBase() = default;

void TurretBase::save(OutputStream& stream) const {
    StructureBase::save(stream);

    stream.writeSint32(findTargetTimer);
    stream.writeSint32(weaponTimer);
}

void TurretBase::updateStructureSpecificStuff() {
    if(target && (target.getObjPointer() != nullptr)) {
        if(!canAttack(target.getObjPointer()) || !targetInWeaponRange()) {
            setTarget(nullptr);
        } else if(targetInWeaponRange()) {
            Coord closestPoint = target.getObjPointer()->getClosestPoint(location);
            int wantedAngle = destinationDrawnAngle(location, closestPoint);

            if(angle != wantedAngle) {
                // turn
                FixPoint  angleLeft = 0;
                FixPoint  angleRight = 0;

                if(angle > wantedAngle) {
                    angleRight = angle - wantedAngle;
                    angleLeft = FixPoint::abs(NUM_ANGLES-angle)+wantedAngle;
                }
                else if(angle < wantedAngle) {
                    angleRight = FixPoint::abs(NUM_ANGLES-wantedAngle) + angle;
                    angleLeft = wantedAngle - angle;
                }

                if(angleLeft <= angleRight) {
                    turnLeft();
                } else {
                    turnRight();
                }
            }

            if(drawnAngle == wantedAngle) {
                attack();
            }

        } else {
            setTarget(nullptr);
        }
    } else if((attackMode != STOP) && (findTargetTimer == 0)) {
        setTarget(findTarget());
        findTargetTimer = 100;
    }

    if(findTargetTimer > 0) {
        findTargetTimer--;
    }

    if(weaponTimer > 0) {
        weaponTimer--;
    }
}

void TurretBase::handleActionCommand(int xPos, int yPos) {
    if(currentGameMap->tileExists(xPos, yPos)) {
        ObjectBase* tempTarget = currentGameMap->getTile(xPos, yPos)->getObject();
        currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_TURRET_ATTACKOBJECT,objectID,tempTarget->getObjectID()));

    }
}

void TurretBase::doAttackObject(Uint32 targetObjectID) {
    const ObjectBase* pObject = currentGame->getObjectManager().getObject(targetObjectID);
    doAttackObject(pObject);
}

void TurretBase::doAttackObject(const ObjectBase* pObject) {
    if(pObject == nullptr) {
        return;
    }

    setDestination(INVALID_POS,INVALID_POS);
    setTarget(pObject);
    setForced(true);
}

void TurretBase::turnLeft() {
    angle += currentGame->objectData.data[itemID][originalHouseID].turnspeed;
    if (angle >= 7.5_fix)    //must keep drawnangle between 0 and 7
        angle -= 8;
    drawnAngle = lround(angle);
    curAnimFrame = firstAnimFrame = lastAnimFrame = ((10-drawnAngle) % 8) + 2;
}

void TurretBase::turnRight() {
    angle -= currentGame->objectData.data[itemID][originalHouseID].turnspeed;
    if(angle < -0.5_fix) {
        //must keep angle between 0 and 7
        angle += 8;
    }
    drawnAngle = lround(angle);
    curAnimFrame = firstAnimFrame = lastAnimFrame = ((10-drawnAngle) % 8) + 2;
}

void TurretBase::attack() {
    if((weaponTimer == 0) && (target.getObjPointer() != nullptr)) {
        Coord centerPoint = getCenterPoint();
        ObjectBase* pObject = target.getObjPointer();
        Coord targetCenterPoint = pObject->getClosestCenterPoint(location);

        bulletList.push_back( new Bullet( objectID, &centerPoint, &targetCenterPoint,bulletType,
                                               currentGame->objectData.data[itemID][originalHouseID].weapondamage,
                                               pObject->isAFlyingUnit(),
                                               pObject) );

        currentGameMap->viewMap(pObject->getOwner()->getHouseID(), location, 2);
        soundPlayer->playSoundAt(attackSound, location);
        weaponTimer = getWeaponReloadTime();
    }
}
