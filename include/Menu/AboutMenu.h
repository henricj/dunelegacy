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

#ifndef ABOUTMENU_H
#define ABOUTMENU_H

#include "MenuBase.h"
#include <GUI/StaticContainer.h>
#include <GUI/Label.h>
#include <GUI/PictureLabel.h>

class AboutMenu : public MenuBase
{
public:
    AboutMenu();
    virtual ~AboutMenu();

    bool doInput(SDL_Event &event) override;

private:
    StaticContainer windowWidget;
    Label           text;

    PictureLabel    planetPicture;
    PictureLabel    duneLegacy;
    PictureLabel    buttonBorder;
};

#endif //ABOUTMENU_H
