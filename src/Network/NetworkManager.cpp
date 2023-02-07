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

#include <Network/NetworkManager.h>

#include <Network/ENetHelper.h>

#include <GameInitSettings.h>

#include <misc/exceptions.h>

#include <algorithm>
#include <limits>

NetworkManager::NetworkManager(uint16_t port, std::string metaserver) {

    if (enet_initialize() != 0) {
        THROW(std::runtime_error, "NetworkManager: An error occurred while initializing ENet.");
    }

    ENetAddress address{};
    address.host = ENET_HOST_ANY;
    address.port = port;

    host_ = enet_host_create(&address, 32, 2, 0, 0);
    if (host_ == nullptr) {
        enet_deinitialize();
        THROW(std::runtime_error, "NetworkManager: An error occurred while trying to create a server host.");
    }

    if (enet_host_compress_with_range_coder(host_) < 0) {
        enet_deinitialize();
        THROW(std::runtime_error, "NetworkManager: Cannot activate range coder.");
    }

    try {
        pLANGameFinderAndAnnouncer_ = std::make_unique<LANGameFinderAndAnnouncer>();
        pMetaServerClient_          = std::make_unique<MetaServerClient>(std::move(metaserver));
    } catch (...) {
        enet_deinitialize();
        throw;
    }
}

NetworkManager::~NetworkManager() {
    pMetaServerClient_.reset();
    pLANGameFinderAndAnnouncer_.reset();
    enet_host_destroy(host_);
    enet_deinitialize();
}

void NetworkManager::startServer(bool bLANServer, std::string serverName, std::string playerName,
                                 GameInitSettings* pGameInitSettings, int numPlayers, int maxPlayers) {
    if (numPlayers <= 0 || numPlayers > std::numeric_limits<uint8_t>::max() || maxPlayers <= 0
        || maxPlayers > std::numeric_limits<uint8_t>::max())
        THROW(std::invalid_argument, "Too many players ({}/{})!", numPlayers, maxPlayers);

    const auto numPlayers8 = static_cast<uint8_t>(numPlayers);
    const auto maxPlayers8 = static_cast<uint8_t>(maxPlayers);

    bIsServer_         = true;
    bLANServer_        = bLANServer;
    numPlayers_        = numPlayers;
    maxPlayers_        = maxPlayers;
    playerName_        = std::move(playerName);
    pGameInitSettings_ = pGameInitSettings;

    std::string map_name{reinterpret_cast<const char*>(pGameInitSettings->getFilename().u8string().c_str())};

    if (bLANServer) {
        if (pLANGameFinderAndAnnouncer_ != nullptr) {
            pLANGameFinderAndAnnouncer_->startAnnounce(std::move(serverName), host_->address.port, std::move(map_name),
                                                       numPlayers8, maxPlayers8);
        }
    } else {
        if (pMetaServerClient_ != nullptr) {
            pMetaServerClient_->startAnnounce(std::move(serverName), host_->address.port, std::move(map_name),
                                              numPlayers8, maxPlayers8);
        }
    }
}

void NetworkManager::updateServer(int numPlayers) {
    if (numPlayers <= 0 || numPlayers > std::numeric_limits<uint8_t>::max())
        THROW(std::invalid_argument, "Too many players ({})!", numPlayers);

    const auto numPlayers8 = static_cast<uint8_t>(numPlayers);

    if (bLANServer_) {
        if (pLANGameFinderAndAnnouncer_ != nullptr) {
            pLANGameFinderAndAnnouncer_->updateAnnounce(numPlayers8);
        }
    } else {
        if (pMetaServerClient_ != nullptr) {
            pMetaServerClient_->updateAnnounce(numPlayers8);
        }
    }

    this->numPlayers_ = numPlayers;
}

void NetworkManager::stopServer() {
    if (bLANServer_) {
        if (pLANGameFinderAndAnnouncer_ != nullptr) {
            pLANGameFinderAndAnnouncer_->stopAnnounce();
        }
    } else {
        if (pMetaServerClient_ != nullptr) {
            pMetaServerClient_->stopAnnounce();
        }
    }

    bIsServer_         = false;
    bLANServer_        = false;
    pGameInitSettings_ = nullptr;
}

