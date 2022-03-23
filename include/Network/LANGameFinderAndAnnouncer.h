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

#ifndef LANGAMEFINDERANDANNOUNCER_H
#define LANGAMEFINDERANDANNOUNCER_H

#include <Network/GameServerInfo.h>
#include <misc/SDL2pp.h>

#include <enet/enet.h>
#include <functional>
#include <list>
#include <string>

#define LANGAME_ANNOUNCER_PORT               28746
#define LANGAME_ANNOUNCER_INTERVAL           3000
#define LANGAME_ANNOUNCER_MAGICNUMBER        82071105
#define LANGAME_ANNOUNCER_MAXGAMENAMESIZE    32
#define LANGAME_ANNOUNCER_MAXGAMEVERSIONSIZE 32
#define LANGAME_ANNOUNCER_MAXMAPNAMESIZE     64

class LANGameFinderAndAnnouncer {
public:
    LANGameFinderAndAnnouncer();
    ~LANGameFinderAndAnnouncer();

    void startAnnounce(const std::string& serverName, int serverPort, const std::string& mapName, uint8_t numPlayers,
                       uint8_t maxPlayers) {
        this->serverName = serverName;
        this->serverPort = serverPort;
        this->mapName    = mapName;
        this->numPlayers = numPlayers;
        this->maxPlayers = maxPlayers;
        lastAnnounce     = 0;
    }

    void updateAnnounce(uint8_t numPlayers) {
        this->numPlayers = numPlayers;
        if (serverPort > 0) {
            announceGame();
        }
    }

    void stopAnnounce() {
        sendRemoveGameAnnouncement();
        serverName = "";
        serverPort = 0;
    }

    void update();

    void announceGame();

    void refreshServerList() const;

    [[nodiscard]] const std::list<GameServerInfo>& getServerInfoList() const { return gameServerInfoList; }

    /**
        Sets the function that should be called when a new server is found
        \param  pOnNewServer    Function to call on new server found
    */
    void setOnNewServer(std::function<void(GameServerInfo)> pOnNewServer) { this->pOnNewServer = pOnNewServer; }

    /**
        Sets the function that should be called when a server is updated
        \param  pOnUpdateServer Function to call on server update
    */
    void setOnUpdateServer(std::function<void(GameServerInfo)> pOnUpdateServer) {
        this->pOnUpdateServer = pOnUpdateServer;
    }

    /**
        Sets the function that should be called when a server is removed from the list of available servers.
        \param  pOnRemoveServer function to call on server remove
    */
    void setOnRemoveServer(std::function<void(GameServerInfo)> pOnRemoveServer) {
        this->pOnRemoveServer = pOnRemoveServer;
    }

private:
    void receivePackets();
    void updateServerInfoList();
    void sendRemoveGameAnnouncement();

    std::string serverName;
    int serverPort = 0;
    std::string mapName;
    uint8_t numPlayers = 0;
    uint8_t maxPlayers = 0;

    uint32_t lastAnnounce = 0;
    ENetSocket announceSocket;

    std::list<GameServerInfo> gameServerInfoList;

    std::function<void(GameServerInfo)> pOnNewServer;
    std::function<void(GameServerInfo)> pOnUpdateServer;
    std::function<void(GameServerInfo)> pOnRemoveServer;
};

#endif // LANGAMEFINDERANDANNOUNCER_H
