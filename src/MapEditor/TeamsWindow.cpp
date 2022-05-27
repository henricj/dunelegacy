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

#include <MapEditor/TeamsWindow.h>

#include <GUI/MsgBox.h>
#include <GUI/Spacer.h>

#include <globals.h>

#include <sand.h>

#include <fmt/printf.h>

#include <MapEditor/MapEditor.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>

TeamsWindow::TeamsWindow(MapEditor* pMapEditor, HOUSETYPE currentHouse)
    : Window(0, 0, 0, 0), pMapEditor(pMapEditor), house_(currentHouse), aiteams_(pMapEditor->getAITeams()) {

    color_ = SDL2RGB(dune::globals::palette[dune::globals::houseToPaletteIndex[static_cast<int>(house_)] + 3]);

    auto* const gfx = dune::globals::pGFXManager.get();

    // set up window
    const auto* const pBackground = gfx->getUIGraphic(UI_NewMapWindow);
    setBackground(pBackground);

    TeamsWindow::setCurrentPosition(calcAlignedDrawingRect(pBackground, HAlign::Center, VAlign::Center));

    TeamsWindow::setWindowWidget(&mainHBox);

    mainHBox.addWidget(Widget::create<HSpacer>(16).release());
    mainHBox.addWidget(&mainVBox);
    mainHBox.addWidget(Widget::create<HSpacer>(16).release());

    titleLabel.setTextColor(COLOR_LIGHTYELLOW, COLOR_TRANSPARENT);
    titleLabel.setAlignment(static_cast<Alignment_Enum>(Alignment_HCenter | Alignment_VCenter));
    titleLabel.setText(_("Teams"));
    mainVBox.addWidget(&titleLabel);

    mainVBox.addWidget(Widget::create<VSpacer>(8).release());

    mainVBox.addWidget(&centralVBox, 360);

    centralVBox.addWidget(&hBox1, 6.0);

    teamsListBox.setColor(color_);
    teamsListBox.setOnSelectionChange([this](auto interactive) { onSelectionChange(interactive); });
    hBox1.addWidget(&teamsListBox, 1.0);

    hBox1.addWidget(Widget::create<HSpacer>(3).release());

    listEntryUpButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_ArrowUp, house_),
                                gfx->getUIGraphicSurface(UI_MapEditor_ArrowUp_Active, house_));
    listEntryUpButton.setTooltipText(_("Move up"));
    listEntryUpButton.setOnClick([this] { onUp(); });
    listControlVBox.addWidget(&listEntryUpButton, 25);
    listControlVBox.addWidget(Widget::create<VSpacer>(3).release());
    listEntryDownButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_ArrowDown, house_),
                                  gfx->getUIGraphicSurface(UI_MapEditor_ArrowDown_Active, house_));
    listEntryDownButton.setTooltipText(_("Move down"));
    listEntryDownButton.setOnClick([this] { onDown(); });
    listControlVBox.addWidget(&listEntryDownButton, 25);

    listControlVBox.addWidget(Widget::create<Spacer>().release(), 6.0);

    addListEntryButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Plus, house_),
                                 gfx->getUIGraphicSurface(UI_MapEditor_Plus_Active, house_));
    addListEntryButton.setTooltipText(_("Add"));
    addListEntryButton.setOnClick([this] { onAdd(); });
    listControlVBox.addWidget(&addListEntryButton, 25);
    listControlVBox.addWidget(Widget::create<VSpacer>(3).release());
    removeListEntryButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Minus, house_),
                                    gfx->getUIGraphicSurface(UI_MapEditor_Minus_Active, house_));
    removeListEntryButton.setTooltipText(_("Remove"));
    removeListEntryButton.setOnClick([this] { onRemove(); });
    listControlVBox.addWidget(&removeListEntryButton, 25);

    hBox1.addWidget(&listControlVBox, 25);

    centralVBox.addWidget(Widget::create<VSpacer>(3).release());

    centralVBox.addWidget(&hBox2);

    playerLabel.setText(_("Player") + ":");
    playerLabel.setTextColor(color_);
    hBox2.addWidget(&playerLabel, 95);
    playerDropDownBox.setColor(color_);
    playerDropDownBox.setOnSelectionChange([this](auto interactive) { onEntryChange(interactive); });

    int currentPlayerNum = 1;
    for (const auto& player : pMapEditor->getPlayers()) {
        if (player.bActive_) {
            std::string entryName = player.bAnyHouse_ ? fmt::sprintf(_("Player %d"), currentPlayerNum++) : player.name_;
            playerDropDownBox.addEntry(entryName, static_cast<int>(player.house_));
        }
    }
    playerDropDownBox.setSelectedItem(0);
    hBox2.addWidget(&playerDropDownBox, 120);
    hBox2.addWidget(Widget::create<Spacer>().release(), 5.0);
    aiTeamBehaviorLabel.setText(_("Team Behavior") + ":");
    aiTeamBehaviorLabel.setTextColor(color_);
    hBox2.addWidget(&aiTeamBehaviorLabel, 120);
    aiTeamBehaviorDropDownBox.setColor(color_);
    aiTeamBehaviorDropDownBox.setOnSelectionChange([this](auto interactive) { onEntryChange(interactive); });

    aiTeamBehaviorDropDownBox.addEntry(getAITeamBehaviorNameByID(AITeamBehavior::AITeamBehavior_Normal),
                                       static_cast<int>(AITeamBehavior::AITeamBehavior_Normal));
    aiTeamBehaviorDropDownBox.addEntry(getAITeamBehaviorNameByID(AITeamBehavior::AITeamBehavior_Guard),
                                       static_cast<int>(AITeamBehavior::AITeamBehavior_Guard));
    aiTeamBehaviorDropDownBox.addEntry(getAITeamBehaviorNameByID(AITeamBehavior::AITeamBehavior_Kamikaze),
                                       static_cast<int>(AITeamBehavior::AITeamBehavior_Kamikaze));
    aiTeamBehaviorDropDownBox.addEntry(getAITeamBehaviorNameByID(AITeamBehavior::AITeamBehavior_Staging),
                                       static_cast<int>(AITeamBehavior::AITeamBehavior_Staging));
    aiTeamBehaviorDropDownBox.addEntry(getAITeamBehaviorNameByID(AITeamBehavior::AITeamBehavior_Flee),
                                       static_cast<int>(AITeamBehavior::AITeamBehavior_Flee));
    aiTeamBehaviorDropDownBox.setSelectedItem(0);
    hBox2.addWidget(&aiTeamBehaviorDropDownBox, 90);

    centralVBox.addWidget(Widget::create<VSpacer>(3).release());

    centralVBox.addWidget(&hBox3);

    aiTeamTypeLabel.setText(_("Team Type") + ":");
    aiTeamTypeLabel.setTextColor(color_);
    hBox3.addWidget(&aiTeamTypeLabel, 95);
    aiTeamTypeDropDownBox.setColor(color_);
    aiTeamTypeDropDownBox.setOnSelectionChange([this](auto interactive) { onEntryChange(interactive); });
    aiTeamTypeDropDownBox.addEntry(_("Foot (Infantry, Troopers)"), static_cast<int>(AITeamType::AITeamType_Foot));
    aiTeamTypeDropDownBox.addEntry(_("Wheeled (Trike, Raider, Quad)"),
                                   static_cast<int>(AITeamType::AITeamType_Wheeled));
    aiTeamTypeDropDownBox.addEntry(_("Tracked (Tank, Launcher, Siege Tank,...)"),
                                   static_cast<int>(AITeamType::AITeamType_Tracked));
    aiTeamTypeDropDownBox.addEntry(_("Winged (Carryall, Ornithopter, Frigate)"),
                                   static_cast<int>(AITeamType::AITeamType_Winged));
    aiTeamTypeDropDownBox.addEntry(_("Slither (Sandworm)"), static_cast<int>(AITeamType::AITeamType_Slither));
    aiTeamTypeDropDownBox.addEntry(_("Harvester (Harvester)"), static_cast<int>(AITeamType::AITeamType_Harvester));
    aiTeamTypeDropDownBox.setSelectedItem(0);
    hBox3.addWidget(&aiTeamTypeDropDownBox, 260);
    hBox3.addWidget(Widget::create<Spacer>().release(), 5.0);
    minUnitsLabel.setText(_("Units") + ":");
    minUnitsLabel.setTextColor(color_);
    hBox3.addWidget(&minUnitsLabel);
    minUnitsTextBox.setColor(house_, color_);
    minUnitsTextBox.setMinMax(0, 99);
    minUnitsTextBox.setOnValueChange([this](auto interactive) { onMinUnitsChange(interactive); });
    hBox3.addWidget(&minUnitsTextBox, 47);
    maxUnitsLabel.setText(_("to"));
    maxUnitsLabel.setTextColor(color_);
    hBox3.addWidget(&maxUnitsLabel, 30);
    maxUnitsTextBox.setColor(house_, color_);
    maxUnitsTextBox.setMinMax(0, 99);
    maxUnitsTextBox.setOnValueChange([this](auto interactive) { onMaxUnitsChange(interactive); });
    hBox3.addWidget(&maxUnitsTextBox, 47);

    mainVBox.addWidget(Widget::create<VSpacer>(5).release());

    mainVBox.addWidget(&buttonHBox);

    cancelButton.setText(_("Cancel"));
    cancelButton.setTextColor(color_);
    cancelButton.setOnClick([this] { onCancel(); });

    buttonHBox.addWidget(&cancelButton);

    buttonHBox.addWidget(Widget::create<HSpacer>(8).release());

    buttonHBox.addWidget(Widget::create<Spacer>().release());

    buttonHBox.addWidget(Widget::create<HSpacer>(8).release());

    okButton.setText(_("OK"));
    okButton.setTextColor(color_);
    okButton.setOnClick([this] { onOK(); });

    buttonHBox.addWidget(&okButton);

    mainVBox.addWidget(Widget::create<VSpacer>(10).release());

    // setup teams listbox
    for (const AITeamInfo& aiteamInfo : aiteams_) {
        teamsListBox.addEntry(getDescribingString(aiteamInfo));
    }

    if (!aiteams_.empty()) {
        teamsListBox.setSelectedItem(0);
        onSelectionChange(true);
    }
}

