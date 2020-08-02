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

#include <structures/WindTrap.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>

#include <GUI/ObjectInterfaces/WindTrapInterface.h>

namespace {
constexpr StructureBaseConstants wind_trap_constants{WindTrap::item_id, Coord{2, 2}};
}

WindTrap::WindTrap(Uint32 objectID, const ObjectInitializer& initializer)
    : StructureBase(wind_trap_constants, objectID, initializer) {
    WindTrap::init();

    setHealth(getMaxHealth());
}

WindTrap::WindTrap(Uint32 objectID, const ObjectStreamInitializer& initializer)
    : StructureBase(wind_trap_constants, objectID, initializer) {
    WindTrap::init();
}

void WindTrap::init() {
    assert(itemID == Structure_WindTrap);
    owner->incrementStructures(itemID);

    graphicID = ObjPic_Windtrap;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    numImagesX = NUM_WINDTRAP_ANIMATIONS_PER_ROW;
    numImagesY = (2+NUM_WINDTRAP_ANIMATIONS+NUM_WINDTRAP_ANIMATIONS_PER_ROW-1)/NUM_WINDTRAP_ANIMATIONS_PER_ROW;
    firstAnimFrame = 2;
    lastAnimFrame = 2+NUM_WINDTRAP_ANIMATIONS-1;
}

WindTrap::~WindTrap() = default;

std::unique_ptr<ObjectInterface> WindTrap::getInterfaceContainer(const GameContext& context) {
    if((pLocalHouse == owner) || (debug)) { return WindTrapInterface::create(context, objectID); }
    return DefaultObjectInterface::create(context, objectID);
}

bool WindTrap::update(const GameContext& context) {
    bool bResult = StructureBase::update(context);

    if(bResult) {
        // we are still alive
        if(justPlacedTimer <= 0 || curAnimFrame != 0) {
            curAnimFrame = 2 + ((currentGame->getGameCycleCount()/8) % NUM_WINDTRAP_ANIMATIONS);
        }
    }

    return bResult;
}

void WindTrap::setHealth(FixPoint newHealth) {
    int producedPowerBefore = getProducedPower();
    StructureBase::setHealth(newHealth);
    int producedPowerAfterwards = getProducedPower();

    owner->setProducedPower(owner->getProducedPower() - producedPowerBefore + producedPowerAfterwards);
}

int WindTrap::getProducedPower() const {
    int windTrapProducedPower = abs(currentGame->objectData.data[Structure_WindTrap][static_cast<int>(originalHouseID)].power);

    FixPoint ratio = getHealth() / getMaxHealth();
    return lround(ratio * windTrapProducedPower);
}
