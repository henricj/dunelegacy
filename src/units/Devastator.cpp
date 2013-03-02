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

#include <units/Devastator.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>
#include <ScreenBorder.h>
#include <Explosion.h>
#include <Map.h>
#include <SoundPlayer.h>

#include <players/HumanPlayer.h>


Devastator::Devastator(House* newOwner) : TrackedUnit(newOwner)
{
    Devastator::init();

    setHealth(getMaxHealth());

    devastateTimer = 0;
}

Devastator::Devastator(InputStream& stream) : TrackedUnit(stream)
{
    Devastator::init();

	devastateTimer = stream.readSint32();
}

void Devastator::init()
{
    itemID = Unit_Devastator;
    owner->incrementUnits(itemID);

	numWeapons = 2;
	bulletType = Bullet_ShellLarge;

	graphicID = ObjPic_Devastator_Base;
	graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
	gunGraphicID = ObjPic_Devastator_Gun;
	turretGraphic = pGFXManager->getObjPic(gunGraphicID,getOwner()->getHouseID());

	numImagesX = NUM_ANGLES;
	numImagesY = 1;
}


Devastator::~Devastator()
{
}

void Devastator::save(OutputStream& stream) const
{
	TrackedUnit::save(stream);
	stream.writeSint32(devastateTimer);
}

void Devastator::blitToScreen()
{
    SDL_Surface* pUnitGraphic = graphic[currentZoomlevel];
    int imageW1 = pUnitGraphic->w/numImagesX;
    int x1 = screenborder->world2screenX(realX);
    int y1 = screenborder->world2screenY(realY);

    SDL_Rect source1 = { drawnAngle * imageW1, 0, imageW1, pUnitGraphic->h };
    SDL_Rect dest1 = { x1 - imageW1/2, y1 - pUnitGraphic->h/2, imageW1, pUnitGraphic->h };

    SDL_BlitSurface(pUnitGraphic, &source1, screen, &dest1);

    const Coord devastatorTurretOffset[] =  {
                                                Coord(8, -16),
                                                Coord(-4, -12),
                                                Coord(0, -16),
                                                Coord(4, -12),
                                                Coord(-8, -16),
                                                Coord(0, -12),
                                                Coord(-4, -12),
                                                Coord(0, -12)
                                            };

    SDL_Surface* pTurretGraphic = turretGraphic[currentZoomlevel];
    int imageW2 = pTurretGraphic->w/numImagesX;
    int x2 = screenborder->world2screenX(realX + devastatorTurretOffset[drawnAngle].x);
    int y2 = screenborder->world2screenY(realY + devastatorTurretOffset[drawnAngle].y);

    SDL_Rect source2 = { drawnAngle * imageW2, 0, imageW2, pTurretGraphic->h };
    SDL_Rect dest2 = { x2 - imageW2/2, y2 - pTurretGraphic->h/2, imageW2, pTurretGraphic->h };

    SDL_BlitSurface(pTurretGraphic, &source2, screen, &dest2);

    if(isBadlyDamaged()) {
        drawSmoke(x1, y1);
    }
}

void Devastator::handleStartDevastateClick() {
	currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_DEVASTATOR_STARTDEVASTATE,objectID));
}

void Devastator::doStartDevastate()
{
	if (devastateTimer <= 0)
		devastateTimer = 200;
}

void Devastator::destroy()
{
    if(currentGameMap->tileExists(location) && isVisible()) {
        for(int i = 0; i < 3; i++) {
            for(int j = 0; j < 3; j++) {
                Coord realPos(lround(realX) + (i - 1)*TILESIZE, lround(realY) + (j - 1)*TILESIZE);

                currentGameMap->damage(objectID, owner, realPos, itemID, 150, 16, false);

                Uint32 explosionID = currentGame->randomGen.getRandOf(2,Explosion_Large1, Explosion_Large2);
                currentGame->getExplosionList().push_back(new Explosion(explosionID, realPos, owner->getHouseID()));
            }
        }

        if(isVisible(getOwner()->getTeam())) {
            screenborder->shakeScreen(18);
            soundPlayer->playSoundAt(Sound_ExplosionLarge,location);
        }
    }

    TrackedUnit::destroy();
}

bool Devastator::update()
{
	if (active) {
		if ((devastateTimer > 0) && (--devastateTimer == 0)) {
			destroy();
			return false;
		}
	}

	return UnitBase::update();
}

void Devastator::playAttackSound() {
	soundPlayer->playSoundAt(Sound_ExplosionSmall,location);
}

