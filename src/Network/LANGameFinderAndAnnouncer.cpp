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

#include <Network/LANGameFinderAndAnnouncer.h>

#include <misc/exceptions.h>

#include <config.h>



#define NETWORKPACKET_ANNOUNCEGAME              1
#define NETWORKPACKET_REMOVEGAMEANNOUNCEMENT    2
#define NETWORKPACKET_REQUESTANNOUNCE           3

#ifdef _MSC_VER_
#pragma pack(push, 1)
#define PACKEDDATASTRUCTURE
#elif defined(__GNUC__)
#define PACKEDDATASTRUCTURE __attribute__ ((packed))
#else
#define PACKEDDATASTRUCTURE
#endif

struct PACKEDDATASTRUCTURE NetworkPacket_Unknown {
    enet_uint32 magicNumber;
    enet_uint8  type;
};

struct PACKEDDATASTRUCTURE NetworkPacket_AnnounceGame {
    enet_uint32 magicNumber;
    enet_uint8  type;
    enet_uint16 serverPort;
    char        serverName[LANGAME_ANNOUNCER_MAXGAMENAMESIZE];
    char        serverVersion[LANGAME_ANNOUNCER_MAXGAMEVERSIONSIZE];
    char        mapName[LANGAME_ANNOUNCER_MAXMAPNAMESIZE];
    char        numPlayers;
    char        maxPlayers;
};

struct PACKEDDATASTRUCTURE NetworkPacket_RemoveGameAnnouncement {
    enet_uint32 magicNumber;
    enet_uint8  type;
    enet_uint16 serverPort;
};

struct PACKEDDATASTRUCTURE NetworkPacket_RequestGame {
    enet_uint32 magicNumber;
    enet_uint8  type;
};

#ifdef _MSC_VER_
#pragma pack(pop)
#endif



LANGameFinderAndAnnouncer::LANGameFinderAndAnnouncer() {

    announceSocket = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
    if(announceSocket == ENET_SOCKET_NULL) {
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Creating socket failed!");
    }

    if(enet_socket_set_option(announceSocket, ENET_SOCKOPT_REUSEADDR, 1) < 0) {
        enet_socket_destroy(announceSocket);
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Setting socket option 'ENET_SOCKOPT_REUSEADDR' failed!");
    }

    if(enet_socket_set_option(announceSocket, ENET_SOCKOPT_NONBLOCK, 1) < 0) {
        enet_socket_destroy(announceSocket);
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Setting socket option 'ENET_SOCKOPT_NONBLOCK' failed!");
    }

    if(enet_socket_set_option(announceSocket, ENET_SOCKOPT_BROADCAST, 1) < 0) {
        enet_socket_destroy(announceSocket);
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Setting socket option 'ENET_SOCKOPT_BROADCAST' failed!");
    }

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = LANGAME_ANNOUNCER_PORT;

    if(enet_socket_bind(announceSocket, &address) < 0) {
        enet_socket_destroy(announceSocket);
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Binding socket to address failed!");
    }
}

LANGameFinderAndAnnouncer::~LANGameFinderAndAnnouncer() {
    if(serverPort > 0) {
        try {
            stopAnnounce();
        } catch (std::exception& e) {
            SDL_Log("LANGameFinderAndAnnouncer::~LANGameFinderAndAnnouncer(): %s", e.what());
        }
    }
    enet_socket_destroy(announceSocket);
}

void LANGameFinderAndAnnouncer::update() {
    if(serverPort > 0) {
        if(SDL_GetTicks() - lastAnnounce > LANGAME_ANNOUNCER_INTERVAL) {
            announceGame();
        }
    }

    updateServerInfoList();

    receivePackets();
}

