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

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>

#include <GUI/ObjectInterfaces/RefineryAndSiloInterface.h>

namespace {
constexpr StructureBaseConstants silo_constants{Silo::item_id, Coord{2, 2}};
}

Silo::Silo(uint32_t objectID, const ObjectInitializer& initializer)
    : StructureBase(silo_constants, objectID, initializer) {
    Silo::init();

    Silo::setHealth(getMaxHealth());
}

Silo::Silo(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : StructureBase(silo_constants, objectID, initializer) {
    Silo::init();
}

void Silo::init() {
    assert(itemID_ == Structure_Silo);
    owner_->incrementStructures(itemID_);

    graphicID_     = ObjPic_Silo;
    graphic_       = dune::globals::pGFXManager->getObjPic(graphicID_, getOwner()->getHouseID());
    numImagesX_    = 4;
    numImagesY_    = 1;
    firstAnimFrame = 2;
    lastAnimFrame  = 3;
}

Silo::~Silo() = default;

std::unique_ptr<ObjectInterface> Silo::getInterfaceContainer(const GameContext& context) {
    if ((dune::globals::pLocalHouse == owner_) || (dune::globals::debug)) {
        return Widget::create<RefineryAndSiloInterface>(context, objectID_);
    }
    return Widget::create<DefaultObjectInterface>(context, objectID_);
}
