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

#include <structures/IX.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>

namespace {
constexpr StructureBaseConstants ix_constants{IX::item_id, Coord{2, 2}};
}

IX::IX(uint32_t objectID, const ObjectInitializer& initializer) : StructureBase(ix_constants, objectID, initializer) {
    IX::init();

    IX::setHealth(getMaxHealth());
}

IX::IX(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : StructureBase(ix_constants, objectID, initializer) {
    IX::init();
}

void IX::init() {
    assert(itemID == Structure_IX);
    owner->incrementStructures(itemID);

    graphicID      = ObjPic_IX;
    graphic        = dune::globals::pGFXManager->getObjPic(graphicID, getOwner()->getHouseID());
    numImagesX     = 4;
    numImagesY     = 1;
    firstAnimFrame = 2;
    lastAnimFrame  = 3;
}

IX::~IX() = default;
