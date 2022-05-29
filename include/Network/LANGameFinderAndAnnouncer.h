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

#include "misc/dune_clock.h"
#include <Network/GameServerInfo.h>

#include <enet/enet.h>

#include <functional>
#include <list>
#include <string>

inline constexpr auto LANGAME_ANNOUNCER_PORT               = 28746;
inline constexpr auto LANGAME_ANNOUNCER_INTERVAL           = dune::as_dune_clock_duration(3000);
inline constexpr auto LANGAME_ANNOUNCER_MAGICNUMBER        = 82071105;
inline constexpr auto LANGAME_ANNOUNCER_MAXGAMENAMESIZE    = 32;
inline constexpr auto LANGAME_ANNOUNCER_MAXGAMEVERSIONSIZE = 32;
inline constexpr auto LANGAME_ANNOUNCER_MAXMAPNAMESIZE     = 64;

class LANGameFinderAndAnnouncer final {
public:
    LANGameFinderAndAnnouncer();
    ~LANGameFinderAndAnnouncer();

    LANGameFinderAndAnnouncer(const LANGameFinderAndAnnouncer&)            = delete;
    LANGameFinderAndAnnouncer(LANGameFinderAndAnnouncer&&)                 = delete;
    LANGameFinderAndAnnouncer& operator=(const LANGameFinderAndAnnouncer&) = delete;
    LANGameFinderAndAnnouncer& operator=(LANGameFinderAndAnnouncer&&)      = delete;

    void startAnnounce(std::string serverName, uint16_t serverPort, std::string mapName, uint8_t numPlayers,
                       uint8_t maxPlayers);

    void updateAnnounce(uint8_t numPlayers);

    void stopAnnounce();

    void update();

    void announceGame();

    void refreshServerList() const;

    [[nodiscard]] const std::list<GameServerInfo>& getServerInfoList() const { return gameServerInfoList_; }

    /**
        Sets the function that should be called when a new server is found
        \param  pOnNewServer    Function to call on new server found
    */
    void setOnNewServer(std::function<void(GameServerInfo)> pOnNewServer) { this->pOnNewServer_ = pOnNewServer; }

    /**
        Sets the function that should be called when a server is updated
        \param  pOnUpdateServer Function to call on server update
    */
    void setOnUpdateServer(std::function<void(GameServerInfo)> pOnUpdateServer) {
        this->pOnUpdateServer_ = pOnUpdateServer;
    }

    /**
        Sets the function that should be called when a server is removed from the list of available servers.
        \param  pOnRemoveServer function to call on server remove
    */
    void setOnRemoveServer(std::function<void(GameServerInfo)> pOnRemoveServer) {
        this->pOnRemoveServer_ = pOnRemoveServer;
    }

private:
    void receivePackets();
    void updateServerInfoList();
    void sendRemoveGameAnnouncement();

    std::string serverName_;
    uint16_t serverPort_ = 0;
    std::string mapName_;
    uint8_t numPlayers_ = 0;
    uint8_t maxPlayers_ = 0;

    dune::dune_clock::time_point lastAnnounce_{};
    ENetSocket announceSocket_;

    std::list<GameServerInfo> gameServerInfoList_;

    std::function<void(GameServerInfo)> pOnNewServer_;
    std::function<void(GameServerInfo)> pOnUpdateServer_;
    std::function<void(GameServerInfo)> pOnRemoveServer_;
};

#endif // LANGAMEFINDERANDANNOUNCER_H
