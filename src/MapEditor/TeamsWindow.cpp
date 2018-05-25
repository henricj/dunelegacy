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

#include <GUI/Spacer.h>
#include <GUI/MsgBox.h>

#include <globals.h>

#include <sand.h>

#include <misc/format.h>

#include <MapEditor/MapEditor.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>

TeamsWindow::TeamsWindow(MapEditor* pMapEditor, HOUSETYPE currentHouse)
 : Window(0,0,0,0), pMapEditor(pMapEditor), house(currentHouse), teams(pMapEditor->getTeams()) {

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
    titleLabel.setText(_("Teams"));
    mainVBox.addWidget(&titleLabel);

    mainVBox.addWidget(VSpacer::create(8));

    mainVBox.addWidget(&centralVBox, 360);


    centralVBox.addWidget(&hBox1, 6.0);

    teamsListBox.setColor(color);
    teamsListBox.setOnSelectionChange(std::bind(&TeamsWindow::onSelectionChange, this, std::placeholders::_1));
    hBox1.addWidget(&teamsListBox, 1.0);

    hBox1.addWidget(HSpacer::create(3));

    listEntryUpButton.setSymbol(pGFXManager->getUIGraphicSurface(UI_MapEditor_ArrowUp, house),
                                pGFXManager->getUIGraphicSurface(UI_MapEditor_ArrowUp_Active, house));
    listEntryUpButton.setTooltipText(_("Move up"));
    listEntryUpButton.setOnClick(std::bind(&TeamsWindow::onUp, this));
    listControlVBox.addWidget(&listEntryUpButton, 25);
    listControlVBox.addWidget(VSpacer::create(3));
    listEntryDownButton.setSymbol(pGFXManager->getUIGraphicSurface(UI_MapEditor_ArrowDown, house),
                                  pGFXManager->getUIGraphicSurface(UI_MapEditor_ArrowDown_Active, house));
    listEntryDownButton.setTooltipText(_("Move down"));
    listEntryDownButton.setOnClick(std::bind(&TeamsWindow::onDown, this));
    listControlVBox.addWidget(&listEntryDownButton, 25);

    listControlVBox.addWidget(Spacer::create(), 6.0);

    addListEntryButton.setSymbol(pGFXManager->getUIGraphicSurface(UI_MapEditor_Plus, house),
                                 pGFXManager->getUIGraphicSurface(UI_MapEditor_Plus_Active, house));
    addListEntryButton.setTooltipText(_("Add"));
    addListEntryButton.setOnClick(std::bind(&TeamsWindow::onAdd, this));
    listControlVBox.addWidget(&addListEntryButton, 25);
    listControlVBox.addWidget(VSpacer::create(3));
    removeListEntryButton.setSymbol(pGFXManager->getUIGraphicSurface(UI_MapEditor_Minus, house),
                                    pGFXManager->getUIGraphicSurface(UI_MapEditor_Minus_Active, house));
    removeListEntryButton.setTooltipText(_("Remove"));
    removeListEntryButton.setOnClick(std::bind(&TeamsWindow::onRemove, this));
    listControlVBox.addWidget(&removeListEntryButton, 25);

    hBox1.addWidget(&listControlVBox, 25);

    centralVBox.addWidget(VSpacer::create(3));

    centralVBox.addWidget(&hBox2);

    playerLabel.setText(_("Player") + ":");
    playerLabel.setTextColor(color);
    hBox2.addWidget(&playerLabel, 95);
    playerDropDownBox.setColor(color);
    playerDropDownBox.setOnSelectionChange(std::bind(&TeamsWindow::onEntryChange, this, std::placeholders::_1));

    int currentPlayerNum = 1;
    for(const MapEditor::Player& player : pMapEditor->getPlayers()) {
        if(player.bActive) {
            std::string entryName = player.bAnyHouse ? fmt::sprintf(_("Player %d"), currentPlayerNum++) : player.name;
            playerDropDownBox.addEntry(entryName, player.house);
        }
    }
    playerDropDownBox.setSelectedItem(0);
    hBox2.addWidget(&playerDropDownBox, 120);
    hBox2.addWidget(Spacer::create(), 5.0);
    teamBehaviorLabel.setText(_("Team Behavior") + ":");
    teamBehaviorLabel.setTextColor(color);
    hBox2.addWidget(&teamBehaviorLabel, 120);
    teamBehaviorDropDownBox.setColor(color);
    teamBehaviorDropDownBox.setOnSelectionChange(std::bind(&TeamsWindow::onEntryChange, this, std::placeholders::_1));

    teamBehaviorDropDownBox.addEntry(getTeamBehaviorNameByID(TeamBehavior_Normal), TeamBehavior_Normal);
    teamBehaviorDropDownBox.addEntry(getTeamBehaviorNameByID(TeamBehavior_Guard), TeamBehavior_Guard);
    teamBehaviorDropDownBox.addEntry(getTeamBehaviorNameByID(TeamBehavior_Kamikaze), TeamBehavior_Kamikaze);
    teamBehaviorDropDownBox.addEntry(getTeamBehaviorNameByID(TeamBehavior_Staging), TeamBehavior_Staging);
    teamBehaviorDropDownBox.addEntry(getTeamBehaviorNameByID(TeamBehavior_Flee), TeamBehavior_Flee);
    teamBehaviorDropDownBox.setSelectedItem(0);
    hBox2.addWidget(&teamBehaviorDropDownBox, 90);

    centralVBox.addWidget(VSpacer::create(3));

    centralVBox.addWidget(&hBox3);

    teamTypeLabel.setText(_("Team Type") + ":");
    teamTypeLabel.setTextColor(color);
    hBox3.addWidget(&teamTypeLabel, 95);
    teamTypeDropDownBox.setColor(color);
    teamTypeDropDownBox.setOnSelectionChange(std::bind(&TeamsWindow::onEntryChange, this, std::placeholders::_1));
    teamTypeDropDownBox.addEntry(_("Foot (Infantry, Troopers)"), TeamType_Foot);
    teamTypeDropDownBox.addEntry(_("Wheeled (Trike, Raider, Quad)"), TeamType_Wheeled);
    teamTypeDropDownBox.addEntry(_("Tracked (Tank, Launcher, Siege Tank,...)"), TeamType_Tracked);
    teamTypeDropDownBox.addEntry(_("Winged (Carryall, Ornithopter, Frigate)"), TeamType_Winged);
    teamTypeDropDownBox.addEntry(_("Slither (Sandworm)"), TeamType_Slither);
    teamTypeDropDownBox.addEntry(_("Harvester (Harvester)"), TeamType_Harvester);
    teamTypeDropDownBox.setSelectedItem(0);
    hBox3.addWidget(&teamTypeDropDownBox, 260);
    hBox3.addWidget(Spacer::create(), 5.0);
    minUnitsLabel.setText(_("Units") + ":");
    minUnitsLabel.setTextColor(color);
    hBox3.addWidget(&minUnitsLabel);
    minUnitsTextBox.setColor(house, color);
    minUnitsTextBox.setMinMax(0,99);
    minUnitsTextBox.setOnValueChange(std::bind(&TeamsWindow::onMinUnitsChange, this, std::placeholders::_1));
    hBox3.addWidget(&minUnitsTextBox, 47);
    maxUnitsLabel.setText(_("to"));
    maxUnitsLabel.setTextColor(color);
    hBox3.addWidget(&maxUnitsLabel, 30);
    maxUnitsTextBox.setColor(house, color);
    maxUnitsTextBox.setMinMax(0,99);
    maxUnitsTextBox.setOnValueChange(std::bind(&TeamsWindow::onMaxUnitsChange, this, std::placeholders::_1));
    hBox3.addWidget(&maxUnitsTextBox, 47);


    mainVBox.addWidget(VSpacer::create(5));

    mainVBox.addWidget(&buttonHBox);

    cancelButton.setText(_("Cancel"));
    cancelButton.setTextColor(color);
    cancelButton.setOnClick(std::bind(&TeamsWindow::onCancel, this));

    buttonHBox.addWidget(&cancelButton);

    buttonHBox.addWidget(HSpacer::create(8));

    buttonHBox.addWidget(Spacer::create());

    buttonHBox.addWidget(HSpacer::create(8));

    okButton.setText(_("OK"));
    okButton.setTextColor(color);
    okButton.setOnClick(std::bind(&TeamsWindow::onOK, this));

    buttonHBox.addWidget(&okButton);

    mainVBox.addWidget(VSpacer::create(10));

    // setup teams listbox
    for(const TeamInfo& teamInfo : teams) {
        teamsListBox.addEntry(getDescribingString(teamInfo));
    }

    if(teams.empty() == false) {
        teamsListBox.setSelectedItem(0);
        onSelectionChange(true);
    }
}

