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

#include <Network/MetaServerCommands.h>
#include <Network/GameServerInfo.h>

#include <memory>
#include <functional>

#include <condition_variable>
#include <list>
#include <mutex>
#include <thread>
#include <string>

inline constexpr int SERVERLIST_UPDATE_INTERVAL = 8 * 1000;
inline constexpr int GAMESERVER_UPDATE_INTERVAL = 10 * 1000;


class MetaServerClient final {
public:

    explicit MetaServerClient(std::string_view metaServerURL);
    ~MetaServerClient();

    MetaServerClient(const MetaServerClient &) = delete;
    MetaServerClient(MetaServerClient &&) = delete;

    MetaServerClient& operator=(const MetaServerClient &) = delete;
    MetaServerClient& operator=(MetaServerClient &&) = delete;

    /**
        Sets the function that shall be called if there is an update to the server list
        \param  pOnGameServerInfoList   Function to call on an update to the server list
    */
    void setOnGameServerInfoList(std::function<void (std::list<GameServerInfo>&)> pOnGameServerInfoList) {
        this->pOnGameServerInfoList = pOnGameServerInfoList;
        lastServerInfoListUpdate = 0;
    }

    /**
        Sets the function that shall be called if the metaserver reports an error
        \param  pOnMetaServerError  Function to call on metaserver error
    */
    void setOnMetaServerError(std::function<void(int, std::string_view)> pOnMetaServerError) {
        this->pOnMetaServerError = pOnMetaServerError;
    }

    void startAnnounce(std::string_view serverName, int serverPort, std::string_view mapName, Uint8 numPlayers,
                       Uint8 maxPlayers);

    void updateAnnounce(Uint8 numPlayers);

    void stopAnnounce();

    void update();


    const std::string metaServerURL;    ///< The url of the meta server which is used for receiving the list of game servers and publish own games to

private:

    /**
        The main function of the thread that performs the complete communication with the metaserver.
        \return returns 0
    */
    int connectionThreadMain();


    /**
        Enqueues a new command in the command queue for processing by the metaserver connection thread.
        \param  metaServerCommand   a shared pointer to a command
    */
    void enqueueMetaServerCommand(std::unique_ptr<MetaServerCommand> metaServerCommand);

    /**
        Dequeues a command from the command queue. This method shall only be called from the metaserver connection thread.
        \return a shared pointer to the first command in the queue
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

    std::list<std::unique_ptr<MetaServerCommand> > metaServerCommandList;       ///< The command queue for the metaserver connection thread (shared between main thread and metaserver connection thread, \see sharedDataMutex)
    std::condition_variable metaServerCommandListCv;                            ///< This semaphore counts how many commands are available in the metaServerCommandList

    int metaserverErrorCause = 0;                                               ///< Set to 0 in case of no error, else the id of the command sent to the metaserver
    std::string metaserverError;                                                ///< Set to some string in case a metaserver error occurs (only one error can be pending at once)
    bool bUpdatedGameServerInfoList = false;                                    ///< Was the gameServerInfoList updated? Set to true by the metaserver connection thread and reset to false in the main thread (\see sharedDataMutex)
    std::list<GameServerInfo> gameServerInfoList;                               ///< A list of all available game servers. Written by the metaserver connection thread and read by the main thread (\see sharedDataMutex)

    std::mutex sharedDataMutex;                                                 ///< This mutex must be locked before any shared data structures between the main thread and the metaserver connection thread is read or modified)

    std::thread connectionThread;                                               ///< The metaserver connection thread that processes all the metaserver communication


    // Non-Shared data (used only by main thread):

    std::string serverName;                                                     ///< The name of the game server
    int serverPort = 0;                                                         ///< The port of the game server
    std::string secret;                                                         ///< The secret used for the metaserver updates
    std::string mapName;                                                        ///< The name of the map for which a game is currently set up
    Uint8 numPlayers = 0;                                                       ///< The current number of players in the currently set up game
    Uint8 maxPlayers = 0;                                                       ///< The maximum number of players in the currently set up game

    Uint32 lastAnnounceUpdate = 0;                                              ///< The last time the game was announced
    Uint32 lastServerInfoListUpdate = 0;                                        ///< The last time the server list was updated by a request to the metaserver

    std::function<void (std::list<GameServerInfo>&)> pOnGameServerInfoList;     ///< Callback for updates to the game server list
    std::function<void (int, std::string)> pOnMetaServerError;                  ///< Callback for metaserver errors
};

#endif // METASERVERCLIENT_H

