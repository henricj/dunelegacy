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
constexpr TrackedUnitConstants sonic_tank_constants {SonicTank::item_id, 1, Bullet_Sonic};
}

SonicTank::SonicTank(uint32_t objectID, const ObjectInitializer& initializer)
    : TrackedUnit(sonic_tank_constants, objectID, initializer) {
    SonicTank::init();

    setHealth(getMaxHealth());
}

SonicTank::SonicTank(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : TrackedUnit(sonic_tank_constants, objectID, initializer) {
    SonicTank::init();
}

void SonicTank::init() {
    assert(itemID == Unit_SonicTank);
    owner->incrementUnits(itemID);

    graphicID     = ObjPic_Tank_Base;
    gunGraphicID  = ObjPic_Sonictank_Gun;
    graphic       = pGFXManager->getObjPic(graphicID, getOwner()->getHouseID());
    turretGraphic = pGFXManager->getObjPic(gunGraphicID, getOwner()->getHouseID());

    numImagesX = static_cast<int>(ANGLETYPE::NUM_ANGLES);
    numImagesY = 1;
}

SonicTank::~SonicTank() = default;

void SonicTank::blitToScreen() {
    const auto x1 = screenborder->world2screenX(realX);
    const auto y1 = screenborder->world2screenY(realY);

    const auto* const pUnitGraphic = graphic[currentZoomlevel];
    const auto source1             = calcSpriteSourceRect(pUnitGraphic, static_cast<int>(drawnAngle), numImagesX);
    const auto dest1               = calcSpriteDrawingRect(pUnitGraphic, x1, y1, numImagesX, 1, HAlign::Center, VAlign::Center);

    Dune_RenderCopy(renderer, pUnitGraphic, &source1, &dest1);

    static constexpr Coord sonicTankTurretOffset[] = {Coord(0, -8), Coord(0, -8), Coord(0, -8), Coord(0, -8),
                                                      Coord(0, -8), Coord(0, -8), Coord(0, -8), Coord(0, -8)};

    const auto* const pTurretGraphic = turretGraphic[currentZoomlevel];
    const auto source2               = calcSpriteSourceRect(pTurretGraphic, static_cast<int>(drawnAngle), numImagesX);
    const auto dest2                 = calcSpriteDrawingRect(
                        pTurretGraphic, screenborder->world2screenX(realX + sonicTankTurretOffset[static_cast<int>(drawnAngle)].x),
                        screenborder->world2screenY(realY + sonicTankTurretOffset[static_cast<int>(drawnAngle)].y), numImagesX, 1,
                        HAlign::Center, VAlign::Center);

    Dune_RenderCopy(renderer, pTurretGraphic, &source2, &dest2);

    if (isBadlyDamaged()) {
        drawSmoke(x1, y1);
    }
}

void SonicTank::destroy(const GameContext& context) {
    if (context.map.tileExists(location) && isVisible()) {
        const Coord realPos(lround(realX), lround(realY));
        context.game.addExplosion(Explosion_SmallUnit, realPos, owner->getHouseID());

        if (isVisible(getOwner()->getTeamID()))
            soundPlayer->playSoundAt(Sound_ExplosionSmall, location);
    }

    parent::destroy(context);
}

void SonicTank::handleDamage(const GameContext& context, int damage, uint32_t damagerID, House* damagerOwner) {
    auto* const damager = context.objectManager.getObject(damagerID);

    if (!damager || (damager->getItemID() != Unit_SonicTank))
        parent::handleDamage(context, damage, damagerID, damagerOwner);
}

bool SonicTank::canAttack(const ObjectBase* object) const {
    return ((object != nullptr) && ObjectBase::canAttack(object) && (object->getItemID() != Unit_SonicTank));
}

void SonicTank::playAttackSound() {
    soundPlayer->playSoundAt(Sound_Sonic, location);
}
