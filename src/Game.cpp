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

#include <config.h>
#include <globals.h>

#include "misc/Fullscreen.h"
#include <FileClasses/GFXManager.h>
#include <FileClasses/music/MusicPlayer.h>
#include <GUI/GUIStyle.h>
#include <GUI/dune/InGameMenu.h>
#include <GUI/dune/WaitingForOtherPlayers.h>
#include <GameInterface.h>
#include <House.h>
#include <INIMap/INIMapLoader.h>
#include <Map.h>
#include <Menu/BriefingMenu.h>
#include <Menu/MentatHelp.h>
#include <Network/NetworkManager.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>
#include <misc/DrawingRectHelper.h>
#include <misc/IFileStream.h>
#include <misc/IMemoryStream.h>
#include <misc/OFileStream.h>
#include <misc/dune_sdl.h>
#include <misc/dune_sdlpp.h>
#include <misc/dune_timer_resolution.h>
#include <misc/fnkdat.h>
#include <players/HumanPlayer.h>
#include <structures/ConstructionYard.h>
#include <structures/Palace.h>
#include <units/Harvester.h>

#include <fmt/format.h>

#include <gsl/gsl>

#include <tuple>

namespace {
// SDL3: Convert event coordinates from window space to render logical space
void convertEventToRenderCoordinates(SDL_Event& event) {
    auto* renderer = dune::globals::renderer.get();
    if (renderer) {
        SDL_ConvertEventToRenderCoordinates(renderer, &event);
    }
}
} // namespace

Game::Game() : localPlayerName_(dune::globals::settings.general.playerName), uiController_(*this), core_(*this) {
    dune::globals::currentZoomlevel = dune::globals::settings.video.preferredZoomLevel;

    dune::globals::unitList.clear();      // holds all the units
    dune::globals::structureList.clear(); // all the structures
    dune::globals::bulletList.clear();

    if (auto* const music_player = dune::globals::musicPlayer.get())
        music_player->changeMusic(MUSIC_PEACE);

    dune::globals::debug = false;

    resize();
}

/**
    The destructor frees up all the used memory.
*/
Game::~Game() {
    if (auto* const network_manager = dune::globals::pNetworkManager.get()) {
        network_manager->setOnReceiveChatMessage({});
        network_manager->setOnReceiveCommandList({});
        network_manager->setOnReceiveSelectionList({});
        network_manager->setOnPeerDisconnected({});
    }

    dune::globals::structureList.clear();

    dune::globals::unitList.clear();

    dune::globals::bulletList.clear();

    explosionList_.clear();

    dune::globals::currentGameMap = nullptr;
    map_.reset();

    dune::globals::screenborder.reset();
}

void Game::resize() {
    const auto* const gfx = dune::globals::pGFXManager.get();

    if (nullptr == gfx)
        return;

    sideBarPos_ = calcAlignedDrawingRect(gfx->getUIGraphic(UI_SideBar), HAlign::Right, VAlign::Top);
    topBarPos_  = calcAlignedDrawingRect(gfx->getUIGraphic(UI_TopBar), HAlign::Left, VAlign::Top);

    const auto renderer_size   = getRendererSize();
    const auto renderer_width  = static_cast<float>(renderer_size.w);
    const auto renderer_height = static_cast<float>(renderer_size.h);

    powerIndicatorPos_.h = spiceIndicatorPos_.h = renderer_height - 146 - 2;

    //////////////////////////////////////////////////////////////////////////
    const SDL_FRect gameBoardRect{0, topBarPos_.h, sideBarPos_.x, renderer_height - topBarPos_.h};

    dune::globals::screenborder = std::make_unique<ScreenBorder>(gameBoardRect);

    if (map_)
        dune::globals::screenborder->adjustScreenBorderToMapsize(map_->getSizeX(), map_->getSizeY());

    uiController_.resize(static_cast<uint32_t>(renderer_width), static_cast<uint32_t>(renderer_height));
}

void Game::publishUIEvent(GameUIEvent event) {
    uiController_.publishUIEvent(std::move(event));
}

bool Game::pollUIEvent(GameUIEvent& event) {
    return uiController_.pollUIEvent(event);
}

