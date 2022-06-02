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

#include "misc/Random.h"
#include <misc/exceptions.h>
#include <misc/string_util.h>

#include <mmath.h>

#include <config.h>

#include <iostream>
#include <map>
#include <sstream>
#include <utility>

MetaServerClient::MetaServerClient(std::string metaServerURL)
    : metaServerURL(std::move(metaServerURL)), availableMetaServerCommandsSemaphore_(SDL_CreateSemaphore(0)) {

    if (availableMetaServerCommandsSemaphore_ == nullptr) {
        THROW(std::runtime_error, "Unable to create semaphore");
    }

    sharedDataMutex_ = SDL_CreateMutex();
    if (sharedDataMutex_ == nullptr) {
        THROW(std::runtime_error, "Unable to create mutex");
    }

    connectionThread_ = SDL_CreateThread(connectionThreadMain, nullptr, this);
    if (connectionThread_ == nullptr) {
        THROW(std::runtime_error, "Unable to create thread");
    }
}

MetaServerClient::~MetaServerClient() {

    stopAnnounce();

    enqueueMetaServerCommand(std::make_unique<MetaServerExit>());

    SDL_WaitThread(connectionThread_, nullptr);

    SDL_DestroyMutex(sharedDataMutex_);

    SDL_DestroySemaphore(availableMetaServerCommandsSemaphore_);
}

void MetaServerClient::startAnnounce(std::string serverName, uint16_t serverPort, std::string mapName,
                                     uint8_t numPlayers, uint8_t maxPlayers) {

    stopAnnounce();

    const auto rng_name =
        fmt::format("MetaServerClient {}:{} {} {}/{}", serverName, serverPort, mapName, numPlayers, maxPlayers);

    auto secret_key = RandomFactory::createRandomSeed(rng_name);

    if (secret_key.size() > 16)
        secret_key.resize(16);

    serverName_ = std::move(serverName);
    serverPort_ = serverPort;
    secret_     = to_hex(secret_key);
    mapName_    = std::move(mapName);
    numPlayers_ = numPlayers;
    maxPlayers_ = maxPlayers;

    enqueueMetaServerCommand(
        std::make_unique<MetaServerAdd>(serverName_, serverPort_, secret_, mapName_, numPlayers_, maxPlayers_));
    lastAnnounceUpdate = dune::dune_clock::now();
}

void MetaServerClient::updateAnnounce(uint8_t numPlayers) {
    if (serverPort_ > 0) {
        this->numPlayers_ = numPlayers;
        enqueueMetaServerCommand(
            std::make_unique<MetaServerUpdate>(serverName_, serverPort_, secret_, mapName_, numPlayers, maxPlayers_));
        lastAnnounceUpdate = dune::dune_clock::now();
    }
}

void MetaServerClient::stopAnnounce() {
    if (serverPort_ != 0) {

        enqueueMetaServerCommand(std::make_unique<MetaServerRemove>(serverPort_, secret_));

        serverName_ = "";
        serverPort_ = 0;
        secret_     = "";
        mapName_    = "";
        numPlayers_ = 0;
        maxPlayers_ = 0;
    }
}

void MetaServerClient::update() {

    if (pOnGameServerInfoList_) {
        // someone is waiting for the list

        if (dune::dune_clock::now() - lastServerInfoListUpdate_ > SERVERLIST_UPDATE_INTERVAL) {
            enqueueMetaServerCommand(std::make_unique<MetaServerList>());
            lastServerInfoListUpdate_ = dune::dune_clock::now();
        }
    }

    if (serverPort_ != 0) {
        if (dune::dune_clock::now() - lastAnnounceUpdate > GAMESERVER_UPDATE_INTERVAL) {
            enqueueMetaServerCommand(std::make_unique<MetaServerUpdate>(serverName_, serverPort_, secret_, mapName_,
                                                                        numPlayers_, maxPlayers_));
            lastAnnounceUpdate = dune::dune_clock::now();
        }
    }

    int errorCause = 0;
    std::string errorMsg;
    bool bTmpUpdatedGameServerInfoList = false;
    std::list<GameServerInfo> tmpGameServerInfoList;

    SDL_LockMutex(sharedDataMutex_);

    if (metaserverErrorCause_ != 0) {
        errorCause            = metaserverErrorCause_;
        errorMsg              = metaserverError_;
        metaserverErrorCause_ = 0;
        metaserverError_      = "";
    }

    if (bUpdatedGameServerInfoList_) {
        tmpGameServerInfoList         = gameServerInfoList_;
        bTmpUpdatedGameServerInfoList = true;
        bUpdatedGameServerInfoList_   = false;
    }

    SDL_UnlockMutex(sharedDataMutex_);

    if (pOnMetaServerError_ && (errorCause != 0)) {
        pOnMetaServerError_(errorCause, errorMsg);
    }

    if (pOnGameServerInfoList_ && bTmpUpdatedGameServerInfoList) {
        pOnGameServerInfoList_(tmpGameServerInfoList);
    }
}

