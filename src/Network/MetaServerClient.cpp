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

#include <config.h>

#include <sstream>
#include <iostream>
#include <map>
#include <stdexcept>


MetaServerClient::MetaServerClient(std::string metaServerURL)
 : metaServerURL(metaServerURL) {

    availableMetaServerCommandsSemaphore = SDL_CreateSemaphore(0);
    if(availableMetaServerCommandsSemaphore == nullptr) {
        throw std::runtime_error("Unable to create semaphore");
    }

    sharedDataMutex = SDL_CreateMutex();
    if(sharedDataMutex == nullptr) {
        throw std::runtime_error("Unable to create mutex");
    }

    connectionThread = SDL_CreateThread(connectionThreadMain, nullptr, (void*) this);
    if(connectionThread == nullptr) {
        throw std::runtime_error("Unable to create thread");
    }
}


MetaServerClient::~MetaServerClient() {

    stopAnnounce();

    enqueueMetaServerCommand(std::shared_ptr<MetaServerCommand>(new MetaServerExit()));

    SDL_WaitThread(connectionThread, nullptr);

    SDL_DestroyMutex(sharedDataMutex);

    SDL_DestroySemaphore(availableMetaServerCommandsSemaphore);
}


void MetaServerClient::startAnnounce(std::string serverName, int serverPort, std::string mapName, Uint8 numPlayers, Uint8 maxPlayers) {

    stopAnnounce();

    this->serverName = serverName;
    this->serverPort = serverPort;
    this->secret = stringify<int>(rand()) + stringify<int>(rand());
    this->mapName = mapName;
    this->numPlayers = numPlayers;
    this->maxPlayers = maxPlayers;

    enqueueMetaServerCommand(std::shared_ptr<MetaServerCommand>(new MetaServerAdd(serverName, serverPort, secret, mapName, numPlayers, maxPlayers)));
    lastAnnounceUpdate = SDL_GetTicks();
}


void MetaServerClient::updateAnnounce(Uint8 numPlayers) {
    if(serverPort > 0) {
        this->numPlayers = numPlayers;
        enqueueMetaServerCommand(std::shared_ptr<MetaServerCommand>(new MetaServerUpdate(serverName, serverPort, secret, mapName, numPlayers, maxPlayers)));
        lastAnnounceUpdate = SDL_GetTicks();
    }
}


void MetaServerClient::stopAnnounce() {
    if(serverPort != 0) {

        enqueueMetaServerCommand(std::shared_ptr<MetaServerCommand>(new MetaServerRemove(serverPort, secret)));

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
            enqueueMetaServerCommand(std::shared_ptr<MetaServerCommand>(new MetaServerList()));
            lastServerInfoListUpdate = SDL_GetTicks();
        }

    }

    if(serverPort != 0) {
        if(SDL_GetTicks() - lastAnnounceUpdate > GAMESERVER_UPDATE_INTERVAL) {
            enqueueMetaServerCommand(std::shared_ptr<MetaServerCommand>(new MetaServerUpdate(serverName, serverPort, secret, mapName, numPlayers, maxPlayers)));
            lastAnnounceUpdate = SDL_GetTicks();
        }
    }


    int errorCause = 0;
    std::string errorMsg;
    bool bTmpUpdatedGameServerInfoList = false;
    std::list<GameServerInfo> tmpGameServerInfoList;

    SDL_LockMutex(sharedDataMutex);

    if(metaserverErrorCause != 0) {
        errorCause = metaserverErrorCause;
        errorMsg = metaserverError;
        metaserverErrorCause = 0;
        metaserverError = "";
    }

    if(bUpdatedGameServerInfoList == true) {
        tmpGameServerInfoList = gameServerInfoList;
        bTmpUpdatedGameServerInfoList = true;
        bUpdatedGameServerInfoList = false;
    }

    SDL_UnlockMutex(sharedDataMutex);

    if(pOnMetaServerError && (errorCause != 0)) {
        pOnMetaServerError(errorCause, errorMsg);
    }

    if(pOnGameServerInfoList && bTmpUpdatedGameServerInfoList) {
        pOnGameServerInfoList(tmpGameServerInfoList);
    }
}


