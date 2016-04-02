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

#include <GUI/dune/DuneStyle.h>
#include <misc/draw_util.h>
#include <GUI/Widget.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/FontManager.h>

#include <iostream>
#include <cmath>
#include <algorithm>

extern GFXManager*  pGFXManager;
extern FontManager* pFontManager;


SDL_Surface* DuneStyle::createSurfaceWithText(const char* text, Uint32 color, unsigned int fontsize) {
	if(pFontManager != NULL) {
		return pFontManager->createSurfaceWithText(text, color, fontsize);
	} else {
		// create dummy surface
		SDL_Surface* surface;

		if((surface = SDL_CreateRGBSurface(0, strlen(text)*10, 12, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)) == NULL) {
			return NULL;
		}

        SDL_FillRect(surface, NULL, COLOR_TRANSPARENT);

		return surface;
	}
}

unsigned int DuneStyle::getTextHeight(unsigned int FontNum) {
	if(pFontManager != NULL) {
		return pFontManager->getTextHeight(FontNum);
	} else {
		return 12;
	}
}

unsigned int DuneStyle::getTextWidth(const char* text, unsigned int FontNum)  {
	if(pFontManager != NULL) {
		return pFontManager->getTextWidth(text,FontNum);
	} else {
		return strlen(text)*10;
	}
}


Point DuneStyle::getMinimumLabelSize(std::string text, int fontID) {
	return Point(getTextWidth(text.c_str(),fontID) + 12,getTextHeight(fontID) + 4);
}

SDL_Surface* DuneStyle::createLabelSurface(Uint32 width, Uint32 height, std::list<std::string> textLines, int fontID, Alignment_Enum alignment, Uint32 textcolor, Uint32 textshadowcolor, Uint32 backgroundcolor) {
	SDL_Surface* surface;

	// create surfaces
    if((surface = SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)) == NULL) {
        return NULL;
    }

	SDL_FillRect(surface, NULL, backgroundcolor);

	if(textcolor == COLOR_DEFAULT) textcolor = defaultForegroundColor;
	if(textshadowcolor == COLOR_DEFAULT) textshadowcolor = defaultShadowColor;

	std::list<SDL_Surface*> TextSurfaces;
	std::list<std::string>::const_iterator iter;
	for(iter = textLines.begin(); iter != textLines.end() ; ++iter) {
		std::string text = *iter;

		// create text background
		TextSurfaces.push_back(createSurfaceWithText(text.c_str(), textshadowcolor, fontID));
		// create text foreground
		TextSurfaces.push_back(createSurfaceWithText(text.c_str(), textcolor, fontID));
	}

	int fontheight = getTextHeight(fontID);
	int spacing = 2;

	int textpos_y;

	if(alignment & Alignment_VCenter) {
		int textheight = fontheight * textLines.size() + spacing * (textLines.size() - 1);
		textpos_y = (((int) height) - textheight) / 2;
	} else if(alignment & Alignment_Bottom) {
		int textheight = fontheight * textLines.size() + spacing * (textLines.size() - 1);
		textpos_y = ((int) height) - textheight - spacing;
	} else {
		// Alignment_Top
		textpos_y = spacing;
	}

	std::list<SDL_Surface*>::const_iterator surfIter = TextSurfaces.begin();
	while(surfIter != TextSurfaces.end()) {
		SDL_Surface* textSurface1 = *surfIter;
		++surfIter;
        SDL_Surface* textSurface2 = *surfIter;
        ++surfIter;

		SDL_Rect textRect1 = calcDrawingRect(textSurface1, 0, textpos_y + 3);
        SDL_Rect textRect2 = calcDrawingRect(textSurface2, 0, textpos_y + 2);
		if(alignment & Alignment_Left) {
			textRect1.x = 4;
            textRect2.x = 3;
        } else if(alignment & Alignment_Right) {
			textRect1.x = width - textSurface1->w - 4;
            textRect2.x = width - textSurface2->w - 3;
        } else {
            // Alignment_HCenter
			textRect1.x = ((surface->w - textSurface1->w) / 2)+3;
            textRect2.x = ((surface->w - textSurface2->w) / 2)+2;
		}

		SDL_BlitSurface(textSurface1,NULL,surface,&textRect1);
		SDL_FreeSurface(textSurface1);

        SDL_BlitSurface(textSurface2,NULL,surface,&textRect2);
        SDL_FreeSurface(textSurface2);

		textpos_y += fontheight + spacing;
	}

	return surface;
}





