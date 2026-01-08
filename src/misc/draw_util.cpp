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

#include <misc/draw_util.h>

#include "misc/DrawingRectHelper.h"
#include <misc/exceptions.h>
#include <misc/sdl_support.h>

#include <globals.h>

#include <cstddef>
#include <mutex>

uint32_t getPixel(SDL_Surface* surface, int x, int y) {
    // SDL3: surface->format is now SDL_PixelFormat (enum), need SDL_GetPixelFormatDetails for bytes_per_pixel
    const auto* formatDetails = SDL_GetPixelFormatDetails(surface->format);
    const int bpp             = formatDetails->bytes_per_pixel;
    /* Here p is the address to the pixel we want to retrieve */
    auto* const p = static_cast<uint8_t*>(surface->pixels) + static_cast<ptrdiff_t>(y) * surface->pitch
                  + static_cast<ptrdiff_t>(x) * bpp;

    switch (bpp) {
        case 1: return *p;

        case 2: return *reinterpret_cast<uint16_t*>(p);

        case 3: {
            if constexpr (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                return p[0] << 16 | p[1] << 8 | p[2];
            }
            return p[0] | p[1] << 8 | p[2] << 16;
        }

        case 4: {
            const auto value = *reinterpret_cast<uint32_t*>(p);
            uint8_t r        = 0;
            uint8_t g        = 0;
            uint8_t b        = 0;
            uint8_t a        = 0;
            // SDL3: SDL_GetRGBA now takes SDL_PixelFormatDetails* and SDL_Palette*
            SDL_GetRGBA(value, formatDetails, nullptr, &r, &g, &b, &a);
            return COLOR_RGBA(r, g, b, a);
        }
        default: THROW(std::runtime_error, "getPixel(): Invalid bpp value!");
    }
}

void putPixel(SDL_Surface* surface, int x, int y, uint32_t color) {
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h)
        return;

    // SDL3: surface->format is now SDL_PixelFormat (enum)
    const auto* formatDetails = SDL_GetPixelFormatDetails(surface->format);
    const int bpp             = formatDetails->bytes_per_pixel;
    /* Here p is the address to the pixel want to set */
    auto* const p = static_cast<uint8_t*>(surface->pixels) + static_cast<ptrdiff_t>(y) * surface->pitch
                  + static_cast<ptrdiff_t>(x) * bpp;

    switch (bpp) {
        case 1: *p = static_cast<uint8_t>(color); break;

        case 2: *reinterpret_cast<uint16_t*>(p) = static_cast<uint16_t>(color); break;

        case 3: {
            if constexpr (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = color >> 16 & 0xff;
                p[1] = color >> 8 & 0xff;
                p[2] = color & 0xff;
            }
            p[0] = color & 0xff;
            p[1] = color >> 8 & 0xff;
            p[2] = color >> 16 & 0xff;
        } break;

        case 4: *reinterpret_cast<uint32_t*>(p) = MapRGBA(surface->format, color); break;
        default: THROW(std::runtime_error, "putPixel(): Invalid bpp value!");
    }
}

void drawHLineNoLock(SDL_Surface* surface, int x1, int y, int x2, uint32_t color) {
    auto min = x1;
    auto max = x2;

    if (min > max) {
        std::swap(min, max);
    }

    for (auto i = min; i <= max; i++) {
        putPixel(surface, i, y, color);
    }
}

void drawVLineNoLock(SDL_Surface* surface, int x, int y1, int y2, uint32_t color) {
    auto min = y1;
    auto max = y2;

    if (min > max) {
        std::swap(min, max);
    }

    for (auto i = min; i <= max; i++) {
        putPixel(surface, x, i, color);
    }
}

void drawHLine(SDL_Surface* surface, int x1, int y, int x2, uint32_t color) {
    sdl2::surface_lock lock{surface};

    drawHLineNoLock(surface, x1, y, x2, color);
}

void drawVLine(SDL_Surface* surface, int x, int y1, int y2, uint32_t color) {
    sdl2::surface_lock lock{surface};

    drawVLineNoLock(surface, x, y1, y2, color);
}

void drawRect(SDL_Surface* surface, int x1, int y1, int x2, int y2, uint32_t color) {
    sdl2::surface_lock lock{surface};

    drawRectNoLock(surface, x1, y1, x2, y2, color);
}

