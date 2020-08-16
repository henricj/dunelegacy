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

#include <Game.h>

#include <engine_sand.h>

#include <config.h>
#include <main.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/TextManager.h>
#include <misc/IFileStream.h>
#include <misc/OFileStream.h>
#include <misc/IMemoryStream.h>
#include <misc/FileSystem.h>
#include <misc/fnkdat.h>
#include <misc/md5.h>
#include <misc/exceptions.h>
#include <fmt/format.h>

#include <players/HumanPlayer.h>

#include <Network/NetworkManager.h>

#include <House.h>
#include <Map.h>
#include <Bullet.h>
#include <Explosion.h>
#include <GameInitSettings.h>

#include <structures/StructureBase.h>
#include <structures/ConstructionYard.h>
#include <units/UnitBase.h>
#include <structures/BuilderBase.h>
#include <structures/Palace.h>
#include <units/Harvester.h>
#include <units/InfantryBase.h>

#include <algorithm>

namespace Dune::Engine {

Game::Game() {
    localPlayerName = settings.general.playerName;

    unitList.clear();      // holds all the units
    structureList.clear(); // all the structures
    bulletList.clear();

    // set to true for now
    debug = false;
}

/**
    The destructor frees up all the used memory.
*/
Game::~Game() {
    if(pNetworkManager != nullptr) {
        pNetworkManager->setOnReceiveChatMessage([](const auto&, const auto&) {});
        pNetworkManager->setOnReceiveCommandList([](const auto&, const auto&) {});
        pNetworkManager->setOnReceiveSelectionList([](const auto&, const auto&, auto) {});
        pNetworkManager->setOnPeerDisconnected([](const auto&, auto, auto) {});
    }

    structureList.clear();

    unitList.clear();

    bulletList.clear();

    explosionList.clear();

    currentGameMap = nullptr;
    map.reset();

    screenborder.reset();
}

void Game::initGame(const GameInitSettings& newGameInitSettings) {
    gameInitSettings = newGameInitSettings;

    switch(gameInitSettings.getGameType()) {
        case GameType::LoadSavegame: {
            if(!loadSaveGame(gameInitSettings.getFilename())) {
                THROW(std::runtime_error, "Loading save game failed!");
            }
        } break;

        case GameType::LoadMultiplayer: {
            IMemoryStream memStream(gameInitSettings.getFiledata().c_str(), gameInitSettings.getFiledata().size());

            if(!loadSaveGame(memStream)) { THROW(std::runtime_error, "Loading save game failed!"); }
        } break;

        case GameType::Campaign:
        case GameType::Skirmish:
        case GameType::CustomGame:
        case GameType::CustomMultiplayer: {
            gameType = gameInitSettings.getGameType();
            randomFactory.setSeed({gameInitSettings.getRandomSeed()});

            randomGen = randomFactory.create("Game");

            objectData.loadFromINIFile("ObjectData.ini");

            if(gameInitSettings.getMission() != 0) { techLevel = ((gameInitSettings.getMission() + 1) / 3) + 1; }

            INIMapLoader loader{this, gameInitSettings.getFilename(), gameInitSettings.getFiledata()};

            map            = loader.load();
            currentGameMap = map.get();

            if(!bReplay && gameInitSettings.getGameType() != GameType::CustomGame &&
               gameInitSettings.getGameType() != GameType::CustomMultiplayer) {
                /* do briefing */
                sdl2::log_info("Briefing...");
                BriefingMenu(gameInitSettings.getHouseID(), gameInitSettings.getMission(), BRIEFING).showMenu();
            }
        } break;

        default: {
        } break;
    }
}

void Game::initReplay(const std::filesystem::path& filename) {
    bReplay = true;

    IFileStream fs;

    if(!fs.open(filename)) { THROW(io_error, "Error while opening '%s'!", filename); }

    // override local player name as it was when the replay was created
    localPlayerName = fs.readString();

    // read GameInitInfo
    const GameInitSettings loadedGameInitSettings(fs);

    // load all commands
    cmdManager.load(fs);

    initGame(loadedGameInitSettings);
}

void Game::processObjects() {
    // update all tiles
    map->for_all([](Tile& t) { t.update(); });

    const GameContext context{*this, *currentGameMap, objectManager};

    for(auto* pStructure : structureList) {
        pStructure->update(context);
    }

    if((currentCursorMode == CursorMode_Placing) && selectedList.empty()) { currentCursorMode = CursorMode_Normal; }

    for(auto* pUnit : unitList) {
        pUnit->update(context);
    }

    auto selection_changed = false;

    map->consume_removed_objects([&](Uint32 objectID) {
        auto* object = objectManager.getObject(objectID);

        if(!object) return;

        if(removeFromSelectionLists(object)) selection_changed = true;
    });

    objectManager.consume_pending_deletes([&](auto& object) {
        object->cleanup(context, pLocalPlayer);

        if(removeFromSelectionLists(object.get())) selection_changed = true;

        removeFromQuickSelectionLists(object->getObjectID());
    });

    if(selection_changed) selectionChanged();

    bulletList.erase(
        std::remove_if(std::begin(bulletList), std::end(bulletList), [&](auto& b) { return b->update(context); }),
        std::end(bulletList));

    explosionList.erase(
        std::remove_if(std::begin(explosionList), std::end(explosionList), [](auto& e) { return e->update(); }),
        std::end(explosionList));
}

void Game::serviceNetwork(bool& bWaitForNetwork) {
    pNetworkManager->update();

    // test if we need to wait for data to arrive
    for(const auto& playername : pNetworkManager->getConnectedPeers()) {
        auto* const pPlayer = dynamic_cast<HumanPlayer*>(getPlayerByName(playername));
        if(pPlayer != nullptr) {
            if(pPlayer->nextExpectedCommandsCycle <= gameCycleCount) {
                // sdl2::log_info("Cycle %d: Waiting for player '%s' to send data for cycle %d...", GameCycleCount,
                // pPlayer->getPlayername().c_str(), pPlayer->nextExpectedCommandsCycle);
                bWaitForNetwork = true;
                break;
            }
        }
    }

    if(bWaitForNetwork) {
        if(startWaitingForOtherPlayersTime == 0) {
            // we just started waiting
            startWaitingForOtherPlayersTime = SDL_GetTicks();
        } else {
            if(SDL_GetTicks() - startWaitingForOtherPlayersTime > 1000) {
                // we waited for more than one second

                if(pWaitingForOtherPlayers == nullptr) {
                    pWaitingForOtherPlayers = std::make_unique<WaitingForOtherPlayers>();
                    bMenu                   = true;
                }
            }
        }

        SDL_Delay(10);
    } else {
        startWaitingForOtherPlayersTime = 0;
        pWaitingForOtherPlayers.reset();
    }
}

void Game::updateGame(const GameContext& context) {
    pInterface->getRadarView().update();
    cmdManager.executeCommands(context, gameCycleCount);

    // sdl2::log_info("cycle %d : %d", gameCycleCount, context.game.randomGen.getSeed());

#ifdef TEST_SYNC
    // add every gamecycles one test sync command
    if(bReplay == false) {
        cmdManager.addCommand(Command(pLocalPlayer->getPlayerID(), CMD_TEST_SYNC, randomGen.getSeed()));
    }
#endif

    std::for_each(house.begin(), house.end(), [](auto& h) {
        if(h) h->update();
    });

    screenborder->update(pGFXManager->random());

    triggerManager.trigger(context, gameCycleCount);

    processObjects();

    if((indicatorFrame != NONE_ID) && (--indicatorTimer <= 0)) {
        indicatorTimer = indicatorTime;

        if(++indicatorFrame > 2) { indicatorFrame = NONE_ID; }
    }

    gameCycleCount++;
}

void Game::doEventsUntil(const GameContext& context, const int until) {
    SDL_Event event;
    // valgrind reports errors in SDL_PollEvent if event is not initialized
    memset(&event, 0, sizeof(event));

    while(!bQuitGame && !finishedLevel) {
        const auto remaining = until - static_cast<int>(SDL_GetTicks());

        if(remaining <= 0 || remaining >= 100) return;

        if(SDL_WaitEventTimeout(&event, remaining)) {
            doInput(context, event);

            while(SDL_PollEvent(&event)) {
                doInput(context, event);
            }
        }
    }
}

void Game::runMainLoop(const GameContext& context) {
    sdl2::log_info("Starting game...");

    // add interface
    if(pInterface == nullptr) {
        pInterface = std::make_unique<GameInterface>(context);
        if(gameState == GameState::Loading) {
            // when loading a save game we set radar directly
            pInterface->getRadarView().setRadarMode(pLocalHouse->hasRadarOn());
        } else if(pLocalHouse->hasRadarOn()) {
            // when starting a new game we switch the radar on with an animation if appropriate
            pInterface->getRadarView().switchRadarMode(true);
        }
    }

    sdl2::log_info("Sizes: Tile %d UnitBase %d StructureBase %d Harvester %d ConstructionYard %d Palace %d",
                   sizeof(Tile), sizeof(UnitBase), sizeof(StructureBase), sizeof(Harvester), sizeof(ConstructionYard),
                   sizeof(Palace));

    gameState = GameState::Running;

    // setup endlevel conditions
    finishedLevel = false;

    bShowTime = winFlags & WINLOSEFLAGS_TIMEOUT;

    // Check if a player has lost
    std::for_each(house.begin(), house.end(), [](auto& h) {
        if(h && !h->isAlive()) h->lose(true);
    });

    if(bReplay) {
        cmdManager.setReadOnly(true);
    } else {
        auto pStream = std::make_unique<OFileStream>();

        const auto [ok, replayname] = fnkdat("replay/auto.rpl", FNKDAT_USER | FNKDAT_CREAT);

        auto isOpen = ok && pStream->open(replayname);

        if(!isOpen) {
            const std::error_code replay_error{errno, std::generic_category()};

            sdl2::log_error(
                SDL_LOG_CATEGORY_APPLICATION,
                fmt::format("Unable to open the default replay file: {}  Retrying...", replay_error.message()).c_str());

            auto& uiRandom = pGFXManager->random();

            for(auto i = 0; i < 10; ++i) {
                const auto [ok2, replayname2] =
                    fnkdat(fmt::format("replay/auto-{}.rpl", uiRandom.rand()), FNKDAT_USER | FNKDAT_CREAT);

                if(pStream->open(replayname2)) {
                    isOpen = true;
                    break;
                }

                const std::error_code replay2_error{errno, std::generic_category()};

                sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION,
                                fmt::format("Unable to open the replay file {}: {}",
                                            std::filesystem::path{replayname2}.filename(), replay2_error.message())
                                    .c_str());
            }
        }

        if(isOpen) {
            pStream->writeString(getLocalPlayerName());

            gameInitSettings.save(*pStream);

            // when this game was loaded we have to save the old commands to the replay file first
            cmdManager.save(*pStream);

            // flush stream
            pStream->flush();

            // now all new commands might be added
            cmdManager.setStream(std::move(pStream));
        } else {
            // This can happen if another instance of the game is running or if the disk is full.
            // TODO: Report problem to user...?
            sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Unable to open the replay log file.");

            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, _("Replay Log").c_str(),
                                     _("Unable to open the replay log file.").c_str(), window);

            quitGame();
        }
    }

    if(pNetworkManager != nullptr) {
        pNetworkManager->setOnReceiveChatMessage(
            [cm = &pInterface->getChatManager()](const auto& username, const auto& message) {
                cm->addChatMessage(username, message);
            });
        pNetworkManager->setOnReceiveCommandList([cm = &cmdManager](const auto& playername, const auto& commands) {
            cm->addCommandList(playername, commands);
        });
        pNetworkManager->setOnReceiveSelectionList(
            [this](const auto& name, const auto& newSelectionList, auto groupListIndex) {
                this->onReceiveSelectionList(name, newSelectionList, groupListIndex);
            });
        pNetworkManager->setOnPeerDisconnected(
            [this](const auto& name, auto bHost, auto cause) { onPeerDisconnected(name, bHost, cause); });

        cmdManager.setNetworkCycleBuffer(MILLI2CYCLES(pNetworkManager->getMaxPeerRoundTripTime()) + 5);
    }

    // Change music to ingame music
    musicPlayer->changeMusic(MUSIC_PEACE);

    const int gameStart = static_cast<int>(SDL_GetTicks());

    int numFrames = 0;

    auto targetGameCycle = gameCycleCount;

    lastTargetGameCycleTime = gameStart;

    SDL_Event event;

    const auto performanceScaleMs = 1000.0f / SDL_GetPerformanceFrequency();

    // sdl2::log_info("Random Seed (GameCycle %d): 0x%0X", GameCycleCount, RandomGen.getSeed());

    // main game loop
    do {
        auto now = static_cast<int>(SDL_GetTicks());

        const auto frameStart = now;

        if(finished && !bPause) {
            // end timer for the ending message
            if(now - finishedLevelTime > END_WAIT_TIME) { finishedLevel = true; }
        }

        const auto renderStart = SDL_GetPerformanceCounter();

        // clear whole screen
        SDL_SetRenderDrawColor(renderer, 100, 50, 0, 255);
        SDL_RenderClear(renderer);

        drawScreen();

        Dune_RenderPresent(renderer);

        const auto renderElapsed = SDL_GetPerformanceCounter() - renderStart;

        if(bShowFPS) { averageRenderTime = 0.97f * averageRenderTime + 0.03f * performanceScaleMs * renderElapsed; }

        numFrames++;

        const auto gameSpeed = getGameSpeed();

        bool bWaitForNetwork = false;

        if(pNetworkManager != nullptr) { serviceNetwork(bWaitForNetwork); }

        while(SDL_PollEvent(&event))
            doInput(context, event);

        pInterface->updateObjectInterface();

        if(pNetworkManager != nullptr) {
            if(bSelectionChanged) {
                pNetworkManager->sendSelectedList(selectedList);

                bSelectionChanged = false;
            }
        }

        if(pInGameMentat != nullptr) { pInGameMentat->update(); }

        if(pWaitingForOtherPlayers != nullptr) { pWaitingForOtherPlayers->update(); }

        cmdManager.update();

        while(!bWaitForNetwork && !bPause) {
            now = static_cast<int>(SDL_GetTicks());

            if(gameCycleCount < skipToGameCycle) {
                targetGameCycle         = skipToGameCycle;
                lastTargetGameCycleTime = now;
            } else {
                auto pendingTicks = now - lastTargetGameCycleTime;

                // Watch for discontinuities...
                if(pendingTicks > 2500) {
                    pendingTicks            = 2 * gameSpeed;
                    lastTargetGameCycleTime = now - pendingTicks;
                }

                while(pendingTicks >= gameSpeed) {
                    pendingTicks -= gameSpeed;
                    lastTargetGameCycleTime += gameSpeed;
                    ++targetGameCycle;
                }
            }

            if(gameCycleCount >= targetGameCycle) break;

            // Reset in case of some massive discontinuity (e.g., computer sleep or debugger break).
            if(targetGameCycle - gameCycleCount > 250) targetGameCycle = gameCycleCount;

            const auto updateStart = SDL_GetPerformanceCounter();

            updateGame(context);

            const auto updateElapsed = SDL_GetPerformanceCounter() - updateStart;

            if(bShowFPS) { averageUpdateTime = 0.97f * averageUpdateTime + 0.03f * performanceScaleMs * updateElapsed; }

            if(takePeriodicalScreenshots && ((gameCycleCount % (MILLI2CYCLES(10 * 1000))) == 0)) { takeScreenshot(); }

            now = static_cast<int>(SDL_GetTicks());
            // Don't block the UI for more than 75ms, even if we are behind.
            if(now - frameStart > 75) break;
        }

        while(SDL_PollEvent(&event))
            doInput(context, event);

        const auto until = frameStart + (settings.video.frameLimit ? 16 : 5);

        doEventsUntil(context, until);

        musicPlayer->musicCheck(); // if song has finished, start playing next one

        now                     = static_cast<int>(SDL_GetTicks());
        const auto frameElapsed = now - frameStart;

        if(bShowFPS) { averageFrameTime = 0.97f * averageFrameTime + 0.03f * frameElapsed; }
    } while(!bQuitGame && !finishedLevel); // not sure if we need this extra bool

    // Game is finished

    if(!bReplay && context.game.won) {
        // save replay

        auto mapnameBase = getBasename(gameInitSettings.getFilename(), true);
        mapnameBase += ".rpl";
        auto rplName          = std::filesystem::path{"replay"} / mapnameBase;
        auto [ok, replayname] = fnkdat(rplName, FNKDAT_USER | FNKDAT_CREAT);

        OFileStream replystream;
        replystream.open(replayname);
        replystream.writeString(getLocalPlayerName());
        gameInitSettings.save(replystream);
        cmdManager.save(replystream);
    }

    if(pNetworkManager != nullptr) { pNetworkManager->disconnect(); }

    gameState = GameState::Deinitialize;
    sdl2::log_info("Game finished!");
}

