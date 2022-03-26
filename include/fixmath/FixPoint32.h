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
#define FixPt32(i, m) (FixPoint32::FromRawValue(F32C(i, m)))

class FixPoint32 final {
public:
    constexpr FixPoint32() noexcept : value {0} { }
    constexpr FixPoint32(const FixPoint32& inValue) noexcept = default;
    constexpr FixPoint32(const int inValue) noexcept : value {inValue * fix32_one} { }
    constexpr FixPoint32(const unsigned int inValue) noexcept : value {static_cast<int>(inValue) * fix32_one} { }

    constexpr FixPoint32(FixPoint32&& inValue) noexcept = default;

    FixPoint32& operator=(const FixPoint32& rhs) noexcept = default;
    FixPoint32& operator=(FixPoint32&& other) noexcept = default;

    ~FixPoint32() = default;

    explicit FixPoint32(const std::string& inValue) { value = fix32_from_str(inValue.c_str()); }

    static constexpr FixPoint32 FromRawValue(const fix32_t value) noexcept {
        FixPoint32 x;
        x.value = value;
        return x;
    }

    [[nodiscard]] constexpr fix32_t getRawValue() const noexcept { return value; }

    [[nodiscard]] constexpr double toDouble() const noexcept { return value * (1.0 / fix32_one); }

    [[nodiscard]] constexpr float toFloat() const noexcept { return value * (1.0f / fix32_one); }

    [[nodiscard]] std::string toString() const {
        char buffer[32];
        fix32_to_str(value, buffer, 10);
        return std::string(buffer);
    }

    [[nodiscard]] int lround() const { return fix32_to_int(value); }
    [[nodiscard]] constexpr int floor() const { return value >> 32; }
    [[nodiscard]] constexpr int ceil() const { return (value + 0x00000000FFFFFFFFLL) >> 32; }

    constexpr FixPoint32& operator>>=(const int rhs) noexcept {
        value >>= rhs;
        return *this;
    }
    constexpr FixPoint32& operator<<=(const int rhs) noexcept {
        value <<= rhs;
        return *this;
    }

    constexpr FixPoint32& operator=(const int rhs) noexcept {
        value = rhs * fix32_one;
        return *this;
    }
    constexpr FixPoint32& operator=(const unsigned int rhs) noexcept { return operator=(static_cast<int>(rhs)); }

    constexpr FixPoint32& operator+=(const FixPoint32& rhs) noexcept {
        value += rhs.value;
        return *this;
    }
    constexpr FixPoint32& operator+=(const int rhs) noexcept {
        value += rhs * fix32_one;
        return *this;
    }
    constexpr FixPoint32& operator+=(const unsigned int rhs) noexcept { return operator+=(static_cast<int>(rhs)); }

    constexpr FixPoint32& operator-=(const FixPoint32& rhs) noexcept {
        value -= rhs.value;
        return *this;
    }
    constexpr FixPoint32& operator-=(const int rhs) noexcept {
        value -= rhs * fix32_one;
        return *this;
    }
    constexpr FixPoint32& operator-=(const unsigned int rhs) noexcept { return operator-=(static_cast<int>(rhs)); }

    FixPoint32& operator*=(const FixPoint32& rhs) noexcept {
        value = fix32_mul(value, rhs.value);
        return *this;
    }
    FixPoint32& operator*=(const int rhs) noexcept {
        value *= rhs;
        return *this;
    }
    FixPoint32& operator*=(const unsigned int rhs) noexcept { return operator*=(static_cast<int>(rhs)); }

    FixPoint32& operator/=(const FixPoint32& rhs) {
        value = fix32_div(value, rhs.value);
        return *this;
    }
    FixPoint32& operator/=(const int rhs) {
        value /= rhs;
        return *this;
    }
    FixPoint32& operator/=(const unsigned int rhs) { return operator/=(static_cast<int>(rhs)); }

    FixPoint32& operator%=(const FixPoint32& rhs) {
        value = fix32_mod(value, rhs.value);
        return *this;
    }
    FixPoint32& operator%=(const int rhs) {
        value %= rhs;
        return *this;
    }
    FixPoint32& operator%=(const unsigned int rhs) { return operator%=(static_cast<int>(rhs)); }

    constexpr FixPoint32 operator-() const noexcept { return FromRawValue(-value); }

