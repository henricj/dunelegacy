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

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>
#include <Map.h>
#include <Explosion.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

#include <structures/StructureBase.h>

#define RANDOMTURRETTURNTIMER 8000    //less of this makes tank turrets randomly turn more

TankBase::TankBase(ItemID_enum itemID, Uint32 objectID, const ObjectInitializer& initializer) : TrackedUnit(itemID, objectID, initializer) {
    TankBase::init();

    drawnTurretAngle = static_cast<ANGLETYPE>(currentGame->randomGen.rand(0, 7));
    turretAngle = static_cast<int>(drawnTurretAngle);
}

TankBase::TankBase(ItemID_enum itemID, Uint32 objectID, const ObjectStreamInitializer& initializer) : TrackedUnit(itemID, objectID, initializer) {
    TankBase::init();

    auto& stream     = initializer.Stream;

    turretAngle      = stream.readFixPoint();
    drawnTurretAngle = static_cast<ANGLETYPE>(stream.readSint8());

    closeTarget.load(stream);
}

void TankBase::init() {
    turreted = true;
}

TankBase::~TankBase() = default;

void TankBase::save(OutputStream& stream) const {
    parent::save(stream);

    stream.writeFixPoint(turretAngle);
    stream.writeSint8(static_cast<Sint8>(drawnTurretAngle));

    closeTarget.save(stream);
}

void TankBase::setTurretAngle(ANGLETYPE newAngle) {
    if((static_cast<int>(newAngle) >= 0) && (newAngle < ANGLETYPE::NUM_ANGLES)) {
        drawnTurretAngle = newAngle;
        turretAngle = static_cast<int>(drawnTurretAngle);
    }
}

ANGLETYPE TankBase::getCurrentAttackAngle() const {
    return drawnTurretAngle;
}

void TankBase::navigate(const GameContext& context) {
    if(moving && !justStoppedMoving) {
        if(location == destination) {
            targetAngle = ANGLETYPE::INVALID_ANGLE;
        } else {
            // change the turret angle so it faces the direction we are moving in
            targetAngle = destinationDrawnAngle(location, destination);
        }
    }

    parent::navigate(context);
}

void TankBase::idleAction(const GameContext& context) {
    if(getAttackMode() == GUARD) {
        auto& randomGen = context.game.randomGen;

        // do some random turning with 20% chance
        switch(randomGen.rand(0, 9)) {
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


    if(closeTarget && (closeTarget.getObjPointer() == nullptr)) {
        // the target does not exist anymore
        // => release target
        closeTarget.pointTo(NONE_ID);
        return;
    }

    if(closeTarget && !closeTarget.getObjPointer()->isActive()) {
        // the target changed its state to inactive
        // => release target
        closeTarget.pointTo(NONE_ID);
        return;
    }

    if(closeTarget && !canAttack(closeTarget.getObjPointer())) {
        // the target cannot be attacked anymore
        // => release target
        closeTarget.pointTo(NONE_ID);
        return;
    }

    if(target && (targetDistance <= getWeaponRange()) && !targetFriendly) {
        // we already have a (non-friendly) target in weapon range
        // => we need no close temporary target
        closeTarget.pointTo(NONE_ID);
        return;
    }

    if(closeTarget) {
        Coord targetLocation = closeTarget.getObjPointer()->getClosestPoint(location);
        FixPoint closeTargetDistance = blockDistance(location, targetLocation);

        if(closeTargetDistance > getWeaponRange()) {
            // we are too far away
            closeTarget.pointTo(NONE_ID);
            return;
        }

        targetAngle = destinationDrawnAngle(location, targetLocation);

        if(drawnTurretAngle == targetAngle) {
            ObjectPointer temp = target;
            target = closeTarget;
            attack(context);
            target = temp;
        }
    }
}

void TankBase::targeting(const GameContext& context) {
    if(findTargetTimer == 0) {
        if(attackMode != STOP && !closeTarget && !moving && !justStoppedMoving) {
            // find a temporary target
            closeTarget = findTarget();
        }
    }

    parent::targeting(context);
}

void TankBase::turn(const GameContext& context) {
    FixPoint angleLeft = 0;
    FixPoint angleRight = 0;

    if(!moving && !justStoppedMoving) {
        if(nextSpotAngle != ANGLETYPE::INVALID_ANGLE) {
            if(angle > static_cast<int>(nextSpotAngle)) {
                angleRight = angle - static_cast<int>(nextSpotAngle);
                angleLeft = FixPoint::abs(8-angle) + static_cast<int>(nextSpotAngle);
            } else if (angle < static_cast<int>(nextSpotAngle)) {
                angleRight = FixPoint::abs(8-static_cast<int>(nextSpotAngle)) + angle;
                angleLeft = static_cast<int>(nextSpotAngle) - angle;
            }

            if(angleLeft <= angleRight) {
                turnLeft(context);
            } else {
                turnRight(context);
            }
        }
    }

    if(targetAngle != ANGLETYPE::INVALID_ANGLE) {
        if(turretAngle > static_cast<int>(targetAngle)) {
            angleRight = turretAngle - static_cast<int>(targetAngle);
            angleLeft = FixPoint::abs(8-turretAngle) + static_cast<int>(targetAngle);
        } else if (turretAngle < static_cast<int>(targetAngle)) {
            angleRight = FixPoint::abs(8-static_cast<int>(targetAngle)) + turretAngle;
            angleLeft = static_cast<int>(targetAngle) - turretAngle;
        }

        if(angleLeft <= angleRight) {
            turnTurretLeft();
        } else {
            turnTurretRight();
        }
    }
}

void TankBase::turnTurretLeft() {
    turretAngle += turretTurnSpeed;
    if(turretAngle >= 7.5_fix) { turretAngle -= static_cast<int>(ANGLETYPE::NUM_ANGLES); }

    drawnTurretAngle = normalizeAngle(static_cast<ANGLETYPE>(lround(turretAngle)));
}

void TankBase::turnTurretRight() {
    turretAngle -= turretTurnSpeed;
    if(turretAngle <= -0.5_fix) { turretAngle += static_cast<int>(ANGLETYPE::NUM_ANGLES); }

    drawnTurretAngle = normalizeAngle(static_cast<ANGLETYPE>(lround(turretAngle)));
}
