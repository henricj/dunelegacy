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

#include <CutScenes/TextEvent.h>
#include <FileClasses/FontManager.h>

extern FontManager* pFontManager;

TextEvent::TextEvent(std::string text, Uint32 color, int startFrame, int lengthInFrames, bool bFadeIn, bool bFadeOut, bool bCenterVertical)
{
    this->text = text;
    this->startFrame = startFrame;
    this->lengthInFrames = lengthInFrames;
    this->bFadeIn = bFadeIn;
    this->bFadeOut = bFadeOut;
    this->bCenterVertical = bCenterVertical;
    this->color = color;
    pSurface = pFontManager->createSurfaceWithMultilineText(text, color, FONT_STD24, true);
}

TextEvent::~TextEvent()
{
    SDL_FreeSurface(pSurface);
}

void TextEvent::draw(SDL_Surface* pScreen, int currentFrameNumber)
{
    if(currentFrameNumber < startFrame || currentFrameNumber > startFrame + lengthInFrames) {
        return;
    }

    int alpha = 0;
    if((bFadeIn == false) && (currentFrameNumber == startFrame)) {
        alpha = 255;
    } else if(bFadeIn && (currentFrameNumber - startFrame <= TEXT_FADE_TIME)) {
        alpha = ((currentFrameNumber - startFrame)*255)/TEXT_FADE_TIME;
    } else if (bFadeOut && ((startFrame + lengthInFrames) - currentFrameNumber <= TEXT_FADE_TIME)) {
        alpha = (((startFrame + lengthInFrames) - currentFrameNumber)*255)/TEXT_FADE_TIME;
    }

    SDL_Rect dest = {   static_cast<Sint16>((pScreen->w - pSurface->w) / 2),
                        static_cast<Sint16>(bCenterVertical ? (pScreen->h - pSurface->h) / 2 : (pScreen->h/2 + 480/2 - 5*pFontManager->getTextHeight(FONT_STD24)/2)),
                        static_cast<Uint16>(pSurface->w),
                        static_cast<Uint16>(pSurface->h) };
    SDL_SetSurfaceAlphaMod(pSurface, alpha);
    SDL_BlitSurface(pSurface,NULL,pScreen,&dest);
}
