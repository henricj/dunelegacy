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

#ifndef DEFAULTSTRUCTUREINTERFACE_H
#define DEFAULTSTRUCTUREINTERFACE_H

#include "DefaultObjectInterface.h"

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>

#include <structures/StructureBase.h>

class DefaultStructureInterface : public DefaultObjectInterface {
public:
    static std::unique_ptr<DefaultStructureInterface> create(const GameContext& context, int objectID) {
        auto tmp = std::unique_ptr<DefaultStructureInterface> {new DefaultStructureInterface {context, objectID}};
        tmp->pAllocated = true;
        return tmp;
    }

protected:
    DefaultStructureInterface(const GameContext& context, int objectID) : DefaultObjectInterface(context, objectID) {
        const auto* const pUIRepair        = pGFXManager->getUIGraphic(UI_Repair);
        const auto* const pUIRepairPressed = pGFXManager->getUIGraphic(UI_Repair_Pressed);

        repairButton.setTextures(pUIRepair, pUIRepairPressed);
        repairButton.setToggleButton(true);
        repairButton.setVisible(false);
        repairButton.setTooltipText(_("Repair this structure (Hotkey: R)"));
        repairButton.setOnClick([&] { OnRepair(); });

        topBox.addWidget(&repairButton, Point(2, 2), getTextureSize(pUIRepair));
    }

    void OnRepair() {
        auto* pStructure = context_.objectManager.getObject<StructureBase>(objectID);
        if (pStructure != nullptr) {
            pStructure->handleRepairClick();
        }
    }

    /**
        This method updates the object interface.
        If the object doesn't exists anymore then update returns false.
        \return true = everything ok, false = the object container should be removed
    */
    bool update() override {
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

    PictureButton repairButton;
};

#endif // DEFAULTSTRUCTUREINTERFACE_H
