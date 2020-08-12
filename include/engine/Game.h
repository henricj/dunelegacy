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

#ifndef ENGINE_GAME_H
#define ENGINE_GAME_H


inline constexpr int xyz2b = 123;
namespace {
inline constexpr int abc2b = ::xyz2b;
}

#include <misc/Random.h>
#include <misc/InputStream.h>
#include <misc/OutputStream.h>

inline constexpr int xyz2c = 123;
namespace {
inline constexpr int abc2c = ::xyz2c;
}

#include <ObjectData.h>
#include <ObjectManager.h>
#include <CommandManager.h>

inline constexpr int xyz2d = 123;
namespace {
inline constexpr int abc2d = ::xyz2d;
}

#include <GameInitSettings.h>
#include <Trigger/TriggerManager.h>

inline constexpr int xyz2e = 123;
namespace {
inline constexpr int abc2e = ::xyz2e;
}

#include <players/Player.h>
#include <players/HumanPlayer.h>

#include <EngineDataTypes.h>

#include <cstdarg>
#include <string>

#include <array>
#include <filesystem>
#include <map>
#include <utility>
#include <unordered_set>


inline constexpr int xyz2 = 123;
namespace {
inline constexpr int abc2 = ::xyz2;
}

namespace Dune::Engine {

// forward declarations
class ObjectBase;
class InGameMenu;
class MentatHelp;
class WaitingForOtherPlayers;
class ObjectManager;
class House;
class Explosion;

class Game final {
public:
    /**
        Default constructor. Call initGame() or initReplay() afterwards.
    */
    Game();

    Game(const Game& o) = delete;
    Game(Game&& o)      = delete;

    /**
        Destructor
    */
    ~Game();

    Game& operator=(const Game&) = delete;
    Game& operator=(Game&&) = delete;

    /**
        Initializes a game with the specified settings
        \param  newGameInitSettings the game init settings to initialize the game
    */
    void initGame(const GameInitSettings& newGameInitSettings);

    /**
        Initializes a replay from the specified filename
        \param  filename    the file containing the replay
    */
    void initReplay(const std::filesystem::path& filename);

    friend class INIMapLoader; // loading INI Maps is done with a INIMapLoader helper object

    /**
        This method processes all objects in the current game. It should be executed exactly once per game tick.
    */
    void processObjects();


    /**
        Returns the current game cycle number.
        \return the current game cycle
    */
    [[nodiscard]] uint32_t getGameCycleCount() const noexcept { return gameCycleCount; };

    /**
        Return the game time in milliseconds.
        \return the current game time in milliseconds
    */
    [[nodiscard]] uint32_t getGameTime() const noexcept { return gameCycleCount * GAMESPEED_DEFAULT; };

    /**
        Get the command manager of this game
        \return the command manager
    */
    CommandManager& getCommandManager() noexcept { return cmdManager; };

    /**
        Get the trigger manager of this game
        \return the trigger manager
    */
    TriggerManager& getTriggerManager() noexcept { return triggerManager; };

    /**
        Add an explosion.
    */
    template<class... Args>
    void addExplosion(Args&&... args) {
        explosionList.emplace_back(std::make_unique<Explosion>(std::forward<Args>(args)...));
    }

    [[nodiscard]] int getHouseIndex(HOUSETYPE houseID) const {
        const auto int_house = static_cast<int>(houseID);

        if(int_house < 0 || int_house >= house.size())
            THROW(std::invalid_argument, "Invalid house index %d!", int_house);

        return int_house;
    }

    /**
        Returns the house with the id houseID
        \param  houseID the id of the house to return
        \return the house with id houseID
    */
    [[nodiscard]] House* getHouse(HOUSETYPE houseID) const {
        const auto int_house = getHouseIndex(houseID);

        return house[int_house].get();
    }

    template<typename F>
    void for_each_house(F&& f) const {
        for(const auto& h : house) {
            if(h) f(*h.get());
        }
    }

    template<typename F>
    House* house_find_if(F&& predicate) {
        for(auto& h : house) {
            if(h) {
                if(predicate(*h.get())) return h.get();
            }
        }
        return nullptr;
    }

    Map* getMap() { return map ? map.get() : currentGameMap; }

    /**
        The current game is finished and the local house has won
    */
    void setGameWon();

    /**
        The current game is finished and the local house has lost
    */
    void setGameLost();

