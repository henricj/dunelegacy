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

#ifndef GAMESERVERINFO_H
#define GAMESERVERINFO_H

#include <misc/SDL2pp.h>
#include <enet/enet.h>
#include <string>

class GameServerInfo {
public:
    ENetAddress serverAddress;
    std::string serverName;
    std::string serverVersion;
    std::string mapName;
    int numPlayers;
    int maxPlayers;
    bool bPasswordProtected;
    Uint32 lastUpdate;

    /**
        Do not compare numPlayers, bPasswordProtected and lastUpdate
        \param gameServerInfo   the other object to compare with
        \return true if equal, false otherwise
    */
    bool operator==(const GameServerInfo& gameServerInfo) const {
        return ((serverAddress.host == gameServerInfo.serverAddress.host)
                 && (serverAddress.port == gameServerInfo.serverAddress.port)
                 && (serverName == gameServerInfo.serverName)
                 && (mapName == gameServerInfo.mapName)
                 && (maxPlayers == gameServerInfo.maxPlayers));
    }
};

#endif // GAMESERVERINFO_H
