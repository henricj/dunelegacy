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

#include <DataTypes.h>

#include <SDL.h>
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


#define END_WAIT_TIME				(6*1000)

#define GAME_NOTHING			-1
#define	GAME_RETURN_TO_MENU		0
#define GAME_NEXTMISSION		1
#define	GAME_LOAD				2
#define GAME_DEBRIEFING_WIN		3
#define	GAME_DEBRIEFING_LOST	4
#define GAME_CUSTOM_GAME_STATS  5


class Game
{
public:

    /**
        Default constructor. Call initGame() or initReplay() afterwards.
    */
	Game();

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
        return house[houseID];
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
	void drawCursor();

    /**
        This method sets up the view. The start position is the center point of all owned units/structures
    */
	void setupView();

    /**
        This method loads a previously saved game.
        \param filename the name of the file to load from
        \return true on success, false on failure
    */
	bool loadSaveGame(std::string filename);

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
	bool saveGame(std::string filename);

    /**
        This method starts the game. Will return when the game is finished or aborted.
    */
	void runMainLoop();

	inline void quitGame() { bQuitGame = true;};

    /**
        This method pauses the current game.
    */
	void pauseGame() {
        if(gameType != GAMETYPE_CUSTOM_MULTIPLAYER) {
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
	void saveObject(OutputStream& stream, ObjectBase* obj);

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
        GAME_LOAD			 - from inside the game the user requests to load a savegame and you should do this now<br>
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
	void selectAll(std::set<Uint32>& aList);

    /**
        This method unselects all units/structures in the list aList.
        \param aList the list containing all the units/structures to be unselected
    */
	void unselectAll(std::set<Uint32>& aList);

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
    };


	void onReceiveSelectionList(std::string name, std::set<Uint32> newSelectionList, int groupListIndex);

    /**
        Returns a list of all currently by  the other player selected objects (Only in multiplayer with multiple players per house).
        \return list of currently selected units/structures by the other player
    */
	std::set<Uint32>& getSelectedByOtherPlayerList() { return selectedByOtherPlayerList; };

    /**
		Called when a peer disconnects the game.
	*/
    void onPeerDisconnected(std::string name, bool bHost, int cause);

    /**
        Adds a new message to the news ticker.
        \param  text    the text to add
    */
	void addToNewsTicker(const std::string& text) {
		if(pInterface != NULL) {
			pInterface->addToNewsTicker(text);
		}
	}

    /**
        Adds an urgent message to the news ticker.
        \param  text    the text to add
    */
	void addUrgentMessageToNewsTicker(const std::string& text) {
		if(pInterface != NULL) {
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

        std::multimap<std::string, Player*>::iterator iter;
        for(iter = playerName2Player.begin(); iter != playerName2Player.end(); ++iter) {
                if(iter->second == player) {
                    playerName2Player.erase(iter);
                    break;
                }
        }
    }

    /**
        Returns the first player with the given name.
        \param  playername  the name of the player
        \return the player or NULL if none was found
    */
	Player* getPlayerByName(const std::string& playername) const {
        std::multimap<std::string, Player*>::const_iterator iter = playerName2Player.find(playername);
        if(iter != playerName2Player.end()) {
            return iter->second;
        } else {
            return NULL;
        }
	}

    /**
        Returns the player with the given id.
        \param  playerID  the name of the player
        \return the player or NULL if none was found
    */
	Player* getPlayerByID(Uint8 playerID) const {
        std::map<Uint8, Player*>::const_iterator iter = playerID2Player.find(playerID);
        if(iter != playerID2Player.end()) {
            return iter->second;
        } else {
            return NULL;
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

private:

    /**
        Checks whether the cursor is on the radar view
        \param  mouseX  x-coordinate of cursor
        \param  mouseY  y-coordinate of cursor
        \return true if on radar view
    */
    bool isOnRadarView(int mouseX, int mouseY);

    /**
        Handles the press of one key for chatting
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

public:
    enum {
        CursorMode_Normal,
        CursorMode_Attack,
        CursorMode_Move,
        CursorMode_Capture,
        CursorMode_CarryallDrop,
        CursorMode_Placing
    };

    int         currentCursorMode;

	GAMETYPE	gameType;
	int			techLevel;
	int			winFlags;
	int         loseFlags;

	int		    gamespeed;

	Random      randomGen;          ///< This is the random number generator for this game
	ObjectData  objectData;         ///< This contains all the unit/structure data

	GAMESTATETYPE gameState;

private:
    bool        chatMode;           ///< chat mode on?
    std::string typingChatMessage;  ///< currently typed chat message

	bool        scrollDownMode;     ///< currently scrolling the map down?
	bool        scrollLeftMode;     ///< currently scrolling the map left?
	bool        scrollRightMode;    ///< currently scrolling the map right?
	bool        scrollUpMode;       ///< currently scrolling the map up?

	bool	    selectionMode;      ///< currently selection multiple units with a selection rectangle?
	SDL_Rect    selectionRect;      ///< the drawn rectangle while selection multiple units

	int		    whatNextParam;

	Uint32      indicatorFrame;
	int         indicatorTime;
	int         indicatorTimer;
	Coord       indicatorPosition;

	float       averageFrameTime;   ///< The weighted average of the frame time of all previous frames (smoothed fps = 1000.0f/averageFrameTime)

	Uint32      gameCycleCount;

	Uint32      skipToGameCycle;    ///< skip to this game cycle

	SDL_Rect	powerIndicatorPos;  ///< position of the power indicator in the right game bar
	SDL_Rect	spiceIndicatorPos;  ///< position of the spice indicator in the right game bar
	SDL_Rect	topBarPos;          ///< position of the top game bar
    SDL_Rect    sideBarPos;         ///< position of the right side bar

	////////////////////

	GameInitSettings	                gameInitSettings;       ///< the init settings this game was started with
	GameInitSettings	                nextGameInitSettings;   ///< the init settings the next game shall be started with (restarting the mission, loading a savegame)
    GameInitSettings::HouseInfoList     houseInfoListSetup;     ///< this saves with which houses and players the game was actually set up. It is a copy of gameInitSettings::houseInfoList but without random houses


	ObjectManager       objectManager;          ///< This manages all the object and maps object ids to the actual objects

	CommandManager      cmdManager;			    ///< This is the manager for all the game commands (e.g. moving a unit)

	TriggerManager      triggerManager;         ///< This is the manager for all the triggers the scenario has (e.g. reinforcements)

	bool	bQuitGame;					///< Should the game be quited after this game tick
	bool	bPause;						///< Is the game currently halted
	bool    bMenu;                      ///< Is there currently a menu shown (options or mentat menu)
	bool	bReplay;					///< Is this game actually a replay

	bool	bShowFPS;					///< Show the FPS

	bool    bShowTime;                  ///< Show how long this game is running

	bool    bCheatsEnabled;             ///< Cheat codes are enabled?

    bool	finished;                   ///< Is the game finished (won or lost) and we are just waiting for the end message to be shown
	bool	won;                        ///< If the game is finished, is it won or lost
	Uint32  finishedLevelTime;          ///< The time in milliseconds when the level was finished (won or lost)
	bool    finishedLevel;              ///< Set, when the game is really finished and the end message was shown

	GameInterface*	        pInterface;			                ///< This is the whole interface (top bar and side bar)
	InGameMenu*		        pInGameMenu;		                ///< This is the menu that is opened by the option button
	MentatHelp*		        pInGameMentat;		                ///< This is the mentat dialog opened by the mentat button
	WaitingForOtherPlayers* pWaitingForOtherPlayers;            ///< This is the dialog that pops up when we are waiting for other players during network hangs
	Uint32                  startWaitingForOtherPlayersTime;    ///< The time in milliseconds when we started waiting for other players

	bool    bSelectionChanged;                          ///< Has the selected list changed (and must be retransmitted to other plays in multiplayer games)
	std::set<Uint32> selectedList;                      ///< A set of all selected units/structures
	std::set<Uint32> selectedByOtherPlayerList;         ///< This is only used in multiplayer games where two players control one house
    RobustList<Explosion*> explosionList;               ///< A list containing all the explosions that must be drawn

    std::vector<House*> house;                          ///< All the houses of this game, index by their houseID; has the size NUM_HOUSES; unused houses are NULL

    std::multimap<std::string, Player*> playerName2Player;  ///< mapping player names to players (one entry per player)
    std::map<Uint8, Player*> playerID2Player;               ///< mapping player ids to players (one entry per player)
};

#endif // GAME_H
