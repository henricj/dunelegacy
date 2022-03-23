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

#include <Explosion.h>
#include <FileClasses/GFXManager.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

#include <players/HumanPlayer.h>

namespace {
constexpr TrackedUnitConstants devastator_constants {Devastator::item_id, 2, Bullet_ShellLarge};
}

Devastator::Devastator(uint32_t objectID, const ObjectInitializer& initializer)
    : TrackedUnit(devastator_constants, objectID, initializer) {
    Devastator::init();

    ObjectBase::setHealth(getMaxHealth());
}

Devastator::Devastator(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : TrackedUnit(devastator_constants, objectID, initializer) {
    Devastator::init();

    auto& stream = initializer.stream();

    devastateTimer = stream.readSint32();
}

void Devastator::init() {
    assert(itemID == Unit_Devastator);
    owner->incrementUnits(itemID);

    graphicID     = ObjPic_Devastator_Base;
    graphic       = pGFXManager->getObjPic(graphicID, getOwner()->getHouseID());
    gunGraphicID  = ObjPic_Devastator_Gun;
    turretGraphic = pGFXManager->getObjPic(gunGraphicID, getOwner()->getHouseID());

    numImagesX = static_cast<int>(ANGLETYPE::NUM_ANGLES);
    numImagesY = 1;
}

Devastator::~Devastator() = default;

void Devastator::save(OutputStream& stream) const {
    parent::save(stream);
    stream.writeSint32(devastateTimer);
}

void Devastator::blitToScreen() {
    const int x1 = screenborder->world2screenX(realX);
    const int y1 = screenborder->world2screenY(realY);

    const auto* const pUnitGraphic = graphic[currentZoomlevel];
    const auto source1             = calcSpriteSourceRect(pUnitGraphic, static_cast<int>(drawnAngle), numImagesX);
    const auto dest1 = calcSpriteDrawingRect(pUnitGraphic, x1, y1, numImagesX, 1, HAlign::Center, VAlign::Center);

    Dune_RenderCopy(renderer, pUnitGraphic, &source1, &dest1);

    constexpr Coord devastatorTurretOffset[] = {Coord(8, -16),  Coord(-4, -12), Coord(0, -16),  Coord(4, -12),
                                                Coord(-8, -16), Coord(0, -12),  Coord(-4, -12), Coord(0, -12)};

    const auto* const pTurretGraphic = turretGraphic[currentZoomlevel];
    const auto source2               = calcSpriteSourceRect(pTurretGraphic, static_cast<int>(drawnAngle), numImagesX);
    const auto dest2                 = calcSpriteDrawingRect(
                        pTurretGraphic, screenborder->world2screenX(realX + devastatorTurretOffset[static_cast<int>(drawnAngle)].x),
                        screenborder->world2screenY(realY + devastatorTurretOffset[static_cast<int>(drawnAngle)].y), numImagesX, 1,
                        HAlign::Center, VAlign::Center);

    Dune_RenderCopy(renderer, pTurretGraphic, &source2, &dest2);

    if (isBadlyDamaged()) {
        drawSmoke(x1, y1);
    }
}

void Devastator::handleStartDevastateClick() {
    currentGame->getCommandManager().addCommand(
        Command(pLocalPlayer->getPlayerID(), CMDTYPE::CMD_DEVASTATOR_STARTDEVASTATE, objectID));
}

void Devastator::doStartDevastate() {
    if (devastateTimer <= 0)
        devastateTimer = 200;
}

void Devastator::destroy(const GameContext& context) {
    if (context.map.tileExists(location) && isVisible()) {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                Coord realPos(lround(realX) + (i - 1) * TILESIZE, lround(realY) + (j - 1) * TILESIZE);

                context.map.damage(context, objectID, owner, realPos, itemID, 150, 16, false);

                uint32_t explosionID = context.game.randomGen.getRandOf(Explosion_Large1, Explosion_Large2);
                context.game.addExplosion(explosionID, realPos, owner->getHouseID());
            }
        }

        if (isVisible(getOwner()->getTeamID())) {
            screenborder->shakeScreen(18);
            soundPlayer->playSoundAt(Sound_ExplosionLarge, location);
        }
    }

    parent::destroy(context);
}

bool Devastator::update(const GameContext& context) {
    if (active) {
        if ((devastateTimer > 0) && (--devastateTimer == 0)) {
            destroy(context);
            return false;
        }
    }

    return UnitBase::update(context);
}

void Devastator::playAttackSound() {
    soundPlayer->playSoundAt(Sound_ExplosionSmall, location);
}
