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

#include <structures/Silo.h>

#include <House.h>

namespace {
using namespace Dune::Engine;

constexpr StructureBaseConstants silo_constants{Silo::item_id, Coord{2, 2}};
}

namespace Dune::Engine {

Silo::Silo(uint32_t objectID, const ObjectInitializer& initializer)
    : StructureBase(silo_constants, objectID, initializer) {
    Silo::init();

    Silo::setHealth(initializer.game(), getMaxHealth(initializer.game()));
}

Silo::Silo(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : StructureBase(silo_constants, objectID, initializer) {
    Silo::init();
}

void Silo::init() {
    assert(itemID == Structure_Silo);
    owner->incrementStructures(itemID);
}

Silo::~Silo() = default;

} // namespace Dune::Engine
