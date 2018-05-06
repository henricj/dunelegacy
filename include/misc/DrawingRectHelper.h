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

#ifndef DRAWINGRECTHELPER_H
#define DRAWINGRECTHELPER_H

#include <misc/SDL2pp.h>

extern SDL_Renderer* renderer;

enum class HAlign {
    Left,
    Center,
    Right,
};

enum class VAlign {
    Top,
    Center,
    Bottom,
};

/**
    Returns the width of this surface
    \param  pSurface    the surface to consider
    \return the width of pSurface
*/
inline int getWidth(SDL_Surface *pSurface) {
    return pSurface->w;
}

/**
    Returns the height of this surface
    \param  pSurface    the surface to consider
    \return the height of pSurface
*/
inline int getHeight(SDL_Surface *pSurface) {
    return pSurface->h;
}

/**
    Returns the width of this texture
    \param  pTexture    the surface to consider
    \return the width of pTexture
*/
inline int getWidth(SDL_Texture *pTexture) {
    int w;
    SDL_QueryTexture(pTexture, nullptr, nullptr, &w, nullptr);
    return w;
}

/**
    Returns the height of this texture
    \param  pTexture    the surface to consider
    \return the height of pTexture
*/
inline int getHeight(SDL_Texture *pTexture) {
    int h;
    SDL_QueryTexture(pTexture, nullptr, nullptr, nullptr, &h);
    return h;
}

/**
    Calculates the source rect for drawing the sprite at (row, col) in pSurface.
    \param  pSurface    the surface to calculate the rect for
    \param  col         the zero-based column index of the sprite
    \param  numCols     the number of sprites per row in pSurface
    \param  row         the zero-based row index of the sprite (default is 0)
    \param  numRows     the number of sprites per column in pSurface (default is 1)
    \return the rectangle for drawing the specified sprite from pSurface when passed to SDL_BlitSurface
*/
inline SDL_Rect calcSpriteSourceRect(SDL_Surface *pSurface, int col, int numCols, int row = 0, int numRows = 1) {
    SDL_Rect rect = { col * (pSurface->w/numCols), row * (pSurface->h/numRows), pSurface->w/numCols, pSurface->h/numRows };
    return rect;
}

/**
    Calculates the source rect for drawing the sprite at (row, col) in pTexture.
    \param  pTexture    the texture to calculate the rect for
    \param  col         the zero-based column index of the sprite
    \param  numCols     the number of sprites per row in pTexture
    \param  row         the zero-based row index of the sprite (default is 0)
    \param  numRows     the number of sprites per column in pTexture (default is 1)
    \return the rectangle for drawing the specified sprite from pTexture when passed to SDL_RenderCopy
*/
inline SDL_Rect calcSpriteSourceRect(SDL_Texture *pTexture, int col, int numCols, int row = 0, int numRows = 1) {
    int w;
    int h;
    SDL_QueryTexture(pTexture, nullptr, nullptr, &w, &h);
    SDL_Rect rect = { col * (w/numCols), row * (h/numRows), w/numCols, h/numRows };
    return rect;
}

/**
    Calculates the drawing rectangle for drawing a sprite from pSurface at (x,y). The parameters halign and valign determine which coordinate in the sprite
    is drawn at (x,y), e.g. if they are HAlign::Right and VAlign::Bottom the bottom right corner of the sprite is drawn at position (x,y)
    \param  pSurface    the surface to calculate the rect for
    \param  x           the x-coordinate
    \param  y           the y-coordinate
    \param  numCols     the number of sprites in each column
    \param  numRows     the number of sprites in each row (default is 1)
    \param  halign      the horizontal alignment of pSurface (default is HAlign::Left)
    \param  valign      the vertical alignment of pSurface (default is VAlign::Top)
    \return the rectangle for drawing pSurface at the specified position when passed to SDL_BlitSurface
*/
inline SDL_Rect calcSpriteDrawingRect(SDL_Surface* pSurface, int x, int y, int numCols, int numRows = 1, HAlign halign = HAlign::Left, VAlign valign = VAlign::Top) {
    SDL_Rect rect = { x, y, pSurface->w/numCols, pSurface->h/numRows };

    switch(halign) {
        case HAlign::Left:      /*nothing*/         break;
        case HAlign::Center:    rect.x -= rect.w/2; break;
        case HAlign::Right:     rect.x -= rect.w-1; break;
    }

    switch(valign) {
        case VAlign::Top:       /*nothing*/         break;
        case VAlign::Center:    rect.y -= rect.h/2; break;
        case VAlign::Bottom:    rect.y -= rect.h-1; break;
    }

    return rect;
}

