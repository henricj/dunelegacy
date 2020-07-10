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

#include <structures/LightFactory.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>

LightFactory::LightFactory(ItemID_enum itemID, Uint32 objectID, const ObjectInitializer& initializer) : BuilderBase(itemID, objectID, initializer) {
    LightFactory::init();

    setHealth(getMaxHealth());
}

LightFactory::LightFactory(ItemID_enum itemID, Uint32 objectID, const ObjectStreamInitializer& initializer) : BuilderBase(itemID, objectID, initializer) {
    LightFactory::init();
}

void LightFactory::init() {
    assert(itemID == Structure_LightFactory);
    owner->incrementStructures(itemID);

    structureSize.x = 2;
    structureSize.y = 2;

    graphicID = ObjPic_LightFactory;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    numImagesX = 6;
    numImagesY = 1;
    firstAnimFrame = 2;
    lastAnimFrame = 3;
}

LightFactory::~LightFactory() = default;

void LightFactory::updateStructureSpecificStuff(const GameContext& context) {
    if(deployTimer > 0) {
        firstAnimFrame = 4;
        lastAnimFrame = 5;
    } else {
        firstAnimFrame = 2;
        lastAnimFrame = 3;
    }
}
