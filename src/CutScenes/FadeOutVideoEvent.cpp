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

#include <CutScenes/FadeOutVideoEvent.h>
#include <misc/Scaler.h>

FadeOutVideoEvent::FadeOutVideoEvent(SDL_Surface* pSurface, int numFrames2FadeOut, bool bFreeSurface, bool bCenterVertical, bool bFadeWhite) : VideoEvent()
{
    this->pSurface = Scaler::defaultDoubleSurface(pSurface, bFreeSurface);
    this->numFrames2FadeOut = numFrames2FadeOut;
    this->bFreeSurface = bFreeSurface;
    this->bCenterVertical = bCenterVertical;
    this->bFadeWhite = bFadeWhite;
    currentFrame = 0;
    pOldScreen = NULL;
}

FadeOutVideoEvent::~FadeOutVideoEvent()
{
    SDL_FreeSurface(pSurface);

    // restore old palette
    if(((currentFrame < numFrames2FadeOut) || (bFadeWhite == false)) && pOldScreen != NULL) {

        // Fixes some flickering
        SDL_FillRect(pOldScreen, NULL, 0);
        SDL_Flip(pOldScreen);

        oldPalette.applyToSurface(pOldScreen, SDL_PHYSPAL);
    }
}

int FadeOutVideoEvent::draw(SDL_Surface* pScreen)
{
    SDL_Rect dest = {   (pScreen->w - pSurface->w) / 2,
                        bCenterVertical ? (pScreen->h - pSurface->h) / 2 : 0,
                        pSurface->w,
                        pSurface->h };
    SDL_BlitSurface(pSurface,NULL,pScreen,&dest);

    currentFrame++;

    return 100;
}

void FadeOutVideoEvent::setupPalette(SDL_Surface* pScreen)
{
    if(pOldScreen == NULL) {
        // backup old colors
        oldPalette = Palette(pScreen->format->palette);
        pOldScreen = pScreen;
    }

    Palette newPalette = oldPalette;

    if(bFadeWhite == false) {
        // fade to black (leave out index 0 and pPalette->ncolors-1)
        for(int i=1; i < newPalette.getNumColors() - 1; i++) {
            newPalette[i].r = newPalette[i].r - (newPalette[i].r*currentFrame)/numFrames2FadeOut;
            newPalette[i].g = newPalette[i].g - (newPalette[i].g*currentFrame)/numFrames2FadeOut;
            newPalette[i].b = newPalette[i].b - (newPalette[i].b*currentFrame)/numFrames2FadeOut;
        }
    } else {
        // fade to white (leave out index 0 and pPalette->ncolors-1)
        for(int i=1; i < newPalette.getNumColors() - 1; i++) {
            newPalette[i].r = newPalette[i].r + ((255-newPalette[i].r)*currentFrame)/numFrames2FadeOut;
            newPalette[i].g = newPalette[i].g + ((255-newPalette[i].g)*currentFrame)/numFrames2FadeOut;
            newPalette[i].b = newPalette[i].b + ((255-newPalette[i].b)*currentFrame)/numFrames2FadeOut;
        }
    }

    newPalette.applyToSurface(pScreen, SDL_PHYSPAL, 1, newPalette.getNumColors() - 2);
}

bool FadeOutVideoEvent::isFinished()
{
    return (currentFrame >= numFrames2FadeOut);
}