void drawRectNoLock(SDL_Surface* surface, int x1, int y1, int x2, int y2, uint32_t color) {
    int min = x1;
    int max = x2;

    if (min > max) {
        std::swap(min, max);
    }

    for (auto i = min; i <= max; i++) {
        putPixel(surface, i, y1, color);
        putPixel(surface, i, y2, color);
    }

    min = y1 + 1;
    max = y2;
    if (min > max) {
        std::swap(min, max);
    }

    for (auto j = min; j < max; j++) {
        putPixel(surface, x1, j, color);
        putPixel(surface, x2, j, color);
    }
}

sdl2::surface_ptr renderReadSurface(SDL_Renderer* renderer) {
    auto w = 0;
    auto h = 0;
    if (!SDL_GetRenderOutputSize(renderer, &w, &h)) {
        sdl2::log_warn("Warning: renderReadSurface() output size failed: {}", SDL_GetError());
        return nullptr;
    }

    // SDL3: SDL_RenderReadPixels now returns an SDL_Surface* directly
    sdl2::surface_ptr pScreen{SDL_RenderReadPixels(renderer, nullptr)};
    if (pScreen == nullptr) {
        sdl2::log_warn("Warning: renderReadSurface() read pixels failed: {}", SDL_GetError());
        return nullptr;
    }

    // Note: The SDL2 OpenGL bug workaround is no longer needed in SDL3

    return pScreen;
}

void replaceColor(SDL_Surface* surface, uint32_t oldColor, uint32_t newColor) {
    sdl2::surface_lock lock{surface};

    for (auto y = 0; y < surface->h; y++) {
        for (auto x = 0; x < surface->w; ++x) {
            const auto color = getPixel(surface, x, y);
            if (color == oldColor) {
                putPixel(surface, x, y, newColor);
            }
        }
    }
}

void mapColor(SDL_Surface* surface, const uint8_t colorMap[256]) {
    sdl2::surface_lock lock{surface};

    for (auto y = 0; y < surface->h; y++) {
        uint8_t* RESTRICT p = static_cast<uint8_t*>(surface->pixels) + static_cast<ptrdiff_t>(y) * surface->pitch;

        for (auto x = 0; x < surface->w; ++x, ++p) {
            *p = colorMap[*p];
        }
    }
}

sdl2::surface_ptr copySurface(SDL_Surface* inSurface) {
    // SDL3: SDL_ConvertSurface(s, s->format) for indexed surfaces does *not* preserve
    // pixel indices: it creates a new surface with a default (grayscale) palette and
    // remaps pixels via nearest-RGB matching. Use SDL_DuplicateSurface for a verbatim
    // clone (preserves pixels and copies the palette).
    sdl2::surface_ptr surface{SDL_DuplicateSurface(inSurface)};
    if (surface == nullptr) {
        THROW(std::invalid_argument, "copySurface(): SDL_DuplicateSurface() failed: {}", SDL_GetError());
    }

    copySurfaceAttributes(surface.get(), inSurface);

    return surface;
}

sdl2::surface_ptr convertSurfaceToDisplayFormat(SDL_Surface* inSurface) {
    // SDL3: SDL_ConvertSurface takes (surface, format) - no flags
    sdl2::surface_ptr pSurface{SDL_ConvertSurface(inSurface, SCREEN_FORMAT)};
    if (pSurface == nullptr) {
        THROW(std::invalid_argument, "convertSurfaceToDisplayFormat(): SDL_ConvertSurface() failed: ", SDL_GetError());
    }

    return pSurface;
}

