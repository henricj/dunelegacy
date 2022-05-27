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

#include <Explosion.h>
#include <FileClasses/GFXManager.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

namespace {
constexpr TrackedUnitConstants sonic_tank_constants{SonicTank::item_id, 1, Bullet_Sonic};
}

SonicTank::SonicTank(uint32_t objectID, const ObjectInitializer& initializer)
    : TrackedUnit(sonic_tank_constants, objectID, initializer) {
    SonicTank::init();

    SonicTank::setHealth(getMaxHealth());
}

SonicTank::SonicTank(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : TrackedUnit(sonic_tank_constants, objectID, initializer) {
    SonicTank::init();
}

void SonicTank::init() {
    assert(itemID_ == Unit_SonicTank);
    owner_->incrementUnits(itemID_);

    const auto* const gfx = dune::globals::pGFXManager.get();

    graphicID_    = ObjPic_Tank_Base;
    gunGraphicID  = ObjPic_Sonictank_Gun;
    graphic_      = gfx->getObjPic(graphicID_, getOwner()->getHouseID());
    turretGraphic = gfx->getObjPic(gunGraphicID, getOwner()->getHouseID());

    numImagesX_ = NUM_ANGLES;
    numImagesY_ = 1;
}

SonicTank::~SonicTank() = default;

void SonicTank::blitToScreen() {
    const auto* const screenborder = dune::globals::screenborder.get();
    auto* const renderer           = dune::globals::renderer.get();
    const auto zoom                = dune::globals::currentZoomlevel;

    const auto x1 = screenborder->world2screenX(realX_);
    const auto y1 = screenborder->world2screenY(realY_);

    const auto* const pUnitGraphic = graphic_[zoom];
    const auto source1             = calcSpriteSourceRect(pUnitGraphic, static_cast<int>(drawnAngle_), numImagesX_);
    const auto dest1 = calcSpriteDrawingRect(pUnitGraphic, x1, y1, numImagesX_, 1, HAlign::Center, VAlign::Center);

    Dune_RenderCopyF(renderer, pUnitGraphic, &source1, &dest1);

    static constexpr auto sonicTankTurretOffset =
        std::to_array<Coord>({{0, -8}, {0, -8}, {0, -8}, {0, -8}, {0, -8}, {0, -8}, {0, -8}, {0, -8}});

    const auto* const pTurretGraphic = turretGraphic[zoom];

    const auto source2 = calcSpriteSourceRect(pTurretGraphic, static_cast<int>(drawnAngle_), numImagesX_);
    const auto dest2   = calcSpriteDrawingRect(
          pTurretGraphic, screenborder->world2screenX(realX_ + sonicTankTurretOffset[static_cast<int>(drawnAngle_)].x),
          screenborder->world2screenY(realY_ + sonicTankTurretOffset[static_cast<int>(drawnAngle_)].y), numImagesX_, 1,
          HAlign::Center, VAlign::Center);

    Dune_RenderCopyF(renderer, pTurretGraphic, &source2, &dest2);

    if (isBadlyDamaged()) {
        drawSmoke(x1, y1);
    }
}

void SonicTank::destroy(const GameContext& context) {
    if (context.map.tileExists(location_) && isVisible()) {
        const Coord realPos(lround(realX_), lround(realY_));
        context.game.addExplosion(Explosion_SmallUnit, realPos, owner_->getHouseID());

        if (isVisible(getOwner()->getTeamID()))
            dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_ExplosionSmall, location_);
    }

    parent::destroy(context);
}

void SonicTank::handleDamage(const GameContext& context, int damage, uint32_t damagerID, House* damagerOwner) {
    const auto* const damager = context.objectManager.getObject(damagerID);

    if (!damager || (damager->getItemID() != Unit_SonicTank))
        parent::handleDamage(context, damage, damagerID, damagerOwner);
}

bool SonicTank::canAttack(const ObjectBase* object) const {
    return ((object != nullptr) && ObjectBase::canAttack(object) && (object->getItemID() != Unit_SonicTank));
}

void SonicTank::playAttackSound() {
    dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_Sonic, location_);
}
