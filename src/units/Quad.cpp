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

#include <units/Quad.h>

#include <globals.h>

#include <Explosion.h>
#include <FileClasses/GFXManager.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <SoundPlayer.h>

namespace {
constexpr GroundUnitConstants quad_constants {Quad::item_id, 2, Bullet_ShellSmall};
}

Quad::Quad(uint32_t objectID, const ObjectInitializer& initializer)
    : GroundUnit(quad_constants, objectID, initializer) {
    Quad::init();

    Quad::setHealth(getMaxHealth());
}

Quad::Quad(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : GroundUnit(quad_constants, objectID, initializer) {
    Quad::init();
}

void Quad::init() {
    assert(itemID == Unit_Quad);
    owner->incrementUnits(itemID);

    graphicID = ObjPic_Quad;
    graphic   = pGFXManager->getObjPic(graphicID, getOwner()->getHouseID());

    numImagesX = static_cast<int>(ANGLETYPE::NUM_ANGLES);
    numImagesY = 1;
}

Quad::~Quad() = default;

void Quad::playAttackSound() {
    soundPlayer->playSoundAt(Sound_enum::Sound_MachineGun, location);
}

void Quad::destroy(const GameContext& context) {
    if (currentGameMap->tileExists(location) && isVisible()) {
        Coord realPos(lround(realX), lround(realY));
        context.game.addExplosion(Explosion_SmallUnit, realPos, owner->getHouseID());

        if (isVisible(getOwner()->getTeamID()))
            soundPlayer->playSoundAt(Sound_enum::Sound_ExplosionSmall, location);
    }

    GroundUnit::destroy(context);
}
