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

#ifndef METASERVERCLIENT_H
#define METASERVERCLIENT_H

#include "misc/dune_clock.h"
#include <Network/GameServerInfo.h>
#include <Network/MetaServerCommands.h>
#include <misc/SDL2pp.h>

#include <functional>
#include <memory>

#include <list>
#include <string>

inline constexpr auto SERVERLIST_UPDATE_INTERVAL = dune::as_dune_clock_duration(8 * 1000);
inline constexpr auto GAMESERVER_UPDATE_INTERVAL = dune::as_dune_clock_duration(10 * 1000);

class MetaServerClient {
public:
    explicit MetaServerClient(std::string metaServerURL);
    ~MetaServerClient();

    MetaServerClient(const MetaServerClient&) = delete;
    MetaServerClient(MetaServerClient&&)      = delete;

    MetaServerClient& operator=(const MetaServerClient&) = delete;
    MetaServerClient& operator=(MetaServerClient&&)      = delete;

    /**
        Sets the function that shall be called if there is an update to the server list
        \param  pOnGameServerInfoList   Function to call on an update to the server list
    */
    void setOnGameServerInfoList(std::function<void(std::list<GameServerInfo>&)> pOnGameServerInfoList) {
        this->pOnGameServerInfoList = pOnGameServerInfoList;
        lastServerInfoListUpdate    = dune::dune_clock::time_point{};
    }

    /**
        Sets the function that shall be called if the metaserver reports an error
        \param  pOnMetaServerError  Function to call on metaserver error
    */
    void setOnMetaServerError(std::function<void(int, const std::string&)> pOnMetaServerError) {
        this->pOnMetaServerError = pOnMetaServerError;
    }

    void startAnnounce(const std::string& serverName, int serverPort, const std::string& mapName, uint8_t numPlayers,
                       uint8_t maxPlayers);

    void updateAnnounce(uint8_t numPlayers);

    void stopAnnounce();

    void update();

    const std::string metaServerURL; ///< The url of the meta server which is used for receiving the list of game
                                     ///< servers and publish own games to

private:
    /**
        The main function of the thread that performes the complete communication with the metaserver.
        \param  data    this void pointer should point to an instance of this MetaServerClient class
        \return returns 0
    */
    static int connectionThreadMain(void* data);

    /**
        Enqueues a new command in the command queue for processing by the metaserver connection thread.
        \param  metaServerCommand   a shared pointer to a command
    */
    void enqueueMetaServerCommand(std::unique_ptr<MetaServerCommand> metaServerCommand);

    /**
        Dequeues a command from the command queue. This method shall only be called from the metaserver connection
       thread. \return a shared pointer to the first command in the queue
    */
    std::unique_ptr<MetaServerCommand> dequeueMetaServerCommand();

    /**
        Sets a metaserver error message. This method shall only be called from the metaserver connection thread.
        \param  errorCause      the id of the command sent to the metaserver
        \param  errorMessage    the error message to post
    */
    void setErrorMessage(int errorCause, const std::string& errorMessage);

    /**
        Sets a new game server info list. This method shall only be called from the metaserver connection thread.
        \param  newGameServerInfoList   the new game server list
    */
    void setNewGameServerInfoList(const std::list<GameServerInfo>& newGameServerInfoList);

    // Shared data (used by main thread and connection thread):

    std::list<std::unique_ptr<MetaServerCommand>>
        metaServerCommandList; ///< The command queue for the metaserver connection thread (shared between main thread
                               ///< and metaserver connection thread, \see sharedDataMutex)
    SDL_sem* availableMetaServerCommandsSemaphore; ///< This semaphore counts how many commands are available in the
                                                   ///< metaServerCommandList

    int metaserverErrorCause = 0; ///< Set to 0 in case of no error, else the id of the command sent to the metaserver
    std::string metaserverError;  ///< Set to some string in case a metaserver error occurs (only one error can be
                                  ///< pending at once)
    bool bUpdatedGameServerInfoList =
        false; ///< Was the gameServerInfoList updated? Set to true by the metaserver connection thread and reset to
               ///< false in the main thread (\see sharedDataMutex)
    std::list<GameServerInfo>
        gameServerInfoList; ///< A list of all available game servers. Written by the metaserver connection thread and
                            ///< read by the main thread (\see sharedDataMutex)

    SDL_mutex* sharedDataMutex; ///< This mutex must be locked before any shared data structures between the main thread
                                ///< and the metaserver connection thread is read or modified)

    SDL_Thread* connectionThread; ///< The metaserver connection thread that processes all the metaserver communication

    // Non-Shared data (used only by main thread):

    std::string serverName; ///< The name of the game server
    int serverPort = 0;     ///< The port of the game server
    std::string secret;     ///< The secret used for the metaserver updates
    std::string mapName;    ///< The name of the map for which a game is currently set up
    uint8_t numPlayers = 0; ///< The current number of players in the currently set up game
    uint8_t maxPlayers = 0; ///< The maximum number of players in the currently set up game

    dune::dune_clock::time_point lastAnnounceUpdate{}; ///< The last time the game was announced
    dune::dune_clock::time_point
        lastServerInfoListUpdate{}; ///< The last time the server list was updated by a request to the metaserver

    std::function<void(std::list<GameServerInfo>&)>
        pOnGameServerInfoList;                                ///< Callback for updates to the game server list
    std::function<void(int, std::string)> pOnMetaServerError; ///< Callback for metaserver errors
};

#endif // METASERVERCLIENT_H
