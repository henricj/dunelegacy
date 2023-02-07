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

#include <misc/dune_clock.h>
#include <misc/exceptions.h>

#include <config.h>

#include <ranges>

inline constexpr auto NETWORKPACKET_ANNOUNCEGAME           = 1;
inline constexpr auto NETWORKPACKET_REMOVEGAMEANNOUNCEMENT = 2;
inline constexpr auto NETWORKPACKET_REQUESTANNOUNCE        = 3;

#ifdef _MSC_VER_
#    pragma pack(push, 1)
#    define PACKEDDATASTRUCTURE
#elif defined(__GNUC__)
#    define PACKEDDATASTRUCTURE __attribute__((packed))
#else
#    define PACKEDDATASTRUCTURE
#endif

struct PACKEDDATASTRUCTURE NetworkPacket_Unknown {
    enet_uint32 magicNumber_;
    enet_uint8 type_;
};

struct PACKEDDATASTRUCTURE NetworkPacket_AnnounceGame {
    enet_uint32 magicNumber_;
    enet_uint8 type_;
    enet_uint16 serverPort_;
    char serverName_[LANGAME_ANNOUNCER_MAXGAMENAMESIZE];
    char serverVersion_[LANGAME_ANNOUNCER_MAXGAMEVERSIONSIZE];
    char mapName_[LANGAME_ANNOUNCER_MAXMAPNAMESIZE];
    char numPlayers_;
    char maxPlayers_;
};

struct PACKEDDATASTRUCTURE NetworkPacket_RemoveGameAnnouncement {
    enet_uint32 magicNumber_;
    enet_uint8 type_;
    enet_uint16 serverPort_;
};

struct PACKEDDATASTRUCTURE NetworkPacket_RequestGame {
    enet_uint32 magicNumber_;
    enet_uint8 type_;
};

#ifdef _MSC_VER_
#    pragma pack(pop)
#endif

LANGameFinderAndAnnouncer::LANGameFinderAndAnnouncer()
    : announceSocket_(enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM)) {

    if (announceSocket_ == ENET_SOCKET_NULL) {
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Creating socket failed!");
    }

    if (enet_socket_set_option(announceSocket_, ENET_SOCKOPT_REUSEADDR, 1) < 0) {
        enet_socket_destroy(announceSocket_);
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Setting socket option 'ENET_SOCKOPT_REUSEADDR' failed!");
    }

    if (enet_socket_set_option(announceSocket_, ENET_SOCKOPT_NONBLOCK, 1) < 0) {
        enet_socket_destroy(announceSocket_);
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Setting socket option 'ENET_SOCKOPT_NONBLOCK' failed!");
    }

    if (enet_socket_set_option(announceSocket_, ENET_SOCKOPT_BROADCAST, 1) < 0) {
        enet_socket_destroy(announceSocket_);
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Setting socket option 'ENET_SOCKOPT_BROADCAST' failed!");
    }

    ENetAddress address{};
    address.host = ENET_HOST_ANY;
    address.port = LANGAME_ANNOUNCER_PORT;

    if (enet_socket_bind(announceSocket_, &address) < 0) {
        enet_socket_destroy(announceSocket_);
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Binding socket to address failed!");
    }
}

LANGameFinderAndAnnouncer::~LANGameFinderAndAnnouncer() {
    if (serverPort_ > 0) {
        try {
            stopAnnounce();
        } catch (std::exception& e) {
            sdl2::log_info("LANGameFinderAndAnnouncer::~LANGameFinderAndAnnouncer(): {}", e.what());
        }
    }
    enet_socket_destroy(announceSocket_);
}

void LANGameFinderAndAnnouncer::startAnnounce(std::string serverName, uint16_t serverPort, std::string mapName,
                                              uint8_t numPlayers, uint8_t maxPlayers) {
    this->serverName_ = std::move(serverName);
    this->serverPort_ = serverPort;
    this->mapName_    = std::move(mapName);
    this->numPlayers_ = numPlayers;
    this->maxPlayers_ = maxPlayers;
    lastAnnounce_     = dune::dune_clock::now();
}

void LANGameFinderAndAnnouncer::updateAnnounce(uint8_t numPlayers) {
    this->numPlayers_ = numPlayers;
    if (serverPort_ > 0) {
        announceGame();
    }
}

void LANGameFinderAndAnnouncer::stopAnnounce() {
    sendRemoveGameAnnouncement();
    serverName_.clear();
    serverPort_ = 0;
}

