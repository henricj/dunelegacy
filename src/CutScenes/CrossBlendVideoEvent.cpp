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
#include <misc/Scaler.h>
#include <globals.h>

CrossBlendVideoEvent::CrossBlendVideoEvent(SDL_Surface* pSourceSurface, SDL_Surface* pDestSurface, bool bFreeSurfaces, bool bCenterVertical) : VideoEvent()
{
    this->pSourceSurface = convertSurfaceToDisplayFormat(Scaler::defaultDoubleTiledSurface(pSourceSurface, 1, 1, bFreeSurfaces), true);
    this->pDestSurface = convertSurfaceToDisplayFormat(Scaler::defaultDoubleTiledSurface(pDestSurface, 1, 1, bFreeSurfaces), true);
    this->bCenterVertical = bCenterVertical;
    currentFrame = 0;

    pStreamingTexture = SDL_CreateTexture(renderer, SCREEN_FORMAT, SDL_TEXTUREACCESS_STREAMING, this->pSourceSurface->w, this->pSourceSurface->h);

    SDL_Rect dest = {	0,0, getWidth(this->pSourceSurface), getHeight(this->pSourceSurface)};
    pBlendBlitter = new BlendBlitter(this->pDestSurface, this->pSourceSurface, dest, 30);
}

CrossBlendVideoEvent::~CrossBlendVideoEvent()
{
    SDL_FreeSurface(pSourceSurface);
    SDL_FreeSurface(pDestSurface);

    SDL_DestroyTexture(pStreamingTexture);

    delete pBlendBlitter;
    pBlendBlitter = NULL;
}

int CrossBlendVideoEvent::draw()
{
	if(pBlendBlitter->nextStep() == 0) {
		delete pBlendBlitter;
		pBlendBlitter = NULL;
    }

    SDL_UpdateTexture(pStreamingTexture, NULL, pSourceSurface->pixels, pSourceSurface->pitch);

    SDL_Rect dest = calcAlignedDrawingRect(pStreamingTexture, HAlign::Center, bCenterVertical ? VAlign::Center : VAlign::Top);

    SDL_RenderCopy(renderer, pStreamingTexture, NULL, &dest);

    currentFrame++;

    return 100;
}

bool CrossBlendVideoEvent::isFinished()
{
    return (pBlendBlitter == NULL);
}
