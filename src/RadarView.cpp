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

#include <RadarView.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>

#include <Game.h>
#include <Map.h>
#include <House.h>
#include <ScreenBorder.h>
#include <Tile.h>
#include <SoundPlayer.h>

#include <globals.h>

#include <misc/draw_util.h>


RadarView::RadarView()
 : RadarViewBase(), currentRadarMode(Mode_RadarOff), animFrame(NUM_STATIC_FRAMES - 1), animCounter(NUM_STATIC_FRAME_TIME)
{
    radarStaticAnimation = pGFXManager->getUIGraphic(UI_RadarAnimation);

    radarSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 128, 128,8,0,0,0,0);
	if(radarSurface == NULL) {
		throw std::runtime_error("RadarView::RadarView(): Cannot create new Picture!");
	}
	palette.applyToSurface(radarSurface);
}


RadarView::~RadarView()
{
    SDL_FreeSurface(radarSurface);
}

int RadarView::getMapSizeX() const {
    return currentGameMap->getSizeX();
}

int RadarView::getMapSizeY() const {
    return currentGameMap->getSizeY();
}


void RadarView::draw(SDL_Surface* screen, Point position)
{
    SDL_Rect radarPosition = { position.x + RADARVIEW_BORDERTHICKNESS, position.y + RADARVIEW_BORDERTHICKNESS, RADARWIDTH, RADARHEIGHT};

    switch(currentRadarMode) {
        case Mode_RadarOff:
        case Mode_RadarOn: {
            int mapSizeX = currentGameMap->getSizeX();
            int mapSizeY = currentGameMap->getSizeY();

            int scale = 1;
            int offsetX = 0;
            int offsetY = 0;

            calculateScaleAndOffsets(mapSizeX, mapSizeY, scale, offsetX, offsetY);

            updateRadarSurface(mapSizeX, mapSizeY, scale, offsetX, offsetY);

            SDL_Rect dest = { radarPosition.x, radarPosition.y, radarSurface->w, radarSurface->h };
            SDL_BlitSurface(radarSurface, NULL, screen, &dest);

            SDL_Rect RadarRect;
            RadarRect.x = (screenborder->getLeft() * mapSizeX*scale) / (mapSizeX*TILESIZE) + offsetX;
            RadarRect.y = (screenborder->getTop() * mapSizeY*scale) / (mapSizeY*TILESIZE) + offsetY;
            RadarRect.w = ((screenborder->getRight() - screenborder->getLeft()) * mapSizeX*scale) / (mapSizeX*TILESIZE);
            RadarRect.h = ((screenborder->getBottom() - screenborder->getTop()) * mapSizeY*scale) / (mapSizeY*TILESIZE);

            if(RadarRect.x < offsetX) {
                RadarRect.w -= RadarRect.x;
                RadarRect.x = offsetX;
            }

            if(RadarRect.y < offsetY) {
                RadarRect.h -= RadarRect.y;
                RadarRect.y = offsetY;
            }

            int offsetFromRightX = 128 - mapSizeX*scale - offsetX;
            if(RadarRect.x + RadarRect.w > radarPosition.w - offsetFromRightX) {
                RadarRect.w  = radarPosition.w - offsetFromRightX - RadarRect.x - 1;
            }

            int offsetFromBottomY = 128 - mapSizeY*scale - offsetY;
            if(RadarRect.y + RadarRect.h > radarPosition.h - offsetFromBottomY) {
                RadarRect.h = radarPosition.h - offsetFromBottomY - RadarRect.y - 1;
            }

            drawRect(   screen,
                        radarPosition.x + RadarRect.x,
                        radarPosition.y + RadarRect.y,
                        radarPosition.x + (RadarRect.x + RadarRect.w),
                        radarPosition.y + (RadarRect.y + RadarRect.h),
                        COLOR_WHITE);

        } break;

        case Mode_AnimationRadarOff:
        case Mode_AnimationRadarOn: {
            int imageW = radarStaticAnimation->w / NUM_STATIC_FRAMES;
            int imageH = radarStaticAnimation->h;
            SDL_Rect source = { animFrame*imageW, 0, imageW, imageH };
            SDL_Rect dest = { radarPosition.x, radarPosition.y, imageW, imageH };
            SDL_BlitSurface(radarStaticAnimation, &source, screen, &dest);
        } break;
    }
}

void RadarView::update() {
    if(pLocalHouse->hasRadarOn()) {
        if(currentRadarMode != Mode_RadarOn && currentRadarMode != Mode_AnimationRadarOn && currentRadarMode != Mode_AnimationRadarOff) {
            switchRadarMode(true);
        }
    } else {
        if(currentRadarMode != Mode_RadarOff && currentRadarMode != Mode_AnimationRadarOn && currentRadarMode != Mode_AnimationRadarOff) {
            switchRadarMode(false);
        }
    }

    switch(currentRadarMode) {
        case Mode_RadarOff: {

        } break;

        case Mode_RadarOn: {

        } break;

        case Mode_AnimationRadarOff: {
            if(animFrame >= NUM_STATIC_FRAMES-1) {
                currentRadarMode = Mode_RadarOff;
            } else {
                animCounter--;
                if(animCounter <= 0) {
                    animFrame++;
                    animCounter = NUM_STATIC_FRAME_TIME;
                }
            }
        } break;

        case Mode_AnimationRadarOn: {
            if(animFrame <= 0) {
                currentRadarMode = Mode_RadarOn;
            } else {
                animCounter--;
                if(animCounter <= 0) {
                    animFrame--;
                    animCounter = NUM_STATIC_FRAME_TIME;
                }
            }
        } break;
    }
}

void RadarView:: switchRadarMode(bool bOn) {
    soundPlayer->playSound(RadarNoise);

    if(bOn == true) {
        soundPlayer->playVoice(RadarActivated,pLocalHouse->getHouseID());
        currentRadarMode = Mode_AnimationRadarOn;
    } else {
        soundPlayer->playVoice(RadarDeactivated,pLocalHouse->getHouseID());
        currentRadarMode = Mode_AnimationRadarOff;
    }
}

void RadarView::updateRadarSurface(int mapSizeX, int mapSizeY, int scale, int offsetX, int offsetY) {

    // Lock radarSurface for direct access to the pixels
    if(!SDL_MUSTLOCK(radarSurface) || (SDL_LockSurface(radarSurface) == 0)) {
        for(int x = 0; x <  mapSizeX; x++) {
            for(int y = 0; y <  mapSizeY; y++) {

                Tile* pTile = currentGameMap->getTile(x,y);

                /* Selecting the right color is handled in Tile::getRadarColor() */
                Uint32 color = pTile->getRadarColor(pLocalHouse, pLocalHouse->hasRadarOn());

                for(int j = 0; j < scale; j++) {
                    Uint8* p = (Uint8 *) radarSurface->pixels + (offsetY + scale*y + j) * radarSurface->pitch + (offsetX + scale*x);

                    for(int i = 0; i < scale; i++, p++) {
                        // Do not use putPixel here to avoid overhead
                        *p = color;
                    }
                }
            }
        }

        if(SDL_MUSTLOCK(screen)) {
            SDL_UnlockSurface(screen);
        }
    }
}
