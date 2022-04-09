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

int FontManager::getTextWidth(std::string_view text, unsigned int fontSize) {
    return getFont(fontSize)->getTextWidth(text);
}

int FontManager::getTextHeight(unsigned int fontSize) {
    return getFont(fontSize)->getTextHeight();
}

sdl2::surface_ptr FontManager::createSurfaceWithText(std::string_view text, uint32_t color, unsigned int fontSize) {
    const auto* const pFont = getFont(fontSize);

    return pFont->createTextSurface(text, color);
}

sdl2::texture_ptr FontManager::createTextureWithText(std::string_view text, uint32_t color, unsigned int fontSize) {
    return convertSurfaceToTexture(createSurfaceWithText(text, color, fontSize));
}

sdl2::surface_ptr FontManager::createSurfaceWithMultilineText(std::string_view text, uint32_t color,
                                                              unsigned int fontSize, bool bCentered) {
    size_t startpos = 0;
    size_t nextpos  = 0;
    std::vector<std::string> textLines;
    do {
        nextpos = text.find('\n', startpos);
        if (nextpos == std::string::npos) {
            textLines.emplace_back(text.substr(startpos, text.length() - startpos));
        } else {
            textLines.emplace_back(text.substr(startpos, nextpos - startpos));
            startpos = nextpos + 1;
        }
    } while (nextpos != std::string::npos);

    const auto* const pFont = getFont(fontSize);

    const auto lineHeight = pFont->getTextHeight();
    const auto width      = pFont->getTextWidth(text);
    const int height      = lineHeight * textLines.size() + lineHeight * (textLines.size() - 1) / 2;

    // create new picture surface
    auto pic = sdl2::surface_ptr{SDL_CreateRGBSurfaceWithFormat(0, width, height, SCREEN_BPP, SCREEN_FORMAT)};
    if (pic == nullptr) {
        return nullptr;
    }

    SDL_SetSurfaceBlendMode(pic.get(), SDL_BLENDMODE_BLEND);
    SDL_FillRect(pic.get(), nullptr, SDL_MapRGBA(pic->format, 0, 0, 0, 0));

    auto currentLineNum = 0;
    for (const auto& textLine : textLines) {
        auto tmpSurface = createSurfaceWithText(textLine, color, fontSize);

        auto dest = calcDrawingRect(tmpSurface.get(), bCentered ? width / 2 : 0, currentLineNum * lineHeight,
                                    bCentered ? HAlign::Center : HAlign::Left, VAlign::Top);
        SDL_BlitSurface(tmpSurface.get(), nullptr, pic.get(), &dest);

        currentLineNum++;
    }

    return pic;
}

sdl2::texture_ptr FontManager::createTextureWithMultilineText(std::string_view text, uint32_t color,
                                                              unsigned int fontSize, bool bCentered) {
    return convertSurfaceToTexture(createSurfaceWithMultilineText(text, color, fontSize, bCentered));
}

Font* FontManager::getFont(uint32_t fontSize) {

    auto& font = fonts[fontSize];

    if (!font)
        font = loadFont(fontSize);

    return font.get();
}

std::unique_ptr<Font> FontManager::loadFont(unsigned int fontSize) const {
    return std::make_unique<TTFFont>(pFileManager->openFile(font_path_), fontSize);
}
