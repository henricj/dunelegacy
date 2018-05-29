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

#include <MapEditor/NewMapWindow.h>
#include <MapEditor/LoadMapWindow.h>

#include <GUI/Spacer.h>
#include <GUI/dune/DuneStyle.h>

#include <globals.h>

#include <sand.h>
#include <mmath.h>

#include <MapEditor/MapGenerator.h>
#include <MapSeed.h>

#include <misc/draw_util.h>

#include <RadarViewBase.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>

NewMapWindow::NewMapWindow(HOUSETYPE currentHouse) : Window(0,0,0,0), house(currentHouse), mapSeed(INVALID), loadMapSingleplayer(false) {

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
    titleLabel.setText(_("New Map"));
    mainVBox.addWidget(&titleLabel);

    mainVBox.addWidget(VSpacer::create(8));

    mainVBox.addWidget(&centralVBox, 360);

    basicMapPropertiesHBox.addWidget(&basicMapPropertiesVBox);
    minimap.setSurface( GUIStyle::getInstance().createButtonSurface(130,130,"", true, false) );
    basicMapPropertiesHBox.addWidget(&minimap);


    emptyMapRadioButton.setText(_("Empty Map"));
    emptyMapRadioButton.setTextColor(color);
    emptyMapRadioButton.setOnClick(std::bind(&NewMapWindow::onMapTypeChanged, this, 0));
    mapTypeHBox.addWidget(&emptyMapRadioButton);
    randomMapRadioButton.setText(_("Random Map"));
    randomMapRadioButton.setTextColor(color);
    randomMapRadioButton.setOnClick(std::bind(&NewMapWindow::onMapTypeChanged, this, 1));
    mapTypeHBox.addWidget(&randomMapRadioButton);
    seedMapRadioButton.setText(_("Dune2-compatible  Map"));
    seedMapRadioButton.setTextColor(color);
    seedMapRadioButton.setOnClick(std::bind(&NewMapWindow::onMapTypeChanged, this, 2));
    mapTypeHBox.addWidget(&seedMapRadioButton);
    mapTypeRadioButtons.registerRadioButtons(3, &emptyMapRadioButton, &randomMapRadioButton, &seedMapRadioButton);
    mapTypeHBox.addWidget(Spacer::create(),5.0);
    basicMapPropertiesVBox.addWidget(&mapTypeHBox);

    basicMapPropertiesVBox.addWidget(VSpacer::create(4));
    basicMapPropertiesVBox.addWidget(&mapSizeHBox);

    // Widgets for MapSize_HBox (will be added when radio button is changed)
    mapSizeXLabel.setText(_("Map Width:"));
    mapSizeXLabel.setTextColor(color);
    mapSizeXDropDownBox.setColor(color);
    mapSizeXDropDownBox.addEntry("32",32);
    mapSizeXDropDownBox.addEntry("64",64);
    mapSizeXDropDownBox.addEntry("128",128);
    mapSizeXDropDownBox.setSelectedItem(2);
    mapSizeXDropDownBox.setOnSelectionChange(std::bind(&NewMapWindow::onMapPropertiesChanged,this));
    mapSizeYLabel.setText(_("Map Height:"));
    mapSizeYLabel.setTextColor(color);
    mapSizeYDropDownBox.setColor(color);
    mapSizeYDropDownBox.addEntry("32",32);
    mapSizeYDropDownBox.addEntry("64",64);
    mapSizeYDropDownBox.addEntry("128",128);
    mapSizeYDropDownBox.setSelectedItem(2);
    mapSizeYDropDownBox.setOnSelectionChange(std::bind(&NewMapWindow::onMapPropertiesChanged,this));


    mapScaleLabel.setText(_("Map Scale:"));
    mapScaleLabel.setTextColor(color);
    mapScaleDropDownBox.setColor(color);
    mapScaleDropDownBox.addEntry(_("Tiny (21x21)"),2);
    mapScaleDropDownBox.addEntry(_("Small (32x32)"),1);
    mapScaleDropDownBox.addEntry(_("Normal (62x62)"),0);
    mapScaleDropDownBox.setSelectedItem(2);
    mapScaleDropDownBox.setOnSelectionChange(std::bind(&NewMapWindow::onMapPropertiesChanged,this));

    basicMapPropertiesVBox.addWidget(VSpacer::create(4));
    basicMapPropertiesVBox.addWidget(&rngHBox);

    rngSeedLabel.setText(_("Random Seed:"));
    rngSeedLabel.setTextColor(color);
    rngHBox.addWidget(&rngSeedLabel);
    rngSeedTextBox.setMinMax(0, 32767);
    rngSeedTextBox.setValue(getRandomInt(0, 32768));
    rngSeedTextBox.setColor(house, color);
    rngSeedTextBox.setOnValueChange(std::bind(&NewMapWindow::onMapPropertiesChanged,this));
    rngHBox.addWidget(&rngSeedTextBox,80);

    rngHBox.addWidget(HSpacer::create(2));
    rngHBox.addWidget(Spacer::create(),3.0);

    rockLabel.setText(_("Rock:"));
    rockLabel.setTextColor(color);
    rngHBox.addWidget(&rockLabel);
    rockDigitsTextBox.setMinMax(0, 99);
    rockDigitsTextBox.setValue(ROCKFIELDS);
    rockDigitsTextBox.setColor(house, color);
    rockDigitsTextBox.setOnValueChange(std::bind(&NewMapWindow::onMapPropertiesChanged,this));
    rngHBox.addWidget(&rockDigitsTextBox,45);

    rngHBox.addWidget(HSpacer::create(2));
    rngHBox.addWidget(Spacer::create(),3.0);

    spiceLabel.setText(_("Spice:"));
    spiceLabel.setTextColor(color);
    rngHBox.addWidget(&spiceLabel);
    spiceDigitsTextBox.setMinMax(0, 99);
    spiceDigitsTextBox.setValue(SPICEFIELDS);
    spiceDigitsTextBox.setColor(house, color);
    spiceDigitsTextBox.setOnValueChange(std::bind(&NewMapWindow::onMapPropertiesChanged,this));
    rngHBox.addWidget(&spiceDigitsTextBox,45);

    rngHBox.addWidget(Spacer::create(),3.0);

    centralVBox.addWidget(&basicMapPropertiesHBox);

    centralVBox.addWidget(VSpacer::create(10));

    mirrorModeLabel.setText(_("Mirror mode") + ":");
    mirrorModeLabel.setTextColor(color);
    mirrorModeHBox.addWidget(&mirrorModeLabel);
    mirrorModeDropDownBox.addEntry(_("Off"), MirrorModeNone);
    mirrorModeDropDownBox.addEntry(_("Horizontal"), MirrorModeHorizontal);
    mirrorModeDropDownBox.addEntry(_("Vertical"), MirrorModeVertical);
    mirrorModeDropDownBox.addEntry(_("Horizontal and vertical"), MirrorModeBoth);
    mirrorModeDropDownBox.addEntry(_("Inverse"), MirrorModePoint);
    mirrorModeDropDownBox.setSelectedItem(0);
    mirrorModeDropDownBox.setOnSelectionChange(std::bind(&NewMapWindow::onMapPropertiesChanged,this));
    mirrorModeDropDownBox.setColor(color);
    mirrorModeHBox.addWidget(&mirrorModeDropDownBox, 160);
    mirrorModeHBox.addWidget(Spacer::create(), 5.0);
    centralVBox.addWidget(&mirrorModeHBox);

    centralVBox.addWidget(Spacer::create(), 5.0);

    centralVBox.addWidget(VSpacer::create(10));

    authorLabel.setText(_("Author:"));
    authorLabel.setTextColor(color);
    authorHBox.addWidget(&authorLabel, 120);
    authorTextBox.setText("");
    authorTextBox.setTextColor(color);
    authorHBox.addWidget(&authorTextBox);
    authorHBox.addWidget(HSpacer::create(140));
    centralVBox.addWidget(&authorHBox);

    centralVBox.addWidget(VSpacer::create(10));

    licenseLabel.setText(_("License:"));
    licenseLabel.setTextColor(color);
    licenseHBox.addWidget(&licenseLabel, 120);
    licenseTextBox.setText("CC-BY-SA");
    licenseTextBox.setTextColor(color);
    licenseHBox.addWidget(&licenseTextBox);
    licenseHBox.addWidget(HSpacer::create(140));
    centralVBox.addWidget(&licenseHBox);

    centralVBox.addWidget(Spacer::create());

    mainVBox.addWidget(VSpacer::create(5));

    mainVBox.addWidget(&buttonHBox);

    cancelButton.setText(_("Cancel"));
    cancelButton.setTextColor(color);
    cancelButton.setOnClick(std::bind(&NewMapWindow::onCancel, this));

    buttonHBox.addWidget(&cancelButton);

    buttonHBox.addWidget(HSpacer::create(8));

    loadButton.setText(_("Load Map") + "...");
    loadButton.setTextColor(color);
    loadButton.setOnClick(std::bind(&NewMapWindow::onLoad, this));

    buttonHBox.addWidget(&loadButton);

    buttonHBox.addWidget(HSpacer::create(8));

    createButton.setText(_("Create"));
    createButton.setTextColor(color);
    createButton.setOnClick(std::bind(&NewMapWindow::onCreate, this));

    buttonHBox.addWidget(&createButton);

    mainVBox.addWidget(VSpacer::create(10));

    randomMapRadioButton.setChecked(true);
    onMapTypeChanged(1);
}