void Game::resumeGame() {
    bMenu = false;
    if(gameType != GameType::CustomMultiplayer) {
        bPause = false;

        // Remove the time spent paused from the targetGameCycle update.
        const auto now       = static_cast<int>(SDL_GetTicks());
        const auto pauseTime = now - pauseGameTime;
        if(pauseTime > 0) lastTargetGameCycleTime += pauseTime;
        else
            lastTargetGameCycleTime = now;
    }
}

GameInitSettings Game::getNextGameInitSettings() {
    if(nextGameInitSettings.getGameType() != GameType::Invalid) {
        // return the prepared game init settings (load game or restart mission)
        return nextGameInitSettings;
    }

    switch(gameInitSettings.getGameType()) {
        case GameType::Campaign: {
            int currentMission = gameInitSettings.getMission();
            if(!won) { currentMission -= (currentMission >= 22) ? 1 : 3; }
            int    nextMission          = gameInitSettings.getMission();
            Uint32 alreadyPlayedRegions = gameInitSettings.getAlreadyPlayedRegions();
            if(currentMission >= -1) {
                // do map choice
                sdl2::log_info("Map Choice...");
                MapChoice mapChoice(gameInitSettings.getHouseID(), currentMission, alreadyPlayedRegions);
                mapChoice.showMenu();
                nextMission          = mapChoice.getSelectedMission();
                alreadyPlayedRegions = mapChoice.getAlreadyPlayedRegions();
            }

            Uint32 alreadyShownTutorialHints =
                won ? pLocalPlayer->getAlreadyShownTutorialHints() : gameInitSettings.getAlreadyShownTutorialHints();
            return GameInitSettings(gameInitSettings, nextMission, alreadyPlayedRegions, alreadyShownTutorialHints);
        } break;

        default: {
            sdl2::log_info("Game::getNextGameInitClass(): Wrong gameType for next Game.");
            return GameInitSettings();
        } break;
    }
}

