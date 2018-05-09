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

#include <misc/Random.h>
#include <misc/RobustList.h>
#include <misc/InputStream.h>
#include <misc/OutputStream.h>
#include <ObjectData.h>
#include <ObjectManager.h>
#include <CommandManager.h>
#include <GameInterface.h>
#include <INIMap/INIMapLoader.h>
#include <GameInitSettings.h>
#include <Trigger/TriggerManager.h>
#include <players/Player.h>
#include <players/HumanPlayer.h>
#include <misc/SDL2pp.h>

#include <DataTypes.h>

#include <stdarg.h>
#include <string>
#include <map>
#include <utility>

// forward declarations
class ObjectBase;
class InGameMenu;
class MentatHelp;
class WaitingForOtherPlayers;
class ObjectManager;
class House;
class Explosion;


#define END_WAIT_TIME               (6*1000)

#define GAME_NOTHING            -1
#define GAME_RETURN_TO_MENU     0
#define GAME_NEXTMISSION        1
#define GAME_LOAD               2
#define GAME_DEBRIEFING_WIN     3
#define GAME_DEBRIEFING_LOST    4
#define GAME_CUSTOM_GAME_STATS  5


class Game
{
public:

    /**
        Default constructor. Call initGame() or initReplay() afterwards.
    */
    Game();


    Game(const Game& o) = delete;

    /**
        Destructor
    */
    ~Game();

    /**
        Initializes a game with the specified settings
        \param  newGameInitSettings the game init settings to initialize the game
    */
    void initGame(const GameInitSettings& newGameInitSettings);

    /**
        Initializes a replay from the specified filename
        \param  filename    the file containing the replay
    */
    void initReplay(const std::string& filename);



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
        This method proccesses all the user input.
    */
    void doInput();

    /**
        Returns the current game cycle number.
        \return the current game cycle
    */
    Uint32 getGameCycleCount() const { return gameCycleCount; };

    /**
        Return the game time in milliseconds.
        \return the current game time in milliseconds
    */
    Uint32 getGameTime() const { return gameCycleCount * GAMESPEED_DEFAULT; };

    /**
        Get the command manager of this game
        \return the command manager
    */
    CommandManager& getCommandManager() { return cmdManager; };

    /**
        Get the trigger manager of this game
        \return the trigger manager
    */
    TriggerManager& getTriggerManager() { return triggerManager; };

    /**
        Get the explosion list.
        \return the explosion list
    */
    RobustList<Explosion*>& getExplosionList() { return explosionList; };

    /**
        Returns the house with the id houseID
        \param  houseID the id of the house to return
        \return the house with id houseID
    */
    House* getHouse(int houseID) {
        return house[houseID].get();
    }

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
    void drawCursor() const;

    /**
        This method sets up the view. The start position is the center point of all owned units/structures
    */
    void setupView();

    /**
        This method loads a previously saved game.
        \param filename the name of the file to load from
        \return true on success, false on failure
    */
    bool loadSaveGame(const std::string& filename);

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
    bool saveGame(const std::string& filename);

    /**
        This method starts the game. Will return when the game is finished or aborted.
    */
    void runMainLoop();

    inline void quitGame() { bQuitGame = true;};

    /**
        This method pauses the current game.
    */
    void pauseGame() {
        if(gameType != GameType::CustomMultiplayer) {
            bPause = true;
        }
    }

    /**
        This method resumes the current paused game.
    */
    void resumeGame();

    /**
        This method writes out an object to a stream.
        \param stream   the stream to write to
        \param obj      the object to be saved
    */
    void saveObject(OutputStream& stream, ObjectBase* obj) const;

    /**
        This method loads an object from the stream.
        \param stream   the stream to read from
        \param ObjectID the object id that this unit/structure should get
        \return the read unit/structure
    */
    ObjectBase* loadObject(InputStream& stream, Uint32 objectID);

    inline ObjectManager& getObjectManager() { return objectManager; };
    inline GameInterface& getGameInterface() { return *pInterface; };

