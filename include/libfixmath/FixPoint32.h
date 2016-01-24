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

#ifndef FIXPOINT32_H
#define FIXPOINT32_H

#include "fix32.h"

#include <string>

// we need to use a macro here as leading zeros in m are important: FixPt(1, 5) != FixPt(1, 05)
#define FixPt32(i,m) (FixPoint32::FromRawValue(F32C(i,m)))

#define FixPt32_PI (FixPoint32::FromRawValue(fix32_pi))
#define FixPt32_E (FixPoint32::FromRawValue(fix32_e))

class FixPoint32
{
public:
	FixPoint32() { value = 0; }
	FixPoint32(const FixPoint32& inValue)        { value = inValue.value;             }
	FixPoint32(const int inValue)                { value = fix32_from_int(inValue); }
	FixPoint32(const unsigned int inValue)       { value = fix32_from_int(static_cast<int>(inValue)); }
	/*explicit*/ FixPoint32(const float inValue)   { value = fix32_from_float(inValue); }
//	/*explicit*/ FixPoint32(const double inValue)  { value = fix32_from_dbl(inValue);   }

	static FixPoint32 FromRawValue(const fix32_t value) {
		FixPoint32 x;
		x.value = value;
		return x;
	}

	fix32_t getRawValue() const { return value; }

	double toDouble() const { return fix32_to_dbl(value);   }
	float toFloat() const   { return fix32_to_float(value); }
	std::string toString() const {
		char buffer[32];
		fix32_to_str(value, buffer, 10);
		return std::string(buffer);
	}


//    operator double()  const { return fix32_to_dbl(value);   }
    operator float()   const { return fix32_to_float(value); }
    explicit operator int() const { return fix32_to_int(value);   }

	int roundToInt() const       { return fix32_to_int(value);   }

	FixPoint32 & operator=(const FixPoint32 rhs)     { value = rhs.value;             return *this; }
	FixPoint32 & operator=(const int rhs)            { value = fix32_from_int(rhs);   return *this; }
	FixPoint32 & operator=(const unsigned int rhs)   { return operator=(static_cast<int>(rhs)); }
	FixPoint32 & operator=(const float rhs)          { value = fix32_from_float(rhs); return *this; }

	FixPoint32 & operator+=(const FixPoint32 rhs)     { value += rhs.value;             return *this; }
	FixPoint32 & operator+=(const int rhs)            { value += fix32_from_int(rhs);   return *this; }
	FixPoint32 & operator+=(const unsigned int rhs)   { return operator+=(static_cast<int>(rhs)); }
	FixPoint32 & operator+=(const float rhs)          { value += fix32_from_float(rhs); return *this; }

	FixPoint32 & operator-=(const FixPoint32 rhs)     { value -= rhs.value; return *this; }
	FixPoint32 & operator-=(const int rhs)            { value -= fix32_from_int(rhs); return *this; }
	FixPoint32 & operator-=(const unsigned int rhs)   { return operator-=(static_cast<int>(rhs)); }
	FixPoint32 & operator-=(const float rhs)          { value -= fix32_from_float(rhs); return *this; }

	FixPoint32 & operator*=(const FixPoint32 rhs)     { value = fix32_mul(value, rhs.value); return *this; }
	FixPoint32 & operator*=(const int rhs)            { value *= rhs; return *this; }
	FixPoint32 & operator*=(const unsigned int rhs)   { return operator*=(static_cast<int>(rhs)); }
	FixPoint32 & operator*=(const float rhs)          { value = fix32_mul(value, fix32_from_float(rhs)); return *this; }

	FixPoint32 & operator/=(const FixPoint32 rhs)     { value = fix32_div(value, rhs.value); return *this; }
	FixPoint32 & operator/=(const int rhs)            { value /= rhs; return *this; }
	FixPoint32 & operator/=(const unsigned int rhs)   { return operator/=(static_cast<int>(rhs)); }
	FixPoint32 & operator/=(const float rhs)          { value = fix32_div(value, fix32_from_float(rhs)); return *this; }

	const FixPoint32 operator-() const { FixPoint32 ret = *this; ret.value = -ret.value; return ret; }

