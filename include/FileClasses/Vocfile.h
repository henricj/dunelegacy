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

#include <SDL.h>
#include <SDL_rwops.h>
#include <SDL_mixer.h>

/**
 Try to load a VOC from the RWop. Returns a pointer to Mix_Chunk.
 It is the callers responsibility to deallocate that data again later on
 with Mix_FreeChunk()!
	\param	rwop	The source SDL_RWops as a pointer. The sample is loaded from this VOC-File.
	\param	freesrc	A non-zero value means it will automatically close/free the src for you.
	\return	a pointer to the sample as a Mix_Chunk. NULL is returned on errors.
 */
extern Mix_Chunk* LoadVOC_RW(SDL_RWops* rwop, int freesrc);

#endif // VOCFILE_H