    const GameInitSettings& getGameInitSettings() const { return gameInitSettings; };
    void setNextGameInitSettings(const GameInitSettings& nextGameInitSettings) { this->nextGameInitSettings = nextGameInitSettings; };

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
    void selectAll(const std::set<Uint32>& aList);

    /**
        This method unselects all units/structures in the list aList.
        \param aList the list containing all the units/structures to be unselected
    */
    void unselectAll(const std::set<Uint32>& aList);

    /**
        Returns a list of all currently selected objects.
        \return list of currently selected units/structures
    */
    std::set<Uint32>& getSelectedList() { return selectedList; };

    /**
        Marks that the selection changed (and must be retransmitted to other players in multiplayer games)
    */
    inline void selectionChanged() {
        bSelectionChanged = true;
        if(pInterface) {
            pInterface->updateObjectInterface();
        }
        pLocalPlayer->onSelectionChanged(selectedList);
    };


    void onReceiveSelectionList(const std::string& name, const std::set<Uint32>& newSelectionList, int groupListIndex);

    /**
        Returns a list of all currently by  the other player selected objects (Only in multiplayer with multiple players per house).
        \return list of currently selected units/structures by the other player
    */
    std::set<Uint32>& getSelectedByOtherPlayerList() { return selectedByOtherPlayerList; };

    /**
        Called when a peer disconnects the game.
    */
    void onPeerDisconnected(const std::string& name, bool bHost, int cause);

    /**
        Adds a new message to the news ticker.
        \param  text    the text to add
    */
    void addToNewsTicker(const std::string& text) {
        if(pInterface != nullptr) {
            pInterface->addToNewsTicker(text);
        }
    }

    /**
        Adds an urgent message to the news ticker.
        \param  text    the text to add
    */
    void addUrgentMessageToNewsTicker(const std::string& text) {
        if(pInterface != nullptr) {
            pInterface->addUrgentMessageToNewsTicker(text);
        }
    }

    /**
        This method returns wether the game is currently paused
        \return true, if paused, false otherwise
    */
    bool isGamePaused() const { return bPause; };

    /**
        This method returns wether the game is finished
        \return true, if paused, false otherwise
    */
    bool isGameFinished() const { return finished; };

    /**
        Are cheats enabled?
        \return true = cheats enabled, false = cheats disabled
    */
    bool areCheatsEnabled() const { return bCheatsEnabled; };

    /**
        Returns the name of the local player; this method should be used instead of using settings.general.playerName directly
        \return the local player name
    */
    const std::string& getLocalPlayerName() const { return localPlayerName; }

    /**
        Register a new player in this game.
        \param  player      the player to register
    */
    void registerPlayer(Player* player) {
        playerName2Player.insert( std::make_pair(player->getPlayername(), player) );
        playerID2Player.insert( std::make_pair(player->getPlayerID(), player) );
    }

    /**
        Unregisters the specified player
        \param  player
    */
    void unregisterPlayer(Player* player) {
        playerID2Player.erase(player->getPlayerID());

        for(auto iter = playerName2Player.begin(); iter != playerName2Player.end(); ++iter) {
                if(iter->second == player) {
                    playerName2Player.erase(iter);
                    break;
                }
        }
    }

    /**
        Returns the first player with the given name.
        \param  playername  the name of the player
        \return the player or nullptr if none was found
    */
    Player* getPlayerByName(const std::string& playername) const {
        auto iter = playerName2Player.find(playername);
        if(iter != playerName2Player.end()) {
            return iter->second;
        } else {
            return nullptr;
        }
    }

    /**
        Returns the player with the given id.
        \param  playerID  the name of the player
        \return the player or nullptr if none was found
    */
    Player* getPlayerByID(Uint8 playerID) const {
        auto iter = playerID2Player.find(playerID);
        if(iter != playerID2Player.end()) {
            return iter->second;
        } else {
            return nullptr;
        }
    }

    /**
        This function is called when the user left clicks on the radar
        \param  worldPosition       position in world coordinates
        \param  bRightMouseButton   true = right mouse button, false = left mouse button
        \param  bDrag               true = the mouse was moved while being pressed, e.g. dragging
        \return true if dragging should start or continue
    */
    bool onRadarClick(Coord worldPosition, bool bRightMouseButton, bool bDrag);

