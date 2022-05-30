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

#ifndef MMATH_H
#define MMATH_H

// forward declaration
class Coord;

#include <DataTypes.h>
#include <algorithm>
#include <cmath>
#include <fixmath/FixPoint.h>

inline FixPoint destinationAngleRad(FixPoint x1, FixPoint y1, FixPoint x2, FixPoint y2) {

    const FixPoint diffX = x2 - x1;
    const FixPoint diffY = -(y2 - y1); // flip y

    if (diffX == 0 && diffY == 0) {
        return FixPt_PI / 2;
    }

    FixPoint destAngle = FixPoint::atan2(diffY, diffX);

    if (destAngle < 0) {
        destAngle += (FixPt_PI << 1); // add 360°
    }

    return destAngle;
}

inline FixPoint destinationAngleRad(const Coord& p1, const Coord& p2) {
    return destinationAngleRad(p1.x, p1.y, p2.x, p2.y);
}

inline FixPoint RadToDeg256(FixPoint angleRad) {
    return (angleRad << 7) / FixPt_PI; // angleRad*256/(2*FixPt_PI)
}

inline FixPoint Deg256ToRad(FixPoint angle) {
    return (angle * FixPt_PI) >> 7;
} // angle*2*FixPt_PI/256;

/**
    Calculates the smaller angle between angle1 and angle2. The result is always positive,
    e.g. angleDiff(RIGHTUP, DOWN) = angleDiff(DOWN, RIGHTUP) = 5
    \param  angle1  the first angle
    \param  angle2  the second angle
    \return the angle between angle1 and angle2.
*/
inline int angleDiff(ANGLETYPE angle1, ANGLETYPE angle2) {
    const int diff = std::abs(static_cast<int>(angle1) - static_cast<int>(angle2));
    return std::min(diff, NUM_ANGLES - diff);
}

inline ANGLETYPE angleToDrawnAngle(FixPoint angle) {
    return static_cast<ANGLETYPE>(lround(angle >> 5) & 0x7); //  lround(angle*NUM_ANGLES/256) % NUM_ANGLES;
}

inline ANGLETYPE destinationDrawnAngle(const Coord& p1, const Coord& p2) {
    return angleToDrawnAngle(RadToDeg256(destinationAngleRad(p1, p2)));
}

inline FixPoint distanceFrom(const Coord& p1, const Coord& p2) {
    const FixPoint first  = (p1.x - p2.x);
    const FixPoint second = (p1.y - p2.y);

    const FixPoint z = FixPoint::sqrt(first * first + second * second);

    return z;
}

inline FixPoint distanceFrom(FixPoint x, FixPoint y, FixPoint to_x, FixPoint to_y) {
    const FixPoint first  = (x - to_x);
    const FixPoint second = (y - to_y);

    const FixPoint z = FixPoint::sqrt(first * first + second * second);

    return z;
}

/**
    Calculates the maximum distances, that is max(abs(diffX), abs(diffY))
    \param  p1  first coordinate
    \param  p2  second coordinate
    \return the distance
*/
inline int maximumDistance(const Coord& p1, const Coord& p2) {
    return std::max(std::abs(p1.x - p2.x), std::abs(p1.y - p2.y));
}

/**
    Calculates the block distance, that is the distance when moving along blocks
    \param  p1  first coordinate
    \param  p2  second coordinate
    \return the distance
*/
inline FixPoint blockDistance(const Coord& p1, const Coord& p2) {
    const int diffX = std::abs(p1.x - p2.x);
    const int diffY = std::abs(p1.y - p2.y);

    if (diffX > diffY) {
        return diffX + diffY * (FixPt_SQRT2 - 1);
    }
    return diffX * (FixPt_SQRT2 - 1) + diffY;
}