    constexpr FixPoint32 operator>>(const int other) const noexcept {
        auto ret = *this;
        ret >>= other;
        return ret;
    }
    constexpr FixPoint32 operator<<(const int other) const noexcept {
        auto ret = *this;
        ret <<= other;
        return ret;
    }

    constexpr FixPoint32 operator+(const FixPoint32& other) const noexcept {
        auto ret = *this;
        ret += other;
        return ret;
    }
    // constexpr FixPoint32 operator+(const int other) const noexcept          { auto ret = *this; ret +=
    // FixPoint32(other); return ret; } constexpr FixPoint32 operator+(const unsigned int other) const noexcept { return
    // operator+(static_cast<int>(other)); }

    constexpr FixPoint32 operator-(const FixPoint32& other) const noexcept {
        auto ret = *this;
        ret -= other;
        return ret;
    }
    // constexpr FixPoint32 operator-(const int other) const noexcept          { auto ret = *this; ret -=
    // FixPoint32(other); return ret; } constexpr FixPoint32 operator-(const unsigned int other) const noexcept { return
    // operator-(static_cast<int>(other)); }

    FixPoint32 operator*(const FixPoint32& other) const noexcept {
        auto ret = *this;
        ret *= other;
        return ret;
    }
    // constexpr FixPoint32 operator*(const int other) const noexcept          { auto ret = *this; ret *=
    // FixPoint32(other); return ret; } constexpr FixPoint32 operator*(const unsigned int other) const noexcept { return
    // operator*(static_cast<int>(other)); }

    FixPoint32 operator/(const FixPoint32& other) const {
        auto ret = *this;
        ret /= other;
        return ret;
    }
    // constexpr FixPoint32 operator/(const int other) const          { auto ret = *this; ret /= FixPoint32(other);
    // return ret; } constexpr FixPoint32 operator/(const unsigned int other) const { return
    // operator/(static_cast<int>(other)); }

    FixPoint32 operator%(const FixPoint32& other) const {
        auto ret = *this;
        ret %= other;
        return ret;
    }
    // constexpr FixPoint32 operator%(const int other) const          { auto ret = *this; ret %= FixPoint32(other);
    // return ret; } constexpr FixPoint32 operator%(const unsigned int other) const { return
    // operator%(static_cast<int>(other)); }

    constexpr bool operator==(const FixPoint32& other) const noexcept { return value == other.value; }
    // bool operator==(const int other) const noexcept                 { return value == fix32_from_int(other); }
    // bool operator==(const unsigned int other) const noexcept        { return operator==(static_cast<int>(other)); }

    constexpr bool operator!=(const FixPoint32& other) const noexcept { return !operator==(other); }
    // bool operator!=(const int other) const          { return (value != fix32_from_int(other));    }
    // bool operator!=(const unsigned int other) const { return operator!=(static_cast<int>(other)); }

    constexpr bool operator<=(const FixPoint32& other) const noexcept { return value <= other.value; }
    // bool operator<=(const int other) const          { return (value <= fix32_from_int(other));    }
    // bool operator<=(const unsigned int other) const { return operator<=(static_cast<int>(other)); }

    constexpr bool operator>=(const FixPoint32& other) const noexcept { return value >= other.value; }
    // bool operator>=(const int other) const          { return (value >= fix32_from_int(other));    }
    // bool operator>=(const unsigned int other) const { return operator>=(static_cast<int>(other)); }

    constexpr bool operator<(const FixPoint32& other) const noexcept { return value < other.value; }
    // bool operator< (const int other) const          { return (value <  fix32_from_int(other));    }
    // bool operator< (const unsigned int other) const { return operator<(static_cast<int>(other));  }

    constexpr bool operator>(const FixPoint32& other) const { return value > other.value; }
    // bool operator> (const int other) const          { return (value >  fix32_from_int(other));    }
    // bool operator> (const unsigned int other) const { return operator>(static_cast<int>(other));  }

    constexpr FixPoint32& operator++() noexcept {
        value += fix32_one;
        return *this;
    }
    constexpr FixPoint32& operator--() noexcept {
        value -= fix32_one;
        return *this;
    }
    // FixPoint32 operator++(int) { FixPoint32 old = *this; *this = *this + 1; return old; }
    // FixPoint32 operator--(int) { FixPoint32 old = *this; *this = *this - 1; return old; }

