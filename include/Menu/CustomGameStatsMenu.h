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

#ifndef CUSTOMGAMESTATSMENU_H
#define CUSTOMGAMESTATSMENU_H

#include "MenuBase.h"
#include <GUI/StaticContainer.h>
#include <GUI/VBox.h>
#include <GUI/HBox.h>
#include <GUI/Label.h>
#include <GUI/TextButton.h>
#include <GUI/ProgressBar.h>

#include <DataTypes.h>

class CustomGameStatsMenu : public MenuBase
{
public:
    CustomGameStatsMenu();
    virtual ~CustomGameStatsMenu();

private:
    void onOK();

    StaticContainer windowWidget;

    VBox            mainVBox;

    Label           captionLabel;

    HBox            mainHBox;

    VBox            playerStatListVBox;

    HBox            headerHBox;

    Label           headerLabelDummy;
    Label           headerLabel1;
    Label           headerLabel2;
    Label           headerLabel3;

    class HouseStat {
    public:
        HBox            houseHBox;
        Label           houseName;

        Label           value1;
        VBox            vBox1;
        ProgressBar     progressBar1;
        Label           value2;
        VBox            vBox2;
        ProgressBar     progressBar2;
        Label           value3;
        VBox            vBox3;
        ProgressBar     progressBar3;
    };

    HouseStat               houseStat[NUM_HOUSES];

    // bottom row of buttons
    HBox            buttonHBox;
    Label           timeLabel;
    TextButton      okButton;
};

#endif //CUSTOMGAMESTATSMENU_H
