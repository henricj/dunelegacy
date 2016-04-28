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

#include <FileClasses/Icnfile.h>
#include <FileClasses/Palette.h>

#include <SDL_endian.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdexcept>

#define SIZE_X  16
#define SIZE_Y  16

extern Palette palette;

/// Constructor
/**
    The constructor reads from icnRWop and mapRWop all data and saves them internal. The SDL_RWops can be readonly but
    must support seeking. Immediately after the Icnfile-Object is constructed both RWops can be closed. All data is saved
    in the class.
    \param  icnRWop SDL_RWops to the icn-File. (can be readonly)
    \param  mapRWop SDL_RWops to the map-File. (can be readonly)
    \param  freesrc A non-zero value means it will automatically close/free the icnRWop and mapRWop for you.
*/
Icnfile::Icnfile(SDL_RWops* icnRWop, SDL_RWops* mapRWop, int freesrc)
{
    pIcnFiledata = nullptr;

    if(icnRWop == nullptr) {
        if(freesrc && mapRWop != nullptr) SDL_RWclose(mapRWop);
        throw std::invalid_argument("Icnfile::Icnfile(): icnRWop == nullptr!");
    } else if(mapRWop == nullptr) {
        if(freesrc) SDL_RWclose(icnRWop);
        throw std::invalid_argument("Icnfile::Icnfile(): mapRWop == nullptr!");
    }

    uint8_t* pMapFiledata = nullptr;

    try {
        int icnFilesize = SDL_RWseek(icnRWop,0,SEEK_END);
        if(icnFilesize <= 0) {
            throw std::runtime_error("Icnfile::Icnfile(): Cannot determine size of this *.icn-File!");
        }
        pIcnFiledata = new uint8_t[icnFilesize];

        if(SDL_RWseek(icnRWop,0,SEEK_SET) != 0) {
            throw std::runtime_error("Icnfile::Icnfile(): Seeking in this *.icn-File failed!");
        }

        if(SDL_RWread(icnRWop, &pIcnFiledata[0], icnFilesize, 1) != 1) {
            throw std::runtime_error("Icnfile::Icnfile(): Reading this *.icn-File failed!");
        }


        int mapFilesize = SDL_RWseek(mapRWop,0,SEEK_END);
        if(mapFilesize <= 0) {
            throw std::runtime_error("Icnfile::Icnfile(): Cannot determine size of this *.map-File!");
        }

        pMapFiledata = new uint8_t[mapFilesize];

        if(SDL_RWseek(mapRWop,0,SEEK_SET) != 0) {
            throw std::runtime_error("Icnfile::Icnfile(): Seeking in this *.map-File failed!");
        }

        if(SDL_RWread(mapRWop, &pMapFiledata[0], mapFilesize, 1) != 1) {
            throw std::runtime_error("Icnfile::Icnfile(): Reading this *.map-File failed!");
        }

        // now we can start creating the Tilesetindex
        if(mapFilesize < 2) {
            throw std::runtime_error("Icnfile::Icnfile(): This *.map-File is too short!");
        }

        Uint16 numTilesets = SDL_SwapLE16( *((Uint16 *) pMapFiledata));

        if(mapFilesize < numTilesets * 2) {
            throw std::runtime_error("Icnfile::Icnfile(): This *.map-File is too short!");
        }

        // calculate size for all entries
        Uint16 index = SDL_SwapLE16( ((Uint16*) pMapFiledata)[0]);
        for(int i = 1; i < numTilesets; i++) {
            Uint16 tmp = SDL_SwapLE16( ((Uint16*) pMapFiledata)[i]);
            MapfileEntry newMapfileEntry;
            newMapfileEntry.numTiles = tmp - index;
            tilesets.push_back(newMapfileEntry);
            index = tmp;
        }
        MapfileEntry newMapfileEntry;
        newMapfileEntry.numTiles = (mapFilesize/2) - index;
        tilesets.push_back(newMapfileEntry);

        for(int i = 0; i < numTilesets; i++) {
            index = SDL_SwapLE16( ((Uint16*) pMapFiledata)[i]);

            if((unsigned int) mapFilesize < (index+tilesets[i].numTiles)*2 ) {
                throw std::runtime_error("Icnfile::Icnfile(): This *.map-File is too short!");
            }

            // now we can read in
            for(unsigned int j = 0; j < tilesets[i].numTiles; j++) {
                tilesets[i].tileIndices.push_back(SDL_SwapLE16( ((Uint16*) pMapFiledata)[index+j]));
            }
        }

        delete [] pMapFiledata;
        // reading MAP-File is now finished

        // check if we can access first section in ICN-File
        if(icnFilesize < 0x20) {
            throw std::runtime_error("Icnfile::Icnfile(): Invalid ICN-File: No SSET-Section found!\n");
        }

        SSET = pIcnFiledata+0x18;

        // check SSET-Section
        if(     (SSET[0] != 'S')
            ||  (SSET[1] != 'S')
            ||  (SSET[2] != 'E')
            ||  (SSET[3] != 'T')) {
            throw std::runtime_error("Icnfile::Icnfile(): Invalid ICN-File: No SSET-Section found!\n");
        }

        SSET_Length = SDL_SwapBE32( *((Uint32*) (SSET + 4))) - 8;
        SSET += 16;

        if(pIcnFiledata + icnFilesize < SSET + SSET_Length) {
            throw std::runtime_error("Icnfile::Icnfile(): Invalid ICN-File: SSET-Section is bigger than ICN-File!\n");
        }

        RPAL = SSET + SSET_Length;

        // check RPAL-Section
        if(     (RPAL[0] != 'R')
            ||  (RPAL[1] != 'P')
            ||  (RPAL[2] != 'A')
            ||  (RPAL[3] != 'L')) {
            throw std::runtime_error("Icnfile::Icnfile(): Invalid ICN-File: No RPAL-Section found!\n");
        }

        RPAL_Length = SDL_SwapBE32( *((Uint32*) (RPAL + 4)));
        RPAL += 8;

        if(pIcnFiledata + icnFilesize < RPAL + RPAL_Length) {
            throw std::runtime_error("Icnfile::Icnfile(): Invalid ICN-File: RPAL-Section is bigger than ICN-File!\n");
        }

        RTBL = RPAL + RPAL_Length;

        // check RTBL-Section
        if(     (RTBL[0] != 'R')
            ||  (RTBL[1] != 'T')
            ||  (RTBL[2] != 'B')
            ||  (RTBL[3] != 'L')) {
            throw std::runtime_error("Icnfile::Icnfile(): Invalid ICN-File: No RTBL-Section found!\n");
        }

        RTBL_Length = SDL_SwapBE32( *((Uint32*) (RTBL + 4)));
        RTBL += 8;

        if(pIcnFiledata + icnFilesize < RTBL + RTBL_Length) {
            throw std::runtime_error("Icnfile::Icnfile(): Invalid ICN-File: RTBL-Section is bigger than ICN-File!\n");
        }

        numFiles = SSET_Length / ((SIZE_X * SIZE_Y) / 2);

        if(RTBL_Length < numFiles) {
            throw std::runtime_error("Icnfile::Icnfile(): Invalid ICN-File: RTBL-Section is too small!\n");
        }

        if(freesrc) SDL_RWclose(icnRWop);
        if(freesrc) SDL_RWclose(mapRWop);
    } catch (std::exception&) {
        delete [] pMapFiledata;
        delete [] pIcnFiledata;

        if(freesrc) SDL_RWclose(icnRWop);
        if(freesrc) SDL_RWclose(mapRWop);
        throw;
    }
}

