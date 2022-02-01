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

#include <misc/draw_util.h>
#include <misc/FileSystem.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/TTFFont.h>
#include <FileClasses/LoadSavePNG.h>

FontManager::FontManager() = default;

FontManager::~FontManager() = default;

void FontManager::drawTextOnSurface(SDL_Surface* pSurface, const std::string& text, Uint32 color, unsigned int fontSize) {
    return getFont(fontSize)->drawTextOnSurface(pSurface,text,color);
}

int FontManager::getTextWidth(const std::string& text, unsigned int fontSize) {
    return getFont(fontSize)->getTextWidth(text);
}

int FontManager::getTextHeight(unsigned int fontSize) {
    return getFont(fontSize)->getTextHeight();
}

sdl2::surface_ptr FontManager::createSurfaceWithText(const std::string& text, Uint32 color, unsigned int fontSize) {
    Font* pFont = getFont(fontSize);

    int width = pFont->getTextWidth(text);
    int height = pFont->getTextHeight();
    sdl2::surface_ptr pic = sdl2::surface_ptr{ SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK) };

    SDL_SetSurfaceBlendMode(pic.get(), SDL_BLENDMODE_NONE);
    SDL_FillRect(pic.get(), nullptr, COLOR_INVALID);
    SDL_SetColorKey(pic.get(), SDL_TRUE, COLOR_INVALID);

<<<<<<< HEAD
    pFont->drawTextOnSurface(pic.get(),text,color);
=======
    font->drawTextOnSurface(pic.get(),text, color);
>>>>>>> Use unique_ptr<> instead of shard_ptr<> for FontManager::fonts.

    return pic;
}

sdl2::texture_ptr FontManager::createTextureWithText(const std::string& text, Uint32 color, unsigned int fontSize) {
    return convertSurfaceToTexture(createSurfaceWithText(text, color, fontSize));
}

sdl2::surface_ptr FontManager::createSurfaceWithMultilineText(const std::string& text, Uint32 color, unsigned int fontSize, bool bCentered) {
    size_t startpos = 0;
    size_t nextpos;
    std::vector<std::string> textLines;
    do {
        nextpos = text.find('\n',startpos);
        if(nextpos == std::string::npos) {
            textLines.emplace_back(text.substr(startpos,text.length()-startpos));
        } else {
            textLines.emplace_back(text.substr(startpos,nextpos-startpos));
            startpos = nextpos+1;
        }
    } while(nextpos != std::string::npos);

    const auto pFont = getFont(fontSize);

    int lineHeight = pFont->getTextHeight();
    int width = pFont->getTextWidth(text);
    int height = lineHeight * textLines.size() + (lineHeight * (textLines.size()-1))/2;

    // create new picture surface
    auto pic = sdl2::surface_ptr{ SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK) };
    if(pic == nullptr) {
        return nullptr;
    }

    SDL_SetSurfaceBlendMode(pic.get(), SDL_BLENDMODE_NONE);
    SDL_FillRect(pic.get(), nullptr, COLOR_INVALID);
    SDL_SetColorKey(pic.get(), SDL_TRUE, COLOR_INVALID);

    int currentLineNum = 0;
    for(const std::string& textLine : textLines) {
        auto tmpSurface = createSurfaceWithText(textLine, color, fontSize);

        SDL_Rect dest = calcDrawingRect(tmpSurface.get(), bCentered ? width/2 : 0, currentLineNum*lineHeight, bCentered ? HAlign::Center : HAlign::Left, VAlign::Top);
        SDL_BlitSurface(tmpSurface.get(),nullptr,pic.get(),&dest);

        currentLineNum++;
    }

    return pic;
}

sdl2::texture_ptr FontManager::createTextureWithMultilineText(const std::string& text, Uint32 color, unsigned int fontSize, bool bCentered) {
    return convertSurfaceToTexture(createSurfaceWithMultilineText(text, color, fontSize, bCentered));
}

std::unique_ptr<Font> FontManager::loadFont(unsigned int fontSize) {
    return std::make_unique<TTFFont>( pFileManager->openFile("Philosopher-Bold.ttf"), fontSize );
}
