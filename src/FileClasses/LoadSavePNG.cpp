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

#include <FileClasses/LoadSavePNG.h>
#include <FileClasses/lodepng.h>
#include <misc/draw_util.h>
#include <misc/exceptions.h>
#include <Colors.h>
#include <globals.h>

#include <stdio.h>

SDL_Surface* LoadPNG_RW(SDL_RWops* RWop, int freesrc) {
    if(RWop == nullptr) {
        return nullptr;
    }

    unsigned char* pFiledata = nullptr;
    unsigned char* pImageOut = nullptr;
    unsigned int width = 0;
    unsigned int height = 0;
    SDL_Surface *pic = nullptr;

    LodePNGState lodePNGState;
    lodepng_state_init(&lodePNGState);

    try {
        // read complete file into memory
        size_t filesize = SDL_RWseek(RWop,0,SEEK_END);
        if(filesize <= 0) {
            THROW(std::runtime_error, "LoadPNG_RW(): Cannot determine size of this *.png-File!");
        }

        if(SDL_RWseek(RWop,0,SEEK_SET) != 0) {
            THROW(std::runtime_error, "LoadPNG_RW(): Seeking in this *.png-File failed!");
        }

        pFiledata = (unsigned char*) malloc(filesize);

        if(SDL_RWread(RWop, pFiledata, filesize, 1) != 1) {
            THROW(std::runtime_error, "LoadPNG_RW(): Reading this *.png-File failed!");
        }


        unsigned int error = lodepng_inspect(&width, &height, &lodePNGState, pFiledata, filesize);
        if(error != 0) {
            THROW(std::runtime_error, "LoadPNG_RW(): Inspecting this *.png-File failed: " + std::string(lodepng_error_text(error)));
        }

        if(lodePNGState.info_png.color.colortype == LCT_PALETTE && lodePNGState.info_png.color.bitdepth == 8) {
            // read image into a palettized SDL_Surface

            // reset state
            lodepng_state_cleanup(&lodePNGState);
            lodepng_state_init(&lodePNGState);

            lodePNGState.decoder.color_convert = 0;     // do not perform any conversion

            error = lodepng_decode(&pImageOut, &width, &height, &lodePNGState, pFiledata, filesize);
            if(error != 0) {
                THROW(std::runtime_error, "LoadPNG_RW(): Decoding this palletized *.png-File failed: " + std::string(lodepng_error_text(error)));
            }

            // create new picture surface
            if((pic = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0)) == nullptr) {
                THROW(std::runtime_error, "LoadPNG_RW(): SDL_CreateRGBSurface has failed!");
            }

            SDL_Color* colors = (SDL_Color*) lodePNGState.info_png.color.palette;
            SDL_SetPaletteColors(pic->format->palette, colors, 0, lodePNGState.info_png.color.palettesize);

            SDL_LockSurface(pic);

            // Now we can copy pixel by pixel
            for(unsigned int y = 0; y < height; y++) {
                unsigned char* in = pImageOut + y * width;
                unsigned char* out = ((unsigned char*) pic->pixels) + y * pic->pitch;
                for(unsigned int x = 0; x < width; x++) {
                    *out = *in;
                    ++in;
                    ++out;
                }
            }

            SDL_UnlockSurface(pic);


        } else {
            // decode to 32-bit RGBA raw image
            error = lodepng_decode32(&pImageOut, &width, &height, pFiledata, filesize);
            if(error != 0) {
                THROW(std::runtime_error, "LoadPNG_RW(): Decoding this *.png-File failed: " + std::string(lodepng_error_text(error)));
            }


            // create new picture surface
            if((pic = SDL_CreateRGBSurface(0, width, height, 32, RMASK, GMASK, BMASK, AMASK)) == nullptr) {
                THROW(std::runtime_error, "LoadPNG_RW(): SDL_CreateRGBSurface has failed!");
            }

            SDL_LockSurface(pic);

            // Now we can copy pixel by pixel
            for(unsigned int y = 0; y < height; y++) {
                unsigned char* in = pImageOut + y * 4*width;
                unsigned char* out = ((unsigned char*) pic->pixels) + y * pic->pitch;
                for(unsigned int x = 0; x < width; x++) {
                    *((Uint32*) out) = SDL_SwapLE32(*((Uint32*) in));
                    in += 4;
                    out += 4;
                }
            }

            SDL_UnlockSurface(pic);

        }

        free(pFiledata);
        free(pImageOut);

        lodepng_state_cleanup(&lodePNGState);

        if(freesrc) {
            SDL_RWclose(RWop);
        }

        return pic;
    } catch (std::exception &e) {
        fprintf(stderr, "%s\n", e.what());

        free(pFiledata);
        free(pImageOut);

        if(pic != nullptr) {
            SDL_FreeSurface(pic);
        }

        if(freesrc) {
            SDL_RWclose(RWop);
        }

        return nullptr;
    }
}

int SavePNG_RW(SDL_Surface* surface, SDL_RWops* RWop, int freedst) {
    if(surface == nullptr) {
        if(freedst) {
            SDL_RWclose(RWop);
        }
        return -1;
    }

    unsigned int width = surface->w;
    unsigned int height = surface->h;

    unsigned char* pImage = (unsigned char*) malloc(width*height*4);

    SDL_LockSurface(surface);

    // Now we can copy pixel by pixel
    for(unsigned int y = 0; y < height; y++) {
        unsigned char* out = pImage + y * 4*width;
        for(unsigned int x = 0; x < width; x++) {
            Uint32 pixel = getPixel(surface, x, y);
            SDL_GetRGBA(pixel, surface->format, &out[0], &out[1], &out[2], &out[3]);
            out += 4;
        }
    }

    SDL_UnlockSurface(surface);


    unsigned char* ppngFile;
    size_t pngFileSize;

    unsigned int error = lodepng_encode32(&ppngFile, &pngFileSize, pImage, width, height);
    free(pImage);
    if(error != 0) {
        fprintf(stderr, "%s\n", lodepng_error_text(error));
        if(freedst) {
            SDL_RWclose(RWop);
        }
        return -1;
    }

    if(SDL_RWwrite(RWop, ppngFile, 1, pngFileSize) != pngFileSize) {
        fprintf(stderr, "%s\n", SDL_GetError());
        if(freedst) {
            SDL_RWclose(RWop);
        }
        return -1;
    }

    free(ppngFile);

    if(freedst) {
        SDL_RWclose(RWop);
    }
    return 0;
}
