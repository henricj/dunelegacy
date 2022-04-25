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
#include <array>
#include <cmath>
#include <limits>
#include <string>
#include <unordered_set>

struct DuneTexture;

using zoomable_texture = std::array<const DuneTexture*, NUM_ZOOMLEVEL>;

class Coord final {
public:
    constexpr Coord() noexcept : x(0), y(0) { }

    constexpr Coord(int x, int y) noexcept : x(x), y(y) { }

    constexpr bool operator==(const Coord& c) const noexcept { return x == c.x && y == c.y; }

    constexpr bool operator!=(const Coord& c) const noexcept { return !operator==(c); }

    constexpr Coord& operator+=(const Coord& c) noexcept {
        x += c.x;
        y += c.y;
        return *this;
    }

    constexpr Coord operator+(const Coord& c) const noexcept {
        Coord ret = *this;
        ret += c;
        return ret;
    }

    constexpr Coord& operator-=(const Coord& c) noexcept {
        x -= c.x;
        y -= c.y;
        return *this;
    }

    constexpr Coord operator-(const Coord& c) const noexcept {
        Coord ret = *this;
        ret -= c;
        return ret;
    }

    constexpr Coord& operator*=(int c) noexcept {
        x *= c;
        y *= c;
        return *this;
    }

    constexpr Coord operator*(int c) const noexcept {
        auto ret = *this;
        ret *= c;
        return ret;
    }

    constexpr Coord& operator/=(int c) {
        x /= c;
        y /= c;
        return *this;
    }

    constexpr Coord operator/(int c) const {
        auto ret = *this;
        ret /= c;
        return ret;
    }

    void invalidate() noexcept {
        x = INVALID_POS;
        y = INVALID_POS;
    }

    [[nodiscard]] constexpr bool isValid() const noexcept { return x != INVALID_POS && y != INVALID_POS; }

    [[nodiscard]] constexpr bool isInvalid() const noexcept { return x == INVALID_POS || y == INVALID_POS; }

    static constexpr Coord Invalid() noexcept { return Coord(INVALID_POS, INVALID_POS); }

    explicit constexpr operator bool() const noexcept { return isValid(); }

public:
    int x;
    int y;
};

class CoordF final {
public:
    static inline constexpr auto INVALID_POSF = std::numeric_limits<float>::quiet_NaN();

    constexpr CoordF() noexcept : x{}, y() { }

    constexpr CoordF(float x, float y) noexcept : x(x), y(y) { }

    constexpr CoordF(int x, int y) noexcept : x(static_cast<float>(x)), y(static_cast<float>(y)) { }

    constexpr bool operator==(const CoordF& c) const noexcept { return x == c.x && y == c.y; }

    constexpr bool operator!=(const CoordF& c) const noexcept { return !operator==(c); }

    constexpr CoordF& operator+=(const CoordF& c) noexcept {
        x += c.x;
        y += c.y;
        return *this;
    }

    constexpr CoordF operator+(const CoordF& c) const noexcept {
        CoordF ret = *this;
        ret += c;
        return ret;
    }

    constexpr CoordF& operator-=(const CoordF& c) noexcept {
        x -= c.x;
        y -= c.y;
        return *this;
    }

    constexpr CoordF operator-(const CoordF& c) const noexcept {
        CoordF ret = *this;
        ret -= c;
        return ret;
    }

    constexpr CoordF& operator*=(float c) noexcept {
        x *= c;
        y *= c;
        return *this;
    }

    constexpr CoordF operator*(float c) const noexcept {
        auto ret = *this;
        ret *= c;
        return ret;
    }

    constexpr CoordF& operator/=(float c) {
        x /= c;
        y /= c;
        return *this;
    }

    constexpr CoordF operator/(float c) const {
        auto ret = *this;
        ret /= c;
        return ret;
    }

    void invalidate() noexcept {
        x = INVALID_POS;
        y = INVALID_POS;
    }

    [[nodiscard]] bool isValid() const noexcept { return !isInvalid(); }

