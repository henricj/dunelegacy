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

#ifndef OBJECTINTERFACE_H
#define OBJECTINTERFACE_H

#include <GUI/StaticContainer.h>



class ObjectInterface : public StaticContainer {
public:
        ObjectInterface() : StaticContainer() { ; };

        virtual ~ObjectInterface() { ; };

        /**
            This method updates the object interface.
            If the object doesn't exists anymore then update returns false.
            \return true = everything ok, false = the object container should be removed
        */
        virtual bool update() = 0;
};

#endif //OBJECTINTERFACE_H
