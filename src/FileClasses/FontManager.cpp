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

#include <FileClasses/FileManager.h>
#include <FileClasses/PictureFont.h>
#include <FileClasses/LoadSavePNG.h>

#include <list>

FontManager::FontManager() {
    fonts[FONT_STD10] = std::shared_ptr<Font>(new PictureFont(LoadPNG_RW(pFileManager->openFile("Font10.png"),true), true));
    fonts[FONT_STD12] = std::shared_ptr<Font>(new PictureFont(LoadPNG_RW(pFileManager->openFile("Font12.png"),true), true));
    fonts[FONT_STD24] = std::shared_ptr<Font>(new PictureFont(LoadPNG_RW(pFileManager->openFile("Font24.png"),true), true));
}

FontManager::~FontManager() {
}

void FontManager::drawTextOnSurface(SDL_Surface* pSurface, std::string text, Uint32 color, unsigned int fontNum) {
    if(fontNum >= NUM_FONTS) {
        return;
    }

    fonts[fontNum]->drawTextOnSurface(pSurface,text,color);
}

int FontManager::getTextWidth(std::string text, unsigned int fontNum) {
    if(fontNum >= NUM_FONTS) {
        return 0;
    }

    return fonts[fontNum]->getTextWidth(text);
}

int FontManager::getTextHeight(unsigned int fontNum) {
    if(fontNum >= NUM_FONTS) {
        return 0;
    }

    return fonts[fontNum]->getTextHeight();
}

SDL_Surface* FontManager::createSurfaceWithText(std::string text, Uint32 color, unsigned int fontNum) {
    if(fontNum >= NUM_FONTS) {
        return nullptr;
    }

    SDL_Surface* pic;

    int width = fonts[fontNum]->getTextWidth(text);
    int height = fonts[fontNum]->getTextHeight();

    // create new picture surface
    if((pic = SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)) == nullptr) {
        return nullptr;
    }

    SDL_SetSurfaceBlendMode(pic, SDL_BLENDMODE_NONE);
    SDL_FillRect(pic, nullptr, COLOR_INVALID);
    SDL_SetColorKey(pic, SDL_TRUE, COLOR_INVALID);

    fonts[fontNum]->drawTextOnSurface(pic,text,color);

    return pic;
}

SDL_Texture* FontManager::createTextureWithText(std::string text, Uint32 color, unsigned int fontNum) {
    return convertSurfaceToTexture(createSurfaceWithText(text, color, fontNum), true);
}

SDL_Surface* FontManager::createSurfaceWithMultilineText(std::string text, Uint32 color, unsigned int fontNum, bool bCentered) {
    if(fontNum >= NUM_FONTS) {
        return 0;
    }

    size_t startpos = 0;
    size_t nextpos;
    std::list<std::string> textLines;
    do {
        nextpos = text.find("\n",startpos);
        if(nextpos == std::string::npos) {
            textLines.push_back(text.substr(startpos,text.length()-startpos));
        } else {
            textLines.push_back(text.substr(startpos,nextpos-startpos));
            startpos = nextpos+1;
        }
    } while(nextpos != std::string::npos);

    SDL_Surface* pic;

    int lineHeight = fonts[fontNum]->getTextHeight();
    int width = fonts[fontNum]->getTextWidth(text);
    int height = lineHeight * textLines.size() + (lineHeight * (textLines.size()-1))/2;

    // create new picture surface
    if((pic = SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)) == nullptr) {
        return nullptr;
    }

    SDL_SetSurfaceBlendMode(pic, SDL_BLENDMODE_NONE);
    SDL_FillRect(pic, nullptr, COLOR_INVALID);
    SDL_SetColorKey(pic, SDL_TRUE, COLOR_INVALID);

    int currentLineNum = 0;
    for(const std::string& textLine : textLines) {
        SDL_Surface* tmpSurface = createSurfaceWithText(textLine, color, fontNum);

        SDL_Rect dest = calcDrawingRect(tmpSurface, bCentered ? width/2 : 0, currentLineNum*lineHeight, bCentered ? HAlign::Center : HAlign::Left, VAlign::Top);
        SDL_BlitSurface(tmpSurface,nullptr,pic,&dest);

        SDL_FreeSurface(tmpSurface);

        currentLineNum++;
    }

    return pic;
}

SDL_Texture* FontManager::createTextureWithMultilineText(std::string text, Uint32 color, unsigned int fontNum, bool bCentered) {
    return convertSurfaceToTexture(createSurfaceWithMultilineText(text, color, fontNum, bCentered), true);
}