int Game::whatNext() {
    if(whatNextParam != GAME_NOTHING) {
        int tmp       = whatNextParam;
        whatNextParam = GAME_NOTHING;
        return tmp;
    }

    if(nextGameInitSettings.getGameType() != GameType::Invalid) { return GAME_LOAD; }

    switch(gameType) {
        case GameType::Campaign: {
            if(bQuitGame) { return GAME_RETURN_TO_MENU; }
            if(won) {
                if(gameInitSettings.getMission() == 22) {
                    // there is no mission after this mission
                    whatNextParam = GAME_RETURN_TO_MENU;
                } else {
                    // there is a mission after this mission
                    whatNextParam = GAME_NEXTMISSION;
                }
                return GAME_DEBRIEFING_WIN;
            }
            // we need to play this mission again
            whatNextParam = GAME_NEXTMISSION;

            return GAME_DEBRIEFING_LOST;
        } break;

        case GameType::Skirmish: {
            if(bQuitGame) { return GAME_RETURN_TO_MENU; }
            if(won) {
                whatNextParam = GAME_RETURN_TO_MENU;
                return GAME_DEBRIEFING_WIN;
            }
            whatNextParam = GAME_RETURN_TO_MENU;
            return GAME_DEBRIEFING_LOST;
        } break;

        case GameType::CustomGame:
        case GameType::CustomMultiplayer: {
            if(bQuitGame) { return GAME_RETURN_TO_MENU; }
            whatNextParam = GAME_RETURN_TO_MENU;
            return GAME_CUSTOM_GAME_STATS;
        } break;

        default: {
            return GAME_RETURN_TO_MENU;
        } break;
    }
}