    static FixPoint32 sin(FixPoint32 inX) { return FromRawValue(fix32_sin(inX.value)); }
    static FixPoint32 cos(FixPoint32 inX) { return FromRawValue(fix32_cos(inX.value)); }
    static FixPoint32 tan(FixPoint32 inX) { return FromRawValue(fix32_tan(inX.value)); }
    static FixPoint32 asin(FixPoint32 inX) { return FromRawValue(fix32_asin(inX.value)); }
    static FixPoint32 acos(FixPoint32 inX) { return FromRawValue(fix32_acos(inX.value)); }
    static FixPoint32 atan(FixPoint32 inX) { return FromRawValue(fix32_atan(inX.value)); }
    static FixPoint32 atan2(FixPoint32 inY, FixPoint32 inX) { return FromRawValue(fix32_atan2(inY.value, inX.value)); }
    static FixPoint32 sqrt(FixPoint32 inX) { return FromRawValue(fix32_sqrt(inX.value)); }
    static constexpr FixPoint32 abs(FixPoint32 inX) noexcept { return (inX < 0) ? -inX : inX; }

private:
    fix32_t value;
};

inline constexpr auto FixPt32_MAX {FixPoint32::FromRawValue(fix32_maximum)};
inline constexpr auto FixPt32_PI {FixPoint32::FromRawValue(fix32_pi)};
inline constexpr auto FixPt32_E {FixPoint32::FromRawValue(fix32_e)};
inline constexpr auto FixPt32_SQRT2 {FixPoint32::FromRawValue(0x000000016A09E668LL)};

static constexpr FixPoint32 operator+(int value, const FixPoint32& other) noexcept {
    return other + value;
}
static constexpr FixPoint32 operator+(unsigned int value, const FixPoint32& other) noexcept {
    return other + value;
}
static constexpr FixPoint32 operator-(int value, const FixPoint32& other) noexcept {
    return FixPoint32(value) - other;
}
static constexpr FixPoint32 operator-(unsigned int value, const FixPoint32& other) noexcept {
    return FixPoint32(value) - other;
}
static inline FixPoint32 operator*(int value, const FixPoint32& other) noexcept {
    return other * value;
}
static inline FixPoint32 operator*(unsigned int value, const FixPoint32& other) noexcept {
    return other * value;
}
static inline FixPoint32 operator/(int value, const FixPoint32& other) noexcept {
    return FixPoint32(value) / other;
}
static inline FixPoint32 operator/(unsigned int value, const FixPoint32& other) noexcept {
    return FixPoint32(value) / other;
}
static inline FixPoint32 operator%(int value, const FixPoint32& other) noexcept {
    return FixPoint32(value) % other;
}
static inline FixPoint32 operator%(unsigned int value, const FixPoint32& other) noexcept {
    return FixPoint32(value) % other;
}

static constexpr bool operator==(int value, const FixPoint32& other) noexcept {
    return other.operator==(value);
}
static constexpr bool operator==(unsigned int value, const FixPoint32& other) noexcept {
    return other.operator==(value);
}
static constexpr bool operator!=(int value, const FixPoint32& other) noexcept {
    return other.operator!=(value);
}
static constexpr bool operator!=(unsigned int value, const FixPoint32& other) noexcept {
    return other.operator!=(value);
}

static constexpr bool operator<=(int value, const FixPoint32& other) noexcept {
    return other.operator>(value);
}
static constexpr bool operator<=(unsigned int value, const FixPoint32& other) noexcept {
    return other.operator>(value);
}
static constexpr bool operator>=(int value, const FixPoint32& other) noexcept {
    return other.operator<(value);
}
static constexpr bool operator>=(unsigned int value, const FixPoint32& other) noexcept {
    return other.operator<(value);
}
static constexpr bool operator<(int value, const FixPoint32& other) noexcept {
    return other.operator>=(value);
}
static constexpr bool operator<(unsigned int value, const FixPoint32& other) noexcept {
    return other.operator>=(value);
}
static constexpr bool operator>(int value, const FixPoint32& other) noexcept {
    return other.operator<=(value);
}
static constexpr bool operator>(unsigned int value, const FixPoint32& other) noexcept {
    return other.operator<=(value);
}

static inline int lround(const FixPoint32& value) {
    return value.lround();
}
static constexpr int floor(const FixPoint32& value) noexcept {
    return value.floor();
}
static constexpr int ceil(const FixPoint32& value) noexcept {
    return value.ceil();
}

#endif // FIXPOINT32_H
