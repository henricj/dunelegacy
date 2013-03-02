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

#include <cmath>

float fixFloat(float number);

int getRandomInt(int min, int max);
int getRandomOf(int numParam, ...);

float destinationAngle(const Coord& p1, const Coord& p2);

float distanceFrom(const Coord& p1, const Coord& p2);
float distanceFrom(float x, float y, float to_x, float to_y);

float blockDistance(const Coord& p1, const Coord& p2);

int mirrorAngleHorizontal(int angle);
int mirrorAngleVertical(int angle);

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


/**
    Check if floating point numbers are calculated internally with higher precision (like x87 does).
    \return true, if it is, false otherwise
*/
bool checkForExcessPrecision();
#endif //MMATH_H
