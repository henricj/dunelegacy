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

#include <structures/ConstructionYard.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>

namespace {
const BuilderBaseConstants construction_yard_constants{ConstructionYard::item_id, Coord{2, 2}};
}

ConstructionYard::ConstructionYard(Uint32 objectID, const ObjectInitializer& initializer) : BuilderBase(construction_yard_constants, objectID, initializer) {
    ConstructionYard::init();

    setHealth(getMaxHealth());
}

ConstructionYard::ConstructionYard(Uint32 objectID, const ObjectStreamInitializer& initializer)
    : BuilderBase(construction_yard_constants, objectID, initializer) {
    ConstructionYard::init();
}

void ConstructionYard::init() {
    owner->incrementStructures(itemID);

    graphicID = ObjPic_ConstructionYard;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    numImagesX = 4;
    numImagesY = 1;

    firstAnimFrame = 2;
    lastAnimFrame = 3;
}

ConstructionYard::~ConstructionYard() = default;

bool ConstructionYard::doPlaceStructure(int x, int y) {
    if(isWaitingToPlace()) {
        return (getOwner()->placeStructure(getObjectID(), getCurrentProducedItem(), x, y) != nullptr);
    }         return false;

   
}
