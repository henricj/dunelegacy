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

#include <units/Launcher.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>
#include <Map.h>
#include <Explosion.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

Launcher::Launcher(House* newOwner) : TrackedUnit(newOwner) {
    Launcher::init();

    setHealth(getMaxHealth());
}

Launcher::Launcher(InputStream& stream) : TrackedUnit(stream) {
    Launcher::init();
}
void Launcher::init() {
    itemID = Unit_Launcher;
    owner->incrementUnits(itemID);

	graphicID = ObjPic_Tank_Base;
	gunGraphicID = ObjPic_Launcher_Gun;
	graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
	turretGraphic = pGFXManager->getObjPic(gunGraphicID,getOwner()->getHouseID());

	numImagesX = NUM_ANGLES;
	numImagesY = 1;

	numWeapons = 2;
	bulletType = Bullet_Rocket;
}

Launcher::~Launcher() {
}

void Launcher::blitToScreen() {
    SDL_Surface* pUnitGraphic = graphic[currentZoomlevel];
    int imageW1 = pUnitGraphic->w/numImagesX;
    int x1 = screenborder->world2screenX(realX);
    int y1 = screenborder->world2screenY(realY);

    SDL_Rect source1 = { drawnAngle * imageW1, 0, imageW1, pUnitGraphic->h };
    SDL_Rect dest1 = { x1 - imageW1/2, y1 - pUnitGraphic->h/2, imageW1, pUnitGraphic->h };

    SDL_BlitSurface(pUnitGraphic, &source1, screen, &dest1);

    const Coord launcherTurretOffset[] =    {   Coord(0, -12),
                                                Coord(0, -8),
                                                Coord(0, -8),
                                                Coord(0, -8),
                                                Coord(0, -12),
                                                Coord(0, -8),
                                                Coord(0, -8),
                                                Coord(0, -8)
                                            };

    SDL_Surface* pTurretGraphic = turretGraphic[currentZoomlevel];
    int imageW2 = pTurretGraphic->w/numImagesX;
    int x2 = screenborder->world2screenX(realX + launcherTurretOffset[drawnAngle].x);
    int y2 = screenborder->world2screenY(realY + launcherTurretOffset[drawnAngle].y);

    SDL_Rect source2 = { drawnAngle * imageW2, 0, imageW2, pTurretGraphic->h };
    SDL_Rect dest2 = { x2 - imageW2/2, y2 - pTurretGraphic->h/2, imageW2, pTurretGraphic->h };

    SDL_BlitSurface(pTurretGraphic, &source2, screen, &dest2);

    if(isBadlyDamaged()) {
        drawSmoke(x1, y1);
    }
}

void Launcher::destroy() {
    if(currentGameMap->tileExists(location) && isVisible()) {
        Coord realPos(lround(realX), lround(realY));
        Uint32 explosionID = currentGame->randomGen.getRandOf(3,Explosion_Medium1, Explosion_Medium2,Explosion_Flames);
        currentGame->getExplosionList().push_back(new Explosion(explosionID, realPos, owner->getHouseID()));

        if(isVisible(getOwner()->getTeam()))
            soundPlayer->playSoundAt(Sound_ExplosionMedium,location);
    }

    TrackedUnit::destroy();
}

bool Launcher::canAttack(const ObjectBase* object) const {
	return ((object != NULL)
			&& ((object->getOwner()->getTeam() != owner->getTeam()) || object->getItemID() == Unit_Sandworm)
			&& object->isVisible(getOwner()->getTeam())
            && object->getItemID() != Unit_Carryall); // Stop launchers attacking carryalls
}

void Launcher::playAttackSound() {
	soundPlayer->playSoundAt(Sound_Rocket,location);
}
