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

#include <SDL2/SDL.h>

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define RMASK 0xFF000000U
    #define GMASK 0x00FF0000U
    #define BMASK 0x0000FF00U
    #define AMASK 0x000000FFU

    #define RSHIFT 24
    #define GSHIFT 16
    #define BSHIFT  8
    #define ASHIFT  0
#else
    #define RMASK 0x000000FFU
    #define GMASK 0x0000FF00U
    #define BMASK 0x00FF0000U
    #define AMASK 0xFF000000U

    #define RSHIFT  0
    #define GSHIFT  8
    #define BSHIFT 16
    #define ASHIFT 24
#endif

#define COLOR_RGBA(r,g,b,a) ( ((((unsigned int) r) & 0xFF) << RSHIFT) | ((((unsigned int) g) & 0xFF) << GSHIFT) | ((((unsigned int) b) & 0xFF) << BSHIFT) | ((((unsigned int) a) & 0xFF) << ASHIFT) )
#define COLOR_RGB(r,g,b) COLOR_RGBA(r,g,b,255)
#define MapRGBA(fmt, color) SDL_MapRGBA(fmt, (color & RMASK) >> RSHIFT, (color & GMASK) >> GSHIFT, (color & BMASK) >> BSHIFT, (color & AMASK) >> ASHIFT)

#define SDL2RGB(sdl_color) COLOR_RGB(sdl_color.r, sdl_color.g, sdl_color.b)

// Palette color indices
#define PALCOLOR_TRANSPARENT 0
#define PALCOLOR_BLACK 12
#define PALCOLOR_DARKGREY 13
#define PALCOLOR_BLUE 11
#define PALCOLOR_LIGHTBLUE 9
#define PALCOLOR_LIGHTGREY 14
#define PALCOLOR_BROWN 95
#define PALCOLOR_YELLOW 123
#define PALCOLOR_GREEN 3
#define PALCOLOR_LIGHTGREEN 4
#define PALCOLOR_RED 231
#define PALCOLOR_LIGHTRED 8
#define PALCOLOR_WHITE 15
#define PALCOLOR_ORANGE 83
#define PALCOLOR_GREY 133
#define PALCOLOR_WINDTRAP_COLORCYCLE 223
#define PALCOLOR_UI_COLORCYCLE 255

#define PALCOLOR_DESERTSAND 105
#define PALCOLOR_SPICE 111
#define PALCOLOR_THICKSPICE 116
#define PALCOLOR_MOUNTAIN 47

#define PALCOLOR_HARKONNEN 144
#define PALCOLOR_ATREIDES 160
#define PALCOLOR_ORDOS 176
#define PALCOLOR_FREMEN 192
#define PALCOLOR_SARDAUKAR 208
#define PALCOLOR_MERCENARY 224

// Colors
#define COLOR_INVALID COLOR_RGBA(0xDE, 0xAD, 0xBE, 0xEF)
#define COLOR_TRANSPARENT COLOR_RGBA(0,0,0,0)
#define COLOR_HALF_TRANSPARENT COLOR_RGBA(0,0,0,128)
#define COLOR_FOG_TRANSPARENT COLOR_RGBA(0,0,0,96)
#define COLOR_SHADOW_TRANSPARENT COLOR_RGBA(0,0,0,128)
#define COLOR_INDICATOR_TRANSPARENT COLOR_RGBA(255,255,255,48)
#define COLOR_BLACK COLOR_RGB(0,0,0)
#define COLOR_WHITE COLOR_RGB(255,255,255)
#define COLOR_DARKGREY COLOR_RGB(85,85,85)
#define COLOR_LIGHTGREY COLOR_RGB(170,170,170)
#define COLOR_LIGHTBLUE COLOR_RGB(85,255,255)
#define COLOR_RED COLOR_RGB(240,0,0)
#define COLOR_YELLOW COLOR_RGB(255,255,0)
#define COLOR_LIGHTYELLOW COLOR_RGB(255,182,44)
#define COLOR_LIGHTGREEN COLOR_RGB(85,255,85)
#define COLOR_GREEN COLOR_RGB(0,170,0)
#define COLOR_BLUE COLOR_RGB(0,0,170)
#define COLOR_ORANGE COLOR_RGB(255,68,0)

#define COLOR_DESERTSAND COLOR_RGB(255,210,125)
#define COLOR_SPICE COLOR_RGB(242,174,36)
#define COLOR_THICKSPICE COLOR_RGB(182,125,12)
#define COLOR_ROCK COLOR_DARKGREY
#define COLOR_MOUNTAIN COLOR_RGB(105,80,4)
#define COLOR_BLOOM COLOR_RED

#endif // COLORS_H
