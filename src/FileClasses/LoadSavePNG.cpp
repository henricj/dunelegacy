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

struct free_deleter
{
    void operator()(void* p) const { std::free(p); }
};

typedef std::unique_ptr<unsigned char, free_deleter> lodepng_ptr;


sdl2::surface_ptr LoadPNG_RW(SDL_RWops* RWop) {
    if(RWop == nullptr) {
        return nullptr;
    }

    unsigned int width = 0;
    unsigned int height = 0;
    sdl2::surface_ptr pic;

    LodePNGState lodePNGState;
    lodepng_state_init(&lodePNGState);

    try {
        // read complete file into memory
        Sint64 endOffset = SDL_RWsize(RWop);
        if(endOffset <= 0) {
            THROW(std::runtime_error, "LoadPNG_RW(): Cannot determine size of this *.png-File!");
        }

        size_t filesize = static_cast<size_t>(endOffset);
        auto pFiledata = std::make_unique<unsigned char[]>(filesize);

        if(SDL_RWread(RWop, pFiledata.get(), filesize, 1) != 1) {
            THROW(std::runtime_error, "LoadPNG_RW(): Reading this *.png-File failed!");
        }


        unsigned int error = lodepng_inspect(&width, &height, &lodePNGState, pFiledata.get(), filesize);
        if(error != 0) {
            THROW(std::runtime_error, "LoadPNG_RW(): Inspecting this *.png-File failed: " + std::string(lodepng_error_text(error)));
        }

        if(lodePNGState.info_png.color.colortype == LCT_PALETTE && lodePNGState.info_png.color.bitdepth == 8) {
            // read image into a palettized SDL_Surface

            // reset state
            lodepng_state_cleanup(&lodePNGState);
            lodepng_state_init(&lodePNGState);

            lodePNGState.decoder.color_convert = 0;     // do not perform any conversion

            unsigned char *lode_out;
            error = lodepng_decode(&lode_out, &width, &height, &lodePNGState, pFiledata.get(), filesize);
            if(error != 0) {
                THROW(std::runtime_error, "LoadPNG_RW(): Decoding this palletized *.png-File failed: " + std::string(lodepng_error_text(error)));
            }

            lodepng_ptr pImageOut{ lode_out };

            // create new picture surface
            pic = sdl2::surface_ptr{ SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0) };
            if(pic == nullptr) {
                THROW(std::runtime_error, "LoadPNG_RW(): SDL_CreateRGBSurface has failed!");
            }

            SDL_Color* colors = (SDL_Color*) lodePNGState.info_png.color.palette;
            SDL_SetPaletteColors(pic->format->palette, colors, 0, lodePNGState.info_png.color.palettesize);

            sdl2::surface_lock pic_lock{pic.get()};

            const unsigned char * RESTRICT const image_out = reinterpret_cast<const unsigned char *>(pImageOut.get());  // NOLINT
            unsigned char * RESTRICT const pic_surface = static_cast<unsigned char *>(pic_lock.pixels()); // NOLINT

            // Now we can copy pixel by pixel
            if (pic->pitch == static_cast<int>(width)) {
                memcpy(pic_surface, image_out, height * width);
            } else {
                for(unsigned int y = 0; y < height; y++) {
                    const unsigned char* in = image_out + y * width;
                    unsigned char* out = pic_surface + y * pic->pitch;

                    //std::copy_n(in, width, out);
                    memcpy(out, in, width);
                }
            }

        } else {
            // decode to 32-bit RGBA raw image
            unsigned char *lode_out;
            error = lodepng_decode32(&lode_out, &width, &height, pFiledata.get(), filesize);
            if(error != 0) {
                THROW(std::runtime_error, "LoadPNG_RW(): Decoding this *.png-File failed: " + std::string(lodepng_error_text(error)));
            }

            lodepng_ptr pImageOut{ lode_out };

            // create new picture surface
            pic = sdl2::surface_ptr{ SDL_CreateRGBSurface(0, width, height, 32, RMASK, GMASK, BMASK, AMASK) };
            if(pic == nullptr) {
                THROW(std::runtime_error, "LoadPNG_RW(): SDL_CreateRGBSurface has failed!");
            }

            sdl2::surface_lock pic_lock{pic.get()};

            const unsigned char * RESTRICT const image_out = reinterpret_cast<const unsigned char *>(pImageOut.get());  // NOLINT
            unsigned char * RESTRICT const pic_surface = static_cast<unsigned char *>(pic_lock.pixels()); // NOLINT

            // Now we can copy pixel by pixel
            for(unsigned int y = 0; y < height; y++) {
                const unsigned char* in = image_out + y * 4*width;
                unsigned char* out = pic_surface + y * pic->pitch;
                for(unsigned int x = 0; x < width; x++) {
                    *((Uint32*) out) = SDL_SwapLE32(*((Uint32*) in));
                    in += 4;
                    out += 4;
                }
            }

        }

        pFiledata.reset();

        lodepng_state_cleanup(&lodePNGState);

        return pic;
    } catch (std::exception &e) {
        SDL_Log("%s", e.what());

        return nullptr;
    }
}

int SavePNG_RW(SDL_Surface* surface, SDL_RWops* RWop) {
    if(surface == nullptr) {
        return -1;
    }

    unsigned int width = surface->w;
    unsigned int height = surface->h;

    std::vector<unsigned char> image(width*height*4);

    {
        sdl2::surface_lock lock{ surface };

        // Now we can copy pixel by pixel
        for(unsigned int y = 0; y < height; y++) {
            unsigned char* out = image.data() + y * 4*width;
            for(unsigned int x = 0; x < width; x++) {
                Uint32 pixel = getPixel(surface, x, y);
                SDL_GetRGBA(pixel, surface->format, &out[0], &out[1], &out[2], &out[3]);
                out += 4;
            }
        }
    }

    unsigned char* ppngFile;
    size_t pngFileSize;

    unsigned int error = lodepng_encode32(&ppngFile, &pngFileSize, image.data(), width, height);
    if(error != 0) {
        SDL_Log("%s", lodepng_error_text(error));
        free(ppngFile);
        return -1;
    }

    if(SDL_RWwrite(RWop, ppngFile, 1, pngFileSize) != pngFileSize) {
        SDL_Log("%s", SDL_GetError());
        free(ppngFile);
        return -1;
    }

    free(ppngFile);
    return 0;
}
