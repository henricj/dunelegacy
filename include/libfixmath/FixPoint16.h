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

#ifndef FIXPOINT16_H
#define FIXPOINT16_H

#include "fix16.h"

#include <string>

// we need to use a macro here as leading zeros in m are important: FixPt(1, 5) != FixPt(1, 05)
#define FixPt16(i,m) (FixPoint16::FromRaw(F16C(i,m)))

#define FixPt16_PI (FixPoint16::FromRaw(fix16_pi))
#define FixPt16_E (FixPoint16::FromRaw(fix16_e))

class FixPoint16
{
public:
	FixPoint16() { value = 0; }
	FixPoint16(const FixPoint16& inValue)        { value = inValue.value;             }
	FixPoint16(const int inValue)                { value = fix16_from_int(inValue); }
	FixPoint16(const unsigned int inValue)       { value = fix16_from_int(static_cast<int>(inValue)); }
	/*explicit*/ FixPoint16(const float inValue)   { value = fix16_from_float(inValue); }
//	/*explicit*/ FixPoint16(const double inValue)  { value = fix16_from_dbl(inValue);   }

	static FixPoint16 FromRaw(const fix16_t value) {
		FixPoint16 x;
		x.value = value;
		return x;
	}

	fix16_t getRaw() const { return value; }

	double toDouble() const { return fix16_to_dbl(value);   }
	float toFloat() const   { return fix16_to_float(value); }
	std::string toString() const {
		char buffer[16];
		fix16_to_str(value, buffer, 7);
		return std::string(buffer);
	}


//    operator double()  const { return fix16_to_dbl(value);   }
    operator float()   const { return fix16_to_float(value); }
    explicit operator int() const { return fix16_to_int(value);   }

	int roundToInt() const       { return fix16_to_int(value);   }

	FixPoint16 & operator=(const FixPoint16 rhs)     { value = rhs.value;             return *this; }
	FixPoint16 & operator=(const int rhs)            { value = fix16_from_int(rhs);   return *this; }
	FixPoint16 & operator=(const unsigned int rhs)   { return operator=(static_cast<int>(rhs)); }
	FixPoint16 & operator=(const float rhs)          { value = fix16_from_float(rhs); return *this; }

	FixPoint16 & operator+=(const FixPoint16 rhs)     { value += rhs.value;             return *this; }
	FixPoint16 & operator+=(const int rhs)            { value += fix16_from_int(rhs);   return *this; }
	FixPoint16 & operator+=(const unsigned int rhs)   { return operator+=(static_cast<int>(rhs)); }
	FixPoint16 & operator+=(const float rhs)          { value += fix16_from_float(rhs); return *this; }

	FixPoint16 & operator-=(const FixPoint16 rhs)     { value -= rhs.value; return *this; }
	FixPoint16 & operator-=(const int rhs)            { value -= fix16_from_int(rhs); return *this; }
	FixPoint16 & operator-=(const unsigned int rhs)   { return operator-=(static_cast<int>(rhs)); }
	FixPoint16 & operator-=(const float rhs)          { value -= fix16_from_float(rhs); return *this; }

	FixPoint16 & operator*=(const FixPoint16 rhs)     { value = fix16_mul(value, rhs.value); return *this; }
	FixPoint16 & operator*=(const int rhs)            { value *= rhs; return *this; }
	FixPoint16 & operator*=(const unsigned int rhs)   { return operator*=(static_cast<int>(rhs)); }
	FixPoint16 & operator*=(const float rhs)          { value = fix16_mul(value, fix16_from_float(rhs)); return *this; }

	FixPoint16 & operator/=(const FixPoint16 rhs)     { value = fix16_div(value, rhs.value); return *this; }
	FixPoint16 & operator/=(const int rhs)            { value /= rhs; return *this; }
	FixPoint16 & operator/=(const unsigned int rhs)   { return operator/=(static_cast<int>(rhs)); }
	FixPoint16 & operator/=(const float rhs)          { value = fix16_div(value, fix16_from_float(rhs)); return *this; }

	const FixPoint16 operator-() const { FixPoint16 ret = *this; ret.value = -ret.value; return ret; }

