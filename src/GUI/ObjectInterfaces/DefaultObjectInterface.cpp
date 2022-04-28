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

#include "GUI/ObjectInterfaces/DefaultObjectInterface.h"

#include "GUI/Spacer.h"

std::unique_ptr<DefaultObjectInterface> DefaultObjectInterface::create(const GameContext& context, int objectID) {
    std::unique_ptr<DefaultObjectInterface> tmp{new DefaultObjectInterface(context, objectID)};
    tmp->pAllocated = true;
    return tmp;
}

DefaultObjectInterface::DefaultObjectInterface(const GameContext& context, int objectID) : context_{context} {
    const auto* const pObject = context_.objectManager.getObject(objectID);
    if (pObject == nullptr) {
        THROW(std::invalid_argument, "Failed to resolve ObjectID %d!", objectID);
    }

    this->objectID = objectID;
    itemID         = pObject->getItemID();

    DefaultObjectInterface::addWidget(&topBox, {0, 0}, {SIDEBARWIDTH - 25, 80});

    DefaultObjectInterface::addWidget(&mainHBox, {0, 80}, {SIDEBARWIDTH - 25, getRendererHeight() - 80 - 148});

    topBox.addWidget(&topBoxHBox, {0, 22}, {SIDEBARWIDTH - 25, 58});

    topBoxHBox.addWidget(Spacer::create());
    topBoxHBox.addWidget(&objPicture);

    objPicture.setTexture(resolveItemPicture(itemID, (HOUSETYPE)pObject->getOriginalHouseID()));

    topBoxHBox.addWidget(Spacer::create());
}

bool DefaultObjectInterface::update() {
    const auto* const pObject = context_.objectManager.getObject(objectID);
    return pObject != nullptr;
}