/**
    Calculates the drawing rectangle for drawing a sprite from pTexture at (x,y). The parameters halign and valign determine which coordinate in the sprite
    is drawn at (x,y), e.g. if they are HAlign::Right and VAlign::Bottom the bottom right corner of the sprite is drawn at position (x,y)
    \param  pTexture    the texture to calculate the rect for
    \param  x           the x-coordinate
    \param  y           the y-coordinate
    \param  numCols     the number of sprites in each column
    \param  numRows     the number of sprites in each row (default is 1)
    \param  halign      the horizontal alignment of pTexture (default is HAlign::Left)
    \param  valign      the vertical alignment of pTexture (default is VAlign::Top)
    \return the rectangle for drawing pTexture at the specified position when passed to SDL_RenderCopy
*/
inline SDL_Rect calcSpriteDrawingRect(SDL_Texture* pTexture, int x, int y, int numCols, int numRows = 1, HAlign halign = HAlign::Left, VAlign valign = VAlign::Top) {
    SDL_Rect rect = { x, y, 0, 0 };
    SDL_QueryTexture(pTexture, nullptr, nullptr, &rect.w, &rect.h);

    rect.w /= numCols;
    rect.h /= numRows;

    switch(halign) {
        case HAlign::Left:      /*nothing*/         break;
        case HAlign::Center:    rect.x -= rect.w/2; break;
        case HAlign::Right:     rect.x -= rect.w-1; break;
    }

    switch(valign) {
        case VAlign::Top:       /*nothing*/         break;
        case VAlign::Center:    rect.y -= rect.h/2; break;
        case VAlign::Bottom:    rect.y -= rect.h-1; break;
    }

    return rect;
}

/**
    Calculates the drawing rectangle for drawing pSurface at (x,y). The parameters halign and valign determine which coordinate in pSurface
    is drawn at (x,y), e.g. if they are HAlign::Right and VAlign::Bottom the bottom right corner of pSurface is drawn at position (x,y)
    \param  pSurface    the surface to calculate the rect for
    \param  x           the x-coordinate
    \param  y           the y-coordinate
    \param  halign      the horizontal alignment of pSurface (default is HAlign::Left)
    \param  valign      the vertical alignment of pSurface (default is VAlign::Top)
    \return the rectangle for drawing pSurface at the specified position when passed to SDL_BlitSurface
*/
inline SDL_Rect calcDrawingRect(SDL_Surface* pSurface, int x, int y, HAlign halign = HAlign::Left, VAlign valign = VAlign::Top) {
    return calcSpriteDrawingRect(pSurface, x, y, 1, 1, halign, valign);
}

/**
    Calculates the drawing rectangle for drawing pTexture at (x,y). The parameters halign and valign determine which coordinate in pTexture
    is drawn at (x,y), e.g. if they are HAlign::Right and VAlign::Bottom the bottom right corner of pTexture is drawn at position (x,y)
    \param  pTexture    the texture to calculate the rect for
    \param  x           the x-coordinate
    \param  y           the y-coordinate
    \param  halign      the horizontal alignment of pTexture (default is HAlign::Left)
    \param  valign      the vertical alignment of pTexture (default is VAlign::Top)
    \return the rectangle for drawing pTexture at the specified position when passed to SDL_RenderCopy
*/
inline SDL_Rect calcDrawingRect(SDL_Texture* pTexture, int x, int y, HAlign halign = HAlign::Left, VAlign valign = VAlign::Top) {
    return calcSpriteDrawingRect(pTexture, x, y, 1, 1, halign, valign);
}

/**
    Returns size of the rendering target.
    \return the rectangle describing the size (w,h) of the rendering target (x and y are always zero)
*/
inline SDL_Rect getRendererSize() {
    SDL_Rect rect = {0, 0, 0, 0};
    SDL_RenderGetLogicalSize(renderer, &rect.w, &rect.h);
    if(rect.w == 0 || rect.h == 0) {
        SDL_GetRendererOutputSize(renderer, &rect.w, &rect.h);
    }
    return rect;
}

/**
    Returns the width of the rendering target.
    \return the width of the rendering target
*/
inline int getRendererWidth() {
    return getRendererSize().w;
}

/**
    Returns the height of the rendering target.
    \return the height of the rendering target
*/
inline int getRendererHeight() {
    return getRendererSize().h;
}

