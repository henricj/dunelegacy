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

    const auto size = SDL_RWsize(pRWOP.get());

    pTTFFont = font_ptr{TTF_OpenFontRW(pRWOP.release(), 1, fontsize)};
    if (!pTTFFont) {
        THROW(std::invalid_argument, "TTFFont::TTFFont(): TTF_OpenFontRW() failed: %s!", TTF_GetError());
    }

    characterHeight = TTF_FontHeight(pTTFFont.get());
}

/// Destructor
/**
    Frees all memory.
*/
TTFFont::~TTFFont() = default;

sdl2::surface_ptr TTFFont::create_text_surface(std::string_view text, uint32_t baseColor) const {
    if (text.empty())
        return {};

    sdl2::surface_ptr surface{TTF_RenderUTF8_Blended(pTTFFont.get(), std::string{text}.c_str(), RGBA2SDL(baseColor))};

    //    SDL_BlendMode blend_mode;
    // SDL_GetSurfaceBlendMode(surface.get(), &blend_mode);

    // if (blend_mode == SDL_BLENDMODE_BLEND) {
    //     static auto count_blend = 0;
    //     ++count_blend;
    // } else if (blend_mode == SDL_BLENDMODE_NONE) {
    //     static auto count_none = 0;
    //     ++count_none;
    // }

    // SDL_SetSurfaceBlendMode(surface.get(), SDL_BLENDMODE_BLEND);

    return surface;
}

sdl2::surface_ptr TTFFont::create_multiline_text_surface(std::string_view text, uint32_t wrapLength,
                                                         uint32_t baseColor) const {
    if (text.empty())
        return {};

    sdl2::surface_ptr surface{
        TTF_RenderUTF8_Blended_Wrapped(pTTFFont.get(), std::string{text}.c_str(), RGBA2SDL(baseColor), wrapLength)};

    SDL_SetSurfaceBlendMode(surface.get(), SDL_BLENDMODE_BLEND);

    return surface;
}

void TTFFont::drawTextOnSurface(SDL_Surface* pSurface, std::string_view text, uint32_t baseColor) {

    if (!text.empty()) {
        const sdl2::surface_ptr pTextSurface{create_text_surface(text, baseColor)};

        if (!pTextSurface) {
            THROW(std::invalid_argument, "TTFFont::drawTextOnSurface(): TTF_RenderUTF8_Blended() failed: %s!",
                  TTF_GetError());
        }

        // SDL_BlendMode blend_mode;
        // SDL_GetSurfaceBlendMode(pTextSurface.get(), &blend_mode);

        // if (blend_mode == SDL_BLENDMODE_BLEND) {
        //     static auto count_blend = 0;
        //     ++count_blend;
        // } else if (blend_mode == SDL_BLENDMODE_NONE) {
        //     static auto count_none = 0;
        //     ++count_none;
        // }

        // SDL_SetSurfaceBlendMode(pTextSurface.get(), SDL_BLENDMODE_BLEND);

        SDL_Rect dest{0, -2, pTextSurface->w, pTextSurface->h};
        SDL_BlitSurface(pTextSurface.get(), nullptr, pSurface, &dest);
    }
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
        THROW(std::invalid_argument, "TTFFont::getTextWidth(): TTF_SizeUTF8() failed: %s!", TTF_GetError());
    }

    return width;
}
