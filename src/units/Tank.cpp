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

#include <Explosion.h>
#include <FileClasses/GFXManager.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

namespace {
constexpr TankBaseConstants tank_constants{Tank::item_id, 1, Bullet_ShellMedium};
}

Tank::Tank(uint32_t objectID, const ObjectInitializer& initializer) : TankBase(tank_constants, objectID, initializer) {
    Tank::init();

    Tank::setHealth(getMaxHealth());
}

Tank::Tank(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : TankBase(tank_constants, objectID, initializer) {
    Tank::init();
}

void Tank::init() {
    assert(itemID == Unit_Tank);
    owner->incrementUnits(itemID);

    auto* const gfx = dune::globals::pGFXManager.get();

    graphicID     = ObjPic_Tank_Base;
    graphic       = gfx->getObjPic(graphicID, getOwner()->getHouseID());
    gunGraphicID  = ObjPic_Tank_Gun;
    turretGraphic = gfx->getObjPic(gunGraphicID, getOwner()->getHouseID());

    numImagesX = static_cast<int>(ANGLETYPE::NUM_ANGLES);
    numImagesY = 1;
}

Tank::~Tank() = default;

void Tank::blitToScreen() {
    auto* const screenborder = dune::globals::screenborder.get();
    auto* const renderer     = dune::globals::renderer.get();
    const auto zoom          = dune::globals::currentZoomlevel;

    const auto x = screenborder->world2screenX(realX);
    const auto y = screenborder->world2screenY(realY);

    const auto* const pUnitGraphic = graphic[zoom];
    const auto source1             = calcSpriteSourceRect(pUnitGraphic, static_cast<int>(drawnAngle), numImagesX);
    const auto dest1 = calcSpriteDrawingRect(pUnitGraphic, x, y, numImagesX, 1, HAlign::Center, VAlign::Center);

    Dune_RenderCopyF(renderer, pUnitGraphic, &source1, &dest1);

    const auto* pTurretGraphic = turretGraphic[zoom];
    const auto source2         = calcSpriteSourceRect(pTurretGraphic, static_cast<int>(drawnTurretAngle),
                                                      static_cast<int>(ANGLETYPE::NUM_ANGLES));
    const auto dest2           = calcSpriteDrawingRect(pTurretGraphic, x, y, static_cast<int>(ANGLETYPE::NUM_ANGLES), 1,
                                                       HAlign::Center, VAlign::Center);

    Dune_RenderCopyF(renderer, pTurretGraphic, &source2, &dest2);

    if (isBadlyDamaged()) {
        drawSmoke(x, y);
    }
}

void Tank::destroy(const GameContext& context) {
    if (context.map.tileExists(location) && isVisible()) {
        const Coord realPos(lround(realX), lround(realY));
        const auto explosionID =
            context.game.randomGen.getRandOf(Explosion_Medium1, Explosion_Medium2, Explosion_Flames);
        context.game.addExplosion(explosionID, realPos, owner->getHouseID());

        if (isVisible(getOwner()->getTeamID()))
            dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_ExplosionMedium, location);
    }

    TankBase::destroy(context);
}

void Tank::playAttackSound() {
    dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_ExplosionSmall, location);
}
