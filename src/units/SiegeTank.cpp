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

#include <units/SiegeTank.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>
#include <Map.h>
#include <Explosion.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

SiegeTank::SiegeTank(House* newOwner) : TankBase(newOwner) {
    SiegeTank::init();

    setHealth(getMaxHealth());
}

SiegeTank::SiegeTank(InputStream& stream) : TankBase(stream) {
    SiegeTank::init();
}

void SiegeTank::init() {
    itemID = Unit_SiegeTank;
    owner->incrementUnits(itemID);

    numWeapons = 2;
    bulletType = Bullet_ShellLarge;

    graphicID = ObjPic_Siegetank_Base;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    gunGraphicID = ObjPic_Siegetank_Gun;
    turretGraphic = pGFXManager->getObjPic(gunGraphicID,getOwner()->getHouseID());

    numImagesX = NUM_ANGLES;
    numImagesY = 1;
}

SiegeTank::~SiegeTank() = default;

void SiegeTank::blitToScreen() {
    int x1 = screenborder->world2screenX(realX);
    int y1 = screenborder->world2screenY(realY);

    SDL_Texture* pUnitGraphic = graphic[currentZoomlevel];
    SDL_Rect source1 = calcSpriteSourceRect(pUnitGraphic, drawnAngle, numImagesX);
    SDL_Rect dest1 = calcSpriteDrawingRect( pUnitGraphic, x1, y1, numImagesX, 1, HAlign::Center, VAlign::Center);

    SDL_RenderCopy(renderer, pUnitGraphic, &source1, &dest1);

    const Coord siegeTankTurretOffset[] =   {   Coord(8, -12),
                                                Coord(0, -20),
                                                Coord(0, -20),
                                                Coord(-4, -20),
                                                Coord(-8, -12),
                                                Coord(-8, -4),
                                                Coord(-4, -12),
                                                Coord(8, -4)
                                            };

    SDL_Texture* pTurretGraphic = turretGraphic[currentZoomlevel];
    SDL_Rect source2 = calcSpriteSourceRect(pTurretGraphic, drawnTurretAngle, NUM_ANGLES);
    SDL_Rect dest2 = calcSpriteDrawingRect( pTurretGraphic,
                                            screenborder->world2screenX(realX + siegeTankTurretOffset[drawnTurretAngle].x),
                                            screenborder->world2screenY(realY + siegeTankTurretOffset[drawnTurretAngle].y),
                                            NUM_ANGLES, 1, HAlign::Center, VAlign::Center);

    SDL_RenderCopy(renderer, pTurretGraphic, &source2, &dest2);

    if(isBadlyDamaged()) {
        drawSmoke(x1, y1);
    }
}

void SiegeTank::destroy() {
    if(currentGameMap->tileExists(location) && isVisible()) {
        Coord realPos(lround(realX), lround(realY));
        Uint32 explosionID = currentGame->randomGen.getRandOf({Explosion_Medium1, Explosion_Medium2});
        currentGame->getExplosionList().push_back(new Explosion(explosionID, realPos, owner->getHouseID()));

        if(isVisible(getOwner()->getTeamID())) {
            screenborder->shakeScreen(18);
            soundPlayer->playSoundAt(Sound_ExplosionLarge,location);
        }
    }

    TankBase::destroy();
}

void SiegeTank::playAttackSound() {
    soundPlayer->playSoundAt(Sound_ExplosionSmall,location);
}