bool Game::loadSaveGame(const std::filesystem::path& filename) {
    IFileStream fs;

    if(!fs.open(filename)) { return false; }

    bool ret = loadSaveGame(fs);

    fs.close();

    return ret;
}

bool Game::loadSaveGame(InputStream& stream) {
    gameState = GameState::Loading;

    Uint32 magicNum = stream.readUint32();
    if(magicNum != SAVEMAGIC) {
        sdl2::log_info("Game::loadSaveGame(): No valid savegame! Expected magic number %.8X, but got %.8X!", SAVEMAGIC,
                       magicNum);
        return false;
    }

    Uint32 savegameVersion = stream.readUint32();
    if(savegameVersion != SAVEGAMEVERSION) {
        sdl2::log_info("Game::loadSaveGame(): No valid savegame! Expected savegame version %d, but got %d!",
                       SAVEGAMEVERSION, savegameVersion);
        return false;
    }

    std::string duneVersion = stream.readString();

    // if this is a multiplayer load we need to save some information before we overwrite gameInitSettings with the
    // settings saved in the savegame
    bool                            bMultiplayerLoad = (gameInitSettings.getGameType() == GameType::LoadMultiplayer);
    GameInitSettings::HouseInfoList oldHouseInfoList = gameInitSettings.getHouseInfoList();

    // read gameInitSettings
    gameInitSettings = GameInitSettings(stream);

    // read the actual house setup chosen at the beginning of the game
    const auto numHouseInfo = stream.readUint32();
    houseInfoListSetup.reserve(numHouseInfo);
    for(Uint32 i = 0; i < numHouseInfo; i++) {
        houseInfoListSetup.push_back(GameInitSettings::HouseInfo(stream));
    }

    // read map size
    const short mapSizeX = stream.readUint32();
    const short mapSizeY = stream.readUint32();

    // create the new map
    map            = std::make_unique<Map>(*this, mapSizeX, mapSizeY);
    currentGameMap = map.get();

    const GameContext context{*this, *map, this->getObjectManager()};

    // read GameCycleCount
    gameCycleCount = stream.readUint32();

    // read some settings
    gameType  = static_cast<GameType>(stream.readSint8());
    techLevel = stream.readUint8();
    randomFactory.setSeed(stream.readUint8Vector());
    auto seed = stream.readUint8Vector();
    if(seed.size() != decltype(randomGen)::state_bytes) THROW(std::runtime_error, "Random state size mismatch!");
    randomGen.setState(seed);

    // read in the unit/structure data
    objectData.load(stream);

    // load the house(s) info
    for(auto i = 0; i < static_cast<int>(HOUSETYPE::NUM_HOUSES); i++) {
        if(stream.readBool()) {
            // house in game
            house[i] = std::make_unique<House>(context, stream);
        }
    }

    // we have to set the local player
    if(bMultiplayerLoad) {
        // get it from the gameInitSettings that started the game (not the one saved in the savegame)
        for(const auto& houseInfo : oldHouseInfoList) {

            // find the right house
            for(int i = 0; i < static_cast<int>(HOUSETYPE::NUM_HOUSES); i++) {
                if((house[i] != nullptr) && (house[i]->getHouseID() == houseInfo.houseID)) {
                    // iterate over all players
                    const auto& players    = house[i]->getPlayerList();
                    auto        playerIter = players.cbegin();
                    for(const auto& playerInfo : houseInfo.playerInfoList) {
                        if(playerInfo.playerClass == HUMANPLAYERCLASS) {
                            while(playerIter != players.cend()) {

                                auto* const pHumanPlayer = dynamic_cast<HumanPlayer*>(playerIter->get());
                                if(pHumanPlayer) {
                                    // we have actually found a human player and now assign the first unused name to it
                                    unregisterPlayer(pHumanPlayer);
                                    pHumanPlayer->setPlayername(playerInfo.playerName);
                                    registerPlayer(pHumanPlayer);

                                    if(playerInfo.playerName == getLocalPlayerName()) {
                                        pLocalHouse  = house[i].get();
                                        pLocalPlayer = pHumanPlayer;
                                    }

                                    ++playerIter;
                                    break;
                                }
                                ++playerIter;
                            }
                        }
                    }
                }
            }
        }
    } else {
        // it is stored in the savegame, so set it up
        const auto localPlayerID = stream.readUint8();
        pLocalPlayer             = dynamic_cast<HumanPlayer*>(getPlayerByID(localPlayerID));
        pLocalHouse              = house[static_cast<int>(pLocalPlayer->getHouse()->getHouseID())].get();
    }

    debug          = stream.readBool();
    bCheatsEnabled = stream.readBool();

    winFlags  = stream.readUint32();
    loseFlags = stream.readUint32();

    map->load(stream);

    // load the structures and units
    objectManager.load(stream);

    const auto numBullets = stream.readUint32();
    bulletList.reserve(numBullets);
    for(auto i = 0u; i < numBullets; i++) {
        map->add_bullet(stream);
    }

    const auto numExplosions = stream.readUint32();
    explosionList.reserve(numExplosions);
    for(auto i = 0u; i < numExplosions; i++) {
        addExplosion(stream);
    }

    if(bMultiplayerLoad) {
        screenborder->adjustScreenBorderToMapsize(map->getSizeX(), map->getSizeY());

        screenborder->setNewScreenCenter(pLocalHouse->getCenterOfMainBase() * TILESIZE);

    } else {
        // load selection list
        selectedList = stream.readUint32Set();

        // load the screenborder info
        screenborder->adjustScreenBorderToMapsize(map->getSizeX(), map->getSizeY());
        screenborder->load(stream);
    }

    // load triggers
    triggerManager.load(stream);

    // CommandManager is at the very end of the file. DO NOT CHANGE THIS!
    cmdManager.load(stream);

    finished = false;

    return true;
}