void NetworkManager::connect(const std::string& hostname, uint16_t port, std::string playerName) {
    ENetAddress address;

    if (enet_address_set_host(&address, hostname.c_str()) < 0) {
        THROW(std::runtime_error, "NetworkManager: Resolving hostname '{}' failed!", hostname);
    }
    address.port = port;

    connect(address, std::move(playerName));
}

void NetworkManager::connect(ENetAddress address, std::string playerName) {
    debugNetwork("Connecting to %s:%d\n", Address2String(address), address.port);

    connectPeer_ = enet_host_connect(host_, &address, 2, 0);
    if (connectPeer_ == nullptr) {
        THROW(std::runtime_error, "NetworkManager: No available peers for initiating a connection.");
    }

    this->playerName_ = std::move(playerName);

    connectPeer_->data = new PeerData(connectPeer_, PeerData::PeerState::WaitingForConnect);
    awaitingConnectionList_.push_back(connectPeer_);
}

void NetworkManager::disconnect() {
    for (auto* pAwaitingConnectionPeer : awaitingConnectionList_) {
        enet_peer_disconnect_later(pAwaitingConnectionPeer, NETWORKDISCONNECT_QUIT);
    }
    for (auto* pCurrentPeer : peerList_) {
        enet_peer_disconnect_later(pCurrentPeer, NETWORKDISCONNECT_QUIT);
    }
}

