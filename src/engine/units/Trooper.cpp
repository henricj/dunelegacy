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

#include <units/Trooper.h>

#include <House.h>

namespace {
using namespace Dune::Engine;

constexpr InfantryBaseConstants trooper_constants{Trooper::item_id, 1, Bullet_SmallRocket};
}

namespace Dune::Engine {

Trooper::Trooper(uint32_t objectID, const ObjectInitializer& initializer)
    : InfantryBase(trooper_constants, objectID, initializer) {
    Trooper::init();

    Trooper::setHealth(initializer.game(), getMaxHealth(initializer.game()));
}

Trooper::Trooper(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : InfantryBase(trooper_constants, objectID, initializer) {
    Trooper::init();
}

void Trooper::init() {
    assert(itemID == Unit_Trooper);
    owner->incrementUnits(itemID);
}

Trooper::~Trooper() = default;

bool Trooper::canAttack(const GameContext& context, const ObjectBase* object) const {
    return (object != nullptr)
           && ((object->getOwner()->getTeamID() != owner->getTeamID()) || (object->getItemID() == Unit_Sandworm))
           && object->isVisible(getOwner()->getTeamID());
}

} // namespace Dune::Engine
