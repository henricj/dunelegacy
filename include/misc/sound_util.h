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

#include <string>
#include <SDL_mixer.h>

Mix_Chunk* concat2Chunks(Mix_Chunk* sound1, Mix_Chunk* sound2);
Mix_Chunk* concat3Chunks(Mix_Chunk* sound1, Mix_Chunk* sound2, Mix_Chunk* sound3);
Mix_Chunk* concat4Chunks(Mix_Chunk* sound1, Mix_Chunk* sound2, Mix_Chunk* sound3, Mix_Chunk* sound4);
Mix_Chunk* createEmptyChunk();
Mix_Chunk* createSilenceChunk(int length);
Mix_Chunk* getChunkFromFile(std::string filename);
Mix_Chunk* getChunkFromFile(std::string filename, std::string alternativeFilename);

#endif // SOUND_UTIL_H
