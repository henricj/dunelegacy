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
#include <misc/draw_util.h>
#include <globals.h>

HoldPictureVideoEvent::HoldPictureVideoEvent(SDL_Surface* pSurface, int numFrames2Hold, bool bCenterVertical)
{
    if(pSurface == nullptr) {
        pTexture = nullptr;
    } else {
        sdl2::surface_ptr pTmp = convertSurfaceToDisplayFormat(Scaler::defaultDoubleSurface(pSurface).get());
        pTexture = sdl2::texture_ptr{ SDL_CreateTextureFromSurface(renderer, pTmp.get()) };
    }
    this->numFrames2Hold = numFrames2Hold;
    this->bCenterVertical = bCenterVertical;
    currentFrame = 0;
}

HoldPictureVideoEvent::~HoldPictureVideoEvent() = default;

int HoldPictureVideoEvent::draw()
{
    if(pTexture != nullptr) {
        SDL_Rect dest = calcAlignedDrawingRect(pTexture.get(), HAlign::Center, bCenterVertical ? VAlign::Center : VAlign::Top);
        SDL_RenderCopy(renderer, pTexture.get(), nullptr, &dest);
    }

    currentFrame++;

    return 100;
}

bool HoldPictureVideoEvent::isFinished()
{
    return (currentFrame >= numFrames2Hold);
}