void Game::initGame(const GameInitSettings& newGameInitSettings) {
    gameInitSettings_ = newGameInitSettings;

    switch (gameInitSettings_.getGameType()) {
        case GameType::LoadSavegame: {
            if (!loadSaveGame(gameInitSettings_.getFilename())) {
                THROW(std::runtime_error, "Loading save game failed!");
            }
        } break;

        case GameType::LoadMultiplayer: {
            IMemoryStream memStream(gameInitSettings_.getFiledata().data(), gameInitSettings_.getFiledata().size());

            if (!loadSaveGame(memStream)) {
                THROW(std::runtime_error, "Loading save game failed!");
            }
        } break;

        case GameType::Campaign:
        case GameType::Skirmish:
        case GameType::CustomGame:
        case GameType::CustomMultiplayer: {
            gameType = gameInitSettings_.getGameType();
            randomFactory.setSeed({gameInitSettings_.getRandomSeed()});

            randomGen = randomFactory.create("Game");

            objectData.loadFromINIFile("ObjectData.ini");

            if (gameInitSettings_.getMission() != 0) {
                techLevel = ((gameInitSettings_.getMission() + 1) / 3) + 1;
            }

            INIMapLoader loader{this, gameInitSettings_.getFilename(), gameInitSettings_.getFiledata()};

            map_ = loader.load();

            dune::globals::currentGameMap = map_.get();

            if (!bReplay_ && gameInitSettings_.getGameType() != GameType::CustomGame
                && gameInitSettings_.getGameType() != GameType::CustomMultiplayer) {
                /* do briefing */
                sdl2::log_info("Briefing...");
                uiController_.showBriefingMenu(
                    gameInitSettings_.getHouseID(), gameInitSettings_.getMission(), BRIEFING, sdl_handler_);
            }
        } break;

        default: {
        } break;
    }
}

void Game::initReplay(const std::filesystem::path& filename) {
    bReplay_ = true;

    IFileStream fs;

    if (!fs.open(filename)) {
        THROW(io_error, "Error while opening '{}'!", filename.string());
    }

    // override local player name as it was when the replay was created
    localPlayerName_ = fs.readString();

    // read GameInitInfo
    const GameInitSettings loadedGameInitSettings(fs);

    // load all commands
    cmdManager_.load(fs);

    initGame(loadedGameInitSettings);
}