void NetworkManager::update() {
    if (pLANGameFinderAndAnnouncer_ != nullptr) {
        pLANGameFinderAndAnnouncer_->update();
    }

    if (pMetaServerClient_ != nullptr) {
        pMetaServerClient_->update();
    }

    if (bIsServer_) {
        // Check for timeout of one client
        if (!awaitingConnectionList_.empty()) {
            auto* const pCurrentPeer = awaitingConnectionList_.front();
            auto* const peerData     = static_cast<PeerData*>(pCurrentPeer->data);

            if (peerData->peerState_ == PeerData::PeerState::ReadyForOtherPeersToConnect) {
                if (numPlayers_ >= maxPlayers_) {
                    enet_peer_disconnect_later(pCurrentPeer, NETWORKDISCONNECT_GAME_FULL);
                } else {
                    // only one peer should be in state 'PeerState::WaitingForOtherPeersToConnect'
                    peerData->peerState_            = PeerData::PeerState::WaitingForOtherPeersToConnect;
                    peerData->timeout_              = dune::dune_clock::now() + AWAITING_CONNECTION_TIMEOUT;
                    peerData->notYetConnectedPeers_ = peerList_;

                    if (peerData->notYetConnectedPeers_.empty()) {
                        // first client on this server
                        // => change immediately to connected

                        // get change event list first
                        const ChangeEventList changeEventList =
                            pGetChangeEventListForNewPlayerCallback_(peerData->name_);

                        debugNetwork("Moving '%s' from awaiting connection list to peer list\n",
                                     peerData->name_.c_str());
                        peerList_.push_back(pCurrentPeer);
                        peerData->peerState_ = PeerData::PeerState::Connected;
                        peerData->timeout_   = dune::dune_clock::time_point{};
                        awaitingConnectionList_.remove(pCurrentPeer);

                        // send peer game settings
                        ENetPacketOStream packetOStream2(ENET_PACKET_FLAG_RELIABLE);
                        packetOStream2.writeUint32(NETWORKPACKET_SENDGAMEINFO);
                        pGameInitSettings_->save(packetOStream2);

                        changeEventList.save(packetOStream2);

                        sendPacketToPeer(pCurrentPeer, packetOStream2);
                    } else {
                        // instruct all connected peers to connect

                        ENetPacketOStream packetOStream(ENET_PACKET_FLAG_RELIABLE);
                        packetOStream.writeUint32(NETWORKPACKET_CONNECT);
                        packetOStream.writeUint32(SDL_SwapBE32(pCurrentPeer->address.host));
                        packetOStream.writeUint16(pCurrentPeer->address.port);
                        packetOStream.writeString(peerData->name_);

                        sendPacketToAllConnectedPeers(packetOStream);
                    }
                }
            }

            if (peerData->timeout_ != dune::dune_clock::time_point{} && dune::dune_clock::now() > peerData->timeout_) {
                // timeout
                switch (peerData->peerState_) {
                    case PeerData::PeerState::WaitingForName: {
                        // nothing to do
                    } break;

                    case PeerData::PeerState::WaitingForOtherPeersToConnect: {
                        // the client awaiting connection has timed out => send everyone a disconnect message
                        ENetPacketOStream packetStream(ENET_PACKET_FLAG_RELIABLE);
                        packetStream.writeUint32(NETWORKPACKET_DISCONNECT);
                        packetStream.writeUint32(SDL_SwapBE32(pCurrentPeer->address.host));
                        packetStream.writeUint16(pCurrentPeer->address.port);

                        sendPacketToAllConnectedPeers(packetStream);

                        enet_peer_disconnect(pCurrentPeer, NETWORKDISCONNECT_TIMEOUT);

                        awaitingConnectionList_.pop_front();
                    } break;

                    case PeerData::PeerState::Connected:
                    default: {
                        // should never happen
                    } break;
                }
            }
        }
    }

    ENetEvent event;
    while (enet_host_service(host_, &event, 0) > 0) {

        ENetPeer* peer = event.peer;

        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                if (bIsServer_) {
                    // Server
                    debugNetwork("NetworkManager: %s:%u connected.\n", Address2String(peer->address),
                                 peer->address.port);

                    auto newPeerData      = std::make_unique<PeerData>(peer, PeerData::PeerState::WaitingForName);
                    newPeerData->timeout_ = dune::dune_clock::now() + AWAITING_CONNECTION_TIMEOUT;

                    debugNetwork("Adding '%s' to awaiting connection list\n", newPeerData->name_);
                    awaitingConnectionList_.push_back(peer);

                    // Send name
                    ENetPacketOStream packetStream(ENET_PACKET_FLAG_RELIABLE);
                    packetStream.writeUint32(NETWORKPACKET_SENDNAME);
                    packetStream.writeString(playerName_);

                    peer->data = newPeerData.release();
                    sendPacketToPeer(peer, packetStream);
                } else if (connectPeer_ != nullptr) {
                    // Client
                    auto* peerData = static_cast<PeerData*>(peer->data);

                    if (peer == connectPeer_) {
                        ENetPacketOStream packetStream(ENET_PACKET_FLAG_RELIABLE);
                        packetStream.writeUint32(NETWORKPACKET_SENDNAME);
                        packetStream.writeString(playerName_);

                        sendPacketToHost(packetStream);

                        peerData->peerState_ = PeerData::PeerState::WaitingForOtherPeersToConnect;
                        peerData->timeout_   = dune::dune_clock::time_point{};
                    } else {
                        debugNetwork("NetworkManager: %s:%u connected.\n", Address2String(peer->address).c_str(),
                                     peer->address.port);

                        const auto* pConnectPeerData = static_cast<PeerData*>(connectPeer_->data);

                        if (pConnectPeerData->peerState_ == PeerData::PeerState::WaitingForOtherPeersToConnect) {
                            if (peerData == nullptr) {
                                peerData   = new PeerData(peer, PeerData::PeerState::Connected);
                                peer->data = peerData;

                                debugNetwork("Adding '%s' to awaiting connection list\n", peerData->name_.c_str());
                                awaitingConnectionList_.push_back(peer);
                            }
                        } else {
                            ENetPacketOStream packetStream1(ENET_PACKET_FLAG_RELIABLE);
                            packetStream1.writeUint32(NETWORKPACKET_PEER_CONNECTED);
                            packetStream1.writeUint32(SDL_SwapBE32(peer->address.host));
                            packetStream1.writeUint16(peer->address.port);

                            sendPacketToHost(packetStream1);

                            ENetPacketOStream packetStream2(ENET_PACKET_FLAG_RELIABLE);
                            packetStream2.writeUint32(NETWORKPACKET_SENDNAME);
                            packetStream2.writeString(playerName_);

                            sendPacketToPeer(peer, packetStream2);
                        }
                    }
                } else {
                    enet_peer_disconnect(peer, NETWORKDISCONNECT_TIMEOUT);
                }
            } break;

            case ENET_EVENT_TYPE_RECEIVE: {
                // debugNetwork("NetworkManager: A packet of length %u was received from %s:%u on channel %u on this
                // server.\n",
                //                 (unsigned int) event.packet->dataLength, Address2String(peer->address).c_str(),
                //                 peer->address.port, event.channelID);

                ENetPacketIStream packetStream(event.packet);

                handlePacket(peer, packetStream);
            } break;

            case ENET_EVENT_TYPE_DISCONNECT: {
                std::unique_ptr<PeerData> peerData{static_cast<PeerData*>(peer->data)};

                const auto disconnectCause = event.data;

                debugNetwork("NetworkManager: %s:%u (%s) disconnected (%d).\n", Address2String(peer->address),
                             peer->address.port, (peerData != nullptr) ? peerData->name_ : "unknown", disconnectCause);

                if (peerData != nullptr) {
                    if (std::ranges::find(awaitingConnectionList_, peer) != awaitingConnectionList_.end()) {
                        if (peerData->peerState_ == PeerData::PeerState::WaitingForOtherPeersToConnect) {
                            ENetPacketOStream packetStream(ENET_PACKET_FLAG_RELIABLE);
                            packetStream.writeUint32(NETWORKPACKET_DISCONNECT);
                            packetStream.writeUint32(SDL_SwapBE32(peer->address.host));
                            packetStream.writeUint16(peer->address.port);

                            sendPacketToAllConnectedPeers(packetStream);
                        }

                        debugNetwork("Removing '%s' from awaiting connection list\n", peerData->name_);
                        awaitingConnectionList_.remove(peer);
                    }

                    if (std::ranges::find(peerList_, peer) != peerList_.end()) {
                        debugNetwork("Removing '%s' from peer list\n", peerData->name_.c_str());
                        peerList_.remove(peer);

                        ENetPacketOStream packetStream(ENET_PACKET_FLAG_RELIABLE);
                        packetStream.writeUint32(NETWORKPACKET_DISCONNECT);
                        packetStream.writeUint32(SDL_SwapBE32(peer->address.host));
                        packetStream.writeUint16(peer->address.port);

                        sendPacketToAllConnectedPeers(packetStream);

                        if (pOnPeerDisconnected_) {
                            pOnPeerDisconnected_(peerData->name_, (peer == connectPeer_), disconnectCause);
                        }
                    } else {
                        if (peer == connectPeer_) {
                            // host disconnected while establishing connection
                            if (pOnPeerDisconnected_) {
                                pOnPeerDisconnected_(peerData->name_, true, disconnectCause);
                            }
                        }
                    }
                }

                // delete peer data
                peer->data = nullptr;
                peerData.reset();

                if (peer == connectPeer_) {
                    connectPeer_ = nullptr;
                }

            } break;

            default: {

            } break;
        }
    }
}

