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

TankBase::TankBase(House* newOwner) : TrackedUnit(newOwner) {
    TankBase::init();

    drawnTurretAngle = currentGame->randomGen.rand(0, 7);
    turretAngle = drawnTurretAngle;
}

TankBase::TankBase(InputStream& stream) : TrackedUnit(stream) {
    TankBase::init();

    turretAngle = stream.readFixPoint();
    drawnTurretAngle = stream.readSint8();

    closeTarget.load(stream);
}

void TankBase::init() {
    turreted = true;
}

TankBase::~TankBase() = default;

void TankBase::save(OutputStream& stream) const {
    TrackedUnit::save(stream);

    stream.writeFixPoint(turretAngle);
    stream.writeSint8(drawnTurretAngle);

    closeTarget.save(stream);
}

void TankBase::setTurretAngle(int newAngle) {
    if((newAngle >= 0) && (newAngle < NUM_ANGLES)) {
        turretAngle = drawnTurretAngle = newAngle;
    }
}

int TankBase::getCurrentAttackAngle() const {
    return drawnTurretAngle;
}

void TankBase::navigate() {
    if(moving && !justStoppedMoving) {
        if(location == destination) {
            targetAngle = INVALID;
        } else {
            // change the turret angle so it faces the direction we are moving in
            targetAngle = destinationDrawnAngle(location, destination);
        }
    }
    TrackedUnit::navigate();
}

void TankBase::idleAction() {
    if(getAttackMode() == GUARD) {
        // do some random turning with 20% chance
        switch(currentGame->randomGen.rand(0, 9)) {
            case 0: {
                // choose a random one of the eight possible angles
                nextSpotAngle = currentGame->randomGen.rand(0, 7);
            } break;

            case 1: {
                // choose a random one of the eight possible angles
                targetAngle = currentGame->randomGen.rand(0, 7);
            } break;

            default: {
                // no change
            } break;
        }
    }
}

void TankBase::engageTarget() {

    TrackedUnit::engageTarget();


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
            attack();
            target = temp;
        }
    }
}

void TankBase::targeting() {
    if(findTargetTimer == 0) {
        if(attackMode != STOP && !closeTarget && !moving && !justStoppedMoving) {
            // find a temporary target
            closeTarget = findTarget();
        }
    }

    TrackedUnit::targeting();
}

void TankBase::turn() {
    FixPoint angleLeft = 0;
    FixPoint angleRight = 0;

    if(!moving && !justStoppedMoving) {
        if(nextSpotAngle != INVALID) {
            if(angle > nextSpotAngle) {
                angleRight = angle - nextSpotAngle;
                angleLeft = FixPoint::abs(8-angle) + nextSpotAngle;
            } else if (angle < nextSpotAngle) {
                angleRight = FixPoint::abs(8-nextSpotAngle) + angle;
                angleLeft = nextSpotAngle - angle;
            }

            if(angleLeft <= angleRight) {
                turnLeft();
            } else {
                turnRight();
            }
        }
    }

    if(targetAngle != INVALID) {
        if(turretAngle > targetAngle) {
            angleRight = turretAngle - targetAngle;
            angleLeft = FixPoint::abs(8-turretAngle) + targetAngle;
        } else if (turretAngle < targetAngle) {
            angleRight = FixPoint::abs(8-targetAngle) + turretAngle;
            angleLeft = targetAngle - turretAngle;
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
    if(turretAngle >= 7.5_fix) {
        drawnTurretAngle = lround(turretAngle) - NUM_ANGLES;
        turretAngle -= NUM_ANGLES;
    } else {
        drawnTurretAngle = lround(turretAngle);
    }
}

void TankBase::turnTurretRight() {
    turretAngle -= turretTurnSpeed;
    if(turretAngle <= -0.5_fix) {
        drawnTurretAngle = lround(turretAngle) + NUM_ANGLES;
        turretAngle += NUM_ANGLES;
    } else {
        drawnTurretAngle = lround(turretAngle);
    }
}
