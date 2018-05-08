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

#ifndef LOADSAVEPNG_H
#define LOADSAVEPNG_H

#include <misc/SDL2pp.h>

#define LoadPNG(file) SDL_LoadPNG_RW(sdl2::RWops_ptr{SDL_RWFromFile(file, "rb")}.get())
#define SavePNG(surface, file) SavePNG_RW(surface, sdl2::RWops_ptr{SDL_RWFromFile(file, "wb")}.get())


sdl2::surface_ptr LoadPNG_RW(SDL_RWops* RWop);

int SavePNG_RW(SDL_Surface* surface, SDL_RWops* RWop);

#endif // LOADSAVEPNG_H
