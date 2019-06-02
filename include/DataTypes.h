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

#ifndef DATATYPES_H
#define DATATYPES_H

#include <Definitions.h>
#include <misc/SDL2pp.h>

// Libraries
#include <string>
#include <array>


typedef std::array<SDL_Texture*, NUM_ZOOMLEVEL> zoomable_texture;


class Coord {
public:
    Coord()
     : x(0), y(0) {
    }

    Coord(int x,int y)
     : x(x), y(y)
    {
    }

    inline bool operator==(const Coord& c) const {
        return (x == c.x && y == c.y);
    }

    inline bool operator!=(const Coord& c) const {
        return !operator==(c);
    }

    inline Coord& operator+=(const Coord& c) {
        x += c.x;
        y += c.y;
        return *this;
    }

    inline Coord operator+(const Coord& c) const {
        Coord ret = *this;
        ret += c;
        return ret;
    }

    inline Coord& operator-=(const Coord& c) {
        x -= c.x;
        y -= c.y;
        return *this;
    }

    inline Coord operator-(const Coord& c) const {
        Coord ret = *this;
        ret -= c;
        return ret;
    }

    inline Coord& operator*=(int c) {
        x *= c;
        y *= c;
        return *this;
    }

    inline Coord operator*(int c) const {
        Coord ret = *this;
        ret *= c;
        return ret;
    }

    inline Coord& operator/=(int c) {
        x /= c;
        y /= c;
        return *this;
    }

    inline Coord operator/(int c) const {
        Coord ret = *this;
        ret /= c;
        return ret;
    }

    inline void invalidate() {
        x = INVALID_POS;
        y = INVALID_POS;
    }

    inline bool isValid() const {
        return ((x != INVALID_POS) && (y != INVALID_POS));
    }

    inline bool isInvalid() const {
        return ((x == INVALID_POS) || (y == INVALID_POS));
    }

    static inline const Coord Invalid() {
        return Coord(INVALID_POS, INVALID_POS);
    }

    inline operator bool() const {
        return isValid();
    };

public:
    int x;
    int y;
};

typedef enum {
    ATTACKMODE_INVALID = -1,
    GUARD = 0,      ///< The unit will attack enemy units but will not move or follow enemy units.
    AREAGUARD = 1,  ///< Area Guard is the most common command for pre-placed AI units. They will scan for targets in a relatively large radius, and return to their original position after their target was either destroyed or left the immediate area.
    AMBUSH = 2,     ///< Ambush means a unit will remain in position until sighted by the enemy, and then proceed to attack any enemy units it might find on the map.
    HUNT = 3,       ///< Hunt makes a unit start from its position towards enemy units, even if the player has not sighted the AI (normally the AI will not attack until there has been a contact between the player's and the AI's units). Also works for human units, they'll go towards any enemy units on the map just as the mission starts.
    HARVEST = 4,    ///< Only used by the map editor
    SABOTAGE = 5,   ///< Only used by the map editor
    STOP = 6,
    CAPTURE = 7,    ///< Capture is only used for infantry units when ordered to capture a building
    CARRYALLREQUESTED = 8, ///< This allows a unit to keep requesting a carryall even if one isn't available right now
    RETREAT = 9,           ///< Ignore other units
    ATTACKMODE_MAX
} ATTACKMODE;

enum class GameState {
    Start,
    Loading,
    Running,
    Deinitialize
};

enum class GameType {
    Invalid           = -1,
    LoadSavegame      = 0,
    Campaign          = 1,
    CustomGame        = 2,
    Skirmish          = 3,
    CustomMultiplayer = 4,
    LoadMultiplayer   = 5
};


class SettingsClass
{
public:
    class GeneralClass {
    public:
        bool            playIntro;          ///< Play the intro when starting the game?
        std::string     playerName;         ///< The name of the player used for multiplayer games
        std::string     language;           ///< Language code: "en" = English, "fr" = French, "de" = German
        int             scrollSpeed;        ///< Scroll speed in pixels
        bool            showTutorialHints;  ///< If true, tutorial hints are shown during the game
    } general;

    class VideoClass {
    public:
        bool        fullscreen;
        int         physicalWidth;
        int         physicalHeight;
        int         width;
        int         height;
        bool        frameLimit;
        int         preferredZoomLevel;
        std::string scaler;
        bool        rotateUnitGraphics;
    } video;

    class AudioClass {
    public:
        bool        playSFX;
        int         sfxVolume;
        bool        playMusic;
        int         musicVolume;
        std::string musicType;
    } audio;

