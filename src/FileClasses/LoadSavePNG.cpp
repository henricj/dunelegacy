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
        const auto endOffset = SDL_RWsize(RWop);
        if(endOffset <= 0) {
            THROW(std::runtime_error, "LoadPNG_RW(): Cannot determine size of this *.png-File!");
        }

        const auto filesize = static_cast<size_t>(endOffset);
        auto pFiledata = std::make_unique<unsigned char[]>(filesize);

        if(SDL_RWread(RWop, pFiledata.get(), filesize, 1) != 1) {
            THROW(std::runtime_error, "LoadPNG_RW(): Reading this *.png-File failed!");
        }


        auto error = lodepng_inspect(&width, &height, &lodePNGState, pFiledata.get(), filesize);
        if(error != 0) {
            THROW(std::runtime_error, "LoadPNG_RW(): Inspecting this *.png-File failed: " + std::string(lodepng_error_text(error)));
        }

        if(lodePNGState.info_png.color.colortype == LCT_PALETTE && lodePNGState.info_png.color.bitdepth == 8) {
            // read image into a palettized SDL_Surface

            // reset state
            lodepng_state_cleanup(&lodePNGState);
            lodepng_state_init(&lodePNGState);

            lodePNGState.decoder.color_convert = 0;     // do not perform any conversion

            unsigned char *lode_out = nullptr;
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

            auto *const colors = reinterpret_cast<SDL_Color*>(lodePNGState.info_png.color.palette);
            SDL_SetPaletteColors(pic->format->palette, colors, 0, lodePNGState.info_png.color.palettesize);

            sdl2::surface_lock pic_lock{pic.get()};

            const unsigned char * RESTRICT const image_out = reinterpret_cast<const unsigned char *>(pImageOut.get());  // NOLINT
            unsigned char * RESTRICT const pic_surface = static_cast<unsigned char *>(pic_lock.pixels()); // NOLINT

            // Now we can copy pixel by pixel
            if (pic->pitch == static_cast<int>(width)) {
                memcpy(pic_surface, image_out, height * width);
            } else for(unsigned int y = 0; y < height; y++) {
                const auto *const in = image_out + y * width;
                auto *const out = pic_surface + y * pic->pitch;

                //std::copy_n(in, width, out);
                memcpy(out, in, width);
            }

        } else {
            // decode to 32-bit RGBA raw image
            unsigned char *lode_out = nullptr;
            error = lodepng_decode32(&lode_out, &width, &height, pFiledata.get(), filesize);
            if(error != 0) {
                THROW(std::runtime_error, "LoadPNG_RW(): Decoding this *.png-File failed: " + std::string(lodepng_error_text(error)));
            }

            lodepng_ptr pImageOut{ lode_out };

            // create new picture surface
            pic = sdl2::surface_ptr{ SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA8888) };
            if(pic == nullptr) {
                THROW(std::runtime_error, "LoadPNG_RW(): SDL_CreateRGBSurface has failed!");
            }

            sdl2::surface_lock pic_lock{pic.get()};

            const Uint32 * RESTRICT const image_out = reinterpret_cast<const Uint32 *>(pImageOut.get());  // NOLINT
            unsigned char * RESTRICT const pic_surface = static_cast<unsigned char *>(pic_lock.pixels()); // NOLINT

            // Now we can copy pixel by pixel
            if (static_cast<int>(sizeof(Uint32) * width) == pic->pitch) {
                const auto *in = image_out;
                auto *out = reinterpret_cast<Uint32*>(pic_surface);
                for (auto x = 0u; x < height * width; ++x) {
                    *out++ = SDL_SwapLE32(*in++);
                }
            } else for(auto y = 0u; y < height; y++) {
                const auto *in = image_out + y * width;
                auto *out = reinterpret_cast<Uint32*>(pic_surface + y * pic->pitch);
                for(auto x = 0u; x < width; ++x)
                    *out++ = SDL_SwapLE32(*in++);
            }

        }

        pFiledata.reset();

        lodepng_state_cleanup(&lodePNGState);

        return pic;
    } catch (std::exception &e) {
        sdl2::log_info("%s", e.what());

        return nullptr;
    }
}

int SavePNG_RW(SDL_Surface* surface, SDL_RWops* RWop) {
    if(surface == nullptr) {
        return -1;
    }

    const unsigned int width = surface->w;
    const unsigned int height = surface->h;

    std::vector<unsigned char> image(width*height * 4);

    {
        sdl2::surface_lock lock{ surface };

        // Now we can copy pixel by pixel
        for(auto y = 0u; y < height; y++) {
            auto * RESTRICT out = image.data() + y * 4*width;
            for(auto x = 0u; x < width; x++) {
                const auto pixel = getPixel(surface, x, y);
                SDL_GetRGBA(pixel, surface->format, &out[0], &out[1], &out[2], &out[3]);
                out += 4;
            }
        }
    }

    unsigned char* ppngFile = nullptr;
    size_t pngFileSize = 0;

    const auto error = lodepng_encode32(&ppngFile, &pngFileSize, image.data(), width, height);
    if(error != 0) {
        sdl2::log_info("%s", lodepng_error_text(error));
        free(ppngFile);
        return -1;
    }

    lodepng_ptr ppngFile_ptr{ ppngFile };

    if(SDL_RWwrite(RWop, ppngFile_ptr.get(), 1, pngFileSize) != pngFileSize) {
        sdl2::log_info("%s", SDL_GetError());
        return -1;
    }

    return 0;
}