void Game::runMainLoop(const GameContext& context, MenuBase::event_handler_type handler) {
    using namespace std::chrono_literals;

    sdl2::log_info("Starting game...");

    sdl_handler_         = handler;
    auto cleanup_handler = gsl::finally([&] { sdl_handler_ = {}; });

    // add interface
    uiController_.createGameInterface(context);

    sdl2::log_info("Sizes: Tile {} UnitBase {} StructureBase {} Harvester {} ConstructionYard {} Palace {}",
                   sizeof(Tile),
                   sizeof(UnitBase),
                   sizeof(StructureBase),
                   sizeof(Harvester),
                   sizeof(ConstructionYard),
                   sizeof(Palace));

    gameState = GameState::Running;

    // setup endlevel conditions
    finishedLevel_ = false;

    bShowTime_ = winFlags & WINLOSEFLAGS_TIMEOUT;

    // Check if a player has lost
    std::ranges::for_each(house_, [](auto& h) {
        if (h && !h->isAlive())
            h->lose(true);
    });

    if (bReplay_) {
        cmdManager_.setReadOnly(true);
    } else {
        auto pStream = std::make_unique<OFileStream>();

        const auto [ok, replayname] = fnkdat("replay/auto.rpl", FNKDAT_USER | FNKDAT_CREAT);

        auto isOpen = ok && pStream->open(replayname);

        if (!isOpen) {
            const std::error_code replay_error{errno, std::generic_category()};

            sdl2::log_error("Unable to open the default replay file: {}  Retrying...", replay_error.message());

            auto& uiRandom = dune::globals::pGFXManager->random();

            for (auto i = 0; i < 10; ++i) {
                const auto [ok2, replayname2] =
                    fnkdat(fmt::format("replay/auto-{}.rpl", uiRandom.rand()), FNKDAT_USER | FNKDAT_CREAT);

                if (pStream->open(replayname2)) {
                    isOpen = true;
                    break;
                }

                const std::error_code replay2_error{errno, std::generic_category()};

                sdl2::log_error(
                    "Unable to open the replay file {}: {}", replayname2.filename().string(), replay2_error.message());
            }
        }

        if (isOpen) {
            pStream->writeString(getLocalPlayerName());

            gameInitSettings_.save(*pStream);

            // when this game was loaded we have to save the old commands to the replay file first
            cmdManager_.save(*pStream);

            // flush stream
            pStream->flush();

            // now all new commands might be added
            cmdManager_.setStream(std::move(pStream));
        } else {
            // This can happen if another instance of the game is running or if the disk is full.
            // TODO: Report problem to user...?
            sdl2::log_error("Unable to open the replay log file.");

            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                     std::string{_("Replay Log")}.c_str(),
                                     std::string{_("Unable to open the replay log file.")}.c_str(),
                                     dune::globals::window.get());

            quitGame();
        }
    }

    auto* const network_manager = dune::globals::pNetworkManager.get();
    if (network_manager) {
        network_manager->setOnReceiveChatMessage(
            [cm = &uiController_.getGameInterface()->getChatManager()](const auto& username, const auto& message) {
                cm->addChatMessage(username, message);
            });
        network_manager->setOnReceiveCommandList([cm = &cmdManager_](const auto& playername, const auto& commands) {
            cm->addCommandList(playername, commands);
        });
        network_manager->setOnReceiveSelectionList(
            [this](const auto& name, const auto& newSelectionList, auto groupListIndex) {
                this->onReceiveSelectionList(name, newSelectionList, groupListIndex);
            });
        network_manager->setOnPeerDisconnected(
            [this](const auto& name, auto bHost, auto cause) { onPeerDisconnected(name, bHost, cause); });

        cmdManager_.setNetworkCycleBuffer(MILLI2CYCLES(network_manager->getMaxPeerRoundTripTime()) + 5);
    }

    // Change music to ingame music
    dune::globals::musicPlayer->changeMusic(MUSIC_PEACE);

    const auto gameStart = dune::dune_clock::now();

    int numFrames = 0;

    auto targetGameCycle = gameCycleCount_;

    lastTargetGameCycleTime_ = gameStart;

    auto previousFrameStart = gameStart;

    SDL_Event event;

    dune::DuneTimerResolution timer_handle;

    auto* const renderer = dune::globals::renderer.get();

    // main game loop
    do {
        auto now = dune::dune_clock::now();

        const auto frameStart = now;

        const auto frameElapsed = frameStart - previousFrameStart;

        previousFrameStart = frameStart;

        if (bShowFPS_) {
            averageFrameTime_ = 0.97f * averageFrameTime_ + 0.03f * dune::as_milliseconds<float>(frameElapsed);
        }

        if (finished_ && !bPause_) {
            // end timer for the ending message
            if (now - finishedLevelTime_ > END_WAIT_TIME) {
                finishedLevel_ = true;
            }
        }

        const auto renderStart = dune::dune_clock::now();

        // clear whole screen
        SDL_SetRenderDrawColor(renderer, 100, 50, 0, 255);
        SDL_RenderClear(renderer);

        drawScreen();

        // Apparently this must be done after drawing, but before render present.
        // https://discourse.libsdl.org/t/sdl-renderreadpixels-always-returns-black-rectangle/20371/6
        if (pendingScreenshot_)
            saveScreenshot();

        Dune_RenderPresent(renderer);

        updateFullscreen();

        const auto renderElapsed = dune::dune_clock::now() - renderStart;

        if (bShowFPS_) {
            averageRenderTime_ = 0.97f * (averageRenderTime_) + 0.03f * dune::as_milliseconds<float>(renderElapsed);
        }

        numFrames++;

        const auto gameSpeed = getGameSpeed();

        bool bWaitForNetwork = false;

        if (network_manager != nullptr) {
            serviceNetwork(bWaitForNetwork);
        }

        while (SDL_PollEvent(&event)) {
            // SDL3: Convert event coordinates from window space to render logical space
            convertEventToRenderCoordinates(event);
            doInput(context, event);
        }

        uiController_.updateObjectInterface();

        if (network_manager != nullptr) {
            if (bSelectionChanged_) {
                network_manager->sendSelectedList(selectedList_);

                bSelectionChanged_ = false;
            }
        }

        uiController_.updateDialogs();

        cmdManager_.update();

        while (!bWaitForNetwork && !bPause_) {
            now = dune::dune_clock::now();

            if (gameCycleCount_ < skipToGameCycle_) {
                targetGameCycle          = skipToGameCycle_;
                lastTargetGameCycleTime_ = now;
            } else {
                auto pendingTicks = now - lastTargetGameCycleTime_;

                // Watch for discontinuities...
                if (pendingTicks > 2500ms) {
                    pendingTicks             = 2 * gameSpeed;
                    lastTargetGameCycleTime_ = now - pendingTicks;
                }

                while (pendingTicks >= gameSpeed) {
                    pendingTicks -= gameSpeed;
                    lastTargetGameCycleTime_ += gameSpeed;
                    ++targetGameCycle;
                }
            }

            if (gameCycleCount_ >= targetGameCycle)
                break;

            // Reset in case of some massive discontinuity (e.g., computer sleep or debugger break).
            if (targetGameCycle - gameCycleCount_ > 250)
                targetGameCycle = gameCycleCount_;

            const auto updateStart = dune::dune_clock::now();

            updateGame(context);
            updateUI();

            const auto updateElapsed = dune::dune_clock::now() - updateStart;

            if (bShowFPS_) {
                averageUpdateTime_ = 0.97f * averageUpdateTime_ + 0.03f * dune::as_milliseconds<float>(updateElapsed);
            }

            if (takePeriodicScreenshots_ && ((gameCycleCount_ % (MILLI2CYCLES(10 * 1000))) == 0)) {
                takeScreenshot();
            }

            now = dune::dune_clock::now();
            // Don't block the UI for more than 75ms, even if we are behind.
            if (now - frameStart > 75ms)
                break;
        }

        while (SDL_PollEvent(&event)) {
            // SDL3: Convert event coordinates from window space to render logical space
            convertEventToRenderCoordinates(event);
            doInput(context, event);
        }

        // Process ingame map scrolling.
        scrollViewport();

        dune::globals::musicPlayer->musicCheck(); // if song has finished, start playing next one

        if (bWaitForNetwork || bPause_ || gameCycleCount_ >= skipToGameCycle_) {
            const auto until = (dune::globals::settings.video.frameLimit ? 16ms : 5ms) + frameStart;

            doEventsUntil(context, until);

            const auto stop = dune::dune_clock::now();

            if (stop > until + 50ms) {
                static auto count = 0;
                ++count;
            }
        }
    } while (!bQuitGame_ && !finishedLevel_); // not sure if we need this extra bool

    // Game is finished

    if (!bReplay_ && context.game.won_) {
        // save replay

        auto mapnameBase = getBasename(gameInitSettings_.getFilename(), true);
        mapnameBase += ".rpl";
        const auto rplName    = std::filesystem::path{"replay"} / mapnameBase;
        auto [ok, replayname] = fnkdat(rplName, FNKDAT_USER | FNKDAT_CREAT);

        OFileStream replystream;
        replystream.open(replayname);
        replystream.writeString(getLocalPlayerName());
        gameInitSettings_.save(replystream);
        cmdManager_.save(replystream);
    }

    if (network_manager != nullptr) {
        network_manager->disconnect();
    }

    gameState = GameState::Deinitialize;
    sdl2::log_info("Game finished!");
}

