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

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <Network/ChangeEventList.h>
#include <Network/CommandList.h>
#include <Network/ENetPacketIStream.h>
#include <Network/ENetPacketOStream.h>

#include <Network/LANGameFinderAndAnnouncer.h>
#include <Network/MetaServerClient.h>

#include <misc/SDL2pp.h>
#include <misc/dune_clock.h>
#include <misc/string_util.h>

#include <enet/enet.h>

#include <functional>
#include <list>
#include <string>

inline constexpr auto NETWORKDISCONNECT_QUIT          = 1;
inline constexpr auto NETWORKDISCONNECT_TIMEOUT       = 2;
inline constexpr auto NETWORKDISCONNECT_PLAYER_EXISTS = 3;
inline constexpr auto NETWORKDISCONNECT_GAME_FULL     = 4;

inline constexpr auto NETWORKPACKET_UNKNOWN         = 0;
inline constexpr auto NETWORKPACKET_CONNECT         = 1;
inline constexpr auto NETWORKPACKET_DISCONNECT      = 2;
inline constexpr auto NETWORKPACKET_PEER_CONNECTED  = 3;
inline constexpr auto NETWORKPACKET_SENDGAMEINFO    = 4;
inline constexpr auto NETWORKPACKET_SENDNAME        = 5;
inline constexpr auto NETWORKPACKET_CHATMESSAGE     = 6;
inline constexpr auto NETWORKPACKET_CHANGEEVENTLIST = 7;
inline constexpr auto NETWORKPACKET_STARTGAME       = 8;
inline constexpr auto NETWORKPACKET_COMMANDLIST     = 9;
inline constexpr auto NETWORKPACKET_SELECTIONLIST   = 10;

inline constexpr auto AWAITING_CONNECTION_TIMEOUT = dune::as_dune_clock_duration(5000);

class GameInitSettings;

class NetworkManager {
public:
    NetworkManager(int port, const std::string& metaserver);
    NetworkManager(const NetworkManager& o) = delete;
    NetworkManager(NetworkManager&& o)      = delete;
    ~NetworkManager();

    NetworkManager& operator=(const NetworkManager&) = delete;
    NetworkManager& operator=(NetworkManager&&)      = delete;

    [[nodiscard]] bool isServer() const noexcept { return bIsServer; }

    void startServer(bool bLANServer, const std::string& serverName, const std::string& playerName,
                     GameInitSettings* pGameInitSettings, int numPlayers, int maxPlayers);
    void updateServer(int numPlayers);
    void stopServer();

    void connect(const std::string& hostname, int port, const std::string& playerName);
    void connect(ENetAddress address, const std::string& playerName);

    void disconnect();

    void update();

    void sendChatMessage(const std::string& message);

    void sendChangeEventList(const ChangeEventList& changeEventList);

    void sendStartGame(unsigned int timeLeft);

    void sendCommandList(const CommandList& commandList);

    void sendSelectedList(const Dune::selected_set_type& selectedList, int groupListIndex = -1);

    [[nodiscard]] std::list<std::string> getConnectedPeers() const {
        std::list<std::string> peerNameList;

        for (const ENetPeer* pPeer : peerList) {
            auto* peerData = static_cast<PeerData*>(pPeer->data);
            if (peerData != nullptr) {
                peerNameList.push_back(peerData->name);
            }
        }

        return peerNameList;
    }

    int getMaxPeerRoundTripTime() const;

    LANGameFinderAndAnnouncer* getLANGameFinderAndAnnouncer() { return pLANGameFinderAndAnnouncer.get(); }

    MetaServerClient* getMetaServerClient() { return pMetaServerClient.get(); }

    /**
        Sets the function that should be called when a chat message is received
        \param  pOnReceiveChatMessage   function to call on new chat message
    */
    void setOnReceiveChatMessage(std::function<void(const std::string&, const std::string&)> pOnReceiveChatMessage) {
        this->pOnReceiveChatMessage = pOnReceiveChatMessage;
    }

    /**
        Sets the function that should be called when game infos are received after connecting to the server.
        \param  pOnReceiveGameInfo  function to call on receive
    */
    void setOnReceiveGameInfo(std::function<void(const GameInitSettings&, const ChangeEventList&)> pOnReceiveGameInfo) {
        this->pOnReceiveGameInfo = pOnReceiveGameInfo;
    }

