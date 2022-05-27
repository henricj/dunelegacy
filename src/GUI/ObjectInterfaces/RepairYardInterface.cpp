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

#include "GUI/ObjectInterfaces/RepairYardInterface.h"

#include "GUI/Spacer.h"

#include <units/UnitBase.h>

#include <structures/RepairYard.h>

RepairYardInterface::RepairYardInterface(const GameContext& context, int objectID)
    : DefaultStructureInterface(context, objectID) {

    repairUnitProgressBar.setColor(COLOR_HALF_TRANSPARENT);

    mainHBox.addWidget(Widget::create<Spacer>().release());
    mainHBox.addWidget(&repairUnitProgressBar);
    mainHBox.addWidget(Widget::create<Spacer>().release());
}

bool RepairYardInterface::update() {
    auto* const pRepairYard = context_.objectManager.getObject<RepairYard>(objectID);
    if (pRepairYard == nullptr)
        return false;

    if (const auto* pUnit = pRepairYard->getRepairUnit()) {
        repairUnitProgressBar.setVisible(true);
        repairUnitProgressBar.setTexture(resolveItemPicture(pUnit->getItemID()));
        repairUnitProgressBar.setProgress(((pUnit->getHealth() * 100) / pUnit->getMaxHealth()).toFloat());
    } else {
        repairUnitProgressBar.setVisible(false);
    }

    return parent::update();
}