void LANGameFinderAndAnnouncer::update() {
    if (serverPort_ > 0) {
        if (dune::dune_clock::now() - lastAnnounce_ > LANGAME_ANNOUNCER_INTERVAL) {
            announceGame();
        }
    }

    updateServerInfoList();

    receivePackets();
}

void LANGameFinderAndAnnouncer::announceGame() {

    ENetAddress destinationAddress{};
    destinationAddress.host = ENET_HOST_BROADCAST;
    destinationAddress.port = LANGAME_ANNOUNCER_PORT;

    NetworkPacket_AnnounceGame announcePacket{};

    announcePacket.magicNumber_ = SDL_SwapLE32(LANGAME_ANNOUNCER_MAGICNUMBER);
    announcePacket.type_        = NETWORKPACKET_ANNOUNCEGAME;
    std::ranges::copy(std::ranges::take_view(serverName_, sizeof announcePacket.serverName_),
                      announcePacket.serverName_);
    std::ranges::copy(std::ranges::take_view(VERSIONSTRING, sizeof announcePacket.serverVersion_),
                      announcePacket.serverVersion_);
    announcePacket.serverPort_ = SDL_SwapLE16(serverPort_);
    std::ranges::copy(std::ranges::take_view(mapName_, sizeof announcePacket.mapName_), announcePacket.mapName_);
    announcePacket.numPlayers_ = static_cast<char>(numPlayers_);
    announcePacket.maxPlayers_ = static_cast<char>(maxPlayers_);

    ENetBuffer enetBuffer{};
    enetBuffer.data       = &announcePacket;
    enetBuffer.dataLength = sizeof(NetworkPacket_AnnounceGame);

    const int err = enet_socket_send(announceSocket_, &destinationAddress, &enetBuffer, 1);
    if (err == 0) {
        // blocked
    } else if (err < 0) {
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Announcing failed!");
    } else {
        lastAnnounce_ = dune::dune_clock::now();
    }
}

void LANGameFinderAndAnnouncer::refreshServerList() const {
    ENetAddress destinationAddress{};
    destinationAddress.host = ENET_HOST_BROADCAST;
    destinationAddress.port = LANGAME_ANNOUNCER_PORT;

    NetworkPacket_RequestGame requestPacket{};

    requestPacket.magicNumber_ = SDL_SwapLE32(LANGAME_ANNOUNCER_MAGICNUMBER);
    requestPacket.type_        = NETWORKPACKET_REQUESTANNOUNCE;

    ENetBuffer enetBuffer{};
    enetBuffer.data       = &requestPacket;
    enetBuffer.dataLength = sizeof(NetworkPacket_RequestGame);

    const int err = enet_socket_send(announceSocket_, &destinationAddress, &enetBuffer, 1);
    if (err == 0) {
        // blocked
    } else if (err < 0) {
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Refreshing server list failed!");
    }
}

