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

#ifndef TTFFONT_H
#define TTFFONT_H

#include "Font.h"
#include <misc/SDL2pp.h>

#include <SDL_ttf.h>

#include <vector>

typedef sdl2::implementation::unique_ptr_deleter<TTF_Font, TTF_CloseFont> font_ptr;

/// A class for loading a ttf font.
/**
    This class can read a ttf font.
*/
class TTFFont : public Font
{
public:
    TTFFont(sdl2::RWops_ptr pRWOP, int fontsize);
    virtual ~TTFFont();

    void drawTextOnSurface(SDL_Surface* pSurface, const std::string& text, Uint32 baseColor = 0xFFFFFFFF) override;

    int getTextWidth(const std::string& text) const override;

    /// Returns the number of pixels this font needs in y-direction.
    /**
        This methods returns the height of this font.
        \return Number of pixels needed
    */
    inline int getTextHeight() const override { return characterHeight; };

private:
    font_ptr pTTFFont;
    int characterHeight;
};

#endif //TTFFONT_H
