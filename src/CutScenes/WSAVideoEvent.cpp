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

#include <CutScenes/WSAVideoEvent.h>

#include <misc/Scaler.h>

WSAVideoEvent::WSAVideoEvent(Wsafile* pWsafile, bool bCenterVertical) : VideoEvent()
{
    this->pWsafile = pWsafile;
    this->bCenterVertical = bCenterVertical;
    currentFrame = 0;
}

WSAVideoEvent::~WSAVideoEvent()
{
}

int WSAVideoEvent::draw(SDL_Surface* pScreen)
{
    SDL_Surface* pSurface = Scaler::defaultDoubleSurface(pWsafile->getPicture(currentFrame), true);

    SDL_Rect dest = {   static_cast<Sint16>((pScreen->w - pSurface->w) / 2),
                        static_cast<Sint16>(bCenterVertical ? (pScreen->h - pSurface->h) / 2 : 0),
                        static_cast<Uint16>(pSurface->w),
                        static_cast<Uint16>(pSurface->h) };
	SDL_BlitSurface(pSurface,NULL,pScreen,&dest);

	SDL_FreeSurface(pSurface);

	currentFrame++;

	//return (int) (1000.0/pWsafile->getFps());
	return 170;
}

bool WSAVideoEvent::isFinished()
{
    return (currentFrame >= pWsafile->getNumFrames());
}