/// Destructor
/**
    Frees all memory.
*/
Icnfile::~Icnfile()
{
    delete [] pIcnFiledata;
}

/// Returns one tile in the icn-File
/**
    This method returns a SDL_Surface containing the nth tile/picture in the icn-File.
    The returned SDL_Surface should be freed with SDL_FreeSurface() if no longer needed.
    \param  indexOfFile specifies which tile/picture to return (zero based)
    \return nth tile/picture in this icn-File
*/
SDL_Surface* Icnfile::getPicture(Uint32 indexOfFile) {
    SDL_Surface * pic;

    if(indexOfFile >= numFiles) {
        return nullptr;
    }

    // check if palette is in range
    if(RTBL[indexOfFile] >= RPAL_Length / 16) {
        return nullptr;
    }

    unsigned char* palettestart = RPAL + (16 * RTBL[indexOfFile]);

    unsigned char * filestart = SSET + (indexOfFile * ((SIZE_X * SIZE_Y)/2));

    // create new picture surface
    if((pic = SDL_CreateRGBSurface(0,SIZE_X,SIZE_Y,8,0,0,0,0))== nullptr) {
        return nullptr;
    }

    palette.applyToSurface(pic);
    SDL_LockSurface(pic);

    //Now we can copy to surface
    unsigned char *dest = (unsigned char*) (pic->pixels);
    unsigned char pixel;
    for(int y = 0; y < SIZE_Y;y++) {
        for(int x = 0; x < SIZE_X; x+=2) {
            pixel = filestart[ (y*SIZE_X + x) / 2];
            pixel = pixel >> 4;
            dest[x] = palettestart[pixel];

            pixel = filestart[ (y*SIZE_X + x) / 2];
            pixel = pixel & 0x0F;
            dest[x+1] = palettestart[pixel];
        }
        dest += pic->pitch;
    }

    SDL_UnlockSurface(pic);

    return pic;
}

