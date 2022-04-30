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

#include "FileClasses/Font.h"

#include <misc/SDL2pp.h>

#include <filesystem>
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

    Font* getFont(uint32_t fontSize);

private:
    std::unique_ptr<Font> loadFont(unsigned int fontSize) const;

    std::filesystem::path font_path_;
    std::unordered_map<uint32_t, std::unique_ptr<Font>> fonts;
};

#endif // FONTMANAGER_H
