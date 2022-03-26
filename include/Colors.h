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
inline constexpr auto RMASK = 0xFF000000U;
inline constexpr auto GMASK = 0x00FF0000U;
inline constexpr auto BMASK = 0x0000FF00U;
inline constexpr auto AMASK = 0x000000FFU;

inline constexpr auto RSHIFT = 24u;
inline constexpr auto GSHIFT = 16u;
inline constexpr auto BSHIFT = 8u;
inline constexpr auto ASHIFT = 0u;
#else
inline constexpr auto RMASK = 0x000000FFU;
inline constexpr auto GMASK = 0x0000FF00U;
inline constexpr auto BMASK = 0x00FF0000U;
inline constexpr auto AMASK = 0xFF000000U;

inline constexpr auto RSHIFT = 0u;
inline constexpr auto GSHIFT = 8u;
inline constexpr auto BSHIFT = 16u;
inline constexpr auto ASHIFT = 24u;
#endif

inline constexpr auto COLOR_RGBA(Uint32 r, Uint32 g, Uint32 b, Uint32 a) {
    return (r & 0xFF) << RSHIFT | (g & 0xFF) << GSHIFT | (b & 0xFF) << BSHIFT | (a & 0xFF) << ASHIFT;
}

inline constexpr auto COLOR_RGB(Uint32 r, Uint32 g, Uint32 b) {
    return COLOR_RGBA(r, g, b, 255);
}

inline auto MapRGBA(const SDL_PixelFormat* fmt, Uint32 color) {
    return SDL_MapRGBA(fmt, (color & RMASK) >> RSHIFT, (color & GMASK) >> GSHIFT, (color & BMASK) >> BSHIFT,
                       (color & AMASK) >> ASHIFT);
}

constexpr auto SDL2RGB(SDL_Color sdl_color) {
    return COLOR_RGB(sdl_color.r, sdl_color.g, sdl_color.b);
}

constexpr auto RGBA2SDL(Uint32 color) {
    return SDL_Color {static_cast<Uint8>((color & RMASK) >> RSHIFT), static_cast<Uint8>((color & GMASK) >> GSHIFT),
                      static_cast<Uint8>((color & BMASK) >> BSHIFT), static_cast<Uint8>((color & AMASK) >> ASHIFT)};
}

// Palette color indices
inline constexpr auto PALCOLOR_TRANSPARENT         = 0;
inline constexpr auto PALCOLOR_BLACK               = 12;
inline constexpr auto PALCOLOR_DARKGREY            = 13;
inline constexpr auto PALCOLOR_BLUE                = 11;
inline constexpr auto PALCOLOR_LIGHTBLUE           = 9;
inline constexpr auto PALCOLOR_LIGHTGREY           = 14;
inline constexpr auto PALCOLOR_BROWN               = 95;
inline constexpr auto PALCOLOR_YELLOW              = 123;
inline constexpr auto PALCOLOR_GREEN               = 3;
inline constexpr auto PALCOLOR_LIGHTGREEN          = 4;
inline constexpr auto PALCOLOR_RED                 = 231;
inline constexpr auto PALCOLOR_LIGHTRED            = 8;
inline constexpr auto PALCOLOR_WHITE               = 15;
inline constexpr auto PALCOLOR_ORANGE              = 83;
inline constexpr auto PALCOLOR_SHADOW              = 122;
inline constexpr auto PALCOLOR_GREY                = 133;
inline constexpr auto PALCOLOR_WINDTRAP_COLORCYCLE = 223;
inline constexpr auto PALCOLOR_UI_COLORCYCLE       = 255;

inline constexpr auto PALCOLOR_DESERTSAND = 105;
inline constexpr auto PALCOLOR_SPICE      = 111;
inline constexpr auto PALCOLOR_THICKSPICE = 116;
inline constexpr auto PALCOLOR_MOUNTAIN   = 47;

inline constexpr auto PALCOLOR_HARKONNEN = 144;
inline constexpr auto PALCOLOR_ATREIDES  = 160;
inline constexpr auto PALCOLOR_ORDOS     = 176;
inline constexpr auto PALCOLOR_FREMEN    = 192;
inline constexpr auto PALCOLOR_SARDAUKAR = 208;
inline constexpr auto PALCOLOR_MERCENARY = 224;

// Colors
inline constexpr auto COLOR_INVALID               = COLOR_RGBA(0xDE, 0xAD, 0xBE, 0xEF);
inline constexpr auto COLOR_TRANSPARENT           = COLOR_RGBA(0, 0, 0, 0);
inline constexpr auto COLOR_HALF_TRANSPARENT      = COLOR_RGBA(0, 0, 0, 128);
inline constexpr auto COLOR_FOG_TRANSPARENT       = COLOR_RGBA(0, 0, 0, 96);
inline constexpr auto COLOR_SHADOW_TRANSPARENT    = COLOR_RGBA(0, 0, 0, 128);
inline constexpr auto COLOR_INDICATOR_TRANSPARENT = COLOR_RGBA(255, 255, 255, 48);
inline constexpr auto COLOR_BLACK                 = COLOR_RGB(0, 0, 0);
inline constexpr auto COLOR_WHITE                 = COLOR_RGB(255, 255, 255);
inline constexpr auto COLOR_DARKGREY              = COLOR_RGB(85, 85, 85);
inline constexpr auto COLOR_LIGHTGREY             = COLOR_RGB(170, 170, 170);
inline constexpr auto COLOR_LIGHTBLUE             = COLOR_RGB(85, 255, 255);
inline constexpr auto COLOR_RED                   = COLOR_RGB(240, 0, 0);
inline constexpr auto COLOR_YELLOW                = COLOR_RGB(255, 255, 0);
inline constexpr auto COLOR_LIGHTYELLOW           = COLOR_RGB(255, 182, 44);
inline constexpr auto COLOR_LIGHTGREEN            = COLOR_RGB(85, 255, 85);
inline constexpr auto COLOR_GREEN                 = COLOR_RGB(0, 170, 0);
inline constexpr auto COLOR_BLUE                  = COLOR_RGB(0, 0, 170);
inline constexpr auto COLOR_ORANGE                = COLOR_RGB(255, 68, 0);

inline constexpr auto COLOR_DESERTSAND = COLOR_RGB(255, 210, 125);
inline constexpr auto COLOR_SPICE      = COLOR_RGB(242, 174, 36);
inline constexpr auto COLOR_THICKSPICE = COLOR_RGB(182, 125, 12);
inline constexpr auto COLOR_ROCK       = COLOR_DARKGREY;
inline constexpr auto COLOR_MOUNTAIN   = COLOR_RGB(105, 80, 4);
inline constexpr auto COLOR_BLOOM      = COLOR_RED;

#endif // COLORS_H
