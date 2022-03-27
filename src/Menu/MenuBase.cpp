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

#include <FileClasses/LoadSavePNG.h>
#include <misc/FileSystem.h>
#include <misc/draw_util.h>
#include <misc/string_util.h>

#include <globals.h>

#include <main.h>
#include <sand.h>

MenuBase::MenuBase() : Window(0, 0, 0, 0) { }

MenuBase::~MenuBase() = default;

void MenuBase::quit(int returnVal) {
    retVal  = returnVal;
    quiting = true;
}

bool MenuBase::doEventsUntil(const dune::dune_clock::time_point until) {
    using namespace std::chrono_literals;

    SDL_Event event{};

    while (!quiting) {
        const auto remaining = until - dune::dune_clock::now();

        if (remaining <= dune::dune_clock::duration::zero() || remaining >= 32ms)
            return true;

        if (SDL_WaitEventTimeout(&event, dune::as_milliseconds<int>(remaining))) {
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

    quiting = false;

    while (!quiting) {
        const auto frameStart = dune::dune_clock::now();

        update();

        if (pNetworkManager != nullptr) {
            pNetworkManager->update();
        }

        if (quiting) {
            return retVal;
        }

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

        if (!settings.video.frameLimit)
            continue;

        if (!doEventsUntil(frameStart + 32ms))
            break;
    }

    return retVal;
}

void MenuBase::draw() {
    const SDL_Rect clipRect{getPosition().x, getPosition().y, getSize().x, getSize().y};
    SDL_RenderSetClipRect(renderer, &clipRect);

    Window::draw();

    drawSpecificStuff();

    Window::drawOverlay();

    SDL_RenderSetClipRect(renderer, nullptr);
}

void MenuBase::drawSpecificStuff() { }

bool MenuBase::doInput(SDL_Event& event) {
    switch (event.type) {
        case SDL_KEYDOWN: {
            // Look for a keypress
            switch (event.key.keysym.sym) {

                case SDLK_ESCAPE: {
                    if (pChildWindow == nullptr && bAllowQuiting) {
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
                } // fall through

                case SDLK_PRINTSCREEN:
                case SDLK_SYSREQ: {
                    SaveScreenshot();
                } break;

                case SDLK_TAB: {
                    if (SDL_GetModState() & KMOD_ALT) {
                        SDL_MinimizeWindow(window);
                    }
                } break;

                default: {
                } break;
            }
        } break;

        case SDL_MOUSEMOTION: {
            const SDL_MouseMotionEvent* mouse = &event.motion;
            drawnMouseX                       = std::max(0, std::min(mouse->x, settings.video.width - 1));
            drawnMouseY                       = std::max(0, std::min(mouse->y, settings.video.height - 1));
        } break;

        case SDL_QUIT: {
            if (pChildWindow == nullptr && bAllowQuiting) {
                quit();
            }
        } break;

        default: {
        } break;
    }

    handleInput(event);

    return !quiting;
}
