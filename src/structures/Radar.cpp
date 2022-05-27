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

#include <structures/Radar.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>

#include <GUI/ObjectInterfaces/RadarInterface.h>

namespace {
constexpr StructureBaseConstants radar_constants{Radar::item_id, Coord{2, 2}};
}

Radar::Radar(uint32_t objectID, const ObjectInitializer& initializer)
    : StructureBase(radar_constants, objectID, initializer) {
    Radar::init();

    Radar::setHealth(getMaxHealth());
}

Radar::Radar(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : StructureBase(radar_constants, objectID, initializer) {
    Radar::init();
}

void Radar::init() {
    assert(itemID_ == Structure_Radar);
    owner_->incrementStructures(itemID_);

    graphicID_     = ObjPic_Radar;
    graphic_       = dune::globals::pGFXManager->getObjPic(graphicID_, getOwner()->getHouseID());
    numImagesX_    = 6;
    numImagesY_    = 1;
    firstAnimFrame = 2;
    lastAnimFrame  = 5;
}

Radar::~Radar() = default;

std::unique_ptr<ObjectInterface> Radar::getInterfaceContainer(const GameContext& context) {
    if ((dune::globals::pLocalHouse == owner_) || (dune::globals::debug)) {
        return RadarInterface::create(context, objectID_);
    }
    return DefaultObjectInterface::create(context, objectID_);
}

void Radar::destroy(const GameContext& context) {
    StructureBase::destroy(context);
}