void NetworkManager::handlePacket(ENetPeer* peer, ENetPacketIStream& packetStream) {
    try {
        const auto packetType = packetStream.readUint32();

        switch (packetType) {
            case NETWORKPACKET_CONNECT: {

                if (!bIsServer_) {
                    ENetAddress address;

                    address.host = SDL_SwapBE32(packetStream.readUint32());
                    address.port = packetStream.readUint16();

                    debugNetwork("Connecting to %s:%d\n", Address2String(address).c_str(), address.port);

                    auto* newPeer = enet_host_connect(host_, &address, 2, 0);
                    if (newPeer == nullptr) {
                        debugNetwork("NetworkManager: No available peers for initiating a connection.");
                    } else {
                        auto* peerData  = new PeerData(newPeer, PeerData::PeerState::WaitingForOtherPeersToConnect);
                        peerData->name_ = packetStream.readString();

                        newPeer->data = peerData;
                        debugNetwork("Adding '%s' to awaiting connection list\n", peerData->name_.c_str());
                        awaitingConnectionList_.push_back(newPeer);
                    }
                }
            } break;

            case NETWORKPACKET_DISCONNECT: {
                ENetAddress address;

                address.host = SDL_SwapBE32(packetStream.readUint32());
                address.port = packetStream.readUint16();

                for (auto* const pCurrentPeer : peerList_) {
                    if ((pCurrentPeer->address.host == address.host) && (pCurrentPeer->address.port == address.port)) {
                        enet_peer_disconnect_later(pCurrentPeer, NETWORKDISCONNECT_QUIT);
                        break;
                    }
                }

                for (auto* const pAwaitingConnectionPeer : awaitingConnectionList_) {
                    if ((pAwaitingConnectionPeer->address.host == address.host)
                        && (pAwaitingConnectionPeer->address.port == address.port)) {
                        enet_peer_disconnect_later(pAwaitingConnectionPeer, NETWORKDISCONNECT_QUIT);
                        break;
                    }
                }

            } break;

            case NETWORKPACKET_PEER_CONNECTED: {

                ENetAddress address;

                address.host = SDL_SwapBE32(packetStream.readUint32());
                address.port = packetStream.readUint16();

                if (isServer()) {

                    if (!awaitingConnectionList_.empty()) {
                        ENetPeer* pCurrentPeer = awaitingConnectionList_.front();
                        auto* peerData         = static_cast<PeerData*>(pCurrentPeer->data);
                        if (!peerData) {
                            break;
                        }

                        if ((pCurrentPeer->address.host == address.host)
                            && (pCurrentPeer->address.port == address.port)) {

                            peerData->notYetConnectedPeers_.remove(peer);

                            if (peerData->notYetConnectedPeers_.empty()) {
                                // send connected to all peers (excluding the new one)
                                ENetPacketOStream packetOStream(ENET_PACKET_FLAG_RELIABLE);
                                packetOStream.writeUint32(NETWORKPACKET_PEER_CONNECTED);
                                packetOStream.writeUint32(SDL_SwapBE32(pCurrentPeer->address.host));
                                packetOStream.writeUint16(pCurrentPeer->address.port);

                                sendPacketToAllConnectedPeers(packetOStream);

                                // get change event list first
                                ChangeEventList changeEventList =
                                    pGetChangeEventListForNewPlayerCallback_(peerData->name_);

                                // move peer to peer list
                                debugNetwork("Moving '%s' from awaiting connection list to peer list\n",
                                             peerData->name_.c_str());
                                peerList_.push_back(pCurrentPeer);
                                peerData->peerState_ = PeerData::PeerState::Connected;
                                peerData->timeout_   = dune::dune_clock::time_point{};
                                awaitingConnectionList_.remove(pCurrentPeer);

                                // send peer game settings
                                ENetPacketOStream packetOStream2(ENET_PACKET_FLAG_RELIABLE);
                                packetOStream2.writeUint32(NETWORKPACKET_SENDGAMEINFO);
                                pGameInitSettings_->save(packetOStream2);

                                changeEventList.save(packetOStream2);

                                sendPacketToPeer(pCurrentPeer, packetOStream2);
                            }
                        }
                    }
                } else {
                    for (auto iter = awaitingConnectionList_.begin(); iter != awaitingConnectionList_.end(); ++iter) {
                        auto* const pCurrentPeer = *iter;

                        if ((pCurrentPeer->address.host == address.host)
                            && (pCurrentPeer->address.port == address.port)) {
                            auto* peerData = static_cast<PeerData*>(pCurrentPeer->data);
                            if (!peerData) {
                                continue;
                            }
                            debugNetwork("Moving '%s' from awaiting connection list to peer list\n",
                                         peerData->name_.c_str());
                            peerList_.push_back(pCurrentPeer);
                            peerData->peerState_ = PeerData::PeerState::Connected;
                            peerData->timeout_   = dune::dune_clock::time_point{};
                            awaitingConnectionList_.erase(iter);
                            break;
                        }
                    }
                }

            } break;

            case NETWORKPACKET_SENDGAMEINFO: {
                if (!connectPeer_) {
                    break;
                }

                auto* peerData = static_cast<PeerData*>(connectPeer_->data);
                if (!peerData) {
                    break;
                }

                peerList_            = awaitingConnectionList_;
                peerData->peerState_ = PeerData::PeerState::Connected;
                peerData->timeout_   = dune::dune_clock::time_point{};
                awaitingConnectionList_.clear();

                const GameInitSettings gameInitSettings(packetStream);
                const ChangeEventList changeEventList(packetStream);

                if (pOnReceiveGameInfo_) {
                    pOnReceiveGameInfo_(gameInitSettings, changeEventList);
                }
            } break;

            case NETWORKPACKET_SENDNAME: {
                auto* peerData = static_cast<PeerData*>(peer->data);
                if (!peerData) {
                    break;
                }

                const auto newName = packetStream.readString();
                auto bFoundName    = false;

                // check if name already exists
                if (bIsServer_) {
                    if (playerName_ == newName) {
                        enet_peer_disconnect_later(peer, NETWORKDISCONNECT_PLAYER_EXISTS);
                        bFoundName = true;
                    }

                    if (!bFoundName) {
                        for (ENetPeer* pCurrentPeer : peerList_) {
                            auto* pCurrentPeerData = static_cast<PeerData*>(pCurrentPeer->data);
                            if (!pCurrentPeerData) {
                                continue;
                            }
                            if (pCurrentPeerData->name_ == newName) {
                                enet_peer_disconnect_later(peer, NETWORKDISCONNECT_PLAYER_EXISTS);
                                bFoundName = true;
                                break;
                            }
                        }
                    }

                    if (!bFoundName) {
                        for (ENetPeer* pAwaitingConnectionPeer : awaitingConnectionList_) {
                            auto* pAwaitingConnectionPeerData = static_cast<PeerData*>(pAwaitingConnectionPeer->data);
                            if (pAwaitingConnectionPeerData && (pAwaitingConnectionPeerData->name_ == newName)) {
                                enet_peer_disconnect_later(peer, NETWORKDISCONNECT_PLAYER_EXISTS);
                                bFoundName = true;
                                break;
                            }
                        }
                    }
                }

                if (!bFoundName) {
                    peerData->name_ = newName;

                    if (peerData->peerState_ == PeerData::PeerState::WaitingForName) {
                        peerData->peerState_ = PeerData::PeerState::ReadyForOtherPeersToConnect;
                    }
                }
            } break;

            case NETWORKPACKET_CHATMESSAGE: {
                auto* peerData = static_cast<PeerData*>(peer->data);
                if (!peerData) {
                    break;
                }

                const auto message = packetStream.readString();
                if (pOnReceiveChatMessage_) {
                    pOnReceiveChatMessage_(peerData->name_, message);
                }
            } break;

            case NETWORKPACKET_CHANGEEVENTLIST: {
                const ChangeEventList changeEventList(packetStream);

                if (pOnReceiveChangeEventList_) {
                    pOnReceiveChangeEventList_(changeEventList);
                }
            } break;

            case NETWORKPACKET_STARTGAME: {
                const auto timeLeft = packetStream.readUint32();

                if (pOnStartGame_) {
                    pOnStartGame_(dune::as_dune_clock_duration(timeLeft));
                }
            } break;

            case NETWORKPACKET_COMMANDLIST: {
                auto* peerData = static_cast<PeerData*>(peer->data);
                if (!peerData) {
                    break;
                }

                const CommandList commandList(packetStream);

                if (pOnReceiveCommandList_) {
                    pOnReceiveCommandList_(peerData->name_, commandList);
                }
            } break;

            case NETWORKPACKET_SELECTIONLIST: {
                auto* peerData = static_cast<PeerData*>(peer->data);
                if (!peerData) {
                    break;
                }

                const auto groupListIndex = packetStream.readSint32();
                const auto selectedList   = packetStream.readUint32Set();

                if (pOnReceiveSelectionList_) {
                    pOnReceiveSelectionList_(peerData->name_, selectedList, groupListIndex);
                }
            } break;

            default: {
                sdl2::log_info("NetworkManager: Unknown packet type {}", packetType);
            }
        }

    } catch (InputStream::eof&) {
        sdl2::log_info("NetworkManager: Received packet is too small");
    } catch (std::exception& e) {
        sdl2::log_info("NetworkManager: {}", e.what());
    }
}

