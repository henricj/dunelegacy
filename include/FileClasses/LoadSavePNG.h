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

#include <filesystem>
#include <optional>
#include <tuple>

#include <misc/SDL2pp.h>

sdl2::surface_ptr LoadPNG_RW(SDL_RWops* RWop);

int SavePNG_RW(SDL_Surface* surface, SDL_RWops* RWop);

int SavePNG(SDL_Surface* surface, std::filesystem::path file);

std::tuple<bool, std::optional<std::filesystem::path>> SaveScreenshot();

#endif // LOADSAVEPNG_H