	const FixPoint16 operator+(const FixPoint16 other) const   { FixPoint16 ret = *this; ret += other; return ret; }
	const FixPoint16 operator+(const int other) const          { FixPoint16 ret = *this; ret += FixPoint16(other); return ret; }
	const FixPoint16 operator+(const unsigned int other) const { return operator+(static_cast<int>(other)); }
	const FixPoint16 operator+(const float other) const        { FixPoint16 ret = *this; ret += other; return ret; }

	const FixPoint16 operator-(const FixPoint16 other) const   { FixPoint16 ret = *this; ret -= other; return ret; }
	const FixPoint16 operator-(const int other) const          { FixPoint16 ret = *this; ret -= FixPoint16(other); return ret; }
	const FixPoint16 operator-(const unsigned int other) const { return operator-(static_cast<int>(other)); }
	const FixPoint16 operator-(const float other) const        { FixPoint16 ret = *this; ret -= other; return ret; }

	const FixPoint16 operator*(const FixPoint16 other) const   { FixPoint16 ret = *this; ret *= other; return ret; }
	const FixPoint16 operator*(const int other) const          { FixPoint16 ret = *this; ret *= FixPoint16(other); return ret; }
	const FixPoint16 operator*(const unsigned int other) const { return operator*(static_cast<int>(other)); }
	const FixPoint16 operator*(const float other) const        { FixPoint16 ret = *this; ret *= other; return ret; }

	const FixPoint16 operator/(const FixPoint16 other) const   { FixPoint16 ret = *this; ret /= other; return ret; }
	const FixPoint16 operator/(const int other) const          { FixPoint16 ret = *this; ret /= FixPoint16(other); return ret; }
	const FixPoint16 operator/(const unsigned int other) const { return operator/(static_cast<int>(other)); }
	const FixPoint16 operator/(const float other) const        { FixPoint16 ret = *this; ret /= other; return ret; }

	bool operator==(const FixPoint16 other) const   { return (value == other.value);              }
	bool operator==(const int other) const          { return (value == fix16_from_int(other));    }
	bool operator==(const unsigned int other) const { return operator==(static_cast<int>(other)); }
	bool operator==(const float other) const        { return (value == fix16_from_float(other));  }

	bool operator!=(const FixPoint16 other) const   { return (value != other.value);              }
	bool operator!=(const int other) const          { return (value != fix16_from_int(other));    }
	bool operator!=(const unsigned int other) const { return operator!=(static_cast<int>(other)); }
	bool operator!=(const float other) const        { return (value != fix16_from_float(other));  }

	bool operator<=(const FixPoint16 other) const   { return (value <= other.value);              }
	bool operator<=(const int other) const          { return (value <= fix16_from_int(other));    }
	bool operator<=(const unsigned int other) const { return operator<=(static_cast<int>(other)); }
	bool operator<=(const float other) const        { return (value <= fix16_from_float(other));  }

	bool operator>=(const FixPoint16 other) const   { return (value >= other.value);              }
	bool operator>=(const int other) const          { return (value >= fix16_from_int(other));    }
	bool operator>=(const unsigned int other) const { return operator>=(static_cast<int>(other)); }
	bool operator>=(const float other) const        { return (value >= fix16_from_float(other));  }

	bool operator< (const FixPoint16 other) const   { return (value <  other.value);              }
	bool operator< (const int other) const          { return (value <  fix16_from_int(other));    }
	bool operator< (const unsigned int other) const { return operator<(static_cast<int>(other));  }
	bool operator< (const float other) const        { return (value <  fix16_from_float(other));  }

	bool operator> (const FixPoint16 other) const   { return (value >  other.value);              }
	bool operator> (const int other) const          { return (value >  fix16_from_int(other));    }
	bool operator> (const unsigned int other) const { return operator>(static_cast<int>(other));  }
	bool operator> (const float other) const        { return (value >  fix16_from_float(other));  }

    FixPoint16& operator++() { *this = *this + 1; return *this; }
    FixPoint16& operator--() { *this = *this - 1; return *this; }
    FixPoint16 operator++(int) { FixPoint16 old = *this; *this = *this + 1; return old; }
    FixPoint16 operator--(int) { FixPoint16 old = *this; *this = *this - 1; return old; }