    /**
        Sets the function that should be called when a change event is received.
        \param  pOnReceiveChangeEventList   function to call on receive
    */
    void setOnReceiveChangeEventList(std::function<void(const ChangeEventList&)> pOnReceiveChangeEventList) {
        this->pOnReceiveChangeEventList = pOnReceiveChangeEventList;
    }

    /**
        Sets the function that should be called when a peer disconnects.
        \param  pOnPeerDisconnected function to call on disconnect
    */
    void setOnPeerDisconnected(std::function<void(const std::string&, bool, int)> pOnPeerDisconnected) {
        this->pOnPeerDisconnected = pOnPeerDisconnected;
    }

    /**
        Sets the function that can be used to retreive all house/player changes to get the current state
        \param pGetChangeEventListForNewPlayerCallback    function to call
    */
    void setGetChangeEventListForNewPlayerCallback(
        std::function<ChangeEventList(const std::string&)> pGetChangeEventListForNewPlayerCallback) {
        this->pGetChangeEventListForNewPlayerCallback = pGetChangeEventListForNewPlayerCallback;
    }

    /**
        Sets the function that should be called when the game is about to start and the time (in ms) left is received
        \param  pOnStartGame    function to call on receive
    */
    void setOnStartGame(std::function<void(dune::dune_clock::duration)> pOnStartGame) {
        this->pOnStartGame = pOnStartGame;
    }

    /**
        Sets the function that should be called when a command list is received.
        \param  pOnReceiveCommandList   function to call on receive
    */
    void setOnReceiveCommandList(std::function<void(const std::string&, const CommandList&)> pOnReceiveCommandList) {
        this->pOnReceiveCommandList = pOnReceiveCommandList;
    }

    /**
        Sets the function that should be called when a selection list is received.
        \param  pOnReceiveSelectionList function to call on receive
    */
    void setOnReceiveSelectionList(
        std::function<void(const std::string&, const Dune::selected_set_type&, int)> pOnReceiveSelectionList) {
        this->pOnReceiveSelectionList = pOnReceiveSelectionList;
    }

private:
    template<typename... Args>
    void debugNetwork(std::string_view format, Args&&... args) {
        sdl2::log_info(format, std::forward<Args>(args)...);
    }

    void sendPacketToHost(ENetPacketOStream& packetStream, int channel = 0);

    static void sendPacketToPeer(ENetPeer* peer, ENetPacketOStream& packetStream, int channel = 0);

    void sendPacketToAllConnectedPeers(ENetPacketOStream& packetStream, int channel = 0);

    void handlePacket(ENetPeer* peer, ENetPacketIStream& packetStream);

    class PeerData {
    public:
        enum class PeerState {
            WaitingForConnect,
            WaitingForName,
            ReadyForOtherPeersToConnect,
            WaitingForOtherPeersToConnect,
            Connected
        };

        PeerData(ENetPeer* pPeer, PeerState peerState) : pPeer(pPeer), peerState(peerState) { }

        ENetPeer* pPeer;

        PeerState peerState;
        dune::dune_clock::time_point timeout{};

        std::string name;
        std::list<ENetPeer*> notYetConnectedPeers;
    };

    ENetHost* host                      = nullptr;
    bool bIsServer                      = false;
    bool bLANServer                     = false;
    GameInitSettings* pGameInitSettings = nullptr;
    int numPlayers                      = 0;
    int maxPlayers                      = 0;

    std::string playerName;

    ENetPeer* connectPeer = nullptr;

    std::list<ENetPeer*> peerList;

    std::list<ENetPeer*> awaitingConnectionList;

    std::function<void(const std::string&, const std::string&)> pOnReceiveChatMessage;
    std::function<void(const GameInitSettings&, const ChangeEventList&)> pOnReceiveGameInfo;
    std::function<void(const ChangeEventList&)> pOnReceiveChangeEventList;
    std::function<void(const std::string&, bool, int)> pOnPeerDisconnected;
    std::function<ChangeEventList(const std::string&)> pGetChangeEventListForNewPlayerCallback;
    std::function<void(dune::dune_clock::duration)> pOnStartGame;
    std::function<void(const std::string&, const CommandList&)> pOnReceiveCommandList;
    std::function<void(const std::string&, const Dune::selected_set_type&, int)> pOnReceiveSelectionList;

    std::unique_ptr<LANGameFinderAndAnnouncer> pLANGameFinderAndAnnouncer = nullptr;
    std::unique_ptr<MetaServerClient> pMetaServerClient                   = nullptr;
};

#endif // NETWORKMANAGER_H
