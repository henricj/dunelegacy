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
#include <misc/draw_util.h>
#include <Colors.h>
#include <globals.h>

#include <algorithm>

FadeOutVideoEvent::FadeOutVideoEvent(SDL_Surface* pSurface, int numFrames2FadeOut, bool bCenterVertical, bool bFadeWhite) : VideoEvent()
{
    sdl2::surface_ptr pTmp = convertSurfaceToDisplayFormat(Scaler::defaultDoubleSurface(pSurface).get());
    pTexture = sdl2::texture_ptr{ SDL_CreateTextureFromSurface(renderer, pTmp.get()) };

    SDL_SetTextureBlendMode(pTexture.get(), SDL_BLENDMODE_BLEND);

    this->numFrames2FadeOut = numFrames2FadeOut;
    this->bCenterVertical = bCenterVertical;
    this->bFadeWhite = bFadeWhite;
    currentFrame = 0;
}

FadeOutVideoEvent::~FadeOutVideoEvent() = default;

int FadeOutVideoEvent::draw()
{
    SDL_Rect dest = calcAlignedDrawingRect(pTexture.get(), HAlign::Center, bCenterVertical ? VAlign::Center : VAlign::Top);

    int alpha = std::max(0, 255 - (255*currentFrame)/numFrames2FadeOut);
    if(bFadeWhite) {
        // fade to white
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &dest);
    }
    SDL_SetTextureAlphaMod(pTexture.get(), alpha);
    SDL_RenderCopy(renderer, pTexture.get(), nullptr, &dest);

    currentFrame++;

    return 100;
}

bool FadeOutVideoEvent::isFinished()
{
    return (currentFrame >= numFrames2FadeOut);
}
