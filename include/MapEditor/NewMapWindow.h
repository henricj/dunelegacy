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

#ifndef NEWMAPWINDOW_H
#define NEWMAPWINDOW_H

#include <MapEditor/MapData.h>

#include <GUI/DropDownBox.h>
#include <GUI/HBox.h>
#include <GUI/Label.h>
#include <GUI/PictureLabel.h>
#include <GUI/RadioButton.h>
#include <GUI/TextButton.h>
#include <GUI/VBox.h>
#include <GUI/Window.h>
#include <GUI/dune/DigitsTextBox.h>

#include <misc/SDL2pp.h>

class NewMapWindow final : public Window {
public:
    explicit NewMapWindow(HOUSETYPE currentHouse);

    /**
        This static method creates a dynamic new map window.
        The idea behind this method is to simply create a new dialog on the fly and
        add it as a child window of some other window. If the window gets closed it will be freed.
        \param  house       the currently selected house; used for button colors, etc.
        \return The new dialog box (will be automatically destroyed when it's closed)
    */
    static NewMapWindow* create(HOUSETYPE house) {
        auto* dlg        = new NewMapWindow(house);
        dlg->pAllocated_ = true;
        return dlg;
    }

    [[nodiscard]] int getMapSeed() const { return mapSeed; }

    [[nodiscard]] std::string getAuthor() const { return std::string{authorTextBox.getText()}; }
    [[nodiscard]] std::string getLicense() const { return std::string{licenseTextBox.getText()}; }

    [[nodiscard]] const auto& getLoadMapFilepath() const { return loadMapFilepath; }
    [[nodiscard]] const auto& getLoadMapname() const { return loadMapname; }
    [[nodiscard]] bool isLoadMapSingleplayer() const { return loadMapSingleplayer; }

    [[nodiscard]] const MapData& getMapData() const { return mapdata; }

    /**
        This method is called, when the child window is about to be closed.
        This child window will be closed after this method returns.
        \param  pChildWindow    The child window that will be closed
    */
    void onChildWindowClose(Window* pChildWindow) override;

private:
    void onMapPropertiesChanged();

    static sdl2::surface_ptr createMinimapPicture(MapData& mapdata, int borderWidth, uint32_t borderColor);

    void onCancel();
    void onLoad();
    void onCreate();

    void onMapTypeChanged(int buttonID);

    HBox mainHBox;
    VBox mainVBox;
    VBox centralVBox;

    HBox mapTypeHBox;
    RadioButtonManager mapTypeRadioButtons;
    RadioButton emptyMapRadioButton;
    RadioButton randomMapRadioButton;
    RadioButton seedMapRadioButton;

    HBox basicMapPropertiesHBox;
    VBox basicMapPropertiesVBox;

    HBox mapSizeHBox;
    Label mapSizeXLabel;
    DropDownBox mapSizeXDropDownBox;
    Label mapSizeYLabel;
    DropDownBox mapSizeYDropDownBox;
    Label mapScaleLabel;
    DropDownBox mapScaleDropDownBox;

    HBox rngHBox;
    Label rngSeedLabel;
    DigitsTextBox rngSeedTextBox;

    Label rockLabel;
    DigitsTextBox rockDigitsTextBox;

    Label spiceLabel;
    DigitsTextBox spiceDigitsTextBox;

    PictureLabel minimap;

    HBox mirrorModeHBox;
    Label mirrorModeLabel;
    DropDownBox mirrorModeDropDownBox;

    HBox authorHBox;
    Label authorLabel;
    TextBox authorTextBox;
    HBox licenseHBox;
    Label licenseLabel;
    TextBox licenseTextBox;

    HBox buttonHBox;

    Label titleLabel;
    TextButton cancelButton;
    TextButton loadButton;
    TextButton createButton;

    HOUSETYPE house;
    uint32_t color;

    int mapSeed = INVALID;
    MapData mapdata;
    std::filesystem::path loadMapFilepath;
    std::string loadMapname;
    bool loadMapSingleplayer = false;
};

#endif // NEWMAPWINDOW_H