sdl2::texture_ptr convertSurfaceToTexture(SDL_Surface* inSurface) {
    if (inSurface == nullptr) {
        return nullptr;
    }

    if (inSurface->w <= 0 || inSurface->h <= 0) {
        return nullptr;
    }

    if (inSurface->w > 2048 || inSurface->h > 2048) {
        sdl2::log_info("Warning: Size of texture created in convertSurfaceToTexture is {}x{}; may exceed hardware "
                       "limits on older GPUs!",
                       inSurface->w,
                       inSurface->h);
    }

    // SDL3: If surface has a color key, convert to RGBA format first to properly handle transparency
    sdl2::surface_ptr convertedSurface;
    SDL_Surface* surfaceToUse = inSurface;

    if (SDL_SurfaceHasColorKey(inSurface)) {
        // Convert to RGBA format - this properly converts color key to alpha=0
        convertedSurface.reset(SDL_ConvertSurface(inSurface, SDL_PIXELFORMAT_ARGB8888));
        if (convertedSurface) {
            surfaceToUse = convertedSurface.get();
        }
    }

    sdl2::texture_ptr pTexture{SDL_CreateTextureFromSurface(dune::globals::renderer.get(), surfaceToUse)};

    if (pTexture == nullptr) {
        THROW(std::invalid_argument,
              "convertSurfaceToTexture(): SDL_CreateTextureFromSurface() failed: {}",
              SDL_GetError());
    }

    SDL_BlendMode blendMode;
    SDL_GetSurfaceBlendMode(inSurface, &blendMode);

    if (blendMode == SDL_BLENDMODE_NONE && SDL_SurfaceHasColorKey(inSurface))
        blendMode = SDL_BLENDMODE_BLEND;

    if (blendMode != SDL_BLENDMODE_NONE) {
        if (!SDL_SetTextureBlendMode(pTexture.get(), blendMode)) {
            THROW(std::invalid_argument,
                  "convertSurfaceToTexture(): SDL_SetTextureBlendMode() failed: {}",
                  SDL_GetError());
        }
    }

    return pTexture;
}

void copySurfaceAttributes(SDL_Surface* target, SDL_Surface* source) {
    // SDL3: Use SDL_GetPixelFormatDetails for bits_per_pixel
    const auto* srcDetails = SDL_GetPixelFormatDetails(source->format);
    const auto* tgtDetails = SDL_GetPixelFormatDetails(target->format);

    // SDL3: Use SDL_GetSurfacePalette instead of surface->format->palette
    SDL_Palette* srcPalette = SDL_GetSurfacePalette(source);

    if (srcDetails->bits_per_pixel == 8 && tgtDetails->bits_per_pixel == 8 && srcPalette) {
        // SDL3: Ensure target has a palette using the consolidated helper
        SDL_Palette* tgtPalette = ensureSurfacePalette(target, srcPalette->ncolors);

        if (tgtPalette) {
            if (!SDL_SetPaletteColors(tgtPalette, srcPalette->colors, 0, srcPalette->ncolors)) {
                THROW(std::runtime_error, "copySurfaceAttributes(): unable to copy palette: {}", SDL_GetError());
            }
        }
    }

    if (SDL_SurfaceHasColorKey(source)) {
        uint32_t ckey = 0;
        if (!SDL_GetSurfaceColorKey(source, &ckey)) {
            THROW(std::runtime_error, "copySurfaceAttributes(): SDL_GetSurfaceColorKey() failed: {}", SDL_GetError());
        }

        if (!SDL_SetSurfaceColorKey(target, true, ckey)) {
            THROW(std::runtime_error, "copySurfaceAttributes(): SDL_SetSurfaceColorKey() failed: {}", SDL_GetError());
        }
    }

    // SDL3: SDL_RLEACCEL is no longer a surface flag, use SDL_SurfaceHasRLE
    if (SDL_SurfaceHasRLE(source))
        SDL_SetSurfaceRLE(target, true);
}

sdl2::surface_ptr scaleSurface(SDL_Surface* surf, double ratio) {

    const auto X2 = static_cast<int>(surf->w * ratio);
    const auto Y2 = static_cast<int>(surf->h * ratio);

    auto scaled = createSurface(surf, X2, Y2);

    if (!scaled)
        return nullptr;

    sdl2::surface_lock lock_scaled{scaled.get()};
    sdl2::surface_lock lock_surf{surf};

    for (int x = 0; x < X2; ++x)
        for (int y = 0; y < Y2; ++y)
            putPixel(scaled.get(), x, y, getPixel(surf, static_cast<int>(x / ratio), static_cast<int>(y / ratio)));

    return scaled;
}

