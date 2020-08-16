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

#include <units/Soldier.h>

#include <House.h>

namespace {
using namespace Dune::Engine;

constexpr InfantryBaseConstants soldier_constants{Soldier::item_id, 1, Bullet_ShellSmall};
}

namespace Dune::Engine {

Soldier::Soldier(uint32_t objectID, const ObjectInitializer& initializer)
    : InfantryBase(soldier_constants, objectID, initializer) {
    Soldier::init();

    Soldier::setHealth(initializer.game(), getMaxHealth(initializer.game()));
}

Soldier::Soldier(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : InfantryBase(soldier_constants, objectID, initializer) {
    Soldier::init();
}

void Soldier::init() {
    assert(itemID == Unit_Soldier);
    owner->incrementUnits(itemID);
}

Soldier::~Soldier() = default;

bool Soldier::canAttack(const GameContext& context, const ObjectBase* object) const {
    return (object != nullptr)
           && (object->isAStructure() || !object->isAFlyingUnit())
           && ((object->getOwner()->getTeamID() != owner->getTeamID()) || object->getItemID() == Unit_Sandworm)
           && object->isVisible(getOwner()->getTeamID());
}

} // namespace Dune::Engine
