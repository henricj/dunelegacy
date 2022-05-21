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

#ifndef HOUSECHOICEMENU_H
#define HOUSECHOICEMENU_H

#include "MenuBase.h"
#include <GUI/PictureButton.h>
#include <GUI/PictureLabel.h>
#include <GUI/StaticContainer.h>

class HouseChoiceMenu final : public TopMenuBase {
    using parent = TopMenuBase;

public:
    HouseChoiceMenu();
    ~HouseChoiceMenu() override;

    /**
        This method resizes the window to width and height.
        \param  width   the new width of this widget
        \param  height  the new height of this widget
    */
    void resize(uint32_t width, uint32_t height) override;

    using parent::resize;

private:
    void onHouseButton(int button);
    void updateHouseChoice();

    void onHouseLeft();
    void onHouseRight();

    StaticContainer windowWidget;

    PictureLabel selectYourHouseLabel;

    PictureButton house1Button;
    PictureButton house2Button;
    PictureButton house3Button;

    PictureButton houseLeftButton;
    PictureButton houseRightButton;

    int currentHouseChoiceScrollPos;
};

#endif // HOUSECHOICEMENU_H
