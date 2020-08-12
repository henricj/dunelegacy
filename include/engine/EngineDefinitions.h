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

#ifndef ENGINEDEFINITIONS_H
#define ENGINEDEFINITIONS_H

#include <cinttypes>
#include <fixmath/FixPoint.h>

inline constexpr uint32_t SAVEMAGIC       = 8675309;
inline constexpr uint32_t SAVEGAMEVERSION = 9704;

inline constexpr int MAX_PLAYERNAMELENGTH = 24;

inline constexpr FixPoint DIAGONALSPEEDCONST = FixPt_SQRT2 >> 1; // = sqrt(2)/2 = 0.707106781

inline constexpr int GAMESPEED_MAX     = 32;
inline constexpr int GAMESPEED_MIN     = 8;
inline constexpr int GAMESPEED_DEFAULT = 16;

constexpr int MILLI2CYCLES(int milliseconds) {
    // this is calculated in game milliseconds (dune 2 has about the same in game speed "fastest")
    return milliseconds / GAMESPEED_DEFAULT;
}

enum WinLoseFlags {
    WINLOSEFLAGS_AI_NO_BUILDINGS     = 0x01,
    WINLOSEFLAGS_HUMAN_HAS_BUILDINGS = 0x02,
    WINLOSEFLAGS_QUOTA               = 0x04,
    WINLOSEFLAGS_TIMEOUT             = 0x08
};

inline constexpr int MAX_XSIZE = 256;
inline constexpr int MAX_YSIZE = 256;

inline constexpr int BUILDRANGE                 = 2;
inline constexpr int MIN_CARRYALL_LIFT_DISTANCE = 6;

inline constexpr int RANDOMSPICEMIN      = 111 - 37; // how much spice on each spice tile
inline constexpr int RANDOMSPICEMAX      = 111 + 37;
inline constexpr int RANDOMTHICKSPICEMIN = 222 - 74;
inline constexpr int RANDOMTHICKSPICEMAX = 222 + 74;

inline constexpr int TILESIZE = 64; // size of tile pieces 16x16 in zoom level 0

inline constexpr int D2_TILESIZE = 16; // the size of a tile in D2

inline constexpr uint32_t NONE_ID           = ~0U; // unsigned -1
inline constexpr int32_t  INVALID_POS       = -1;
inline constexpr int32_t  INVALID           = -1;
inline constexpr uint32_t INVALID_GAMECYCLE = ~0U;

inline constexpr int NUM_TEAMS = 7;

inline constexpr int      DEVIATIONTIME     = MILLI2CYCLES(120 * 1000);
inline constexpr int      TRACKSTIME        = MILLI2CYCLES(1 << 16);
inline constexpr int      HARVESTERMAXSPICE = 700;
inline constexpr FixPoint HARVESTSPEED      = 0.1344_fix;
inline constexpr FixPoint BADLYDAMAGEDRATIO =
    0.5_fix; // if health/getMaxHealth() < this, damage will become bad - smoke and shit
inline constexpr FixPoint HEAVILYDAMAGEDRATIO =
    0.25_fix; // if health/getMaxHealth() < this, damage will become heavy damage - red color
inline constexpr FixPoint HEAVILYDAMAGEDSPEEDMULTIPLIER = 0.75_fix;

inline constexpr int NUMSELECTEDLISTS      = 9;
inline constexpr int NUM_INFANTRY_PER_TILE = 5; // how many infantry can fit on a tile

inline constexpr FixPoint UNIT_REPAIRCOST         = (0.1_fix);
inline constexpr int      DEFAULT_GUARDRANGE      = 10; // 0 - 10, how far unit will search for enemy when guarding
inline constexpr int      DEFAULT_STARTINGCREDITS = 3000;

inline constexpr const char* const HUMANPLAYERCLASS     = "HumanPlayer";
inline constexpr const char* const DEFAULTAIPLAYERCLASS = "qBotMedium";

inline constexpr char houseChar[] = {'H', 'A', 'O', 'F', 'S', 'M'}; ///< character for each house

#ifndef RESTRICT
#if defined(_MSC_VER)
#define RESTRICT __restrict
#elif defined(__GNUC__) || defined(__clang__)
#define RESTRICT __restrict__
#else
#define RESTRICT
#endif
#endif // RESTRICT

#endif //ENGINEDEFINITIONS_H
