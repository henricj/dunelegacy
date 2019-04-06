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

#include <MapEditor/ChoamWindow.h>

#include <GUI/Spacer.h>

#include <globals.h>

#include <sand.h>

#include <MapEditor/MapEditor.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>

static const ItemID_enum choamUnits[] = { Unit_Carryall,  Unit_Ornithopter,
                                           Unit_Harvester, Unit_MCV,
                                           Unit_Trike,     Unit_RaiderTrike,
                                           Unit_Quad,      Unit_Tank,
                                           Unit_Launcher,  Unit_SiegeTank,
                                           Unit_Devastator,Unit_Deviator,
                                           Unit_SonicTank, ItemID_Invalid
                                          };

ChoamWindow::ChoamWindow(MapEditor* pMapEditor, HOUSETYPE currentHouse) : Window(0,0,0,0), pMapEditor(pMapEditor), house(currentHouse) {

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
    titleLabel.setText(_("Choam"));
    mainVBox.addWidget(&titleLabel);

    mainVBox.addWidget(VSpacer::create(8));

    mainVBox.addWidget(&centralVBox, 360);

    Label_Explanation.setTextColor(color);
    Label_Explanation.setTextFontSize(12);
    Label_Explanation.setText(_("These units are available at the starport. The given amount of these units is available at the start of the game and every 30s one unit is added."));
    centralVBox.addWidget(&Label_Explanation);

    centralVBox.addWidget(VSpacer::create(4));

    std::map<int,int>& choam = pMapEditor->getChoam();

    for(int i=0;i<7;i++) {

        ItemID_enum unit1 = choamUnits[i*2];
        ItemID_enum unit2 = choamUnits[i*2+1];

        centralVBox.addWidget(VSpacer::create(2));

        choamRows[i].Checkbox_Unit1.setText(resolveItemName(unit1));
        choamRows[i].Checkbox_Unit1.setTextColor(color);
        choamRows[i].Checkbox_Unit1.setOnClick(std::bind(&ChoamWindow::onUnitCheckbox, this, unit1));
        choamRows[i].Checkbox_Unit1.setChecked(choam.count(unit1) > 0);
        choamRows[i].HBox_Unit.addWidget(&choamRows[i].Checkbox_Unit1, 180);

        choamRows[i].TextBox_Unit1.setColor(house, color);
        choamRows[i].TextBox_Unit1.setMinMax(0,1000);
        choamRows[i].TextBox_Unit1.setValue((choam.count(unit1) > 0) ? choam[unit1] : 0);
        choamRows[i].TextBox_Unit1.setVisible(choam.count(unit1) > 0);
        choamRows[i].HBox_Unit.addWidget(&choamRows[i].TextBox_Unit1, 65);

        choamRows[i].HBox_Unit.addWidget(Spacer::create(), 20.0);

        if(unit2 != ItemID_Invalid) {
            choamRows[i].Checkbox_Unit2.setText(resolveItemName(unit2));
            choamRows[i].Checkbox_Unit2.setTextColor(color);
            choamRows[i].Checkbox_Unit2.setOnClick(std::bind(&ChoamWindow::onUnitCheckbox, this, unit2));
            choamRows[i].Checkbox_Unit2.setChecked(choam.count(unit2) > 0);
            choamRows[i].HBox_Unit.addWidget(&choamRows[i].Checkbox_Unit2, 180);

            choamRows[i].TextBox_Unit2.setColor(house, color);
            choamRows[i].TextBox_Unit2.setMinMax(0,1000);
            choamRows[i].TextBox_Unit2.setValue((choam.count(unit2) > 0) ? choam[unit2] : 0);
            choamRows[i].TextBox_Unit2.setVisible(choam.count(unit2) > 0);
            choamRows[i].HBox_Unit.addWidget(&choamRows[i].TextBox_Unit2, 65);
        }

        centralVBox.addWidget(&choamRows[i].HBox_Unit);

    }

    centralVBox.addWidget(Spacer::create());

    mainVBox.addWidget(VSpacer::create(5));

    mainVBox.addWidget(&buttonHBox);

    cancelButton.setText(_("Cancel"));
    cancelButton.setTextColor(color);
    cancelButton.setOnClick(std::bind(&ChoamWindow::onCancel, this));

    buttonHBox.addWidget(&cancelButton);

    buttonHBox.addWidget(HSpacer::create(8));

    buttonHBox.addWidget(Spacer::create());

    buttonHBox.addWidget(HSpacer::create(8));

    okButton.setText(_("OK"));
    okButton.setTextColor(color);
    okButton.setOnClick(std::bind(&ChoamWindow::onOK, this));

    buttonHBox.addWidget(&okButton);

    mainVBox.addWidget(VSpacer::create(10));
}

void ChoamWindow::onCancel() {
    Window* pParentWindow = dynamic_cast<Window*>(getParent());
    if(pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}


void ChoamWindow::onOK() {

    pMapEditor->startOperation();

    for(unsigned int i = 0; i < sizeof(choamUnits)/sizeof(choamUnits[0]); i++) {
        int rowNum = i/2;
        int itemID = choamUnits[i];

        if(itemID != ItemID_Invalid) {

            bool bChecked = false;
            int amount = 0;

            if((i%2) == 0) {
                bChecked = choamRows[rowNum].Checkbox_Unit1.isChecked();
                amount = choamRows[rowNum].TextBox_Unit1.getValue();
            } else {
                bChecked = choamRows[rowNum].Checkbox_Unit2.isChecked();
                amount = choamRows[rowNum].TextBox_Unit2.getValue();
            }

            if(bChecked == false) {
                // set amount to -1 as we want to remove this item from choam
                // (THIS IS DIFFERENT TO -1 IN THE INI FILE WHERE -1 MEANS AN AMOUNT OF 0)
                amount = -1;
            }

            MapEditorChangeChoam changeChoamOperation(itemID, amount);

            pMapEditor->addUndoOperation(changeChoamOperation.perform(pMapEditor));
        }
    }


    Window* pParentWindow = dynamic_cast<Window*>(getParent());
    if(pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void ChoamWindow::onUnitCheckbox(int itemID) {
    for(unsigned int i = 0; i < sizeof(choamUnits)/sizeof(choamUnits[0]); i++) {
        if(choamUnits[i] == itemID) {
            int rowNum = i/2;

            if((i%2) == 0) {
                choamRows[rowNum].TextBox_Unit1.setVisible(choamRows[rowNum].Checkbox_Unit1.isChecked());
            } else {
                choamRows[rowNum].TextBox_Unit2.setVisible(choamRows[rowNum].Checkbox_Unit2.isChecked());
            }

            break;
        }
    }
}
