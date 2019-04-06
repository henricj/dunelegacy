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

#ifndef REFINERYANDSILOINTERFACE_H
#define REFINERYANDSILOINTERFACE_H

#include "DefaultStructureInterface.h"

#include <FileClasses/FontManager.h>
#include <FileClasses/TextManager.h>

#include <House.h>

#include <GUI/Label.h>
#include <GUI/VBox.h>

#include <misc/string_util.h>

class RefineryAndSiloInterface : public DefaultStructureInterface {
public:
    static RefineryAndSiloInterface* create(int objectID) {
        RefineryAndSiloInterface* tmp = new RefineryAndSiloInterface(objectID);
        tmp->pAllocated = true;
        return tmp;
    }

protected:
    explicit RefineryAndSiloInterface(int objectID) : DefaultStructureInterface(objectID) {
        Uint32 color = SDL2RGB(palette[houseToPaletteIndex[pLocalHouse->getHouseID()]+3]);

        mainHBox.addWidget(&textVBox);

        capacityLabel.setTextFontSize(12);
        capacityLabel.setTextColor(color);
        textVBox.addWidget(&capacityLabel, 0.005);
        storedCreditsLabel.setTextFontSize(12);
        storedCreditsLabel.setTextColor(color);
        textVBox.addWidget(&storedCreditsLabel, 0.005);
        textVBox.addWidget(Spacer::create(), 0.99);
    }

    /**
        This method updates the object interface.
        If the object doesn't exists anymore then update returns false.
        \return true = everything ok, false = the object container should be removed
    */
    bool update() override
    {
        ObjectBase* pObject = currentGame->getObjectManager().getObject(objectID);
        if(pObject == nullptr) {
            return false;
        }

        House* pOwner = pObject->getOwner();

        capacityLabel.setText(" " + _("Capacity") + ": " + std::to_string(pOwner->getCapacity()));
        storedCreditsLabel.setText(" " + _("Stored") + ": " + std::to_string(lround(pOwner->getStoredCredits())));

        return DefaultStructureInterface::update();
    }

private:
    VBox    textVBox;

    Label   capacityLabel;
    Label   storedCreditsLabel;
};

#endif // REFINERYANDSILOINTERFACE_H