sdl2::surface_ptr getSubPicture(SDL_Surface* pic, int left, int top, int width, int height) {
    if (pic == nullptr) {
        THROW(std::invalid_argument, "getSubPicture(): pic == nullptr!");
    }

    const SDL_Rect rect{left, top, width, height};

    return cloneSurface(pic, &rect);
}

sdl2::surface_ptr getSubFrame(SDL_Surface* pic, int i, int j, int numX, int numY) {
    if (pic == nullptr) {
        THROW(std::invalid_argument, "getSubFrame(): pic == nullptr!");
    }

    const auto frameWidth  = pic->w / numX;
    const auto frameHeight = pic->h / numY;

    return getSubPicture(pic, frameWidth * i, frameHeight * j, frameWidth, frameHeight);
}

sdl2::surface_ptr combinePictures(SDL_Surface* basePicture, SDL_Surface* topPicture, int x, int y) {
    if (basePicture == nullptr || topPicture == nullptr) {
        return nullptr;
    }

    auto dest{copySurface(basePicture)};
    if (dest == nullptr) {
        return nullptr;
    }

    auto destRect = calcDrawingRect(topPicture, x, y);
    SDL_BlitSurface(topPicture, nullptr, dest.get(), &destRect);

    return dest;
}

sdl2::surface_ptr rotateSurfaceLeft(SDL_Surface* inputPic) {
    if (inputPic == nullptr) {
        THROW(std::invalid_argument, "rotateSurface(): inputPic == nullptr!");
    }

    // create new picture surface
    // note the swapped height/width since we are rotating
    auto returnPic{createSurface(inputPic, inputPic->h, inputPic->w)};

    if (returnPic == nullptr) {
        THROW(std::runtime_error, "rotateSurface(): Cannot create new Picture!");
    }

    const sdl2::surface_lock lock_pic{returnPic.get()};
    const sdl2::surface_lock lock_input{inputPic};

    const auto* const RESTRICT input_pixels = static_cast<char*>(lock_input.pixels());
    auto* const RESTRICT return_pixels      = static_cast<char*>(lock_pic.pixels());

    // Now we can copy pixel by pixel
    for (auto y = 0; y < inputPic->h; ++y) {
        for (auto x = 0; x < inputPic->w; ++x) {
            return_pixels[(returnPic->h - x - 1) * lock_pic.pitch() + y] = input_pixels[y * lock_input.pitch() + x];
        }
    }

    return returnPic;
}

sdl2::surface_ptr rotateSurfaceRight(SDL_Surface* inputPic) {
    if (inputPic == nullptr) {
        THROW(std::invalid_argument, "rotateSurface(): inputPic == nullptr!");
    }

    // create new picture surface
    // note the swapped height/width since we are rotating
    auto returnPic{createSurface(inputPic, inputPic->h, inputPic->w)};
    if (returnPic == nullptr) {
        THROW(std::runtime_error, "rotateSurface(): Cannot create new Picture!");
    }

    const sdl2::surface_lock lock_pic{returnPic.get()};
    const sdl2::surface_lock lock_input{inputPic};

    const auto* const RESTRICT input_pixels = static_cast<char*>(lock_input.pixels());
    auto* const RESTRICT return_pixels      = static_cast<char*>(lock_pic.pixels());

    // Now we can copy pixel by pixel
    for (auto y = 0; y < inputPic->h; ++y) {
        for (auto x = 0; x < inputPic->w; ++x) {
            return_pixels[x * lock_pic.pitch() + (returnPic->w - y - 1)] = input_pixels[y * inputPic->pitch + x];
        }
    }

    return returnPic;
}

sdl2::surface_ptr flipHSurface(SDL_Surface* inputPic) {
    if (inputPic == nullptr) {
        THROW(std::invalid_argument, "flipHSurface(): inputPic == nullptr!");
    }

    // create new picture surface
    auto returnPic = createSurface(inputPic);
    if (returnPic == nullptr) {
        THROW(std::runtime_error, "flipHSurface(): Cannot create new Picture!");
    }

    sdl2::surface_lock lock_pic{returnPic.get()};
    sdl2::surface_lock lock_input{inputPic};

    // Now we can copy pixel by pixel
    for (auto y = 0; y < inputPic->h; y++) {
        for (auto x = 0; x < inputPic->w; x++) {
            putPixel(returnPic.get(), x, inputPic->h - y - 1, getPixel(inputPic, x, y));
        }
    }

    return returnPic;
}

