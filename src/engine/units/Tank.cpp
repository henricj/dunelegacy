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

#include <House.h>
#include <Game.h>
#include <Map.h>


namespace {
using namespace Dune::Engine;

constexpr TankBaseConstants tank_constants{Tank::item_id, 1, Bullet_ShellMedium};
}

namespace Dune::Engine {

Tank::Tank(uint32_t objectID, const ObjectInitializer& initializer) : TankBase(tank_constants, objectID, initializer) {
    Tank::init();

    Tank::setHealth(initializer.game(), getMaxHealth(initializer.game()));
}

Tank::Tank(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : TankBase(tank_constants, objectID, initializer) {
    Tank::init();
}

void Tank::init() {
    assert(itemID == Unit_Tank);
    owner->incrementUnits(itemID);
}

Tank::~Tank() = default;

void Tank::destroy(const GameContext& context) {
    if(context.map.tileExists(location) && isVisible()) {
        const Coord realPos(lround(realX), lround(realY));
        const auto  explosionID =
            context.game.randomGen.getRandOf(Explosion_Medium1, Explosion_Medium2, Explosion_Flames);
        context.game.addExplosion(explosionID, realPos, owner->getHouseID());
    }

    TankBase::destroy(context);
}

} // namespace Dune::Engine
