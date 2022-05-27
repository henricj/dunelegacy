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

#include <Explosion.h>
#include <FileClasses/GFXManager.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

namespace {
constexpr TankBaseConstants siege_tank_constants{SiegeTank::item_id, 2, Bullet_ShellLarge};
}

SiegeTank::SiegeTank(uint32_t objectID, const ObjectInitializer& initializer)
    : TankBase(siege_tank_constants, objectID, initializer) {
    SiegeTank::init();

    ObjectBase::setHealth(getMaxHealth());
}

SiegeTank::SiegeTank(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : TankBase(siege_tank_constants, objectID, initializer) {
    SiegeTank::init();
}

void SiegeTank::init() {
    assert(itemID_ == Unit_SiegeTank);
    owner_->incrementUnits(itemID_);

    const auto* const gfx = dune::globals::pGFXManager.get();

    graphicID_    = ObjPic_Siegetank_Base;
    graphic_      = gfx->getObjPic(graphicID_, getOwner()->getHouseID());
    gunGraphicID  = ObjPic_Siegetank_Gun;
    turretGraphic = gfx->getObjPic(gunGraphicID, getOwner()->getHouseID());

    numImagesX_ = NUM_ANGLES;
    numImagesY_ = 1;
}

SiegeTank::~SiegeTank() = default;

void SiegeTank::blitToScreen() {
    const auto* const screenborder = dune::globals::screenborder.get();
    auto* const renderer           = dune::globals::renderer.get();

    const auto x1 = screenborder->world2screenX(realX_);
    const auto y1 = screenborder->world2screenY(realY_);

    const auto zoom = dune::globals::currentZoomlevel;

    const auto* const pUnitGraphic = graphic_[zoom];
    const auto source1             = calcSpriteSourceRect(pUnitGraphic, static_cast<int>(drawnAngle_), numImagesX_);
    const auto dest1 = calcSpriteDrawingRect(pUnitGraphic, x1, y1, numImagesX_, 1, HAlign::Center, VAlign::Center);

    Dune_RenderCopyF(renderer, pUnitGraphic, &source1, &dest1);

    static constexpr auto siegeTankTurretOffset =
        std::to_array<Coord>({{8, -12}, {0, -20}, {0, -20}, {-4, -20}, {-8, -12}, {-8, -4}, {-4, -12}, {8, -4}});

    const auto* const pTurretGraphic = turretGraphic[zoom];

    const auto source2 = calcSpriteSourceRect(pTurretGraphic, static_cast<int>(drawnTurretAngle), NUM_ANGLES);

    const auto offset = siegeTankTurretOffset[static_cast<int>(drawnTurretAngle)];

    const auto dest2 = calcSpriteDrawingRect(pTurretGraphic, screenborder->world2screenX(realX_ + offset.x),
                                             screenborder->world2screenY(realY_ + offset.y), NUM_ANGLES, 1,
                                             HAlign::Center, VAlign::Center);

    Dune_RenderCopyF(renderer, pTurretGraphic, &source2, &dest2);

    if (isBadlyDamaged()) {
        drawSmoke(x1, y1);
    }
}

void SiegeTank::destroy(const GameContext& context) {
    if (context.map.tileExists(location_) && isVisible()) {
        const Coord realPos(lround(realX_), lround(realY_));
        const auto explosionID = context.game.randomGen.getRandOf(Explosion_Medium1, Explosion_Medium2);
        context.game.addExplosion(explosionID, realPos, owner_->getHouseID());

        if (isVisible(getOwner()->getTeamID())) {
            dune::globals::screenborder->shakeScreen(18);
            dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_ExplosionLarge, location_);
        }
    }

    TankBase::destroy(context);
}

void SiegeTank::playAttackSound() {
    dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_ExplosionSmall, location_);
}