void MetaServerClient::enqueueMetaServerCommand(std::shared_ptr<MetaServerCommand> metaServerCommand) {

    SDL_LockMutex(sharedDataMutex);

    bool bInsert = true;

    std::list<std::shared_ptr<MetaServerCommand> >::const_iterator iter;
    for(iter = metaServerCommandList.begin(); iter != metaServerCommandList.end(); ++iter) {
        if(**iter == *metaServerCommand) {
            bInsert = false;
            break;
        }
    }

    if(bInsert == true) {
        metaServerCommandList.push_back(metaServerCommand);
    }

    SDL_UnlockMutex(sharedDataMutex);

    if(bInsert == true) {
        SDL_SemPost(availableMetaServerCommandsSemaphore);
    }
}


std::shared_ptr<MetaServerCommand> MetaServerClient::dequeueMetaServerCommand() {

    while(SDL_SemWait(availableMetaServerCommandsSemaphore) != 0) {
        ;   // try again in case of error
    }

    SDL_LockMutex(sharedDataMutex);

    std::shared_ptr<MetaServerCommand> nextMetaServerCommand = metaServerCommandList.front();
    metaServerCommandList.pop_front();

    SDL_UnlockMutex(sharedDataMutex);

    return nextMetaServerCommand;
}


void MetaServerClient::setErrorMessage(int errorCause, std::string errorMessage) {
    SDL_LockMutex(sharedDataMutex);

    if(metaserverErrorCause == 0) {
        metaserverErrorCause = errorCause;
        this->metaserverError = errorMessage;
    }

    SDL_UnlockMutex(sharedDataMutex);
}


void MetaServerClient::setNewGameServerInfoList(std::list<GameServerInfo>& newGameServerInfoList) {
    SDL_LockMutex(sharedDataMutex);

    gameServerInfoList = newGameServerInfoList;
    bUpdatedGameServerInfoList = true;

    SDL_UnlockMutex(sharedDataMutex);
}


