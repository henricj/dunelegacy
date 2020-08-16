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

#include <Bullet.h>

#include <ObjectBase.h>
#include <Game.h>
#include <Map.h>
#include <House.h>
#include <Explosion.h>

#include <misc/exceptions.h>
#include <engine_mmath.h>

#include <algorithm>


namespace Dune::Engine {

Bullet::Bullet(const GameContext& context, uint32_t shooterID, const Coord* newRealLocation, const Coord* newRealDestination, uint32_t bulletID,
               int damage, bool air, const ObjectBase* pTarget) {
    airAttack = air;

    this->shooterID = shooterID;

    this->owner = context.objectManager.getObject(shooterID)->getOwner();

    this->bulletID = bulletID;

    this->damage = damage;

    target.pointTo(pTarget);

    Bullet::init();

    destination = *newRealDestination;

    if(bulletID == Bullet_Sonic) {
        const auto diffX = destination.x - newRealLocation->x;
        auto       diffY = destination.y - newRealLocation->y;

        const int weaponrange =
            context.game.getObjectData(Unit_SonicTank, owner->getHouseID()).weaponrange;

        if((diffX == 0) && (diffY == 0)) { diffY = weaponrange * TILESIZE; }

        const auto square_root = FixPoint::sqrt(diffX * diffX + diffY * diffY);
        const auto ratio       = (weaponrange * TILESIZE) / square_root;
        destination.x          = newRealLocation->x + floor(diffX * ratio);
        destination.y          = newRealLocation->y + floor(diffY * ratio);
    } else if(bulletID == Bullet_Rocket || bulletID == Bullet_DRocket) {
        const auto distance = distanceFrom(*newRealLocation, *newRealDestination);

        const auto randAngle = 2 * FixPt_PI * context.game.randomGen.randFixPoint();
        const auto radius    = context.game.randomGen.rand(0, lround(TILESIZE / 2 + (distance / TILESIZE)));

        destination.x += lround(FixPoint::cos(randAngle) * radius);
        destination.y -= lround(FixPoint::sin(randAngle) * radius);
    }

    realX      = newRealLocation->x;
    realY      = newRealLocation->y;
    source.x   = newRealLocation->x;
    source.y   = newRealLocation->y;
    location.x = newRealLocation->x / TILESIZE;
    location.y = newRealLocation->y / TILESIZE;

    const auto angleRad = destinationAngleRad(*newRealLocation, *newRealDestination);
    angle               = RadToDeg256(angleRad);

    xSpeed = speed * FixPoint::cos(angleRad);
    ySpeed = speed * -FixPoint::sin(angleRad);
}

Bullet::Bullet(Game& game, InputStream& stream) {
    bulletID = stream.readUint32();

    airAttack = stream.readBool();
    target.load(stream);
    damage = stream.readSint32();

    shooterID = stream.readUint32();
    auto x = stream.readUint32();
    if(x >= static_cast<uint32_t>(HOUSETYPE::NUM_HOUSES)) x = 0;

    owner = game.getHouse(static_cast<HOUSETYPE>(x));

    source.x      = stream.readSint32();
    source.y      = stream.readSint32();
    destination.x = stream.readSint32();
    destination.y = stream.readSint32();
    location.x    = stream.readSint32();
    location.y    = stream.readSint32();
    realX         = stream.readFixPoint();
    realY         = stream.readFixPoint();

    xSpeed = stream.readFixPoint();
    ySpeed = stream.readFixPoint();

    angle      = stream.readFixPoint();

    detonationTimer = stream.readSint8();

    Bullet::init();
}

void Bullet::init() {
    explodesAtGroundObjects = false;

    switch(bulletID) {
        case Bullet_DRocket: {
            damageRadius    = TILESIZE / 2;
            speed           = 20;
            detonationTimer = 19;
        } break;

        case Bullet_LargeRocket: {
            damageRadius    = TILESIZE;
            speed           = 20;
            detonationTimer = -1;
        } break;

        case Bullet_Rocket: {
            damageRadius    = TILESIZE / 2;
            speed           = 17.5_fix;
            detonationTimer = 22;
        } break;

        case Bullet_TurretRocket: {
            damageRadius    = TILESIZE / 2;
            speed           = 20;
            detonationTimer = -1;
        } break;

        case Bullet_ShellSmall: {
            damageRadius            = TILESIZE / 2;
            explodesAtGroundObjects = true;
            speed                   = 20;
            detonationTimer         = -1;
        } break;

        case Bullet_ShellMedium: {
            damageRadius            = TILESIZE / 2;
            explodesAtGroundObjects = true;
            speed                   = 20;
            detonationTimer         = -1;
        } break;

        case Bullet_ShellLarge: {
            damageRadius            = TILESIZE / 2;
            explodesAtGroundObjects = true;
            speed                   = 20;
            detonationTimer         = -1;
        } break;

        case Bullet_ShellTurret: {
            damageRadius            = TILESIZE / 2;
            explodesAtGroundObjects = true;
            speed                   = 20;
            detonationTimer         = -1;
        } break;

        case Bullet_SmallRocket: {
            damageRadius    = TILESIZE / 2;
            speed           = 20;
            detonationTimer = 7;
        } break;

        case Bullet_Sonic: {
            damageRadius    = (TILESIZE * 3) / 4;
            speed           = 6; // For Sonic bullets this is only half the actual speed; see Bullet::update()
            detonationTimer = 45;
        } break;

        case Bullet_Sandworm: {
            THROW(std::domain_error, "Cannot init 'Bullet_Sandworm': Not allowed!");
        }

        default: {
            THROW(std::domain_error, "Unknown Bullet type %d!", bulletID);
        }
    }
}


void Bullet::save(OutputStream& stream) const {
    stream.writeUint32(bulletID);

    stream.writeBool(airAttack);
    target.save(stream);
    stream.writeSint32(damage);

    stream.writeUint32(shooterID);
    stream.writeUint32(static_cast<uint32_t>(owner->getHouseID()));

    stream.writeSint32(source.x);
    stream.writeSint32(source.y);
    stream.writeSint32(destination.x);
    stream.writeSint32(destination.y);
    stream.writeSint32(location.x);
    stream.writeSint32(location.y);
    stream.writeFixPoint(realX);
    stream.writeFixPoint(realY);

    stream.writeFixPoint(xSpeed);
    stream.writeFixPoint(ySpeed);

    stream.writeFixPoint(angle);

    stream.writeSint8(detonationTimer);
}

bool Bullet::update(const GameContext& context) {
    if(bulletID == Bullet_Rocket || bulletID == Bullet_DRocket || bulletID == Bullet_TurretRocket) {

        ObjectBase* pTarget = target.getObjPointer(context.objectManager);
        if(pTarget && pTarget->isAFlyingUnit()) { destination = pTarget->getCenterPoint(); }

        const auto angleToDestinationRad = destinationAngleRad(Coord(lround(realX), lround(realY)), destination);
        const auto angleToDestination    = RadToDeg256(angleToDestinationRad);

        auto angleDifference = angleToDestination - angle;
        if(angleDifference > 128) {
            angleDifference -= 256;
        } else if(angleDifference < -128) {
            angleDifference += 256;
        }

        static const FixPoint turnSpeed = 4.5_fix;

        if(angleDifference >= turnSpeed) {
            angleDifference = turnSpeed;
        } else if(angleDifference <= -turnSpeed) {
            angleDifference = -turnSpeed;
        }

        angle += angleDifference;

        if(angle < 0) {
            angle += 256;
        } else if(angle >= 256) {
            angle -= 256;
        }

        xSpeed = speed * FixPoint::cos(Deg256ToRad(angle));
        ySpeed = speed * -FixPoint::sin(Deg256ToRad(angle));
    }

    const auto oldDistanceToDestination = distanceFrom(realX, realY, destination.x, destination.y);

    realX += xSpeed; // keep the bullet moving by its current speeds
    realY += ySpeed;
    location.x = floor(realX / TILESIZE);
    location.y = floor(realY / TILESIZE);

    if((location.x < -5) || (location.x >= context.map.getSizeX() + 5) || (location.y < -5) ||
       (location.y >= context.map.getSizeY() + 5)) {
        // it's off the map => delete it
        return true;
    }

    const auto newDistanceToDestination = distanceFrom(realX, realY, destination.x, destination.y);

    if(detonationTimer > 0) { detonationTimer--; }

    auto& map = context.map;

    if(bulletID == Bullet_Sonic) {

        if(detonationTimer == 0) {
            destroy(context);
            return true;
        }

        const FixPoint weaponDamage =  context.game.getObjectData(Unit_SonicTank, owner->getHouseID()).weapondamage;

        const auto startDamage = (weaponDamage / 4 + 1) / 4.5_fix;
        const auto endDamage   = ((weaponDamage - 9) / 4 + 1) / 4.5_fix;

        const auto damageDecrease = -(startDamage - endDamage) / (45 * 2 * speed);
        const auto dist           = distanceFrom(source.x, source.y, realX, realY);

        const auto currentDamage = dist * damageDecrease + startDamage;

        auto realPos = Coord(lround(realX), lround(realY));
        map.damage(context, shooterID, owner, realPos, bulletID, currentDamage / 2, damageRadius, false);

        realX += xSpeed; // keep the bullet moving by its current speeds
        realY += ySpeed;

        realPos = Coord(lround(realX), lround(realY));
        map.damage(context, shooterID, owner, realPos, bulletID, currentDamage / 2, damageRadius, false);

        return false;
    }

    if(explodesAtGroundObjects) {
        auto* tile = map.tryGetTile(location.x, location.y);

        if(tile && tile->hasANonInfantryGroundObject()) {
            auto* structure = tile->getNonInfantryGroundObject(context.objectManager);

            if(structure && structure->isAStructure() &&
               ((bulletID != Bullet_ShellTurret) || (structure->getOwner() != owner))) {
                destroy(context);

                return true;
            }
        }
    }

    if(oldDistanceToDestination < newDistanceToDestination || newDistanceToDestination < 4) {

        if(bulletID == Bullet_Rocket || bulletID == Bullet_DRocket) {
            if(detonationTimer == 0) {
                destroy(context);

                return true;
            }
        } else {
            realX = destination.x;
            realY = destination.y;
            destroy(context);

            return true;
        }
    }

    return false;
}

void Bullet::destroy(const GameContext& context) const {
    auto position = Coord(lround(realX), lround(realY));

    const auto& [game, map, objectManager] = context;

    const auto houseID = owner->getHouseID();

    switch(bulletID) {
        case Bullet_DRocket: {
            map.damage(context, shooterID, owner, position, bulletID, damage, damageRadius, airAttack);
            game.addExplosion(Explosion_Gas, position, houseID);
        } break;

        case Bullet_LargeRocket: {

            for(auto i = 0; i < 5; i++) {
                for(auto j = 0; j < 5; j++) {
                    if(((i != 0) && (i != 4)) || ((j != 0) && (j != 4))) {
                        position.x = lround(realX) + (i - 2) * TILESIZE;
                        position.y = lround(realY) + (j - 2) * TILESIZE;

                        map.damage(context, shooterID, owner, position, bulletID, damage, damageRadius, airAttack);

                        uint32_t explosionID = game.randomGen.getRandOf(Explosion_Large1, Explosion_Large2);
                        game.addExplosion(explosionID, position, houseID);
                    }
                }
            }
        } break;

        case Bullet_Rocket:
        case Bullet_TurretRocket:
        case Bullet_SmallRocket: {
            map.damage(context, shooterID, owner, position, bulletID, damage, damageRadius, airAttack);
            game.addExplosion(Explosion_Small, position, houseID);
        } break;

        case Bullet_ShellSmall: {
            map.damage(context, shooterID, owner, position, bulletID, damage, damageRadius, airAttack);
            game.addExplosion(Explosion_ShellSmall, position, houseID);
        } break;

        case Bullet_ShellMedium: {
            map.damage(context, shooterID, owner, position, bulletID, damage, damageRadius, airAttack);
            game.addExplosion(Explosion_ShellMedium, position, houseID);
        } break;

        case Bullet_ShellLarge: {
            map.damage(context, shooterID, owner, position, bulletID, damage, damageRadius, airAttack);
            game.addExplosion(Explosion_ShellLarge, position, houseID);
        } break;

        case Bullet_ShellTurret: {
            map.damage(context, shooterID, owner, position, bulletID, damage, damageRadius, airAttack);
            game.addExplosion(Explosion_ShellMedium, position, houseID);
        } break;

        case Bullet_Sonic:
        case Bullet_Sandworm:
        default: {
            // do nothing
        } break;
    }
}

} // namespace Dune::Engine