    /**
        This method loads a previously saved game.
        \param filename the name of the file to load from
        \return true on success, false on failure
    */
    bool loadSaveGame(const ::std::filesystem::path& filename);

    /**
        This method loads a previously saved game.
        \param stream the stream to load from
        \return true on success, false on failure
    */
    bool loadSaveGame(InputStream& stream);

    /**
        This method saves the current running game.
        \param filename the name of the file to save to
        \return true on success, false on failure
    */
    bool saveGame(const std::filesystem::path& filename);

    /**
        This method starts the game. Will return when the game is finished or aborted.
    */
    void runMainLoop(const GameContext& context);

    void quitGame() { bQuitGame = true; };

    /**
        This method resumes the current paused game.
    */
    void resumeGame();

    /**
        This method writes out an object to a stream.
        \param stream   the stream to write to
        \param obj      the object to be saved
    */
    static void saveObject(OutputStream& stream, ObjectBase* obj);

    /**
        This method loads an object from the stream.
        \param stream   the stream to read from
        \param objectID the object id that this unit/structure should get
        \return the read unit/structure
    */
    static ::std::unique_ptr<ObjectBase> loadObject(InputStream& stream, uint32_t objectID);

    ObjectManager&                     getObjectManager() noexcept { return objectManager; };
    [[nodiscard]] const ObjectManager& getObjectManager() const noexcept { return objectManager; };

    [[nodiscard]] const GameInitSettings& getGameInitSettings() const noexcept { return gameInitSettings; };
    void                                  setNextGameInitSettings(const GameInitSettings& nextGameInitSettings) {
        this->nextGameInitSettings = nextGameInitSettings;
    };

    /**
        This method should be called if whatNext() returns GAME_NEXTMISSION or GAME_LOAD. You should
        destroy this Game and create a new one. Call Game::initGame() with the GameInitClass
        that was returned previously by getNextGameInitSettings().
        \return a GameInitSettings-Object that describes the next game.
    */
    GameInitSettings getNextGameInitSettings();

    /**
        This method should be called after startGame() has returned. whatNext() will tell the caller
        what should be done after the current game has finished.<br>
        Possible return values are:<br>
        GAME_RETURN_TO_MENU  - the game is finished and you should return to the main menu<br>
        GAME_NEXTMISSION     - the game is finished and you should load the next mission<br>
        GAME_LOAD            - from inside the game the user requests to load a savegame and you should do this now<br>
        GAME_DEBRIEFING_WIN  - show debriefing (player has won) and call whatNext() again afterwards<br>
        GAME_DEBRIEFING_LOST - show debriefing (player has lost) and call whatNext() again afterwards<br>
        <br>
        \return one of GAME_RETURN_TO_MENU, GAME_NEXTMISSION, GAME_LOAD, GAME_DEBRIEFING_WIN, GAME_DEBRIEFING_LOST
    */
    int whatNext();

    /**
        This method is the callback method for the OPTIONS button at the top of the screen.
        It pauses the game and loads the in game menu.
    */
    void onOptions();

    /**
        This method is the callback method for the MENTAT button at the top of the screen.
        It pauses the game and loads the mentat help screen.
    */
    void onMentat();

    /**
        This method returns wether the game is finished
        \return true, if paused, false otherwise
    */
    [[nodiscard]] bool isGameFinished() const noexcept { return finished; }

    /**
        Are cheats enabled?
        \return true = cheats enabled, false = cheats disabled
    */
    [[nodiscard]] bool areCheatsEnabled() const noexcept { return bCheatsEnabled; }

    /**
        Returns the name of the local player; this method should be used instead of using settings.general.playerName
       directly \return the local player name
    */
    [[nodiscard]] const std::string& getLocalPlayerName() const noexcept { return localPlayerName; }

    /**
        Register a new player in this game.
        \param  player      the player to register
    */
    void registerPlayer(Player* player) {
        playerName2Player.emplace(player->getPlayername(), player);
        playerID2Player[player->getPlayerID()] = player;
    }

    /**
        Unregisters the specified player
        \param  player
    */
    void unregisterPlayer(Player* player) {
        playerID2Player.erase(player->getPlayerID());

        const auto iter = std::find_if(playerName2Player.begin(), playerName2Player.end(),
                                       [=](decltype(playerName2Player)::reference kv) { return kv.second == player; });

        if(iter != playerName2Player.end()) playerName2Player.erase(iter);
    }