void NetworkManager::sendPacketToHost(ENetPacketOStream& packetStream, int channel) {
    if (connectPeer_ == nullptr) {
        sdl2::log_info("NetworkManager: sendPacketToHost() called on server!");
        return;
    }

    if (channel < 0 || channel >= std::numeric_limits<enet_uint8>::max())
        THROW(std::invalid_argument, "Invalid channel ({})!", channel);

    ENetPacket* enetPacket = packetStream.getPacket();

    if (enet_peer_send(connectPeer_, static_cast<enet_uint8>(channel), enetPacket) < 0) {
        sdl2::log_info("NetworkManager: Cannot send packet!");
    }
}

void NetworkManager::sendPacketToPeer(ENetPeer* peer, ENetPacketOStream& packetStream, int channel) {
    if (channel < 0 || channel >= std::numeric_limits<enet_uint8>::max())
        THROW(std::invalid_argument, "Invalid channel ({})!", channel);

    ENetPacket* enetPacket = packetStream.getPacket();

    if (enet_peer_send(peer, static_cast<enet_uint8>(channel), enetPacket) < 0) {
        sdl2::log_info("NetworkManager: Cannot send packet!");
    }

    if (enetPacket->referenceCount == 0) {
        enet_packet_destroy(enetPacket);
    }
}