    [[nodiscard]] bool isInvalid() const noexcept { return std::isnan(x) || std::isnan(y); }

    static constexpr CoordF Invalid() noexcept { return {INVALID_POSF, INVALID_POSF}; }

    explicit operator bool() const noexcept { return isValid(); }

public:
    float x;
    float y;
};

typedef enum : int8_t {
    ATTACKMODE_INVALID [[maybe_unused]] = -1,
    GUARD     = 0, ///< The unit will attack enemy units but will not move or follow enemy units.
    AREAGUARD = 1, ///< Area Guard is the most common command for pre-placed AI units. They will scan for targets in a
                   ///< relatively large radius, and return to their original position after their target was either
                   ///< destroyed or left the immediate area.
    AMBUSH = 2, ///< Ambush means a unit will remain in position until sighted by the enemy, and then proceed to attack
                ///< any enemy units it might find on the map.
    HUNT = 3, ///< Hunt makes a unit start from its position towards enemy units, even if the player has not sighted the
              ///< AI (normally the AI will not attack until there has been a contact between the player's and the AI's
              ///< units). Also works for human units, they'll go towards any enemy units on the map just as the mission
              ///< starts.
    HARVEST           = 4, ///< Only used by the map editor
    SABOTAGE          = 5, ///< Only used by the map editor
    STOP              = 6,
    CAPTURE           = 7, ///< Capture is only used for infantry units when ordered to capture a building
    CARRYALLREQUESTED = 8, ///< This allows a unit to keep requesting a carryall even if one isn't available right now
    RETREAT           = 9, ///< Ignore other units
    ATTACKMODE_MAX [[maybe_unused]]
} ATTACKMODE;

enum class GameState { Start, Loading, Running, Deinitialize };

enum class GameType {
    Invalid [[maybe_unused]] = -1,
    LoadSavegame             = 0,
    Campaign                 = 1,
    CustomGame               = 2,
    Skirmish                 = 3,
    CustomMultiplayer        = 4,
    LoadMultiplayer          = 5
};

class SettingsClass {
public:
    class GeneralClass {
    public:
        bool playIntro;         ///< Play the intro when starting the game?
        std::string playerName; ///< The name of the player used for multiplayer games
        std::string language;   ///< Language code: "en" = English, "fr" = French, "de" = German
        int scrollSpeed;        ///< Scroll speed in pixels
        bool showTutorialHints; ///< If true, tutorial hints are shown during the game
    } general;

    class VideoClass {
    public:
        bool fullscreen;
        int physicalWidth;
        int physicalHeight;
        int width;
        int height;
        bool frameLimit;
        int preferredZoomLevel;
        std::string scaler;
        bool rotateUnitGraphics;
        std::string renderer;
        std::string typeface;
    } video;

    class AudioClass {
    public:
        bool playSFX;
        int sfxVolume;
        bool playMusic;
        int musicVolume;
        std::string musicType;
    } audio;

    class NetworkClass {
    public:
        int serverPort;
        std::string metaServer;
        bool debugNetwork;
    } network;

    class AIClass {
    public:
        std::string campaignAI;
    } ai;

    class GameOptionsClass {
    public:
        GameOptionsClass() noexcept
            : gameSpeed(GAMESPEED_DEFAULT), concreteRequired(true), structuresDegradeOnConcrete(true), fogOfWar(false),
              startWithExploredMap(false), instantBuild(false), onlyOnePalace(false), rocketTurretsNeedPower(false),
              sandwormsRespawn(false), killedSandwormsDropSpice(false), manualCarryallDrops(false),
              maximumNumberOfUnitsOverride(-1) { }