void LANGameFinderAndAnnouncer::receivePackets() {
    NetworkPacket_AnnounceGame announcePacket{};

    ENetAddress senderAddress;

    ENetBuffer enetBuffer{};
    enetBuffer.data       = &announcePacket;
    enetBuffer.dataLength = sizeof(announcePacket);

    int receivedBytes = enet_socket_receive(announceSocket_, &senderAddress, &enetBuffer, 1);
    if (receivedBytes == 0) {
        // blocked
    } else if (receivedBytes < 0) {
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Receiving data failed!");
    } else {
        if ((receivedBytes == sizeof(NetworkPacket_AnnounceGame))
            && (SDL_SwapLE32(announcePacket.magicNumber_) == LANGAME_ANNOUNCER_MAGICNUMBER)
            && (announcePacket.type_ == NETWORKPACKET_ANNOUNCEGAME)
            && (strncmp(announcePacket.serverVersion_, VERSIONSTRING, LANGAME_ANNOUNCER_MAXGAMEVERSIONSIZE) == 0)) {

            auto make_string = [](auto& src) { return std::string{std::begin(src), std::ranges::find(src, '\0')}; };

            const auto serverName = make_string(announcePacket.serverName_);
            const auto serverPort = SDL_SwapLE16(announcePacket.serverPort_);

            const auto mapName = make_string(announcePacket.mapName_);

            const auto numPlayers = static_cast<int>(announcePacket.numPlayers_);
            const auto maxPlayers = static_cast<int>(announcePacket.maxPlayers_);

            GameServerInfo gameServerInfo;
            gameServerInfo.serverAddress.host = senderAddress.host;
            gameServerInfo.serverAddress.port = serverPort;
            gameServerInfo.serverName         = serverName;
            gameServerInfo.mapName            = mapName;
            gameServerInfo.numPlayers         = numPlayers;
            gameServerInfo.maxPlayers         = maxPlayers;
            gameServerInfo.bPasswordProtected = false;
            gameServerInfo.lastUpdate         = dune::as_milliseconds(dune::dune_clock::now().time_since_epoch());

            bool bUpdate = false;
            for (auto& curGameServerInfo : gameServerInfoList_) {
                if ((curGameServerInfo.serverAddress.host == gameServerInfo.serverAddress.host)
                    && (curGameServerInfo.serverAddress.port == gameServerInfo.serverAddress.port)) {
                    curGameServerInfo = gameServerInfo;
                    bUpdate           = true;
                    break;
                }
            }

            if (bUpdate) {
                if (pOnUpdateServer_)
                    pOnUpdateServer_(gameServerInfo);
            } else {
                gameServerInfoList_.push_back(std::move(gameServerInfo));
                if (pOnNewServer_)
                    pOnNewServer_(gameServerInfoList_.back());
            }
        } else if ((receivedBytes == sizeof(NetworkPacket_RemoveGameAnnouncement))
                   && (SDL_SwapLE32(announcePacket.magicNumber_) == LANGAME_ANNOUNCER_MAGICNUMBER)
                   && (announcePacket.type_ == NETWORKPACKET_REMOVEGAMEANNOUNCEMENT)) {

            int serverPort = SDL_SwapLE16(announcePacket.serverPort_);

            auto iter = gameServerInfoList_.begin();
            while (iter != gameServerInfoList_.end()) {
                if ((iter->serverAddress.host == senderAddress.host) && (iter->serverAddress.port == serverPort)) {
                    auto tmpGameServerInfo = std::move(*iter);

                    iter = gameServerInfoList_.erase(iter);

                    if (pOnRemoveServer_) {
                        pOnRemoveServer_(std::move(tmpGameServerInfo));
                    }
                } else {
                    ++iter;
                }
            }
        } else if ((serverPort_ > 0) && (receivedBytes == sizeof(NetworkPacket_RequestGame))
                   && (SDL_SwapLE32(announcePacket.magicNumber_) == LANGAME_ANNOUNCER_MAGICNUMBER)
                   && (announcePacket.type_ == NETWORKPACKET_REQUESTANNOUNCE)) {

            announceGame();
        }
    }
}

void LANGameFinderAndAnnouncer::updateServerInfoList() {
    const auto currentTime = dune::dune_clock::now();

    auto iter = gameServerInfoList_.begin();
    while (iter != gameServerInfoList_.end()) {
        if (dune::as_dune_clock_duration(iter->lastUpdate) + 3 * LANGAME_ANNOUNCER_INTERVAL
            < currentTime.time_since_epoch()) {
            if (pOnRemoveServer_) {
                pOnRemoveServer_(*iter);
            }
            iter = gameServerInfoList_.erase(iter);
        } else {
            ++iter;
        }
    }
}

void LANGameFinderAndAnnouncer::sendRemoveGameAnnouncement() {

    ENetAddress destinationAddress{};
    destinationAddress.host = ENET_HOST_BROADCAST;
    destinationAddress.port = LANGAME_ANNOUNCER_PORT;

    NetworkPacket_RemoveGameAnnouncement removeAnnouncementPacket{};

    removeAnnouncementPacket.magicNumber_ = SDL_SwapLE32(LANGAME_ANNOUNCER_MAGICNUMBER);
    removeAnnouncementPacket.type_        = NETWORKPACKET_REMOVEGAMEANNOUNCEMENT;
    removeAnnouncementPacket.serverPort_  = SDL_SwapLE16(serverPort_);

    ENetBuffer enetBuffer{};
    enetBuffer.data       = &removeAnnouncementPacket;
    enetBuffer.dataLength = sizeof(NetworkPacket_RemoveGameAnnouncement);

    const int err = enet_socket_send(announceSocket_, &destinationAddress, &enetBuffer, 1);
    if (err == 0) {
        // would have blocked, need to resend later
    } else if (err < 0) {
        THROW(std::runtime_error, "LANGameFinderAndAnnouncer: Removing game announcement failed!");
    }
}
