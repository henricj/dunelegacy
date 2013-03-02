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

TextEvent::TextEvent(std::string text, int startFrame, int lengthInFrames, bool bFadeIn, bool bFadeOut, bool bCenterVertical, unsigned char color)
{
    this->text = text;
    this->startFrame = startFrame;
    this->lengthInFrames = lengthInFrames;
    this->bFadeIn = bFadeIn;
    this->bFadeOut = bFadeOut;
    this->bCenterVertical = bCenterVertical;
    this->color = color;
    pSurface = pFontManager->createSurfaceWithMultilineText(text, 255, FONT_STD24, true);
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

    SDL_Rect dest = {   (pScreen->w - pSurface->w) / 2,
                        bCenterVertical ? (pScreen->h - pSurface->h) / 2 : (pScreen->h/2 + 480/2 - 5*pFontManager->getTextHeight(FONT_STD24)/2),
                        pSurface->w,
                        pSurface->h };
    SDL_BlitSurface(pSurface,NULL,pScreen,&dest);
}

void TextEvent::setupPalette(SDL_Surface* pScreen, int currentFrameNumber) {
    if(currentFrameNumber < startFrame || currentFrameNumber > startFrame + lengthInFrames) {
        return;
    }

    if((bFadeIn == false) && (currentFrameNumber == startFrame)) {
        SDL_SetPalette(pScreen, SDL_PHYSPAL, &pScreen->format->palette->colors[color], pScreen->format->palette->ncolors-1,1);
    } else if(bFadeIn && (currentFrameNumber - startFrame <= TEXT_FADE_TIME)) {
        SDL_Color newColor = pScreen->format->palette->colors[color];

        newColor.r = ((currentFrameNumber - startFrame)*newColor.r)/TEXT_FADE_TIME;
        newColor.g = ((currentFrameNumber - startFrame)*newColor.g)/TEXT_FADE_TIME;
        newColor.b = ((currentFrameNumber - startFrame)*newColor.b)/TEXT_FADE_TIME;

        SDL_SetPalette(pScreen, SDL_PHYSPAL, &newColor, pScreen->format->palette->ncolors-1,1);
    } else if (bFadeOut && ((startFrame + lengthInFrames) - currentFrameNumber <= TEXT_FADE_TIME)) {
        SDL_Color newColor = pScreen->format->palette->colors[color];

        newColor.r = (((startFrame + lengthInFrames) - currentFrameNumber)*newColor.r)/TEXT_FADE_TIME;
        newColor.g = (((startFrame + lengthInFrames) - currentFrameNumber)*newColor.g)/TEXT_FADE_TIME;
        newColor.b = (((startFrame + lengthInFrames) - currentFrameNumber)*newColor.b)/TEXT_FADE_TIME;

        SDL_SetPalette(pScreen, SDL_PHYSPAL, &newColor, pScreen->format->palette->ncolors-1,1);
    }
}