/**
    Calculates the drawing rectangle for drawing pSurface at the edge or in the center of rect.
    The parameters halign and valign determine at which edge pSurface is drawn, e.g. HAlign::Left and VAlign::Top draws pSurface in the
    top left corner.
    \param  pSurface    the surface to calculate the rect for
    \param  rect        the rect to center around
    \param  halign      the horizontal alignment of pSurface (default is HAlign::Center)
    \param  valign      the vertical alignment of pSurface (default is VAlign::Center)
    \return the rectangle for drawing pSurface at the specified position when passed to SDL_BlitSurface
*/
inline SDL_Rect calcAlignedDrawingRect(SDL_Surface* pSurface, const SDL_Rect& rect, HAlign halign = HAlign::Center, VAlign valign = VAlign::Center) {
    int x = 0;
    int y = 0;

    switch(halign) {
        case HAlign::Left:      x = 0;          break;
        case HAlign::Center:    x = rect.w/2;   break;
        case HAlign::Right:     x = rect.w-1;   break;
    }

    switch(valign) {
        case VAlign::Top:       y = 0;          break;
        case VAlign::Center:    y = rect.h/2;   break;
        case VAlign::Bottom:    y = rect.h-1;   break;
    }

    return calcDrawingRect(pSurface, x, y, halign, valign);
}

/**
    Calculates the drawing rectangle for drawing pSurface at the edge or in the center of the current rendering target (usually the screen).
    The parameters halign and valign determine at which edge pSurface is drawn, e.g. HAlign::Left and VAlign::Top draws pSurface in the
    top left corner.
    \param  pSurface    the surface to calculate the rect for
    \param  halign      the horizontal alignment of pSurface (default is HAlign::Center)
    \param  valign      the vertical alignment of pSurface (default is VAlign::Center)
    \return the rectangle for drawing pSurface at the specified position when passed to SDL_BlitSurface
*/
inline SDL_Rect calcAlignedDrawingRect(SDL_Surface* pSurface, HAlign halign = HAlign::Center, VAlign valign = VAlign::Center) {
    return calcAlignedDrawingRect(pSurface, getRendererSize(), halign, valign);
}

/**
    Calculates the drawing rectangle for drawing pSurface at the edge or in the center of pBaseSurface.
    The parameters halign and valign determine at which edge pSurface is drawn, e.g. HAlign::Left and VAlign::Top draws pSurface in the
    top left corner.
    \param  pSurface        the surface to calculate the rect for
    \param  pBaseSurface    the rect to center around
    \param  halign          the horizontal alignment of pSurface (default is HAlign::Center)
    \param  valign          the vertical alignment of pSurface (default is VAlign::Center)
    \return the rectangle for drawing pSurface at the specified position when passed to SDL_BlitSurface
*/
inline SDL_Rect calcAlignedDrawingRect(SDL_Surface* pSurface, SDL_Surface* pBaseSurface, HAlign halign = HAlign::Center, VAlign valign = VAlign::Center) {
    SDL_Rect rect = {0, 0, pBaseSurface->w, pBaseSurface->h};
    return calcAlignedDrawingRect(pSurface, rect, halign, valign);
}

/**
    Calculates the drawing rectangle for drawing pTexture at the edge or in the center of rect.
    The parameters halign and valign determine at which edge pTexture is drawn, e.g. HAlign::Left and VAlign::Top draws pTexture in the
    top left corner.
    \param  pTexture    the texture to calculate the rect for
    \param  rect        the rect to center around
    \param  halign      the horizontal alignment of pTexture (default is HAlign::Center)
    \param  valign      the vertical alignment of pTexture (default is VAlign::Center)
    \return the rectangle for drawing pTexture at the specified position when passed to SDL_RenderCopy
*/
inline SDL_Rect calcAlignedDrawingRect(SDL_Texture* pTexture, const SDL_Rect& rect, HAlign halign = HAlign::Center, VAlign valign = VAlign::Center) {
    int x = 0;
    int y = 0;

    switch(halign) {
        case HAlign::Left:      x = 0;          break;
        case HAlign::Center:    x = rect.w/2;   break;
        case HAlign::Right:     x = rect.w-1;   break;
    }

    switch(valign) {
        case VAlign::Top:       y = 0;          break;
        case VAlign::Center:    y = rect.h/2;   break;
        case VAlign::Bottom:    y = rect.h-1;   break;
    }

    return calcDrawingRect(pTexture, x, y, halign, valign);
}

/**
    Calculates the drawing rectangle for drawing pTexture at the edge or in the center of the current rendering target (usually the screen).
    The parameters halign and valign determine at which edge pTexture is drawn, e.g. HAlign::Left and VAlign::Top draws pTexture in the
    top left corner.
    \param  pTexture    the texture to calculate the rect for
    \param  halign      the horizontal alignment of pTexture (default is HAlign::Center)
    \param  valign      the vertical alignment of pTexture (default is VAlign::Center)
    \return the rectangle for drawing pTexture at the specified position when passed to SDL_RenderCopy
*/
inline SDL_Rect calcAlignedDrawingRect(SDL_Texture* pTexture, HAlign halign = HAlign::Center, VAlign valign = VAlign::Center) {
    return calcAlignedDrawingRect(pTexture, getRendererSize(), halign, valign);
}


#endif // DRAWINGRECTHELPER_H
