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

#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include <misc/SDL2pp.h>
#include "Font.h"

#include <memory>
#include <string>

typedef enum {
    FONT_STD10,
    FONT_STD12,
    FONT_STD24,
    NUM_FONTS
} Fonts_enum;

/// A class for managing fonts.
/**
    This class manages all fonts used in Dune Legacy and provides methods for rendering texts with a specific font.
*/
class FontManager
{
public:
    FontManager();
    ~FontManager();

    void drawTextOnSurface(SDL_Surface* pSurface, const std::string& text, Uint32 color, unsigned int fontNum);
    int getTextWidth(const std::string& text, unsigned int fontNum) const;
    int getTextHeight(unsigned int fontNum) const;
    sdl2::surface_ptr createSurfaceWithText(const std::string& text, Uint32 color, unsigned int fontNum);
    sdl2::texture_ptr createTextureWithText(const std::string& text, Uint32 color, unsigned int fontNum);
    sdl2::surface_ptr createSurfaceWithMultilineText(const std::string& text, Uint32 color, unsigned int fontNum, bool bCentered = false);
    sdl2::texture_ptr createTextureWithMultilineText(const std::string& text, Uint32 color, unsigned int fontNum, bool bCentered = false);
private:
    std::shared_ptr<Font> fonts[NUM_FONTS];

};

#endif // FONTMANAGER_H
