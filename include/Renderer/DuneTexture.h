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

#ifndef DUNETEXTURE_H
#define DUNETEXTURE_H

#include "misc/SDL2pp.h"
#include <SDL2/SDL.h>

#include <cassert>
#include <limits>

struct SDL_Renderer;

struct DuneTextureRect final {
    short x{};
    short y{};
    short w{};
    short h{};

    DuneTextureRect() = default;

    explicit DuneTextureRect(const SDL_Rect& rect) {
        assert(rect.x >= 0 && rect.y >= 0 && rect.w > 0 && rect.h > 0);
        assert(rect.x <= std::numeric_limits<short>::max() && rect.y <= std::numeric_limits<short>::max()
               && rect.w <= std::numeric_limits<short>::max() && rect.h <= std::numeric_limits<short>::max());

        x = static_cast<short>(rect.x);
        y = static_cast<short>(rect.y);
        w = static_cast<short>(rect.w);
        h = static_cast<short>(rect.h);
    }

    [[nodiscard]] SDL_Rect as_sdl() const noexcept { return SDL_Rect{x, y, w, h}; }
    [[nodiscard]] SDL_FRect as_sdlf() const noexcept {
        return SDL_FRect{static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h)};
    }

    static DuneTextureRect create(int x, int y, int w, int h) {
        const SDL_Rect rect{x, y, w, h};
        return DuneTextureRect(rect);
    }

    static DuneTextureRect create(SDL_Texture* texture) {
        if (!texture) {
            return {};
        }

        int w, h;
        SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
        return create(0, 0, w, h);
    }

    DuneTextureRect& operator=(const DuneTextureRect&) = default;
    DuneTextureRect& operator=(const SDL_Rect& rect) { return operator=(DuneTextureRect{rect}); }
};

struct DuneTexture final {
    SDL_Texture* texture_{};
    DuneTextureRect source_{};
    float width_{};
    float height_{};

    DuneTexture()                   = default;
    DuneTexture(const DuneTexture&) = default;
    DuneTexture(DuneTexture&&)      = default;

    DuneTexture(SDL_Texture* texture, const SDL_Rect& rect);

    explicit DuneTexture(SDL_Texture* texture);

    ~DuneTexture() = default;

    DuneTexture& operator=(const DuneTexture&) = default;
    DuneTexture& operator=(DuneTexture&&)      = default;

    operator bool() const noexcept { return nullptr != texture_; }

    [[nodiscard]] SDL_Rect source_rect() const noexcept { return source_.as_sdl(); }

    void reset();

    void draw(SDL_Renderer* renderer, float x, float y) const noexcept;
    void draw(SDL_Renderer* renderer, float x, float y, const SDL_Rect& source) const noexcept;
    void draw(SDL_Renderer* renderer, float x, float y, double angle) const noexcept;
};

struct DuneTextureOwned final {
    sdl2::texture_ptr texture_;
    float width_{};
    float height_{};

    DuneTextureOwned()                        = default;
    DuneTextureOwned(const DuneTextureOwned&) = delete;
    DuneTextureOwned(DuneTextureOwned&&)      = default;

    explicit DuneTextureOwned(sdl2::texture_ptr texture, float width = 0.f, float height = 0.f);

    ~DuneTextureOwned();

    DuneTextureOwned& operator=(const DuneTextureOwned&) = delete;
    DuneTextureOwned& operator=(DuneTextureOwned&&)      = default;

    DuneTexture as_dune_texture() const;

    operator bool() const noexcept { return texture_.operator bool(); }

    [[nodiscard]] auto get() const noexcept { return texture_.get(); }
    [[nodiscard]] auto operator->() const noexcept { return texture_.operator->(); }

    void reset();

    void draw(SDL_Renderer* renderer, float x, float y) const noexcept;

    void draw(SDL_Renderer* renderer, int x, int y) const noexcept {
        draw(renderer, static_cast<float>(x), static_cast<float>(y));
    }
};

namespace dune {

void defer_destroy_texture(sdl2::texture_ptr texture);
void destroy_textures();

inline void defer_destroy_texture(DuneTextureOwned texture) {
    defer_destroy_texture(std::move(texture.texture_));
}

} // namespace dune

#endif // DUNETEXTURE_H
