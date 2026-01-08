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
    // update all tiles
    map_->for_all([](Tile& t) { t.update(); });

    const GameContext context{*this, *dune::globals::currentGameMap, objectManager_};

    for (auto* pStructure : dune::globals::structureList) {
        pStructure->update(context);
    }

    if ((currentCursorMode == CursorMode_Placing) && selectedList_.empty()) {
        currentCursorMode = CursorMode_Normal;
    }

    for (auto* pUnit : dune::globals::unitList) {
        pUnit->update(context);
    }

    auto selection_changed = false;

    map_->consume_removed_objects([&](uint32_t objectID) {
        auto* object = objectManager_.getObject(objectID);

        if (!object)
            return;

        if (removeFromSelectionLists(object))
            selection_changed = true;
    });

    objectManager_.consume_pending_deletes([&](auto& object) {
        object->cleanup(context, dune::globals::pLocalPlayer);

        if (removeFromSelectionLists(object.get()))
            selection_changed = true;

        removeFromQuickSelectionLists(object->getObjectID());
    });

    if (selection_changed)
        selectionChanged();

    std::erase_if(dune::globals::bulletList, [&](auto& b) { return b->update(context); });

    std::erase_if(explosionList_, [](auto& e) { return e->update(); });
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

                if (pWaitingForOtherPlayers_ == nullptr) {
                    pWaitingForOtherPlayers_ = std::make_unique<WaitingForOtherPlayers>();
                    bMenu_                   = true;
                }
            }
        }

        SDL_Delay(10);
    } else {
        startWaitingForOtherPlayersTime_ = dune::dune_clock::time_point{};
        pWaitingForOtherPlayers_.reset();
    }
}

void Game::updateGame(const GameContext& context) {
    pInterface_->getRadarView().update();
    cmdManager_.executeCommands(context, gameCycleCount_);

    // sdl2::log_info("cycle {} : {}", gameCycleCount, context.game.randomGen.getSeed());

#ifdef TEST_SYNC
    // add every gamecycles one test sync command
    if (bReplay == false) {
        cmdManager.addCommand(Command(pLocalPlayer->getPlayerID(), CMD_TEST_SYNC, randomGen.getSeed()));
    }
#endif

    std::ranges::for_each(house_, [](auto& h) {
        if (h)
            h->update();
    });

    dune::globals::screenborder->update(dune::globals::pGFXManager->random());

    triggerManager_.trigger(context, gameCycleCount_);

    processObjects();

    if ((indicatorFrame_ != NONE_ID) && (--indicatorTimer_ <= 0)) {
        indicatorTimer_ = indicatorTime_;

        if (++indicatorFrame_ > 2) {
            indicatorFrame_ = NONE_ID;
        }
    }

    gameCycleCount_++;
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
