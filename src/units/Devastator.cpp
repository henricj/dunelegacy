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


Devastator::~Devastator() = default;

void Devastator::save(OutputStream& stream) const
{
    TrackedUnit::save(stream);
    stream.writeSint32(devastateTimer);
}

void Devastator::blitToScreen()
{
    int x1 = screenborder->world2screenX(realX);
    int y1 = screenborder->world2screenY(realY);

    SDL_Texture* pUnitGraphic = graphic[currentZoomlevel];
    SDL_Rect source1 = calcSpriteSourceRect(pUnitGraphic, drawnAngle, numImagesX);
    SDL_Rect dest1 = calcSpriteDrawingRect( pUnitGraphic, x1, y1, numImagesX, 1, HAlign::Center, VAlign::Center);

    SDL_RenderCopy(renderer, pUnitGraphic, &source1, &dest1);

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

    SDL_Texture* pTurretGraphic = turretGraphic[currentZoomlevel];
    SDL_Rect source2 = calcSpriteSourceRect(pTurretGraphic, drawnAngle, numImagesX);
    SDL_Rect dest2 = calcSpriteDrawingRect( pTurretGraphic,
                                            screenborder->world2screenX(realX + devastatorTurretOffset[drawnAngle].x),
                                            screenborder->world2screenY(realY + devastatorTurretOffset[drawnAngle].y),
                                            numImagesX, 1, HAlign::Center, VAlign::Center);

    SDL_RenderCopy(renderer, pTurretGraphic, &source2, &dest2);

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

                Uint32 explosionID = currentGame->randomGen.getRandOf({Explosion_Large1, Explosion_Large2});
                currentGame->getExplosionList().push_back(new Explosion(explosionID, realPos, owner->getHouseID()));
            }
        }

        if(isVisible(getOwner()->getTeamID())) {
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

