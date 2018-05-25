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

#include <MapEditor/PlayerSettingsWindow.h>

#include <GUI/Spacer.h>

#include <globals.h>

#include <sand.h>

#include <misc/format.h>

#include <MapEditor/MapEditor.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>



PlayerSettingsWindow::PlayerSettingsWindow(MapEditor* pMapEditor, HOUSETYPE currentHouse) : Window(0,0,0,0), pMapEditor(pMapEditor), house(currentHouse) {

    color = SDL2RGB(palette[houseToPaletteIndex[house]+3]);

    // set up window
    SDL_Texture *pBackground = pGFXManager->getUIGraphic(UI_NewMapWindow);
    setBackground(pBackground);

    setCurrentPosition(calcAlignedDrawingRect(pBackground, HAlign::Center, VAlign::Center));

    setWindowWidget(&mainHBox);

    mainHBox.addWidget(HSpacer::create(16));
    mainHBox.addWidget(&mainVBox);
    mainHBox.addWidget(HSpacer::create(16));

    titleLabel.setTextColor(COLOR_LIGHTYELLOW, COLOR_TRANSPARENT);
    titleLabel.setAlignment((Alignment_Enum) (Alignment_HCenter | Alignment_VCenter));
    titleLabel.setText(_("Player Settings"));
    mainVBox.addWidget(&titleLabel);

    mainVBox.addWidget(VSpacer::create(8));

    mainVBox.addWidget(&centralVBox, 360);

    for(int i=0;i<NUM_HOUSES;i++) {

        MapEditor::Player& playerInfo = pMapEditor->getPlayers()[i];

        Uint32 currentColor = SDL2RGB(palette[houseToPaletteIndex[playerInfo.colorOfHouse] + 3]);

        centralVBox.addWidget(VSpacer::create(15));

        playerWidgets[i].playerCheckbox.setTextColor(currentColor);
        playerWidgets[i].playerCheckbox.setOnClick(std::bind(&PlayerSettingsWindow::onPlayerCheckbox, this, i));
        if(pMapEditor->getMapVersion() < 2) {
            playerWidgets[i].playerCheckbox.setText(_("House") + " " + getHouseNameByNumber((HOUSETYPE) i) + ":");
            playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].playerCheckbox, 150);
        } else {
            playerWidgets[i].playerCheckbox.setText(fmt::sprintf(_("Player %d:"), i+1));
            playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].playerCheckbox, 0.22);
        }

        if(pMapEditor->getMapVersion() >= 2) {
            playerWidgets[i].anyHouseRadioButton.setText(_("any"));
            playerWidgets[i].anyHouseRadioButton.setTextColor(currentColor);
            playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].anyHouseRadioButton);

            playerWidgets[i].houseRadioButton.setText( getHouseNameByNumber(playerInfo.house));
            playerWidgets[i].houseRadioButton.setTextColor(currentColor);
            playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].houseRadioButton, 110);

            playerWidgets[i].radioButtonManager.registerRadioButtons(2, &playerWidgets[i].anyHouseRadioButton, &playerWidgets[i].houseRadioButton);
            if(playerInfo.bAnyHouse) {
                playerWidgets[i].anyHouseRadioButton.setChecked(true);
            } else {
                playerWidgets[i].houseRadioButton.setChecked(true);
            }
        }

        playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].spacer, 5.0);

        playerWidgets[i].creditsLabel.setText(_("Credits") + ":");
        playerWidgets[i].creditsLabel.setTextColor(currentColor);
        playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].creditsLabel);

        playerWidgets[i].creditsTextBox.setMinMax(0,100000);
        playerWidgets[i].creditsTextBox.setValue(playerInfo.credits);
        playerWidgets[i].creditsTextBox.setIncrementValue(100);
        playerWidgets[i].creditsTextBox.setColor(house, currentColor);
        playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].creditsTextBox, 80);

        playerWidgets[i].playerHBox.addWidget(Spacer::create(), 5.0);

        playerWidgets[i].teamLabel.setText(_("Team") + ":");
        playerWidgets[i].teamLabel.setTextColor(currentColor);
        playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].teamLabel);

        if(pMapEditor->getMapVersion() < 2) {
            playerWidgets[i].teamDropDownBox.addEntry("Human", 0);
            playerWidgets[i].teamDropDownBox.addEntry("CPU", 1);

            if(playerInfo.brain == "Human") {
                playerWidgets[i].teamDropDownBox.setSelectedItem(0);
            } else {
                playerWidgets[i].teamDropDownBox.setSelectedItem(1);
            }
        } else {
            playerWidgets[i].teamDropDownBox.addEntry("Team1", 0);
            playerWidgets[i].teamDropDownBox.addEntry("Team2", 1);
            playerWidgets[i].teamDropDownBox.addEntry("Team3", 2);
            playerWidgets[i].teamDropDownBox.addEntry("Team4", 3);
            playerWidgets[i].teamDropDownBox.addEntry("Team5", 4);
            playerWidgets[i].teamDropDownBox.addEntry("Team6", 5);

            for(int j = 0; j < 6; j++) {
                if(playerWidgets[i].teamDropDownBox.getEntry(j) == playerInfo.brain) {
                    playerWidgets[i].teamDropDownBox.setSelectedItem(j);
                    break;
                }
            }
        }

        playerWidgets[i].teamDropDownBox.setColor(currentColor);
        playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].teamDropDownBox, 65);

        // prepare advanced widgets
        playerWidgets[i].spiceQuotaLabel.setText(_("Quota") + ":");
        playerWidgets[i].spiceQuotaLabel.setTextColor(currentColor);

        playerWidgets[i].spiceQuotaTextBox.setMinMax(0,99999);
        playerWidgets[i].spiceQuotaTextBox.setValue(playerInfo.quota);
        playerWidgets[i].spiceQuotaTextBox.setIncrementValue(100);
        playerWidgets[i].spiceQuotaTextBox.setColor(house, currentColor);

        playerWidgets[i].maxUnitsLabel.setText(_("Max Units") + ":");
        playerWidgets[i].maxUnitsLabel.setTextColor(currentColor);

        playerWidgets[i].maxUnitsTextBox.setMinMax(0,999);
        playerWidgets[i].maxUnitsTextBox.setValue(playerInfo.maxunit);
        playerWidgets[i].maxUnitsTextBox.setColor(house, currentColor);


        centralVBox.addWidget(&playerWidgets[i].playerHBox);

        // activate first 4 players
        playerWidgets[i].playerCheckbox.setChecked(playerInfo.bActive);
        onPlayerCheckbox(i);
    }


    centralVBox.addWidget(Spacer::create());

    mainVBox.addWidget(VSpacer::create(5));

    mainVBox.addWidget(&buttonHBox);

    cancelButton.setText(_("Cancel"));
    cancelButton.setTextColor(color);
    cancelButton.setOnClick(std::bind(&PlayerSettingsWindow::onCancel, this));

    buttonHBox.addWidget(&cancelButton);

    buttonHBox.addWidget(HSpacer::create(8));

    advancedBasicToggle.setText(_("Advanced..."));
    advancedBasicToggle.setTextColor(color);
    advancedBasicToggle.setOnClick(std::bind(&PlayerSettingsWindow::onAdvancedBasicToggle, this));

    buttonHBox.addWidget(&advancedBasicToggle);

    buttonHBox.addWidget(HSpacer::create(8));

    okButton.setText(_("OK"));
    okButton.setTextColor(color);
    okButton.setOnClick(std::bind(&PlayerSettingsWindow::onOK, this));

    buttonHBox.addWidget(&okButton);

    mainVBox.addWidget(VSpacer::create(10));
}

