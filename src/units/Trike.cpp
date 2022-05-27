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

#include <units/Trike.h>

#include <globals.h>

#include <Explosion.h>
#include <FileClasses/GFXManager.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <SoundPlayer.h>

namespace {
constexpr GroundUnitConstants trike_constants{Trike::item_id, 2, Bullet_ShellSmall};
}

Trike::Trike(uint32_t objectID, const ObjectInitializer& initializer)
    : GroundUnit(trike_constants, objectID, initializer) {
    Trike::init();

    Trike::setHealth(getMaxHealth());
}

Trike::Trike(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : GroundUnit(trike_constants, objectID, initializer) {
    Trike::init();
}

void Trike::init() {
    assert(itemID_ == Unit_Trike);
    owner_->incrementUnits(itemID_);

    graphicID_ = ObjPic_Trike;
    graphic_   = dune::globals::pGFXManager->getObjPic(graphicID_, getOwner()->getHouseID());

    numImagesX_ = NUM_ANGLES;
    numImagesY_ = 1;
}

Trike::~Trike() = default;

void Trike::destroy(const GameContext& context) {
    if (context.map.tileExists(location_) && isVisible()) {
        Coord realPos(lround(realX_), lround(realY_));
        context.game.addExplosion(Explosion_SmallUnit, realPos, owner_->getHouseID());

        if (isVisible(getOwner()->getTeamID()))
            dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_ExplosionSmall, location_);
    }

    GroundUnit::destroy(context);
}

void Trike::playAttackSound() {
    dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_MachineGun, location_);
}
