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

#include <units/RaiderTrike.h>

#include <House.h>
#include <Game.h>
#include <Map.h>

namespace {
using namespace Dune::Engine;

constexpr GroundUnitConstants raider_trike_constants{RaiderTrike::item_id, 2, Bullet_ShellSmall};
}

namespace Dune::Engine {

RaiderTrike::RaiderTrike(uint32_t objectID, const ObjectInitializer& initializer)
    : GroundUnit(raider_trike_constants, objectID, initializer) {
    RaiderTrike::init();

    RaiderTrike::setHealth(initializer.game(), getMaxHealth(initializer.game()));
}

RaiderTrike::RaiderTrike(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : GroundUnit(raider_trike_constants, objectID, initializer) {
    RaiderTrike::init();
}

void RaiderTrike::init() {
    assert(itemID == Unit_RaiderTrike);
    owner->incrementUnits(itemID);
}

RaiderTrike::~RaiderTrike() = default;

void RaiderTrike::destroy(const GameContext& context) {
    if(context.map.tileExists(location) && isVisible()) {
        Coord realPos(lround(realX), lround(realY));
        context.game.addExplosion(Explosion_SmallUnit, realPos, owner->getHouseID());
    }

    parent::destroy(context);
}

} // namespace Dune::Engine
