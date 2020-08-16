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

#include <Network/MetaServerClient.h>

#include <Network/ENetHttp.h>

#include <misc/string_util.h>
#include <misc/exceptions.h>
#include <mmath.h>
#include <misc/Random.h>
#include <config.h>

#include <chrono>
#include <sstream>
#include <map>
#include <utility>

MetaServerClient::MetaServerClient(std::string_view metaServerURL)
 : metaServerURL(metaServerURL) {
 this->connectionThread = std::thread{[&] { connectionThreadMain(); }};
}


MetaServerClient::~MetaServerClient() {
    stopAnnounce();

    enqueueMetaServerCommand(std::make_unique<MetaServerExit>());

    connectionThread.join();
}


void MetaServerClient::startAnnounce(std::string_view serverName, int serverPort, std::string_view mapName, Uint8 numPlayers, Uint8 maxPlayers) {

    stopAnnounce();

    const auto rng_name =
        fmt::format("MetaServerClient {}:{} {} {}/{}", serverName, serverPort, mapName, numPlayers, maxPlayers);

    auto secret_key = RandomFactory::createRandomSeed(rng_name);

    if(secret_key.size() > 16) secret_key.resize(16);

    this->serverName = serverName;
    this->serverPort = serverPort;
    this->secret     = to_hex(secret_key);
    this->mapName    = mapName;
    this->numPlayers = numPlayers;
    this->maxPlayers = maxPlayers;

    enqueueMetaServerCommand(std::make_unique<MetaServerAdd>(serverName, serverPort, secret, mapName, numPlayers, maxPlayers));
    lastAnnounceUpdate = SDL_GetTicks();
}


void MetaServerClient::updateAnnounce(Uint8 numPlayers) {
    if(serverPort > 0) {
        this->numPlayers = numPlayers;
        enqueueMetaServerCommand(std::make_unique<MetaServerUpdate>(serverName, serverPort, secret, mapName, numPlayers, maxPlayers));
        lastAnnounceUpdate = SDL_GetTicks();
    }
}


void MetaServerClient::stopAnnounce() {
    if(serverPort != 0) {

        enqueueMetaServerCommand(std::make_unique<MetaServerRemove>(serverPort, secret));

        serverName = "";
        serverPort = 0;
        secret = "";
        mapName = "";
        numPlayers = 0;
        maxPlayers = 0;
    }
}


void MetaServerClient::update() {

    if(pOnGameServerInfoList) {
        // someone is waiting for the list

        if(SDL_GetTicks() - lastServerInfoListUpdate > SERVERLIST_UPDATE_INTERVAL) {
            enqueueMetaServerCommand(std::make_unique<MetaServerList>());
            lastServerInfoListUpdate = SDL_GetTicks();
        }

    }

    if(serverPort != 0) {
        if(SDL_GetTicks() - lastAnnounceUpdate > GAMESERVER_UPDATE_INTERVAL) {
            enqueueMetaServerCommand(std::make_unique<MetaServerUpdate>(serverName, serverPort, secret, mapName, numPlayers, maxPlayers));
            lastAnnounceUpdate = SDL_GetTicks();
        }
    }


    int errorCause = 0;
    std::string errorMsg;
    bool bTmpUpdatedGameServerInfoList = false;
    std::list<GameServerInfo> tmpGameServerInfoList;

    { // Scope
        std::unique_lock<std::mutex> lock{sharedDataMutex};

        if(metaserverErrorCause != 0) {
            errorCause           = metaserverErrorCause;
            errorMsg             = metaserverError;
            metaserverErrorCause = 0;
            metaserverError      = "";
        }

        if(bUpdatedGameServerInfoList) {
            tmpGameServerInfoList         = gameServerInfoList;
            bTmpUpdatedGameServerInfoList = true;
            bUpdatedGameServerInfoList    = false;
        }
    }

    if(pOnMetaServerError && (errorCause != 0)) {
        pOnMetaServerError(errorCause, errorMsg);
    }

    if(pOnGameServerInfoList && bTmpUpdatedGameServerInfoList) {
        pOnGameServerInfoList(tmpGameServerInfoList);
    }
}