Point DuneStyle::getMinimumCheckboxSize(std::string text) {
	return Point(getTextWidth(text.c_str(),FONT_STD12) + 20 + 17,getTextHeight(FONT_STD12) + 8);
}

SDL_Surface* DuneStyle::createCheckboxSurface(Uint32 width, Uint32 height, std::string text, bool checked, bool activated, Uint32 textcolor, Uint32 textshadowcolor, Uint32 backgroundcolor) {
	SDL_Surface* surface;

	// create surfaces
    if((surface = SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)) == NULL) {
        return NULL;
    }

	SDL_FillRect(surface, NULL, backgroundcolor);

	if(textcolor == COLOR_DEFAULT) textcolor = defaultForegroundColor;
	if(textshadowcolor == COLOR_DEFAULT) textshadowcolor = defaultShadowColor;

    if(activated) {
        textcolor = brightenUp(textcolor);
    }

	drawRect(surface, 4, 5, 4 + 17, 5 + 17, textcolor);
	drawRect(surface, 4 + 1, 5 + 1, 4 + 17 - 1, 5 + 17 - 1, textcolor);

	if(checked) {
		int x1 = 4 + 2;
		int y1 = 5 + 2;
		int x2 = 4 + 17 - 2;

		for(int i = 0; i < 15; i++) {
			// North-West to South-East
			putPixel(surface, x1, y1, textcolor);
			putPixel(surface, x1+1, y1, textcolor);
			putPixel(surface, x1-1, y1, textcolor);

			// North-East to South-West
			putPixel(surface, x2, y1, textcolor);
			putPixel(surface, x2+1, y1, textcolor);
			putPixel(surface, x2-1, y1, textcolor);

			x1++;
			y1++;
			x2--;
		}
	}


	SDL_Surface* textSurface1 = createSurfaceWithText(text.c_str(), textshadowcolor, FONT_STD12);
	SDL_Rect textRect1 = calcDrawingRect(textSurface1,  10+2 + 17, surface->h/2 + 3, HAlign::Left, VAlign::Center);
	SDL_BlitSurface(textSurface1,NULL,surface,&textRect1);
	SDL_FreeSurface(textSurface1);

	SDL_Surface* textSurface2 = createSurfaceWithText(text.c_str(), textcolor, FONT_STD12);
	SDL_Rect textRect2 = calcDrawingRect(textSurface2,  10+1 + 17, surface->h/2 + 2, HAlign::Left, VAlign::Center);
	SDL_BlitSurface(textSurface2,NULL,surface,&textRect2);
	SDL_FreeSurface(textSurface2);

	return surface;
}





Point DuneStyle::getMinimumRadioButtonSize(std::string text) {
	return Point(getTextWidth(text.c_str(),FONT_STD12) + 16 + 15,getTextHeight(FONT_STD12) + 8);
}

