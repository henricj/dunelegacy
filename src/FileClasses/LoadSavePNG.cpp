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

#include <Colors.h>
#include <FileClasses/LoadSavePNG.h>
#include <globals.h>
#include <misc/draw_util.h>
#include <misc/dune_localtime.h>
#include <misc/exceptions.h>
#include <misc/fnkdat.h>

#include <lodepng.h>

#include <optional>

struct free_deleter {
    void operator()(void* p) const { std::free(p); }
};

typedef std::unique_ptr<unsigned char, free_deleter> lodepng_ptr;

sdl2::surface_ptr LoadPNG_RW(SDL_RWops* RWop) {
    if (RWop == nullptr) {
        return nullptr;
    }

    unsigned int width  = 0;
    unsigned int height = 0;
    sdl2::surface_ptr pic;

    LodePNGState lodePNGState;
    lodepng_state_init(&lodePNGState);

    try {
        // read complete file into memory
        const auto endOffset = SDL_RWsize(RWop);
        if (endOffset <= 0) {
            THROW(std::runtime_error, "LoadPNG_RW(): Cannot determine size of this *.png-File!");
        }

        const auto filesize = static_cast<size_t>(endOffset);
        auto pFiledata      = std::make_unique<unsigned char[]>(filesize);

        if (SDL_RWread(RWop, pFiledata.get(), filesize, 1) != 1) {
            THROW(std::runtime_error, "LoadPNG_RW(): Reading this *.png-File failed!");
        }

        auto error = lodepng_inspect(&width, &height, &lodePNGState, pFiledata.get(), filesize);
        if (error != 0) {
            THROW(std::runtime_error,
                  "LoadPNG_RW(): Inspecting this *.png-File failed: " + std::string(lodepng_error_text(error)));
        }

        if (lodePNGState.info_png.color.colortype == LCT_PALETTE && lodePNGState.info_png.color.bitdepth == 8) {
            // read image into a palettized SDL_Surface

            // reset state
            lodepng_state_cleanup(&lodePNGState);
            lodepng_state_init(&lodePNGState);

            lodePNGState.decoder.color_convert = 0; // do not perform any conversion

            unsigned char* lode_out = nullptr;
            error = lodepng_decode(&lode_out, &width, &height, &lodePNGState, pFiledata.get(), filesize);
            if (error != 0) {
                THROW(std::runtime_error, "LoadPNG_RW(): Decoding this palletized *.png-File failed: "
                                              + std::string(lodepng_error_text(error)));
            }

            const lodepng_ptr pImageOut{lode_out};

            // create new picture surface
            pic = sdl2::surface_ptr{SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0)};
            if (pic == nullptr) {
                THROW(std::runtime_error, "LoadPNG_RW(): SDL_CreateRGBSurface has failed!");
            }

            const auto* const colors = reinterpret_cast<SDL_Color*>(lodePNGState.info_png.color.palette);
            SDL_SetPaletteColors(pic->format->palette, colors, 0, lodePNGState.info_png.color.palettesize);

            const sdl2::surface_lock pic_lock{pic.get()};

            const unsigned char* RESTRICT const image_out = pImageOut.get();                                // NOLINT
            unsigned char* RESTRICT const pic_surface     = static_cast<unsigned char*>(pic_lock.pixels()); // NOLINT

            // Now we can copy pixel by pixel
            if (pic->pitch == static_cast<int>(width)) {
                memcpy(pic_surface, image_out, height * width);
            } else
                for (unsigned int y = 0; y < height; y++) {
                    const auto* const in = image_out + y * width;
                    auto* const out      = pic_surface + y * pic->pitch;

                    // std::copy_n(in, width, out);
                    memcpy(out, in, width);
                }

        } else {
            // decode to 32-bit RGBA raw image
            unsigned char* lode_out = nullptr;
            error                   = lodepng_decode32(&lode_out, &width, &height, pFiledata.get(), filesize);
            if (error != 0) {
                THROW(std::runtime_error,
                      "LoadPNG_RW(): Decoding this *.png-File failed: " + std::string(lodepng_error_text(error)));
            }

            const lodepng_ptr pImageOut{lode_out};

            // create new picture surface
            pic = sdl2::surface_ptr{SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA8888)};
            if (pic == nullptr) {
                THROW(std::runtime_error, "LoadPNG_RW(): SDL_CreateRGBSurface has failed!");
            }

            const sdl2::surface_lock pic_lock{pic.get()};

            const uint32_t* RESTRICT const image_out  = reinterpret_cast<const uint32_t*>(pImageOut.get()); // NOLINT
            unsigned char* RESTRICT const pic_surface = static_cast<unsigned char*>(pic_lock.pixels());     // NOLINT

            // Now we can copy pixel by pixel
            if (static_cast<int>(sizeof(uint32_t) * width) == pic->pitch) {
                const auto* in = image_out;
                auto* out      = reinterpret_cast<uint32_t*>(pic_surface);
                for (auto x = 0u; x < height * width; ++x) {
                    *out++ = SDL_SwapLE32(*in++);
                }
            } else
                for (auto y = 0u; y < height; y++) {
                    const auto* in = image_out + y * width;
                    auto* out      = reinterpret_cast<uint32_t*>(pic_surface + y * pic->pitch);
                    for (auto x = 0u; x < width; ++x)
                        *out++ = SDL_SwapLE32(*in++);
                }
        }

        pFiledata.reset();

        lodepng_state_cleanup(&lodePNGState);

        return pic;
    } catch (std::exception& e) {
        sdl2::log_info("%s", e.what());

        return nullptr;
    }
}

