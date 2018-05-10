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

#ifndef SOUND_UTIL_H
#define SOUND_UTIL_H

#include <misc/SDL2pp.h>
#include <string>
#include <SDL2/SDL_mixer.h>

sdl2::mix_chunk_ptr create_chunk();
sdl2::mix_chunk_ptr concat2Chunks(Mix_Chunk* sound1, Mix_Chunk* sound2);
sdl2::mix_chunk_ptr concat3Chunks(Mix_Chunk* sound1, Mix_Chunk* sound2, Mix_Chunk* sound3);
sdl2::mix_chunk_ptr concat4Chunks(Mix_Chunk* sound1, Mix_Chunk* sound2, Mix_Chunk* sound3, Mix_Chunk* sound4);
sdl2::mix_chunk_ptr createEmptyChunk();
sdl2::mix_chunk_ptr createSilenceChunk(int length);
sdl2::mix_chunk_ptr getChunkFromFile(const std::string& filename);
sdl2::mix_chunk_ptr getChunkFromFile(const std::string& filename, const std::string& alternativeFilename);

#endif // SOUND_UTIL_H
