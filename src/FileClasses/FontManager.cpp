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

#include <FileClasses/FileManager.h>
#include <FileClasses/Fntfile.h>
#include <FileClasses/PictureFont.h>

#include <list>

FontManager::FontManager() {
	fonts[FONT_STD10] = std::shared_ptr<Font>(new PictureFont(SDL_LoadBMP_RW(pFileManager->openFile("Font10.bmp"),true), true));
	fonts[FONT_STD12] = std::shared_ptr<Font>(new PictureFont(SDL_LoadBMP_RW(pFileManager->openFile("Font12.bmp"),true), true));
	fonts[FONT_STD24] = std::shared_ptr<Font>(new PictureFont(SDL_LoadBMP_RW(pFileManager->openFile("Font24.bmp"),true), true));
}

FontManager::~FontManager() {
}

void FontManager::drawTextOnSurface(SDL_Surface* pSurface, std::string text, Uint32 color, unsigned int fontNum) {
	if(fontNum >= NUM_FONTS) {
		return;
	}

	fonts[fontNum]->drawTextOnSurface(pSurface,text,color);
}

int	FontManager::getTextWidth(std::string text, unsigned int fontNum) {
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
		return 0;
	}

    SDL_Surface* pic;

    int width = fonts[fontNum]->getTextWidth(text);
    int height = fonts[fontNum]->getTextHeight();

    // create new picture surface
    if((pic = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)) == NULL) {
        return NULL;
    }

    SDL_FillRect(pic, NULL, COLOR_INVALID);
    SDL_SetAlpha(pic, SDL_RLEACCEL, 0);
    SDL_SetColorKey(pic, SDL_RLEACCEL | SDL_SRCCOLORKEY, COLOR_INVALID);

    fonts[fontNum]->drawTextOnSurface(pic,text,color);

    return pic;
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
    if((pic = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)) == NULL) {
        return NULL;
    }

    SDL_FillRect(pic, NULL, COLOR_INVALID);
    SDL_SetAlpha(pic, SDL_RLEACCEL, 0);
    SDL_SetColorKey(pic, SDL_RLEACCEL | SDL_SRCCOLORKEY, COLOR_INVALID);

    int line = 0;
    std::list<std::string>::iterator iter;
    for(iter = textLines.begin(); iter != textLines.end(); ++iter, line++) {
        SDL_Surface* tmpSurface = createSurfaceWithText(*iter, color, fontNum);

        SDL_Rect dest = {   static_cast<Sint16>((bCentered == false) ? 0 : (width - tmpSurface->w)/2),
                            static_cast<Sint16>(line*lineHeight),
                            static_cast<Uint16>(tmpSurface->w),
                            static_cast<Uint16>(tmpSurface->h) };
        SDL_BlitSurface(tmpSurface,NULL,pic,&dest);

        SDL_FreeSurface(tmpSurface);
    }

    return pic;
}
