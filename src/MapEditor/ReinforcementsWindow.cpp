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

#include <MapEditor/ReinforcementsWindow.h>

#include <GUI/Spacer.h>
#include <GUI/MsgBox.h>

#include <globals.h>

#include <misc/draw_util.h>
#include <misc/format.h>

#include <sand.h>

#include <MapEditor/MapEditor.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>

ReinforcementsWindow::ReinforcementsWindow(MapEditor* pMapEditor, HOUSETYPE currentHouse)
 : Window(0,0,0,0), pMapEditor(pMapEditor), house(currentHouse), reinforcements(pMapEditor->getReinforcements()) {

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
    titleLabel.setText(_("Reinforcements"));
    mainVBox.addWidget(&titleLabel);

    mainVBox.addWidget(VSpacer::create(8));

    mainVBox.addWidget(&centralVBox, 360);

    Label_Explanation.setTextColor(color);
    Label_Explanation.setTextFontSize(12);
    Label_Explanation.setText(_("Reinforcements are brought by a carryall. Multiple reinforcements at the same time are combined."));
    centralVBox.addWidget(&Label_Explanation);

    centralVBox.addWidget(VSpacer::create(4));


    centralVBox.addWidget(&hBox1, 6.0);

    reinforcementsListBox.setColor(color);
    reinforcementsListBox.setOnSelectionChange(std::bind(&ReinforcementsWindow::onSelectionChange, this, std::placeholders::_1));
    hBox1.addWidget(&reinforcementsListBox, 1.0);

    hBox1.addWidget(HSpacer::create(3));

    listEntryUpButton.setSymbol(pGFXManager->getUIGraphicSurface(UI_MapEditor_ArrowUp, house),
                                pGFXManager->getUIGraphicSurface(UI_MapEditor_ArrowUp_Active, house));
    listEntryUpButton.setTooltipText(_("Move up"));
    listEntryUpButton.setOnClick(std::bind(&ReinforcementsWindow::onUp, this));
    listControlVBox.addWidget(&listEntryUpButton, 25);
    listControlVBox.addWidget(VSpacer::create(3));
    listEntryDownButton.setSymbol(pGFXManager->getUIGraphicSurface(UI_MapEditor_ArrowDown, house),
                                  pGFXManager->getUIGraphicSurface(UI_MapEditor_ArrowDown_Active, house));
    listEntryDownButton.setTooltipText(_("Move down"));
    listEntryDownButton.setOnClick(std::bind(&ReinforcementsWindow::onDown, this));
    listControlVBox.addWidget(&listEntryDownButton, 25);

    listControlVBox.addWidget(Spacer::create(), 6.0);

    addListEntryButton.setSymbol(pGFXManager->getUIGraphicSurface(UI_MapEditor_Plus, house),
                                 pGFXManager->getUIGraphicSurface(UI_MapEditor_Plus_Active, house));
    addListEntryButton.setTooltipText(_("Add"));
    addListEntryButton.setOnClick(std::bind(&ReinforcementsWindow::onAdd, this));
    listControlVBox.addWidget(&addListEntryButton, 25);
    listControlVBox.addWidget(VSpacer::create(3));
    removeListEntryButton.setSymbol(pGFXManager->getUIGraphicSurface(UI_MapEditor_Minus, house),
                                    pGFXManager->getUIGraphicSurface(UI_MapEditor_Minus_Active, house));
    removeListEntryButton.setTooltipText(_("Remove"));
    removeListEntryButton.setOnClick(std::bind(&ReinforcementsWindow::onRemove, this));
    listControlVBox.addWidget(&removeListEntryButton, 25);

    hBox1.addWidget(&listControlVBox, 25);

    centralVBox.addWidget(VSpacer::create(3));

    centralVBox.addWidget(&hBox2);

    playerLabel.setText(_("Player") + ":");
    playerLabel.setTextColor(color);
    hBox2.addWidget(&playerLabel, 120);
    playerDropDownBox.setColor(color);
    playerDropDownBox.setOnSelectionChange(std::bind(&ReinforcementsWindow::onEntryChange, this, std::placeholders::_1));

    int currentPlayerNum = 1;
    for(const MapEditor::Player& player : pMapEditor->getPlayers()) {
        std::string entryName = player.bActive ? (player.bAnyHouse ? fmt::sprintf(_("Player %d"), currentPlayerNum++)
                                                    : player.name) : ("(" + player.name + ")");
        playerDropDownBox.addEntry(entryName, player.house);
    }
    playerDropDownBox.setSelectedItem(0);
    hBox2.addWidget(&playerDropDownBox, 120);
    hBox2.addWidget(HSpacer::create(15));
    unitLabel.setText(_("Unit") + ":");
    unitLabel.setTextColor(color);
    hBox2.addWidget(&unitLabel, 125);
    unitDropDownBox.setColor(color);
    unitDropDownBox.setOnSelectionChange(std::bind(&ReinforcementsWindow::onEntryChange, this, std::placeholders::_1));
    for(int itemID = Unit_FirstID; itemID <= Unit_LastID; ++itemID) {
        if(itemID == Unit_Carryall || itemID == Unit_Ornithopter || itemID == Unit_Frigate) {
            continue;
        }
        unitDropDownBox.addEntry(resolveItemName(itemID), itemID);
    }
    unitDropDownBox.setSelectedItem(0);
    hBox2.addWidget(&unitDropDownBox);

    centralVBox.addWidget(VSpacer::create(3));

    centralVBox.addWidget(&hBox3);

    dropLocationLabel.setText(_("Drop Location") + ":");
    dropLocationLabel.setTextColor(color);
    hBox3.addWidget(&dropLocationLabel, 120);
    dropLocationDropDownBox.setColor(color);
    dropLocationDropDownBox.setOnSelectionChange(std::bind(&ReinforcementsWindow::onEntryChange, this, std::placeholders::_1));
    dropLocationDropDownBox.addEntry(resolveDropLocationName(Drop_North), Drop_North);
    dropLocationDropDownBox.addEntry(resolveDropLocationName(Drop_East), Drop_East);
    dropLocationDropDownBox.addEntry(resolveDropLocationName(Drop_South), Drop_South);
    dropLocationDropDownBox.addEntry(resolveDropLocationName(Drop_West), Drop_West);
    dropLocationDropDownBox.addEntry(resolveDropLocationName(Drop_Air), Drop_Air);
    dropLocationDropDownBox.addEntry(resolveDropLocationName(Drop_Visible), Drop_Visible);
    dropLocationDropDownBox.addEntry(resolveDropLocationName(Drop_Enemybase), Drop_Enemybase);
    dropLocationDropDownBox.addEntry(resolveDropLocationName(Drop_Homebase), Drop_Homebase);
    dropLocationDropDownBox.setSelectedItem(7);
    hBox3.addWidget(&dropLocationDropDownBox, 120);
    hBox3.addWidget(HSpacer::create(15));
    timeLabel.setText(_("Time") + " (min):");
    timeLabel.setTextColor(color);
    hBox3.addWidget(&timeLabel, 125);
    timeTextBox.setColor(house, color);
    timeTextBox.setMinMax(0,999);
    timeTextBox.setOnValueChange(std::bind(&ReinforcementsWindow::onEntryChange, this, std::placeholders::_1));
    hBox3.addWidget(&timeTextBox, 50);
    hBox3.addWidget(HSpacer::create(12));
    repeatCheckbox.setText(_("Repeat"));
    repeatCheckbox.setTextColor(color);
    repeatCheckbox.setOnClick(std::bind(&ReinforcementsWindow::onEntryChange, this, true));
    hBox3.addWidget(&repeatCheckbox);


    mainVBox.addWidget(VSpacer::create(5));

    mainVBox.addWidget(&buttonHBox);

    cancelButton.setText(_("Cancel"));
    cancelButton.setTextColor(color);
    cancelButton.setOnClick(std::bind(&ReinforcementsWindow::onCancel, this));

    buttonHBox.addWidget(&cancelButton);

    buttonHBox.addWidget(HSpacer::create(8));

    buttonHBox.addWidget(Spacer::create());

    buttonHBox.addWidget(HSpacer::create(8));

    okButton.setText(_("OK"));
    okButton.setTextColor(color);
    okButton.setOnClick(std::bind(&ReinforcementsWindow::onOK, this));

    buttonHBox.addWidget(&okButton);

    mainVBox.addWidget(VSpacer::create(10));

    // setup reinforcements listbox
    for(const ReinforcementInfo& reinforcement : reinforcements) {
        reinforcementsListBox.addEntry(getDescribingString(reinforcement));
    }

    if(reinforcements.empty() == false) {
        reinforcementsListBox.setSelectedItem(0);
        onSelectionChange(true);
    }
}