        bool operator==(const GameOptionsClass& goc) const noexcept {
            return gameSpeed == goc.gameSpeed && concreteRequired == goc.concreteRequired
                && structuresDegradeOnConcrete == goc.structuresDegradeOnConcrete && fogOfWar == goc.fogOfWar
                && startWithExploredMap == goc.startWithExploredMap && instantBuild == goc.instantBuild
                && onlyOnePalace == goc.onlyOnePalace && rocketTurretsNeedPower == goc.rocketTurretsNeedPower
                && sandwormsRespawn == goc.sandwormsRespawn && killedSandwormsDropSpice == goc.killedSandwormsDropSpice
                && manualCarryallDrops == goc.manualCarryallDrops
                && maximumNumberOfUnitsOverride == goc.maximumNumberOfUnitsOverride;
        }

        bool operator!=(const GameOptionsClass& goc) const { return !this->operator==(goc); }

        int gameSpeed;
        bool concreteRequired;
        bool structuresDegradeOnConcrete;
        bool fogOfWar;
        bool startWithExploredMap;
        bool instantBuild;
        bool onlyOnePalace;
        bool rocketTurretsNeedPower;
        bool sandwormsRespawn;
        bool killedSandwormsDropSpice;
        bool manualCarryallDrops;
        int maximumNumberOfUnitsOverride;
    } gameOptions;
};

enum class HOUSETYPE : int8_t {
    HOUSE_UNUSED [[maybe_unused]]  = -2,
    HOUSE_INVALID [[maybe_unused]] = -1,
    HOUSE_HARKONNEN                = 0,
    HOUSE_ATREIDES                 = 1,
    HOUSE_ORDOS                    = 2,
    HOUSE_FREMEN                   = 3,
    HOUSE_SARDAUKAR                = 4,
    HOUSE_MERCENARY                = 5,
    NUM_HOUSES [[maybe_unused]],
    HOUSE_FIRST [[maybe_unused]] = HOUSE_HARKONNEN,
    HOUSE_LAST [[maybe_unused]]  = HOUSE_MERCENARY
};

template<typename F>
void for_each_housetype(F&& f) {
    for (auto i = 0; i < static_cast<int>(HOUSETYPE::NUM_HOUSES); ++i)
        f(static_cast<HOUSETYPE>(i));
}

enum class ANGLETYPE : int8_t {
    RIGHT,
    RIGHTUP,
    UP,
    LEFTUP,
    LEFT,
    LEFTDOWN,
    DOWN,
    RIGHTDOWN,
    NUM_ANGLES [[maybe_unused]],
    INVALID_ANGLE [[maybe_unused]] = INVALID
};

enum class DropLocation : int8_t {
    Drop_Invalid [[maybe_unused]] = -1,
    Drop_North,     ///< unit will appear at a random position at the top of the map
    Drop_East,      ///< unit will appear at a random position on the right side of the map
    Drop_South,     ///< unit will appear at a random position at the bottom of the map
    Drop_West,      ///< unit will appear at a random position on the left side of the map
    Drop_Air,       ///< unit will be dropped at a random position
    Drop_Visible,   ///< unit will be dropped at a random position in the middle of the map
    Drop_Enemybase, ///< unit will be dropped near the enemy base
    Drop_Homebase   ///< unit will be dropped near the base of the owner of the new unit
};

enum class AITeamBehavior {
    AITeamBehavior_Invalid [[maybe_unused]] = -1,
    AITeamBehavior_Normal,   ///< Attack units and/or structures when building up the team is complete
    AITeamBehavior_Guard,    ///< Same as AITeamBehavior_Normal
    AITeamBehavior_Kamikaze, ///< Directly attack structures when building up the team is complete
    AITeamBehavior_Staging,  ///< A team in the process of being built up
    AITeamBehavior_Flee      ///< Do nothing (Unimplemented in Dune II?)
};

enum class AITeamType {
    AITeamType_Invalid [[maybe_unused]] = -1,
    AITeamType_Foot,
    AITeamType_Wheeled,
    AITeamType_Tracked,
    AITeamType_Winged,
    AITeamType_Slither,
    AITeamType_Harvester
};

namespace Dune {
using object_id_type    = uint32_t;
using selected_set_type = std::unordered_set<object_id_type>;
} // namespace Dune

#endif // DATATYPES_H