sdl2::surface_ptr flipVSurface(SDL_Surface* inputPic) {
    if (inputPic == nullptr) {
        THROW(std::invalid_argument, "flipHSurface(): inputPic == nullptr!");
    }

    // create new picture surface
    auto returnPic = createSurface(inputPic);
    if (returnPic == nullptr) {
        THROW(std::runtime_error, "flipVSurface(): Cannot create new Picture!");
    }

    sdl2::surface_lock lock_pic{returnPic.get()};
    sdl2::surface_lock lock_input{inputPic};

    // Now we can copy pixel by pixel
    for (auto y = 0; y < inputPic->h; y++) {
        for (auto x = 0; x < inputPic->w; x++) {
            putPixel(returnPic.get(), inputPic->w - x - 1, y, getPixel(inputPic, x, y));
        }
    }

    return returnPic;
}

sdl2::surface_ptr createShadowSurface(SDL_Surface* source) {
    if (source == nullptr) {
        THROW(std::invalid_argument, "createShadowSurface(): source == nullptr!");
    }

    // SDL3: Use SDL_DuplicateSurface to clone (preserves indices + palette for indexed
    // surfaces). SDL_ConvertSurface(src, src->format) would remap indices to a default
    // palette and corrupt the data.
    sdl2::surface_ptr retPic{SDL_DuplicateSurface(source)};

    if (retPic == nullptr) {
        THROW(std::runtime_error, "createShadowSurface(): Cannot copy image!");
    }

    // SDL3: Use SDL_GetPixelFormatDetails for bytes_per_pixel
    const auto* formatDetails = SDL_GetPixelFormatDetails(retPic->format);
    if (formatDetails->bytes_per_pixel == 1) {
        SDL_SetSurfaceBlendMode(retPic.get(), SDL_BLENDMODE_NONE);
    }

    const sdl2::surface_lock lock{retPic.get()};

    auto* const pixels = static_cast<uint8_t*>(lock.pixels());

    for (auto j = 0; j < retPic->h; ++j) {
        auto* const p = &pixels[static_cast<ptrdiff_t>(j * lock.pitch())];
        for (auto i = 0; i < retPic->w; ++i) {
            if (p[i] != PALCOLOR_TRANSPARENT) {
                p[i] = PALCOLOR_BLACK;
            }
        }
    }

    static constexpr SDL_Color transparent = {0, 0, 0, 128};
    // SDL3: Use SDL_GetSurfacePalette
    SDL_Palette* palette = SDL_GetSurfacePalette(retPic.get());
    if (palette) {
        SDL_SetPaletteColors(palette, &transparent, PALCOLOR_BLACK, 1);
    }

    return retPic;
}

sdl2::surface_ptr mapSurfaceColorRange(SDL_Surface* source, int srcColor, int destColor) {
    if (!source)
        THROW(std::runtime_error, "mapSurfaceColorRange(): Null source!");

    // SDL3: Use SDL_DuplicateSurface to clone (see copySurface for rationale).
    sdl2::surface_ptr retPic{SDL_DuplicateSurface(source)};

    if (!retPic)
        THROW(std::runtime_error, "mapSurfaceColorRange(): Cannot copy image!");

    // SDL3: Use SDL_GetPixelFormatDetails for bytes_per_pixel
    const auto* formatDetails = SDL_GetPixelFormatDetails(retPic->format);
    if (formatDetails->bytes_per_pixel == 1) {
        SDL_SetSurfaceBlendMode(retPic.get(), SDL_BLENDMODE_NONE);
    }

    const auto offset = static_cast<uint8_t>(srcColor - destColor);

    const sdl2::surface_lock lock{retPic.get()};

    const auto pitch            = static_cast<ptrdiff_t>(lock.pitch());
    auto* const RESTRICT pixels = static_cast<uint8_t*>(lock.pixels());

    for (auto y = 0; y < retPic->h; ++y) {
        auto* RESTRICT p = &pixels[y * pitch];

        for (auto x = 0; x < retPic->w; ++x, ++p) {
            if (*p >= srcColor && *p < srcColor + 7)
                *p -= offset;
        }
    }

    return retPic;
}

