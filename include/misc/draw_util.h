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

#ifndef DRAW_UTIL_H
#define DRAW_UTIL_H

#include <Colors.h>
#include <misc/SDL2pp.h>

/**
    Return the pixel value at (x, y) in surface
    NOTE: The surface must be locked before calling this!
    \param  surface the surface
    \param  x       the x coordinate
    \param  y       the y coordinate
    \return the value of the pixel
 */
Uint32 getPixel(SDL_Surface *surface, int x, int y);

void putPixel(SDL_Surface *surface, int x, int y, Uint32 color);

void drawHLineNoLock(SDL_Surface *surface, int x1, int y, int x2, Uint32 color);
void drawVLineNoLock(SDL_Surface *surface, int x, int y1, int y2, Uint32 color);
void drawHLine(SDL_Surface *surface, int x1, int y, int x2, Uint32 color);
void drawVLine(SDL_Surface *surface, int x, int y1, int y2, Uint32 color);

void drawRect(SDL_Surface *surface, int x1, int y1, int x2, int y2, Uint32 color);

inline void setRenderDrawColor(SDL_Renderer* renderer, Uint32 color) {
    if(((color & AMASK) >> ASHIFT) != 255) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    }
    SDL_SetRenderDrawColor(renderer, (color & RMASK) >> RSHIFT, (color & GMASK) >> GSHIFT, (color & BMASK) >> BSHIFT, (color & AMASK) >> ASHIFT);
}

inline void renderDrawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, Uint32 color) {
    setRenderDrawColor(renderer, color);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

inline void renderDrawHLine(SDL_Renderer* renderer, int x1, int y, int x2, Uint32 color) {
    renderDrawLine(renderer, x1, y, x2, y, color);
}

inline void renderDrawVLine(SDL_Renderer* renderer, int x, int y1, int y2, Uint32 color) {
    renderDrawLine(renderer, x, y1, x, y2, color);
}

inline void renderDrawRect(SDL_Renderer* renderer, SDL_Rect* rect, Uint32 color) {
    setRenderDrawColor(renderer, color);
    SDL_RenderDrawRect(renderer, rect);
}

inline void renderDrawRect(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, Uint32 color) {
    SDL_Rect rect = { x1, y1, x2-x1+1, y2-y1+1 };
    renderDrawRect(renderer, &rect, color);
}

inline void renderFillRect(SDL_Renderer* renderer, SDL_Rect* rect, Uint32 color) {
    setRenderDrawColor(renderer, color);
    SDL_RenderFillRect(renderer, rect);
}

inline void renderFillRect(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, Uint32 color) {
    SDL_Rect rect = { x1, y1, x2-x1+1, y2-y1+1 };
    renderFillRect(renderer, &rect, color);
}

sdl2::surface_ptr renderReadSurface(SDL_Renderer* renderer);

void replaceColor(SDL_Surface *surface, Uint32 oldColor, Uint32 newColor);
void mapColor(SDL_Surface *surface, Uint8 colorMap[256]);

sdl2::surface_ptr   copySurface(SDL_Surface* inSurface);

sdl2::surface_ptr   convertSurfaceToDisplayFormat(SDL_Surface* inSurface);

sdl2::texture_ptr   convertSurfaceToTexture(SDL_Surface* inSurface);
inline sdl2::texture_ptr convertSurfaceToTexture(sdl2::surface_ptr inSurface) {
    return convertSurfaceToTexture(inSurface.get());
}

sdl2::surface_ptr   scaleSurface(SDL_Surface *surf, double ratio);

sdl2::surface_ptr   getSubPicture(SDL_Surface* Pic, int left, int top, int width, int height);

sdl2::surface_ptr   getSubFrame(SDL_Surface* Pic, int i, int j, int numX, int numY);

sdl2::surface_ptr   combinePictures(SDL_Surface* basePicture, SDL_Surface* topPicture, int x = 0, int y = 0);

sdl2::surface_ptr    rotateSurfaceLeft(SDL_Surface* inputPic);
sdl2::surface_ptr    rotateSurfaceRight(SDL_Surface* inputPic);

sdl2::surface_ptr    flipHSurface(SDL_Surface* inputPic);
sdl2::surface_ptr    flipVSurface(SDL_Surface* inputPic);


sdl2::surface_ptr    createShadowSurface(SDL_Surface* source);

/**
    This function maps all the colors in source which are between srcColor and srcColor+7 to colors between destColor and destColor+7. This is
    useful for mapping the house color.
    \param  source      The source image
    \param  srcColor    Color range to change = [srcColor;srcColor+7]
    \param  destColor   Color range to change to = [destColor;destColor+7]
    \return The mapped surface
*/
sdl2::surface_ptr    mapSurfaceColorRange(SDL_Surface* source, int srcColor, int destColor);

#endif // DRAW_UTIL_H