/// Returns an array of pictures in the icn-File
/**
    This method returns a SDL_Surface containing multiple tiles/pictures. Which tiles to include is specified by MapfileIndex. The
    MapfileIndex specifies the tileset. One tileset constists of multiple tiles of the icn-File.
    The last 3 parameters specify how to arrange the tiles:
     - If all 3 parameters are 0 then a "random" layout is choosen, which should look good.
     - If tilesX and tilesY is set to non-zero values then the result surface contains tilesX*tilesY tiles and this tilesN-times side by side.
     - If all there parameters are non-zero then the result surface is exactly in this arrangement.

    tilesX*tilesY*tilesN must always the number of tiles in this tileset. Otherwise nullptr is returned.<br><br>
    Example:
    \code
    Tileset = 10,11,12,13,14,15,16,17,18,19,20,21
    tilesX = 2; tilesY = 2; tilesN = 3

    returned picture:
     10 11 14 15 18 19
     12 13 16 17 20 21
    \endcode
    <br>
    The returned SDL_Surface should be freed with SDL_FreeSurface() if no longer needed.
    \param  mapfileIndex    specifies which tileset to use (zero based)
    \param  tilesX          how many tiles in x direction
    \param  tilesY          how many tiles in y direction
    \param  tilesN          how many tilesX*tilesY blocks in a row
    \return the result surface with tilesX*tilesY*tilesN tiles
*/
SDL_Surface* Icnfile::getPictureArray(Uint32 mapfileIndex, int tilesX, int tilesY, int tilesN) {
    SDL_Surface * pic;

    if(mapfileIndex >= tilesets.size()) {
        return nullptr;
    }

    if((tilesX == 0) && (tilesY == 0) && (tilesN == 0)) {
        // guest what is best
        int tmp = tilesets[mapfileIndex].numTiles;
        if(tmp == 24) {
            // special case (radar station and light factory)
            tilesX = 2;
            tilesY = 2;
            tilesN = 6;
        } else if((tmp % 9) == 0) {
            tilesX = 3;
            tilesY = 3;
            tilesN = tmp / 9;
        } else if((tmp % 6) == 0) {
            tilesX = 3;
            tilesY = 2;
            tilesN = tmp / 6;
        } else if((tmp % 4) == 0) {
            tilesX = 2;
            tilesY = 2;
            tilesN = tmp / 4;
        } else if((tmp>=40) && ((tmp % 5) == 0)) {
            tilesX = tmp/5;
            tilesY = 5;
            tilesN = 1;
        } else {
            tilesX = 1;
            tilesY = 1;
            tilesN = tmp;
        }

    } else if( ((tilesX == 0) || (tilesY == 0)) && (tilesN == 0)) {
        // not possible
        return nullptr;
    } else if((tilesX == 0) && (tilesY == 0) && (tilesN != 0)) {
        if(tilesets[mapfileIndex].numTiles % tilesN == 0) {
            // guest what is best
            int tmp = tilesets[mapfileIndex].numTiles / tilesN;
            if((tmp % 3) == 0) {
                tilesX = tmp/3;
                tilesY = 3;
            } else if((tmp % 2) == 0) {
                tilesX = tmp/2;
                tilesY = 2;
            } else {
                tilesX = tmp;
                tilesY = 1;
            }
        } else {
            // not possible
            return nullptr;
        }
    } else {
        if((unsigned int)tilesX*tilesY*tilesN != tilesets[mapfileIndex].numTiles) {
            return nullptr;
        }
    }

    // create new picture surface
    if((pic = SDL_CreateRGBSurface(0,SIZE_X*tilesX*tilesN,SIZE_Y*tilesY,8,0,0,0,0))== nullptr) {
        return nullptr;
    }

    palette.applyToSurface(pic);
    SDL_LockSurface(pic);

    int tileidx=0;
    for(int n = 0; n < tilesN; n++) {
        for(int y = 0; y < tilesY; y++) {
            for(int x = 0; x < tilesX; x++) {
                int IndexOfFile = tilesets[mapfileIndex].tileIndices[tileidx];

                // check if palette is in range
                if(RTBL[IndexOfFile] >= RPAL_Length / 16) {
                    SDL_UnlockSurface(pic);
                    SDL_FreeSurface(pic);
                    return nullptr;
                }

                unsigned char* palettestart = RPAL + (16 * RTBL[IndexOfFile]);
                unsigned char * filestart = SSET + (IndexOfFile * ((SIZE_X * SIZE_Y)/2));

                //Now we can copy to surface
                unsigned char *dest = (unsigned char*) (pic->pixels) + (pic->pitch)*y*SIZE_Y + (x+n*tilesX) * SIZE_X;
                unsigned char pixel;
                for(int y = 0; y < SIZE_Y;y++) {
                    for(int x = 0; x < SIZE_X; x+=2) {
                        pixel = filestart[ (y*SIZE_X + x) / 2];
                        pixel = pixel >> 4;
                        dest[x] = palettestart[pixel];

                        pixel = filestart[ (y*SIZE_X + x) / 2];
                        pixel = pixel & 0x0F;
                        dest[x+1] = palettestart[pixel];
                    }
                    dest += pic->pitch;
                }

                tileidx++;
            }
        }
    }

    SDL_UnlockSurface(pic);

    return pic;
}