bool drawSurface(SDL_Surface* src, const SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect,
                 SDL_BlendMode blendMode) {
    SDL_BlendMode oldBlendMode;
    SDL_GetSurfaceBlendMode(src, &oldBlendMode);

    // Use the requested blend mode directly - the caller knows what they want.
    // If the caller passes SDL_BLENDMODE_NONE, they want direct pixel copying
    // (e.g., for atlas building where surfaces already have proper alpha).
    SDL_SetSurfaceBlendMode(src, blendMode);

    const auto ret = SDL_BlitSurface(src, srcrect, dst, dstrect);

    if (!ret)
        sdl2::log_warn("drawSurface was unable to blit surface: {}", SDL_GetError());

    SDL_SetSurfaceBlendMode(src, oldBlendMode);

    return ret;
}

sdl2::surface_ptr createSurface(SDL_Surface* model, int width, int height) {
    if (0 == width)
        width = model->w;
    if (0 == height)
        height = model->h;

    // SDL3: Use SDL_GetPixelFormatDetails for bits_per_pixel, and surface->format is now SDL_PixelFormat enum
    const auto* formatDetails = SDL_GetPixelFormatDetails(model->format);
    auto copy                 = sdl2::surface_ptr{SDL_CreateSurface(width, height, model->format)};

    if (!copy)
        return nullptr;

    copySurfaceAttributes(copy.get(), model);

    return copy;
}

sdl2::surface_ptr cloneSurface(SDL_Surface* surface, const SDL_Rect* srcrect) {
    sdl2::surface_ptr copy;

    if (srcrect) {
        copy = createSurface(surface, srcrect->w, srcrect->h);
    } else {
        copy = createSurface(surface);
    }

    if (!copy)
        return nullptr;

    if (!drawSurface(surface, srcrect, copy.get(), nullptr))
        return nullptr;

    return copy;
}

sdl2::surface_ptr createTiledSurface(SDL_Surface* tile, int width, int height) {
    // SDL3: SDL_CreateSurface takes (w, h, format)
    auto surface = sdl2::surface_ptr{SDL_CreateSurface(width, height, SCREEN_FORMAT)};

    if (surface == nullptr)
        THROW(std::runtime_error, "createTiledSurface: Cannot create new tiled surface!");

    const auto w = tile->w;
    const auto h = tile->h;

    SDL_Rect dest{0, 0, w, h};

    for (dest.y = 0; dest.y < height; dest.y += h) {
        for (dest.x = 0; dest.x < width; dest.x += w) {
            SDL_Rect tmpDest = dest;
            SDL_BlitSurface(tile, nullptr, surface.get(), &tmpDest);
        }
    }

    return surface;
}

sdl2::surface_ptr createIndexedSurface(int width, int height) {
    // SDL3: Create an 8-bit indexed surface
    sdl2::surface_ptr surface{SDL_CreateSurface(width, height, SDL_PIXELFORMAT_INDEX8)};

    if (!surface) {
        THROW(std::runtime_error, "createIndexedSurface(): SDL_CreateSurface() failed: {}", SDL_GetError());
    }

    // SDL3: Indexed surfaces don't automatically get a palette, so create one
    if (!ensureSurfacePalette(surface.get(), 256)) {
        THROW(std::runtime_error, "createIndexedSurface(): Failed to create palette: {}", SDL_GetError());
    }

    return surface;
}

SDL_Palette* ensureSurfacePalette(SDL_Surface* surface, int ncolors) {
    if (!surface) {
        return nullptr;
    }

    // Check if surface already has a palette
    SDL_Palette* palette = SDL_GetSurfacePalette(surface);
    if (palette) {
        return palette;
    }

    // SDL3: Indexed surfaces don't automatically get a palette, so create one
    SDL_Palette* newPalette = SDL_CreatePalette(ncolors);
    if (!newPalette) {
        return nullptr;
    }

    if (!SDL_SetSurfacePalette(surface, newPalette)) {
        SDL_DestroyPalette(newPalette);
        return nullptr;
    }

    SDL_DestroyPalette(newPalette); // Surface now owns a reference

    return SDL_GetSurfacePalette(surface);
}
