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

#include <units/SiegeTank.h>

#include <House.h>
#include <Game.h>
#include <Map.h>

namespace {
using namespace Dune::Engine;

constexpr TankBaseConstants siege_tank_constants{SiegeTank::item_id, 2, Bullet_ShellLarge};
}

namespace Dune::Engine {

SiegeTank::SiegeTank(uint32_t objectID, const ObjectInitializer& initializer)
    : TankBase(siege_tank_constants, objectID, initializer) {
    SiegeTank::init();

    SiegeTank::setHealth(initializer.game(), getMaxHealth(initializer.game()));
}

SiegeTank::SiegeTank(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : TankBase(siege_tank_constants, objectID, initializer) {
    SiegeTank::init();
}

void SiegeTank::init() {
    assert(itemID == Unit_SiegeTank);
    owner->incrementUnits(itemID);
}

SiegeTank::~SiegeTank() = default;

void SiegeTank::destroy(const GameContext& context) {
    if(context.map.tileExists(location) && isVisible()) {
        const Coord realPos(lround(realX), lround(realY));
        const auto  explosionID = context.game.randomGen.getRandOf(Explosion_Medium1, Explosion_Medium2);
        context.game.addExplosion(explosionID, realPos, owner->getHouseID());

        if(isVisible(getOwner()->getTeamID())) { }
    }

    TankBase::destroy(context);
}

} // namespace Dune::Engine
