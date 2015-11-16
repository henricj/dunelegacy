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

#include <CutScenes/FadeInVideoEvent.h>
#include <misc/Scaler.h>

FadeInVideoEvent::FadeInVideoEvent(SDL_Surface* pSurface, int numFrames2FadeIn, bool bFreeSurface, bool bCenterVertical, bool bFadeWhite) : VideoEvent()
{
    this->pSurface = Scaler::defaultDoubleSurface(pSurface, bFreeSurface);
    this->numFrames2FadeIn = numFrames2FadeIn;
    this->bFreeSurface = bFreeSurface;
    this->bCenterVertical = bCenterVertical;
    this->bFadeWhite = bFadeWhite;
    currentFrame = 0;
    pOldScreen = NULL;
}

FadeInVideoEvent::~FadeInVideoEvent()
{
    SDL_FreeSurface(pSurface);

    // restore old palette if we abort before finish
    if((currentFrame < numFrames2FadeIn) && (pOldScreen != NULL)) {
        oldPalette.applyToSurface(pOldScreen, SDL_PHYSPAL);
    }
}

int FadeInVideoEvent::draw(SDL_Surface* pScreen)
{
    SDL_Rect dest = {   static_cast<Sint16>((pScreen->w - pSurface->w) / 2),
                        static_cast<Sint16>(bCenterVertical ? (pScreen->h - pSurface->h) / 2 : 0),
                        static_cast<Uint16>(pSurface->w),
                        static_cast<Uint16>(pSurface->h) };
    SDL_BlitSurface(pSurface,NULL,pScreen,&dest);

    currentFrame++;

    return 100;
}

void FadeInVideoEvent::setupPalette(SDL_Surface* pScreen)
{
    if(pOldScreen == NULL) {
        // backup old palette
        oldPalette = Palette(pScreen->format->palette);
        pOldScreen = pScreen;
    }

    Palette newPalette = oldPalette;

    if(bFadeWhite == false) {
        // fade from black (leave out index 0 and pPalette->ncolors-1)
        for(int i=1; i < newPalette.getNumColors() - 1; i++) {
            newPalette[i].r = (newPalette[i].r*currentFrame)/numFrames2FadeIn;
            newPalette[i].g = (newPalette[i].g*currentFrame)/numFrames2FadeIn;
            newPalette[i].b = (newPalette[i].b*currentFrame)/numFrames2FadeIn;
        }
    } else {
        // fade from white (leave out index 0 and pPalette->ncolors-1)
        for(int i=1; i < newPalette.getNumColors() - 1; i++) {
            newPalette[i].r = ((255*(numFrames2FadeIn-currentFrame))/numFrames2FadeIn) + ((newPalette[i].r*currentFrame)/numFrames2FadeIn);
            newPalette[i].g = ((255*(numFrames2FadeIn-currentFrame))/numFrames2FadeIn) + ((newPalette[i].g*currentFrame)/numFrames2FadeIn);
            newPalette[i].b = ((255*(numFrames2FadeIn-currentFrame))/numFrames2FadeIn) + ((newPalette[i].b*currentFrame)/numFrames2FadeIn);
        }
    }

    newPalette.applyToSurface(pScreen, SDL_PHYSPAL, 1, newPalette.getNumColors() - 2);
}

bool FadeInVideoEvent::isFinished()
{
    return (currentFrame >= numFrames2FadeIn);
}