void MetaServerClient::enqueueMetaServerCommand(std::unique_ptr<MetaServerCommand> metaServerCommand) {

    SDL_LockMutex(sharedDataMutex_);

    bool bInsert = true;

    for (const auto& pMetaServerCommand : metaServerCommandList_) {
        if (*pMetaServerCommand == *metaServerCommand) {
            bInsert = false;
            break;
        }
    }

    if (bInsert) {
        metaServerCommandList_.push_back(std::move(metaServerCommand));
    }

    SDL_UnlockMutex(sharedDataMutex_);

    if (bInsert) {
        SDL_SemPost(availableMetaServerCommandsSemaphore_);
    }
}

std::unique_ptr<MetaServerCommand> MetaServerClient::dequeueMetaServerCommand() {

    while (SDL_SemWait(availableMetaServerCommandsSemaphore_) != 0) { }

    SDL_LockMutex(sharedDataMutex_);

    std::unique_ptr<MetaServerCommand> nextMetaServerCommand = std::move(metaServerCommandList_.front());
    metaServerCommandList_.pop_front();

    SDL_UnlockMutex(sharedDataMutex_);

    return nextMetaServerCommand;
}

void MetaServerClient::setErrorMessage(int errorCause, const std::string& errorMessage) {
    SDL_LockMutex(sharedDataMutex_);

    if (metaserverErrorCause_ == 0) {
        metaserverErrorCause_  = errorCause;
        this->metaserverError_ = errorMessage;
    }

    SDL_UnlockMutex(sharedDataMutex_);
}

void MetaServerClient::setNewGameServerInfoList(const std::list<GameServerInfo>& newGameServerInfoList) {
    SDL_LockMutex(sharedDataMutex_);

    gameServerInfoList_         = newGameServerInfoList;
    bUpdatedGameServerInfoList_ = true;

    SDL_UnlockMutex(sharedDataMutex_);
}

