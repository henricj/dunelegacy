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

#ifndef REPAIRYARDINTERFACE_H
#define REPAIRYARDINTERFACE_H

#include "DefaultStructureInterface.h"

#include <GUI/ProgressBar.h>

#include <units/UnitBase.h>

#include <structures/RepairYard.h>

class RepairYardInterface : public DefaultStructureInterface {
public:
    static RepairYardInterface* create(int objectID) {
        RepairYardInterface* tmp = new RepairYardInterface(objectID);
        tmp->pAllocated = true;
        return tmp;
    }

protected:
    explicit RepairYardInterface(int objectID) : DefaultStructureInterface(objectID) {
        mainHBox.addWidget(Spacer::create());
        mainHBox.addWidget(&repairUnitProgressBar);
        mainHBox.addWidget(Spacer::create());
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

        RepairYard* pRepairYard = dynamic_cast<RepairYard*>(pObject);
        if(pRepairYard != nullptr) {
            UnitBase* pUnit = pRepairYard->getRepairUnit();

            if(pUnit != nullptr) {
                repairUnitProgressBar.setVisible(true);
                repairUnitProgressBar.setTexture(resolveItemPicture(pUnit->getItemID()));
                repairUnitProgressBar.setProgress( ((pUnit->getHealth()*100)/pUnit->getMaxHealth()).toDouble());
            } else {
                repairUnitProgressBar.setVisible(false);
            }
        }

        return DefaultStructureInterface::update();
    }

private:
    PictureProgressBar  repairUnitProgressBar;
};

#endif // REPAIRYARDINTERFACE_H
