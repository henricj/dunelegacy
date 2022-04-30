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

#ifndef DUNESURFACE_H
#define DUNESURFACE_H

#include "DuneTexture.h"
#include "misc/SDL2pp.h"

struct SDL_Renderer;

struct DuneSurfaceOwned final {
    sdl2::surface_ptr surface_;
    float width_{};
    float height_{};

    DuneSurfaceOwned()                        = default;
    DuneSurfaceOwned(const DuneSurfaceOwned&) = delete;
    DuneSurfaceOwned(DuneSurfaceOwned&&)      = default;

    explicit DuneSurfaceOwned(sdl2::surface_ptr surface, float width = 0.f, float height = 0.f);
    DuneSurfaceOwned(sdl2::surface_ptr surface, int width, int height)
        : DuneSurfaceOwned(std::move(surface), static_cast<float>(width), static_cast<float>(height)) { }

    ~DuneSurfaceOwned();

    DuneSurfaceOwned& operator=(const DuneSurfaceOwned&) = delete;
    DuneSurfaceOwned& operator=(DuneSurfaceOwned&&)      = default;

    operator bool() const noexcept { return nullptr != surface_; }

    [[nodiscard]] auto get() const noexcept { return surface_.get(); }
    [[nodiscard]] auto operator->() const noexcept { return surface_.operator->(); }

    DuneTextureOwned createTexture(SDL_Renderer* renderer) const;
};

#endif // DUNESURFACE_H