/**
    Calculates the block distance the same as the original, that is diffX + diffY/2 for diffX > diffY and diffX/2 +
   diffY for diffX <= diffY \param  p1  first coordinate \param  p2  second coordinate \return the distance
*/
inline int blockDistanceApprox(const Coord& p1, const Coord& p2) {
    const int diffX = std::abs(p1.x - p2.x);
    const int diffY = std::abs(p1.y - p2.y);

    if (diffX > diffY) {
        return ((diffX * 2 + diffY) + 1) / 2;
    }
    return ((diffX + diffY * 2) + 1) / 2;
}

constexpr ANGLETYPE normalizeAngle(ANGLETYPE angle) {
    const auto int_angle = static_cast<int>(angle);

    if (int_angle >= 0 && int_angle < NUM_ANGLES)
        return angle;

    auto mod_angle = int_angle % NUM_ANGLES;
    if (int_angle < 0)
        mod_angle += NUM_ANGLES;

    return static_cast<ANGLETYPE>(mod_angle);
}

constexpr ANGLETYPE mirrorAngleHorizontal(ANGLETYPE angle) {
    angle = normalizeAngle(angle);

    // clang-format off
    switch(angle) {
        case ANGLETYPE::RIGHT:     return ANGLETYPE::LEFT;
        case ANGLETYPE::RIGHTUP:   return ANGLETYPE::LEFTUP;
        case ANGLETYPE::UP:        return ANGLETYPE::UP;
        case ANGLETYPE::LEFTUP:    return ANGLETYPE::RIGHTUP;
        case ANGLETYPE::LEFT:      return ANGLETYPE::RIGHT;
        case ANGLETYPE::LEFTDOWN:  return ANGLETYPE::RIGHTDOWN;
        case ANGLETYPE::DOWN:      return ANGLETYPE::DOWN;
        case ANGLETYPE::RIGHTDOWN: return ANGLETYPE::LEFTDOWN;
        default:
            assert(false);
            return angle;
    }
    // clang-format on
}

constexpr ANGLETYPE mirrorAngleVertical(ANGLETYPE angle) {
    angle = normalizeAngle(angle);

    // clang-format off
    switch(angle) {
        case ANGLETYPE::RIGHT:     return ANGLETYPE::RIGHT;
        case ANGLETYPE::RIGHTUP:   return ANGLETYPE::RIGHTDOWN;
        case ANGLETYPE::UP:        return ANGLETYPE::DOWN;
        case ANGLETYPE::LEFTUP:    return ANGLETYPE::LEFTDOWN;
        case ANGLETYPE::LEFT:      return ANGLETYPE::LEFT;
        case ANGLETYPE::LEFTDOWN:  return ANGLETYPE::LEFTUP;
        case ANGLETYPE::DOWN:      return ANGLETYPE::UP;
        case ANGLETYPE::RIGHTDOWN: return ANGLETYPE::RIGHTUP;
        default:
            assert(false);
            return angle;
    }
    // clang-format on
}

/**
    This method converts between world coordinates and zoomed world coordinates.
    \param  x   a coordinate in world coordinates
    \return a coordinate which is converted by the current zoom level
*/
int world2zoomedWorld(int x);

/**
    This method converts between world coordinates and zoomed world coordinates.
    \param  x   a coordinate in world coordinates
    \return a coordinate which is converted by the current zoom level
*/
int world2zoomedWorld(float x);

/**
    This method converts between world coordinates and zoomed world coordinates.
    \param  coord   a coordinate in world coordinates
    \return a coordinate which is converted by the current zoom level
*/
Coord world2zoomedWorld(const Coord& coord);

/**
    This method converts between world coordinates and zoomed world coordinates.
    \param  x   a coordinate in world coordinates
    \return a coordinate which is converted by the current zoom level
*/
int zoomedWorld2world(int x);

/**
    This method converts between world coordinates and zoomed world coordinates.
    \param  coord   a coordinate in world coordinates
    \return a coordinate which is converted by the current zoom level
*/
Coord zoomedWorld2world(const Coord& coord);

#endif // MMATH_H