    class NetworkClass {
    public:
        int         serverPort;
        std::string metaServer;
        bool        debugNetwork;
    } network;

    class AIClass {
    public:
        std::string campaignAI;
    } ai;

    class GameOptionsClass {
    public:
        GameOptionsClass()
         : gameSpeed(GAMESPEED_DEFAULT), concreteRequired(true), structuresDegradeOnConcrete(true), fogOfWar(false),
           startWithExploredMap(false), instantBuild(false), onlyOnePalace(false), rocketTurretsNeedPower(false),
           sandwormsRespawn(false), killedSandwormsDropSpice(false), manualCarryallDrops(false), maximumNumberOfUnitsOverride(-1)  {
        }


        bool operator==(const GameOptionsClass& goc) const {
            return (gameSpeed == goc.gameSpeed)
                    && (concreteRequired == goc.concreteRequired)
                    && (structuresDegradeOnConcrete == goc.structuresDegradeOnConcrete)
                    && (fogOfWar == goc.fogOfWar)
                    && (startWithExploredMap == goc.startWithExploredMap)
                    && (instantBuild == goc.instantBuild)
                    && (onlyOnePalace == goc.onlyOnePalace)
                    && (rocketTurretsNeedPower == goc.rocketTurretsNeedPower)
                    && (sandwormsRespawn == goc.sandwormsRespawn)
                    && (killedSandwormsDropSpice == goc.killedSandwormsDropSpice)
                    && (manualCarryallDrops == goc.manualCarryallDrops)
                    && (maximumNumberOfUnitsOverride == goc.maximumNumberOfUnitsOverride);
        }

        bool operator!=(const GameOptionsClass& goc) const {
            return !this->operator==(goc);
        }

        int         gameSpeed;
        bool        concreteRequired;
        bool        structuresDegradeOnConcrete;
        bool        fogOfWar;
        bool        startWithExploredMap;
        bool        instantBuild;
        bool        onlyOnePalace;
        bool        rocketTurretsNeedPower;
        bool        sandwormsRespawn;
        bool        killedSandwormsDropSpice;
        bool        manualCarryallDrops;
        int         maximumNumberOfUnitsOverride;
    } gameOptions;
};

typedef enum
{
    HOUSE_UNUSED    = -2,
    HOUSE_INVALID   = -1,
    HOUSE_HARKONNEN =  0,
    HOUSE_ATREIDES  =  1,
    HOUSE_ORDOS     =  2,
    HOUSE_FREMEN    =  3,
    HOUSE_SARDAUKAR =  4,
    HOUSE_MERCENARY =  5,
    NUM_HOUSES
} HOUSETYPE;

typedef enum {
    RIGHT,
    RIGHTUP,
    UP,
    LEFTUP,
    LEFT,
    LEFTDOWN,
    DOWN,
    RIGHTDOWN,
    NUM_ANGLES
} ANGLETYPE;

typedef enum  {
    Drop_Invalid = -1,
    Drop_North,         ///< unit will appear at a random position at the top of the map
    Drop_East,          ///< unit will appear at a random position on the right side of the map
    Drop_South,         ///< unit will appear at a random position at the bottom of the map
    Drop_West,          ///< unit will appear at a random position on the left side of the map
    Drop_Air,           ///< unit will be dropped at a random position
    Drop_Visible,       ///< unit will be dropped at a random position in the middle of the map
    Drop_Enemybase,     ///< unit will be dropped near the enemy base
    Drop_Homebase       ///< unit will be dropped near the base of the owner of the new unit
} DropLocation;

typedef enum {
    AITeamBehavior_Invalid = -1,
    AITeamBehavior_Normal,            ///< Attack units and/or structures when building up the team is complete
    AITeamBehavior_Guard,             ///< Same as AITeamBehavior_Normal
    AITeamBehavior_Kamikaze,          ///< Directly attack structures when building up the team is complete
    AITeamBehavior_Staging,           ///< A team in the process of being built up
    AITeamBehavior_Flee               ///< Do nothing (Unimplemented in Dune II?)
} AITeamBehavior;

typedef enum {
    AITeamType_Invalid = -1,
    AITeamType_Foot,
    AITeamType_Wheeled,
    AITeamType_Tracked,
    AITeamType_Winged,
    AITeamType_Slither,
    AITeamType_Harvester
} AITeamType;

#endif //DATATYPES_H
