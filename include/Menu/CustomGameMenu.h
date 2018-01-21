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

#ifndef CUSTOMGAMEMENU_H
#define CUSTOMGAMEMENU_H

#include <GUI/StaticContainer.h>
#include <GUI/VBox.h>
#include <GUI/HBox.h>
#include <GUI/Label.h>
#include <GUI/TextButton.h>
#include <GUI/ListBox.h>
#include <GUI/PictureLabel.h>
#include <GUI/Checkbox.h>

#include <DataTypes.h>

#include <string>

#include "MenuBase.h"

class CustomGameMenu : public MenuBase
{
public:
    CustomGameMenu(bool multiplayer, bool LANServer = true);
    virtual ~CustomGameMenu();

    /**
        This method is called, when the child window is about to be closed.
        This child window will be closed after this method returns.
        \param  pChildWindow    The child window that will be closed
    */
    void onChildWindowClose(Window* pChildWindow) override;

private:
    void onNext();
    void onCancel();
    void onLoad();
    void onGameOptions();
    void onMapTypeChange(int buttonID);
    void onMapListSelectionChange(bool bInteractive);

    bool bMultiplayer;
    bool bLANServer;

    std::string currentMapDirectory;

    SettingsClass::GameOptionsClass currentGameOptions;

    StaticContainer windowWidget;
    VBox            mainVBox;

    Label           captionLabel;

    HBox            mainHBox;

    // left VBox with map list and map options
    VBox            leftVBox;
    HBox            mapTypeButtonsHBox;
    TextButton      singleplayerMapsButton;
    TextButton      singleplayerUserMapsButton;
    TextButton      multiplayerMapsButton;
    TextButton      multiplayerUserMapsButton;
    TextButton      dummyButton;
    ListBox         mapList;
    HBox            optionsHBox;
    Checkbox        multiplePlayersPerHouseCheckbox;
    TextButton      gameOptionsButton;

    // right VBox with mini map
    VBox            rightVBox;
    PictureLabel    minimap;
    HBox            mapPropertiesHBox;
    VBox            mapPropertyNamesVBox;
    VBox            mapPropertyValuesVBox;
    Label           mapPropertySize;
    Label           mapPropertyPlayers;
    Label           mapPropertyAuthors;
    Label           mapPropertyLicense;

    // bottom row of buttons
    HBox            buttonHBox;
    TextButton      nextButton;
    TextButton      loadButton;
    TextButton      cancelButton;

};

#endif //CUSTOMGAMEMENU_H
