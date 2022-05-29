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

#ifndef GAME_H
#define GAME_H

#include <CommandManager.h>
#include <GameInitSettings.h>
#include <GameInterface.h>
#include <INIMap/INIMapLoader.h>
#include <ObjectData.h>
#include <ObjectManager.h>
#include <Trigger/TriggerManager.h>
#include <misc/InputStream.h>
#include <misc/OutputStream.h>
#include <misc/Random.h>
#include <misc/dune_clock.h>
#include <players/HumanPlayer.h>
#include <players/Player.h>

#include <DataTypes.h>

#include <string>

#include <array>
#include <filesystem>
#include <unordered_set>
#include <utility>

// forward declarations
class ObjectBase;
class InGameMenu;
class MentatHelp;
class WaitingForOtherPlayers;
class ObjectManager;
class House;
class Explosion;

inline constexpr auto END_WAIT_TIME = dune::as_dune_clock_duration(6 * 1000);

inline constexpr auto GAME_NOTHING           = -1;
inline constexpr auto GAME_RETURN_TO_MENU    = 0;
inline constexpr auto GAME_NEXTMISSION       = 1;
inline constexpr auto GAME_LOAD              = 2;
inline constexpr auto GAME_DEBRIEFING_WIN    = 3;
inline constexpr auto GAME_DEBRIEFING_LOST   = 4;
inline constexpr auto GAME_CUSTOM_GAME_STATS = 5;

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
    Game& operator=(Game&&)      = delete;

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
        This method draws a complete frame.
    */
    void drawScreen();

    /**
        This method processes all the user input.
    */
    void doInput(const GameContext& context, SDL_Event& event);

    /**
        Returns the current game cycle number.
        \return the current game cycle
    */
    [[nodiscard]] uint32_t getGameCycleCount() const noexcept { return gameCycleCount_; }

    /**
        Return the game time in milliseconds.
        \return the current game time in milliseconds
    */
    [[nodiscard]] uint32_t getGameTime() const noexcept { return gameCycleCount_ * GAMESPEED_DEFAULT; }

    /**
        Get the command manager of this game
        \return the command manager
    */
    CommandManager& getCommandManager() noexcept { return cmdManager_; }

    /**
        Get the trigger manager of this game
        \return the trigger manager
    */
    TriggerManager& getTriggerManager() noexcept { return triggerManager_; }

    /**
        Add an explosion.
    */
    template<class... Args>
    void addExplosion(Args&&... args) {
        explosionList_.emplace_back(std::make_unique<Explosion>(std::forward<Args>(args)...));
    }

    [[nodiscard]] int getHouseIndex(HOUSETYPE houseID) const {
        const auto int_house = static_cast<int>(houseID);

        if (int_house < 0 || int_house >= house_.size())
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

        return house_[int_house].get();
    }

    template<typename F>
    void for_each_house(F&& f) const {
        for (const auto& h : house_) {
            if (h)
                f(*h.get());
        }
    }

    template<typename F>
    House* house_find_if(F&& predicate) {
        for (auto& h : house_) {
            if (h) {
                if (predicate(*h.get()))
                    return h.get();
            }
        }
        return nullptr;
    }

    Map* getMap() { return map_ ? map_.get() : dune::globals::currentGameMap; }

    /**
        The current game is finished and the local house has won
    */
    void setGameWon();

    /**
        The current game is finished and the local house has lost
    */
    void setGameLost();

    /**
        Draws the cursor.
    */
    void drawCursor(const SDL_Rect& map_rect) const;

    /**
        This method sets up the view. The start position is the center point of all owned units/structures
    */
    void setupView(const GameContext& context) const;

    /**
        This method loads a previously saved game.
        \param filename the name of the file to load from
        \return true on success, false on failure
    */
    bool loadSaveGame(const std::filesystem::path& filename);

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
    void runMainLoop(const GameContext& context, MenuBase::event_handler_type handler);

    void quitGame() { bQuitGame_ = true; }

    /**
        This method pauses the current game.
    */
    void pauseGame();

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

    ObjectManager& getObjectManager() noexcept { return objectManager_; }
    [[nodiscard]] const ObjectManager& getObjectManager() const noexcept { return objectManager_; }
    [[nodiscard]] GameInterface& getGameInterface() const noexcept { return *pInterface_; }

    [[nodiscard]] const GameInitSettings& getGameInitSettings() const noexcept { return gameInitSettings_; }
    void setNextGameInitSettings(const GameInitSettings& nextGameInitSettings) {
        this->nextGameInitSettings_ = nextGameInitSettings;
    }

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
        This method selects all units/structures in the list aList.
        \param aList the list containing all the units/structures to be selected
    */
    void selectAll(const Dune::selected_set_type& aList) const;

    /**
        This method unselects all units/structures in the list aList.
        \param aList the list containing all the units/structures to be unselected
    */
    void unselectAll(const Dune::selected_set_type& aList) const;

    /**
        Returns a list of all currently selected objects.
        \return list of currently selected units/structures
    */
    [[nodiscard]] Dune::selected_set_type& getSelectedList() noexcept { return selectedList_; }
    [[nodiscard]] const Dune::selected_set_type& getSelectedList() const noexcept { return selectedList_; }

    /**
        Unselect all objects in the selected list, clears the selected list, and sends a selected changed notification.
    */
    void clearSelectedList();

    /**
        Marks that the selection changed (and must be retransmitted to other players in multiplayer games)
    */
    void selectionChanged() {
        bSelectionChanged_ = true;
        if (pInterface_) {
            pInterface_->updateObjectInterface();
        }
        dune::globals::pLocalPlayer->onSelectionChanged(selectedList_);
    }

    void onReceiveSelectionList(const std::string& name, const Dune::selected_set_type& newSelectionList,
                                int groupListIndex);

    /**
        Returns a list of all currently by  the other player selected objects (Only in multiplayer with multiple players
       per house). \return list of currently selected units/structures by the other player
    */
    [[nodiscard]] Dune::selected_set_type& getSelectedByOtherPlayerList() noexcept {
        return selectedByOtherPlayerList_;
    }
    [[nodiscard]] const Dune::selected_set_type& getSelectedByOtherPlayerList() const noexcept {
        return selectedByOtherPlayerList_;
    }

    /**
        Called when a peer disconnects the game.
    */
    void onPeerDisconnected(const std::string& name, bool bHost, int cause) const;

    /**
        Adds a new message to the news ticker.
        \param  text    the text to add
    */
    void addToNewsTicker(const std::string& text) const {
        if (pInterface_ != nullptr) {
            pInterface_->addToNewsTicker(text);
        }
    }

    /**
        Adds an urgent message to the news ticker.
        \param  text    the text to add
    */
    void addUrgentMessageToNewsTicker(const std::string& text) const {
        if (pInterface_ != nullptr) {
            pInterface_->addUrgentMessageToNewsTicker(text);
        }
    }

    /**
        This method returns whether the game is currently paused
        \return true, if paused, false otherwise
    */
    [[nodiscard]] bool isGamePaused() const noexcept { return bPause_; }

    /**
        This method returns whether the game is finished
        \return true, if paused, false otherwise
    */
    [[nodiscard]] bool isGameFinished() const noexcept { return finished_; }

    /**
        Are cheats enabled?
        \return true = cheats enabled, false = cheats disabled
    */
    [[nodiscard]] bool areCheatsEnabled() const noexcept { return bCheatsEnabled_; }

    /**
        Returns the name of the local player; this method should be used instead of using settings.general.playerName
       directly \return the local player name
    */
    [[nodiscard]] const std::string& getLocalPlayerName() const noexcept { return localPlayerName_; }

    /**
        Register a new player in this game.
        \param  player      the player to register
    */
    void registerPlayer(Player* player) {
        playerName2Player_.emplace(player->getPlayername(), player);
        playerID2Player_[player->getPlayerID()] = player;
    }

    /**
        Unregisters the specified player
        \param  player
    */
    void unregisterPlayer(Player* player) {
        playerID2Player_.erase(player->getPlayerID());

        const auto iter = std::ranges::find_if(
            playerName2Player_, [=](decltype(playerName2Player_)::reference kv) { return kv.second == player; });

        if (iter != playerName2Player_.end())
            playerName2Player_.erase(iter);
    }

    /**
        Returns the first player with the given name.
        \param  playername  the name of the player
        \return the player or nullptr if none was found
    */
    [[nodiscard]] Player* getPlayerByName(const std::string& playername) const {
        const auto iter = playerName2Player_.find(playername);
        if (iter != playerName2Player_.end()) {
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
        const auto iter = playerID2Player_.find(playerID);
        if (iter != playerID2Player_.end()) {
            return iter->second;
        }
        return nullptr;
    }

    /**
        This function is called when the user left clicks on the radar
        \param  worldPosition       position in world coordinates
        \param  bRightMouseButton   true = right mouse button, false = left mouse button
        \param  bDrag               true = the mouse was moved while being pressed, e.g. dragging
        \return true if dragging should start or continue
    */
    bool onRadarClick(const GameContext& context, Coord worldPosition, bool bRightMouseButton, bool bDrag);

    /**
        Request a screenshot
    */
    void takeScreenshot() { pendingScreenshot_ = true; }

private:
    /**
        Take a screenshot and save it with a unique name
    */
    void saveScreenshot();

    /**
        Checks whether the cursor is on the radar view
        \param  mouseX  x-coordinate of cursor
        \param  mouseY  y-coordinate of cursor
        \return true if on radar view
    */
    [[nodiscard]] bool isOnRadarView(int mouseX, int mouseY) const;

    /**
        Handles the press of one key while chatting
        \param keyboardEvent the key pressed
    */
    void handleChatInput(const GameContext& context, SDL_KeyboardEvent& keyboardEvent);

    /**
        Handles the press of one key
        \param  keyboardEvent the key pressed
    */
    void handleKeyInput(const GameContext& context, SDL_KeyboardEvent& keyboardEvent);

    /**
        Performs a building placement
        \param  xPos    x-coordinate in map coordinates
        \param  yPos    x-coordinate in map coordinates
        \return true if placement was successful
    */
    bool handlePlacementClick(const GameContext& context, int xPos, int yPos);

    /**
        Performs a attack click for the currently selected units/structures.
        \param  xPos    x-coordinate in map coordinates
        \param  yPos    x-coordinate in map coordinates
        \return true if attack is possible
    */
    bool handleSelectedObjectsAttackClick(const GameContext& context, int xPos, int yPos);

    /**
        Performs a move click for the currently selected units/structures.
        \param  xPos    x-coordinate in map coordinates
        \param  yPos    x-coordinate in map coordinates
        \return true if move is possible
    */
    bool handleSelectedObjectsMoveClick(const GameContext& context, int xPos, int yPos);

    /**
        Performs a capture click for the currently selected units/structures.
        \param  xPos    x-coordinate in map coordinates
        \param  yPos    x-coordinate in map coordinates
        \return true if capture is possible
    */
    bool handleSelectedObjectsCaptureClick(const GameContext& context, int xPos, int yPos);

    /**
        Performs a request carryall click for the currently selected units.
        \param  xPos    x-coordinate in map coordinates
        \param  yPos    x-coordinate in map coordinates
        \return true if carryall drop is possible
    */
    bool handleSelectedObjectsRequestCarryallDropClick(const GameContext& context, int xPos, int yPos);

    /**
        Performs an action click for the currently selected units/structures.
        \param  xPos    x-coordinate in map coordinates
        \param  yPos    x-coordinate in map coordinates
        \return true if action click is possible
    */
    bool handleSelectedObjectsActionClick(const GameContext& context, int xPos, int yPos);

    /**
        Selects the next structure of any of the types specified in itemIDs. If none of this type is currently selected
       the first one is selected. \param  itemIDs  the ids of the structures to select
    */
    void selectNextStructureOfType(const Dune::selected_set_type& itemIDs);

    /**
        Returns the game speed of this game: The wall clock time per game cycle.
        For singleplayer games this is a global setting (but can be adjusted in the in-game settings menu). For
       multiplayer games the game speed can be set by the person creating the game.
       \return the current game speed
    */
    [[nodiscard]] dune::dune_clock::duration getGameSpeed() const;

    bool removeFromSelectionLists(ObjectBase* pObject);
    void removeFromQuickSelectionLists(uint32_t objectID);

    void serviceNetwork(bool& bWaitForNetwork);
    void updateGame(const GameContext& context);

    void doEventsUntil(const GameContext& context, dune::dune_clock::time_point until);

public:
    enum {
        CursorMode_Normal,
        CursorMode_Attack,
        CursorMode_Move,
        CursorMode_Capture,
        CursorMode_CarryallDrop,
        CursorMode_Placing
    };

    int currentCursorMode = CursorMode_Normal;

    GameType gameType = GameType::Campaign;
    int techLevel     = 0;
    int winFlags      = 0;
    int loseFlags     = 0;

    RandomFactory randomFactory;
    Random randomGen;      ///< This is the random number generator for this game
    ObjectData objectData; ///< This contains all the unit/structure data

    GameState gameState = GameState::Start;

private:
    void resize();

    bool chatMode_ = false;         ///< chat mode on?
    std::string typingChatMessage_; ///< currently typed chat message

    bool scrollDownMode_  = false; ///< currently scrolling the map down?
    bool scrollLeftMode_  = false; ///< currently scrolling the map left?
    bool scrollRightMode_ = false; ///< currently scrolling the map right?
    bool scrollUpMode_    = false; ///< currently scrolling the map up?

    bool selectionMode_ = false; ///< currently selection multiple units with a selection rectangle?
    SDL_Rect selectionRect_{};   ///< the drawn rectangle while selection multiple units

    int whatNextParam_ = GAME_NOTHING;

    uint32_t indicatorFrame_ = NONE_ID;
    int indicatorTime_       = 5;
    int indicatorTimer_      = 0;
    Coord indicatorPosition_ = Coord::Invalid();

    float averageFrameTime_ = 31.25f; ///< The weighted average of the frame time of all previous frames (smoothed fps =
                                      ///< 1000.0f/averageFrameTime)
    float averageRenderTime_ = 10.0f; ///< The weighted average of the render time
    float averageUpdateTime_ = 10.0f; ///< The weighted average of the update time

    dune::dune_clock::time_point
        lastTargetGameCycleTime_{}; //< Remember the last time the target gameCycleCount was updated

    uint32_t gameCycleCount_ = 0;

    uint32_t skipToGameCycle_ = 0; ///< skip to this game cycle

    bool takePeriodicScreenshots_ = false; ///< take a screenshot every 10 seconds
    bool pendingScreenshot_       = false;

    SDL_FRect powerIndicatorPos_{14, 146, 4, 0}; ///< position of the power indicator in the right game bar
    SDL_FRect spiceIndicatorPos_{20, 146, 4, 0}; ///< position of the spice indicator in the right game bar
    SDL_FRect topBarPos_{};                      ///< position of the top game bar
    SDL_FRect sideBarPos_{};                     ///< position of the right side bar

    ////////////////////

    GameInitSettings gameInitSettings_;     ///< the init settings this game was started with
    GameInitSettings nextGameInitSettings_; ///< the init settings the next game shall be started with (restarting the
                                            ///< mission, loading a savegame)
    GameInitSettings::HouseInfoList
        houseInfoListSetup_; ///< this saves with which houses and players the game was actually set up. It is a copy of
                             ///< gameInitSettings::houseInfoList but without random houses

    ObjectManager objectManager_; ///< This manages all the object and maps object ids to the actual objects

    CommandManager cmdManager_; ///< This is the manager for all the game commands (e.g. moving a unit)

    TriggerManager triggerManager_; ///< This is the manager for all the triggers the scenario has (e.g. reinforcements)

    std::unique_ptr<Map> map_;

    bool bQuitGame_ = false;                       ///< Should the game quit after this game tick
    bool bPause_    = false;                       ///< Is the game currently halted
    dune::dune_clock::time_point pauseGameTime_{}; ///< Remember when the game was paused
    bool bMenu_   = false;                         ///< Is there currently a menu shown (options or mentat menu)
    bool bReplay_ = false;                         ///< Is this game actually a replay

    bool bShowFPS_ = false; ///< Show the FPS

    bool bShowTime_ = false; ///< Show how long this game is running

    bool bCheatsEnabled_ = false; ///< Cheat codes are enabled?

    bool finished_ =
        false;         ///< Is the game finished (won or lost) and we are just waiting for the end message to be shown
    bool won_ = false; ///< If the game is finished, is it won or lost
    dune::dune_clock::time_point
        finishedLevelTime_{};    ///< The time in milliseconds when the level was finished (won or lost)
    bool finishedLevel_ = false; ///< Set, when the game is really finished and the end message was shown

    std::unique_ptr<GameInterface> pInterface_; ///< This is the whole interface (top bar and side bar)
    std::unique_ptr<InGameMenu> pInGameMenu_;   ///< This is the menu that is opened by the option button
    std::unique_ptr<MentatHelp> pInGameMentat_; ///< This is the mentat dialog opened by the mentat button
    std::unique_ptr<WaitingForOtherPlayers> pWaitingForOtherPlayers_; ///< This is the dialog that pops up when we are
                                                                      ///< waiting for other players during network hangs
    dune::dune_clock::time_point
        startWaitingForOtherPlayersTime_{}; ///< The time in milliseconds when we started waiting for other players

    bool bSelectionChanged_ =
        false; ///< Has the selected list changed (and must be retransmitted to other plays in multiplayer games)
    Dune::selected_set_type selectedList_; ///< A set of all selected units/structures
    Dune::selected_set_type
        selectedByOtherPlayerList_; ///< This is only used in multiplayer games where two players control one house
    std::vector<std::unique_ptr<Explosion>> explosionList_; ///< A list containing all the explosions that must be drawn

    std::string localPlayerName_; ///< the name of the local player
    std::unordered_multimap<std::string, Player*>
        playerName2Player_;                                ///< mapping player names to players (one entry per player)
    std::unordered_map<uint8_t, Player*> playerID2Player_; ///< mapping player ids to players (one entry per player)

    std::array<std::unique_ptr<House>, NUM_HOUSES> house_; ///< All the houses of this game, index by their houseID; has
                                                           ///< the size NUM_HOUSES; unused houses are nullptr
    MenuBase::event_handler_type sdl_handler_;
};

#endif // GAME_H