bool Game::saveGame(const std::filesystem::path& filename) {
    OFileStream fs;

    if(!fs.open(filename)) {
        sdl2::log_info("Game::saveGame(): %s", strerror(errno));
        currentGame->addToNewsTicker(std::string("Game NOT saved: Cannot open \"") + filename.u8string() + "\".");
        return false;
    }

    fs.writeUint32(SAVEMAGIC);

    fs.writeUint32(SAVEGAMEVERSION);

    fs.writeString(VERSIONSTRING);

    // write gameInitSettings
    gameInitSettings.save(fs);

    fs.writeUint32(houseInfoListSetup.size());
    for(const GameInitSettings::HouseInfo& houseInfo : houseInfoListSetup) {
        houseInfo.save(fs);
    }

    // write the map size
    fs.writeUint32(map->getSizeX());
    fs.writeUint32(map->getSizeY());

    // write GameCycleCount
    fs.writeUint32(gameCycleCount);

    // write some settings
    fs.writeSint8(static_cast<Sint8>(gameType));
    fs.writeUint8(techLevel);
    fs.writeUint8Vector(randomFactory.getSeed());
    fs.writeUint8Vector(randomGen.getState());

    // write out the unit/structure data
    objectData.save(fs);

    // write the house(s) info
    for(int i = 0; i < static_cast<int>(HOUSETYPE::NUM_HOUSES); i++) {
        fs.writeBool(house[i] != nullptr);

        if(house[i] != nullptr) { house[i]->save(fs); }
    }

    if(gameInitSettings.getGameType() != GameType::CustomMultiplayer) { fs.writeUint8(pLocalPlayer->getPlayerID()); }

    fs.writeBool(debug);
    fs.writeBool(bCheatsEnabled);

    fs.writeUint32(winFlags);
    fs.writeUint32(loseFlags);

    map->save(fs, getGameCycleCount());

    // save the structures and units
    objectManager.save(fs);

    fs.writeUint32(bulletList.size());
    for(const auto& pBullet : bulletList) {
        pBullet->save(fs);
    }

    fs.writeUint32(explosionList.size());
    for(const auto& pExplosion : explosionList) {
        pExplosion->save(fs);
    }

    if(gameInitSettings.getGameType() != GameType::CustomMultiplayer) {
        // save selection lists

        // write out selected units list
        fs.writeUint32Set(selectedList);

        // write the screenborder info
        screenborder->save(fs);
    }

    // save triggers
    triggerManager.save(fs);

    // CommandManager is at the very end of the file. DO NOT CHANGE THIS!
    cmdManager.save(fs);

    fs.close();

    return true;
}