    /**
        Take a screenshot and save it with a unique name
    */
    void takeScreenshot() const;

private:

    /**
        Checks whether the cursor is on the radar view
        \param  mouseX  x-coordinate of cursor
        \param  mouseY  y-coordinate of cursor
        \return true if on radar view
    */
    bool isOnRadarView(int mouseX, int mouseY) const;

    /**
        Handles the press of one key while chatting
        \param  key the key pressed
    */
    void handleChatInput(SDL_KeyboardEvent& keyboardEvent);

    /**
        Handles the press of one key
        \param  key the key pressed
    */
    void handleKeyInput(SDL_KeyboardEvent& keyboardEvent);

    /**
        Performs a building placement
        \param  xPos    x-coordinate in map coordinates
        \param  yPos    x-coordinate in map coordinates
        \return true if placement was successful
    */
    bool handlePlacementClick(int xPos, int yPos);

    /**
        Performs a attack click for the currently selected units/structures.
        \param  xPos    x-coordinate in map coordinates
        \param  yPos    x-coordinate in map coordinates
        \return true if attack is possible
    */
    bool handleSelectedObjectsAttackClick(int xPos, int yPos);

    /**
        Performs a move click for the currently selected units/structures.
        \param  xPos    x-coordinate in map coordinates
        \param  yPos    x-coordinate in map coordinates
        \return true if move is possible
    */
    bool handleSelectedObjectsMoveClick(int xPos, int yPos);

    /**
        Performs a capture click for the currently selected units/structures.
        \param  xPos    x-coordinate in map coordinates
        \param  yPos    x-coordinate in map coordinates
        \return true if capture is possible
    */
    bool handleSelectedObjectsCaptureClick(int xPos, int yPos);


    /**
        Performs a request carryall click for the currently selected units.
        \param  xPos    x-coordinate in map coordinates
        \param  yPos    x-coordinate in map coordinates
        \return true if carryall drop is possible
    */
    bool handleSelectedObjectsRequestCarryallDropClick(int xPos, int yPos);


    /**
        Performs an action click for the currently selected units/structures.
        \param  xPos    x-coordinate in map coordinates
        \param  yPos    x-coordinate in map coordinates
        \return true if action click is possible
    */
    bool handleSelectedObjectsActionClick(int xPos, int yPos);


    /**
        Selects the next structure of any of the types specified in itemIDs. If none of this type is currently selected the first one is selected.
        \param  itemIDs  the ids of the structures to select
    */
    void selectNextStructureOfType(const std::set<Uint32>& itemIDs);

    /**
        Returns the game speed of this game: The number of ms per game cycle.
        For singleplayer games this is a global setting (but can be adjusted in the in-game settings menu). For multiplayer games the game speed
        can be set by the person creating the game.
        \return the current game speed
    */
    int getGameSpeed() const;

public:
    enum {
        CursorMode_Normal,
        CursorMode_Attack,
        CursorMode_Move,
        CursorMode_Capture,
        CursorMode_CarryallDrop,
        CursorMode_Placing
    };

    int         currentCursorMode = CursorMode_Normal;

    GameType    gameType = GameType::Campaign;
    int         techLevel = 0;
    int         winFlags = 0;
    int         loseFlags = 0;

    Random      randomGen;          ///< This is the random number generator for this game
    ObjectData  objectData;         ///< This contains all the unit/structure data

    GameState   gameState = GameState::Start;

private:
    bool        chatMode = false;   ///< chat mode on?
    std::string typingChatMessage;  ///< currently typed chat message

    bool        scrollDownMode = false;     ///< currently scrolling the map down?
    bool        scrollLeftMode = false;     ///< currently scrolling the map left?
    bool        scrollRightMode = false;    ///< currently scrolling the map right?
    bool        scrollUpMode = false;       ///< currently scrolling the map up?

    bool        selectionMode = false;          ///< currently selection multiple units with a selection rectangle?
    SDL_Rect    selectionRect = {0, 0, 0, 0};   ///< the drawn rectangle while selection multiple units

    int         whatNextParam = GAME_NOTHING;

