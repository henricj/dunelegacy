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

#include <Menu/MenuBase.h>

#include <Network/NetworkManager.h>

#include "FileClasses/LoadSavePNG.h"
#include "GUI/GUIStyle.h"
#include "misc/DrawingRectHelper.h"
#include "misc/draw_util.h"
#include "misc/dune_clock.h"
#include "misc/dune_events.h"

#include <globals.h>

#include <main.h>
#include <sand.h>

MenuBase::MenuBase() : Window(0, 0, 0, 0) { }

MenuBase::~MenuBase() = default;

void MenuBase::quit(int returnVal) {
    retVal   = returnVal;
    quitting = true;
}

bool MenuBase::doEventsUntil(const dune::dune_clock::time_point until) {
    using namespace std::chrono_literals;

    SDL_Event event{};

    while (!quitting) {
        const auto remaining = until - dune::dune_clock::now();

        if (remaining <= dune::dune_clock::duration::zero() || remaining >= 32ms)
            return true;

        if (dune::Dune_WaitEvent(&event, dune::as_milliseconds<int>(remaining))) {
            if (!doInput(event))
                return false;

            while (SDL_PollEvent(&event)) {
                // check the events
                if (!doInput(event))
                    return false;
            }
        }
    }

    return true;
}

int MenuBase::showMenu() {
    using namespace std::chrono_literals;

    SDL_Event event{};

    quitting = false;

    while (!quitting) {
        const auto frameStart = dune::dune_clock::now();

        update();

        if (dune::globals::pNetworkManager != nullptr) {
            dune::globals::pNetworkManager->update();
        }

        if (quitting) {
            return retVal;
        }

        auto* const renderer = dune::globals::renderer.get();

        if (bClearScreen) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
        }
        draw();
        drawCursor();
        Dune_RenderPresent(renderer);

        updateFullscreen();

        while (SDL_PollEvent(&event)) {
            // check the events
            if (!doInput(event))
                break;
        }

        if (!dune::globals::settings.video.frameLimit)
            continue;

        if (!doEventsUntil(frameStart + 32ms))
            break;
    }

    return retVal;
}

void MenuBase::draw() {
    auto* const renderer = dune::globals::renderer.get();

    const SDL_Rect clipRect{getPosition().x, getPosition().y, getSize().x, getSize().y};
    SDL_RenderSetClipRect(renderer, &clipRect);

    parent::draw();

    drawSpecificStuff();

    parent::drawOverlay();

    SDL_RenderSetClipRect(renderer, nullptr);
}

void MenuBase::drawSpecificStuff() { }

bool MenuBase::doInput(SDL_Event& event) {
    switch (event.type) {
        case SDL_KEYDOWN: {
            // Look for a keypress
            switch (event.key.keysym.sym) {

                case SDLK_ESCAPE: {
                    if (pChildWindow == nullptr && bAllowQuitting) {
                        quit();
                    }
                } break;

                case SDLK_RETURN: {
                    if (SDL_GetModState() & KMOD_ALT) {
                        toggleFullscreen();
                    }
                } break;

                case SDLK_p: {
                    if (SDL_GetModState() & KMOD_CTRL) {
                        // fall through to SDLK_PRINT
                    } else {
                        break; // do not fall through
                    }
                    [[fallthrough]];
                } // fall through

                case SDLK_PRINTSCREEN:
                case SDLK_SYSREQ: {
                    SaveScreenshot();
                } break;

                case SDLK_TAB: {
                    if (SDL_GetModState() & KMOD_ALT) {
                        SDL_MinimizeWindow(dune::globals::window.get());
                    }
                } break;

                default: {
                } break;
            }
        } break;

        case SDL_MOUSEMOTION: {
            const SDL_MouseMotionEvent* mouse = &event.motion;

            const auto actual = getSize();

            dune::globals::drawnMouseX = std::max(0, std::min(mouse->x, actual.x - 1));
            dune::globals::drawnMouseY = std::max(0, std::min(mouse->y, actual.y - 1));
        } break;

        case SDL_QUIT: {
            if (pChildWindow == nullptr && bAllowQuitting) {
                quit();
            }
        } break;

        default: {
        } break;
    }

    handleInput(event);

    return !quitting;
}

TopMenuBase::TopMenuBase() {
    // We are a top level window
    const auto size = getRendererSize();

    TopMenuBase::resize(size.w, size.h);
}

TopMenuBase::~TopMenuBase() = default;

void TopMenuBase::draw_background(Point position) {
    MenuBase::draw_background(position);

    auto& gui = GUIStyle::getInstance();

    const auto& size = getSize();

    const auto dest = SDL_FRect{static_cast<float>(position.x), static_cast<float>(position.y),
                                static_cast<float>(size.x), static_cast<float>(size.y)};

    gui.drawBackground(dune::globals::renderer.get(), dest);
}
