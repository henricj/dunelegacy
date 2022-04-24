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

#include <GUI/GUIStyle.h>

std::unique_ptr<GUIStyle> GUIStyle::currentGUIStyle;

GUIStyle::GUIStyle() = default;

GUIStyle::~GUIStyle() = default;

void GUIStyle::setZoom(float zoom) {
    if (zoom < 0.01f || zoom > 100)
        THROW(std::invalid_argument, "GUIStyle scale out of range %f", zoom);

    zoom_ = zoom;
}

void GUIStyle::setDisplayDpi(float ratio) {
    if (ratio < 0.01f || ratio > 100)
        THROW(std::invalid_argument, "GUIStyle scale out of range %f", ratio);

    dpi_ratio_ = ratio;
}

namespace {
auto scale_xy(float scale, int x, int y) {
    const auto w = static_cast<int>(std::round(static_cast<float>(x) * scale));
    const auto h = static_cast<int>(std::round(static_cast<float>(y) * scale));

    return std::make_pair(w, h);
}
} // namespace

void GUIStyle::setLogicalSize(SDL_Renderer* renderer, int physical_width, int physical_height) {
    auto scale = getScale();

    const auto scale_inverse = 1.f / scale;

    auto [w, h] = scale_xy(scale_inverse, physical_width, physical_height);

    if (w < MINIMUM_WIDTH || h < MINIMUM_HEIGHT) {
        const auto w_ratio = static_cast<float>(MINIMUM_WIDTH) / static_cast<float>(w);
        const auto h_ratio = static_cast<float>(MINIMUM_HEIGHT) / static_cast<float>(h);

        const auto worst_ratio = std::max(w_ratio, h_ratio);

        scale /= worst_ratio;

        const auto [w2, h2] = scale_xy(scale_inverse * worst_ratio, physical_width, physical_height);

        w = w2;
        h = h2;
    }

    actual_scale_ = scale;

    SDL_RenderSetLogicalSize(renderer, w, h);
}

sdl2::surface_ptr GUIStyle::createEmptySurface(uint32_t width, uint32_t height, bool transparent) {
    sdl2::surface_ptr pSurface =
        sdl2::surface_ptr{SDL_CreateRGBSurface(0, width, height, 32, RMASK, GMASK, BMASK, AMASK)};

    if (!pSurface) {
        return nullptr;
    }
    SDL_FillRect(pSurface.get(), nullptr, COLOR_TRANSPARENT);
    SDL_SetColorKey(pSurface.get(), SDL_TRUE, COLOR_TRANSPARENT);

    return pSurface;
}
