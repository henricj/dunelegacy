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

#ifndef CHOAMWINDOW_H
#define CHOAMWINDOW_H

#include <MapEditor/MapData.h>

#include <GUI/Window.h>
#include <GUI/HBox.h>
#include <GUI/VBox.h>
#include <GUI/Label.h>
#include <GUI/DropDownBox.h>
#include <GUI/RadioButton.h>
#include <GUI/TextButton.h>
#include <GUI/Checkbox.h>
#include <GUI/dune/DigitsTextBox.h>
#include <misc/SDL2pp.h>

// forward declaration
class MapEditor;


class  ChoamWindow : public Window
{
public:

    ChoamWindow(MapEditor* pMapEditor, HOUSETYPE currentHouse);


    /**
        This static method creates a dynamic choam window.
        The idea behind this method is to simply create a new dialog on the fly and
        add it as a child window of some other window. If the window gets closed it will be freed.
        \param  pMapEditor  pointer to the currently running map editor
        \param  house       the currently selected house; used for button colors, etc.
        \return The new dialog box (will be automatically destroyed when it's closed)
    */
    static ChoamWindow* create(MapEditor* pMapEditor, HOUSETYPE house) {
        ChoamWindow* dlg = new ChoamWindow(pMapEditor, house);
        dlg->pAllocated = true;
        return dlg;
    }


private:

    void onCancel();
    void onOK();

    void onUnitCheckbox(int itemID);


    HBox    mainHBox;
    VBox    mainVBox;
    VBox    centralVBox;

    Label   Label_Explanation;


    struct ChoamRow {
        HBox            HBox_Unit;
        Checkbox        Checkbox_Unit1;
        DigitsTextBox   TextBox_Unit1;
        Checkbox        Checkbox_Unit2;
        DigitsTextBox   TextBox_Unit2;
    };

    ChoamRow   choamRows[7];

    HBox        buttonHBox;

    Label       titleLabel;
    TextButton  cancelButton;
    TextButton  okButton;

    MapEditor*  pMapEditor;

    HOUSETYPE   house;
    Uint32      color;
};


#endif // CHOAMWINDOW_H
