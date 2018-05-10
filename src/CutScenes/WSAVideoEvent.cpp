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
#include <misc/draw_util.h>
#include <globals.h>

WSAVideoEvent::WSAVideoEvent(Wsafile* pWsafile, bool bCenterVertical)
{
    this->pWsafile = pWsafile;
    this->bCenterVertical = bCenterVertical;
    currentFrame = 0;
    pStreamingTexture = sdl2::texture_ptr{ SDL_CreateTexture(renderer, SCREEN_FORMAT, SDL_TEXTUREACCESS_STREAMING, 2 * pWsafile->getWidth(), 2 * pWsafile->getHeight()) };
}

WSAVideoEvent::~WSAVideoEvent() = default;

int WSAVideoEvent::draw()
{
    sdl2::surface_ptr pSurface = convertSurfaceToDisplayFormat(Scaler::defaultDoubleSurface(pWsafile->getPicture(currentFrame).get()).get());

    SDL_UpdateTexture(pStreamingTexture.get(), nullptr, pSurface->pixels, pSurface->pitch);

    SDL_Rect dest = calcAlignedDrawingRect(pStreamingTexture.get(), HAlign::Center, bCenterVertical ? VAlign::Center : VAlign::Top);

    SDL_RenderCopy(renderer, pStreamingTexture.get(), nullptr, &dest);

    currentFrame++;

    //return (int) (1000.0/pWsafile->getFps());
    return 170;
}

bool WSAVideoEvent::isFinished()
{
    return (currentFrame >= pWsafile->getNumFrames());
}
