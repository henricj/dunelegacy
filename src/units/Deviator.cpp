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

#include <units/Deviator.h>

#include <globals.h>

#include <Explosion.h>
#include <FileClasses/GFXManager.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

namespace {
constexpr TrackedUnitConstants deviator_constants{Deviator::item_id, 1, Bullet_DRocket};
}

Deviator::Deviator(uint32_t objectID, const ObjectInitializer& initializer)
    : TrackedUnit(deviator_constants, objectID, initializer) {
    Deviator::init();

    ObjectBase::setHealth(getMaxHealth());
}

Deviator::Deviator(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : TrackedUnit(deviator_constants, objectID, initializer) {
    Deviator::init();
}

void Deviator::init() {
    assert(itemID_ == Unit_Deviator);
    owner_->incrementUnits(itemID_);

    const auto* const gfx = dune::globals::pGFXManager.get();

    graphicID_    = ObjPic_Tank_Base;
    gunGraphicID  = ObjPic_Launcher_Gun;
    graphic_      = gfx->getObjPic(graphicID_, getOwner()->getHouseID());
    turretGraphic = gfx->getObjPic(gunGraphicID, getOwner()->getHouseID());

    numImagesX_ = NUM_ANGLES;
    numImagesY_ = 1;
}

Deviator::~Deviator() = default;

void Deviator::blitToScreen() {
    const auto* const screenborder = dune::globals::screenborder.get();
    auto* const renderer           = dune::globals::renderer.get();

    const auto x1 = screenborder->world2screenX(realX_);
    const auto y1 = screenborder->world2screenY(realY_);

    const auto zoom = dune::globals::currentZoomlevel;

    const auto* const pUnitGraphic = graphic_[zoom];

    const auto source1 = calcSpriteSourceRect(pUnitGraphic, static_cast<int>(drawnAngle_), numImagesX_);
    const auto dest1   = calcSpriteDrawingRect(pUnitGraphic, x1, y1, numImagesX_, 1, HAlign::Center, VAlign::Center);

    Dune_RenderCopyF(renderer, pUnitGraphic, &source1, &dest1);

    static constexpr auto deviatorTurretOffset =
        std::to_array<Coord>({{0, -12}, {0, -8}, {0, -8}, {0, -8}, {0, -12}, {0, -8}, {0, -8}, {0, -8}});

    const auto* const pTurretGraphic = turretGraphic[zoom];

    const auto source2 = calcSpriteSourceRect(pTurretGraphic, static_cast<int>(drawnAngle_), numImagesX_);
    const auto dest2   = calcSpriteDrawingRect(
        pTurretGraphic, screenborder->world2screenX(realX_ + deviatorTurretOffset[static_cast<int>(drawnAngle_)].x),
        screenborder->world2screenY(realY_ + deviatorTurretOffset[static_cast<int>(drawnAngle_)].y), numImagesX_, 1,
        HAlign::Center, VAlign::Center);

    Dune_RenderCopyF(renderer, pTurretGraphic, &source2, &dest2);

    if (isBadlyDamaged()) {
        drawSmoke(x1, y1);
    }
}

void Deviator::destroy(const GameContext& context) {
    if (context.map.tileExists(location_) && isVisible()) {
        Coord realPos(lround(realX_), lround(realY_));
        uint32_t explosionID = context.game.randomGen.getRandOf(Explosion_Medium1, Explosion_Medium2, Explosion_Flames);
        context.game.addExplosion(explosionID, realPos, owner_->getHouseID());

        if (isVisible(getOwner()->getTeamID()))
            dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_ExplosionMedium, location_);
    }

    parent::destroy(context);
}

bool Deviator::canAttack(const ObjectBase* object) const {
    return (object != nullptr) && !object->isAStructure() && !object->isAFlyingUnit()
        && (object->getOwner()->getTeamID() != owner_->getTeamID()) && object->isVisible(getOwner()->getTeamID());
}

void Deviator::playAttackSound() {
    dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_Rocket, location_);
}
