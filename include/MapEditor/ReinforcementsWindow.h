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

#ifndef REINFORCEMENTSWINDOW_H
#define REINFORCEMENTSWINDOW_H

#include <MapEditor/MapData.h>
#include <MapEditor/ReinforcementInfo.h>

#include <GUI/Window.h>
#include <GUI/HBox.h>
#include <GUI/VBox.h>
#include <GUI/Label.h>
#include <GUI/DropDownBox.h>
#include <GUI/TextButton.h>
#include <GUI/SymbolButton.h>
#include <GUI/ListBox.h>
#include <GUI/Checkbox.h>
#include <GUI/dune/DigitsTextBox.h>

#include <misc/SDL2pp.h>

// forward declaration
class MapEditor;


class  ReinforcementsWindow : public Window
{
public:

    ReinforcementsWindow(MapEditor* pMapEditor, HOUSETYPE currentHouse);


    /**
        This static method creates a dynamic reinforcements window.
        The idea behind this method is to simply create a new dialog on the fly and
        add it as a child window of some other window. If the window gets closed it will be freed.
        \param  pMapEditor  pointer to the currently running map editor
        \param  house       the currently selected house; used for button colors, etc.
        \return The new dialog box (will be automatically destroyed when it's closed)
    */
    static ReinforcementsWindow* create(MapEditor* pMapEditor, HOUSETYPE house) {
        ReinforcementsWindow* dlg = new ReinforcementsWindow(pMapEditor, house);
        dlg->pAllocated = true;
        return dlg;
    }


private:

    void onCancel();
    void onOK();

    void onUp();
    void onDown();

    void onAdd();
    void onRemove();

    void onEntryChange(bool bInteractive);

    void onSelectionChange(bool bInteractive);

    std::string getDescribingString(const ReinforcementInfo& reinforcementInfo);

    std::string getPlayerName(HOUSETYPE house);

    HBox            mainHBox;
    VBox            mainVBox;
    VBox            centralVBox;

    Label           Label_Explanation;

    HBox            hBox1;
    ListBox         reinforcementsListBox;
    VBox            listControlVBox;
    SymbolButton    listEntryUpButton;
    SymbolButton    listEntryDownButton;
    SymbolButton    addListEntryButton;
    SymbolButton    removeListEntryButton;

    HBox            hBox2;
    Label           playerLabel;
    DropDownBox     playerDropDownBox;
    Label           unitLabel;
    DropDownBox     unitDropDownBox;

    HBox            hBox3;
    Label           dropLocationLabel;
    DropDownBox     dropLocationDropDownBox;
    Label           timeLabel;
    DigitsTextBox   timeTextBox;
    Checkbox        repeatCheckbox;

    HBox            buttonHBox;

    Label           titleLabel;
    TextButton      cancelButton;
    TextButton      okButton;

    MapEditor*      pMapEditor;

    HOUSETYPE       house;
    Uint32          color;

    std::vector<ReinforcementInfo>  reinforcements;
};


#endif // REINFORCEMENTSWINDOW_H