	const FixPoint32 operator+(const FixPoint32 other) const   { FixPoint32 ret = *this; ret += other; return ret; }
	const FixPoint32 operator+(const int other) const          { FixPoint32 ret = *this; ret += FixPoint32(other); return ret; }
	const FixPoint32 operator+(const unsigned int other) const { return operator+(static_cast<int>(other)); }
	const FixPoint32 operator+(const float other) const        { FixPoint32 ret = *this; ret += other; return ret; }

	const FixPoint32 operator-(const FixPoint32 other) const   { FixPoint32 ret = *this; ret -= other; return ret; }
	const FixPoint32 operator-(const int other) const          { FixPoint32 ret = *this; ret -= FixPoint32(other); return ret; }
	const FixPoint32 operator-(const unsigned int other) const { return operator-(static_cast<int>(other)); }
	const FixPoint32 operator-(const float other) const        { FixPoint32 ret = *this; ret -= other; return ret; }

	const FixPoint32 operator*(const FixPoint32 other) const   { FixPoint32 ret = *this; ret *= other; return ret; }
	const FixPoint32 operator*(const int other) const          { FixPoint32 ret = *this; ret *= FixPoint32(other); return ret; }
	const FixPoint32 operator*(const unsigned int other) const { return operator*(static_cast<int>(other)); }
	const FixPoint32 operator*(const float other) const        { FixPoint32 ret = *this; ret *= other; return ret; }

	const FixPoint32 operator/(const FixPoint32 other) const   { FixPoint32 ret = *this; ret /= other; return ret; }
	const FixPoint32 operator/(const int other) const          { FixPoint32 ret = *this; ret /= FixPoint32(other); return ret; }
	const FixPoint32 operator/(const unsigned int other) const { return operator/(static_cast<int>(other)); }
	const FixPoint32 operator/(const float other) const        { FixPoint32 ret = *this; ret /= other; return ret; }

	bool operator==(const FixPoint32 other) const   { return (value == other.value);              }
	bool operator==(const int other) const          { return (value == fix32_from_int(other));    }
	bool operator==(const unsigned int other) const { return operator==(static_cast<int>(other)); }
	bool operator==(const float other) const        { return (value == fix32_from_float(other));  }

	bool operator!=(const FixPoint32 other) const   { return (value != other.value);              }
	bool operator!=(const int other) const          { return (value != fix32_from_int(other));    }
	bool operator!=(const unsigned int other) const { return operator!=(static_cast<int>(other)); }
	bool operator!=(const float other) const        { return (value != fix32_from_float(other));  }

	bool operator<=(const FixPoint32 other) const   { return (value <= other.value);              }
	bool operator<=(const int other) const          { return (value <= fix32_from_int(other));    }
	bool operator<=(const unsigned int other) const { return operator<=(static_cast<int>(other)); }
	bool operator<=(const float other) const        { return (value <= fix32_from_float(other));  }

	bool operator>=(const FixPoint32 other) const   { return (value >= other.value);              }
	bool operator>=(const int other) const          { return (value >= fix32_from_int(other));    }
	bool operator>=(const unsigned int other) const { return operator>=(static_cast<int>(other)); }
	bool operator>=(const float other) const        { return (value >= fix32_from_float(other));  }

	bool operator< (const FixPoint32 other) const   { return (value <  other.value);              }
	bool operator< (const int other) const          { return (value <  fix32_from_int(other));    }
	bool operator< (const unsigned int other) const { return operator<(static_cast<int>(other));  }
	bool operator< (const float other) const        { return (value <  fix32_from_float(other));  }

	bool operator> (const FixPoint32 other) const   { return (value >  other.value);              }
	bool operator> (const int other) const          { return (value >  fix32_from_int(other));    }
	bool operator> (const unsigned int other) const { return operator>(static_cast<int>(other));  }
	bool operator> (const float other) const        { return (value >  fix32_from_float(other));  }

    FixPoint32& operator++() { *this = *this + 1; return *this; }
    FixPoint32& operator--() { *this = *this - 1; return *this; }
    FixPoint32 operator++(int) { FixPoint32 old = *this; *this = *this + 1; return old; }
    FixPoint32 operator--(int) { FixPoint32 old = *this; *this = *this - 1; return old; }