void PlayerSettingsWindow::onCancel() {
    Window* pParentWindow = dynamic_cast<Window*>(getParent());
    if(pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void PlayerSettingsWindow::onAdvancedBasicToggle() {
    if(advancedBasicToggle.getText() == _("Advanced...")) {
        advancedBasicToggle.setText(_("Basic..."));

        for(int i=0;i<NUM_HOUSES;i++) {
            playerWidgets[i].playerHBox.removeChildWidget(&playerWidgets[i].creditsLabel);
            playerWidgets[i].playerHBox.removeChildWidget(&playerWidgets[i].creditsTextBox);
            playerWidgets[i].playerHBox.removeChildWidget(&playerWidgets[i].spacer);
            playerWidgets[i].playerHBox.removeChildWidget(&playerWidgets[i].teamLabel);
            playerWidgets[i].playerHBox.removeChildWidget(&playerWidgets[i].teamDropDownBox);


            playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].spiceQuotaLabel);
            playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].spiceQuotaTextBox, 75);
            playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].spacer, 5.0);
            playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].maxUnitsLabel);
            playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].maxUnitsTextBox, 50);
        }

    } else {
        advancedBasicToggle.setText(_("Advanced..."));

        for(int i=0;i<NUM_HOUSES;i++) {
            playerWidgets[i].playerHBox.removeChildWidget(&playerWidgets[i].spiceQuotaLabel);
            playerWidgets[i].playerHBox.removeChildWidget(&playerWidgets[i].spiceQuotaTextBox);
            playerWidgets[i].playerHBox.removeChildWidget(&playerWidgets[i].spacer);
            playerWidgets[i].playerHBox.removeChildWidget(&playerWidgets[i].maxUnitsLabel);
            playerWidgets[i].playerHBox.removeChildWidget(&playerWidgets[i].maxUnitsTextBox);


            playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].creditsLabel);
            playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].creditsTextBox, 80);
            playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].spacer, 5.0);
            playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].teamLabel);
            playerWidgets[i].playerHBox.addWidget(&playerWidgets[i].teamDropDownBox, 65);
        }
    }
}

