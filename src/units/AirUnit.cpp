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

#include <units/AirUnit.h>

#include <globals.h>

#include <House.h>
#include <Game.h>
#include <Explosion.h>
#include <SoundPlayer.h>
#include <Map.h>
#include <ScreenBorder.h>

#include <FileClasses/GFXManager.h>

#include <misc/draw_util.h>

#include <structures/RepairYard.h>

AirUnit::AirUnit(House* newOwner) : UnitBase(newOwner)
{
    AirUnit::init();
}

AirUnit::AirUnit(InputStream& stream) : UnitBase(stream)
{
    AirUnit::init();
}

void AirUnit::init()
{
    shadowGraphic = NULL;
    aFlyingUnit = true;
}

AirUnit::~AirUnit()
{
}

void AirUnit::save(OutputStream& stream) const
{
	UnitBase::save(stream);
}

void AirUnit::destroy()
{
    if(isVisible()) {
        Coord position(lround(realX), lround(realY));
        currentGame->getExplosionList().push_back(new Explosion(Explosion_Medium2, position, owner->getHouseID()));

        if(isVisible(getOwner()->getTeam()))
            soundPlayer->playSoundAt(Sound_ExplosionMedium,location);
    }

    UnitBase::destroy();
}

void AirUnit::assignToMap(const Coord& pos)
{
	if(currentGameMap->tileExists(pos)) {
		currentGameMap->getTile(pos)->assignAirUnit(getObjectID());
//		currentGameMap->viewMap(owner->getTeam(), location, getViewRange());
	}
}

void AirUnit::checkPos()
{
}

void AirUnit::blitToScreen()
{
    int imageW = graphic[currentZoomlevel]->w/numImagesX;
    int imageH = graphic[currentZoomlevel]->h/numImagesY;

    if(screenborder->isInsideScreen(Coord(lround(realX + 4), lround(realY + 12)),Coord(imageW, imageH)) == true) {
		// Not out of screen

        SDL_Rect dest = { screenborder->world2screenX(realX + 4) - imageW/2 + 1, screenborder->world2screenY(realY + 12) - imageH/2, imageW, imageH };
        SDL_Rect source = { drawnAngle*imageW, drawnFrame*imageH, imageW, imageH };

        if(shadowGraphic != NULL) {
            SDL_Rect mini = {0, 0, 1, 1};
            SDL_Surface* transparentSurf = pGFXManager->getTransparent150Surface();

            SDL_LockSurface(shadowGraphic[currentZoomlevel]);
            for(int i=0;i<imageW; i++) {
                for(int j=0;j<imageH; j++) {
                    if(getPixel(shadowGraphic[currentZoomlevel],source.x+i,source.y+j) == 12) {
                        SDL_Rect drawLoc = {dest.x + i, dest.y + j, 1, 1};
                        SDL_BlitSurface(transparentSurf,&mini,screen,&drawLoc);
                    };
                }
            }
            SDL_UnlockSurface(shadowGraphic[currentZoomlevel]);
        }
	}

	UnitBase::blitToScreen();
}

bool AirUnit::canPass(int xPos, int yPos) const
{
	return currentGameMap->tileExists(xPos, yPos);
}
