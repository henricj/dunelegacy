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
#include <Colors.h>

FadeOutVideoEvent::FadeOutVideoEvent(SDL_Surface* pSurface, int numFrames2FadeOut, bool bFreeSurface, bool bCenterVertical, bool bFadeWhite) : VideoEvent()
{
    SDL_Surface* tmp = Scaler::defaultDoubleSurface(pSurface, bFreeSurface);
    this->pSurface = SDL_ConvertSurfaceFormat(tmp, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_SetSurfaceBlendMode(this->pSurface, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(tmp);

    this->numFrames2FadeOut = numFrames2FadeOut;
    this->bFreeSurface = bFreeSurface;
    this->bCenterVertical = bCenterVertical;
    this->bFadeWhite = bFadeWhite;
    currentFrame = 0;
}

FadeOutVideoEvent::~FadeOutVideoEvent()
{
    SDL_FreeSurface(pSurface);
}

int FadeOutVideoEvent::draw(SDL_Surface* pScreen)
{
    SDL_Rect dest = {   static_cast<Sint16>((pScreen->w - pSurface->w) / 2),
                        static_cast<Sint16>(bCenterVertical ? (pScreen->h - pSurface->h) / 2 : 0),
                        static_cast<Uint16>(pSurface->w),
                        static_cast<Uint16>(pSurface->h) };
    int alpha  = std::max(0, 255 - (255*currentFrame)/numFrames2FadeOut);
    if(bFadeWhite) {
        // fade to white
        SDL_FillRect(pScreen, &dest, COLOR_WHITE);
    }
    SDL_SetSurfaceAlphaMod(pSurface, alpha);
    SDL_BlitSurface(pSurface,NULL,pScreen,&dest);

    currentFrame++;

    return 100;
}

bool FadeOutVideoEvent::isFinished()
{
    return (currentFrame >= numFrames2FadeOut);
}
