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

#include <FileClasses/FontManager.h>

#include <globals.h>

#include <misc/FileSystem.h>
#include <misc/draw_util.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/LoadSavePNG.h>
#include <FileClasses/TTFFont.h>

FontManager::FontManager(std::filesystem::path font_path) : font_path_(std::move(font_path)) { }

FontManager::~FontManager() = default;

// int FontManager::getTextWidth(std::string_view text, unsigned int fontSize) {
//     return getFont(fontSize)->getTextWidth(text);
// }
//
// int FontManager::getTextHeight(unsigned int fontSize) {
//     return getFont(fontSize)->getTextHeight();
// }
//
// sdl2::surface_ptr FontManager::createSurfaceWithText(std::string_view text, uint32_t color, unsigned int fontSize) {
//     const auto* const pFont = getFont(fontSize);
//
//     return pFont->createTextSurface(text, color);
// }
//
// sdl2::texture_ptr FontManager::createTextureWithText(std::string_view text, uint32_t color, unsigned int fontSize) {
//     return convertSurfaceToTexture(createSurfaceWithText(text, color, fontSize));
// }
//
// sdl2::surface_ptr FontManager::createSurfaceWithMultilineText(std::string_view text, uint32_t color,
//                                                               unsigned int fontSize, bool bCentered) {
//     const auto* const pFont = getFont(fontSize);
//
//     return pFont->createMultilineTextSurface(text, color, bCentered);
// }
//
// sdl2::texture_ptr FontManager::createTextureWithMultilineText(std::string_view text, uint32_t color,
//                                                               unsigned int fontSize, bool bCentered) {
//     return convertSurfaceToTexture(createSurfaceWithMultilineText(text, color, fontSize, bCentered));
// }

Font* FontManager::getFont(uint32_t fontSize) {

    auto& font = fonts[fontSize];

    if (!font)
        font = loadFont(fontSize);

    return font.get();
}

std::unique_ptr<Font> FontManager::loadFont(unsigned int fontSize) const {
    return std::make_unique<TTFFont>(dune::globals::pFileManager->openFile(font_path_), fontSize);
}
