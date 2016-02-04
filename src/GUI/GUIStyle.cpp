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

GUIStyle* GUIStyle::currentGUIStyle = NULL;

GUIStyle::GUIStyle() {
	;
}

GUIStyle::~GUIStyle() {
	;
}

SDL_Surface* GUIStyle::createEmptySurface(Uint32 width, Uint32 height, bool transparent)  {
	SDL_Surface* pSurface;

	if((pSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 32, RMASK, GMASK, BMASK, AMASK)) == NULL) {
		return NULL;
	}
	SDL_FillRect(pSurface,NULL, COLOR_TRANSPARENT);
    SDL_SetAlpha(pSurface, 0, 0);
    SDL_SetColorKey(pSurface, SDL_SRCCOLORKEY, COLOR_TRANSPARENT);

	return pSurface;
}