void NetworkManager::sendPacketToAllConnectedPeers(ENetPacketOStream& packetStream, int channel) {
    if (channel < 0 || channel >= std::numeric_limits<enet_uint8>::max())
        THROW(std::invalid_argument, "Invalid channel ({})!", channel);

    ENetPacket* enetPacket = packetStream.getPacket();

    for (auto* pCurrentPeer : peerList_) {
        if (enet_peer_send(pCurrentPeer, static_cast<enet_uint8>(channel), enetPacket) < 0) {
            sdl2::log_info("NetworkManager: Cannot send packet!");
        }
    }

    if (enetPacket->referenceCount == 0) {
        enet_packet_destroy(enetPacket);
    }
}

void NetworkManager::sendChatMessage(std::string_view message) {
    ENetPacketOStream packetStream(ENET_PACKET_FLAG_RELIABLE);
    packetStream.writeUint32(NETWORKPACKET_CHATMESSAGE);
    packetStream.writeString(message);

    sendPacketToAllConnectedPeers(packetStream);
}

void NetworkManager::sendChangeEventList(const ChangeEventList& changeEventList) {
    ENetPacketOStream packetStream(ENET_PACKET_FLAG_RELIABLE);
    packetStream.writeUint32(NETWORKPACKET_CHANGEEVENTLIST);
    changeEventList.save(packetStream);

    if (bIsServer_) {
        sendPacketToAllConnectedPeers(packetStream);
    } else {
        sendPacketToHost(packetStream);
    }
}

