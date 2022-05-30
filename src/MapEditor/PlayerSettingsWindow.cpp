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

#include <fmt/printf.h>

#include <MapEditor/MapEditor.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>

#include <utility>

PlayerSettingsWindow::PlayerSettingsWindow(MapEditor* pMapEditor, HOUSETYPE currentHouse)
    : Window(0, 0, 0, 0), pMapEditor(pMapEditor), house(currentHouse) {

    color = SDL2RGB(dune::globals::palette[dune::globals::houseToPaletteIndex[static_cast<int>(house)] + 3]);

    // set up window
    const auto* const pBackground = dune::globals::pGFXManager->getUIGraphic(UI_NewMapWindow);
    setBackground(pBackground);

    PlayerSettingsWindow::setCurrentPosition(calcAlignedDrawingRect(pBackground, HAlign::Center, VAlign::Center));

    PlayerSettingsWindow::setWindowWidget(&mainHBox);

    mainHBox.addWidget(Widget::create<HSpacer>(16).release());
    mainHBox.addWidget(&mainVBox);
    mainHBox.addWidget(Widget::create<HSpacer>(16).release());

    titleLabel.setTextColor(COLOR_LIGHTYELLOW, COLOR_TRANSPARENT);
    titleLabel.setAlignment(static_cast<Alignment_Enum>(Alignment_HCenter | Alignment_VCenter));
    titleLabel.setText(_("Player Settings"));
    mainVBox.addWidget(&titleLabel);

    mainVBox.addWidget(Widget::create<VSpacer>(8).release());

    mainVBox.addWidget(&centralVBox, 360);

    const auto& players = pMapEditor->getPlayers();
    for (auto i = 0; std::cmp_less(i, players.size()); ++i) {

        const auto& playerInfo = players[i];

        const auto color_id     = static_cast<int>(playerInfo.colorOfHouse_);
        const auto currentColor = SDL2RGB(dune::globals::palette[dune::globals::houseToPaletteIndex[color_id] + 3]);

        centralVBox.addWidget(Widget::create<VSpacer>(15).release());

        auto& pw = playerWidgets[i];

        pw.playerCheckbox.setTextColor(currentColor);
        pw.playerCheckbox.setOnClick([this, i] { onPlayerCheckbox(i); });
        if (pMapEditor->getMapVersion() < 2) {
            pw.playerCheckbox.setText(_("House") + " " + getHouseNameByNumber(static_cast<HOUSETYPE>(i)) + ":");
            pw.playerHBox.addWidget(&pw.playerCheckbox, 150);
        } else {
            pw.playerCheckbox.setText(fmt::sprintf(_("Player %d:"), i + 1));
            pw.playerHBox.addWidget(&pw.playerCheckbox, 0.22);
        }

        if (pMapEditor->getMapVersion() >= 2) {
            pw.anyHouseRadioButton.setText(_("any"));
            pw.anyHouseRadioButton.setTextColor(currentColor);
            pw.playerHBox.addWidget(&pw.anyHouseRadioButton);

            pw.houseRadioButton.setText(getHouseNameByNumber(playerInfo.house_));
            pw.houseRadioButton.setTextColor(currentColor);
            pw.playerHBox.addWidget(&pw.houseRadioButton, 110);

            pw.radioButtonManager.registerRadioButtons({&pw.anyHouseRadioButton, &pw.houseRadioButton});
            if (playerInfo.bAnyHouse_) {
                pw.anyHouseRadioButton.setChecked(true);
            } else {
                pw.houseRadioButton.setChecked(true);
            }
        }

        pw.playerHBox.addWidget(&pw.spacer, 5.0);

        pw.creditsLabel.setText(_("Credits") + ":");
        pw.creditsLabel.setTextColor(currentColor);
        pw.playerHBox.addWidget(&pw.creditsLabel);

        pw.creditsTextBox.setMinMax(0, 100000);
        pw.creditsTextBox.setValue(playerInfo.credits_);
        pw.creditsTextBox.setIncrementValue(100);
        pw.creditsTextBox.setColor(house, currentColor);
        pw.playerHBox.addWidget(&pw.creditsTextBox, 80);

        pw.playerHBox.addWidget(Widget::create<Spacer>().release(), 5.0);

        pw.teamLabel.setText(_("Team") + ":");
        pw.teamLabel.setTextColor(currentColor);
        pw.playerHBox.addWidget(&pw.teamLabel);

        if (pMapEditor->getMapVersion() < 2) {
            pw.teamDropDownBox.addEntry("Human", 0);
            pw.teamDropDownBox.addEntry("CPU", 1);

            if (playerInfo.brain_ == "Human") {
                pw.teamDropDownBox.setSelectedItem(0);
            } else {
                pw.teamDropDownBox.setSelectedItem(1);
            }
        } else {
            pw.teamDropDownBox.addEntry("Team1", 0);
            pw.teamDropDownBox.addEntry("Team2", 1);
            pw.teamDropDownBox.addEntry("Team3", 2);
            pw.teamDropDownBox.addEntry("Team4", 3);
            pw.teamDropDownBox.addEntry("Team5", 4);
            pw.teamDropDownBox.addEntry("Team6", 5);

            for (int j = 0; j < 6; j++) {
                if (pw.teamDropDownBox.getEntry(j) == playerInfo.brain_) {
                    pw.teamDropDownBox.setSelectedItem(j);
                    break;
                }
            }
        }

        pw.teamDropDownBox.setColor(currentColor);
        pw.playerHBox.addWidget(&pw.teamDropDownBox, 65);

        // prepare advanced widgets
        pw.spiceQuotaLabel.setText(_("Quota") + ":");
        pw.spiceQuotaLabel.setTextColor(currentColor);

        pw.spiceQuotaTextBox.setMinMax(0, 99999);
        pw.spiceQuotaTextBox.setValue(playerInfo.quota_);
        pw.spiceQuotaTextBox.setIncrementValue(100);
        pw.spiceQuotaTextBox.setColor(house, currentColor);

        pw.maxUnitsLabel.setText(_("Max Units") + ":");
        pw.maxUnitsLabel.setTextColor(currentColor);

        pw.maxUnitsTextBox.setMinMax(0, 999);
        pw.maxUnitsTextBox.setValue(playerInfo.maxunit_);
        pw.maxUnitsTextBox.setColor(house, currentColor);

        centralVBox.addWidget(&pw.playerHBox);

        // activate first 4 players
        pw.playerCheckbox.setChecked(playerInfo.bActive_);
        onPlayerCheckbox(i);
    }

    centralVBox.addWidget(Widget::create<Spacer>().release());

    mainVBox.addWidget(Widget::create<VSpacer>(5).release());

    mainVBox.addWidget(&buttonHBox);

    cancelButton.setText(_("Cancel"));
    cancelButton.setTextColor(color);
    cancelButton.setOnClick([this] { onCancel(); });

    buttonHBox.addWidget(&cancelButton);

    buttonHBox.addWidget(Widget::create<HSpacer>(8).release());

    advancedBasicToggle.setText(_("Advanced..."));
    advancedBasicToggle.setTextColor(color);
    advancedBasicToggle.setOnClick([this] { onAdvancedBasicToggle(); });

    buttonHBox.addWidget(&advancedBasicToggle);

    buttonHBox.addWidget(Widget::create<HSpacer>(8).release());

    okButton.setText(_("OK"));
    okButton.setTextColor(color);
    okButton.setOnClick([this] { onOK(); });

    buttonHBox.addWidget(&okButton);

    mainVBox.addWidget(Widget::create<VSpacer>(10).release());
}

