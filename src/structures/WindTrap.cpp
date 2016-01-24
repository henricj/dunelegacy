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

WindTrap::WindTrap(House* newOwner) : StructureBase(newOwner) {
    WindTrap::init();

	setHealth(getMaxHealth());
}

WindTrap::WindTrap(InputStream& stream) : StructureBase(stream) {
    WindTrap::init();
}

void WindTrap::init() {
	itemID = Structure_WindTrap;
	owner->incrementStructures(itemID);

	structureSize.x = 2;
	structureSize.y = 2;

	graphicID = ObjPic_Windtrap;
	graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
	numImagesX = 4;
	numImagesY = 1;
	firstAnimFrame = 2;
	lastAnimFrame = 3;
}

WindTrap::~WindTrap() {
}

ObjectInterface* WindTrap::getInterfaceContainer() {
	if((pLocalHouse == owner) || (debug == true)) {
		return WindTrapInterface::create(objectID);
	} else {
		return DefaultObjectInterface::create(objectID);
	}
}

void WindTrap::setHealth(FixPoint newHealth) {
    int producedPowerBefore = getProducedPower();
    StructureBase::setHealth(newHealth);
    int producedPowerAfterwards = getProducedPower();

    owner->setProducedPower(owner->getProducedPower() - producedPowerBefore + producedPowerAfterwards);
}

int WindTrap::getProducedPower() const {
    int windTrapProducedPower = abs(currentGame->objectData.data[Structure_WindTrap][originalHouseID].power);

    FixPoint ratio = getHealth() / getMaxHealth();
    return lround(ratio * windTrapProducedPower);
}