void NetworkManager::sendStartGame(unsigned int timeLeft) {
    for (auto* pCurrentPeer : peerList_) {
        ENetPacketOStream packetStream(ENET_PACKET_FLAG_RELIABLE);
        packetStream.writeUint32(NETWORKPACKET_STARTGAME);

        packetStream.writeUint32(timeLeft - pCurrentPeer->roundTripTime / 2);

        sendPacketToPeer(pCurrentPeer, packetStream);
    }
}

void NetworkManager::sendCommandList(const CommandList& commandList) {
    ENetPacketOStream packetStream(ENET_PACKET_FLAG_UNSEQUENCED);
    packetStream.writeUint32(NETWORKPACKET_COMMANDLIST);
    commandList.save(packetStream);

    sendPacketToAllConnectedPeers(packetStream, 1);
}

void NetworkManager::sendSelectedList(const dune::selected_set_type& selectedList, int groupListIndex) {
    ENetPacketOStream packetStream(ENET_PACKET_FLAG_RELIABLE);
    packetStream.writeUint32(NETWORKPACKET_SELECTIONLIST);
    packetStream.writeSint32(groupListIndex);
    packetStream.writeUint32Set(selectedList);

    sendPacketToAllConnectedPeers(packetStream, 0);
}

std::vector<std::string> NetworkManager::getConnectedPeers() const {
    std::vector<std::string> peerNameList;
    peerNameList.reserve(peerList_.size());

    for (const auto* pPeer : peerList_) {
        const auto* peerData = static_cast<PeerData*>(pPeer->data);
        if (peerData != nullptr) {
            peerNameList.push_back(peerData->name_);
        }
    }

    return peerNameList;
}

int NetworkManager::getMaxPeerRoundTripTime() const {
    if (peerList_.empty())
        return 1; // No peers - RTT is not meaningful.
    const auto max_rtt =
        std::ranges::max(peerList_, {}, [](const auto* const p) { return p->roundTripTime; })->roundTripTime;

    return static_cast<int>(max_rtt);
}
