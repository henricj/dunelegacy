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
#include <fixmath/FixPoint.h>
#include <cmath>
#include <algorithm>

int getRandomInt();

inline bool getRandomBool() {
    return getRandomInt() % 2;
}


inline int getRandomInt(int min, int max) {
    max++;
    return ((getRandomInt() % (max-min)) + min);
}

/**
    This method returns randomly one of the given parameters.
    \return one of the parameters, e.g. getRandOf({13,17,19}) returns 13, 17 or 19
*/
template<typename T>
T getRandomOf(std::initializer_list<T> initlist) {
    return initlist.begin()[getRandomInt(0,initlist.size()-1)];
}

inline FixPoint destinationAngleRad(FixPoint x1, FixPoint y1, FixPoint x2, FixPoint y2)
{

    FixPoint diffX = x2 - x1;
    FixPoint diffY = -(y2 - y1);    // flip y

    if(diffX == 0 && diffY == 0) {
        return FixPt_PI/2;
    }

    FixPoint destAngle = FixPoint::atan2(diffY, diffX);

    if(destAngle < 0) {
        destAngle += (FixPt_PI << 1);   // add 360°
    }

    return destAngle;
}

inline FixPoint destinationAngleRad(const Coord& p1, const Coord& p2)
{
    return destinationAngleRad(p1.x, p1.y, p2.x, p2.y);
}

inline FixPoint RadToDeg256(FixPoint angleRad) {
    return (angleRad << 7)/FixPt_PI; // angleRad*256/(2*FixPt_PI)
}

inline FixPoint Deg256ToRad(FixPoint angle) { return (angle*FixPt_PI) >> 7; }   // angle*2*FixPt_PI/256;

/**
    Calculates the smaller angle between angle1 and angle2. The result is always positive,
    e.g. angleDiff(RIGHTUP, DOWN) = angleDiff(DOWN, RIGHTUP) = 5
    \param  angle1  the first angle
    \param  angle2  the second angle
    \return the angle between angle1 and angle2.
*/
inline int angleDiff(int angle1, int angle2) {
    int diff = abs(angle1 - angle2);
    return std::min(diff, NUM_ANGLES - diff);
}

inline int angleToDrawnAngle(FixPoint angle) {
    return lround(angle >> 5) & 0x7;  //  lround(angle*NUM_ANGLES/256) % NUM_ANGLES;
}

inline int destinationDrawnAngle(const Coord& p1, const Coord& p2) {
    return angleToDrawnAngle(RadToDeg256(destinationAngleRad(p1, p2)));
}

inline FixPoint distanceFrom(const Coord& p1, const Coord& p2)
{
    FixPoint first = (p1.x - p2.x);
    FixPoint second = (p1.y - p2.y);

    FixPoint z = FixPoint::sqrt(first*first + second*second);

    return z;
}

inline FixPoint distanceFrom(FixPoint x, FixPoint y, FixPoint to_x, FixPoint to_y)
{
    FixPoint first = (x - to_x);
    FixPoint second = (y - to_y);

    FixPoint z = FixPoint::sqrt(first*first + second*second);

    return z;
}

/**
    Calculates the maximum distances, that is max(abs(diffX), abs(diffY))
    \param  p1  first coordinate
    \param  p2  second coordinate
    \return the distance
*/
inline int maximumDistance(const Coord& p1, const Coord& p2) {
    return std::max(abs(p1.x - p2.x), abs(p1.y - p2.y));
}

/**
    Calculates the block distance, that is the distance when moving along blocks
    \param  p1  first coordinate
    \param  p2  second coordinate
    \return the distance
*/
inline FixPoint blockDistance(const Coord& p1, const Coord& p2) {
    int diffX = abs(p1.x - p2.x);
    int diffY = abs(p1.y - p2.y);

    if(diffX > diffY) {
        return diffX + diffY*(FixPt_SQRT2 - 1);
    } else {
        return diffX*(FixPt_SQRT2 - 1) + diffY;
    }
}

/**
    Calculates the block distance the same as the original, that is diffX + diffY/2 for diffX > diffY and diffX/2 + diffY for diffX <= diffY
    \param  p1  first coordinate
    \param  p2  second coordinate
    \return the distance
*/
inline int blockDistanceApprox(const Coord& p1, const Coord& p2) {
    int diffX = abs(p1.x - p2.x);
    int diffY = abs(p1.y - p2.y);

    if(diffX > diffY) {
        return ((diffX*2 + diffY) + 1)/2;
    } else {
        return ((diffX + diffY*2) + 1)/2;
    }
}

inline int mirrorAngleHorizontal(int angle) {
    switch(angle % NUM_ANGLES) {
        case RIGHT:     return LEFT;
        case RIGHTUP:   return LEFTUP;
        case UP:        return UP;
        case LEFTUP:    return RIGHTUP;
        case LEFT:      return RIGHT;
        case LEFTDOWN:  return RIGHTDOWN;
        case DOWN:      return DOWN;
        case RIGHTDOWN: return LEFTDOWN;
        default:        return angle;
    }
}



inline int mirrorAngleVertical(int angle) {
    switch(angle % NUM_ANGLES) {
        case RIGHT:     return RIGHT;
        case RIGHTUP:   return RIGHTDOWN;
        case UP:        return DOWN;
        case LEFTUP:    return LEFTDOWN;
        case LEFT:      return LEFT;
        case LEFTDOWN:  return LEFTUP;
        case DOWN:      return UP;
        case RIGHTDOWN: return RIGHTUP;
        default:        return angle;
    }
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


#endif //MMATH_H