void Game::saveObject(OutputStream& stream, ObjectBase* obj) {
    if(obj == nullptr) return;

    stream.writeUint32(obj->getItemID());
    obj->save(*this, stream);
}

void Game::setGameWon() {
    if(!bQuitGame && !finished) {
        won               = true;
        finished          = true;
        finishedLevelTime = SDL_GetTicks();
        soundPlayer->playVoice(YourMissionIsComplete, pLocalHouse->getHouseID());
    }
}

void Game::setGameLost() {
    if(!bQuitGame && !finished) {
        won               = false;
        finished          = true;
        finishedLevelTime = SDL_GetTicks();
        soundPlayer->playVoice(YouHaveFailedYourMission, pLocalHouse->getHouseID());
    }
}

void Game::add_bullet(const GameContext& context, uint32_t            shooterID, const Coord* newRealLocation,
                      const Coord*       newRealDestination, uint32_t bulletID, int           damage, bool air,
                      const ObjectBase*  pTarget)
{
    bulletList.push_back(std::make_unique<Bullet>(context, shooterID, newRealLocation, newRealDestination, bulletID,
                                                  damage, air, pTarget);
}

int Game::getGameSpeed() const {
    if(gameType == GameType::CustomMultiplayer) { return gameInitSettings.getGameOptions().gameSpeed; }
    return settings.gameOptions.gameSpeed;
}

void Game::addExplosion(uint32_t explosionID, const Coord& position, HOUSETYPE house) {
    explosionList.emplace_back(std::make_unique<Explosion>(explosionID, position, house));
}


} // namespace Dune::Engine
