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

#ifndef DEFAULTOBJECTINTERFACE_H
#define DEFAULTOBJECTINTERFACE_H

#include "ObjectInterface.h"

#include <GUI/PictureLabel.h>

#include <globals.h>

#include <Game.h>

#include <sand.h>

#include <misc/exceptions.h>

#include <ObjectBase.h>
#include <House.h>
#include <units/UnitBase.h>


class DefaultObjectInterface : public ObjectInterface {
public:
    static DefaultObjectInterface* create(int objectID) {
        DefaultObjectInterface* tmp = new DefaultObjectInterface(objectID);
        tmp->pAllocated = true;
        return tmp;
    }

protected:
    explicit DefaultObjectInterface(int objectID) : ObjectInterface() {
        ObjectBase* pObject = currentGame->getObjectManager().getObject(objectID);
        if(pObject == nullptr) {
            THROW(std::invalid_argument, "Failed to resolve ObjectID %d!", objectID);
        }

        this->objectID = objectID;
        itemID = pObject->getItemID();

        addWidget(&topBox,Point(0,0),Point(SIDEBARWIDTH - 25,80));

        addWidget(&mainHBox,Point(0,80),Point(SIDEBARWIDTH - 25,getRendererHeight() - 80 - 148));

        topBox.addWidget(&topBoxHBox,Point(0,22),Point(SIDEBARWIDTH - 25,58));

        topBoxHBox.addWidget(Spacer::create());
        topBoxHBox.addWidget(&objPicture);

        objPicture.setTexture(resolveItemPicture(itemID, (HOUSETYPE) pObject->getOriginalHouseID()));

        topBoxHBox.addWidget(Spacer::create());
    };

    virtual ~DefaultObjectInterface() { ; };

    /**
        This method updates the object interface.
        If the object doesn't exists anymore then update returns false.
        \return true = everything ok, false = the object container should be removed
    */
    bool update() override
    {
        ObjectBase* pObject = currentGame->getObjectManager().getObject(objectID);
        return (pObject != nullptr);
    }

    int             objectID;
    int             itemID;

    StaticContainer topBox;
    HBox            topBoxHBox;
    HBox            mainHBox;
    PictureLabel    objPicture;
};

#endif // DEFAULTOBJECTINTERFACE_H