SDL_Surface* DuneStyle::createRadioButtonSurface(Uint32 width, Uint32 height, std::string text, bool checked, bool activated, Uint32 textcolor, Uint32 textshadowcolor, Uint32 backgroundcolor) {
	SDL_Surface* surface;

	// create surfaces
    if((surface = SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)) == NULL) {
        return NULL;
    }

	SDL_FillRect(surface, NULL, backgroundcolor);

	if(textcolor == COLOR_DEFAULT) textcolor = defaultForegroundColor;
	if(textshadowcolor == COLOR_DEFAULT) textshadowcolor = defaultShadowColor;

    if(activated) {
        textcolor = brightenUp(textcolor);
    }

    drawHLineNoLock(surface, 8, 7, 13, textcolor);
    drawHLineNoLock(surface, 7, 8, 14, textcolor);
    drawHLineNoLock(surface, 6, 9, 8, textcolor);
    drawHLineNoLock(surface, 13, 9, 15, textcolor);
    drawHLineNoLock(surface, 5, 10, 6, textcolor);
    drawHLineNoLock(surface, 15, 10, 16, textcolor);
    drawHLineNoLock(surface, 4, 11, 6, textcolor);
    drawHLineNoLock(surface, 15, 11, 17, textcolor);

    drawVLineNoLock(surface, 4, 12, 15, textcolor);
    drawVLineNoLock(surface, 5, 12, 15, textcolor);
    drawVLineNoLock(surface, 16, 12, 15, textcolor);
    drawVLineNoLock(surface, 17, 12, 15, textcolor);

    drawHLineNoLock(surface, 4, 16, 6, textcolor);
    drawHLineNoLock(surface, 15, 16, 17, textcolor);
    drawHLineNoLock(surface, 5, 17, 6, textcolor);
    drawHLineNoLock(surface, 15, 17, 16, textcolor);
    drawHLineNoLock(surface, 6, 18, 8, textcolor);
    drawHLineNoLock(surface, 13, 18, 15, textcolor);
    drawHLineNoLock(surface, 7, 19, 14, textcolor);
    drawHLineNoLock(surface, 8, 20, 13, textcolor);

    if(checked) {
        drawHLineNoLock(surface, 9, 11, 12, textcolor);
        drawHLineNoLock(surface, 8, 12, 13, textcolor);
        drawHLineNoLock(surface, 8, 13, 13, textcolor);
        drawHLineNoLock(surface, 8, 14, 13, textcolor);
        drawHLineNoLock(surface, 8, 15, 13, textcolor);
        drawHLineNoLock(surface, 9, 16, 12, textcolor);
	}


	SDL_Surface* textSurface1 = createSurfaceWithText(text.c_str(), textshadowcolor, FONT_STD12);
	SDL_Rect textRect1 = calcDrawingRect(textSurface1, 8+2 + 15, surface->h/2 + 3, HAlign::Left, VAlign::Center);
	SDL_BlitSurface(textSurface1,NULL,surface,&textRect1);
	SDL_FreeSurface(textSurface1);

	SDL_Surface* textSurface2 = createSurfaceWithText(text.c_str(), textcolor, FONT_STD12);
	SDL_Rect textRect2 = calcDrawingRect(textSurface2, 8+1 + 15, surface->h/2 + 2, HAlign::Left, VAlign::Center);
	SDL_BlitSurface(textSurface2,NULL,surface,&textRect2);
	SDL_FreeSurface(textSurface2);

	return surface;
}



SDL_Surface* DuneStyle::createDropDownBoxButton(Uint32 size, bool pressed, bool activated, Uint32 color) {
    if(color == COLOR_DEFAULT) {
        color = defaultForegroundColor;
    }

	// create surfaces
	SDL_Surface* surface;
    if((surface = SDL_CreateRGBSurface(0, size, size, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)) == NULL) {
        return NULL;
    }

	// create button background
	if(pressed == false) {
		// normal mode
		SDL_FillRect(surface, NULL, buttonBackgroundColor);
		drawRect(surface, 0, 0, surface->w-1, surface->h-1, buttonBorderColor);
		drawHLine(surface, 1, 1, surface->w-2, buttonEdgeTopLeftColor);
		drawVLine(surface, 1, 1, surface->h-2, buttonEdgeTopLeftColor);
		drawHLine(surface, 1, surface->h-2, surface->w-2, buttonEdgeBottomRightColor);
		drawVLine(surface, surface->w-2, 1, surface->h-2, buttonEdgeBottomRightColor);
	} else {
		// pressed button mode
		SDL_FillRect(surface, NULL, pressedButtonBackgroundColor);
		drawRect(surface, 0, 0, surface->w-1, surface->h-1, buttonBorderColor);
		drawRect(surface, 1, 1, surface->w-2, surface->h-2, buttonEdgeBottomRightColor);
	}

	int col = (pressed | activated) ? brightenUp(color) : color;

	int x1 = 3;
	int x2 = size-3-1;
	int y = size/3 - 1;

	// draw separated hline
	drawHLine(surface, x1, y, x2, col);
	y+=2;

	// down arrow
	for(;x1 <= x2; ++x1, --x2, ++y) {
		drawHLine(surface, x1, y, x2, col);
	}

	return surface;
}




Point DuneStyle::getMinimumButtonSize(std::string text) {
	return Point(getTextWidth(text.c_str(),FONT_STD10)+12,getTextHeight(FONT_STD10));
}

