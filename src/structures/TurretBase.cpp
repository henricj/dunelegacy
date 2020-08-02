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


TurretBase::TurretBase(const TurretBaseConstants& constants, Uint32 objectID, const ObjectInitializer& initializer)
    : StructureBase(constants, objectID, initializer) {
    TurretBase::init();

    angle = initializer.Game.randomGen.rand(0, 7);
    drawnAngle = static_cast<ANGLETYPE>(lround(angle));

    findTargetTimer = 0;
    weaponTimer = 0;
}

TurretBase::TurretBase(const TurretBaseConstants& constants, Uint32 objectID,
                       const ObjectStreamInitializer& initializer)
    : StructureBase(constants, objectID, initializer) {
    TurretBase::init();

    auto& stream    = initializer.Stream;

    findTargetTimer = stream.readSint32();
    weaponTimer = stream.readSint32();
}

void TurretBase::init() {
    attackSound = Sound_Gun;

    attackMode = AREAGUARD;
}

TurretBase::~TurretBase() = default;

void TurretBase::save(OutputStream& stream) const {
    StructureBase::save(stream);

    stream.writeSint32(findTargetTimer);
    stream.writeSint32(weaponTimer);
}

void TurretBase::updateStructureSpecificStuff(const GameContext& context) {
    if(target && (target.getObjPointer() != nullptr)) {
        if(!canAttack(target.getObjPointer()) || !targetInWeaponRange()) {
            setTarget(nullptr);
        } else if(targetInWeaponRange()) {
            Coord closestPoint = target.getObjPointer()->getClosestPoint(location);
            const auto wantedAngle = destinationDrawnAngle(location, closestPoint);

            if(angle != static_cast<int>(wantedAngle)) {
                // turn
                FixPoint  angleLeft = 0;
                FixPoint  angleRight = 0;

                if(angle > static_cast<int>(wantedAngle)) {
                    angleRight = angle - static_cast<int>(wantedAngle);
                    angleLeft =
                        FixPoint::abs(static_cast<int>(ANGLETYPE::NUM_ANGLES) - angle) + static_cast<int>(wantedAngle);
                } else if(angle < static_cast<int>(wantedAngle)) {
                    angleRight =
                        FixPoint::abs(static_cast<int>(ANGLETYPE::NUM_ANGLES) - static_cast<int>(wantedAngle)) + angle;
                    angleLeft = static_cast<int>(wantedAngle) - angle;
                }

                if(angleLeft <= angleRight) {
                    turnLeft(context);
                } else {
                    turnRight(context);
                }
            }

            if(drawnAngle == wantedAngle) {
                attack(context);
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

void TurretBase::handleActionCommand(const GameContext& context, int xPos, int yPos) {
    auto& [game, map, objectManager] = context;

    if(auto* tile = map.tryGetTile(xPos, yPos)) {
        ObjectBase* tempTarget = tile->getObject(objectManager);
        game.getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMDTYPE::CMD_TURRET_ATTACKOBJECT,
                                                    objectID, tempTarget->getObjectID()));
    }
}

void TurretBase::doAttackObject(const GameContext& context, Uint32 targetObjectID) {
    const auto* pObject = context.objectManager.getObject(targetObjectID);
    doAttackObject(pObject);
}

void TurretBase::doAttackObject(const ObjectBase* pObject) {
    if(pObject == nullptr) { return; }

    setDestination(INVALID_POS, INVALID_POS);
    setTarget(pObject);
    setForced(true);
}

void TurretBase::turnLeft(const GameContext& context) {
    angle += context.game.objectData.data[itemID][static_cast<int>(originalHouseID)].turnspeed;
    if (angle >= 7.5_fix)    //must keep drawnangle between 0 and 7
        angle -= 8;
    drawnAngle = static_cast<ANGLETYPE>(lround(angle));
    curAnimFrame = firstAnimFrame = lastAnimFrame = ((10-static_cast<int>(drawnAngle)) % 8) + 2;
}

void TurretBase::turnRight(const GameContext& context) {
    angle -= context.game.objectData.data[itemID][static_cast<int>(originalHouseID)].turnspeed;
    if(angle < -0.5_fix) {
        //must keep angle between 0 and 7
        angle += 8;
    }
    drawnAngle = static_cast<ANGLETYPE>(lround(angle));
    curAnimFrame = firstAnimFrame = lastAnimFrame = ((10-static_cast<int>(drawnAngle)) % 8) + 2;
}

void TurretBase::attack(const GameContext& context) {
    if((weaponTimer == 0) && (target.getObjPointer() != nullptr)) {
        const auto centerPoint = getCenterPoint();
        auto *const pObject = target.getObjPointer();
        const auto targetCenterPoint = pObject->getClosestCenterPoint(location);

        const auto& [game, map, objectManager] = context;

        map.add_bullet(objectID, &centerPoint, &targetCenterPoint, turret_constants().bulletType(),
                                   game.objectData.data[itemID][static_cast<int>(originalHouseID)].weapondamage,
                                   pObject->isAFlyingUnit(), pObject);

        map.viewMap(pObject->getOwner()->getHouseID(), location, 2);
        soundPlayer->playSoundAt(attackSound, location);
        weaponTimer = getWeaponReloadTime();
    }
}