int SavePNG_RW(SDL_Surface* surface, SDL_RWops* RWop) {
    if (surface == nullptr || nullptr == RWop) {
        return -1;
    }

    sdl2::surface_ptr surface_copy;

    if (surface->format->format != SDL_PIXELFORMAT_RGBA32) {
        surface_copy = sdl2::surface_ptr{SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0)};

        surface = surface_copy.get();
    }

    const unsigned int width  = surface->w;
    const unsigned int height = surface->h;

    unsigned char* ppngFile = nullptr;
    size_t pngFileSize      = 0;

    { // Scope
        sdl2::surface_lock lock{surface};

        const auto error = lodepng_encode32(&ppngFile, &pngFileSize, static_cast<const unsigned char*>(surface->pixels),
                                            width, height);
        if (error != 0) {
            sdl2::log_info("%s", lodepng_error_text(error));
            free(ppngFile);
            return -1;
        }
    }

    const lodepng_ptr ppngFile_ptr{ppngFile};

    if (SDL_RWwrite(RWop, ppngFile_ptr.get(), 1, pngFileSize) != pngFileSize) {
        sdl2::log_info("%s", SDL_GetError());
        return -1;
    }

    return 0;
}

std::tuple<bool, std::optional<std::filesystem::path>> SaveScreenshot() {
    const auto timeinfo0 = dune::dune_localtime();

    if (!timeinfo0.has_value()) {
        sdl2::log_warn("Saving screenshot failed: unable to get local time");
        return {false, std::nullopt};
    }

    const auto& timeinfo = timeinfo0.value();

    std::array<char, 128> buffer;
    const auto length = strftime(buffer.data(), buffer.size(), "screenshot/dunelegacy %F %H%M%S.png", &timeinfo);

    if (0 == length) {
        sdl2::log_warn("Saving screenshot failed: unable to format time");
        return {false, std::nullopt};
    }

    auto [ok, path] = fnkdat(buffer.data(), FNKDAT_USER | FNKDAT_CREAT);

    if (!ok) {
        sdl2::log_warn("Saving screenshot failed: unable to get path");
        return {false, std::nullopt};
    }

    { // Scope
        const sdl2::surface_ptr pCurrentScreen = renderReadSurface(renderer);

        if (!pCurrentScreen) {
            sdl2::log_warn("Saving screenshot failed: unable to copy screen");
            return {false, std::nullopt};
        }

        const auto saved_ok = 0 == SavePNG(pCurrentScreen.get(), path);

        if (saved_ok)
            sdl2::log_info("Saving screenshot to %s", path.u8string());
        else
            sdl2::log_warn("Saving screenshot to %s failed", path.u8string());

        return {saved_ok, path};
    }
}
