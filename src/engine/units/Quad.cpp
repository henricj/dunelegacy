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

#include <House.h>
#include <Game.h>
#include <Map.h>
#include <Explosion.h>

namespace {
using namespace Dune::Engine;

constexpr GroundUnitConstants quad_constants{Quad::item_id, 2, Bullet_ShellSmall};
}

namespace Dune::Engine {

Quad::Quad(uint32_t objectID, const ObjectInitializer& initializer)
    : GroundUnit(quad_constants, objectID, initializer) {
    Quad::init();

    Quad::setHealth(initializer.game(), getMaxHealth(initializer.game()));
}

Quad::Quad(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : GroundUnit(quad_constants, objectID, initializer) {
    Quad::init();
}

void Quad::init() {
    assert(itemID == Unit_Quad);
    owner->incrementUnits(itemID);
}

Quad::~Quad() = default;

void Quad::destroy(const GameContext& context) {
    if(context.map.tileExists(location) && isVisible()) {
        Coord realPos(lround(realX), lround(realY));
        context.game.addExplosion(Explosion_SmallUnit, realPos, owner->getHouseID());
    }

    parent::destroy(context);
}

} // namespace Dune::Engine