SDL_Surface* DuneStyle::createButtonSurface(Uint32 width, Uint32 height, std::string text, bool pressed, bool activated, Uint32 textcolor, Uint32 textshadowcolor) {

	// create surfaces
	SDL_Surface* surface;
    if((surface = SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)) == NULL) {
        return NULL;
    }

	// create button background
	if(pressed == false) {
		// normal mode
		SDL_FillRect(surface, NULL, buttonBackgroundColor);
		drawRect(surface, 0, 0, surface->w-1, surface->h-1, buttonBorderColor);
		drawHLine(surface, 1, 1, surface->w-2, buttonEdgeTopLeftColor);
		drawVLine(surface, 1, 1, surface->h-2, buttonEdgeTopLeftColor);
		drawHLine(surface, 1, surface->h-2, surface->w-2, buttonEdgeBottomRightColor);
		drawVLine(surface, surface->w-2, 1, surface->h-2, buttonEdgeBottomRightColor);
	} else {
		// pressed button mode
		SDL_FillRect(surface, NULL, pressedButtonBackgroundColor);
		drawRect(surface, 0, 0, surface->w-1, surface->h-1, buttonBorderColor);
		drawRect(surface, 1, 1, surface->w-2, surface->h-2, buttonEdgeBottomRightColor);

	}

	// create text on this button
	int fontsize;
	if(	(width < getTextWidth(text.c_str(),FONT_STD12) + 12) ||
		(height < getTextHeight(FONT_STD12) + 2)) {
		fontsize = FONT_STD10;
	} else {
		fontsize = FONT_STD12;
	}

    if(textcolor == COLOR_DEFAULT) textcolor = defaultForegroundColor;
	if(textshadowcolor == COLOR_DEFAULT) textshadowcolor = defaultShadowColor;

	SDL_Surface* textSurface1 = createSurfaceWithText(text.c_str(), textshadowcolor, fontsize);
	SDL_Rect textRect1 = calcDrawingRect(textSurface1, surface->w/2 + 2 + (pressed ? 1 : 0), surface->h/2 + 3 + (pressed ? 1 : 0), HAlign::Center, VAlign::Center);
	SDL_BlitSurface(textSurface1,NULL,surface,&textRect1);
	SDL_FreeSurface(textSurface1);

	SDL_Surface* textSurface2 = createSurfaceWithText(text.c_str(), (activated == true) ? brightenUp(textcolor) : textcolor, fontsize);
	SDL_Rect textRect2 = calcDrawingRect(textSurface2, surface->w/2 + 1 + (pressed ? 1 : 0), surface->h/2 + 2 + (pressed ? 1 : 0), HAlign::Center, VAlign::Center);
	SDL_BlitSurface(textSurface2,NULL,surface,&textRect2);
	SDL_FreeSurface(textSurface2);

	return surface;
}




Point DuneStyle::getMinimumTextBoxSize(int fontID) {
	return Point(10,getTextHeight(fontID) + 6);
}

SDL_Surface* DuneStyle::createTextBoxSurface(Uint32 width, Uint32 height, std::string text, bool carret, int fontID, Alignment_Enum alignment, Uint32 textcolor, Uint32 textshadowcolor) {

	// create surfaces
	SDL_Surface* surface;
    if((surface = SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)) == NULL) {
        return NULL;
    }

	SDL_FillRect(surface, NULL, buttonBackgroundColor);
	drawRect(surface,0,0,surface->w-1,surface->h-1,buttonBorderColor);

	drawHLine(surface,1,1,surface->w-2,buttonEdgeBottomRightColor);
	drawHLine(surface,1,2,surface->w-2,buttonEdgeBottomRightColor);
	drawVLine(surface,1,1,surface->h-2,buttonEdgeBottomRightColor);
	drawVLine(surface,2,1,surface->h-2,buttonEdgeBottomRightColor);
	drawHLine(surface,1,surface->h-2,surface->w-2,buttonEdgeTopLeftColor);
	drawVLine(surface,surface->w-2,1,surface->h-2,buttonEdgeTopLeftColor);

    if(textcolor == COLOR_DEFAULT) textcolor = defaultForegroundColor;
	if(textshadowcolor == COLOR_DEFAULT) textshadowcolor = defaultShadowColor;

	SDL_Rect cursorPos;

	// create text in this text box
	if(text.size() != 0) {
		SDL_Surface* textSurface1 = createSurfaceWithText(text.c_str(), textshadowcolor, fontID);
		SDL_Surface* textSurface2 = createSurfaceWithText(text.c_str(), textcolor, fontID);
		SDL_Rect textRect1 = calcDrawingRect(textSurface1, 0, surface->h/2 + 3, HAlign::Left, VAlign::Center);
		SDL_Rect textRect2 = calcDrawingRect(textSurface2, 0, surface->h/2 + 2, HAlign::Left, VAlign::Center);

        if(alignment & Alignment_Left) {
            textRect1.x = 6;
            textRect2.x = 5;
        } else if(alignment & Alignment_Right) {
            textRect1.x = width - textSurface1->w - 5;
            textRect2.x = width - textSurface2->w - 4;
        } else {
            // Alignment_HCenter
            textRect1.x = ((surface->w - textSurface1->w) / 2)+3;
            textRect2.x = ((surface->w - textSurface2->w) / 2)+2;
        }

		if(textRect1.w > surface->w - 10) {
			textRect1.x -= (textSurface1->w - (surface->w - 10));
			textRect2.x -= (textSurface2->w - (surface->w - 10));
		}

		cursorPos.x = textRect2.x + textSurface2->w + 2;

		SDL_BlitSurface(textSurface1,NULL,surface,&textRect1);
		SDL_FreeSurface(textSurface1);

		SDL_BlitSurface(textSurface2,NULL,surface,&textRect2);
		SDL_FreeSurface(textSurface2);

		cursorPos.w = 1;
	} else {
		if(alignment & Alignment_Left) {
            cursorPos.x = 6;
        } else if(alignment & Alignment_Right) {
            cursorPos.x = width - 5;
        } else {
            // Alignment_HCenter
            cursorPos.x = surface->w / 2;
        }
		cursorPos.w = 1;
	}

	cursorPos.y = surface->h / 2 - 8;
	cursorPos.h = 16;

	if(carret == true) {
		drawVLine(surface,cursorPos.x,cursorPos.y,cursorPos.y+cursorPos.h,textcolor);
		drawVLine(surface,cursorPos.x+1,cursorPos.y,cursorPos.y+cursorPos.h,textcolor);
	}

	return surface;
}




