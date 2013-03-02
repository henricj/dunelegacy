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

#ifndef STRICTMATH_H
#define STRICTMATH_H

#include <math.h>

namespace strictmath {

const float pi =  3.1415925026e+00f;

// misc functions
inline float abs(float x) {	return ::fabsf(x); /* standardized by IEEE 754 */ }
inline float floor(float x) { return ::floor(x); /* standardized by IEEE 754 */ }
inline float ceil(float x) { return ::ceil(x); /* standardized by IEEE 754 */ }
inline float fmod(float x, float y) { return ::fmod(x,y); /* standardized by IEEE 754 */ }

// trigonometric functions
float sin(float x);
float cos(float x);
float tan(float x);

float asin(float x);
float acos(float x);
float atan(float x);
float atan2(float y, float x);

inline float sqrt(float x) { return ::sqrtf(x); /* standardized by IEEE 754 */ }

} // namespace strictmath

#endif // STRICTMATH_H
