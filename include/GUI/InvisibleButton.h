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

#ifndef INVISIBLEBUTTON_H
#define INVISIBLEBUTTON_H

#include "Button.h"

/// A class for a invisible button that can be clicked
class InvisibleButton : public Button {
public:
    /// Default contructor
    InvisibleButton() : Button() {
        enableResizing(true,true);
    }

    /// destructor
    virtual ~InvisibleButton() = default;

    /**
        This method draws nothing to screen. This method is called before drawOverlay().
        \param  position    Position to draw the button to
    */
    void draw(Point position) override
    {
        ;
    }

};

#endif // INVISIBLEBUTTON_H
