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

#ifndef RADARINTERFACE_H
#define RADARINTERFACE_H

#include "DefaultStructureInterface.h"

#include <FileClasses/FontManager.h>
#include <FileClasses/TextManager.h>

#include <House.h>

#include <GUI/Label.h>
#include <GUI/VBox.h>

#include <misc/string_util.h>

class RadarInterface : public DefaultStructureInterface {
public:
    static RadarInterface* create(int objectID) {
        RadarInterface* tmp = new RadarInterface(objectID);
        tmp->pAllocated = true;
        return tmp;
    }

protected:
    explicit RadarInterface(int objectID) : DefaultStructureInterface(objectID) {
        Uint32 color = SDL2RGB(palette[houseToPaletteIndex[pLocalHouse->getHouseID()]+3]);

        mainHBox.addWidget(&textVBox);

        friendlyUnitsLabel.setTextFontSize(12);
        friendlyUnitsLabel.setTextColor(color);
        textVBox.addWidget(&friendlyUnitsLabel, 0.005);
        enemyUnitsLabel.setTextFontSize(12);
        enemyUnitsLabel.setTextColor(color);
        textVBox.addWidget(&enemyUnitsLabel, 0.005);
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

        friendlyUnitsLabel.setText(" " + _("Friend") + ": " + std::to_string(pOwner->getNumVisibleFriendlyUnits()));
        enemyUnitsLabel.setText(" " + _("Enemy") + ": " + std::to_string(pOwner->getNumVisibleEnemyUnits()));

        return DefaultStructureInterface::update();
    }

private:
    VBox    textVBox;

    Label   friendlyUnitsLabel;
    Label   enemyUnitsLabel;
};

#endif // RADARINTERFACE_H