/// Returns a row of pictures in the icn-File
/**
    This method returns a SDL_Surface containing multiple tiles/pictures. The returned surface contains all
    tiles from startIndex to endIndex.
    The returned SDL_Surface should be freed with SDL_FreeSurface() if no longer needed.
    \param  startIndex      The first tile to use
    \param  endIndex        The last tile to use
    \return the result surface with (endIndex-startIndex+1) tiles. nullptr on errors.
*/
SDL_Surface* Icnfile::getPictureRow(Uint32 startIndex, Uint32 endIndex) {
    SDL_Surface * pic;

    if((startIndex >= numFiles)||(endIndex >= numFiles)||(startIndex > endIndex)) {
        return nullptr;
    }

    Uint32 numTiles = endIndex - startIndex + 1;
    // create new picture surface
    if((pic = SDL_CreateRGBSurface(0,SIZE_X*numTiles,SIZE_Y,8,0,0,0,0))== nullptr) {
        return nullptr;
    }

    palette.applyToSurface(pic);
    SDL_LockSurface(pic);

    for(Uint32 i = 0; i < numTiles; i++) {
        int indexOfFile = i+startIndex;

        // check if palette is in range
        if(RTBL[indexOfFile] >= RPAL_Length / 16) {
            SDL_UnlockSurface(pic);
            SDL_FreeSurface(pic);
            return nullptr;
        }

        unsigned char* palettestart = RPAL + (16 * RTBL[indexOfFile]);
        unsigned char * filestart = SSET + (indexOfFile * ((SIZE_X * SIZE_Y)/2));

        //Now we can copy to surface
        unsigned char *dest = (unsigned char*) (pic->pixels) + i*SIZE_X;
        unsigned char pixel;
        for(int y = 0; y < SIZE_Y;y++) {
            for(int x = 0; x < SIZE_X; x+=2) {
                pixel = filestart[ (y*SIZE_X + x) / 2];
                pixel = pixel >> 4;
                dest[x] = palettestart[pixel];

                pixel = filestart[ (y*SIZE_X + x) / 2];
                pixel = pixel & 0x0F;
                dest[x+1] = palettestart[pixel];
            }
            dest += pic->pitch;
        }
    }

    SDL_UnlockSurface(pic);
    return pic;
}

