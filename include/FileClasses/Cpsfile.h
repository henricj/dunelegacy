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

#ifndef CPSFILE_H
#define CPSFILE_H

#include <SDL.h>
#include <SDL_rwops.h>

/**
    This function reads a cps-File from a SDL_RWop and returns it as a SDL_Surface. The SDL_RWops can be readonly but must support
    seeking. The returned SDL_Surface should be freed with SDL_FreeSurface() if no longer needed.
    \param  RWop    SDL_RWops to the cps-File. (can be readonly)
    \param  freesrc A non-zero value means it will automatically close/free the rwop for you.
    \return Picture in this CPS-File
*/
SDL_Surface * LoadCPS_RW(SDL_RWops* RWop, int freesrc);

//  unsigned char* Filedata;
//  Uint32 CpsFilesize;


#endif // CPSFILE_H
