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

#include "mmath.h"
#include <Bullet.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <SoundPlayer.h>

#include <players/HumanPlayer.h>

TurretBase::TurretBase(const TurretBaseConstants& constants, uint32_t objectID, const ObjectInitializer& initializer)
    : StructureBase(constants, objectID, initializer) {
    TurretBase::init();

    angle_      = initializer.game().randomGen.rand(0, 7);
    drawnAngle_ = static_cast<ANGLETYPE>(lround(angle_));
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
    attackSound = Sound_enum::Sound_Gun;

    attackMode_ = AREAGUARD;
}

TurretBase::~TurretBase() = default;

void TurretBase::save(OutputStream& stream) const {
    StructureBase::save(stream);

    stream.writeSint32(findTargetTimer);
    stream.writeSint32(weaponTimer);
}

void TurretBase::updateStructureSpecificStuff(const GameContext& context) {
    if (target_ && (target_.getObjPointer() != nullptr)) {
        if (!canAttack(target_.getObjPointer()) || !targetInWeaponRange()) {
            setTarget(nullptr);
        } else if (targetInWeaponRange()) {
            const auto closestPoint = target_.getObjPointer()->getClosestPoint(location_);
            const auto wantedAngle  = destinationDrawnAngle(location_, closestPoint);

            if (angle_ != static_cast<int>(wantedAngle)) {
                // turn
                FixPoint angleLeft  = 0;
                FixPoint angleRight = 0;

                if (angle_ > static_cast<int>(wantedAngle)) {
                    angleRight = angle_ - static_cast<int>(wantedAngle);
                    angleLeft  = FixPoint::abs(NUM_ANGLES - angle_) + static_cast<int>(wantedAngle);
                } else if (angle_ < static_cast<int>(wantedAngle)) {
                    angleRight = FixPoint::abs(NUM_ANGLES - static_cast<int>(wantedAngle)) + angle_;
                    angleLeft  = static_cast<int>(wantedAngle) - angle_;
                }

                if (angleLeft <= angleRight) {
                    turnLeft(context);
                } else {
                    turnRight(context);
                }
            }

            if (drawnAngle_ == wantedAngle) {
                attack(context);
            }

        } else {
            setTarget(nullptr);
        }
    } else if ((attackMode_ != STOP) && (findTargetTimer == 0)) {
        setTarget(findTarget());
        findTargetTimer = 100;
    }

    if (findTargetTimer > 0) {
        findTargetTimer--;
    }

    if (weaponTimer > 0) {
        weaponTimer--;
    }
}

void TurretBase::handleActionCommand(const GameContext& context, int xPos, int yPos) {
    auto& [game, map, objectManager] = context;

    if (const auto* tile = map.tryGetTile(xPos, yPos)) {
        const auto* tempTarget = tile->getObject(objectManager);
        game.getCommandManager().addCommand(Command(dune::globals::pLocalPlayer->getPlayerID(),
                                                    CMDTYPE::CMD_TURRET_ATTACKOBJECT, objectID_,
                                                    tempTarget->getObjectID()));
    }
}

void TurretBase::doAttackObject(const GameContext& context, uint32_t targetObjectID) {
    const auto* pObject = context.objectManager.getObject(targetObjectID);
    doAttackObject(pObject);
}

void TurretBase::doAttackObject(const ObjectBase* pObject) {
    if (pObject == nullptr) {
        return;
    }

    setDestination(INVALID_POS, INVALID_POS);
    setTarget(pObject);
    setForced(true);
}

void TurretBase::turnLeft(const GameContext& context) {
    angle_ += context.game.objectData.data[itemID_][static_cast<int>(originalHouseID_)].turnspeed;
    if (angle_ >= 7.5_fix) // must keep drawnangle between 0 and 7
        angle_ -= 8;
    drawnAngle_  = static_cast<ANGLETYPE>(lround(angle_));
    curAnimFrame = firstAnimFrame = lastAnimFrame = ((10 - static_cast<int>(drawnAngle_)) % 8) + 2;
}

void TurretBase::turnRight(const GameContext& context) {
    angle_ -= context.game.objectData.data[itemID_][static_cast<int>(originalHouseID_)].turnspeed;
    if (angle_ < -0.5_fix) {
        // must keep angle between 0 and 7
        angle_ += 8;
    }
    drawnAngle_  = static_cast<ANGLETYPE>(lround(angle_));
    curAnimFrame = firstAnimFrame = lastAnimFrame = ((10 - static_cast<int>(drawnAngle_)) % 8) + 2;
}

void TurretBase::attack(const GameContext& context) {
    if ((weaponTimer == 0) && (target_.getObjPointer() != nullptr)) {
        const auto centerPoint       = getCenterPoint();
        auto* const pObject          = target_.getObjPointer();
        const auto targetCenterPoint = pObject->getClosestCenterPoint(location_);

        const auto& [game, map, objectManager] = context;

        map.add_bullet(objectID_, &centerPoint, &targetCenterPoint, turret_constants().bulletType(),
                       game.objectData.data[itemID_][static_cast<int>(originalHouseID_)].weapondamage,
                       pObject->isAFlyingUnit(), pObject);

        map.viewMap(pObject->getOwner()->getHouseID(), location_, 2);
        dune::globals::soundPlayer->playSoundAt(attackSound, location_);
        weaponTimer = getWeaponReloadTime();
    }
}