/// Returns a row of pictures in the icn-File
/**
    This method returns a SDL_Surface containing multiple tiles/pictures. The returned surface contains all
    tiles specified be the parameters
    The returned SDL_Surface should be freed with SDL_FreeSurface() if no longer needed.
    \param  numTiles    the number of tiles that should be extrated.
    \return the result surface with all specified tiles in a row. nullptr on errors.
*/
SDL_Surface* Icnfile::getPictureRow2(unsigned int numTiles, ...) {
    SDL_Surface * pic;

    // create new picture surface
    if((pic = SDL_CreateRGBSurface(0,SIZE_X*numTiles,SIZE_Y,8,0,0,0,0))== nullptr) {
        return nullptr;
    }

    palette.applyToSurface(pic);
    SDL_LockSurface(pic);

    va_list arg_ptr;
    va_start(arg_ptr, numTiles);

    for(unsigned int i = 0; i < numTiles; i++) {
        unsigned int indexOfFile = va_arg( arg_ptr, unsigned int);

        if(indexOfFile >= numFiles) {
            SDL_UnlockSurface(pic);
            SDL_FreeSurface(pic);
            va_end(arg_ptr);
            return nullptr;
        }

        // check if palette is in range
        if(RTBL[indexOfFile] >= RPAL_Length / 16) {
            SDL_UnlockSurface(pic);
            SDL_FreeSurface(pic);
            va_end(arg_ptr);
            return nullptr;
        }

        unsigned char* palettestart = RPAL + (16 * RTBL[indexOfFile]);
        unsigned char * filestart = SSET + (indexOfFile * ((SIZE_X * SIZE_Y)/2));

        //Now we can copy to surface
        unsigned char *dest = (unsigned char*) (pic->pixels) + i*SIZE_X;
        unsigned char pixel;
        for(int y = 0; y < SIZE_Y;y++) {
            for(int x = 0; x < SIZE_X; x+=2) {
                pixel = filestart[ (y*SIZE_X + x) / 2];
                pixel = pixel >> 4;
                dest[x] = palettestart[pixel];

                pixel = filestart[ (y*SIZE_X + x) / 2];
                pixel = pixel & 0x0F;
                dest[x+1] = palettestart[pixel];
            }
            dest += pic->pitch;
        }
    }

    va_end(arg_ptr);

    SDL_UnlockSurface(pic);
    return pic;
}
