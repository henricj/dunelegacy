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
#include <misc/exceptions.h>

#include <Game.h>

#include <ObjectBase.h>

class DefaultObjectInterface : public ObjectInterface {
    using parent = ObjectInterface;

public:
    DefaultObjectInterface(const GameContext& context, int objectID);
    ~DefaultObjectInterface() override;

protected:
    /**
        This method updates the object interface.
        If the object doesn't exists anymore then update returns false.
        \return true = everything ok, false = the object container should be removed
    */
    bool update() override;

    int objectID;
    ItemID_enum itemID;

    StaticContainer topBox;
    HBox topBoxHBox;
    HBox mainHBox;
    PictureLabel objPicture;

    const GameContext context_;
};

#endif // DEFAULTOBJECTINTERFACE_H
