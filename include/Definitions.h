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

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define SCREEN_FORMAT SDL_PIXELFORMAT_ARGB8888
#define SCREEN_BPP    SDL_BITSPERPIXEL(SCREEN_FORMAT)

inline constexpr auto SCREEN_MIN_WIDTH            = 640;
inline constexpr auto SCREEN_MIN_HEIGHT           = 480;
inline constexpr auto SCREEN_DEFAULT_DISPLAYINDEX = 0;

// Yamaha's OPL used an interesting sampling frequency.
inline constexpr auto AUDIO_FREQUENCY = 49716;

inline constexpr auto DEFAULT_PORT       = 28747;
inline constexpr auto DEFAULT_METASERVER = "http://dunelegacy.sourceforge.net/metaserver/metaserver.php";

inline constexpr auto SAVEMAGIC       = 8675309;
inline constexpr auto SAVEGAMEVERSION = 9704;

inline constexpr auto MAX_PLAYERNAMELENGTH = 24;

#define DIAGONALSPEEDCONST (FixPt_SQRT2 >> 1) // = sqrt(2)/2 = 0.707106781

inline constexpr auto GAMESPEED_MAX     = 32;
inline constexpr auto GAMESPEED_MIN     = 8;
inline constexpr auto GAMESPEED_DEFAULT = 16;

constexpr auto MILLI2CYCLES(int milliseconds) {
    // this is calculated in game milliseconds (dune 2 has about the same in game speed "fastest")
    return milliseconds / GAMESPEED_DEFAULT;
}

constexpr auto MILLI2CYCLES(uint32_t milliseconds) {
    // this is calculated in game milliseconds (dune 2 has about the same in game speed "fastest")
    return milliseconds / static_cast<uint32_t>(GAMESPEED_DEFAULT);
}

inline constexpr auto VOLUME_MAX = 100;
inline constexpr auto VOLUME_MIN = 0;

inline constexpr auto NUM_ZOOMLEVEL = 3;

inline constexpr auto WINLOSEFLAGS_AI_NO_BUILDINGS     = 0x01;
inline constexpr auto WINLOSEFLAGS_HUMAN_HAS_BUILDINGS = 0x02;
inline constexpr auto WINLOSEFLAGS_QUOTA               = 0x04;
inline constexpr auto WINLOSEFLAGS_TIMEOUT             = 0x08;

inline constexpr auto MAX_XSIZE = 256;
inline constexpr auto MAX_YSIZE = 256;

inline constexpr auto BUILDRANGE                 = 2;
inline constexpr auto MIN_CARRYALL_LIFT_DISTANCE = 6;
inline constexpr auto STRUCTURE_ANIMATIONTIMER   = 31;

inline constexpr auto RANDOMSPICEMIN      = 111 - 37; // how much spice on each spice tile
inline constexpr auto RANDOMSPICEMAX      = 111 + 37;
inline constexpr auto RANDOMTHICKSPICEMIN = 222 - 74;
inline constexpr auto RANDOMTHICKSPICEMAX = 222 + 74;

inline constexpr auto TILESIZE = 64; // size of tile pieces 16x16 in zoom level 0

inline constexpr auto D2_TILESIZE = 16; // the size of a tile in D2

inline constexpr auto SIDEBARWIDTH         = 144;
inline constexpr auto SIDEBAR_COLUMN_WIDTH = 12;

inline constexpr auto NONE_ID           = ~0U; // unsigned -1
inline constexpr auto INVALID_POS       = -1;
inline constexpr auto INVALID           = -1;
inline constexpr auto INVALID_GAMECYCLE = ~0U;

inline constexpr auto NUM_TEAMS = 7;

inline constexpr auto DEVIATIONTIME     = MILLI2CYCLES(120 * 1000);
inline constexpr auto TRACKSTIME        = MILLI2CYCLES((1 << 16));
inline constexpr auto HARVESTERMAXSPICE = 700;
#define HARVESTSPEED      (0.1344_fix)
#define BADLYDAMAGEDRATIO (0.5_fix) // if health/getMaxHealth() < this, damage will become bad - smoke and shit

#define HEAVILYDAMAGEDRATIO (025_fix) // if health/getMaxHealth() < this, damage will become heavy damage - red color

#define HEAVILYDAMAGEDSPEEDMULTIPLIER (0.75_fix)
inline constexpr auto NUMSELECTEDLISTS      = 9;
inline constexpr auto NUM_INFANTRY_PER_TILE = 5; // how many infantry can fit on a tile

#define UNIT_REPAIRCOST (0.1_fix)
inline constexpr auto DEFAULT_GUARDRANGE      = 10; // 0 - 10, how far unit will search for enemy when guarding
inline constexpr auto DEFAULT_STARTINGCREDITS = 3000;

inline constexpr auto HUMANPLAYERCLASS     = "HumanPlayer";
inline constexpr auto DEFAULTAIPLAYERCLASS = "qBotMedium";

#ifndef RESTRICT
#    if defined(_MSC_VER)
#        define RESTRICT __restrict
#    elif defined(__GNUC__) || defined(__clang__)
#        define RESTRICT __restrict__
#    else
#        define RESTRICT
#    endif
#endif // RESTRICT

#endif // DEFINITIONS_H
