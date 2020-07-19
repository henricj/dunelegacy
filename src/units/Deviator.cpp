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

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>
#include <Map.h>
#include <Explosion.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

Deviator::Deviator(ItemID_enum itemID, Uint32 objectID, const ObjectInitializer& initializer) : TrackedUnit(itemID, objectID, initializer) {
    Deviator::init();

    ObjectBase::setHealth(getMaxHealth());
}

Deviator::Deviator(ItemID_enum itemID, Uint32 objectID, const ObjectStreamInitializer& initializer) : TrackedUnit(itemID, objectID, initializer) {
    Deviator::init();
}

void Deviator::init()
{
    assert(itemID == Unit_Deviator);
    owner->incrementUnits(itemID);

    graphicID = ObjPic_Tank_Base;
    gunGraphicID = ObjPic_Launcher_Gun;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    turretGraphic = pGFXManager->getObjPic(gunGraphicID,getOwner()->getHouseID());

    numImagesX = static_cast<int>(ANGLETYPE::NUM_ANGLES);
    numImagesY = 1;

    numWeapons = 1;
    bulletType = Bullet_DRocket;
}

Deviator::~Deviator() = default;

void Deviator::blitToScreen()
{
    const auto x1 = screenborder->world2screenX(realX);
    const auto y1 = screenborder->world2screenY(realY);

    const auto* const pUnitGraphic = graphic[currentZoomlevel];
    const auto source1 = calcSpriteSourceRect(pUnitGraphic, static_cast<int>(drawnAngle), numImagesX);
    const auto dest1 = calcSpriteDrawingRect( pUnitGraphic, x1, y1, numImagesX, 1, HAlign::Center, VAlign::Center);

    Dune_RenderCopy(renderer, pUnitGraphic, &source1, &dest1);

    static constexpr Coord deviatorTurretOffset[] = {Coord(0, -12), Coord(0, -8), Coord(0, -8), Coord(0, -8),
                                                     Coord(0, -12), Coord(0, -8), Coord(0, -8), Coord(0, -8)};

    const auto* const pTurretGraphic = turretGraphic[currentZoomlevel];
    const auto     source2        = calcSpriteSourceRect(pTurretGraphic, static_cast<int>(drawnAngle), numImagesX);
    const auto     dest2          = calcSpriteDrawingRect(
        pTurretGraphic, screenborder->world2screenX(realX + deviatorTurretOffset[static_cast<int>(drawnAngle)].x),
        screenborder->world2screenY(realY + deviatorTurretOffset[static_cast<int>(drawnAngle)].y), numImagesX, 1,
        HAlign::Center, VAlign::Center);

    Dune_RenderCopy(renderer, pTurretGraphic, &source2, &dest2);

    if(isBadlyDamaged()) {
        drawSmoke(x1, y1);
    }
}

void Deviator::destroy(const GameContext& context) {
    if(context.map.tileExists(location) && isVisible()) {
        Coord realPos(lround(realX), lround(realY));
        Uint32 explosionID = context.game.randomGen.getRandOf(Explosion_Medium1, Explosion_Medium2,Explosion_Flames);
        context.game.addExplosion(explosionID, realPos, owner->getHouseID());

        if(isVisible(getOwner()->getTeamID()))
            soundPlayer->playSoundAt(Sound_ExplosionMedium,location);
    }

    parent::destroy(context);
}

bool Deviator::canAttack(const ObjectBase* object) const {
    return (object != nullptr) && !object->isAStructure() && !object->isAFlyingUnit() &&
           (object->getOwner()->getTeamID() != owner->getTeamID()) && object->isVisible(getOwner()->getTeamID());
}

void Deviator::playAttackSound() {
    soundPlayer->playSoundAt(Sound_Rocket,location);
}
