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

#ifndef BULLET_H
#define BULLET_H

#include <globals.h>

#include <ObjectPointer.h>
#include <DataTypes.h>
#include <ScreenBorder.h>
#include <misc/InputStream.h>
#include <misc/OutputStream.h>
#include <fixmath/FixPoint.h>

// forward declarations
class House;


class Bullet
{
public:
    Bullet(Uint32 shooterID, Coord* newRealLocation, Coord* newRealDestination, Uint32 bulletID, int damage, bool air, const ObjectBase* pTarget);
    explicit Bullet(InputStream& stream);
    void init();
    ~Bullet();

    Bullet(const Bullet &) = delete;
    Bullet(Bullet &&) = delete;
    Bullet& operator=(const Bullet &) = delete;
    Bullet& operator=(Bullet &&) = delete;

    void save(OutputStream& stream) const;

    void blitToScreen() const;

    void update();
    void destroy();

    inline int getBulletID() const { return bulletID; }
    inline FixPoint getRealX() const { return realX; }
    inline FixPoint getRealY() const { return realY; }

private:
    // constants for each bullet type
    int      damageRadius;               ///< The radius of the bullet
    bool     explodesAtGroundObjects;    ///< false = bullet goes through objects, true = bullet explodes at ground objects
    FixPoint speed;                      ///< The speed of this bullet
    Sint8    detonationTimer;            ///< How long is this bullet alive before it explodes

    // bullet state
    Uint32   bulletID;                   ///< The ID of the bullet

    Sint32   damage;                     ///< the damage this bullet causes

    Uint32   shooterID;                  ///< the ItemId of the shooter
    House*   owner;                      ///< the owner of this bullet

    Coord    source;                     ///< the source location (in world coordinates) of this bullet
    Coord    destination;                ///< the destination (in world coordinates) of this bullet
    Coord    location;                   ///< the current location of this bullet (in map coordinates)
    FixPoint realX;                      ///< the x-coordinate of the current position (in world coordinates)
    FixPoint realY;                      ///< the y-coordinate of the current position (in world coordinates)

    FixPoint xSpeed;                     ///< Speed in x direction
    FixPoint ySpeed;                     ///< Speed in x direction

    FixPoint angle;                      ///< the angle of the bullet
    Sint8    drawnAngle;                 ///< the drawn angle of the bullet

    bool     airAttack;                  ///< Is this an air attack?
    ObjectPointer target;                ///< The target to hit

    // drawing information
    zoomable_texture graphic{};          ///< The graphic of the bullet
    int              numFrames = 0;      ///< Number of frames of the bullet
};

#endif // BULLET_H
