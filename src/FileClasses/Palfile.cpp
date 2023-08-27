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

#include <FileClasses/Palfile.h>

#include "Definitions.h"
#include <misc/exceptions.h>

#include <cstdio>

Palette LoadPalette_RW(SDL_RWops* rwop) {
    if (rwop == nullptr) {
        THROW(std::invalid_argument, "Palfile::Palfile(): rwop == nullptr!");
    }

    const int64_t endOffset = SDL_RWsize(rwop);
    if (endOffset < 0) {
        THROW(std::runtime_error, "Palfile::Palfile(): Cannot determine size of this *.pal-File!");
    }

    const auto filesize = static_cast<size_t>(endOffset);

    if (filesize % 3 != 0) {
        THROW(std::runtime_error, "Palfile::Palfile(): Filesize must be multiple of 3!");
    }

    auto buf = std::make_unique<unsigned char[]>(filesize);

    if (SDL_RWread(rwop, &buf[0], filesize, 1) != 1) {
        THROW(std::runtime_error, "Palfile::Palfile(): SDL_RWread failed!");
    }

    const auto numColors = filesize / 3;

    auto colors = std::make_unique<SDL_Color[]>(numColors);

    const auto* RESTRICT p = &buf[0];

    if (numColors > 0) {
        // The first color is always transparent... (?)
        // colors[0].r = 0;
        // colors[0].g = 0;
        // colors[0].b = 0;
        // colors[0].a = 0;

        for (auto i = 0U; i < numColors; ++i) {
            auto& RESTRICT color = colors[i];

            color.r = static_cast<uint8_t>(*p++ * (255.0 / 63.0));
            color.g = static_cast<uint8_t>(*p++ * (255.0 / 63.0));
            color.b = static_cast<uint8_t>(*p++ * (255.0 / 63.0));
            color.a = 0xFF;
        }
    }

    sdl2::palette_ptr sdl_palette{SDL_AllocPalette(static_cast<int>(numColors))};

    SDL_SetPaletteColors(sdl_palette.get(), &colors[0], 0, static_cast<int>(numColors));

    return Palette{std::move(sdl_palette)};
}
