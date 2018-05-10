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

CrossBlendVideoEvent::CrossBlendVideoEvent(SDL_Surface* pStartSurface, SDL_Surface* pEndSurface, bool bCenterVertical)
{
    pBlendBlitterTargetSurface = convertSurfaceToDisplayFormat(Scaler::defaultDoubleTiledSurface(pStartSurface, 1, 1).get());
    this->bCenterVertical = bCenterVertical;
    currentFrame = 0;

    pStreamingTexture = sdl2::texture_ptr{ SDL_CreateTexture(renderer, SCREEN_FORMAT, SDL_TEXTUREACCESS_STREAMING, pBlendBlitterTargetSurface->w, pBlendBlitterTargetSurface->h) };

    const SDL_Rect dest = { 0,0, getWidth(pBlendBlitterTargetSurface.get()), getHeight(pBlendBlitterTargetSurface.get()) };
    auto pBlendBlitterSourceSurface = convertSurfaceToDisplayFormat(Scaler::defaultDoubleTiledSurface(pEndSurface, 1, 1).get());
    pBlendBlitter = std::make_unique<BlendBlitter>(std::move(pBlendBlitterSourceSurface), pBlendBlitterTargetSurface.get(), dest, 30);
}

CrossBlendVideoEvent::~CrossBlendVideoEvent() = default;

int CrossBlendVideoEvent::draw()
{
    if(pBlendBlitter->nextStep() == 0) {
        pBlendBlitter.reset();
    }

    SDL_UpdateTexture(pStreamingTexture.get(), nullptr, pBlendBlitterTargetSurface->pixels, pBlendBlitterTargetSurface->pitch);

    auto dest = calcAlignedDrawingRect(pStreamingTexture.get(), HAlign::Center, bCenterVertical ? VAlign::Center : VAlign::Top);

    SDL_RenderCopy(renderer, pStreamingTexture.get(), nullptr, &dest);

    currentFrame++;

    return 100;
}

bool CrossBlendVideoEvent::isFinished()
{
    return (pBlendBlitter == nullptr);
}
