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

Launcher::~Launcher() = default;

void Launcher::blitToScreen() {
    int x1 = screenborder->world2screenX(realX);
    int y1 = screenborder->world2screenY(realY);

    SDL_Texture* pUnitGraphic = graphic[currentZoomlevel];
    SDL_Rect source1 = calcSpriteSourceRect(pUnitGraphic, drawnAngle, numImagesX);
    SDL_Rect dest1 = calcSpriteDrawingRect( pUnitGraphic, x1, y1, numImagesX, 1, HAlign::Center, VAlign::Center);

    SDL_RenderCopy(renderer, pUnitGraphic, &source1, &dest1);

    const Coord launcherTurretOffset[] =    {   Coord(0, -12),
                                                Coord(0, -8),
                                                Coord(0, -8),
                                                Coord(0, -8),
                                                Coord(0, -12),
                                                Coord(0, -8),
                                                Coord(0, -8),
                                                Coord(0, -8)
                                            };

    SDL_Texture* pTurretGraphic = turretGraphic[currentZoomlevel];
    SDL_Rect source2 = calcSpriteSourceRect(pTurretGraphic, drawnAngle, numImagesX);
    SDL_Rect dest2 = calcSpriteDrawingRect( pTurretGraphic,
                                            screenborder->world2screenX(realX + launcherTurretOffset[drawnAngle].x),
                                            screenborder->world2screenY(realY + launcherTurretOffset[drawnAngle].y),
                                            numImagesX, 1, HAlign::Center, VAlign::Center);

    SDL_RenderCopy(renderer, pTurretGraphic, &source2, &dest2);

    if(isBadlyDamaged()) {
        drawSmoke(x1, y1);
    }
}

void Launcher::destroy() {
    if(currentGameMap->tileExists(location) && isVisible()) {
        Coord realPos(lround(realX), lround(realY));
        Uint32 explosionID = currentGame->randomGen.getRandOf({Explosion_Medium1, Explosion_Medium2,Explosion_Flames});
        currentGame->getExplosionList().push_back(new Explosion(explosionID, realPos, owner->getHouseID()));

        if(isVisible(getOwner()->getTeamID()))
            soundPlayer->playSoundAt(Sound_ExplosionMedium,location);
    }

    TrackedUnit::destroy();
}

bool Launcher::canAttack(const ObjectBase* object) const {
    return ((object != nullptr)
            && ((object->getOwner()->getTeamID() != owner->getTeamID()) || object->getItemID() == Unit_Sandworm)
            && object->isVisible(getOwner()->getTeamID()));
}

void Launcher::playAttackSound() {
    soundPlayer->playSoundAt(Sound_Rocket,location);
}
