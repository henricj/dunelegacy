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

#include <SDL2/SDL_ttf.h>

#include <string_view>

using font_ptr = sdl2::implementation::unique_ptr_deleter<TTF_Font, TTF_CloseFont>;

/// A class for loading a ttf font.
/**
    This class can read a ttf font.
*/
class TTFFont final : public Font {
public:
    TTFFont(sdl2::RWops_ptr pRWOP, int fontsize);
    ~TTFFont() override;

    TTFFont(const TTFFont&)            = delete;
    TTFFont(TTFFont&&)                 = delete;
    TTFFont& operator=(const TTFFont&) = delete;
    TTFFont& operator=(TTFFont&&)      = delete;

    [[nodiscard]] sdl2::surface_ptr
    createTextSurface(std::string_view text, uint32_t baseColor = 0xFFFFFFFFu) const override;

    [[nodiscard]] int getTextWidth(std::string_view text) const override;

    /// Returns the number of pixels this font needs in y-direction.
    /**
        This methods returns the height of this font.
        \return Number of pixels needed
    */
    [[nodiscard]] int getTextHeight() const override { return characterHeight; }

private:
    font_ptr pTTFFont;
    int characterHeight;
};

#endif // TTFFONT_H
