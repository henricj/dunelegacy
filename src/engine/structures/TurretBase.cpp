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

#include "engine_mmath.h"

#include <structures/TurretBase.h>

#include <Game.h>
#include <Map.h>
#include <House.h>
#include <Bullet.h>

#include <players/HumanPlayer.h>

namespace Dune::Engine {

TurretBase::TurretBase(const TurretBaseConstants& constants, uint32_t objectID, const ObjectInitializer& initializer)
    : StructureBase(constants, objectID, initializer) {
    TurretBase::init();

    angle      = initializer.game().randomGen.rand(0, 7);
    drawnAngle = static_cast<ANGLETYPE>(lround(angle));

    findTargetTimer = 0;
    weaponTimer     = 0;
}

TurretBase::TurretBase(const TurretBaseConstants& constants, uint32_t objectID,
                       const ObjectStreamInitializer& initializer)
    : StructureBase(constants, objectID, initializer) {
    TurretBase::init();

    auto& stream = initializer.stream();

    findTargetTimer = stream.readSint32();
    weaponTimer     = stream.readSint32();
}

void TurretBase::init() {
    attackMode = AREAGUARD;
}

TurretBase::~TurretBase() = default;

void TurretBase::save(const Game& game, OutputStream& stream) const {
    parent::save(game, stream);

    stream.writeSint32(findTargetTimer);
    stream.writeSint32(weaponTimer);
}

void TurretBase::updateStructureSpecificStuff(const GameContext& context) {
    auto* const pTarget = target.getObjPointer(context.objectManager);

    if(pTarget) {
        if(!canAttack(context, pTarget) || !targetInWeaponRange(context)) {
            setTarget(context.objectManager, nullptr);
        } else if(targetInWeaponRange(context)) {
            Coord      closestPoint = pTarget->getClosestPoint(location);
            const auto wantedAngle  = destinationDrawnAngle(location, closestPoint);

            if(angle != static_cast<int>(wantedAngle)) {
                // turn
                FixPoint angleLeft  = 0;
                FixPoint angleRight = 0;

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

            if(drawnAngle == wantedAngle) { attack(context); }

        } else {
            setTarget(context.objectManager, nullptr);
        }
    } else if((attackMode != STOP) && (findTargetTimer == 0)) {
        setTarget(context.objectManager, findTarget(context));
        findTargetTimer = 100;
    }

    if(findTargetTimer > 0) { findTargetTimer--; }

    if(weaponTimer > 0) { weaponTimer--; }
}

void TurretBase::doAttackObject(const GameContext& context, uint32_t targetObjectID) {
    const auto* pObject = context.objectManager.getObject(targetObjectID);
    if(pObject) doAttackObject(context, pObject);
}

void TurretBase::doAttackObject(const GameContext& context, const ObjectBase* pObject) {
    if(pObject == nullptr) { return; }

    setDestination(context, INVALID_POS, INVALID_POS);
    setTarget(context.objectManager, pObject);
    setForced(true);
}

void TurretBase::turnLeft(const GameContext& context) {
    angle += context.game.getObjectData(itemID, originalHouseID).turnspeed;
    if(angle >= 7.5_fix) // must keep drawnangle between 0 and 7
        angle -= 8;
    drawnAngle   = static_cast<ANGLETYPE>(lround(angle));
}

void TurretBase::turnRight(const GameContext& context) {
    angle -= context.game.getObjectData(itemID, originalHouseID).turnspeed;
    if(angle < -0.5_fix) {
        // must keep angle between 0 and 7
        angle += 8;
    }
    drawnAngle   = static_cast<ANGLETYPE>(lround(angle));
}

void TurretBase::attack(const GameContext& context) {
    if(weaponTimer != 0) return;

    const auto& [game, map, objectManager] = context;

    auto* const pObject = target.getObjPointer(objectManager);

    if(pObject == nullptr) return;

    const auto centerPoint       = getCenterPoint();
    const auto targetCenterPoint = pObject->getClosestCenterPoint(location);

    game.add_bullet(context, objectID, &centerPoint, &targetCenterPoint, turret_constants().bulletType(),
                    game.getObjectData(itemID, originalHouseID).weapondamage,
                    pObject->isAFlyingUnit(), pObject);

    map.viewMap(pObject->getOwner()->getHouseID(), location, 2);
    weaponTimer = getWeaponReloadTime(context.game);
}

} // namespace Dune::Engine
