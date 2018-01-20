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

#include <structures/HeavyFactory.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>

HeavyFactory::HeavyFactory(House* newOwner) : BuilderBase(newOwner) {
    HeavyFactory::init();

    setHealth(getMaxHealth());
}

HeavyFactory::HeavyFactory(InputStream& stream) : BuilderBase(stream) {
    HeavyFactory::init();
}

void HeavyFactory::init() {
    itemID = Structure_HeavyFactory;
    owner->incrementStructures(itemID);

    structureSize.x = 3;
    structureSize.y = 2;

    graphicID = ObjPic_HeavyFactory;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    numImagesX = 8;
    numImagesY = 1;
    firstAnimFrame = 2;
    lastAnimFrame = 3;
}

HeavyFactory::~HeavyFactory() = default;

void HeavyFactory::doBuildRandom() {
    if(isAllowedToUpgrade() && (getUpgradeCost() <= owner->getCredits())) {
        doUpgrade();
        return;
    }

    if(!buildList.empty()) {
        int item2Produce = ItemID_Invalid;

        do {
            item2Produce = std::next(buildList.begin(), currentGame->randomGen.rand(0, static_cast<Sint32>(buildList.size())-1))->itemID;
        } while((item2Produce == Unit_Harvester) || (item2Produce == Unit_MCV));

        doProduceItem(item2Produce);
    }
}

void HeavyFactory::updateStructureSpecificStuff() {
    if(deployTimer > 0) {
        firstAnimFrame = 4;
        lastAnimFrame = 5;
    } else {
        firstAnimFrame = 2;
        lastAnimFrame = 3;
    }
}
