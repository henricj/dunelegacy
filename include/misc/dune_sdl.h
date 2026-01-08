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

#ifndef DUNE_LEGACY_SDL_H
#define DUNE_LEGACY_SDL_H

/**
 * @file dune_sdl.h
 * @brief Centralized SDL include header for Dune Legacy.
 *
 * This header provides a single point of control for all SDL includes.
 * Do NOT include SDL headers directly in other source files.
 */

#include <SDL3/SDL.h>

// SDL2 to SDL3 compatibility macros for incremental migration.
// This header will be removed once migration is complete.
#include <misc/dune_sdl2to3.h>

#endif // DUNE_LEGACY_SDL_H
