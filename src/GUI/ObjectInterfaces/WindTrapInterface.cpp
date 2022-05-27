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

#include "GUI/ObjectInterfaces/WindTrapInterface.h"

#include "GUI/Spacer.h"

#include <FileClasses/TextManager.h>

#include <House.h>

WindTrapInterface::WindTrapInterface(const GameContext& context, int objectID)
    : DefaultStructureInterface(context, objectID) {
    const auto house_id = static_cast<int>(dune::globals::pLocalHouse->getHouseID());
    const Uint32 color  = SDL2RGB(dune::globals::palette[dune::globals::houseToPaletteIndex[house_id] + 3]);

    mainHBox.addWidget(&textVBox);

    requiredEnergyLabel.setTextFontSize(12);
    requiredEnergyLabel.setTextColor(color);
    textVBox.addWidget(&requiredEnergyLabel, 0.005);
    producedEnergyLabel.setTextFontSize(12);
    producedEnergyLabel.setTextColor(color);
    textVBox.addWidget(&producedEnergyLabel, 0.005);
    textVBox.addWidget(Widget::create<Spacer>().release(), 0.99);
}

bool WindTrapInterface::update() {
    const auto* const pObject = context_.objectManager.getObject(objectID);
    if (pObject == nullptr)
        return false;

    const House* pOwner = pObject->getOwner();

    requiredEnergyLabel.setText(" " + _("Required") + ": " + std::to_string(pOwner->getPowerRequirement()));
    producedEnergyLabel.setText(" " + _("Produced") + ": " + std::to_string(pOwner->getProducedPower()));

    return parent::update();
}