	static FixPoint16 sin(FixPoint16 inX) { return FromRaw(fix16_sin(inX.value)); }
	static FixPoint16 cos(FixPoint16 inX) { return FromRaw(fix16_cos(inX.value)); }
	static FixPoint16 tan(FixPoint16 inX) { return FromRaw(fix16_tan(inX.value)); }
	static FixPoint16 asin(FixPoint16 inX) { return FromRaw(fix16_asin(inX.value)); }
	static FixPoint16 acos(FixPoint16 inX) { return FromRaw(fix16_acos(inX.value)); }
	static FixPoint16 atan(FixPoint16 inX) { return FromRaw(fix16_atan(inX.value)); }
	static FixPoint16 atan2(FixPoint16 inX, FixPoint16 inY) { return FromRaw(fix16_atan2(inX.value, inY.value)); }
	static FixPoint16 sqrt(FixPoint16 inX) { return FromRaw(fix16_sqrt(inX.value)); }
	static FixPoint16 abs(FixPoint16 inX) { return (inX < 0) ? -inX : inX; }

private:
	fix16_t value;
};

static inline const FixPoint16 operator+(int value, const FixPoint16 other) { return other+value; }
static inline const FixPoint16 operator+(unsigned int value, const FixPoint16 other) { return other+value; }
static inline const FixPoint16 operator+(float value, const FixPoint16 other) { return other+value; }
static inline const FixPoint16 operator-(int value, const FixPoint16 other) { return FixPoint16(value) - other; }
static inline const FixPoint16 operator-(unsigned int value, const FixPoint16 other) { return FixPoint16(value) - other; }
static inline const FixPoint16 operator-(float value, const FixPoint16 other) { return FixPoint16(value) - other; }
static inline const FixPoint16 operator*(int value, const FixPoint16 other) { return other*value; }
static inline const FixPoint16 operator*(unsigned int value, const FixPoint16 other) { return other*value; }
static inline const FixPoint16 operator*(float value, const FixPoint16 other) { return other*value; }
static inline const FixPoint16 operator/(int value, const FixPoint16 other) { return FixPoint16(value) / other; }
static inline const FixPoint16 operator/(unsigned int value, const FixPoint16 other) { return FixPoint16(value) / other; }
static inline const FixPoint16 operator/(float value, const FixPoint16 other) { return FixPoint16(value) / other; }


static inline bool operator==(int value, const FixPoint16 other) { return other.operator==(value); }
static inline bool operator==(unsigned int value, const FixPoint16 other) { return other.operator==(value); }
static inline bool operator==(float value, const FixPoint16 other) { return other.operator==(value); }
static inline bool operator!=(int value, const FixPoint16 other) { return other.operator!=(value); }
static inline bool operator!=(unsigned int value, const FixPoint16 other) { return other.operator!=(value); }
static inline bool operator!=(float value, const FixPoint16 other) { return other.operator!=(value); }

static inline bool operator<=(int value, const FixPoint16 other) { return other.operator>(value); }
static inline bool operator<=(unsigned int value, const FixPoint16 other) { return other.operator>(value); }
static inline bool operator<=(float value, const FixPoint16 other) { return other.operator>(value); }
static inline bool operator>=(int value, const FixPoint16 other) { return other.operator<(value); }
static inline bool operator>=(unsigned int value, const FixPoint16 other) { return other.operator<(value); }
static inline bool operator>=(float value, const FixPoint16 other) { return other.operator<(value); }
static inline bool operator<(int value, const FixPoint16 other) { return other.operator>=(value); }
static inline bool operator<(unsigned int value, const FixPoint16 other) { return other.operator>=(value); }
static inline bool operator<(float value, const FixPoint16 other) { return other.operator>=(value); }
static inline bool operator>(int value, const FixPoint16 other) { return other.operator<=(value); }
static inline bool operator>(unsigned int value, const FixPoint16 other) { return other.operator<=(value); }
static inline bool operator>(float value, const FixPoint16 other) { return other.operator<=(value); }

#endif // FIXPOINT16_H