void MetaServerClient::enqueueMetaServerCommand(std::unique_ptr<MetaServerCommand> metaServerCommand) {
    auto bInsert = true;

    { // Scope
        std::unique_lock<std::mutex> lock{sharedDataMutex};

        for(const auto& pMetaServerCommand : metaServerCommandList) {
            if(*pMetaServerCommand == *metaServerCommand) {
                bInsert = false;
                break;
            }
        }

        if(bInsert) { metaServerCommandList.push_back(std::move(metaServerCommand)); }
    }

    if(bInsert) {
        metaServerCommandListCv.notify_one();
    }
}


std::unique_ptr<MetaServerCommand> MetaServerClient::dequeueMetaServerCommand() {
    std::unique_lock<std::mutex> lock{sharedDataMutex};

    metaServerCommandListCv.wait(lock, [&] { return !metaServerCommandList.empty(); });

    auto nextMetaServerCommand = std::move(metaServerCommandList.front());
    metaServerCommandList.pop_front();

    return nextMetaServerCommand;
}


void MetaServerClient::setErrorMessage(int errorCause, const std::string& errorMessage) {
    std::unique_lock<std::mutex> lock{sharedDataMutex};

    if(metaserverErrorCause == 0) {
        metaserverErrorCause = errorCause;
        this->metaserverError = errorMessage;
    }
}


void MetaServerClient::setNewGameServerInfoList(const std::list<GameServerInfo>& newGameServerInfoList) {
    std::unique_lock<std::mutex> lock{sharedDataMutex};

    gameServerInfoList = newGameServerInfoList;
    bUpdatedGameServerInfoList = true;
}


