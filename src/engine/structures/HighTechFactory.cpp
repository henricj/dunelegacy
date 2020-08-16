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

#include <structures/HighTechFactory.h>

#include <House.h>
#include <Game.h>

namespace {
using namespace Dune::Engine;

constexpr BuilderBaseConstants high_tech_factory_constants{HighTechFactory::item_id, Coord{3, 2}};
}

namespace Dune::Engine {

HighTechFactory::HighTechFactory(uint32_t objectID, const ObjectInitializer& initializer)
    : BuilderBase(high_tech_factory_constants, objectID, initializer) {
    HighTechFactory::init();

    HighTechFactory::setHealth(initializer.game(), getMaxHealth(initializer.game()));
}

HighTechFactory::HighTechFactory(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : BuilderBase(high_tech_factory_constants, objectID, initializer) {
    HighTechFactory::init();
}

void HighTechFactory::init() {
    owner->incrementStructures(itemID);
}

HighTechFactory::~HighTechFactory() = default;

} // namespace Dune::Engine
