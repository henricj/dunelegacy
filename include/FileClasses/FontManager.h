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

#include <memory>
#include <string_view>
#include <unordered_map>

class Font;

/// A class for managing fonts.
/**
    This class manages all fonts used in Dune Legacy and provides methods for rendering texts with a specific font.
*/
class FontManager final {
public:
    FontManager(std::filesystem::path font_path);
    ~FontManager();

    FontManager(const FontManager&)            = delete;
    FontManager(FontManager&&)                 = delete;
    FontManager& operator=(const FontManager&) = delete;
    FontManager& operator=(FontManager&&)      = delete;

    int getTextWidth(std::string_view text, unsigned int fontSize);
    int getTextHeight(unsigned int fontSize);

    sdl2::texture_ptr createTextureWithText(std::string_view text, uint32_t color, unsigned int fontSize);
    sdl2::texture_ptr createTextureWithMultilineText(std::string_view text, uint32_t color, unsigned int fontSize,
                                                     bool bCentered = false);

    sdl2::surface_ptr createSurfaceWithText(std::string_view, uint32_t color, unsigned int fontSize);
    sdl2::surface_ptr createSurfaceWithMultilineText(std::string_view text, uint32_t color, unsigned int fontSize,
                                                     bool bCentered = false);

private:
    Font* getFont(uint32_t fontSize);

    std::unique_ptr<Font> loadFont(unsigned int fontSize) const;

    std::filesystem::path font_path_;
    std::unordered_map<uint32_t, std::unique_ptr<Font>> fonts;
};

#endif // FONTMANAGER_H
