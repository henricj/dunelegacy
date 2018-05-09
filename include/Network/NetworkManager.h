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

#include <Network/ENetPacketIStream.h>
#include <Network/ENetPacketOStream.h>
#include <Network/ChangeEventList.h>
#include <Network/CommandList.h>

#include <Network/LANGameFinderAndAnnouncer.h>
#include <Network/MetaServerClient.h>

#include <misc/string_util.h>
#include <misc/SDL2pp.h>

#include <enet/enet.h>
#include <string>
#include <list>
#include <functional>
#include <stdarg.h>

#define NETWORKDISCONNECT_QUIT              1
#define NETWORKDISCONNECT_TIMEOUT           2
#define NETWORKDISCONNECT_PLAYER_EXISTS     3
#define NETWORKDISCONNECT_GAME_FULL         4

#define NETWORKPACKET_UNKNOWN               0
#define NETWORKPACKET_CONNECT               1
#define NETWORKPACKET_DISCONNECT            2
#define NETWORKPACKET_PEER_CONNECTED        3
#define NETWORKPACKET_SENDGAMEINFO          4
#define NETWORKPACKET_SENDNAME              5
#define NETWORKPACKET_CHATMESSAGE           6
#define NETWORKPACKET_CHANGEEVENTLIST       7
#define NETWORKPACKET_STARTGAME             8
#define NETWORKPACKET_COMMANDLIST           9
#define NETWORKPACKET_SELECTIONLIST         10

#define AWAITING_CONNECTION_TIMEOUT     5000

class GameInitSettings;

class NetworkManager {
public:
    NetworkManager(int port, const std::string& metaserver);
    NetworkManager(const NetworkManager& o) = delete;
    ~NetworkManager();

    bool isServer() const { return bIsServer; };

    void startServer(bool bLANServer, const std::string& serverName, const std::string& playerName, GameInitSettings* pGameInitSettings, int numPlayers, int maxPlayers);
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

    void sendSelectedList(const std::set<Uint32>& selectedList, int groupListIndex = -1);

    std::list<std::string> getConnectedPeers() const {
        std::list<std::string> peerNameList;

        for(const ENetPeer* pPeer : peerList) {
            PeerData* peerData = static_cast<PeerData*>(pPeer->data);
            if(peerData != nullptr) {
                peerNameList.push_back(peerData->name);
            }
        }

        return peerNameList;
    }

    int getMaxPeerRoundTripTime();

    LANGameFinderAndAnnouncer* getLANGameFinderAndAnnouncer() {
        return pLANGameFinderAndAnnouncer.get();
    };

    MetaServerClient* getMetaServerClient() {
        return pMetaServerClient.get();
    };

    /**
        Sets the function that should be called when a chat message is received
        \param  pOnReceiveChatMessage   function to call on new chat message
    */
    inline void setOnReceiveChatMessage(std::function<void (const std::string&, const std::string&)> pOnReceiveChatMessage) {
        this->pOnReceiveChatMessage = pOnReceiveChatMessage;
    }

    /**
        Sets the function that should be called when game infos are received after connecting to the server.
        \param  pOnReceiveGameInfo  function to call on receive
    */
    inline void setOnReceiveGameInfo(std::function<void (const GameInitSettings&, const ChangeEventList&)> pOnReceiveGameInfo) {
        this->pOnReceiveGameInfo = pOnReceiveGameInfo;
    }


    /**
        Sets the function that should be called when a change event is received.
        \param  pOnReceiveChangeEventList   function to call on receive
    */
    inline void setOnReceiveChangeEventList(std::function<void (const ChangeEventList&)> pOnReceiveChangeEventList) {
        this->pOnReceiveChangeEventList = pOnReceiveChangeEventList;
    }


    /**
        Sets the function that should be called when a peer disconnects.
        \param  pOnPeerDisconnected function to call on disconnect
    */
    inline void setOnPeerDisconnected(std::function<void (const std::string&, bool, int)> pOnPeerDisconnected) {
        this->pOnPeerDisconnected = pOnPeerDisconnected;
    }

    /**
        Sets the function that can be used to retreive all house/player changes to get the current state
        \param  pGetGameInitSettingsCallback    function to call
    */
    inline void setGetChangeEventListForNewPlayerCallback(std::function<ChangeEventList (const std::string&)> pGetChangeEventListForNewPlayerCallback) {
        this->pGetChangeEventListForNewPlayerCallback = pGetChangeEventListForNewPlayerCallback;
    }

    /**
        Sets the function that should be called when the game is about to start and the time (in ms) left is received
        \param  pOnStartGame    function to call on receive
    */
    inline void setOnStartGame(std::function<void (unsigned int)> pOnStartGame) {
        this->pOnStartGame = pOnStartGame;
    }

    /**
        Sets the function that should be called when a command list is received.
        \param  pOnReceiveCommandList   function to call on receive
    */
    inline void setOnReceiveCommandList(std::function<void (const std::string&, const CommandList&)> pOnReceiveCommandList) {
        this->pOnReceiveCommandList = pOnReceiveCommandList;
    }

    /**
        Sets the function that should be called when a selection list is received.
        \param  pOnReceiveSelectionList function to call on receive
    */
    inline void setOnReceiveSelectionList(std::function<void (const std::string&, const std::set<Uint32>&, int)> pOnReceiveSelectionList) {
        this->pOnReceiveSelectionList = pOnReceiveSelectionList;
    }

private:
    static void debugNetwork(PRINTF_FORMAT_STRING const char* fmt, ...) PRINTF_VARARG_FUNC(1);

    void sendPacketToHost(ENetPacketOStream& packetStream, int channel = 0);

    void sendPacketToPeer(ENetPeer* peer, ENetPacketOStream& packetStream, int channel = 0);

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


        PeerData(ENetPeer* pPeer, PeerState peerState)
         : pPeer(pPeer), peerState(peerState), timeout(0)  {
        }


        ENetPeer*               pPeer;

        PeerState               peerState;
        Uint32                  timeout;

        std::string             name;
        std::list<ENetPeer*>    notYetConnectedPeers;
    };

    ENetHost* host = nullptr;
    bool bIsServer = false;
    bool bLANServer = false;
    GameInitSettings* pGameInitSettings = nullptr;
    int numPlayers = 0;
    int maxPlayers = 0;

    std::string playerName;

    ENetPeer*   connectPeer = nullptr;

    std::list<ENetPeer*> peerList;

    std::list<ENetPeer*> awaitingConnectionList;

    std::function<void (const std::string&, const std::string&)>            pOnReceiveChatMessage;
    std::function<void (const GameInitSettings&, const ChangeEventList&)>   pOnReceiveGameInfo;
    std::function<void (const ChangeEventList&)>                            pOnReceiveChangeEventList;
    std::function<void (const std::string&, bool, int)>                     pOnPeerDisconnected;
    std::function<ChangeEventList (const std::string&)>                     pGetChangeEventListForNewPlayerCallback;
    std::function<void (unsigned int)>                                      pOnStartGame;
    std::function<void (const std::string&, const CommandList&)>            pOnReceiveCommandList;
    std::function<void (const std::string&, const std::set<Uint32>&, int)>  pOnReceiveSelectionList;

    std::unique_ptr<LANGameFinderAndAnnouncer>  pLANGameFinderAndAnnouncer = nullptr;
    std::unique_ptr<MetaServerClient>           pMetaServerClient = nullptr;
};

#endif // NETWORKMANAGER_H
