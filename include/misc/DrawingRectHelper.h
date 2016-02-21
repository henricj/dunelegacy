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

#include <SDL.h>

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
    Calculates the drawing rectangle for drawing pTexture at (x,y). The parameters halign and valign determine which coordinate in pTexture
    is drawn at (x,y), e.g. if they are Right and Bottom the bottom right corner of pTexture is drawn at position (x,y)
    \param  pTexture    the texture to calculate the rect for
    \param  x           the x-coordinate
    \param  y           the y-coordinate
    \param  halign      the horizontal alignment of pTexture (default is HAlign::Left)
    \param  valign      the vertical alignment of pTexture (default is VAlign::Top)
    \return the rectangle for drawing pTexture at the specified position when passed to SDL_RenderCopy
*/
inline SDL_Rect calcDrawingRect(SDL_Texture* pTexture, int x, int y, HAlign halign = HAlign::Left, VAlign valign = VAlign::Top) {
    SDL_Rect rect = { x, y, 0, 0 };
    SDL_QueryTexture(pTexture, NULL, NULL, &rect.w, &rect.h);

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
    Calculates the drawing rectangle for drawing pTexture at the edge or in the center of the current rendering target (usually the screen).
    The parameters halign and valign determine at which edge pTexture is drawn, e.g. HAlign::Left and VAlign::Top draws pTexture in the
    top left corner.
    \param  pTexture    the texture to calculate the rect for
    \param  halign      the horizontal alignment of pTexture (default is HAlign::Center)
    \param  valign      the vertical alignment of pTexture (default is VAlign::Center)
    \return the rectangle for drawing pTexture at the specified position when passed to SDL_RenderCopy
*/
inline SDL_Rect calcAlignedDrawingRect(SDL_Texture* pTexture, HAlign halign = HAlign::Center, VAlign valign = VAlign::Center) {
    SDL_Rect renderRect = getRendererSize();

    int x = 0;
    int y = 0;

    switch(halign) {
        case HAlign::Left:      x = 0;              break;
        case HAlign::Center:    x = renderRect.w/2; break;
        case HAlign::Right:     x = renderRect.w-1; break;
    }

    switch(valign) {
        case VAlign::Top:       y = 0;              break;
        case VAlign::Center:    y = renderRect.h/2; break;
        case VAlign::Bottom:    y = renderRect.h-1; break;
    }

    return calcDrawingRect(pTexture, x, y, halign, valign);
}


#endif // DRAWINGRECTHELPER_H