void LANGameFinderAndAnnouncer::announceGame() {

    ENetAddress destinationAddress;
    destinationAddress.host = ENET_HOST_BROADCAST;
    destinationAddress.port = LANGAME_ANNOUNCER_PORT;

    NetworkPacket_AnnounceGame announcePacket;
    memset(&announcePacket, 0, sizeof(NetworkPacket_AnnounceGame));

    announcePacket.magicNumber = SDL_SwapLE32(LANGAME_ANNOUNCER_MAGICNUMBER);
    announcePacket.type = NETWORKPACKET_ANNOUNCEGAME;
    strncpy(announcePacket.serverName, serverName.c_str(), LANGAME_ANNOUNCER_MAXGAMENAMESIZE);
    strncpy(announcePacket.serverVersion, VERSIONSTRING, LANGAME_ANNOUNCER_MAXGAMEVERSIONSIZE);
    announcePacket.serverPort = SDL_SwapLE16(serverPort);
    strncpy(announcePacket.mapName, mapName.c_str(), LANGAME_ANNOUNCER_MAXMAPNAMESIZE);
    announcePacket.numPlayers = numPlayers;
    announcePacket.maxPlayers = maxPlayers;

    ENetBuffer enetBuffer;
    enetBuffer.data = &announcePacket;
    enetBuffer.dataLength = sizeof(NetworkPacket_AnnounceGame);
    int err = enet_socket_send(announceSocket, &destinationAddress, &enetBuffer, 1);
    if(err==0) {
        // blocked
    } else if(err < 0) {
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Announcing failed!");
    } else {
        lastAnnounce = SDL_GetTicks();
    }
}

void LANGameFinderAndAnnouncer::refreshServerList() const {
    ENetAddress destinationAddress;
    destinationAddress.host = ENET_HOST_BROADCAST;
    destinationAddress.port = LANGAME_ANNOUNCER_PORT;

    NetworkPacket_RequestGame requestPacket;
    memset(&requestPacket, 0, sizeof(NetworkPacket_RequestGame));

    requestPacket.magicNumber = SDL_SwapLE32(LANGAME_ANNOUNCER_MAGICNUMBER);
    requestPacket.type = NETWORKPACKET_REQUESTANNOUNCE;


    ENetBuffer enetBuffer;
    enetBuffer.data = &requestPacket;
    enetBuffer.dataLength = sizeof(NetworkPacket_RequestGame);
    int err = enet_socket_send(announceSocket, &destinationAddress, &enetBuffer, 1);
    if(err==0) {
        // blocked
    } else if(err < 0) {
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Refreshing server list failed!");
    }
}

