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

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>

HighTechFactory::HighTechFactory(House* newOwner) : BuilderBase(newOwner) {
    HighTechFactory::init();

    setHealth(getMaxHealth());
}

HighTechFactory::HighTechFactory(InputStream& stream) : BuilderBase(stream) {
    HighTechFactory::init();
}

void HighTechFactory::init() {
    itemID = Structure_HighTechFactory;
    owner->incrementStructures(itemID);

    structureSize.x = 3;
    structureSize.y = 2;

    graphicID = ObjPic_HighTechFactory;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    numImagesX = 8;
    numImagesY = 1;
    firstAnimFrame = 2;
    lastAnimFrame = 3;
}

HighTechFactory::~HighTechFactory() = default;
