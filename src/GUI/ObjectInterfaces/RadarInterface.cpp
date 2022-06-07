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

#include "GUI/ObjectInterfaces/RadarInterface.h"

#include "GUI/Spacer.h"

#include <FileClasses/TextManager.h>

#include <House.h>

#include <fmt/core.h>

RadarInterface::RadarInterface(const GameContext& context, int objectID)
    : DefaultStructureInterface(context, objectID) {
    const auto house_id = static_cast<int>(dune::globals::pLocalHouse->getHouseID());
    const auto color    = SDL2RGB(dune::globals::palette[dune::globals::houseToPaletteIndex[house_id] + 3]);

    mainHBox.addWidget(&textVBox);

    friendlyUnitsLabel.setTextFontSize(12);
    friendlyUnitsLabel.setTextColor(color);
    textVBox.addWidget(&friendlyUnitsLabel, 0.005);
    enemyUnitsLabel.setTextFontSize(12);
    enemyUnitsLabel.setTextColor(color);
    textVBox.addWidget(&enemyUnitsLabel, 0.005);
    textVBox.addWidget(Widget::create<Spacer>().release(), 0.99);
}

RadarInterface::~RadarInterface() = default;

bool RadarInterface::update() {
    const auto* pObject = context_.objectManager.getObject(objectID);
    if (pObject == nullptr)
        return false;

    const auto* pOwner = pObject->getOwner();

    friendlyUnitsLabel.setText(fmt::format(" {}: {}", _("Friend"), pOwner->getNumVisibleFriendlyUnits()));
    enemyUnitsLabel.setText(fmt::format(" {}: {}", _("Enemy"), pOwner->getNumVisibleEnemyUnits()));

    return DefaultStructureInterface::update();
}
