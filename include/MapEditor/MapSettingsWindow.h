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

#ifndef MAPSETTINGSWINDOW_H
#define MAPSETTINGSWINDOW_H

#include <MapEditor/MapData.h>
#include <MapEditor/ReinforcementInfo.h>

#include <GUI/Window.h>
#include <GUI/HBox.h>
#include <GUI/VBox.h>
#include <GUI/Label.h>
#include <GUI/DropDownBox.h>
#include <GUI/TextButton.h>
#include <GUI/Checkbox.h>
#include <GUI/dune/DigitsTextBox.h>

#include <misc/SDL2pp.h>

// forward declaration
class MapEditor;


class  MapSettingsWindow : public Window
{
public:

    MapSettingsWindow(MapEditor* pMapEditor, HOUSETYPE currentHouse);


    /**
        This static method creates a dynamic map settings window.
        The idea behind this method is to simply create a new dialog on the fly and
        add it as a child window of some other window. If the window gets closed it will be freed.
        \param  pMapEditor  pointer to the currently running map editor
        \param  house       the currently selected house; used for button colors, etc.
        \return The new dialog box (will be automatically destroyed when it's closed)
    */
    static MapSettingsWindow* create(MapEditor* pMapEditor, HOUSETYPE house) {
        MapSettingsWindow* dlg = new MapSettingsWindow(pMapEditor, house);
        dlg->pAllocated = true;
        return dlg;
    }


private:

    void onCancel();
    void onOK();


    HBox            mainHBox;
    VBox            mainVBox;
    VBox            centralVBox;

    HBox            pictureHBox;

    VBox            winPictureVBox;
    Label           winPictureLabel;
    DropDownBox     winPictureDropDownBox;

    VBox            losePictureVBox;
    Label           losePictureLabel;
    DropDownBox     losePictureDropDownBox;

    VBox            briefingPictureVBox;
    Label           briefingPictureLabel;
    DropDownBox     briefingPictureDropDownBox;

    Label           gameFinishingConditionsLabel;

    HBox            winFlags1HBox;
    Checkbox        winFlagsTimeoutCheckbox;
    DigitsTextBox   winFlagsTimeoutTextBox;
    Checkbox        winFlagsSpiceQuotaCheckbox;

    HBox            winFlags2HBox;
    Checkbox        winFlagsPlayerNoObjectsLeftCheckbox;
    Checkbox        winFlagsAIPlayerNoObjectsLeftCheckbox;

    Label           gameWinningConditionsLabel;

    HBox            loseFlags1HBox;
    Checkbox        loseFlagsTimeoutCheckbox;
    Checkbox        loseFlagsSpiceQuotaCheckbox;

    HBox            loseFlags2HBox;
    Checkbox        loseFlagsPlayerHasObjectsLeftCheckbox;
    Checkbox        loseFlagsAIPlayerNoObjectsLeftCheckbox;

    HBox            techLevelHBox;
    Label           techLevelLabel;
    DropDownBox     techLevelDropDownBox;

    HBox            authorHBox;
    Label           authorLabel;
    TextBox         authorTextBox;
    HBox            licenseHBox;
    Label           licenseLabel;
    TextBox         licenseTextBox;


    HBox            buttonHBox;

    Label           titleLabel;
    TextButton      cancelButton;
    TextButton      okButton;

    MapEditor*      pMapEditor;

    HOUSETYPE       house;
    Uint32          color;

    std::vector<std::string>    availableWinPictures;
    std::vector<std::string>    availableLosePictures;
    std::vector<std::string>    availableBriefingPictures;
};


#endif // MAPSETTINGSWINDOW_H
