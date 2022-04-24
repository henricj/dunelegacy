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

#include <Renderer/DuneTexture.h>

DuneTexture::DuneTexture(SDL_Texture* texture, const SDL_Rect& rect)
    : texture_{texture}, source_{rect}, width_{static_cast<float>(rect.w)}, height_{static_cast<float>(rect.h)} { }

DuneTexture::DuneTexture(SDL_Texture* texture)
    : texture_{texture}, source_{DuneTextureRect::create(texture)}, width_{static_cast<float>(source_.w)},
      height_{static_cast<float>(source_.h)} {
    if (!texture) {
        return;
    }

    int w, h;
    SDL_QueryTexture(texture_, nullptr, nullptr, &w, &h);
    source_ = DuneTextureRect::create(0, 0, w, h);

    width_  = static_cast<float>(w);
    height_ = static_cast<float>(h);
}

void DuneTexture::draw(SDL_Renderer* renderer, float x, float y) const noexcept {
    DuneRendererImplementation::countRenderCopy(texture_);

    const auto src = source_.as_sdl();
    const SDL_FRect dst{x, y, width_, height_};

    if (SDL_RenderCopyF(renderer, texture_, &src, &dst))
        sdl2::log_info("SDL_RenderCopy failed: %s", SDL_GetError());
}

void DuneTexture::draw(SDL_Renderer* renderer, float x, float y, const SDL_Rect& source) const noexcept {
    DuneRendererImplementation::countRenderCopy(texture_);

    if (source.x < 0 || source.y < 0 || source.w < 1 || source.h < 1) {
        sdl2::log_info("The source rectangle is invalid (%dx%d at %dx%d)", source.w, source.h, source.x, source.y);
        return;
    }

    const SDL_Rect src{source_.x + source.x, source_.y + source.y, source.w, source.h};
    const SDL_FRect dst{x, y, width_, height_};

    if (src.x + src.w > source_.x + source_.w || src.y + src.h > source_.y + source_.h) {
        sdl2::log_info("source rectangle out of bounds");
        return;
    }

    if (SDL_RenderCopyF(renderer, texture_, &src, &dst))
        sdl2::log_info("SDL_RenderCopyF failed: %s", SDL_GetError());
}

void DuneTexture::draw(SDL_Renderer* renderer, float x, float y, double angle) const noexcept {
    DuneRendererImplementation::countRenderCopy(texture_);

    const auto src = source_.as_sdl();
    const SDL_FRect dst{x, y, width_, height_};

    if (SDL_RenderCopyExF(renderer, texture_, &src, &dst, angle, nullptr, SDL_RendererFlip::SDL_FLIP_NONE))
        sdl2::log_info("SDL_RenderCopyEx failed: %s", SDL_GetError());
}

DuneTextureOwned::DuneTextureOwned(sdl2::texture_ptr texture, float width, float height)
    : texture_{std::move(texture)}, width_{width}, height_{height} {
    if (!texture)
        return;

    if (width_ > 0.f && height_ > 0.f)
        return;

    int w, h;
    SDL_QueryTexture(texture_.get(), nullptr, nullptr, &w, &h);

    width_  = width_ > 0.f ? width_ : static_cast<float>(w);
    height_ = height_ > 0.f ? height_ : static_cast<float>(h);
}

DuneTextureOwned::~DuneTextureOwned() = default;

void DuneTextureOwned::draw(SDL_Renderer* renderer, float x, float y) const noexcept {
    const SDL_FRect dst{x, y, width_, height_};

    if (SDL_RenderCopyF(renderer, texture_.get(), nullptr, &dst))
        sdl2::log_info("SDL_RenderCopyF failed: %s", SDL_GetError());
}
