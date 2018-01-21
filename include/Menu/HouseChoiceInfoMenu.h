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

#ifndef HOUSECHOICEINFOMENU_H
#define HOUSECHOICEINFOMENU_H

#include "MentatMenu.h"
#include <GUI/PictureButton.h>
#include <GUI/PictureLabel.h>
#include <GUI/dune/AnimationLabel.h>

#define MENU_QUIT_HOUSECHOICE_YES   (1)

class HouseChoiceInfoMenu : public MentatMenu {
public:
    explicit HouseChoiceInfoMenu(int newHouse);
    virtual ~HouseChoiceInfoMenu();

    void onMentatTextFinished() override;

    void drawSpecificStuff() override;
private:
    void onYes();
    void onNo();

    AnimationLabel  planetAnimation;
    PictureLabel    questionLabel;
    PictureButton   yesButton;
    PictureButton   noButton;
};

#endif // HOUSECHOICEINFOMENU_H
