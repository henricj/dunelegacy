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

#include <structures/WindTrap.h>

#include <Game.h>
#include <House.h>

namespace {
using namespace Dune::Engine;

constexpr StructureBaseConstants wind_trap_constants{WindTrap::item_id, Coord{2, 2}};
}

namespace Dune::Engine {

WindTrap::WindTrap(uint32_t objectID, const ObjectInitializer& initializer)
    : StructureBase(wind_trap_constants, objectID, initializer) {
    WindTrap::init();

    WindTrap::setHealth(initializer.game(), getMaxHealth(initializer.game()));
}

WindTrap::WindTrap(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : StructureBase(wind_trap_constants, objectID, initializer) {
    WindTrap::init();
}

void WindTrap::init() {
    assert(itemID == Structure_WindTrap);
    owner->incrementStructures(itemID);
}

WindTrap::~WindTrap() = default;

bool WindTrap::update(const GameContext& context) {
    bool bResult = parent::update(context);

    if(bResult) {
        // we are still alive
    }

    return bResult;
}

void WindTrap::setHealth(const Game& game, FixPoint newHealth) {
    const auto producedPowerBefore = getProducedPower(game);
    parent::setHealth(game, newHealth);
    const auto producedPowerAfterwards = getProducedPower(game);

    owner->setProducedPower(owner->getProducedPower() - producedPowerBefore + producedPowerAfterwards);
}

int WindTrap::getProducedPower(const Game& game) const {
    const auto windTrapProducedPower = abs(game.getObjectData(Structure_WindTrap, originalHouseID).power);

    const auto ratio = getHealth() / getMaxHealth(game);

    return lround(ratio * windTrapProducedPower);
}

} // namespace Dune::Engine
