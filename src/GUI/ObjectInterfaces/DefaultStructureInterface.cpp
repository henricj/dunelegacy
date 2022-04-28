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

#include "GUI/ObjectInterfaces/DefaultStructureInterface.h"

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>

#include <structures/StructureBase.h>

std::unique_ptr<DefaultStructureInterface> DefaultStructureInterface::create(const GameContext& context, int objectID) {
    auto tmp        = std::unique_ptr<DefaultStructureInterface>{new DefaultStructureInterface{context, objectID}};
    tmp->pAllocated = true;
    return tmp;
}

DefaultStructureInterface::DefaultStructureInterface(const GameContext& context, int objectID)
    : DefaultObjectInterface(context, objectID) {
    auto* const gfx = dune::globals::pGFXManager.get();

    const auto* const pUIRepair        = gfx->getUIGraphic(UI_Repair);
    const auto* const pUIRepairPressed = gfx->getUIGraphic(UI_Repair_Pressed);

    repairButton.setTextures(pUIRepair, pUIRepairPressed);
    repairButton.setToggleButton(true);
    repairButton.setVisible(false);
    repairButton.setTooltipText(_("Repair this structure (Hotkey: R)"));
    repairButton.setOnClick([&] { OnRepair(); });

    topBox.addWidget(&repairButton, Point(2, 2), getTextureSize(pUIRepair));
}

void DefaultStructureInterface::OnRepair() {
    auto* pStructure = context_.objectManager.getObject<StructureBase>(objectID);
    if (pStructure != nullptr) {
        pStructure->handleRepairClick();
    }
}

bool DefaultStructureInterface::update() {
    auto* pStructure = context_.objectManager.getObject<StructureBase>(objectID);

    if (pStructure != nullptr) {
        if (pStructure->getHealth() >= pStructure->getMaxHealth()) {
            repairButton.setVisible(false);
        } else {
            repairButton.setVisible(true);
            repairButton.setToggleState(pStructure->isRepairing());
        }
    }

    return true;
}
