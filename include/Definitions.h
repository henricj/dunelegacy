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

#define SCREEN_BPP                  32
#define SCREEN_FORMAT               SDL_PIXELFORMAT_ABGR8888
#define SCREEN_MIN_WIDTH            640
#define SCREEN_MIN_HEIGHT           480
#define SCREEN_DEFAULT_DISPLAYINDEX 0

#define AUDIO_FREQUENCY     44100

#define DEFAULT_PORT        28747
#define DEFAULT_METASERVER  "http://dunelegacy.sourceforge.net/metaserver/metaserver.php"

#define SAVEMAGIC           8675309
#define SAVEGAMEVERSION     9704

#define MAX_PLAYERNAMELENGHT    24

#define DIAGONALSPEEDCONST (FixPt_SQRT2 >> 1)           // = sqrt(2)/2 = 0.707106781


#define GAMESPEED_MAX 32
#define GAMESPEED_MIN 8
#define GAMESPEED_DEFAULT 16
#define MILLI2CYCLES(MILLISECONDS) ((MILLISECONDS)/GAMESPEED_DEFAULT)   // this is calculated in game milliseconds (dune 2 has about the same in game speed "fastest")
#define VOLUME_MAX 100
#define VOLUME_MIN 0

#define NUM_ZOOMLEVEL   3

#define WINLOSEFLAGS_AI_NO_BUILDINGS        0x01
#define WINLOSEFLAGS_HUMAN_HAS_BUILDINGS    0x02
#define WINLOSEFLAGS_QUOTA                  0x04
#define WINLOSEFLAGS_TIMEOUT                0x08

#define MAX_XSIZE 256
#define MAX_YSIZE 256

#define BUILDRANGE 2
#define MIN_CARRYALL_LIFT_DISTANCE 6
#define STRUCTURE_ANIMATIONTIMER 31

#define RANDOMSPICEMIN (111 - 37)        //how much spice on each spice tile
#define RANDOMSPICEMAX (111 + 37)
#define RANDOMTHICKSPICEMIN (222 - 74)
#define RANDOMTHICKSPICEMAX (222 + 74)

#define TILESIZE    64              // size of tile pieces 16x16 in zoom level 0

#define D2_TILESIZE 16              // the size of a tile in D2

#define SIDEBARWIDTH 144
#define SIDEBAR_COLUMN_WIDTH 12

#define NONE_ID (static_cast<Uint32>(-1))          // unsigned -1
#define INVALID_POS (-1)
#define INVALID (-1)
#define INVALID_GAMECYCLE (static_cast<Uint32>(-1))

#define NUM_TEAMS 7

#define DEVIATIONTIME MILLI2CYCLES(120*1000)
#define TRACKSTIME MILLI2CYCLES((1 << 16))
#define HARVESTERMAXSPICE 700
#define HARVESTSPEED (0.1344_fix)
#define BADLYDAMAGEDRATIO (0.5_fix)                //if health/getMaxHealth() < this, damage will become bad - smoke and shit
#define HEAVILYDAMAGEDRATIO (025_fix)             //if health/getMaxHealth() < this, damage will become heavy damage - red color
#define HEAVILYDAMAGEDSPEEDMULTIPLIER (0.75_fix)
#define NUMSELECTEDLISTS 9
#define NUM_INFANTRY_PER_TILE 5                 //how many infantry can fit on a tile

#define UNIT_REPAIRCOST (0.1_fix)
#define DEFAULT_GUARDRANGE 10                   //0 - 10, how far unit will search for enemy when guarding
#define DEFAULT_STARTINGCREDITS 3000

#define HUMANPLAYERCLASS        "HumanPlayer"
#define DEFAULTAIPLAYERCLASS    "qBotMedium"


#ifndef RESTRICT
#if defined(_MSC_VER)
#define RESTRICT __restrict
#elif defined(__GNUC__) || defined(__clang__)
#define RESTRICT __restrict__
#else
#define RESTRICT
#endif
#endif // RESTRICT

#endif //DEFINITIONS_H