void NewMapWindow::onCancel() {
    mapdata = MapData(0,0);

    Window* pParentWindow = dynamic_cast<Window*>(getParent());
    if(pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void NewMapWindow::onLoad() {
    openWindow(LoadMapWindow::create(color));
}

void NewMapWindow::onCreate() {
    Window* pParentWindow = dynamic_cast<Window*>(getParent());
    if(pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void NewMapWindow::onChildWindowClose(Window* pChildWindow) {
    LoadMapWindow* pLoadMapWindow = dynamic_cast<LoadMapWindow*>(pChildWindow);
    if(pLoadMapWindow != nullptr) {
        loadMapFilepath = pLoadMapWindow->getLoadMapFilepath();
        loadMapname = pLoadMapWindow->getLoadMapname();
        loadMapSingleplayer = pLoadMapWindow->isLoadMapSingleplayer();

        if(loadMapFilepath != "") {
            Window* pParentWindow = dynamic_cast<Window*>(getParent());
            if(pParentWindow != nullptr) {
                pParentWindow->closeChildWindow();
            }
        }
    }
}

void NewMapWindow::onMapTypeChanged(int buttonID) {

    mapSizeHBox.removeAllChildWidgets();

    if(buttonID == 2) {
        mapSizeHBox.addWidget(&mapScaleLabel);
        mapSizeHBox.addWidget(&mapScaleDropDownBox,120);
        mapSizeHBox.addWidget(Spacer::create(),5.0);
    } else {
        mapSizeHBox.addWidget(&mapSizeXLabel);
        mapSizeHBox.addWidget(&mapSizeXDropDownBox, 50);
        mapSizeHBox.addWidget(HSpacer::create(30));
        mapSizeHBox.addWidget(&mapSizeYLabel);
        mapSizeHBox.addWidget(&mapSizeYDropDownBox, 50);
        mapSizeHBox.addWidget(Spacer::create(),5.0);
    }

    rngSeedLabel.setVisible( (buttonID != 0) );
    rngSeedTextBox.setVisible( (buttonID != 0) );

    rockLabel.setVisible( (buttonID == 1) );
    rockDigitsTextBox.setVisible( (buttonID == 1) );
    spiceLabel.setVisible( (buttonID == 1) );
    spiceDigitsTextBox.setVisible( (buttonID == 1) );

    mirrorModeLabel.setVisible( (buttonID == 1) );
    mirrorModeDropDownBox.setVisible( (buttonID == 1) );

    onMapPropertiesChanged();
}

void NewMapWindow::onMapPropertiesChanged() {

    if(emptyMapRadioButton.isChecked()) {
        int sizeX = mapSizeXDropDownBox.getSelectedEntryIntData();
        int sizeY = mapSizeYDropDownBox.getSelectedEntryIntData();

        mapSeed = INVALID;
        mapdata = MapData(sizeX, sizeY, Terrain_Sand);
    } else if(randomMapRadioButton.isChecked()) {
        int sizeX = mapSizeXDropDownBox.getSelectedEntryIntData();
        int sizeY = mapSizeYDropDownBox.getSelectedEntryIntData();

        int seed = rngSeedTextBox.getValue();
        int rock = rockDigitsTextBox.getValue();
        int spice = spiceDigitsTextBox.getValue();

        MirrorMode mirrorMode = (MirrorMode) mirrorModeDropDownBox.getSelectedEntryIntData();

        mapSeed = INVALID;
        mapdata = generateRandomMap(sizeX, sizeY, seed, rock, spice, mirrorMode);

    } else if(seedMapRadioButton.isChecked()) {
        int seed = rngSeedTextBox.getValue();
        int scale = mapScaleDropDownBox.getSelectedEntryIntData();

        mapSeed = seed;
        mapdata = createMapWithSeed(seed, scale);
    }

    minimap.setSurface(createMinimapPicture(mapdata, 1, DuneStyle::buttonBorderColor));
}

sdl2::surface_ptr NewMapWindow::createMinimapPicture(MapData& mapdata, int borderWidth, Uint32 borderColor) {
    sdl2::surface_ptr pMinimap = sdl2::surface_ptr{ SDL_CreateRGBSurface(0, 128+2*borderWidth, 128+2*borderWidth, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK) };
    if(!pMinimap) {
        return nullptr;
    }
    SDL_FillRect(pMinimap.get(), nullptr, borderColor);
    SDL_Rect dest = { borderWidth, borderWidth, pMinimap->w - 2*borderWidth, pMinimap->h - 2*borderWidth};
    SDL_FillRect(pMinimap.get(), &dest, COLOR_BLACK);

    int scale = 1;
    int offsetX;
    int offsetY;

    RadarViewBase::calculateScaleAndOffsets(mapdata.getSizeX(), mapdata.getSizeY(), scale, offsetX, offsetY);

    offsetX += borderWidth;
    offsetY += borderWidth;

    for(int y = 0; y < mapdata.getSizeY(); y++) {
        for(int x = 0; x < mapdata.getSizeX(); x++) {

            TERRAINTYPE terrainType = mapdata(x,y);

            Uint32 color = getColorByTerrainType(terrainType);

            for(int i=0;i<scale;i++) {
                for(int j=0;j<scale;j++) {
                    putPixel(pMinimap.get(), x*scale + i + offsetX, y*scale + j + offsetY, color);
                }
            }
        }
    }

    return pMinimap;
}

