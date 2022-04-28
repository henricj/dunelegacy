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

#include "GUI/ObjectInterfaces/RefineryAndSiloInterface.h"

#include "GUI/Spacer.h"

#include <FileClasses/TextManager.h>

#include <House.h>


std::unique_ptr<RefineryAndSiloInterface> RefineryAndSiloInterface::create(const GameContext& context, int objectID) {
    auto tmp        = std::unique_ptr<RefineryAndSiloInterface>{new RefineryAndSiloInterface{context, objectID}};
    tmp->pAllocated = true;
    return tmp;
}

RefineryAndSiloInterface::RefineryAndSiloInterface(const GameContext& context, int objectID): DefaultStructureInterface(context, objectID) {
    const Uint32 color = SDL2RGB(
        dune::globals::palette[houseToPaletteIndex[static_cast<int>(dune::globals::pLocalHouse->getHouseID())] + 3]);

    mainHBox.addWidget(&textVBox);

    capacityLabel.setTextFontSize(12);
    capacityLabel.setTextColor(color);
    textVBox.addWidget(&capacityLabel, 0.005);
    storedCreditsLabel.setTextFontSize(12);
    storedCreditsLabel.setTextColor(color);
    textVBox.addWidget(&storedCreditsLabel, 0.005);
    textVBox.addWidget(Spacer::create(), 0.99);
}

bool RefineryAndSiloInterface::update() {
    auto* pObject = dune::globals::currentGame->getObjectManager().getObject(objectID);
    if (pObject == nullptr) {
        return false;
    }

    const House* pOwner = pObject->getOwner();

    capacityLabel.setText(" " + _("Capacity") + ": " + std::to_string(pOwner->getCapacity()));
    storedCreditsLabel.setText(" " + _("Stored") + ": " + std::to_string(lround(pOwner->getStoredCredits())));

    return DefaultStructureInterface::update();
}
