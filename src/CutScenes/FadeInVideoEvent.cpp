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

#include "misc/DrawingRectHelper.h"
#include <globals.h>
#include <misc/Scaler.h>
#include <misc/draw_util.h>

#include <algorithm>

FadeInVideoEvent::FadeInVideoEvent(SDL_Surface* pSurface, int numFrames2FadeIn, bool bCenterVertical, bool bFadeWhite)
    : numFrames2FadeIn(numFrames2FadeIn), bCenterVertical(bCenterVertical), bFadeWhite(bFadeWhite) {
    const auto pTmp = convertSurfaceToDisplayFormat(Scaler::defaultDoubleSurface(pSurface).get());
    pTexture        = sdl2::texture_ptr{SDL_CreateTextureFromSurface(dune::globals::renderer.get(), pTmp.get())};

    SDL_SetTextureBlendMode(pTexture.get(), SDL_BLENDMODE_BLEND);
}

FadeInVideoEvent::~FadeInVideoEvent() = default;

int FadeInVideoEvent::draw() {
    auto* const renderer = dune::globals::renderer.get();

    const auto dest =
        calcAlignedDrawingRect(pTexture.get(), HAlign::Center, bCenterVertical ? VAlign::Center : VAlign::Top);

    const auto alpha = static_cast<uint8_t>(std::min(255, 255 * currentFrame / numFrames2FadeIn));
    if (bFadeWhite) {
        // fade from white
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &dest);
    }
    SDL_SetTextureAlphaMod(pTexture.get(), alpha);
    Dune_RenderCopy(renderer, pTexture.get(), nullptr, &dest);

    currentFrame++;

    return 100;
}

bool FadeInVideoEvent::isFinished() {
    return currentFrame >= numFrames2FadeIn;
}
