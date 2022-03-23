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
#include <misc/exceptions.h>
#include <misc/sdl_support.h>

#include <globals.h>

#include <mutex>

uint32_t getPixel(SDL_Surface* surface, int x, int y) {
    const int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    auto* const p = static_cast<uint8_t*>(surface->pixels) + y * surface->pitch + x * bpp;

    switch (bpp) {
        case 1: return *p;

        case 2: return *reinterpret_cast<uint16_t*>(p);

        case 3: {
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                return p[0] << 16 | p[1] << 8 | p[2];
            }
            return p[0] | p[1] << 8 | p[2] << 16;
        }

        case 4: {
            const auto value = *reinterpret_cast<uint32_t*>(p);
            uint8_t r        = 0;

            uint8_t g = 0;

            uint8_t b = 0;

            uint8_t a = 0;
            SDL_GetRGBA(value, surface->format, &r, &g, &b, &a);
            return COLOR_RGBA(r, g, b, a);
        }
        default: THROW(std::runtime_error, "getPixel(): Invalid bpp value!");
    }
}

void putPixel(SDL_Surface* surface, int x, int y, uint32_t color) {
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h)
        return;

    const int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel want to set */
    auto* const p = static_cast<uint8_t*>(surface->pixels) + y * surface->pitch + x * bpp;

    switch (bpp) {
        case 1: *p = color; break;

        case 2: *reinterpret_cast<uint16_t*>(p) = color; break;

        case 3: {
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
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
    sdl2::surface_lock lock {surface};

    drawHLineNoLock(surface, x1, y, x2, color);
}

void drawVLine(SDL_Surface* surface, int x, int y1, int y2, uint32_t color) {
    sdl2::surface_lock lock {surface};

    drawVLineNoLock(surface, x, y1, y2, color);
}

void drawRect(SDL_Surface* surface, int x1, int y1, int x2, int y2, uint32_t color) {
    sdl2::surface_lock lock {surface};

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
    assert(renderer == ::renderer);

    auto w = 0;
    auto h = 0;
    if (SDL_GetRendererOutputSize(renderer, &w, &h)) {
        sdl2::log_warn("Warning: renderReadSurface() output size failed: %s", SDL_GetError());
        return nullptr;
    }

    sdl2::surface_ptr pScreen {SDL_CreateRGBSurface(0, w, h, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)};
    if (pScreen == nullptr) {
        sdl2::log_warn("Warning: renderReadSurface() create surface failed: %s", SDL_GetError());
        return nullptr;
    }

    { // Scope
        sdl2::surface_lock lock {pScreen.get()};

        if (0 != SDL_RenderReadPixels(renderer, nullptr, pScreen->format->format, pScreen->pixels, pScreen->pitch)) {
            sdl2::log_warn("Warning: renderReadSurface() copy failed: %s", SDL_GetError());
            return nullptr;
        }
    }

    // Fix bug in SDL2 OpenGL Backend (SDL bug #2740 and #3350) in SDL version <= 2.0.4

    static bool need_workaround;
    static std::once_flag flag;
    std::call_once(flag, [&] {
        need_workaround = false;

        SDL_version version;
        SDL_GetVersion(&version);

        if (version.major > 2 || version.minor > 0 || version.patch > 4)
            return;

        SDL_RendererInfo rendererInfo;
        SDL_GetRendererInfo(renderer, &rendererInfo);
        need_workaround = 0 == strcmp(rendererInfo.name, "opengl");
    });

    if (need_workaround) {
        if (SDL_GetRenderTarget(renderer) != nullptr)
            return flipHSurface(pScreen.get());
    }

    return pScreen;
}

void replaceColor(SDL_Surface* surface, uint32_t oldColor, uint32_t newColor) {
    sdl2::surface_lock lock {surface};

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
    sdl2::surface_lock lock {surface};

    for (auto y = 0; y < surface->h; y++) {
        uint8_t* RESTRICT p = static_cast<uint8_t*>(surface->pixels) + y * surface->pitch;

        for (auto x = 0; x < surface->w; ++x, ++p) {
            *p = colorMap[*p];
        }
    }
}

sdl2::surface_ptr copySurface(SDL_Surface* inSurface) {
    sdl2::surface_ptr surface {SDL_ConvertSurface(inSurface, inSurface->format, inSurface->flags)};
    if (surface == nullptr) {
        THROW(std::invalid_argument,
              std::string("copySurface(): SDL_ConvertSurface() failed: ") + std::string(SDL_GetError()));
    }

    copySurfaceAttributes(surface.get(), inSurface);

    return surface;
}

sdl2::surface_ptr convertSurfaceToDisplayFormat(SDL_Surface* inSurface) {

    sdl2::surface_ptr pSurface {SDL_ConvertSurfaceFormat(inSurface, SCREEN_FORMAT, 0)};
    if (pSurface == nullptr) {
        THROW(std::invalid_argument, std::string("convertSurfaceToDisplayFormat(): SDL_ConvertSurfaceFormat() failed: ")
                                         + std::string(SDL_GetError()));
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
        sdl2::log_info("Warning: Size of texture created in convertSurfaceToTexture is %dx%d; may exceed hardware "
                       "limits on older GPUs!",
                       inSurface->w, inSurface->h);
    }

    sdl2::texture_ptr pTexture {SDL_CreateTextureFromSurface(renderer, inSurface)};

    if (pTexture == nullptr) {
        THROW(std::invalid_argument, std::string("convertSurfaceToTexture(): SDL_CreateTextureFromSurface() failed: ")
                                         + std::string(SDL_GetError()));
    }

    SDL_BlendMode blendMode;
    SDL_GetSurfaceBlendMode(inSurface, &blendMode);

    if (blendMode == SDL_BlendMode::SDL_BLENDMODE_NONE && SDL_HasColorKey(inSurface))
        blendMode = SDL_BlendMode::SDL_BLENDMODE_BLEND;

    if (blendMode != SDL_BlendMode::SDL_BLENDMODE_NONE) {
        if (SDL_SetTextureBlendMode(pTexture.get(), blendMode)) {
            THROW(std::invalid_argument,
                  std::string("convertSurfaceToTexture(): SDL_CreateTextureFromSurface() failed: ")
                      + std::string(SDL_GetError()));
        }
    }

    return pTexture;
}

void copySurfaceAttributes(SDL_Surface* target, SDL_Surface* source) {
    if (source->format->BitsPerPixel == 8 && target->format->BitsPerPixel == 8 && source->format->palette) {
        if (SDL_SetPaletteColors(target->format->palette, source->format->palette->colors, 0,
                                 source->format->palette->ncolors)) {
            THROW(std::runtime_error,
                  "copySurfaceAttributes(): unable to copy palette: " + std::string(SDL_GetError()));
        }
    }

    if (const auto has_ckey = SDL_HasColorKey(source)) {
        uint32_t ckey = 0;
        if (SDL_GetColorKey(source, &ckey)) {
            THROW(std::runtime_error,
                  "copySurfaceAttributes(): SDL_GetColorKey() failed: " + std::string(SDL_GetError()));
        }

        if (SDL_SetColorKey(target, SDL_TRUE, ckey)) {
            THROW(std::runtime_error,
                  "copySurfaceAttributes(): SDL_SetColorKey() failed: " + std::string(SDL_GetError()));
        }
    }

    if (source->flags & SDL_RLEACCEL)
        SDL_SetSurfaceRLE(target, SDL_TRUE);
}

sdl2::surface_ptr scaleSurface(SDL_Surface* surf, double ratio) {

    const auto X2 = static_cast<int>(surf->w * ratio);
    const auto Y2 = static_cast<int>(surf->h * ratio);

    auto scaled = createSurface(surf, X2, Y2);

    if (!scaled)
        return nullptr;

    sdl2::surface_lock lock_scaled {scaled.get()};
    sdl2::surface_lock lock_surf {surf};

    for (int x = 0; x < X2; ++x)
        for (int y = 0; y < Y2; ++y)
            putPixel(scaled.get(), x, y, getPixel(surf, static_cast<int>(x / ratio), static_cast<int>(y / ratio)));

    return scaled;
}

sdl2::surface_ptr getSubPicture(SDL_Surface* pic, int left, int top, int width, int height) {
    if (pic == nullptr) {
        THROW(std::invalid_argument, "getSubPicture(): pic == nullptr!");
    }

    const SDL_Rect rect {left, top, width, height};

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

    auto dest {copySurface(basePicture)};
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
    auto returnPic {createSurface(inputPic, inputPic->h, inputPic->w)};

    if (returnPic == nullptr) {
        THROW(std::runtime_error, "rotateSurface(): Cannot create new Picture!");
    }

    const sdl2::surface_lock lock_pic {returnPic.get()};
    const sdl2::surface_lock lock_input {inputPic};

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
    auto returnPic {createSurface(inputPic, inputPic->h, inputPic->w)};
    if (returnPic == nullptr) {
        THROW(std::runtime_error, "rotateSurface(): Cannot create new Picture!");
    }

    const sdl2::surface_lock lock_pic {returnPic.get()};
    const sdl2::surface_lock lock_input {inputPic};

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

    sdl2::surface_lock lock_pic {returnPic.get()};
    sdl2::surface_lock lock_input {inputPic};

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

    sdl2::surface_lock lock_pic {returnPic.get()};
    sdl2::surface_lock lock_input {inputPic};

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

    sdl2::surface_ptr retPic {SDL_ConvertSurface(source, source->format, source->flags)};

    if (retPic == nullptr) {
        THROW(std::runtime_error, "createShadowSurface(): Cannot copy image!");
    }

    if (retPic->format->BytesPerPixel == 1) {
        SDL_SetSurfaceBlendMode(retPic.get(), SDL_BLENDMODE_NONE);
    }

    const sdl2::surface_lock lock {retPic.get()};

    auto* const pixels = static_cast<uint8_t*>(lock.pixels());

    for (auto j = 0; j < retPic->h; ++j) {
        auto* const p = &pixels[j * lock.pitch()];
        for (auto i = 0; i < retPic->w; ++i) {
            if (p[i] != PALCOLOR_TRANSPARENT) {
                p[i] = PALCOLOR_BLACK;
            }
        }
    }

    const SDL_Color transparent = {0, 0, 0, 128};
    SDL_SetPaletteColors(retPic->format->palette, &transparent, PALCOLOR_BLACK, 1);

    return retPic;
}

sdl2::surface_ptr mapSurfaceColorRange(SDL_Surface* source, int srcColor, int destColor) {
    if (!source)
        THROW(std::runtime_error, "mapSurfaceColorRange(): Null source!");

    sdl2::surface_ptr retPic {SDL_ConvertSurface(source, source->format, 0)};

    if (!source)
        THROW(std::runtime_error, "mapSurfaceColorRange(): Cannot copy image!");

    if (retPic->format->BytesPerPixel == 1) {
        SDL_SetSurfaceBlendMode(retPic.get(), SDL_BLENDMODE_NONE);
    }

    const sdl2::surface_lock lock {retPic.get()};

    auto* const pixels = static_cast<uint8_t*>(lock.pixels());

    for (auto y = 0; y < retPic->h; ++y) {
        auto* p = &pixels[y * lock.pitch()];
        for (auto x = 0; x < retPic->w; ++x, ++p) {
            if (*p >= srcColor && *p < srcColor + 7)
                *p -= srcColor - destColor;
        }
    }

    return retPic;
}

bool drawSurface(SDL_Surface* src, const SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect,
                 SDL_BlendMode blendMode) {
    SDL_BlendMode oldBlendMode;
    SDL_GetSurfaceBlendMode(src, &oldBlendMode);
    SDL_SetSurfaceBlendMode(src, blendMode);

    const auto ret = SDL_BlitSurface(src, srcrect, dst, dstrect);

    if (ret)
        sdl2::log_warn("drawSurface was unable to blit surface: %s", SDL_GetError());

    SDL_SetSurfaceBlendMode(src, oldBlendMode);

    return 0 == ret;
}

sdl2::surface_ptr createSurface(SDL_Surface* model, int width, int height) {
    if (0 == width)
        width = model->w;
    if (0 == height)
        height = model->h;

    auto copy = sdl2::surface_ptr {
        SDL_CreateRGBSurfaceWithFormat(0, width, height, model->format->BitsPerPixel, model->format->format)};

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
