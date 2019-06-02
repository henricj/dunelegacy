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

#ifndef TEAMSWINDOW_H
#define TEAMSWINDOW_H

#include <MapEditor/MapData.h>
#include <AITeamInfo.h>

#include <GUI/Window.h>
#include <GUI/HBox.h>
#include <GUI/VBox.h>
#include <GUI/Label.h>
#include <GUI/DropDownBox.h>
#include <GUI/TextButton.h>
#include <GUI/SymbolButton.h>
#include <GUI/ListBox.h>
#include <GUI/dune/DigitsTextBox.h>

#include <misc/SDL2pp.h>

// forward declaration
class MapEditor;


class  TeamsWindow : public Window
{
public:

    TeamsWindow(MapEditor* pMapEditor, HOUSETYPE currentHouse);


    /**
        This static method creates a dynamic teams window.
        The idea behind this method is to simply create a new dialog on the fly and
        add it as a child window of some other window. If the window gets closed it will be freed.
        \param  pMapEditor  pointer to the currently running map editor
        \param  house       the currently selected house; used for button colors, etc.
        \return The new dialog box (will be automatically destroyed when it's closed)
    */
    static TeamsWindow* create(MapEditor* pMapEditor, HOUSETYPE house) {
        TeamsWindow* dlg = new TeamsWindow(pMapEditor, house);
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

    void onMinUnitsChange(bool bInteractive);

    void onMaxUnitsChange(bool bInteractive);

    std::string getDescribingString(const AITeamInfo& aiteamInfo);

    std::string getPlayerName(HOUSETYPE house);


    HBox            mainHBox;
    VBox            mainVBox;
    VBox            centralVBox;

    HBox            hBox1;
    ListBox         teamsListBox;
    VBox            listControlVBox;
    SymbolButton    listEntryUpButton;
    SymbolButton    listEntryDownButton;
    SymbolButton    addListEntryButton;
    SymbolButton    removeListEntryButton;

    HBox            hBox2;
    Label           playerLabel;
    DropDownBox     playerDropDownBox;
    Label           aiTeamBehaviorLabel;
    DropDownBox     aiTeamBehaviorDropDownBox;

    HBox            hBox3;
    Label           aiTeamTypeLabel;
    DropDownBox     aiTeamTypeDropDownBox;
    Label           minUnitsLabel;
    DigitsTextBox   minUnitsTextBox;
    Label           maxUnitsLabel;
    DigitsTextBox   maxUnitsTextBox;

    HBox            buttonHBox;

    Label           titleLabel;
    TextButton      cancelButton;
    TextButton      okButton;

    MapEditor*      pMapEditor;

    HOUSETYPE       house;
    Uint32          color;

    std::vector<AITeamInfo>  aiteams;
};


#endif // TEAMSWINDOW_H
