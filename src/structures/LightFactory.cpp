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
#include <Game.h>
#include <House.h>

namespace {
constexpr BuilderBaseConstants light_factory_constants{LightFactory::item_id, Coord{2, 2}};
}

LightFactory::LightFactory(uint32_t objectID, const ObjectInitializer& initializer)
    : BuilderBase(light_factory_constants, objectID, initializer) {
    LightFactory::init();

    LightFactory::setHealth(getMaxHealth());
}

LightFactory::LightFactory(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : BuilderBase(light_factory_constants, objectID, initializer) {
    LightFactory::init();
}

void LightFactory::init() {
    assert(itemID_ == Structure_LightFactory);
    owner_->incrementStructures(itemID_);

    graphicID_     = ObjPic_LightFactory;
    graphic_       = dune::globals::pGFXManager->getObjPic(graphicID_, getOwner()->getHouseID());
    numImagesX_    = 6;
    numImagesY_    = 1;
    firstAnimFrame = 2;
    lastAnimFrame  = 3;
}

LightFactory::~LightFactory() = default;

void LightFactory::updateStructureSpecificStuff([[maybe_unused]] const GameContext& context) {
    if (deployTimer_ > 0) {
        firstAnimFrame = 4;
        lastAnimFrame  = 5;
    } else {
        firstAnimFrame = 2;
        lastAnimFrame  = 3;
    }
}