int MetaServerClient::connectionThreadMain() {
    while(true) {
        try {
            auto nextMetaServerCommand = dequeueMetaServerCommand();

            switch(nextMetaServerCommand->type) {

                case METASERVERCOMMAND_ADD: {

                    auto* pMetaServerAdd = dynamic_cast<MetaServerAdd*>(nextMetaServerCommand.get());
                    if(!pMetaServerAdd) {
                        break;
                    }

                    std::map<std::string, std::string> parameters;

                    parameters["command"] = "add";
                    parameters["port"] = std::to_string(pMetaServerAdd->serverPort);
                    parameters["secret"] = pMetaServerAdd->secret;
                    parameters["gamename"] = pMetaServerAdd->serverName;
                    parameters["gameversion"] = VERSION;
                    parameters["mapname"] = pMetaServerAdd->mapName;
                    parameters["numplayers"] = std::to_string(pMetaServerAdd->numPlayers);
                    parameters["maxplayers"] = std::to_string(pMetaServerAdd->maxPlayers);
                    parameters["pwdprotected"] = "false";

                    std::string result;

                    try {
                        result = loadFromHttp(metaServerURL, parameters);
                    } catch(std::exception& e) {
                        setErrorMessage(METASERVERCOMMAND_ADD, e.what());
                        break;
                    }

                    if(result.substr(0,2) != "OK") {
                        const std::string errorMsg = result.substr(result.find_first_not_of("\x0D\x0A",5), std::string::npos);

                        setErrorMessage(METASERVERCOMMAND_ADD, errorMsg);
                    }


                } break;

                case METASERVERCOMMAND_UPDATE: {
                    auto* pMetaServerUpdate = dynamic_cast<MetaServerUpdate*>(nextMetaServerCommand.get());
                    if(!pMetaServerUpdate) {
                        break;
                    }

                    std::map<std::string, std::string> parameters{
                        {"command", "update"},
                        {"port", std::to_string(pMetaServerUpdate->serverPort)},
                        {"secret",pMetaServerUpdate->secret},
                        {"numplayers", std::to_string(pMetaServerUpdate->numPlayers)}
                    };


                    std::string result1;

                    try {
                        result1 = loadFromHttp(metaServerURL, parameters);
                    } catch(std::exception& e) {
                        setErrorMessage(METASERVERCOMMAND_UPDATE, e.what());
                        break;
                    }


                    if(result1.substr(0,2) != "OK") {
                        // try adding the game again

                        parameters["command"] = "add";
                        parameters["gamename"] = pMetaServerUpdate->serverName;
                        parameters["gameversion"] = VERSION;
                        parameters["mapname"] = pMetaServerUpdate->mapName;
                        parameters["maxplayers"] = std::to_string(pMetaServerUpdate->maxPlayers);
                        parameters["pwdprotected"] = "false";

                        std::string result2;

                        try {
                            result2 = loadFromHttp(metaServerURL, parameters);
                        } catch(std::exception&) {
                            // adding the game again did not work => report updating error

                            const std::string errorMsg = result1.substr(result1.find_first_not_of("\x0D\x0A",5), std::string::npos);

                            setErrorMessage(METASERVERCOMMAND_UPDATE, errorMsg);

                            break;
                        }

                        if(result2.substr(0,2) != "OK") {
                            // adding the game again did not work => report updating error

                            const std::string errorMsg = result1.substr(result1.find_first_not_of("\x0D\x0A",5), std::string::npos);

                            setErrorMessage(METASERVERCOMMAND_UPDATE, errorMsg);
                        }
                    }

                } break;

                case METASERVERCOMMAND_REMOVE: {
                    auto* pMetaServerRemove = dynamic_cast<MetaServerRemove*>(nextMetaServerCommand.get());
                    if(!pMetaServerRemove) {
                        break;
                    }

                    std::map<std::string, std::string> parameters;

                    parameters["command"] = "remove";
                    parameters["port"] = std::to_string(pMetaServerRemove->serverPort);
                    parameters["secret"] = pMetaServerRemove->secret;

                    try {
                        loadFromHttp(metaServerURL, parameters);
                    } catch(std::exception& e) {
                        setErrorMessage(METASERVERCOMMAND_REMOVE, e.what());
                        break;
                    }
                } break;

                case METASERVERCOMMAND_LIST: {
                    std::map<std::string, std::string> parameters;

                    parameters["command"] = "list";
                    parameters["gameversion"] = VERSION;

                    std::string result;

                    try {
                        result = loadFromHttp(metaServerURL, parameters);
                    } catch(std::exception& e) {
                        setErrorMessage(METASERVERCOMMAND_LIST, e.what());
                        break;
                    }

                    std::istringstream resultstream(result);

                    std::string status;

                    resultstream >> status;

                    if(status == "OK") {

                        // skip all newlines
                        resultstream >> std::ws;

                        std::list<GameServerInfo> newGameServerInfoList;

                        while(true) {

                            std::string completeLine;
                            getline(resultstream, completeLine);

                            std::vector<std::string> parts = splitStringToStringVector(completeLine, "\\t");

                            if(parts.size() != 9) {
                                break;
                            }

                            GameServerInfo gameServerInfo;

                            enet_address_set_host(&gameServerInfo.serverAddress, parts[0].c_str());

                            int port = 0;
                            if(!parseString(parts[1], port) || port <= 0 || port > 65535) {
                                break;
                            }

                            gameServerInfo.serverAddress.port = static_cast<Uint16>(port);

                            gameServerInfo.serverName = parts[2];
                            gameServerInfo.serverVersion = parts[3];
                            gameServerInfo.mapName = parts[4];
                            if(!parseString(parts[6], gameServerInfo.maxPlayers) || (gameServerInfo.maxPlayers < 1) || (gameServerInfo.maxPlayers > 12)) {
                                continue;
                            }

                            if(!parseString(parts[5], gameServerInfo.numPlayers) || (gameServerInfo.numPlayers < 0) || (gameServerInfo.numPlayers > gameServerInfo.maxPlayers)) {
                                continue;
                            }

                            gameServerInfo.bPasswordProtected = (parts[7] == "true");

                            if(!parseString(parts[8], gameServerInfo.lastUpdate)) {
                                continue;
                            }

                            if(!resultstream.good()) {
                                break;
                            }

                            newGameServerInfoList.push_back(gameServerInfo);

                        }

                        setNewGameServerInfoList(newGameServerInfoList);

                    } else {
                        const std::string errorMsg = result.substr(result.find_first_not_of("\x0D\x0A",5), std::string::npos);

                        setErrorMessage(METASERVERCOMMAND_LIST, errorMsg);
                    }

                } break;

                case METASERVERCOMMAND_EXIT: {
                    return 0;
                } break;

                default: {
                    // ignore
                } break;

            }
        } catch(std::exception& e) {
            sdl2::log_info("MetaServerClient::connectionThreadMain(): %s", e.what());
        }
    }
}

