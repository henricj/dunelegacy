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

#include <units/Tank.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>
#include <Map.h>
#include <Explosion.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>


Tank::Tank(House* newOwner) : TankBase(newOwner) {
    Tank::init();

    setHealth(getMaxHealth());
}

Tank::Tank(InputStream& stream) : TankBase(stream) {
    Tank::init();
}

void Tank::init() {
    itemID = Unit_Tank;
    owner->incrementUnits(itemID);

    numWeapons = 1;
    bulletType = Bullet_ShellMedium;

    graphicID = ObjPic_Tank_Base;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    gunGraphicID = ObjPic_Tank_Gun;
    turretGraphic = pGFXManager->getObjPic(gunGraphicID,getOwner()->getHouseID());

    numImagesX = NUM_ANGLES;
    numImagesY = 1;
}

Tank::~Tank() = default;


void Tank::blitToScreen() {
    int x = screenborder->world2screenX(realX);
    int y = screenborder->world2screenY(realY);

    SDL_Texture* pUnitGraphic = graphic[currentZoomlevel];
    SDL_Rect source1 = calcSpriteSourceRect(pUnitGraphic, drawnAngle, numImagesX);
    SDL_Rect dest1 = calcSpriteDrawingRect( pUnitGraphic, x, y, numImagesX, 1, HAlign::Center, VAlign::Center);

    SDL_RenderCopy(renderer, pUnitGraphic, &source1, &dest1);

    SDL_Texture* pTurretGraphic = turretGraphic[currentZoomlevel];
    SDL_Rect source2 = calcSpriteSourceRect(pTurretGraphic, drawnTurretAngle, NUM_ANGLES);
    SDL_Rect dest2 = calcSpriteDrawingRect( pTurretGraphic, x, y, NUM_ANGLES, 1, HAlign::Center, VAlign::Center);

    SDL_RenderCopy(renderer, pTurretGraphic, &source2, &dest2);

    if(isBadlyDamaged()) {
        drawSmoke(x, y);
    }
}

void Tank::destroy() {
    if(currentGameMap->tileExists(location) && isVisible()) {
        Coord realPos(lround(realX), lround(realY));
        Uint32 explosionID = currentGame->randomGen.getRandOf({Explosion_Medium1, Explosion_Medium2,Explosion_Flames});
        currentGame->getExplosionList().push_back(new Explosion(explosionID, realPos, owner->getHouseID()));

        if(isVisible(getOwner()->getTeamID()))
            soundPlayer->playSoundAt(Sound_ExplosionMedium,location);
    }

    TankBase::destroy();
}

void Tank::playAttackSound() {
    soundPlayer->playSoundAt(Sound_ExplosionSmall,location);
}