    /**
        Returns the first player with the given name.
        \param  playername  the name of the player
        \return the player or nullptr if none was found
    */
    [[nodiscard]] Player* getPlayerByName(const std::string& playername) const {
        const auto iter = playerName2Player.find(playername);
        if(iter != playerName2Player.end()) {
            return iter->second;
        }

        return nullptr;
    }

    /**
        Returns the player with the given id.
        \param  playerID  the name of the player
        \return the player or nullptr if none was found
    */
    [[nodiscard]] Player* getPlayerByID(uint8_t playerID) const {
        const auto iter = playerID2Player.find(playerID);
        if(iter != playerID2Player.end()) {
            return iter->second;
        }

        return nullptr;
    }

private:
    /**
        Checks whether the cursor is on the radar view
        \param  mouseX  x-coordinate of cursor
        \param  mouseY  y-coordinate of cursor
        \return true if on radar view
    */
    [[nodiscard]] bool isOnRadarView(int mouseX, int mouseY) const;


    /**
        Returns the game speed of this game: The number of ms per game cycle.
        For singleplayer games this is a global setting (but can be adjusted in the in-game settings menu). For
       multiplayer games the game speed can be set by the person creating the game. \return the current game speed
    */
    [[nodiscard]] int getGameSpeed() const;

    bool removeFromSelectionLists(ObjectBase* pObject);
    void removeFromQuickSelectionLists(uint32_t objectID);

    void serviceNetwork(bool& bWaitForNetwork);
    void updateGame(const GameContext& context);

    void doEventsUntil(const GameContext& context, int until);

public:

    GameType gameType  = GameType::Campaign;
    int      techLevel = 0;
    int      winFlags  = 0;
    int      loseFlags = 0;

    RandomFactory randomFactory;
    Random        randomGen;  ///< This is the random number generator for this game
    ObjectData    objectData; ///< This contains all the unit/structure data

    GameState gameState = GameState::Start;

private:

    int lastTargetGameCycleTime{}; //< Remember the last time the target gameCycleCount was updated

    uint32_t gameCycleCount = 0;

    uint32_t skipToGameCycle = 0; ///< skip to this game cycle

    ////////////////////

    GameInitSettings gameInitSettings;     ///< the init settings this game was started with
    GameInitSettings nextGameInitSettings; ///< the init settings the next game shall be started with (restarting the
                                           ///< mission, loading a savegame)
    GameInitSettings::HouseInfoList
        houseInfoListSetup; ///< this saves with which houses and players the game was actually set up. It is a copy of
                            ///< gameInitSettings::houseInfoList but without random houses

    ::Dune::Engine::ObjectManager objectManager; ///< This manages all the object and maps object ids to the actual objects

    CommandManager cmdManager; ///< This is the manager for all the game commands (e.g. moving a unit)

    TriggerManager triggerManager; ///< This is the manager for all the triggers the scenario has (e.g. reinforcements)

    std::unique_ptr<Map> map;

    bool bQuitGame = false; ///< Should the game quit after this game tick
    bool bReplay = false;   ///< Is this game actually a replay

    bool bCheatsEnabled = false; ///< Cheat codes are enabled?

    bool finished =
        false; ///< Is the game finished (won or lost) and we are just waiting for the end message to be shown
    bool     won               = false; ///< If the game is finished, is it won or lost
    uint32_t finishedLevelTime = 0;     ///< The time in milliseconds when the level was finished (won or lost)
    bool     finishedLevel     = false; ///< Set, when the game is really finished and the end message was shown

    std::vector<std::unique_ptr<Explosion>> explosionList; ///< A list containing all the explosions that must be drawn

    std::string localPlayerName; ///< the name of the local player
    std::unordered_multimap<std::string, Player*>
                                         playerName2Player; ///< mapping player names to players (one entry per player)
    std::unordered_map<uint8_t, Player*> playerID2Player;   ///< mapping player ids to players (one entry per player)

    std::array<std::unique_ptr<House>, static_cast<size_t>(HOUSETYPE::NUM_HOUSES)>
        house; ///< All the houses of this game, index by their houseID; has the size NUM_HOUSES; unused houses are
               ///< nullptr
};

} // namespace Dune::Engine

#endif // ENGINE_GAME_H