	static FixPoint32 sin(FixPoint32 inX) { return FromRawValue(fix32_sin(inX.value)); }
	static FixPoint32 cos(FixPoint32 inX) { return FromRawValue(fix32_cos(inX.value)); }
	static FixPoint32 tan(FixPoint32 inX) { return FromRawValue(fix32_tan(inX.value)); }
	static FixPoint32 asin(FixPoint32 inX) { return FromRawValue(fix32_asin(inX.value)); }
	static FixPoint32 acos(FixPoint32 inX) { return FromRawValue(fix32_acos(inX.value)); }
	static FixPoint32 atan(FixPoint32 inX) { return FromRawValue(fix32_atan(inX.value)); }
	static FixPoint32 atan2(FixPoint32 inX, FixPoint32 inY) { return FromRawValue(fix32_atan2(inX.value, inY.value)); }
	static FixPoint32 sqrt(FixPoint32 inX) { return FromRawValue(fix32_sqrt(inX.value)); }
	static FixPoint32 abs(FixPoint32 inX) { return (inX < 0) ? -inX : inX; }

private:
	fix32_t value;
};

static inline const FixPoint32 operator+(int value, const FixPoint32 other) { return other+value; }
static inline const FixPoint32 operator+(unsigned int value, const FixPoint32 other) { return other+value; }
static inline const FixPoint32 operator+(float value, const FixPoint32 other) { return other+value; }
static inline const FixPoint32 operator-(int value, const FixPoint32 other) { return FixPoint32(value) - other; }
static inline const FixPoint32 operator-(unsigned int value, const FixPoint32 other) { return FixPoint32(value) - other; }
static inline const FixPoint32 operator-(float value, const FixPoint32 other) { return FixPoint32(value) - other; }
static inline const FixPoint32 operator*(int value, const FixPoint32 other) { return other*value; }
static inline const FixPoint32 operator*(unsigned int value, const FixPoint32 other) { return other*value; }
static inline const FixPoint32 operator*(float value, const FixPoint32 other) { return other*value; }
static inline const FixPoint32 operator/(int value, const FixPoint32 other) { return FixPoint32(value) / other; }
static inline const FixPoint32 operator/(unsigned int value, const FixPoint32 other) { return FixPoint32(value) / other; }
static inline const FixPoint32 operator/(float value, const FixPoint32 other) { return FixPoint32(value) / other; }


static inline bool operator==(int value, const FixPoint32 other) { return other.operator==(value); }
static inline bool operator==(unsigned int value, const FixPoint32 other) { return other.operator==(value); }
static inline bool operator==(float value, const FixPoint32 other) { return other.operator==(value); }
static inline bool operator!=(int value, const FixPoint32 other) { return other.operator!=(value); }
static inline bool operator!=(unsigned int value, const FixPoint32 other) { return other.operator!=(value); }
static inline bool operator!=(float value, const FixPoint32 other) { return other.operator!=(value); }

static inline bool operator<=(int value, const FixPoint32 other) { return other.operator>(value); }
static inline bool operator<=(unsigned int value, const FixPoint32 other) { return other.operator>(value); }
static inline bool operator<=(float value, const FixPoint32 other) { return other.operator>(value); }
static inline bool operator>=(int value, const FixPoint32 other) { return other.operator<(value); }
static inline bool operator>=(unsigned int value, const FixPoint32 other) { return other.operator<(value); }
static inline bool operator>=(float value, const FixPoint32 other) { return other.operator<(value); }
static inline bool operator<(int value, const FixPoint32 other) { return other.operator>=(value); }
static inline bool operator<(unsigned int value, const FixPoint32 other) { return other.operator>=(value); }
static inline bool operator<(float value, const FixPoint32 other) { return other.operator>=(value); }
static inline bool operator>(int value, const FixPoint32 other) { return other.operator<=(value); }
static inline bool operator>(unsigned int value, const FixPoint32 other) { return other.operator<=(value); }
static inline bool operator>(float value, const FixPoint32 other) { return other.operator<=(value); }

#endif // FIXPOINT32_H