void Game::pauseGame() {
    if (gameType != GameType::CustomMultiplayer) {
        bPause_        = true;
        pauseGameTime_ = dune::dune_clock::now();
    }
}

void Game::resumeGame() {
    bMenu_ = false;
    if (gameType != GameType::CustomMultiplayer) {
        bPause_ = false;

        // Remove the time spent paused from the targetGameCycle update.
        const auto now       = dune::dune_clock::now();
        const auto pauseTime = now - pauseGameTime_;
        if (pauseTime > dune::dune_clock::duration::zero())
            lastTargetGameCycleTime_ += pauseTime;
        else
            lastTargetGameCycleTime_ = now;
    }
}

void Game::onOptions() {
    if (bReplay_) {
        // don't show menu
        quitGame();
    } else {
        const auto color = SDL2RGB(
            dune::globals::palette
                [dune::globals::houseToPaletteIndex[static_cast<int>(dune::globals::pLocalHouse->getHouseID())] + 3]);
        uiController_.showInGameMenu((gameType == GameType::CustomMultiplayer), color);
        bMenu_ = true;
        pauseGame();
    }
}

void Game::onMentat() {
    uiController_.showMentatHelp(dune::globals::pLocalHouse->getHouseID(), techLevel, gameInitSettings_.getMission());
    bMenu_ = true;
    pauseGame();
}

