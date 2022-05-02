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

#ifndef SINGLEPLAYERSKIRMISHMENU_H
#define SINGLEPLAYERSKIRMISHMENU_H

#include "MenuBase.h"

#include <GUI/InvisibleButton.h>
#include <GUI/PictureButton.h>
#include <GUI/PictureLabel.h>
#include <GUI/StaticContainer.h>
#include <GUI/TextButton.h>
#include <GUI/VBox.h>
#include <GUI/dune/DigitsCounter.h>

class SinglePlayerSkirmishMenu final : public MainMenuBase {
    using parent = MainMenuBase;

public:
    SinglePlayerSkirmishMenu();
    ~SinglePlayerSkirmishMenu() override;

private:
    void onStart();
    void onCancel();

    void onSelectHouseButton(int button);
    void onHouseLeft();
    void onHouseRight();

    void onMissionIncrement();
    void onMissionDecrement();

    void updateHouseChoice();

    InvisibleButton house1Button;
    PictureLabel house1Picture;
    PictureLabel house1SelectedPicture;
    InvisibleButton house2Button;
    PictureLabel house2Picture;
    PictureLabel house2SelectedPicture;
    InvisibleButton house3Button;
    PictureLabel house3Picture;
    PictureLabel house3SelectedPicture;

    PictureButton houseLeftButton;
    PictureButton houseRightButton;

    PictureButton missionPlusButton;
    PictureButton missionMinusButton;
    DigitsCounter missionCounter;

    StaticContainer windowWidget;
    StaticContainer houseChoiceContainer;
    VBox menuButtonsVBox;

    TextButton startButton;
    TextButton backButton;

    PictureLabel heraldPicture;
    PictureLabel duneLegacy;
    PictureLabel buttonBorder;

    int currentHouseChoiceScrollPos = 0;
    int selectedButton              = 1;
    int mission                     = 1;
};

#endif // SINGLEPLAYERSKIRMISHMENU_H
