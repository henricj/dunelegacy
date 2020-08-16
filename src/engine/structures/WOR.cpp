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

#include <structures/WOR.h>

#include <House.h>

namespace {
using namespace Dune::Engine;

constexpr BuilderBaseConstants wor_constants{WOR::item_id, Coord{2, 2}};
}

namespace Dune::Engine {

WOR::WOR(uint32_t objectID, const ObjectInitializer& initializer) : BuilderBase(wor_constants, objectID, initializer) {
    WOR::init();

    WOR::setHealth(initializer.game(), getMaxHealth(initializer.game()));
}

WOR::WOR(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : BuilderBase(wor_constants, objectID, initializer) {
    WOR::init();
}

void WOR::init() { owner->incrementStructures(itemID); }

WOR::~WOR() = default;

} // namespace Dune::Engine