void ReinforcementsWindow::onCancel() {
    Window* pParentWindow = dynamic_cast<Window*>(getParent());
    if(pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}


void ReinforcementsWindow::onOK() {
    pMapEditor->startOperation();

    MapEditorChangeReinforcements changeReinforcementsOperation(reinforcements);

    pMapEditor->addUndoOperation(changeReinforcementsOperation.perform(pMapEditor));

    Window* pParentWindow = dynamic_cast<Window*>(getParent());
    if(pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void ReinforcementsWindow::onUp() {
    int index = reinforcementsListBox.getSelectedIndex();

    if(index >= 1) {
        ReinforcementInfo reinforcementInfo = reinforcements.at(index);
        reinforcements.erase(reinforcements.begin()+index);
        reinforcementsListBox.removeEntry(index);

        reinforcements.insert(reinforcements.begin()+index-1,reinforcementInfo);
        reinforcementsListBox.insertEntry(index-1, getDescribingString(reinforcementInfo));
        reinforcementsListBox.setSelectedItem(index-1);
    }
}

void ReinforcementsWindow::onDown() {
    int index = reinforcementsListBox.getSelectedIndex();

    if((index >= 0) && (index < reinforcementsListBox.getNumEntries()-1)) {
        ReinforcementInfo reinforcementInfo = reinforcements.at(index);
        reinforcements.erase(reinforcements.begin()+index);
        reinforcementsListBox.removeEntry(index);

        reinforcements.insert(reinforcements.begin()+index+1,reinforcementInfo);
        reinforcementsListBox.insertEntry(index+1, getDescribingString(reinforcementInfo));
        reinforcementsListBox.setSelectedItem(index+1);
    }
}

void ReinforcementsWindow::onAdd() {
    if(pMapEditor->getMapVersion() < 2 && reinforcementsListBox.getNumEntries() >= 16) {
        MsgBox* pMsgBox = MsgBox::create(_("Dune2-compatible maps support only up to 16 entries!"));
        pMsgBox->setTextColor(color);
        openWindow(pMsgBox);
        return;
    }

    int index = reinforcementsListBox.getSelectedIndex();

    ReinforcementInfo reinforcementInfo(playerDropDownBox.getSelectedEntryIntData(),
                                        unitDropDownBox.getSelectedEntryIntData(),
                                        (DropLocation) dropLocationDropDownBox.getSelectedEntryIntData(),
                                        timeTextBox.getValue(),
                                        repeatCheckbox.isChecked());
    reinforcements.insert(reinforcements.begin()+index+1,reinforcementInfo);
    reinforcementsListBox.insertEntry(index+1, getDescribingString(reinforcementInfo));
    reinforcementsListBox.setSelectedItem(index+1);
}

void ReinforcementsWindow::onRemove() {
    int index = reinforcementsListBox.getSelectedIndex();

    if(index >= 0) {
        reinforcements.erase(reinforcements.begin()+index);
        reinforcementsListBox.removeEntry(index);
        reinforcementsListBox.setSelectedItem(index < reinforcementsListBox.getNumEntries() ? index : (reinforcementsListBox.getNumEntries()-1) );
    }
}

void ReinforcementsWindow::onSelectionChange(bool bInteractive) {
    int index = reinforcementsListBox.getSelectedIndex();

    if(index >= 0) {
        ReinforcementInfo& reinforcementInfo = reinforcements.at(index);

        for(int i=0;i<playerDropDownBox.getNumEntries();i++) {
            if(playerDropDownBox.getEntryIntData(i) == reinforcementInfo.houseID) {
                playerDropDownBox.setSelectedItem(i);
                break;
            }
        }

        for(int i=0;i<unitDropDownBox.getNumEntries();i++) {
            if(unitDropDownBox.getEntryIntData(i) == reinforcementInfo.unitID) {
                unitDropDownBox.setSelectedItem(i);
                break;
            }
        }

        for(int i=0;i<dropLocationDropDownBox.getNumEntries();i++) {
            if(dropLocationDropDownBox.getEntryIntData(i) == reinforcementInfo.dropLocation) {
                dropLocationDropDownBox.setSelectedItem(i);
                break;
            }
        }

        timeTextBox.setValue(reinforcementInfo.droptime);

        repeatCheckbox.setChecked(reinforcementInfo.bRepeat);
    }
}

void ReinforcementsWindow::onEntryChange(bool bInteractive) {
    if(bInteractive) {
        int index = reinforcementsListBox.getSelectedIndex();

        if(index >= 0) {
            ReinforcementInfo& reinforcementInfo = reinforcements.at(index);
            reinforcementInfo.houseID = playerDropDownBox.getSelectedEntryIntData();
            reinforcementInfo.unitID = unitDropDownBox.getSelectedEntryIntData();
            reinforcementInfo.dropLocation = (DropLocation) dropLocationDropDownBox.getSelectedEntryIntData();
            reinforcementInfo.droptime = timeTextBox.getValue();
            reinforcementInfo.bRepeat = repeatCheckbox.isChecked();
            reinforcementsListBox.setEntry(index, getDescribingString(reinforcementInfo));
        }
    }
}

std::string ReinforcementsWindow::getDescribingString(const ReinforcementInfo& reinforcementInfo) {

    return getPlayerName((HOUSETYPE) reinforcementInfo.houseID) + ", "
            + resolveItemName(reinforcementInfo.unitID) + ", "
            + resolveDropLocationName(reinforcementInfo.dropLocation) + ", "
            + std::to_string(reinforcementInfo.droptime) + " min"
            + (reinforcementInfo.bRepeat ? ", +" : "");
}

std::string ReinforcementsWindow::getPlayerName(HOUSETYPE house) {
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
