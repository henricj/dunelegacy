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

#include "misc/Fullscreen.h"

#include "misc/SDL2pp.h"

#include "globals.h"

#include <SDL2/SDL.h>

namespace {
bool pendingFullscreen = false;
}

void toggleFullscreen() {
    pendingFullscreen = !pendingFullscreen;
}

void updateFullscreen() {
    if (!pendingFullscreen)
        return;

    pendingFullscreen = false;

    auto* const window = dune::globals::window.get();

    const auto window_flags = SDL_GetWindowFlags(window);

    if (window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        // switch to windowed mode
        sdl2::log_info("Switching to windowed mode.");
        SDL_SetWindowFullscreen(window, window_flags & ~SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
        // switch to fullscreen mode
        sdl2::log_info("Switching to fullscreen mode.");

        SDL_SetWindowFullscreen(window, window_flags | SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}
