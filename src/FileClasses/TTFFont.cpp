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

#include <FileClasses/TTFFont.h>
#include <misc/exceptions.h>

#include <Colors.h>

/// Constructor
/**
    The constructor reads the font from the ttf file given via rwop.
    \param  pRWOP        The file which contains the font
    \param  fontsize    The size of the font in pixels
*/
TTFFont::TTFFont(sdl2::RWops_ptr pRWOP, int fontsize) {
    if (!pRWOP) {
        THROW(std::invalid_argument, "TTFFont::TTFFont(): pRWOP == nullptr!");
    }

    pTTFFont = font_ptr{TTF_OpenFontRW(pRWOP.release(), 1, fontsize)};
    if (!pTTFFont) {
        THROW(std::invalid_argument, "TTFFont::TTFFont(): TTF_OpenFontRW() failed: {}!", TTF_GetError());
    }

    characterHeight = TTF_FontLineSkip(pTTFFont.get());
}

/// Destructor
/**
    Frees all memory.
*/
TTFFont::~TTFFont() = default;

sdl2::surface_ptr TTFFont::createTextSurface(std::string_view text, uint32_t baseColor) const {
    if (text.empty())
        return {};

    sdl2::surface_ptr surface{TTF_RenderUTF8_Blended(pTTFFont.get(), std::string{text}.c_str(), RGBA2SDL(baseColor))};

    return surface;
}

/// Returns the number of pixels a text needs
/**
        This methods returns the number of pixels this text would need if printed.
        \param  text    The text to be checked for it's length in pixel
        \return Number of pixels needed
*/
int TTFFont::getTextWidth(std::string_view text) const {

    int width = 0;
    if (TTF_SizeUTF8(pTTFFont.get(), std::string{text}.c_str(), &width, nullptr) < 0) {
        THROW(std::invalid_argument, "TTFFont::getTextWidth(): TTF_SizeUTF8() failed: {}!", TTF_GetError());
    }

    return width;
}