void LANGameFinderAndAnnouncer::receivePackets() {
    NetworkPacket_AnnounceGame announcePacket;

    ENetAddress senderAddress;

    ENetBuffer enetBuffer;
    enetBuffer.data = &announcePacket;
    enetBuffer.dataLength = sizeof(announcePacket);
    int receivedBytes = enet_socket_receive(announceSocket, &senderAddress, &enetBuffer, 1);
    if(receivedBytes==0) {
        // blocked
    } else if(receivedBytes < 0) {
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Receiving data failed!");
    } else {
        if((receivedBytes == sizeof(NetworkPacket_AnnounceGame))
            && (SDL_SwapLE32(announcePacket.magicNumber) == LANGAME_ANNOUNCER_MAGICNUMBER)
            && (announcePacket.type == NETWORKPACKET_ANNOUNCEGAME)
            && (strncmp(announcePacket.serverVersion, VERSIONSTRING, LANGAME_ANNOUNCER_MAXGAMEVERSIONSIZE) == 0)) {

            std::string serverName = std::string(announcePacket.serverName, LANGAME_ANNOUNCER_MAXGAMENAMESIZE);
            serverName.resize(strlen(serverName.c_str()));  // strip off '\0' at the end of the std::string
            int serverPort = SDL_SwapLE16(announcePacket.serverPort);

            std::string mapName = std::string(announcePacket.mapName, LANGAME_ANNOUNCER_MAXMAPNAMESIZE);
            mapName.resize(strlen(mapName.c_str()));  // strip off '\0' at the end of the std::string

            int numPlayers = announcePacket.numPlayers;
            int maxPlayers = announcePacket.maxPlayers;

            GameServerInfo gameServerInfo;
            gameServerInfo.serverAddress.host = senderAddress.host;
            gameServerInfo.serverAddress.port = serverPort;
            gameServerInfo.serverName = serverName;
            gameServerInfo.mapName = mapName;
            gameServerInfo.numPlayers = numPlayers;
            gameServerInfo.maxPlayers = maxPlayers;
            gameServerInfo.bPasswordProtected = false;
            gameServerInfo.lastUpdate = SDL_GetTicks();

            bool bUpdate = false;
            for(GameServerInfo& curGameServerInfo : gameServerInfoList) {
                if((curGameServerInfo.serverAddress.host == gameServerInfo.serverAddress.host)
                    && (curGameServerInfo.serverAddress.port == gameServerInfo.serverAddress.port)) {
                    curGameServerInfo = gameServerInfo;
                    bUpdate = true;
                    break;
                }

            }

            if(bUpdate) {
                if(pOnUpdateServer) {
                    pOnUpdateServer(gameServerInfo);
                }
            } else {
                gameServerInfoList.push_back(gameServerInfo);
                if(pOnNewServer) {
                    pOnNewServer(gameServerInfo);
                }
            }
        } else if((receivedBytes == sizeof(NetworkPacket_RemoveGameAnnouncement))
            && (SDL_SwapLE32(announcePacket.magicNumber) == LANGAME_ANNOUNCER_MAGICNUMBER)
            && (announcePacket.type == NETWORKPACKET_REMOVEGAMEANNOUNCEMENT)) {

            int serverPort = SDL_SwapLE16(announcePacket.serverPort);

            std::list<GameServerInfo>::iterator iter = gameServerInfoList.begin();
            while(iter != gameServerInfoList.end()) {
                if((iter->serverAddress.host == senderAddress.host)
                    && (iter->serverAddress.port == serverPort)) {
                    GameServerInfo tmpGameServerInfo = *iter;
                    iter = gameServerInfoList.erase(iter);

                    if(pOnRemoveServer) {
                        pOnRemoveServer(tmpGameServerInfo);
                    }
                } else {
                    ++iter;
                }
            }
        } else if((serverPort > 0)
            && (receivedBytes == sizeof(NetworkPacket_RequestGame))
            && (SDL_SwapLE32(announcePacket.magicNumber) == LANGAME_ANNOUNCER_MAGICNUMBER)
            && (announcePacket.type == NETWORKPACKET_REQUESTANNOUNCE)) {

            announceGame();
        }

    }
}

void LANGameFinderAndAnnouncer::updateServerInfoList() {
    Uint32 currentTime = SDL_GetTicks();

    std::list<GameServerInfo>::iterator iter = gameServerInfoList.begin();
    while(iter != gameServerInfoList.end()) {
        if(iter->lastUpdate + 3 * LANGAME_ANNOUNCER_INTERVAL < currentTime) {
            if(pOnRemoveServer) {
                pOnRemoveServer(*iter);
            }
            iter = gameServerInfoList.erase(iter);
        } else {
            ++iter;
        }
    }

}

void LANGameFinderAndAnnouncer::sendRemoveGameAnnouncement() {

    ENetAddress destinationAddress;
    destinationAddress.host = ENET_HOST_BROADCAST;
    destinationAddress.port = LANGAME_ANNOUNCER_PORT;

    NetworkPacket_RemoveGameAnnouncement removeAnnouncementPacket;
    memset(&removeAnnouncementPacket, 0, sizeof(NetworkPacket_RemoveGameAnnouncement));

    removeAnnouncementPacket.magicNumber = SDL_SwapLE32(LANGAME_ANNOUNCER_MAGICNUMBER);
    removeAnnouncementPacket.type = NETWORKPACKET_REMOVEGAMEANNOUNCEMENT;
    removeAnnouncementPacket.serverPort = SDL_SwapLE16(serverPort);

    ENetBuffer enetBuffer;
    enetBuffer.data = &removeAnnouncementPacket;
    enetBuffer.dataLength = sizeof(NetworkPacket_RemoveGameAnnouncement);
    int err = enet_socket_send(announceSocket, &destinationAddress, &enetBuffer, 1);
    if(err==0) {
        // would have blocked, need to resend later
    } else if(err < 0) {
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Removing game announcement failed!");
    }
}


