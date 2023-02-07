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

#include <DataTypes.h>
#include <ObjectPointer.h>
#include <fixmath/FixPoint.h>
#include <misc/InputStream.h>
#include <misc/OutputStream.h>

// forward declarations
class GameContext;
class House;

class Bullet final {
public:
    Bullet(uint32_t shooterID, const Coord* newRealLocation, const Coord* newRealDestination, uint32_t bulletID,
           int damage, bool air, const ObjectBase* pTarget);
    explicit Bullet(InputStream& stream);
    void init();
    ~Bullet();

    Bullet(const Bullet&)            = delete;
    Bullet(Bullet&&)                 = delete;
    Bullet& operator=(const Bullet&) = delete;
    Bullet& operator=(Bullet&&)      = delete;

    void save(OutputStream& stream) const;

    void blitToScreen(uint32_t cycleCount) const;

    bool update(const GameContext& context);
    void destroy(const GameContext& context) const;

    auto getBulletID() const noexcept { return bulletID_; }
    auto getRealX() const noexcept { return realX_; }
    auto getRealY() const noexcept { return realY_; }

private:
    // constants for each bullet type
    FixPoint speed_;               ///< The speed of this bullet
    int damageRadius_;             ///< The radius of the bullet
    bool explodesAtGroundObjects_; ///< false = bullet goes through objects, true = bullet explodes at ground objects
    int8_t detonationTimer_;       ///< How long is this bullet alive before it explodes

    // bullet state
    uint32_t bulletID_; ///< The ID of the bullet

    int32_t damage_; ///< the damage this bullet causes

    uint32_t shooterID_; ///< the ItemId of the shooter
    House* owner_;       ///< the owner of this bullet

    Coord source_;      ///< the source location (in world coordinates) of this bullet
    Coord destination_; ///< the destination (in world coordinates) of this bullet
    Coord location_;    ///< the current location of this bullet (in map coordinates)
    FixPoint realX_;    ///< the x-coordinate of the current position (in world coordinates)
    FixPoint realY_;    ///< the y-coordinate of the current position (in world coordinates)

    FixPoint xSpeed_; ///< Speed in x direction
    FixPoint ySpeed_; ///< Speed in x direction

    FixPoint angle_;    ///< the angle of the bullet
    int8_t drawnAngle_; ///< the drawn angle of the bullet

    bool airAttack_;       ///< Is this an air attack?
    ObjectPointer target_; ///< The target to hit

    // drawing information
    zoomable_texture graphic_{}; ///< The graphic of the bullet
    int numFrames_ = 0;          ///< Number of frames of the bullet
};

#endif // BULLET_H
