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

#include <globals.h>

#include <Bullet.h>
#include <FileClasses/GFXManager.h>
#include <GUI/dune/WaitingForOtherPlayers.h>
#include <GameInterface.h>
#include <House.h>
#include <Map.h>
#include <Network/NetworkManager.h>
#include <ScreenBorder.h>
#include <misc/dune_events.h>
#include <players/HumanPlayer.h>
#include <structures/StructureBase.h>
#include <units/UnitBase.h>

namespace {
// SDL3: Convert event coordinates from window space to render logical space
void convertEventToRenderCoordinates(SDL_Event& event) {
    auto* renderer = dune::globals::renderer.get();
    if (renderer) {
        SDL_ConvertEventToRenderCoordinates(renderer, &event);
    }
}
} // namespace

void Game::processObjects() {
    core_.processObjects();
}

void Game::setupView(const GameContext& context) const {
    // setup start location/view

    int i     = 0;
    int j     = 0;
    int count = 0;

    const auto* const house = dune::globals::pLocalHouse;

    for (const auto* pUnit : dune::globals::unitList) {
        if ((pUnit->getOwner() == house) && (pUnit->getItemID() != Unit_Sandworm)) {
            i += pUnit->getX();
            j += pUnit->getY();
            count++;
        }
    }

    for (const auto* pStructure : dune::globals::structureList) {
        if (pStructure->getOwner() == house) {
            i += pStructure->getX();
            j += pStructure->getY();
            count++;
        }
    }

    if (count == 0) {
        i = context.map.getSizeX() * TILESIZE / 2 - 1;
        j = context.map.getSizeY() * TILESIZE / 2 - 1;
    } else {
        i = i * TILESIZE / count;
        j = j * TILESIZE / count;
    }

    if (auto* const screen_border = dune::globals::screenborder.get())
        screen_border->setNewScreenCenter(Coord(i, j));
}

void Game::serviceNetwork(bool& bWaitForNetwork) {
    auto* const network_manager = dune::globals::pNetworkManager.get();

    network_manager->update();

    // test if we need to wait for data to arrive
    for (const auto& playername : network_manager->getConnectedPeers()) {
        const auto* const pPlayer = dynamic_cast<HumanPlayer*>(getPlayerByName(playername));
        if (pPlayer != nullptr) {
            if (pPlayer->nextExpectedCommandsCycle <= gameCycleCount_) {
                // sdl2::log_info("Cycle {}: Waiting for player '{}' to send data for cycle {}...", GameCycleCount,
                // pPlayer->getPlayername(), pPlayer->nextExpectedCommandsCycle);
                bWaitForNetwork = true;
                break;
            }
        }
    }

    if (bWaitForNetwork) {
        if (startWaitingForOtherPlayersTime_ == dune::dune_clock::time_point{}) {
            // we just started waiting
            startWaitingForOtherPlayersTime_ = dune::dune_clock::now();
        } else {
            using namespace std::chrono_literals;

            if (dune::dune_clock::now() - startWaitingForOtherPlayersTime_ > 1000ms) {
                // we waited for more than one second

                if (uiController_.getWaitingForOtherPlayers() == nullptr) {
                    uiController_.showWaitingDialog();
                    bMenu_ = true;
                }
            }
        }

        SDL_Delay(10);
    } else {
        startWaitingForOtherPlayersTime_ = dune::dune_clock::time_point{};
        uiController_.hideWaitingDialog();
    }
}

void Game::updateGame(const GameContext& context) {
    core_.updateGame(context);
}

void Game::stepSimulation(uint32_t ticks) {
    core_.stepSimulation(ticks);
}

void Game::updateUI() {
    // Pull the next available snapshot from the handoff if one is queued and ready.
    // This enables the UI to advance to a new published frame at a non-blocking, non-real-time boundary.
    frameHandoff_.tryAcquireNextForUi();

    if (uiController_.getGameInterface() != nullptr)
        uiController_.getGameInterface()->getRadarView().update();

    if (auto* const screen_border = dune::globals::screenborder.get())
        screen_border->update(dune::globals::pGFXManager->random());

    if ((indicatorFrame_ != NONE_ID) && (--indicatorTimer_ <= 0)) {
        indicatorTimer_ = indicatorTime_;

        if (++indicatorFrame_ > 2) {
            indicatorFrame_ = NONE_ID;
        }
    }

    if (selectedList_.empty() && currentCursorMode == CursorMode_Placing)
        currentCursorMode = CursorMode_Normal;
}

void Game::doEventsUntil(const GameContext& context, const dune::dune_clock::time_point until) {
    using namespace std::chrono_literals;

    SDL_Event event{};

    while (!bQuitGame_ && !finishedLevel_) {
        const auto remaining = until - dune::dune_clock::now();

        if (remaining <= dune::dune_clock::duration::zero() || remaining >= 100ms)
            return;

        const auto timeout = dune::as_milliseconds<int>(remaining);

        if (timeout < 1)
            return;

        if (dune::Dune_WaitEvent(&event, timeout)) {
            // SDL3: Convert event coordinates from window space to render logical space
            convertEventToRenderCoordinates(event);
            doInput(context, event);

            while (SDL_PollEvent(&event)) {
                // SDL3: Convert event coordinates from window space to render logical space
                convertEventToRenderCoordinates(event);
                doInput(context, event);
            }
        }
    }
}