Point DuneStyle::getMinimumScrollBarArrowButtonSize() {
	return Point(17,17);
}

SDL_Surface* DuneStyle::createScrollBarArrowButton(bool down, bool pressed, bool activated, Uint32 color) {
    if(color == COLOR_DEFAULT) {
        color = defaultForegroundColor;
    }

	// create surfaces
	SDL_Surface* surface;
    if((surface = SDL_CreateRGBSurface(0, 17, 17, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)) == NULL) {
        return NULL;
    }

	// create button background
	if(pressed == false) {
		// normal mode
		SDL_FillRect(surface, NULL, buttonBackgroundColor);
		drawRect(surface, 0, 0, surface->w-1, surface->h-1, buttonBorderColor);
		drawHLine(surface, 1, 1, surface->w-2, buttonEdgeTopLeftColor);
		drawVLine(surface, 1, 1, surface->h-2, buttonEdgeTopLeftColor);
		drawHLine(surface, 1, surface->h-2, surface->w-2, buttonEdgeBottomRightColor);
		drawVLine(surface, surface->w-2, 1, surface->h-2, buttonEdgeBottomRightColor);
	} else {
		// pressed button mode
		SDL_FillRect(surface, NULL, pressedButtonBackgroundColor);
		drawRect(surface, 0, 0, surface->w-1, surface->h-1, buttonBorderColor);
		drawRect(surface, 1, 1, surface->w-2, surface->h-2, buttonEdgeBottomRightColor);
	}

	int col = (pressed | activated) ? brightenUp(color) : color;

	// draw arrow
	if(down == true) {
		// down arrow
		drawHLine(surface,3,4,13,col);
		drawHLine(surface,4,5,12,col);
		drawHLine(surface,5,6,11,col);
		drawHLine(surface,6,7,10,col);
		drawHLine(surface,7,8,9,col);
		drawHLine(surface,8,9,8,col);
	} else {
		// up arrow
		drawHLine(surface,8,5,8,col);
		drawHLine(surface,7,6,9,col);
		drawHLine(surface,6,7,10,col);
		drawHLine(surface,5,8,11,col);
		drawHLine(surface,4,9,12,col);
		drawHLine(surface,3,10,13,col);
	}

	return surface;
}




Uint32 DuneStyle::getListBoxEntryHeight() {
	return 16;
}

SDL_Surface* DuneStyle::createListBoxEntry(Uint32 width, std::string text, bool selected, Uint32 color) {
    if(color == COLOR_DEFAULT) {
        color = defaultForegroundColor;
    }

	// create surfaces
	SDL_Surface* surface;
    if((surface = SDL_CreateRGBSurface(0, width, getListBoxEntryHeight(), SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)) == NULL) {
        return NULL;
    }

	if(selected == true) {
		SDL_FillRect(surface, NULL, buttonBackgroundColor);
	} else {
        SDL_FillRect(surface, NULL, COLOR_TRANSPARENT);
	}

	SDL_Surface* textSurface;
	textSurface = createSurfaceWithText(text.c_str(), color, FONT_STD10);
	SDL_Rect textRect = calcDrawingRect(textSurface, 3, surface->h/2 + 1, HAlign::Left, VAlign::Center);
	SDL_BlitSurface(textSurface,NULL,surface,&textRect);
	SDL_FreeSurface(textSurface);

	return surface;
}