int MetaServerClient::connectionThreadMain(void* data) {
    MetaServerClient* pMetaServerClient = static_cast<MetaServerClient*>(data);

    while(true) {
        std::shared_ptr<MetaServerCommand> nextMetaServerCommand = pMetaServerClient->dequeueMetaServerCommand();

        switch(nextMetaServerCommand->type) {

            case METASERVERCOMMAND_ADD: {

                std::shared_ptr<MetaServerAdd> pMetaServerAdd = std::dynamic_pointer_cast<MetaServerAdd>(nextMetaServerCommand);

                std::map<std::string, std::string> parameters;

                parameters["command"] = "add";
                parameters["port"] = stringify<int>(pMetaServerAdd->serverPort);
                parameters["secret"] = pMetaServerAdd->secret;
                parameters["gamename"] = pMetaServerAdd->serverName;
                parameters["gameversion"] = VERSION;
                parameters["mapname"] = pMetaServerAdd->mapName;
                parameters["numplayers"] = stringify<int>(pMetaServerAdd->numPlayers);
                parameters["maxplayers"] = stringify<int>(pMetaServerAdd->maxPlayers);
                parameters["pwdprotected"] = "false";

                std::string result;

                try {
                    result = loadFromHttp(pMetaServerClient->metaServerURL, parameters);
                } catch(std::exception& e) {
                    pMetaServerClient->setErrorMessage(METASERVERCOMMAND_ADD, e.what());
                    break;
                }

                if(result.substr(0,2) != "OK") {
                    std::string errorMsg = result.substr(result.find_first_not_of("\x0D\x0A",5), std::string::npos);

                    pMetaServerClient->setErrorMessage(METASERVERCOMMAND_ADD, errorMsg);
                }


            } break;

            case METASERVERCOMMAND_UPDATE: {
                std::shared_ptr<MetaServerUpdate> pMetaServerUpdate = std::dynamic_pointer_cast<MetaServerUpdate>(nextMetaServerCommand);

                std::map<std::string, std::string> parameters;

                parameters["command"] = "update";
                parameters["port"] = stringify<int>(pMetaServerUpdate->serverPort);
                parameters["secret"] = pMetaServerUpdate->secret;
                parameters["numplayers"] = stringify<int>(pMetaServerUpdate->numPlayers);

                std::string result1;

                try {
                    result1 = loadFromHttp(pMetaServerClient->metaServerURL, parameters);
                } catch(std::exception& e) {
                    pMetaServerClient->setErrorMessage(METASERVERCOMMAND_UPDATE, e.what());
                    break;
                }


                if(result1.substr(0,2) != "OK") {
                    // try adding the game again

                    parameters["command"] = "add";
                    parameters["gamename"] = pMetaServerUpdate->serverName;
                    parameters["gameversion"] = VERSION;
                    parameters["mapname"] = pMetaServerUpdate->mapName;
                    parameters["maxplayers"] = stringify<int>(pMetaServerUpdate->maxPlayers);
                    parameters["pwdprotected"] = "false";

                    std::string result2;

                    try {
                        result2 = loadFromHttp(pMetaServerClient->metaServerURL, parameters);
                    } catch(std::exception&) {
                        // adding the game again did not work => report updating error

                        std::string errorMsg = result1.substr(result1.find_first_not_of("\x0D\x0A",5), std::string::npos);

                        pMetaServerClient->setErrorMessage(METASERVERCOMMAND_UPDATE, errorMsg);

                        break;
                    }

                    if(result2.substr(0,2) != "OK") {
                        // adding the game again did not work => report updating error

                        std::string errorMsg = result1.substr(result1.find_first_not_of("\x0D\x0A",5), std::string::npos);

                        pMetaServerClient->setErrorMessage(METASERVERCOMMAND_UPDATE, errorMsg);
                    }
                }

            } break;

            case METASERVERCOMMAND_REMOVE: {
                std::shared_ptr<MetaServerRemove> pMetaServerRemove = std::dynamic_pointer_cast<MetaServerRemove>(nextMetaServerCommand);

                std::map<std::string, std::string> parameters;

                parameters["command"] = "remove";
                parameters["port"] = stringify<int>(pMetaServerRemove->serverPort);
                parameters["secret"] = pMetaServerRemove->secret;

                loadFromHttp(pMetaServerClient->metaServerURL, parameters);
            } break;

            case METASERVERCOMMAND_LIST: {
                std::map<std::string, std::string> parameters;

                parameters["command"] = "list";
                parameters["gameversion"] = VERSION;

                std::string result;

                try {
                    result = loadFromHttp(pMetaServerClient->metaServerURL, parameters);
                } catch(std::exception& e) {
                    pMetaServerClient->setErrorMessage(METASERVERCOMMAND_LIST, e.what());
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

                        std::vector<std::string> parts = splitString(completeLine, std::string("\t"));

                        if(parts.size() != 9) {
                            break;
                        }

                        GameServerInfo gameServerInfo;

                        enet_address_set_host(&gameServerInfo.serverAddress, parts[0].c_str());

                        int port;
                        if(!parseString(parts[1], port) || port <= 0 || port > 65535) {
                            break;
                        }

                        gameServerInfo.serverAddress.port = (Uint16) port;

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

                        if(resultstream.good() == false) {
                            break;
                        }

                        newGameServerInfoList.push_back(gameServerInfo);

                    }

                    pMetaServerClient->setNewGameServerInfoList(newGameServerInfoList);

                } else {
                    std::string errorMsg = result.substr(result.find_first_not_of("\x0D\x0A",5), std::string::npos);

                    pMetaServerClient->setErrorMessage(METASERVERCOMMAND_LIST, errorMsg);
                }

            } break;

            case METASERVERCOMMAND_EXIT: {
                return 0;
            } break;

            default: {
                // ignore
            } break;

        }
    }
}

