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

#include <units/SonicTank.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>
#include <Map.h>
#include <Explosion.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>


SonicTank::SonicTank(House* newOwner) : TrackedUnit(newOwner) {
    SonicTank::init();

    setHealth(getMaxHealth());
}

SonicTank::SonicTank(InputStream& stream) : TrackedUnit(stream) {
    SonicTank::init();
}

void SonicTank::init() {
    itemID = Unit_SonicTank;
    owner->incrementUnits(itemID);

	numWeapons = 1;
	bulletType = Bullet_Sonic;

	graphicID = ObjPic_Tank_Base;
	gunGraphicID = ObjPic_Sonictank_Gun;
	graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
	turretGraphic = pGFXManager->getObjPic(gunGraphicID,getOwner()->getHouseID());

	numImagesX = NUM_ANGLES;
	numImagesY = 1;
}

SonicTank::~SonicTank() {
}

void SonicTank::blitToScreen() {
    SDL_Surface* pUnitGraphic = graphic[currentZoomlevel];
    int imageW1 = pUnitGraphic->w/numImagesX;
    int x1 = screenborder->world2screenX(realX);
    int y1 = screenborder->world2screenY(realY);

    SDL_Rect source1 = { drawnAngle * imageW1, 0, imageW1, pUnitGraphic->h };
    SDL_Rect dest1 = { x1 - imageW1/2, y1 - pUnitGraphic->h/2, imageW1, pUnitGraphic->h };

    SDL_BlitSurface(pUnitGraphic, &source1, screen, &dest1);

    const Coord sonicTankTurretOffset[] =   {   Coord(0, -8),
                                                Coord(0, -8),
                                                Coord(0, -8),
                                                Coord(0, -8),
                                                Coord(0, -8),
                                                Coord(0, -8),
                                                Coord(0, -8),
                                                Coord(0, -8)
                                            };

    SDL_Surface* pTurretGraphic = turretGraphic[currentZoomlevel];
    int imageW2 = pTurretGraphic->w/numImagesX;
    int x2 = screenborder->world2screenX(realX + sonicTankTurretOffset[drawnAngle].x);
    int y2 = screenborder->world2screenY(realY + sonicTankTurretOffset[drawnAngle].y);

    SDL_Rect source2 = { drawnAngle * imageW2, 0, imageW2, pTurretGraphic->h };
    SDL_Rect dest2 = { x2 - imageW2/2, y2 - pTurretGraphic->h/2, imageW2, pTurretGraphic->h };

    SDL_BlitSurface(pTurretGraphic, &source2, screen, &dest2);

    if(isBadlyDamaged()) {
        drawSmoke(x1, y1);
    }
}

void SonicTank::destroy() {
    if(currentGameMap->tileExists(location) && isVisible()) {
        Coord realPos(lround(realX), lround(realY));
        currentGame->getExplosionList().push_back(new Explosion(Explosion_SmallUnit, realPos, owner->getHouseID()));

        if(isVisible(getOwner()->getTeam()))
            soundPlayer->playSoundAt(Sound_ExplosionSmall,location);
    }

    TrackedUnit::destroy();
}

void SonicTank::handleDamage(int damage, Uint32 damagerID, House* damagerOwner) {
    ObjectBase* damager = currentGame->getObjectManager().getObject(damagerID);

	if (!damager || (damager->getItemID() != Unit_SonicTank))
		TrackedUnit::handleDamage(damage, damagerID, damagerOwner);
}

bool SonicTank::canAttack(const ObjectBase *object) const {
	return ((object != NULL) && ObjectBase::canAttack(object) && (object->getItemID() != Unit_SonicTank));
}

void SonicTank::playAttackSound() {
	soundPlayer->playSoundAt(Sound_Sonic,location);
}
