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

#include <CutScenes/HoldPictureVideoEvent.h>
#include <misc/Scaler.h>

HoldPictureVideoEvent::HoldPictureVideoEvent(SDL_Surface* pSurface, int numFrames2Hold, bool bFreeSurface, bool bCenterVertical) : VideoEvent()
{
    if(pSurface == NULL) {
        this->pSurface = NULL;
    } else {
        this->pSurface = Scaler::defaultDoubleSurface(pSurface, bFreeSurface);
    }
    this->numFrames2Hold = numFrames2Hold;
    this->bFreeSurface = bFreeSurface;
    this->bCenterVertical = bCenterVertical;
    currentFrame = 0;
}

HoldPictureVideoEvent::~HoldPictureVideoEvent()
{
    if(pSurface != NULL) {
        SDL_FreeSurface(pSurface);
    }
}

int HoldPictureVideoEvent::draw(SDL_Surface* pScreen)
{
    if(pSurface != NULL) {
        SDL_Rect dest = {   static_cast<Sint16>((pScreen->w - pSurface->w) / 2),
                            static_cast<Sint16>(bCenterVertical ? (pScreen->h - pSurface->h) / 2 : 0),
                            static_cast<Uint16>(pSurface->w),
                            static_cast<Uint16>(pSurface->h) };
        SDL_BlitSurface(pSurface,NULL,pScreen,&dest);
    }

    currentFrame++;

    return 100;
}

bool HoldPictureVideoEvent::isFinished()
{
    return (currentFrame >= numFrames2Hold);
}