    Uint32      indicatorFrame = NONE_ID;
    int         indicatorTime = 5;
    int         indicatorTimer = 0;
    Coord       indicatorPosition = Coord::Invalid();

    float       averageFrameTime = 31.25f;      ///< The weighted average of the frame time of all previous frames (smoothed fps = 1000.0f/averageFrameTime)

    Uint32      gameCycleCount = 0;

    Uint32      skipToGameCycle = 0;            ///< skip to this game cycle

    bool        takePeriodicalScreenshots = false;      ///< take a screenshot every 10 seconds

    SDL_Rect    powerIndicatorPos = {14, 146, 4, 0};    ///< position of the power indicator in the right game bar
    SDL_Rect    spiceIndicatorPos = {20, 146, 4, 0};    ///< position of the spice indicator in the right game bar
    SDL_Rect    topBarPos = {0, 0, 0, 0};               ///< position of the top game bar
    SDL_Rect    sideBarPos = {0, 0, 0, 0};              ///< position of the right side bar

    ////////////////////

    GameInitSettings                    gameInitSettings;       ///< the init settings this game was started with
    GameInitSettings                    nextGameInitSettings;   ///< the init settings the next game shall be started with (restarting the mission, loading a savegame)
    GameInitSettings::HouseInfoList     houseInfoListSetup;     ///< this saves with which houses and players the game was actually set up. It is a copy of gameInitSettings::houseInfoList but without random houses


    ObjectManager       objectManager;          ///< This manages all the object and maps object ids to the actual objects

    CommandManager      cmdManager;             ///< This is the manager for all the game commands (e.g. moving a unit)

    TriggerManager      triggerManager;         ///< This is the manager for all the triggers the scenario has (e.g. reinforcements)

    bool    bQuitGame = false;                  ///< Should the game be quited after this game tick
    bool    bPause = false;                     ///< Is the game currently halted
    bool    bMenu = false;                      ///< Is there currently a menu shown (options or mentat menu)
    bool    bReplay = false;                    ///< Is this game actually a replay

    bool    bShowFPS = false;                   ///< Show the FPS

    bool    bShowTime = false;                  ///< Show how long this game is running

    bool    bCheatsEnabled = false;             ///< Cheat codes are enabled?

    bool    finished = false;                   ///< Is the game finished (won or lost) and we are just waiting for the end message to be shown
    bool    won = false;                        ///< If the game is finished, is it won or lost
    Uint32  finishedLevelTime = 0;              ///< The time in milliseconds when the level was finished (won or lost)
    bool    finishedLevel = false;              ///< Set, when the game is really finished and the end message was shown

    std::unique_ptr<GameInterface>          pInterface;                             ///< This is the whole interface (top bar and side bar)
    std::unique_ptr<InGameMenu>             pInGameMenu;                            ///< This is the menu that is opened by the option button
    std::unique_ptr<MentatHelp>             pInGameMentat;                          ///< This is the mentat dialog opened by the mentat button
    std::unique_ptr<WaitingForOtherPlayers> pWaitingForOtherPlayers;                ///< This is the dialog that pops up when we are waiting for other players during network hangs
    Uint32                                  startWaitingForOtherPlayersTime = 0;    ///< The time in milliseconds when we started waiting for other players

    bool    bSelectionChanged = false;                  ///< Has the selected list changed (and must be retransmitted to other plays in multiplayer games)
    std::set<Uint32> selectedList;                      ///< A set of all selected units/structures
    std::set<Uint32> selectedByOtherPlayerList;         ///< This is only used in multiplayer games where two players control one house
    RobustList<Explosion*> explosionList;               ///< A list containing all the explosions that must be drawn

    std::string localPlayerName;                            ///< the name of the local player
    std::multimap<std::string, Player*> playerName2Player;  ///< mapping player names to players (one entry per player)
    std::map<Uint8, Player*> playerID2Player;               ///< mapping player ids to players (one entry per player)

    std::array<std::unique_ptr<House>, NUM_HOUSES> house;   ///< All the houses of this game, index by their houseID; has the size NUM_HOUSES; unused houses are nullptr
};

#endif // GAME_H
