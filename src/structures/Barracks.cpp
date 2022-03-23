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

#include <structures/Barracks.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>

namespace {
const BuilderBaseConstants barracks_constants {Barracks::item_id, Coord {2, 2}};
}

Barracks::Barracks(uint32_t objectID, const ObjectInitializer& initializer)
    : BuilderBase(barracks_constants, objectID, initializer) {
    Barracks::init();

    setHealth(getMaxHealth());
}

Barracks::Barracks(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : BuilderBase(barracks_constants, objectID, initializer) {
    Barracks::init();
}

void Barracks::init() {
    owner->incrementStructures(itemID);

    graphicID = ObjPic_Barracks, graphic = pGFXManager->getObjPic(graphicID, getOwner()->getHouseID());
    numImagesX     = 4;
    numImagesY     = 1;
    firstAnimFrame = 2;
    lastAnimFrame  = 3;
}

Barracks::~Barracks() = default;
