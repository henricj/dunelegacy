#include "misc/SDL2pp.h"
#include <SDL2/SDL.h>
#include <misc/sdl_support.h>

#include "FileClasses/SaveTextureAsBmp.h"

namespace {
class RestoreRenderTarget final {
    SDL_Renderer* const renderer_;
    SDL_Texture* const texture_;

public:
    RestoreRenderTarget(SDL_Renderer* renderer) : renderer_(renderer), texture_(SDL_GetRenderTarget(renderer)) { }
    ~RestoreRenderTarget() { SDL_SetRenderTarget(renderer_, texture_); }
};
} // namespace

void SaveTextureAsBmp(SDL_Renderer* renderer, SDL_Texture* texture, const char* filename) {
    // From https://stackoverflow.com/a/48176678 and https://stackoverflow.com/a/51238719

    uint32_t format = SDL_PIXELFORMAT_RGBA32;

    int w = 0;
    int h = 0;

    /* Get information about texture we want to save */
    if (SDL_QueryTexture(texture, &format, nullptr, &w, &h)) {
        sdl2::log_info("Failed querying texture: %s\n", SDL_GetError());
        return;
    }

    const auto ren_tex = sdl2::texture_ptr {SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_TARGET, w, h)};
    if (!ren_tex) {
        sdl2::log_info("Failed creating render texture: %s\n", SDL_GetError());
        return;
    }

    RestoreRenderTarget rrt {renderer};

    /*
     * Initialize our canvas, then copy texture to a target whose pixel data we
     * can access
     */
    if (SDL_SetRenderTarget(renderer, ren_tex.get())) {
        sdl2::log_info("Failed setting render target: %s\n", SDL_GetError());
        return;
    }

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);

    SDL_BlendMode oldBlendMode;
    SDL_GetTextureBlendMode(texture, &oldBlendMode);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);

    if (SDL_RenderCopy(renderer, texture, nullptr, nullptr)) {
        sdl2::log_info("Failed copying texture data: %s\n", SDL_GetError());
        return;
    }

    SDL_SetTextureBlendMode(texture, oldBlendMode);

    const auto surface = sdl2::surface_ptr {SDL_CreateRGBSurfaceWithFormat(0, w, h, SDL_BYTESPERPIXEL(format), format)};

    { // Scope
        const sdl2::surface_lock lock {surface.get()};

        if (SDL_RenderReadPixels(renderer, nullptr, format, lock.pixels(), lock.pitch())) {
            sdl2::log_info("Failed reading pixel data: %s\n", SDL_GetError());
            return;
        }
    }

    /* Save result to an image */
    if (SDL_SaveBMP(surface.get(), filename)) {
        sdl2::log_info("Failed saving image: %s\n", SDL_GetError());
        return;
    }

    sdl2::log_info("Saved texture as BMP to \"%s\"\n", filename);
}
