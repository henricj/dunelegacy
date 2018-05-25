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

#include <GUI/GUIStyle.h>

std::unique_ptr<GUIStyle> GUIStyle::currentGUIStyle;

GUIStyle::GUIStyle() {
    ;
}

GUIStyle::~GUIStyle() {
    ;
}

sdl2::surface_ptr GUIStyle::createEmptySurface(Uint32 width, Uint32 height, bool transparent)  {
    sdl2::surface_ptr pSurface = sdl2::surface_ptr{ SDL_CreateRGBSurface(0, width, height, 32, RMASK, GMASK, BMASK, AMASK) };

    if(!pSurface) {
        return nullptr;
    }
    SDL_FillRect(pSurface.get(), nullptr, COLOR_TRANSPARENT);
    SDL_SetColorKey(pSurface.get(), SDL_TRUE, COLOR_TRANSPARENT);

    return pSurface;
}
