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

#include <SDL.h>

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

void replaceColor(SDL_Surface *surface, Uint32 oldColor, Uint32 newColor);
void mapColor(SDL_Surface *surface, Uint8 colorMap[256]);

SDL_Surface*    copySurface(SDL_Surface* inSurface);

SDL_Surface*    convertSurfaceToDisplayFormat(SDL_Surface* inSurface, bool freeSrcSurface = true);

SDL_Surface*    scaleSurface(SDL_Surface *surf, double ratio, bool freeSrcSurface = true);

SDL_Surface*	getSubPicture(SDL_Surface* Pic, int left, int top, int width, int height);

SDL_Surface*    getSubFrame(SDL_Surface* Pic, int i, int j, int numX, int numY);

SDL_Surface*    combinePictures(SDL_Surface* basePicture, SDL_Surface* topPicture, int x = 0, int y = 0, bool bFreeBasePicture = true, bool bFreeTopPicture = true);

SDL_Surface*	rotateSurfaceLeft(SDL_Surface* inputPic, bool bFreeInputPic = true);
SDL_Surface*	rotateSurfaceRight(SDL_Surface* inputPic, bool bFreeInputPic = true);

SDL_Surface*	flipHSurface(SDL_Surface* inputPic, bool bFreeInputPic = true);
SDL_Surface*	flipVSurface(SDL_Surface* inputPic, bool bFreeInputPic = true);



SDL_Surface*    createShadowSurface(SDL_Surface* source);

/**
    This function maps all the colors in source which are between srcColor and srcColor+7 to colors between destColor and destColor+7. This is
    useful for mapping the house color.
    \param  source      The source image
    \param  srcColor    Color range to change = [srcColor;srcColor+7]
    \param  destColor   Color range to change to = [destColor;destColor+7]
    \return The mapped surface
*/
SDL_Surface*	mapSurfaceColorRange(SDL_Surface* source, int srcColor, int destColor, bool bFreeSource = false);

#endif // DRAW_UTIL_H
