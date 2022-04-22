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

#ifndef SAVETEXTUREASBMP_H
#define SAVETEXTUREASBMP_H

#pragma once

struct SDL_Renderer;
struct SDL_Texture;

void SaveTextureAsBmp(SDL_Renderer* renderer, SDL_Texture* texture, const char* filename);
void SaveTextureAsPng(SDL_Renderer* renderer, SDL_Texture* texture, const char* filename);

#endif // SAVETEXTUREASBMP_H
