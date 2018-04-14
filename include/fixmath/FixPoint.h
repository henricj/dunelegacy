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

#define FixPt(i,m) FixPt32(i,m)

#define FixPt_MAX FixPt32_MAX
#define FixPt_PI FixPt32_PI
#define FixPt_E FixPt32_E
#define FixPt_SQRT2 FixPt32_SQRT2

#include "fix32_support.h"

template<char...Chars>
constexpr auto operator""_fix()
{
    return FixPoint32::FromRawValue(fix32_support::operator ""_fix32<Chars...>());
}



#endif // FIXPOINT_H
