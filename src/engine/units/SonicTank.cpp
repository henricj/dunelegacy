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

#include <units/SonicTank.h>

#include <House.h>
#include <Game.h>
#include <Map.h>

namespace {
using namespace Dune::Engine;

constexpr TrackedUnitConstants sonic_tank_constants{SonicTank::item_id, 1, Bullet_Sonic};
}

namespace Dune::Engine {

SonicTank::SonicTank(uint32_t objectID, const ObjectInitializer& initializer)
    : TrackedUnit(sonic_tank_constants, objectID, initializer) {
    SonicTank::init();

    SonicTank::setHealth(initializer.game(), getMaxHealth(initializer.game()));
}

SonicTank::SonicTank(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : TrackedUnit(sonic_tank_constants, objectID, initializer) {
    SonicTank::init();
}

void SonicTank::init() {
    assert(itemID == Unit_SonicTank);
    owner->incrementUnits(itemID);
}

SonicTank::~SonicTank() = default;

void SonicTank::destroy(const GameContext& context) {
    if(context.map.tileExists(location) && isVisible()) {
        const Coord realPos(lround(realX), lround(realY));
        context.game.addExplosion(Explosion_SmallUnit, realPos, owner->getHouseID());
    }

    parent::destroy(context);
}

void SonicTank::handleDamage(const GameContext& context, int damage, uint32_t damagerID, House* damagerOwner) {
    auto* const damager = context.objectManager.getObject(damagerID);

    if(!damager || (damager->getItemID() != Unit_SonicTank))
        parent::handleDamage(context, damage, damagerID, damagerOwner);
}

bool SonicTank::canAttack(const GameContext& context, const ObjectBase* object) const {
    return ((object != nullptr) && parent::canAttack(context, object) && (object->getItemID() != Unit_SonicTank));
}

} // namespace Dune::Engine