GameInitSettings Game::getNextGameInitSettings() {
    if (nextGameInitSettings_.getGameType() != GameType::Invalid) {
        // return the prepared game init settings (load game or restart mission)
        return nextGameInitSettings_;
    }

    switch (gameInitSettings_.getGameType()) {
        case GameType::Campaign: {
            int currentMission = gameInitSettings_.getMission();
            if (!won_) {
                currentMission -= (currentMission >= 22) ? 1 : 3;
            }
            int nextMission               = gameInitSettings_.getMission();
            uint32_t alreadyPlayedRegions = gameInitSettings_.getAlreadyPlayedRegions();
            if (currentMission >= -1) {
                // do map choice
                sdl2::log_info("Map Choice...");
                std::tie(nextMission, alreadyPlayedRegions) = uiController_.showMapChoiceMenu(
                    gameInitSettings_.getHouseID(), currentMission, alreadyPlayedRegions, sdl_handler_);
            }

            const uint32_t alreadyShownTutorialHints = won_
                                                         ? dune::globals::pLocalPlayer->getAlreadyShownTutorialHints()
                                                         : gameInitSettings_.getAlreadyShownTutorialHints();
            return GameInitSettings(gameInitSettings_, nextMission, alreadyPlayedRegions, alreadyShownTutorialHints);
        }

        default: {
            sdl2::log_info("Game::getNextGameInitClass(): Wrong gameType for next Game.");
            return {};
        }
    }
}

int Game::whatNext() {
    if (whatNextParam_ != GAME_NOTHING) {
        const int tmp  = whatNextParam_;
        whatNextParam_ = GAME_NOTHING;
        return tmp;
    }

    if (nextGameInitSettings_.getGameType() != GameType::Invalid) {
        return GAME_LOAD;
    }

    switch (gameType) {
        case GameType::Campaign: {
            if (bQuitGame_) {
                return GAME_RETURN_TO_MENU;
            }
            if (won_) {
                if (gameInitSettings_.getMission() == 22) {
                    // there is no mission after this mission
                    whatNextParam_ = GAME_RETURN_TO_MENU;
                } else {
                    // there is a mission after this mission
                    whatNextParam_ = GAME_NEXTMISSION;
                }
                return GAME_DEBRIEFING_WIN;
            }
            // we need to play this mission again
            whatNextParam_ = GAME_NEXTMISSION;

            return GAME_DEBRIEFING_LOST;
        }

        case GameType::Skirmish: {
            if (bQuitGame_) {
                return GAME_RETURN_TO_MENU;
            }
            if (won_) {
                whatNextParam_ = GAME_RETURN_TO_MENU;
                return GAME_DEBRIEFING_WIN;
            }
            whatNextParam_ = GAME_RETURN_TO_MENU;
            return GAME_DEBRIEFING_LOST;
        }

        case GameType::CustomGame:
        case GameType::CustomMultiplayer: {
            if (bQuitGame_) {
                return GAME_RETURN_TO_MENU;
            }
            whatNextParam_ = GAME_RETURN_TO_MENU;
            return GAME_CUSTOM_GAME_STATS;
        }

        default: {
            return GAME_RETURN_TO_MENU;
        }
    }
}

void Game::setGameWon() {
    if (!bQuitGame_ && !finished_) {
        won_               = true;
        finished_          = true;
        finishedLevelTime_ = dune::dune_clock::now();
        dune::globals::soundPlayer->playVoice(Voice_enum::YourMissionIsComplete,
                                              dune::globals::pLocalHouse->getHouseID());
    }
}

void Game::setGameLost() {
    if (!bQuitGame_ && !finished_) {
        won_               = false;
        finished_          = true;
        finishedLevelTime_ = dune::dune_clock::now();
        dune::globals::soundPlayer->playVoice(Voice_enum::YouHaveFailedYourMission,
                                              dune::globals::pLocalHouse->getHouseID());
    }
}

dune::dune_clock::duration Game::getGameSpeed() const {
    if (gameType == GameType::CustomMultiplayer) {
        return dune::as_dune_clock_duration(gameInitSettings_.getGameOptions().gameSpeed);
    }
    return dune::as_dune_clock_duration(dune::globals::settings.gameOptions.gameSpeed);
}