void TeamsWindow::onCancel() {
    Window* pParentWindow = dynamic_cast<Window*>(getParent());
    if(pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}


void TeamsWindow::onOK() {
    pMapEditor->startOperation();

    MapEditorChangeTeams changeTeamsOperation(teams);

    pMapEditor->addUndoOperation(changeTeamsOperation.perform(pMapEditor));

    Window* pParentWindow = dynamic_cast<Window*>(getParent());
    if(pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void TeamsWindow::onUp() {
    int index = teamsListBox.getSelectedIndex();

    if(index >= 1) {
        TeamInfo teamInfo = teams.at(index);
        teams.erase(teams.begin()+index);
        teamsListBox.removeEntry(index);

        teams.insert(teams.begin()+index-1,teamInfo);
        teamsListBox.insertEntry(index-1, getDescribingString(teamInfo));
        teamsListBox.setSelectedItem(index-1);
    }
}

void TeamsWindow::onDown() {
    int index = teamsListBox.getSelectedIndex();

    if((index >= 0) && (index < teamsListBox.getNumEntries()-1)) {
        TeamInfo teamInfo = teams.at(index);
        teams.erase(teams.begin()+index);
        teamsListBox.removeEntry(index);

        teams.insert(teams.begin()+index+1,teamInfo);
        teamsListBox.insertEntry(index+1, getDescribingString(teamInfo));
        teamsListBox.setSelectedItem(index+1);
    }
}

void TeamsWindow::onAdd() {
    if(pMapEditor->getMapVersion() < 2 && teamsListBox.getNumEntries() >= 16) {
        MsgBox* pMsgBox = MsgBox::create(_("Dune2-compatible maps support only up to 16 entries!"));
        pMsgBox->setTextColor(color);
        openWindow(pMsgBox);
        return;
    }

    int index = teamsListBox.getSelectedIndex();

    TeamInfo teamInfo(  playerDropDownBox.getSelectedEntryIntData(),
                        (TeamBehavior) teamBehaviorDropDownBox.getSelectedEntryIntData(),
                        (TeamType) teamTypeDropDownBox.getSelectedEntryIntData(),
                        minUnitsTextBox.getValue(),
                        maxUnitsTextBox.getValue());
    teams.insert(teams.begin()+index+1,teamInfo);
    teamsListBox.insertEntry(index+1, getDescribingString(teamInfo));
    teamsListBox.setSelectedItem(index+1);
}

void TeamsWindow::onRemove() {
    int index = teamsListBox.getSelectedIndex();

    if(index >= 0) {
        teams.erase(teams.begin()+index);
        teamsListBox.removeEntry(index);
        teamsListBox.setSelectedItem(index < teamsListBox.getNumEntries() ? index : (teamsListBox.getNumEntries()-1) );
    }
}

void TeamsWindow::onSelectionChange(bool bInteractive) {
    int index = teamsListBox.getSelectedIndex();

    if(index >= 0) {
        TeamInfo& teamInfo = teams.at(index);

        for(int i=0;i<playerDropDownBox.getNumEntries();i++) {
            if(playerDropDownBox.getEntryIntData(i) == teamInfo.houseID) {
                playerDropDownBox.setSelectedItem(i);
                break;
            }
        }

        for(int i=0;i<teamBehaviorDropDownBox.getNumEntries();i++) {
            if(teamBehaviorDropDownBox.getEntryIntData(i) == teamInfo.teamBehavior) {
                teamBehaviorDropDownBox.setSelectedItem(i);
                break;
            }
        }

        for(int i=0;i<teamTypeDropDownBox.getNumEntries();i++) {
            if(teamTypeDropDownBox.getEntryIntData(i) == teamInfo.teamType) {
                teamTypeDropDownBox.setSelectedItem(i);
                break;
            }
        }

        minUnitsTextBox.setValue(teamInfo.minUnits);
        maxUnitsTextBox.setValue(teamInfo.maxUnits);
    }
}

void TeamsWindow::onMinUnitsChange(bool bInteractive) {
    if(bInteractive) {
        int minUnits = minUnitsTextBox.getValue();
        int maxUnits = maxUnitsTextBox.getValue();

        if(minUnits > maxUnits) {
            maxUnitsTextBox.setValue(minUnits);
        }
    }

    onEntryChange(bInteractive);
}

void TeamsWindow::onMaxUnitsChange(bool bInteractive) {
    if(bInteractive) {
        int minUnits = minUnitsTextBox.getValue();
        int maxUnits = maxUnitsTextBox.getValue();

        if(minUnits > maxUnits) {
            minUnitsTextBox.setValue(maxUnits);
        }
    }

    onEntryChange(bInteractive);
}

void TeamsWindow::onEntryChange(bool bInteractive) {
    if(bInteractive) {
        int index = teamsListBox.getSelectedIndex();

        if(index >= 0) {
            TeamInfo& teamInfo = teams.at(index);
            teamInfo.houseID = playerDropDownBox.getSelectedEntryIntData();
            teamInfo.teamBehavior = (TeamBehavior) teamBehaviorDropDownBox.getSelectedEntryIntData();
            teamInfo.teamType = (TeamType) teamTypeDropDownBox.getSelectedEntryIntData();
            teamInfo.minUnits = minUnitsTextBox.getValue();
            teamInfo.maxUnits = maxUnitsTextBox.getValue();
            teamsListBox.setEntry(index, getDescribingString(teamInfo));
        }
    }
}

std::string TeamsWindow::getDescribingString(const TeamInfo& teamInfo) {

    return getPlayerName((HOUSETYPE) teamInfo.houseID) + ", "
            + getTeamBehaviorNameByID(teamInfo.teamBehavior) + ", "
            + getTeamTypeNameByID(teamInfo.teamType) + ", "
            + stringify(teamInfo.minUnits) + ", "
            + stringify(teamInfo.maxUnits);
}

std::string TeamsWindow::getPlayerName(HOUSETYPE house) {
    int currentPlayerNum = 1;
    for(const MapEditor::Player& player : pMapEditor->getPlayers()) {
        if(player.house == house) {
            return player.bAnyHouse ? fmt::sprintf(_("Player %d"), currentPlayerNum)
                                          : (_("House") + " " + player.name);
        }

        if(player.bActive && player.bAnyHouse) {
            currentPlayerNum++;
        }
    }

    return "";
}

