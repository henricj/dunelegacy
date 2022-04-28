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

std::unique_ptr<RepairYardInterface> RepairYardInterface::create(const GameContext& context, int objectID) {
    auto tmp        = std::unique_ptr<RepairYardInterface>{new RepairYardInterface{context, objectID}};
    tmp->pAllocated = true;
    return tmp;
}

RepairYardInterface::RepairYardInterface(const GameContext& context, int objectID)
    : DefaultStructureInterface(context, objectID) {
    mainHBox.addWidget(Spacer::create());
    mainHBox.addWidget(&repairUnitProgressBar);
    mainHBox.addWidget(Spacer::create());
}

bool RepairYardInterface::update() {
    auto* pObject = dune::globals::currentGame->getObjectManager().getObject(objectID);
    if (pObject == nullptr) {
        return false;
    }

    auto* const pRepairYard = dune_cast<RepairYard>(pObject);
    if (pRepairYard != nullptr) {

        if (const auto* pUnit = pRepairYard->getRepairUnit()) {
            repairUnitProgressBar.setVisible(true);
            repairUnitProgressBar.setTexture(resolveItemPicture(pUnit->getItemID()));
            repairUnitProgressBar.setProgress(((pUnit->getHealth() * 100) / pUnit->getMaxHealth()).toDouble());
        } else {
            repairUnitProgressBar.setVisible(false);
        }
    }

    return DefaultStructureInterface::update();
}
