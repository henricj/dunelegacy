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

#ifndef FIXPOINT_H
#define FIXPOINT_H

#include "FixPoint32.h"

typedef FixPoint32 FixPoint;

inline constexpr auto FixPt_MAX   = FixPt32_MAX;
inline constexpr auto FixPt_MIN   = FixPt32_MIN;
inline constexpr auto FixPt_ZERO  = FixPt32_ZERO;
inline constexpr auto FixPt_PI    = FixPt32_PI;
inline constexpr auto FixPt_E     = FixPt32_E;
inline constexpr auto FixPt_SQRT2 = FixPt32_SQRT2;

namespace fix32_parserimpl {
#pragma warning(push)
// The unused branches in the template functions have integer overflows.
#pragma warning(disable : 4307)

template<uint64_t I, uint64_t F, uint64_t Scale>
static constexpr uint64_t fp32_fraction_parser() {
    return ((I) << 32) | (((F) << 32) / Scale);
}

template<uint64_t I, uint64_t F, uint64_t Scale, char First, char... Chars>
static constexpr uint64_t fp32_fraction_parser() {
    return fp32_fraction_parser<I, 10 * F + (First - '0'), 10 * Scale, Chars...>();
}

template<uint64_t I>
static constexpr uint64_t fp32_parser() {
    return (I) << 32;
}

template<uint64_t I, char First, char... Chars>
static constexpr uint64_t fp32_parser() {
    static_assert(First == '.' || (First >= '0' && First <= '9'), "Character must be '0' to '9' or '.'");

    if (First == '.')
        return fp32_fraction_parser<I, 0ull, 1, Chars...>();

    return fp32_parser<10 * I + (static_cast<int>(First) - '0'), Chars...>();
}
#pragma warning(pop)
} // namespace fix32_parserimpl

template<char... Chars>
constexpr auto operator""_fix() {
    return FixPoint32::FromRawValue(fix32_parserimpl::fp32_parser<0ull, Chars...>());
}

#endif // FIXPOINT_H
