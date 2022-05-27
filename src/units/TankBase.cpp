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

#include <units/TankBase.h>

#include <globals.h>

#include <Explosion.h>
#include <FileClasses/GFXManager.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

#include <structures/StructureBase.h>

inline constexpr auto RANDOMTURRETTURNTIMER = 8000; // less of this makes tank turrets randomly turn more

TankBase::TankBase(const TankBaseConstants& constants, uint32_t objectID, const ObjectInitializer& initializer)
    : TrackedUnit(constants, objectID, initializer),
      drawnTurretAngle(static_cast<ANGLETYPE>(initializer.game().randomGen.rand(0, 7))) {

    turretAngle = static_cast<int>(drawnTurretAngle);
}

TankBase::TankBase(const TankBaseConstants& constants, uint32_t objectID, const ObjectStreamInitializer& initializer)
    : TrackedUnit(constants, objectID, initializer) {
    auto& stream = initializer.stream();

    turretAngle      = stream.readFixPoint();
    drawnTurretAngle = static_cast<ANGLETYPE>(stream.readSint8());

    closeTarget.load(stream);
}

TankBase::~TankBase() = default;

void TankBase::save(OutputStream& stream) const {
    parent::save(stream);

    stream.writeFixPoint(turretAngle);
    stream.writeSint8(static_cast<int8_t>(drawnTurretAngle));

    closeTarget.save(stream);
}

void TankBase::setTurretAngle(ANGLETYPE newAngle) {
    if ((static_cast<int>(newAngle) >= 0) && (newAngle < ANGLETYPE::NUM_ANGLES)) {
        drawnTurretAngle = newAngle;
        turretAngle      = static_cast<int>(drawnTurretAngle);
    }
}

ANGLETYPE TankBase::getCurrentAttackAngle() const {
    return drawnTurretAngle;
}

void TankBase::navigate(const GameContext& context) {
    if (moving && !justStoppedMoving) {
        if (location_ == destination_) {
            targetAngle = ANGLETYPE::INVALID_ANGLE;
        } else {
            // change the turret angle so it faces the direction we are moving in
            targetAngle = destinationDrawnAngle(location_, destination_);
        }
    }

    parent::navigate(context);
}

void TankBase::idleAction(const GameContext& context) {
    if (getAttackMode() == GUARD) {
        auto& randomGen = context.game.randomGen;

        // do some random turning with 20% chance
        switch (randomGen.rand(0, 9)) {
            case 0: {
                // choose a random one of the eight possible angles
                nextSpotAngle = static_cast<ANGLETYPE>(randomGen.rand(0, 7));
            } break;

            case 1: {
                // choose a random one of the eight possible angles
                targetAngle = static_cast<ANGLETYPE>(randomGen.rand(0, 7));
            } break;

            default: {
                // no change
            } break;
        }
    }
}

void TankBase::engageTarget(const GameContext& context) {

    parent::engageTarget(context);

    if (closeTarget && (closeTarget.getObjPointer() == nullptr)) {
        // the target does not exist anymore
        // => release target
        closeTarget.pointTo(NONE_ID);
        return;
    }

    if (closeTarget && !closeTarget.getObjPointer()->isActive()) {
        // the target changed its state to inactive
        // => release target
        closeTarget.pointTo(NONE_ID);
        return;
    }

    if (closeTarget && !canAttack(closeTarget.getObjPointer())) {
        // the target cannot be attacked anymore
        // => release target
        closeTarget.pointTo(NONE_ID);
        return;
    }

    if (target_ && (targetDistance <= getWeaponRange()) && !targetFriendly_) {
        // we already have a (non-friendly) target in weapon range
        // => we need no close temporary target
        closeTarget.pointTo(NONE_ID);
        return;
    }

    if (closeTarget) {
        const Coord targetLocation         = closeTarget.getObjPointer()->getClosestPoint(location_);
        const FixPoint closeTargetDistance = blockDistance(location_, targetLocation);

        if (closeTargetDistance > getWeaponRange()) {
            // we are too far away
            closeTarget.pointTo(NONE_ID);
            return;
        }

        targetAngle = destinationDrawnAngle(location_, targetLocation);

        if (drawnTurretAngle == targetAngle) {
            const ObjectPointer temp = target_;
            target_                  = closeTarget;
            attack(context);
            target_ = temp;
        }
    }
}

void TankBase::targeting(const GameContext& context) {
    if (findTargetTimer == 0) {
        if (attackMode_ != STOP && !closeTarget && !moving && !justStoppedMoving) {
            // find a temporary target
            closeTarget = findTarget();
        }
    }

    parent::targeting(context);
}

void TankBase::turn(const GameContext& context) {
    FixPoint angleLeft  = 0;
    FixPoint angleRight = 0;

    if (!moving && !justStoppedMoving) {
        if (nextSpotAngle != ANGLETYPE::INVALID_ANGLE) {
            if (angle_ > static_cast<int>(nextSpotAngle)) {
                angleRight = angle_ - static_cast<int>(nextSpotAngle);
                angleLeft  = FixPoint::abs(8 - angle_) + static_cast<int>(nextSpotAngle);
            } else if (angle_ < static_cast<int>(nextSpotAngle)) {
                angleRight = FixPoint::abs(8 - static_cast<int>(nextSpotAngle)) + angle_;
                angleLeft  = static_cast<int>(nextSpotAngle) - angle_;
            }

            if (angleLeft <= angleRight) {
                turnLeft(context);
            } else {
                turnRight(context);
            }
        }
    }

    if (targetAngle != ANGLETYPE::INVALID_ANGLE) {
        if (turretAngle > static_cast<int>(targetAngle)) {
            angleRight = turretAngle - static_cast<int>(targetAngle);
            angleLeft  = FixPoint::abs(8 - turretAngle) + static_cast<int>(targetAngle);
        } else if (turretAngle < static_cast<int>(targetAngle)) {
            angleRight = FixPoint::abs(8 - static_cast<int>(targetAngle)) + turretAngle;
            angleLeft  = static_cast<int>(targetAngle) - turretAngle;
        }

        if (angleLeft <= angleRight) {
            turnTurretLeft();
        } else {
            turnTurretRight();
        }
    }
}

void TankBase::turnTurretLeft() {
    turretAngle += turretTurnSpeed;
    if (turretAngle >= 7.5_fix) {
        turretAngle -= NUM_ANGLES;
    }

    drawnTurretAngle = normalizeAngle(static_cast<ANGLETYPE>(lround(turretAngle)));
}

void TankBase::turnTurretRight() {
    turretAngle -= turretTurnSpeed;
    if (turretAngle <= -0.5_fix) {
        turretAngle += NUM_ANGLES;
    }

    drawnTurretAngle = normalizeAngle(static_cast<ANGLETYPE>(lround(turretAngle)));
}