void TeamsWindow::onCancel() {
    auto* pParentWindow = dynamic_cast<Window*>(getParent());
    if (pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void TeamsWindow::onOK() {
    pMapEditor->startOperation();

    MapEditorChangeTeams changeTeamsOperation(aiteams_);

    pMapEditor->addUndoOperation(changeTeamsOperation.perform(pMapEditor));

    auto* pParentWindow = dynamic_cast<Window*>(getParent());
    if (pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void TeamsWindow::onUp() {
    const int index = teamsListBox.getSelectedIndex();

    if (index >= 1) {
        const AITeamInfo aiteamInfo = aiteams_.at(index);
        aiteams_.erase(aiteams_.begin() + index);
        teamsListBox.removeEntry(index);

        aiteams_.insert(aiteams_.begin() + index - 1, aiteamInfo);
        teamsListBox.insertEntry(index - 1, getDescribingString(aiteamInfo));
        teamsListBox.setSelectedItem(index - 1);
    }
}

void TeamsWindow::onDown() {
    const int index = teamsListBox.getSelectedIndex();

    if ((index >= 0) && (index < teamsListBox.getNumEntries() - 1)) {
        const AITeamInfo aiteamInfo = aiteams_.at(index);
        aiteams_.erase(aiteams_.begin() + index);
        teamsListBox.removeEntry(index);

        aiteams_.insert(aiteams_.begin() + index + 1, aiteamInfo);
        teamsListBox.insertEntry(index + 1, getDescribingString(aiteamInfo));
        teamsListBox.setSelectedItem(index + 1);
    }
}

void TeamsWindow::onAdd() {
    if (pMapEditor->getMapVersion() < 2 && teamsListBox.getNumEntries() >= 16) {
        MsgBox* pMsgBox = MsgBox::create(_("Dune2-compatible maps support only up to 16 entries!"));
        pMsgBox->setTextColor(color_);
        openWindow(pMsgBox);
        return;
    }

    const int index = teamsListBox.getSelectedIndex();

    const AITeamInfo aiteamInfo(static_cast<HOUSETYPE>(playerDropDownBox.getSelectedEntryIntData()),
                                static_cast<AITeamBehavior>(aiTeamBehaviorDropDownBox.getSelectedEntryIntData()),
                                static_cast<AITeamType>(aiTeamTypeDropDownBox.getSelectedEntryIntData()),
                                minUnitsTextBox.getValue(), maxUnitsTextBox.getValue());
    aiteams_.insert(aiteams_.begin() + index + 1, aiteamInfo);
    teamsListBox.insertEntry(index + 1, getDescribingString(aiteamInfo));
    teamsListBox.setSelectedItem(index + 1);
}

void TeamsWindow::onRemove() {
    const int index = teamsListBox.getSelectedIndex();

    if (index >= 0) {
        aiteams_.erase(aiteams_.begin() + index);
        teamsListBox.removeEntry(index);
        teamsListBox.setSelectedItem(index < teamsListBox.getNumEntries() ? index : (teamsListBox.getNumEntries() - 1));
    }
}

void TeamsWindow::onSelectionChange(bool bInteractive) {
    const int index = teamsListBox.getSelectedIndex();

    if (index >= 0) {
        AITeamInfo& aiteamInfo = aiteams_.at(index);

        for (int i = 0; i < playerDropDownBox.getNumEntries(); i++) {
            if (playerDropDownBox.getEntryIntData(i) == static_cast<int>(aiteamInfo.houseID)) {
                playerDropDownBox.setSelectedItem(i);
                break;
            }
        }

        for (int i = 0; i < aiTeamBehaviorDropDownBox.getNumEntries(); i++) {
            if (aiTeamBehaviorDropDownBox.getEntryIntData(i) == static_cast<int>(aiteamInfo.aiTeamBehavior)) {
                aiTeamBehaviorDropDownBox.setSelectedItem(i);
                break;
            }
        }

        for (int i = 0; i < aiTeamTypeDropDownBox.getNumEntries(); i++) {
            if (aiTeamTypeDropDownBox.getEntryIntData(i) == static_cast<int>(aiteamInfo.aiTeamType)) {
                aiTeamTypeDropDownBox.setSelectedItem(i);
                break;
            }
        }

        minUnitsTextBox.setValue(aiteamInfo.minUnits);
        maxUnitsTextBox.setValue(aiteamInfo.maxUnits);
    }
}

void TeamsWindow::onMinUnitsChange(bool bInteractive) {
    if (bInteractive) {
        const int minUnits = minUnitsTextBox.getValue();
        const int maxUnits = maxUnitsTextBox.getValue();

        if (minUnits > maxUnits) {
            maxUnitsTextBox.setValue(minUnits);
        }
    }

    onEntryChange(bInteractive);
}

void TeamsWindow::onMaxUnitsChange(bool bInteractive) {
    if (bInteractive) {
        const int minUnits = minUnitsTextBox.getValue();
        const int maxUnits = maxUnitsTextBox.getValue();

        if (minUnits > maxUnits) {
            minUnitsTextBox.setValue(maxUnits);
        }
    }

    onEntryChange(bInteractive);
}

void TeamsWindow::onEntryChange(bool bInteractive) {
    if (bInteractive) {
        const int index = teamsListBox.getSelectedIndex();

        if (index >= 0) {
            AITeamInfo& aiteamInfo = aiteams_.at(index);
            aiteamInfo.houseID     = static_cast<HOUSETYPE>(playerDropDownBox.getSelectedEntryIntData());
            aiteamInfo.aiTeamBehavior =
                static_cast<AITeamBehavior>(aiTeamBehaviorDropDownBox.getSelectedEntryIntData());
            aiteamInfo.aiTeamType = static_cast<AITeamType>(aiTeamTypeDropDownBox.getSelectedEntryIntData());
            aiteamInfo.minUnits   = minUnitsTextBox.getValue();
            aiteamInfo.maxUnits   = maxUnitsTextBox.getValue();
            teamsListBox.setEntry(index, getDescribingString(aiteamInfo));
        }
    }
}

std::string TeamsWindow::getDescribingString(const AITeamInfo& aiteamInfo) {

    return getPlayerName(aiteamInfo.houseID) + ", " + getAITeamBehaviorNameByID(aiteamInfo.aiTeamBehavior) + ", "
         + getAITeamTypeNameByID(aiteamInfo.aiTeamType) + ", " + std::to_string(aiteamInfo.minUnits) + ", "
         + std::to_string(aiteamInfo.maxUnits);
}

std::string TeamsWindow::getPlayerName(HOUSETYPE house) const {
    int currentPlayerNum = 1;
    for (const auto& player : pMapEditor->getPlayers()) {
        if (player.house_ == house) {
            return player.bAnyHouse_ ? fmt::sprintf(_("Player %d"), currentPlayerNum)
                                     : (_("House") + " " + player.name_);
        }

        if (player.bActive_ && player.bAnyHouse_) {
            currentPlayerNum++;
        }
    }

    return "";
}
