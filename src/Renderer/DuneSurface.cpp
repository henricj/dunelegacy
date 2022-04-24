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

#include "Renderer/DuneSurface.h"

DuneSurfaceOwned::DuneSurfaceOwned(sdl2::surface_ptr surface, float width, float height)
    : surface_{std::move(surface)}, width_{width}, height_{height} {
    if (!surface)
        return;

    if (width_ > 0.f && height_ > 0.f)
        return;

    width_  = width_ > 0.f ? width_ : static_cast<float>(surface_->w);
    height_ = height_ > 0.f ? height_ : static_cast<float>(surface_->h);
}

DuneSurfaceOwned::~DuneSurfaceOwned() = default;

sdl2::texture_ptr DuneSurfaceOwned::createTexture(SDL_Renderer* renderer) const {
    return sdl2::texture_ptr{SDL_CreateTextureFromSurface(renderer, surface_.get())};
}
