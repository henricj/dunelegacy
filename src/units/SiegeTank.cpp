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

namespace {
constexpr TankBaseConstants siege_tank_constants{SiegeTank::item_id, 2, Bullet_ShellLarge};
}

SiegeTank::SiegeTank(Uint32 objectID, const ObjectInitializer& initializer)
    : TankBase(siege_tank_constants, objectID, initializer) {
    SiegeTank::init();

    ObjectBase::setHealth(getMaxHealth());
}

SiegeTank::SiegeTank(Uint32 objectID, const ObjectStreamInitializer& initializer)
    : TankBase(siege_tank_constants, objectID, initializer) {
    SiegeTank::init();
}

void SiegeTank::init() {
    assert(itemID == Unit_SiegeTank);
    owner->incrementUnits(itemID);

    graphicID = ObjPic_Siegetank_Base;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    gunGraphicID = ObjPic_Siegetank_Gun;
    turretGraphic = pGFXManager->getObjPic(gunGraphicID,getOwner()->getHouseID());

    numImagesX = static_cast<int>(ANGLETYPE::NUM_ANGLES);
    numImagesY = 1;
}

SiegeTank::~SiegeTank() = default;

void SiegeTank::blitToScreen() {
    const auto x1 = screenborder->world2screenX(realX);
    const auto y1 = screenborder->world2screenY(realY);

    const auto* const pUnitGraphic = graphic[currentZoomlevel];
    const auto        source1      = calcSpriteSourceRect(pUnitGraphic, static_cast<int>(drawnAngle), numImagesX);
    const auto dest1 = calcSpriteDrawingRect(pUnitGraphic, x1, y1, numImagesX, 1, HAlign::Center, VAlign::Center);

    Dune_RenderCopy(renderer, pUnitGraphic, &source1, &dest1);

    static constexpr Coord siegeTankTurretOffset[] = {Coord(8, -12),  Coord(0, -20), Coord(0, -20),  Coord(-4, -20),
                                                      Coord(-8, -12), Coord(-8, -4), Coord(-4, -12), Coord(8, -4)};

    const auto* const pTurretGraphic = turretGraphic[currentZoomlevel];
    const auto        source2        = calcSpriteSourceRect(pTurretGraphic, static_cast<int>(drawnTurretAngle),
                                              static_cast<int>(ANGLETYPE::NUM_ANGLES));
    const auto        dest2          = calcSpriteDrawingRect(
        pTurretGraphic,
        screenborder->world2screenX(realX + siegeTankTurretOffset[static_cast<int>(drawnTurretAngle)].x),
        screenborder->world2screenY(realY + siegeTankTurretOffset[static_cast<int>(drawnTurretAngle)].y),
        static_cast<int>(ANGLETYPE::NUM_ANGLES), 1, HAlign::Center, VAlign::Center);

    Dune_RenderCopy(renderer, pTurretGraphic, &source2, &dest2);

    if(isBadlyDamaged()) { drawSmoke(x1, y1); }
}

void SiegeTank::destroy(const GameContext& context) {
    if(context.map.tileExists(location) && isVisible()) {
        const Coord realPos(lround(realX), lround(realY));
        const auto explosionID = context.game.randomGen.getRandOf(Explosion_Medium1, Explosion_Medium2);
        context.game.addExplosion(explosionID, realPos, owner->getHouseID());

        if(isVisible(getOwner()->getTeamID())) {
            screenborder->shakeScreen(18);
            soundPlayer->playSoundAt(Sound_ExplosionLarge,location);
        }
    }

    TankBase::destroy(context);
}

void SiegeTank::playAttackSound() {
    soundPlayer->playSoundAt(Sound_ExplosionSmall,location);
}