void PlayerSettingsWindow::onCancel() {
    auto* pParentWindow = dynamic_cast<Window*>(getParent());
    if (pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void PlayerSettingsWindow::onAdvancedBasicToggle() {
    if (advancedBasicToggle.getText() == _("Advanced...")) {
        advancedBasicToggle.setText(_("Basic..."));

        for (auto& playerWidget : playerWidgets) {
            playerWidget.playerHBox.removeChildWidget(&playerWidget.creditsLabel);
            playerWidget.playerHBox.removeChildWidget(&playerWidget.creditsTextBox);
            playerWidget.playerHBox.removeChildWidget(&playerWidget.spacer);
            playerWidget.playerHBox.removeChildWidget(&playerWidget.teamLabel);
            playerWidget.playerHBox.removeChildWidget(&playerWidget.teamDropDownBox);

            playerWidget.playerHBox.addWidget(&playerWidget.spiceQuotaLabel);
            playerWidget.playerHBox.addWidget(&playerWidget.spiceQuotaTextBox, 75);
            playerWidget.playerHBox.addWidget(&playerWidget.spacer, 5.0);
            playerWidget.playerHBox.addWidget(&playerWidget.maxUnitsLabel);
            playerWidget.playerHBox.addWidget(&playerWidget.maxUnitsTextBox, 50);
        }

    } else {
        advancedBasicToggle.setText(_("Advanced..."));

        for (auto& playerWidget : playerWidgets) {
            playerWidget.playerHBox.removeChildWidget(&playerWidget.spiceQuotaLabel);
            playerWidget.playerHBox.removeChildWidget(&playerWidget.spiceQuotaTextBox);
            playerWidget.playerHBox.removeChildWidget(&playerWidget.spacer);
            playerWidget.playerHBox.removeChildWidget(&playerWidget.maxUnitsLabel);
            playerWidget.playerHBox.removeChildWidget(&playerWidget.maxUnitsTextBox);

            playerWidget.playerHBox.addWidget(&playerWidget.creditsLabel);
            playerWidget.playerHBox.addWidget(&playerWidget.creditsTextBox, 80);
            playerWidget.playerHBox.addWidget(&playerWidget.spacer, 5.0);
            playerWidget.playerHBox.addWidget(&playerWidget.teamLabel);
            playerWidget.playerHBox.addWidget(&playerWidget.teamDropDownBox, 65);
        }
    }
}

void PlayerSettingsWindow::onOK() {

    pMapEditor->startOperation();

    for (auto i = 0; std::cmp_less(i, playerWidgets.size()); ++i) {
        auto& pw = playerWidgets[i];

        const auto bActive   = pw.playerCheckbox.isChecked();
        const auto bAnyHouse = pMapEditor->getMapVersion() < 2 ? false : pw.anyHouseRadioButton.isChecked();
        const auto credits   = pw.creditsTextBox.getValue();
        auto brain           = pw.teamDropDownBox.getSelectedEntry();
        const auto quota     = pw.spiceQuotaTextBox.getValue();
        const auto maxunit   = pw.maxUnitsTextBox.getValue();

        MapEditorChangePlayer changePlayerOperation(i, bActive, bAnyHouse, credits, std::move(brain), quota, maxunit);

        pMapEditor->addUndoOperation(changePlayerOperation.perform(pMapEditor));
    }

    auto* const pParentWindow = dynamic_cast<Window*>(getParent());
    if (pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void PlayerSettingsWindow::onPlayerCheckbox(int i) {
    auto& pw = playerWidgets.at(i);

    const bool bChecked = pw.playerCheckbox.isChecked();

    pw.anyHouseRadioButton.setVisible(bChecked);
    pw.houseRadioButton.setVisible(bChecked);
    pw.creditsLabel.setVisible(bChecked);
    pw.creditsTextBox.setVisible(bChecked);
    pw.teamLabel.setVisible(bChecked);
    pw.teamDropDownBox.setVisible(bChecked);
    pw.spiceQuotaLabel.setVisible(bChecked);
    pw.spiceQuotaTextBox.setVisible(bChecked);
    pw.maxUnitsLabel.setVisible(bChecked);
    pw.maxUnitsTextBox.setVisible(bChecked);
}
