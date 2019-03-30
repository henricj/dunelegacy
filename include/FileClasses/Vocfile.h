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

#ifndef VOCFILE_H
#define VOCFILE_H

#include <misc/SDL2pp.h>
#include <SDL2/SDL_mixer.h>

/**
    Try to load a VOC from the RWop. Returns a pointer to Mix_Chunk.
    \param  rwop    The source SDL_RWops as a pointer. The sample is loaded from this VOC-File.
    \return a pointer to the sample as a Mix_Chunk. nullptr is returned on errors.
 */
sdl2::mix_chunk_ptr LoadVOC_RW(SDL_RWops* rwop);

#endif // VOCFILE_H

