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

#include "Font.h"

#include <misc/SDL2pp.h>
#include <misc/sdl_support.h>

#include <map>
#include <string_view>

/// A class for managing fonts.
/**
    This class manages all fonts used in Dune Legacy and provides methods for rendering texts with a specific font.
*/
class FontManager {
public:
    FontManager();
    ~FontManager();

    FontManager(const FontManager&) = delete;
    FontManager(FontManager&&)      = delete;
    FontManager& operator=(const FontManager&) = delete;
    FontManager& operator=(FontManager&&) = delete;

    void drawTextOnSurface(SDL_Surface* pSurface, std::string_view text, uint32_t color, unsigned int fontSize);
    int getTextWidth(std::string_view text, unsigned int fontSize);
    int getTextHeight(unsigned int fontSize);
    sdl2::surface_ptr createSurfaceWithText(std::string_view, uint32_t color, unsigned int fontSize);
    sdl2::texture_ptr createTextureWithText(std::string_view text, uint32_t color, unsigned int fontSize);
    sdl2::surface_ptr createSurfaceWithMultilineText(std::string_view text, uint32_t color, unsigned int fontSize,
                                                     bool bCentered = false);
    sdl2::texture_ptr createTextureWithMultilineText(std::string_view text, uint32_t color, unsigned int fontSize,
                                                     bool bCentered = false);

private:
    Font* getFont(unsigned int fontSize) {
        const auto iter = fonts.find(fontSize);

        if (iter != fonts.end()) {
            return iter->second.get();
        }

        fonts[fontSize] = loadFont(fontSize);
        return fonts[fontSize].get();
    }

    std::unique_ptr<Font> loadFont(unsigned int fontSize);

    std::map<unsigned int, std::unique_ptr<Font>> fonts;
};

#endif // FONTMANAGER_H
