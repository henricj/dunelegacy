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
#include <Game.h>
#include <House.h>

namespace {
const BuilderBaseConstants heavy_factory_constants{HeavyFactory::item_id, Coord{3, 2}};
}

HeavyFactory::HeavyFactory(uint32_t objectID, const ObjectInitializer& initializer)
    : BuilderBase(heavy_factory_constants, objectID, initializer) {
    HeavyFactory::init();

    ObjectBase::setHealth(getMaxHealth());
}

HeavyFactory::HeavyFactory(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : BuilderBase(heavy_factory_constants, objectID, initializer) {
    HeavyFactory::init();
}

void HeavyFactory::init() {
    assert(itemID == Structure_HeavyFactory);
    owner->incrementStructures(itemID);

    graphicID      = ObjPic_HeavyFactory;
    graphic        = pGFXManager->getObjPic(graphicID, getOwner()->getHouseID());
    numImagesX     = 8;
    numImagesY     = 1;
    firstAnimFrame = 2;
    lastAnimFrame  = 3;
}

HeavyFactory::~HeavyFactory() = default;

void HeavyFactory::doBuildRandom(const GameContext& context) {
    if (isAllowedToUpgrade() && (getUpgradeCost(context) <= owner->getCredits())) {
        doUpgrade(context);
        return;
    }

    if (!buildList.empty()) {
        auto item2Produce = ItemID_Invalid;

        do {
            item2Produce =
                std::next(buildList.begin(), context.game.randomGen.rand(0, static_cast<int32_t>(buildList.size()) - 1))
                    ->itemID;
        } while ((item2Produce == Unit_Harvester) || (item2Produce == Unit_MCV));

        doProduceItem(item2Produce);
    }
}

void HeavyFactory::updateStructureSpecificStuff(const GameContext& context) {
    if (deployTimer > 0) {
        firstAnimFrame = 4;
        lastAnimFrame  = 5;
    } else {
        firstAnimFrame = 2;
        lastAnimFrame  = 3;
    }
}
