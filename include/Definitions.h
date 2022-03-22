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

static constexpr auto SCREEN_MIN_WIDTH            = 640;
static constexpr auto SCREEN_MIN_HEIGHT           = 480;
static constexpr auto SCREEN_DEFAULT_DISPLAYINDEX = 0;

// Yamaha's OPL used an interesting sampling frequency.
static constexpr auto AUDIO_FREQUENCY = 49716;

static constexpr auto DEFAULT_PORT       = 28747;
static constexpr auto DEFAULT_METASERVER = "http://dunelegacy.sourceforge.net/metaserver/metaserver.php";

static constexpr auto SAVEMAGIC       = 8675309;
static constexpr auto SAVEGAMEVERSION = 9704;

static constexpr auto MAX_PLAYERNAMELENGTH = 24;

#define DIAGONALSPEEDCONST (FixPt_SQRT2 >> 1) // = sqrt(2)/2 = 0.707106781

static constexpr auto GAMESPEED_MAX     = 32;
static constexpr auto GAMESPEED_MIN     = 8;
static constexpr auto GAMESPEED_DEFAULT = 16;

constexpr auto MILLI2CYCLES(int milliseconds) {
    // this is calculated in game milliseconds (dune 2 has about the same in game speed "fastest")
    return milliseconds / GAMESPEED_DEFAULT;
}

static constexpr auto VOLUME_MAX = 100;
static constexpr auto VOLUME_MIN = 0;

static constexpr auto NUM_ZOOMLEVEL = 3;

static constexpr auto WINLOSEFLAGS_AI_NO_BUILDINGS     = 0x01;
static constexpr auto WINLOSEFLAGS_HUMAN_HAS_BUILDINGS = 0x02;
static constexpr auto WINLOSEFLAGS_QUOTA               = 0x04;
static constexpr auto WINLOSEFLAGS_TIMEOUT             = 0x08;

static constexpr auto MAX_XSIZE = 256;
static constexpr auto MAX_YSIZE = 256;

static constexpr auto BUILDRANGE                 = 2;
static constexpr auto MIN_CARRYALL_LIFT_DISTANCE = 6;
static constexpr auto STRUCTURE_ANIMATIONTIMER   = 31;

static constexpr auto RANDOMSPICEMIN      = 111 - 37; // how much spice on each spice tile
static constexpr auto RANDOMSPICEMAX      = 111 + 37;
static constexpr auto RANDOMTHICKSPICEMIN = 222 - 74;
static constexpr auto RANDOMTHICKSPICEMAX = 222 + 74;

static constexpr auto TILESIZE = 64; // size of tile pieces 16x16 in zoom level 0

static constexpr auto D2_TILESIZE = 16; // the size of a tile in D2

static constexpr auto SIDEBARWIDTH         = 144;
static constexpr auto SIDEBAR_COLUMN_WIDTH = 12;

static constexpr auto NONE_ID           = ~0U; // unsigned -1
static constexpr auto INVALID_POS       = -1;
static constexpr auto INVALID           = -1;
static constexpr auto INVALID_GAMECYCLE = ~0U;

static constexpr auto NUM_TEAMS = 7;

static constexpr auto DEVIATIONTIME     = MILLI2CYCLES(120 * 1000);
static constexpr auto TRACKSTIME        = MILLI2CYCLES((1 << 16));
static constexpr auto HARVESTERMAXSPICE = 700;
#define HARVESTSPEED      (0.1344_fix)
#define BADLYDAMAGEDRATIO (0.5_fix) // if health/getMaxHealth() < this, damage will become bad - smoke and shit

#define HEAVILYDAMAGEDRATIO (025_fix) // if health/getMaxHealth() < this, damage will become heavy damage - red color

#define HEAVILYDAMAGEDSPEEDMULTIPLIER (0.75_fix)
static constexpr auto NUMSELECTEDLISTS      = 9;
static constexpr auto NUM_INFANTRY_PER_TILE = 5; // how many infantry can fit on a tile

#define UNIT_REPAIRCOST (0.1_fix)
static constexpr auto DEFAULT_GUARDRANGE      = 10; // 0 - 10, how far unit will search for enemy when guarding
static constexpr auto DEFAULT_STARTINGCREDITS = 3000;

static constexpr auto HUMANPLAYERCLASS     = "HumanPlayer";
static constexpr auto DEFAULTAIPLAYERCLASS = "qBotMedium";

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
