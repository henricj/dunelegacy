#include <Renderer/DuneTexture.h>

void DuneTexture::draw(SDL_Renderer* renderer, int x, int y) const noexcept {
    DuneRendererImplementation::countRenderCopy(texture_);

    const auto     src = source_.as_sdl();
    const SDL_FRect dst{x, y, source_.w, source_.h};

    if(SDL_RenderCopyF(renderer, texture_, &src, &dst)) SDL_Log("SDL_RenderCopy failed: %s", SDL_GetError());
}

void DuneTexture::draw(SDL_Renderer* renderer, int x, int y, const SDL_Rect& source) const noexcept {
    DuneRendererImplementation::countRenderCopy(texture_);

    if(source.x < 0 || source.y < 0 || source.w < 1 || source.h < 1) {
        SDL_Log("The source rectangle is invalid (%dx%d at %dx%d)", source.w, source.h, source.x, source.y);
        return;
    }

    const SDL_Rect src{source_.x + source.x, source_.y + source.y, source.w, source.h};
    const SDL_FRect dst{x, y, source_.w, source_.h};

    if(src.x + src.w > source_.x + source_.w || src.y + src.h > source_.y + source_.h) {
        SDL_Log("source rectangle out of bounds");
        return;
    }

    if(SDL_RenderCopyF(renderer, texture_, &src, &dst)) SDL_Log("SDL_RenderCopy failed: %s", SDL_GetError());
}

void DuneTexture::draw(SDL_Renderer* renderer, int x, int y, double angle) const noexcept {
    DuneRendererImplementation::countRenderCopy(texture_);

    const auto     src = source_.as_sdl();
    const SDL_FRect dst{x, y, source_.w, source_.h};

    if(SDL_RenderCopyExF(renderer, texture_, &src, &dst, angle, nullptr, SDL_RendererFlip::SDL_FLIP_NONE))
        SDL_Log("SDL_RenderCopyEx failed: %s", SDL_GetError());
}
