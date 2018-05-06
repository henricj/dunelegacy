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

#include <CutScenes/CrossBlendVideoEvent.h>
#include <misc/SDL2pp.h>
#include <misc/Scaler.h>
#include <globals.h>

CrossBlendVideoEvent::CrossBlendVideoEvent(SDL_Surface* pSourceSurface, SDL_Surface* pDestSurface, bool bFreeSurfaces, bool bCenterVertical)
{
    this->pSourceSurface = convertSurfaceToDisplayFormat(Scaler::defaultDoubleTiledSurface(pSourceSurface, 1, 1, bFreeSurfaces).release(), true);
    this->pDestSurface = convertSurfaceToDisplayFormat(Scaler::defaultDoubleTiledSurface(pDestSurface, 1, 1, bFreeSurfaces).release(), true);
    this->bCenterVertical = bCenterVertical;
    currentFrame = 0;

    pStreamingTexture = sdl2::texture_ptr{ SDL_CreateTexture(renderer, SCREEN_FORMAT, SDL_TEXTUREACCESS_STREAMING, this->pSourceSurface->w, this->pSourceSurface->h) };

    const SDL_Rect dest = { 0,0, getWidth(this->pSourceSurface.get()), getHeight(this->pSourceSurface.get()) };
    pBlendBlitter = std::make_unique<BlendBlitter>(this->pDestSurface.get(), false, this->pSourceSurface.get(), dest, 30);
}

CrossBlendVideoEvent::~CrossBlendVideoEvent() = default;

int CrossBlendVideoEvent::draw()
{
    if(pBlendBlitter->nextStep() == 0) {
        pBlendBlitter.reset();
    }

    SDL_UpdateTexture(pStreamingTexture.get(), nullptr, pSourceSurface->pixels, pSourceSurface->pitch);

    auto dest = calcAlignedDrawingRect(pStreamingTexture.get(), HAlign::Center, bCenterVertical ? VAlign::Center : VAlign::Top);

    SDL_RenderCopy(renderer, pStreamingTexture.get(), nullptr, &dest);

    currentFrame++;

    return 100;
}

bool CrossBlendVideoEvent::isFinished()
{
    return (pBlendBlitter == nullptr);
}
