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

#include "FileClasses/SaveTextureAsBmp.h"

#include "FileClasses/LoadSavePNG.h"
#include "misc/dune_sdlpp.h"
#include "misc/sdl_support.h"

namespace {
class RestoreRenderTarget final {
    SDL_Renderer* const renderer_;
    SDL_Texture* const texture_;

public:
    RestoreRenderTarget(SDL_Renderer* renderer) : renderer_(renderer), texture_(SDL_GetRenderTarget(renderer)) { }
    ~RestoreRenderTarget() { SDL_SetRenderTarget(renderer_, texture_); }
};
} // namespace

sdl2::surface_ptr CreateSurfaceFromTexture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* src) {
    // From https://stackoverflow.com/a/48176678 and https://stackoverflow.com/a/51238719

    SDL_PixelFormat format = SDL_PIXELFORMAT_RGBA32;

    int w = 0;
    int h = 0;

    /* Get information about texture we want to save */
    // SDL3: SDL_QueryTexture is removed, use SDL_GetTextureSize and SDL_GetTextureProperties
    {
        float fw = 0, fh = 0;
        if (!SDL_GetTextureSize(texture, &fw, &fh)) {
            sdl2::log_info("Failed querying texture size: {}\n", SDL_GetError());
            return {};
        }
        w = static_cast<int>(fw);
        h = static_cast<int>(fh);

        // Get format from properties
        SDL_PropertiesID props = SDL_GetTextureProperties(texture);
        if (props != 0) {
            format = static_cast<SDL_PixelFormat>(
                SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_FORMAT_NUMBER, SDL_PIXELFORMAT_RGBA32));
        }
    }

    if (src) {
        w = src->w;
        h = src->h;
    }

    // SDL3: SDL_CreateTexture now takes SDL_PixelFormat directly
    const auto ren_tex = sdl2::texture_ptr{SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_TARGET, w, h)};
    if (!ren_tex) {
        sdl2::log_info("Failed creating render texture: {}\n", SDL_GetError());
        return {};
    }

    RestoreRenderTarget rrt{renderer};

    /*
     * Initialize our canvas, then copy texture to a target whose pixel data we
     * can access
     */
    if (!SDL_SetRenderTarget(renderer, ren_tex.get())) {
        sdl2::log_info("Failed setting render target: {}\n", SDL_GetError());
        return {};
    }

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);

    SDL_BlendMode oldBlendMode;
    SDL_GetTextureBlendMode(texture, &oldBlendMode);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);

    // SDL3: SDL_RenderCopy is replaced by SDL_RenderTexture with SDL_FRect
    {
        SDL_FRect fsrc{}, *pfsrc = nullptr;
        if (src) {
            fsrc  = {static_cast<float>(src->x),
                     static_cast<float>(src->y),
                     static_cast<float>(src->w),
                     static_cast<float>(src->h)};
            pfsrc = &fsrc;
        }
        if (!SDL_RenderTexture(renderer, texture, pfsrc, nullptr)) {
            sdl2::log_info("Failed copying texture data: {}\n", SDL_GetError());
            return {};
        }
    }

    SDL_SetTextureBlendMode(texture, oldBlendMode);

    // SDL3: SDL_RenderReadPixels now returns an SDL_Surface* directly
    auto surface = sdl2::surface_ptr{SDL_RenderReadPixels(renderer, nullptr)};

    if (!surface) {
        sdl2::log_info("Failed reading pixel data: {}\n", SDL_GetError());
        return {};
    }

    return surface;
}

void SaveTextureAsBmp(SDL_Renderer* renderer, SDL_Texture* texture, std::filesystem::path filename) {
    const auto surface = CreateSurfaceFromTexture(renderer, texture, nullptr);

    if (!surface)
        return;

    /* Save result to an image */
    // SDL3: Need to convert u8string to char* for SDL_SaveBMP
    const std::string filepath = filename.string();
    if (!SDL_SaveBMP(surface.get(), filepath.c_str())) {
        sdl2::log_info("Failed saving image: {}\n", SDL_GetError());
        return;
    }

    sdl2::log_info("Saved texture as BMP to \"{}\"\n", filename.string());
}

void SaveTextureAsPng(SDL_Renderer* renderer, SDL_Texture* texture, std::filesystem::path filename) {
    const auto surface = CreateSurfaceFromTexture(renderer, texture, nullptr);

    if (!surface)
        return;

    /* Save result to an image */
    if (SavePNG(surface.get(), filename)) {
        sdl2::log_info("Failed saving image: {}\n", SDL_GetError());
        return;
    }

    sdl2::log_info("Saved texture as PNG to \"{}\"\n", filename.string());
}
