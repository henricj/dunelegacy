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

#include <FileClasses/Cpsfile.h>
#include <FileClasses/Decode.h>
#include <FileClasses/Palette.h>

#include <SDL.h>
#include <SDL_endian.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdexcept>

#define	SIZE_X	320
#define SIZE_Y	200

extern Palette palette;

SDL_Surface * LoadCPS_RW(SDL_RWops* RWop, int freesrc)
{
	if(RWop == NULL) {
		return NULL;
	}

    uint8_t* pFiledata = NULL;
    uint8_t* pImageOut = NULL;
    SDL_Surface *pic = NULL;

	try {
        Uint32 CpsFilesize = SDL_RWseek(RWop,0,SEEK_END);
        if(CpsFilesize <= 0) {
            throw std::runtime_error("LoadCPS_RW(): Cannot determine size of this *.cps-File!");
        }

        if(SDL_RWseek(RWop,0,SEEK_SET) != 0) {
            throw std::runtime_error("LoadCPS_RW(): Seeking in this *.cps-File failed!");
        }

        pFiledata = new uint8_t[CpsFilesize];

        if(SDL_RWread(RWop, pFiledata, CpsFilesize, 1) != 1) {
            throw std::runtime_error("LoadCPS_RW(): Reading this *.cps-File failed!");
        }

        uint16_t format = SDL_SwapLE16(*(uint16_t*)(pFiledata + 2));

        if(format != 0x0004) {
            throw std::runtime_error("LoadCPS_RW(): Only Format80 encoded *.cps-Files are supported!");
        }

        unsigned int SizeXTimeSizeY = SDL_SwapLE16(*((uint16_t*)(pFiledata + 4)));
        SizeXTimeSizeY += SDL_SwapLE16(*((uint16_t*)(pFiledata + 6)));

        if(SizeXTimeSizeY != SIZE_X * SIZE_Y) {
            throw std::runtime_error("LoadCPS_RW(): Images must be 320x200 pixels big!");
        }

        uint16_t PaletteSize = SDL_SwapLE16(*((uint16_t*)(pFiledata + 8)));

        pImageOut = new uint8_t[SIZE_X*SIZE_Y];
        memset(pImageOut, 0, SIZE_X*SIZE_Y);

        if(decode80(pFiledata + 10 + PaletteSize, pImageOut, 0) == -2) {
            throw std::runtime_error("LoadCPS_RW(): Decoding this *.cps-File failed!");
        }

        // create new picture surface
        if((pic = SDL_CreateRGBSurface(0,SIZE_X,SIZE_Y,8,0,0,0,0))== NULL) {
            throw std::runtime_error("LoadCPS_RW(): SDL_CreateRGBSurface has failed!");
        }

        palette.applyToSurface(pic);
        SDL_LockSurface(pic);

        //Now we can copy line by line
        for(int y = 0; y < SIZE_Y;y++) {
            memcpy(	((char*) (pic->pixels)) + y * pic->pitch , pImageOut + y * SIZE_X, SIZE_X);
        }

        SDL_UnlockSurface(pic);

	    delete [] pFiledata;
	    delete [] pImageOut;

        if(freesrc) {
            SDL_RWclose(RWop);
        }

        return pic;
	} catch (std::exception &e) {
		fprintf(stderr, "%s\n", e.what());

	    delete [] pFiledata;
	    delete [] pImageOut;

	    if(pic != NULL) {
            SDL_FreeSurface(pic);
	    }

        if(freesrc) {
            SDL_RWclose(RWop);
        }

        return NULL;
	}
}
