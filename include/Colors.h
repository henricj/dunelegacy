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

#ifndef COLORS_H
#define COLORS_H

#include <SDL2/SDL_pixels.h>

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
static constexpr auto RMASK = 0xFF000000U;
static constexpr auto GMASK = 0x00FF0000U;
static constexpr auto BMASK = 0x0000FF00U;
static constexpr auto AMASK = 0x000000FFU;

static constexpr auto RSHIFT = 24u;
static constexpr auto GSHIFT = 16u;
static constexpr auto BSHIFT = 8u;
static constexpr auto ASHIFT = 0u;
#else
static constexpr auto RMASK = 0x000000FFU;
static constexpr auto GMASK = 0x0000FF00U;
static constexpr auto BMASK = 0x00FF0000U;
static constexpr auto AMASK = 0xFF000000U;

static constexpr auto RSHIFT = 0u;
static constexpr auto GSHIFT = 8u;
static constexpr auto BSHIFT = 16u;
static constexpr auto ASHIFT = 24u;
#endif

static constexpr auto COLOR_RGBA(Uint32 r, Uint32 g, Uint32 b, Uint32 a) {
    return (r & 0xFF) << RSHIFT | (g & 0xFF) << GSHIFT | (b & 0xFF) << BSHIFT | (a & 0xFF) << ASHIFT;
}

static constexpr auto COLOR_RGB(Uint32 r, Uint32 g, Uint32 b) {
    return COLOR_RGBA(r, g, b, 255);
}

inline auto MapRGBA(const SDL_PixelFormat* fmt, Uint32 color) {
    return SDL_MapRGBA(fmt, (color & RMASK) >> RSHIFT, (color & GMASK) >> GSHIFT, (color & BMASK) >> BSHIFT, (color & AMASK) >> ASHIFT);
}

constexpr auto SDL2RGB(SDL_Color sdl_color) {
    return COLOR_RGB(sdl_color.r, sdl_color.g, sdl_color.b);
}

constexpr auto RGBA2SDL(Uint32 color) {
    return SDL_Color {
        static_cast<Uint8>((color & RMASK) >> RSHIFT),
        static_cast<Uint8>((color & GMASK) >> GSHIFT),
        static_cast<Uint8>((color & BMASK) >> BSHIFT),
        static_cast<Uint8>((color & AMASK) >> ASHIFT)};
}

// Palette color indices
static constexpr auto PALCOLOR_TRANSPARENT         = 0;
static constexpr auto PALCOLOR_BLACK               = 12;
static constexpr auto PALCOLOR_DARKGREY            = 13;
static constexpr auto PALCOLOR_BLUE                = 11;
static constexpr auto PALCOLOR_LIGHTBLUE           = 9;
static constexpr auto PALCOLOR_LIGHTGREY           = 14;
static constexpr auto PALCOLOR_BROWN               = 95;
static constexpr auto PALCOLOR_YELLOW              = 123;
static constexpr auto PALCOLOR_GREEN               = 3;
static constexpr auto PALCOLOR_LIGHTGREEN          = 4;
static constexpr auto PALCOLOR_RED                 = 231;
static constexpr auto PALCOLOR_LIGHTRED            = 8;
static constexpr auto PALCOLOR_WHITE               = 15;
static constexpr auto PALCOLOR_ORANGE              = 83;
static constexpr auto PALCOLOR_SHADOW              = 122;
static constexpr auto PALCOLOR_GREY                = 133;
static constexpr auto PALCOLOR_WINDTRAP_COLORCYCLE = 223;
static constexpr auto PALCOLOR_UI_COLORCYCLE       = 255;

static constexpr auto PALCOLOR_DESERTSAND = 105;
static constexpr auto PALCOLOR_SPICE      = 111;
static constexpr auto PALCOLOR_THICKSPICE = 116;
static constexpr auto PALCOLOR_MOUNTAIN   = 47;

static constexpr auto PALCOLOR_HARKONNEN = 144;
static constexpr auto PALCOLOR_ATREIDES  = 160;
static constexpr auto PALCOLOR_ORDOS     = 176;
static constexpr auto PALCOLOR_FREMEN    = 192;
static constexpr auto PALCOLOR_SARDAUKAR = 208;
static constexpr auto PALCOLOR_MERCENARY = 224;

// Colors
static constexpr auto COLOR_INVALID               = COLOR_RGBA(0xDE, 0xAD, 0xBE, 0xEF);
static constexpr auto COLOR_TRANSPARENT           = COLOR_RGBA(0, 0, 0, 0);
static constexpr auto COLOR_HALF_TRANSPARENT      = COLOR_RGBA(0, 0, 0, 128);
static constexpr auto COLOR_FOG_TRANSPARENT       = COLOR_RGBA(0, 0, 0, 96);
static constexpr auto COLOR_SHADOW_TRANSPARENT    = COLOR_RGBA(0, 0, 0, 128);
static constexpr auto COLOR_INDICATOR_TRANSPARENT = COLOR_RGBA(255, 255, 255, 48);
static constexpr auto COLOR_BLACK                 = COLOR_RGB(0, 0, 0);
static constexpr auto COLOR_WHITE                 = COLOR_RGB(255, 255, 255);
static constexpr auto COLOR_DARKGREY              = COLOR_RGB(85, 85, 85);
static constexpr auto COLOR_LIGHTGREY             = COLOR_RGB(170, 170, 170);
static constexpr auto COLOR_LIGHTBLUE             = COLOR_RGB(85, 255, 255);
static constexpr auto COLOR_RED                   = COLOR_RGB(240, 0, 0);
static constexpr auto COLOR_YELLOW                = COLOR_RGB(255, 255, 0);
static constexpr auto COLOR_LIGHTYELLOW           = COLOR_RGB(255, 182, 44);
static constexpr auto COLOR_LIGHTGREEN            = COLOR_RGB(85, 255, 85);
static constexpr auto COLOR_GREEN                 = COLOR_RGB(0, 170, 0);
static constexpr auto COLOR_BLUE                  = COLOR_RGB(0, 0, 170);
static constexpr auto COLOR_ORANGE                = COLOR_RGB(255, 68, 0);

static constexpr auto COLOR_DESERTSAND = COLOR_RGB(255, 210, 125);
static constexpr auto COLOR_SPICE      = COLOR_RGB(242, 174, 36);
static constexpr auto COLOR_THICKSPICE = COLOR_RGB(182, 125, 12);
static constexpr auto COLOR_ROCK       = COLOR_DARKGREY;
static constexpr auto COLOR_MOUNTAIN   = COLOR_RGB(105, 80, 4);
static constexpr auto COLOR_BLOOM      = COLOR_RED;

#endif // COLORS_H
