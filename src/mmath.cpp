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

#include <mmath.h>

#include <Definitions.h>
#include <DataTypes.h>

#include <stdarg.h>
#include <algorithm>
#include <limits>

#include <misc/strictmath.h>

extern int currentZoomlevel;


float fixFloat(float number)
{
	if(strictmath::abs(number - (float)((int)number)) < 0.00001f) {
		number = (float)((int)number);
	}

	return number;
}


int getRandomInt(int min, int max)
{
	max++;
	return ((rand() % (max-min)) + min);
}


int getRandomOf(int numParam, ...) {
	int nthParam = getRandomInt(0,numParam-1);

	va_list arg_ptr;
	va_start(arg_ptr, numParam);

	int ret = va_arg(arg_ptr, int);

	for(int i = 1; i <= nthParam; i++) {
		ret = va_arg(arg_ptr, int);
	}
	va_end(arg_ptr);

	return ret;
}




float destinationAngle(const Coord& p1, const Coord& p2)
{
    float destAngle = 0.0f;

    float diffX = p2.x - p1.x;
    float diffY = p2.y - p1.y;

    if(diffX != 0) {
        if((diffX >= 0) && (diffY >= 0)) {
            //if x2 >= x1 && y2 >= y1
            destAngle = 2.0f * strictmath::pi - strictmath::atan(strictmath::abs(diffY/diffX));
        } else if ((diffX < 0) && (diffY >= 0)) {
            //if x2 < x1 && y2 >= y1
            destAngle = strictmath::atan(strictmath::abs(diffY/diffX)) + strictmath::pi;
        } else if ((diffX < 0) && (diffY < 0)) {
            //if x2 < x1 && y2 < y1
            destAngle = strictmath::pi - strictmath::atan(strictmath::abs(diffY/diffX));
        } else if ((diffX >= 0) && (diffY < 0)) {
            //if x2 >= x1 && y2 < y1
            destAngle = strictmath::atan(strictmath::abs(diffY/diffX));
        }
    } else if (diffY <= 0) {
        destAngle = strictmath::pi/2.0f;
    } else {
        destAngle = 3.0f*strictmath::pi/2.0f;
    }

    if(destAngle == 2.0f*strictmath::pi) {
        destAngle = 0.0f;
    }

    return (destAngle*256.0f/(2.0f*strictmath::pi));
}


FixPoint distanceFrom(const Coord& p1, const Coord& p2)
{
	FixPoint first = (p1.x - p2.x);
	FixPoint second = (p1.y - p2.y);

	FixPoint z = FixPoint::sqrt(first*first + second*second);

	return z;
}

FixPoint distanceFrom(FixPoint x, FixPoint y, FixPoint to_x, FixPoint to_y)
{
	FixPoint first = (x - to_x);
    FixPoint second = (y - to_y);

    FixPoint z = FixPoint::sqrt(first*first + second*second);

	return z;
}

float blockDistance(const Coord& p1, const Coord& p2)
{
	int xDis = abs(p1.x - p2.x),
		yDis = abs(p1.y - p2.y),

		minDis = std::min(xDis, yDis);

	return ((float)std::max(xDis, yDis) + (float)minDis*(DIAGONALCOST - 1.0f));
	//return (((float)minDis)*DIAGONALCOST + max(xDis, yDis) - minDis);
}


// Move one square away from the enemy
Coord retreatLocation(const Coord& p1, const Coord& p2)
{
	int j = 0;
	int k = 0;

	if(p1.x < p2.x){
        j = p1.x - 1;
	}

	if(p1.x > p2.x){
        j = p1.x + 1;
	}

    if(p1.y < p2.y){
        k = p1.y - 1;
	}

    if(p1.y > p2.y){
        k = p1.y + 1;
	}

    return Coord(j,k);

}




int mirrorAngleHorizontal(int angle) {
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



int mirrorAngleVertical(int angle) {
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


int world2zoomedWorld(int x) {
    if(x<0) {
        switch(currentZoomlevel) {
            case 0:     return (x-3)/4;
            case 1:     return (x-1)/2;
            case 2:     return ((x-1)*3)/4;
            case 3:
            default:    return x;
        }
    } else {
        switch(currentZoomlevel) {
            case 0:     return x/4;
            case 1:     return x/2;
            case 2:     return (x*3)/4;
            case 3:
            default:    return x;
        }
    }
}


int world2zoomedWorld(float x) {
    switch(currentZoomlevel) {
        case 0:     return lround(x*0.25f);
        case 1:     return lround(x*0.5f);
        case 2:     return lround(x*0.75f);
        case 3:
        default:    return lround(x);
    }
}


Coord world2zoomedWorld(const Coord& coord) {
    return Coord(world2zoomedWorld(coord.x), world2zoomedWorld(coord.y));
}


int zoomedWorld2world(int x) {
    switch(currentZoomlevel) {
        case 0:     return x*4;
        case 1:     return x*2;
        case 2:     return (x*4)/3;
        case 3:
        default:    return x;
    }
}


Coord zoomedWorld2world(const Coord& coord) {
    return Coord(zoomedWorld2world(coord.x), zoomedWorld2world(coord.y));
}


//// floating point check begin

/*
	Check if floating point values in an register have an higher precision than those in memory (e.g. x87 fpu):

		- load maximum floating point value into memory (y = max float)

		- load this value to a register (reg = y)

		- square it (reg2 = reg1*reg1)
		 => shall be infinity if we have the same precision for memory as for registers, otherwise it is a normal number

		- devide the result (still in register) by the largest floating point value (reg3 = reg2/reg1)
		 => infinity divided by a "normal" value is still infinity but a "normal" value divided by another "normal" value is a "normal" value

		- check if we have infinity
		 => if yes, memory and registers have the same precision, otherwise registers have extended precision

	As the test requires us to specify whether a variable is stored in memory or in a register, we need to make some extra work to ensure
	that no clever compiler optimizes the complete test out.
*/


void getMaxFloat(volatile float& result) {
	float x = std::numeric_limits<float>::max();
	result = x;
}

// global array of function pointers
void (*volatile pGetMaxFloat[2])(volatile float&) = { getMaxFloat, getMaxFloat };

bool checkForExcessPrecision() {

	// load maximum floating point value into memory
	volatile float y = 0.0f;
	pGetMaxFloat[rand() % 2](y);

	// load this value to a register, square it and devide the result (still in register) by the largest floating point value
	register float x = y;
	x = (x*x)/x;

	// check if we have infinity
	//	=> if yes, memory and registers have the same precision, otherwise registers have extended precision
	return !std::isinf(x);
}

//// floating point check end
