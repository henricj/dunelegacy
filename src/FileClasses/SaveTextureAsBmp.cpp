#include <SDL2/SDL.h>
#include <misc/sdl_support.h>
#include "misc/SDL2pp.h"

#include "FileClasses/SaveTextureAsBmp.h"

namespace
{
    class RestoreRenderTarget final
    {
        SDL_Renderer* const renderer_;
        SDL_Texture* const texture_;
    public:
        RestoreRenderTarget(SDL_Renderer* renderer) : renderer_(renderer), texture_(SDL_GetRenderTarget(renderer)) { }
        ~RestoreRenderTarget() { SDL_SetRenderTarget(renderer_, texture_); }
    };
}
void SaveTextureAsBmp(SDL_Renderer *renderer, SDL_Texture* texture, const char* filename)
{
    // From https://stackoverflow.com/a/48176678

    const int format = SDL_PIXELFORMAT_RGBA32;

    int w = 0;
    int h = 0;

    /* Get information about texture we want to save */
    if (SDL_QueryTexture(texture, nullptr, nullptr, &w, &h)) {
        SDL_Log("Failed querying texture: %s\n", SDL_GetError());
        return;
    }

    const auto ren_tex = sdl2::texture_ptr{ SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_TARGET, w, h) };
    if (!ren_tex) {
        SDL_Log("Failed creating render texture: %s\n", SDL_GetError());
        return;
    }

    RestoreRenderTarget rrt{ renderer };

    /*
    * Initialize our canvas, then copy texture to a target whose pixel data we
    * can access
    */
    if (SDL_SetRenderTarget(renderer, ren_tex.get())) {
        SDL_Log("Failed setting render target: %s\n", SDL_GetError());
        return;
    }

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);

    if (SDL_RenderCopy(renderer, texture, nullptr, nullptr)) {
        SDL_Log("Failed copying texture data: %s\n", SDL_GetError());
        return;
    }

    /* Create buffer to hold texture data and load it */
    const auto pixels = std::make_unique<char[]>(w * h * SDL_BYTESPERPIXEL(format));
    if (!pixels) {
        SDL_Log("Failed allocating memory\n");
        return;
    }

    if (SDL_RenderReadPixels(renderer, nullptr, format, pixels.get(), w * SDL_BYTESPERPIXEL(format))) {
        SDL_Log("Failed reading pixel data: %s\n", SDL_GetError());
        return;
    }

    /* Copy pixel data over to surface */
    auto surf = sdl2::surface_ptr{ SDL_CreateRGBSurfaceWithFormatFrom(pixels.get(), w, h, SDL_BITSPERPIXEL(format), w * SDL_BYTESPERPIXEL(format), format) };
    if (!surf) {
        SDL_Log("Failed creating new surface: %s\n", SDL_GetError());
        return;
    }

    /* Save result to an image */
    if (SDL_SaveBMP(surf.get(), filename)) {
        SDL_Log("Failed saving image: %s\n", SDL_GetError());
        return;
    }

    SDL_Log("Saved texture as BMP to \"%s\"\n", filename);
}