SDL_Surface* DuneStyle::createProgressBarOverlay(Uint32 width, Uint32 height, double percent, Uint32 color) {

	// create surfaces
	SDL_Surface* pSurface;
    if((pSurface = SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)) == NULL) {
        return NULL;
    }

	SDL_FillRect(pSurface, NULL, COLOR_TRANSPARENT);

	if(color == COLOR_DEFAULT) {
		// default color

        int max_i = std::max( (int) lround(percent*(( ((int) width) - 4)/100.0)), 0);

		if (!SDL_MUSTLOCK(pSurface) || (SDL_LockSurface(pSurface) == 0)) {
			SDL_Rect dest = { 2, 2, max_i, ((int)height)-4 };
			SDL_FillRect(pSurface, &dest, COLOR_HALF_TRANSPARENT);


			if (SDL_MUSTLOCK(pSurface))
				SDL_UnlockSurface(pSurface);
		}
	} else {
	    int max_i = lround(percent*(width/100.0));

		SDL_Rect dest = { 0, 0, max_i, (int)height };
        SDL_FillRect(pSurface, &dest, color);
	}

	return pSurface;
}



SDL_Surface* DuneStyle::createToolTip(std::string text) {
	SDL_Surface* surface;
	SDL_Surface* helpTextSurface;

	if((helpTextSurface = createSurfaceWithText(text.c_str(), COLOR_YELLOW, FONT_STD10)) == NULL) {
		return NULL;
	}

	// create surfaces
    if((surface = SDL_CreateRGBSurface(0, helpTextSurface->w + 4, helpTextSurface->h + 2, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)) == NULL) {
        return NULL;
    }

	SDL_FillRect(surface, NULL, COLOR_HALF_TRANSPARENT);

	drawRect(surface, 0, 0, helpTextSurface->w + 4 - 1, helpTextSurface->h + 2 - 1, COLOR_YELLOW);

	SDL_Rect textRect = calcDrawingRect(helpTextSurface, 3, 3);
	SDL_BlitSurface(helpTextSurface, NULL, surface, &textRect);

	SDL_FreeSurface(helpTextSurface);
	return surface;
}



SDL_Surface* DuneStyle::createBackground(Uint32 width, Uint32 height) {
	SDL_Surface* pSurface;
	if(pGFXManager != NULL) {
		pSurface = getSubPicture(pGFXManager->getBackgroundSurface(), 0, 0, width, height);
		if(pSurface == NULL) {
			return NULL;
		}
	} else {
		// data manager not yet loaded
		if((pSurface = SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)) == NULL) {
			return NULL;
		}
		SDL_FillRect(pSurface, NULL, buttonBackgroundColor);
	}


	drawRect(pSurface, 0, 0, pSurface->w-1, pSurface->h-1, buttonBorderColor);
	drawHLine(pSurface, 1, 1, pSurface->w-2, buttonEdgeTopLeftColor);
	drawHLine(pSurface, 2, 2, pSurface->w-3, buttonEdgeTopLeftColor);
	drawVLine(pSurface, 1, 1, pSurface->h-2, buttonEdgeTopLeftColor);
	drawVLine(pSurface, 2, 2, pSurface->h-3, buttonEdgeTopLeftColor);
	drawHLine(pSurface, 1, pSurface->h-2, pSurface->w-2, buttonEdgeBottomRightColor);
	drawHLine(pSurface, 2, pSurface->h-3, pSurface->w-3, buttonEdgeBottomRightColor);
	drawVLine(pSurface, pSurface->w-2, 1, pSurface->h-2, buttonEdgeBottomRightColor);
	drawVLine(pSurface, pSurface->w-3, 2, pSurface->h-3, buttonEdgeBottomRightColor);

	return pSurface;
}

SDL_Surface* DuneStyle::createWidgetBackground(Uint32 width, Uint32 height) {
	SDL_Surface* surface;

	// create surfaces
    if((surface = SDL_CreateRGBSurface(0, width, height, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK)) == NULL) {
        return NULL;
    }

	SDL_FillRect(surface, NULL, pressedButtonBackgroundColor);
	drawRect(surface, 0, 0, surface->w-1, surface->h-1, buttonBorderColor);
	drawRect(surface, 1, 1, surface->w-2, surface->h-2, buttonEdgeBottomRightColor);

	return surface;
}
