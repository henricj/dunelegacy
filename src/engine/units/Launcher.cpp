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

#include <units/Launcher.h>

#include <House.h>
#include <Game.h>
#include <Map.h>

namespace {
using namespace Dune::Engine;

constexpr TrackedUnitConstants launcher_constants{Launcher::item_id, 2, Bullet_Rocket};
}

namespace Dune::Engine {

Launcher::Launcher(uint32_t objectID, const ObjectInitializer& initializer)
    : TrackedUnit(launcher_constants, objectID, initializer) {
    Launcher::init();

    Launcher::setHealth(initializer.game(), getMaxHealth(initializer.game()));
}

Launcher::Launcher(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : TrackedUnit(launcher_constants, objectID, initializer) {
    Launcher::init();
}

void Launcher::init() {
    assert(itemID == Unit_Launcher);
    owner->incrementUnits(itemID);
}

Launcher::~Launcher() = default;

void Launcher::destroy(const GameContext& context) {
    if(context.map.tileExists(location) && isVisible()) {
        const Coord realPos(lround(realX), lround(realY));
        const auto  explosionID =
            context.game.randomGen.getRandOf(Explosion_Medium1, Explosion_Medium2, Explosion_Flames);
        context.game.addExplosion(explosionID, realPos, owner->getHouseID());
    }

    parent::destroy(context);
}

bool Launcher::canAttack(const GameContext& context, const ObjectBase* object) const {
    return ((object != nullptr) &&
            ((object->getOwner()->getTeamID() != owner->getTeamID()) || object->getItemID() == Unit_Sandworm) &&
            object->isVisible(getOwner()->getTeamID()));
}

} // namespace Dune::Engine