void PlayerSettingsWindow::onOK() {

    pMapEditor->startOperation();

    for(int i=0;i<NUM_HOUSES;i++) {
        bool bActive = playerWidgets[i].playerCheckbox.isChecked();
        bool bAnyHouse = pMapEditor->getMapVersion() < 2 ? false : playerWidgets[i].anyHouseRadioButton.isChecked();
        int credits = playerWidgets[i].creditsTextBox.getValue();
        std::string brain = playerWidgets[i].teamDropDownBox.getSelectedEntry();
        int quota = playerWidgets[i].spiceQuotaTextBox.getValue();
        int maxunit = playerWidgets[i].maxUnitsTextBox.getValue();

        MapEditorChangePlayer changePlayerOperation(i, bActive, bAnyHouse, credits, brain, quota, maxunit);

        pMapEditor->addUndoOperation(changePlayerOperation.perform(pMapEditor));
    }

    Window* pParentWindow = dynamic_cast<Window*>(getParent());
    if(pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void PlayerSettingsWindow::onPlayerCheckbox(int i) {
    bool bChecked = playerWidgets[i].playerCheckbox.isChecked();

    playerWidgets[i].anyHouseRadioButton.setVisible(bChecked);
    playerWidgets[i].houseRadioButton.setVisible(bChecked);
    playerWidgets[i].creditsLabel.setVisible(bChecked);
    playerWidgets[i].creditsTextBox.setVisible(bChecked);
    playerWidgets[i].teamLabel.setVisible(bChecked);
    playerWidgets[i].teamDropDownBox.setVisible(bChecked);
    playerWidgets[i].spiceQuotaLabel.setVisible(bChecked);
    playerWidgets[i].spiceQuotaTextBox.setVisible(bChecked);
    playerWidgets[i].maxUnitsLabel.setVisible(bChecked);
    playerWidgets[i].maxUnitsTextBox.setVisible(bChecked);
}
