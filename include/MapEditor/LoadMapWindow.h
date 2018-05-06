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

#ifndef LOADMAPWINDOW_H
#define LOADMAPWINDOW_H

#include <MapEditor/MapData.h>

#include <GUI/Window.h>
#include <GUI/HBox.h>
#include <GUI/VBox.h>
#include <GUI/Label.h>
#include <GUI/TextButton.h>
#include <GUI/ListBox.h>
#include <GUI/PictureLabel.h>
#include <misc/SDL2pp.h>

class  LoadMapWindow : public Window
{
public:

    explicit LoadMapWindow(Uint32 color = COLOR_DEFAULT);

    const std::string& getLoadMapFilepath() const { return loadMapFilepath; };
    const std::string& getLoadMapname() const { return loadMapname; };
    bool isLoadMapSingleplayer() const { return loadMapSingleplayer; };

    bool handleKeyPress(SDL_KeyboardEvent& key) override;

    /**
        This method is called, when the child window is about to be closed.
        This child window will be closed after this method returns.
        \param  pChildWindow    The child window that will be closed
    */
    void onChildWindowClose(Window* pChildWindow) override;

    /**
        This static method creates a dynamic load map window.
        The idea behind this method is to simply create a new dialog on the fly and
        add it as a child window of some other window. If the window gets closed it will be freed.
        \return The new dialog box (will be automatically destroyed when it's closed)
    */
    static LoadMapWindow* create(int color = -1) {
        LoadMapWindow* dlg = new LoadMapWindow(color);
        dlg->pAllocated = true;
        return dlg;
    }


private:
    void onCancel();
    void onLoad();
    void onMapTypeChange(int buttonID);
    void onMapListSelectionChange(bool bInteractive);

    HBox    mainHBox;
    VBox    mainVBox;
    HBox    centralHBox;

    // left VBox with map list
    VBox            leftVBox;
    HBox            mapTypeButtonsHBox;
    TextButton      singleplayerUserMapsButton;
    TextButton      multiplayerUserMapsButton;
    ListBox         mapList;

    // right VBox with mini map
    VBox            rightVBox;
    PictureLabel    minimap;
    HBox            mapPropertiesHBox;
    VBox            mapPropertyNamesVBox;
    VBox            mapPropertyValuesVBox;
    Label           mapPropertySize;
    Label           mapPropertyPlayers;
    Label           mapPropertyAuthors;
    Label           mapPropertyLicense;

    HBox            buttonHBox;

    Label       titleLabel;
    TextButton  cancelButton;
    TextButton  loadButton;

    Uint32      color;

    std::string loadMapFilepath;
    std::string loadMapname;
    bool        loadMapSingleplayer;
    std::string currentMapDirectory;
};


#endif // LOADMAPWINDOW_H