int MetaServerClient::connectionThreadMain(void* data) {
    auto* pMetaServerClient = static_cast<MetaServerClient*>(data);

    while (true) {
        try {
            std::unique_ptr<MetaServerCommand> nextMetaServerCommand = pMetaServerClient->dequeueMetaServerCommand();

            switch (nextMetaServerCommand->type) {

                case METASERVERCOMMAND_ADD: {

                    auto* pMetaServerAdd = dynamic_cast<MetaServerAdd*>(nextMetaServerCommand.get());
                    if (!pMetaServerAdd) {
                        break;
                    }

                    std::map<std::string, std::string> parameters;

                    parameters["command"]      = "add";
                    parameters["port"]         = std::to_string(pMetaServerAdd->serverPort);
                    parameters["secret"]       = pMetaServerAdd->secret;
                    parameters["gamename"]     = pMetaServerAdd->serverName;
                    parameters["gameversion"]  = VERSION;
                    parameters["mapname"]      = pMetaServerAdd->mapName;
                    parameters["numplayers"]   = std::to_string(pMetaServerAdd->numPlayers);
                    parameters["maxplayers"]   = std::to_string(pMetaServerAdd->maxPlayers);
                    parameters["pwdprotected"] = "false";

                    std::string result;

                    try {
                        result = loadFromHttp(pMetaServerClient->metaServerURL, parameters);
                    } catch (std::exception& e) {
                        pMetaServerClient->setErrorMessage(METASERVERCOMMAND_ADD, e.what());
                        break;
                    }

                    if (result.substr(0, 2) != "OK") {
                        const std::string errorMsg =
                            result.substr(result.find_first_not_of("\x0D\x0A", 5), std::string::npos);

                        pMetaServerClient->setErrorMessage(METASERVERCOMMAND_ADD, errorMsg);
                    }

                } break;

                case METASERVERCOMMAND_UPDATE: {
                    auto* pMetaServerUpdate = dynamic_cast<MetaServerUpdate*>(nextMetaServerCommand.get());
                    if (!pMetaServerUpdate) {
                        break;
                    }

                    std::map<std::string, std::string> parameters;

                    parameters["command"]    = "update";
                    parameters["port"]       = std::to_string(pMetaServerUpdate->serverPort);
                    parameters["secret"]     = pMetaServerUpdate->secret;
                    parameters["numplayers"] = std::to_string(pMetaServerUpdate->numPlayers);

                    std::string result1;

                    try {
                        result1 = loadFromHttp(pMetaServerClient->metaServerURL, parameters);
                    } catch (std::exception& e) {
                        pMetaServerClient->setErrorMessage(METASERVERCOMMAND_UPDATE, e.what());
                        break;
                    }

                    if (result1.substr(0, 2) != "OK") {
                        // try adding the game again

                        parameters["command"]      = "add";
                        parameters["gamename"]     = pMetaServerUpdate->serverName;
                        parameters["gameversion"]  = VERSION;
                        parameters["mapname"]      = pMetaServerUpdate->mapName;
                        parameters["maxplayers"]   = std::to_string(pMetaServerUpdate->maxPlayers);
                        parameters["pwdprotected"] = "false";

                        std::string result2;

                        try {
                            result2 = loadFromHttp(pMetaServerClient->metaServerURL, parameters);
                        } catch (std::exception&) {
                            // adding the game again did not work => report updating error

                            const std::string errorMsg =
                                result1.substr(result1.find_first_not_of("\x0D\x0A", 5), std::string::npos);

                            pMetaServerClient->setErrorMessage(METASERVERCOMMAND_UPDATE, errorMsg);

                            break;
                        }

                        if (result2.substr(0, 2) != "OK") {
                            // adding the game again did not work => report updating error

                            const std::string errorMsg =
                                result1.substr(result1.find_first_not_of("\x0D\x0A", 5), std::string::npos);

                            pMetaServerClient->setErrorMessage(METASERVERCOMMAND_UPDATE, errorMsg);
                        }
                    }

                } break;

                case METASERVERCOMMAND_REMOVE: {
                    auto* pMetaServerRemove = dynamic_cast<MetaServerRemove*>(nextMetaServerCommand.get());
                    if (!pMetaServerRemove) {
                        break;
                    }

                    std::map<std::string, std::string> parameters;

                    parameters["command"] = "remove";
                    parameters["port"]    = std::to_string(pMetaServerRemove->serverPort);
                    parameters["secret"]  = pMetaServerRemove->secret;

                    try {
                        loadFromHttp(pMetaServerClient->metaServerURL, parameters);
                    } catch (std::exception& e) {
                        pMetaServerClient->setErrorMessage(METASERVERCOMMAND_REMOVE, e.what());
                        break;
                    }
                } break;

                case METASERVERCOMMAND_LIST: {
                    std::map<std::string, std::string> parameters;

                    parameters["command"]     = "list";
                    parameters["gameversion"] = VERSION;

                    std::string result;

                    try {
                        result = loadFromHttp(pMetaServerClient->metaServerURL, parameters);
                    } catch (std::exception& e) {
                        pMetaServerClient->setErrorMessage(METASERVERCOMMAND_LIST, e.what());
                        break;
                    }

                    std::istringstream resultstream(result);

                    std::string status;

                    resultstream >> status;

                    if (status == "OK") {

                        // skip all newlines
                        resultstream >> std::ws;

                        std::list<GameServerInfo> newGameServerInfoList;

                        while (true) {

                            std::string completeLine;
                            getline(resultstream, completeLine);

                            std::vector<std::string> parts = splitStringToStringVector(completeLine, "\\t");

                            if (parts.size() != 9) {
                                break;
                            }

                            GameServerInfo gameServerInfo;

                            enet_address_set_host(&gameServerInfo.serverAddress, parts[0].c_str());

                            int port = 0;
                            if (!parseString(parts[1], port) || port <= 0 || port > 65535) {
                                break;
                            }

                            gameServerInfo.serverAddress.port = static_cast<uint16_t>(port);

                            gameServerInfo.serverName    = parts[2];
                            gameServerInfo.serverVersion = parts[3];
                            gameServerInfo.mapName       = parts[4];
                            if (!parseString(parts[6], gameServerInfo.maxPlayers) || (gameServerInfo.maxPlayers < 1)
                                || (gameServerInfo.maxPlayers > 12)) {
                                continue;
                            }

                            if (!parseString(parts[5], gameServerInfo.numPlayers) || (gameServerInfo.numPlayers < 0)
                                || (gameServerInfo.numPlayers > gameServerInfo.maxPlayers)) {
                                continue;
                            }

                            gameServerInfo.bPasswordProtected = (parts[7] == "true");

                            if (!parseString(parts[8], gameServerInfo.lastUpdate)) {
                                continue;
                            }

                            if (!resultstream.good()) {
                                break;
                            }

                            newGameServerInfoList.push_back(gameServerInfo);
                        }

                        pMetaServerClient->setNewGameServerInfoList(newGameServerInfoList);

                    } else {
                        const std::string errorMsg =
                            result.substr(result.find_first_not_of("\x0D\x0A", 5), std::string::npos);

                        pMetaServerClient->setErrorMessage(METASERVERCOMMAND_LIST, errorMsg);
                    }

                } break;

                case METASERVERCOMMAND_EXIT: {
                    return 0;
                }

                default: {
                    // ignore
                } break;
            }
        } catch (std::exception& e) {
            sdl2::log_info("MetaServerClient::connectionThreadMain(): %s", e.what());
        }
    }
}
