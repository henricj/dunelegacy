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

#include <Menu/CustomGameMenu.h>
#include <Menu/CustomGamePlayers.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/INIFile.h>

#include <GUI/Spacer.h>
#include <GUI/GUIStyle.h>
#include <GUI/dune/GameOptionsWindow.h>
#include <GUI/dune/LoadSaveWindow.h>
#include <GUI/dune/DuneStyle.h>

#include <config.h>
#include <misc/fnkdat.h>
#include <misc/FileSystem.h>
#include <misc/draw_util.h>
#include <misc/string_util.h>

#include <INIMap/INIMapPreviewCreator.h>
#include <GameInitSettings.h>

#include <globals.h>

#include <memory>


CustomGameMenu::CustomGameMenu(bool multiplayer, bool LANServer)
 : MenuBase(), bMultiplayer(multiplayer), bLANServer(LANServer), currentGameOptions(settings.gameOptions) {
    // set up window
    SDL_Texture *pBackground = pGFXManager->getUIGraphic(UI_MenuBackground);
    setBackground(pBackground, false);
    resize(getTextureSize(pBackground));

    setWindowWidget(&windowWidget);

    windowWidget.addWidget(&mainVBox, Point(24,23), Point(getRendererWidth() - 48, getRendererHeight() - 32));

    captionLabel.setText(bMultiplayer ? (bLANServer ? _("LAN Game") : _("Internet Game")) : _("Custom Game"));
    captionLabel.setAlignment(Alignment_HCenter);
    mainVBox.addWidget(&captionLabel, 24);
    mainVBox.addWidget(VSpacer::create(24));

    mainVBox.addWidget(Spacer::create(), 0.05);

    mainVBox.addWidget(&mainHBox, 0.80);

    mainHBox.addWidget(Spacer::create(), 0.05);
    mainHBox.addWidget(&leftVBox, 0.8);

    leftVBox.addWidget(&mapTypeButtonsHBox, 24);

    singleplayerMapsButton.setText(_("SP Maps"));
    singleplayerMapsButton.setToggleButton(true);
    singleplayerMapsButton.setOnClick(std::bind(&CustomGameMenu::onMapTypeChange, this, 0));
    mapTypeButtonsHBox.addWidget(&singleplayerMapsButton);

    singleplayerUserMapsButton.setText(_("SP User Maps"));
    singleplayerUserMapsButton.setToggleButton(true);
    singleplayerUserMapsButton.setOnClick(std::bind(&CustomGameMenu::onMapTypeChange, this, 1));
    mapTypeButtonsHBox.addWidget(&singleplayerUserMapsButton);

    multiplayerMapsButton.setText(_("MP Maps"));
    multiplayerMapsButton.setToggleButton(true);
    multiplayerMapsButton.setOnClick(std::bind(&CustomGameMenu::onMapTypeChange, this, 2));
    mapTypeButtonsHBox.addWidget(&multiplayerMapsButton);

    multiplayerUserMapsButton.setText(_("MP User Maps"));
    multiplayerUserMapsButton.setToggleButton(true);
    multiplayerUserMapsButton.setOnClick(std::bind(&CustomGameMenu::onMapTypeChange, this, 3));
    mapTypeButtonsHBox.addWidget(&multiplayerUserMapsButton);

    dummyButton.setEnabled(false);
    mapTypeButtonsHBox.addWidget(&dummyButton, 17);
    mapList.setAutohideScrollbar(false);
    mapList.setOnSelectionChange(std::bind(&CustomGameMenu::onMapListSelectionChange, this, std::placeholders::_1));
    mapList.setOnDoubleClick(std::bind(&CustomGameMenu::onNext, this));
    leftVBox.addWidget(&mapList, 0.95);

    leftVBox.addWidget(VSpacer::create(10));

    multiplePlayersPerHouseCheckbox.setText(_("Multiple players per house"));
    optionsHBox.addWidget(&multiplePlayersPerHouseCheckbox);
    optionsHBox.addWidget(Spacer::create());
    gameOptionsButton.setText(_("Game Options..."));
    gameOptionsButton.setOnClick(std::bind(&CustomGameMenu::onGameOptions, this));
    optionsHBox.addWidget(&gameOptionsButton, 140);

    leftVBox.addWidget(Spacer::create(), 0.05);

    leftVBox.addWidget(&optionsHBox, 0.05);

    mainHBox.addWidget(HSpacer::create(8));
    mainHBox.addWidget(Spacer::create(), 0.05);

    mainHBox.addWidget(&rightVBox, 180);
    mainHBox.addWidget(Spacer::create(), 0.05);
    minimap.setSurface( GUIStyle::getInstance().createButtonSurface(130,130,_("Choose map"), true, false), true);
    rightVBox.addWidget(&minimap);

    rightVBox.addWidget(VSpacer::create(10));
    rightVBox.addWidget(&mapPropertiesHBox, 0.01);
    mapPropertiesHBox.addWidget(&mapPropertyNamesVBox, 75);
    mapPropertiesHBox.addWidget(&mapPropertyValuesVBox, 105);
    mapPropertyNamesVBox.addWidget(Label::create(_("Size") + ":"));
    mapPropertyValuesVBox.addWidget(&mapPropertySize);
    mapPropertyNamesVBox.addWidget(Label::create(_("Players") + ":"));
    mapPropertyValuesVBox.addWidget(&mapPropertyPlayers);
    mapPropertyNamesVBox.addWidget(Label::create(_("Author") + ":"));
    mapPropertyValuesVBox.addWidget(&mapPropertyAuthors);
    mapPropertyNamesVBox.addWidget(Label::create(_("License") + ":"));
    mapPropertyValuesVBox.addWidget(&mapPropertyLicense);
    rightVBox.addWidget(Spacer::create());

    mainVBox.addWidget(Spacer::create(), 0.05);

    mainVBox.addWidget(VSpacer::create(20));
    mainVBox.addWidget(&buttonHBox, 24);
    mainVBox.addWidget(VSpacer::create(14), 0.0);

    buttonHBox.addWidget(HSpacer::create(70));
    cancelButton.setText(_("Back"));
    cancelButton.setOnClick(std::bind(&CustomGameMenu::onCancel, this));
    buttonHBox.addWidget(&cancelButton, 0.1);

    buttonHBox.addWidget(Spacer::create(), 0.0625);

    buttonHBox.addWidget(Spacer::create(), 0.25);
    loadButton.setText(_("Load"));
    loadButton.setVisible(bMultiplayer);
    loadButton.setEnabled(bMultiplayer);
    loadButton.setOnClick(std::bind(&CustomGameMenu::onLoad, this));
    buttonHBox.addWidget(&loadButton, 0.175);
    buttonHBox.addWidget(Spacer::create(), 0.25);

    buttonHBox.addWidget(Spacer::create(), 0.0625);

    nextButton.setText(_("Next"));
    nextButton.setOnClick(std::bind(&CustomGameMenu::onNext, this));
    buttonHBox.addWidget(&nextButton, 0.1);
    buttonHBox.addWidget(HSpacer::create(90));

    onMapTypeChange(0);
}

CustomGameMenu::~CustomGameMenu()
{
    ;
}


void CustomGameMenu::onChildWindowClose(Window* pChildWindow) {
    LoadSaveWindow* pLoadSaveWindow = dynamic_cast<LoadSaveWindow*>(pChildWindow);
    if(pLoadSaveWindow != nullptr) {
        std::string filename = pLoadSaveWindow->getFilename();

        if(filename != "") {
            std::string savegamedata = readCompleteFile(filename);

            std::string servername = settings.general.playerName + "'s Game";
            GameInitSettings gameInitSettings(getBasename(filename, true), savegamedata, servername);

            CustomGamePlayers* pCustomGamePlayers = new CustomGamePlayers(gameInitSettings, true, bLANServer);
            int ret = pCustomGamePlayers->showMenu();
            delete pCustomGamePlayers;
            if(ret != MENU_QUIT_DEFAULT) {
                quit(ret);
            }
        }
    }

    GameOptionsWindow* pGameOptionsWindow = dynamic_cast<GameOptionsWindow*>(pChildWindow);
    if(pGameOptionsWindow != nullptr) {
        currentGameOptions = pGameOptionsWindow->getGameOptions();
    }
}

void CustomGameMenu::onNext()
{
    if(mapList.getSelectedIndex() < 0) {
        return;
    }

    std::string mapFilename = currentMapDirectory + mapList.getSelectedEntry() + ".ini";
    getCaseInsensitiveFilename(mapFilename);

    GameInitSettings gameInitSettings;
    if(bMultiplayer) {
        std::string servername = settings.general.playerName + "'s Game";
        gameInitSettings = GameInitSettings(getBasename(mapFilename, true), readCompleteFile(mapFilename), servername, multiplePlayersPerHouseCheckbox.isChecked(), currentGameOptions);
    } else {
        gameInitSettings = GameInitSettings(mapFilename, multiplePlayersPerHouseCheckbox.isChecked(), currentGameOptions);
    }

    CustomGamePlayers* pCustomGamePlayers = new CustomGamePlayers(gameInitSettings, true, bLANServer);
    int ret = pCustomGamePlayers->showMenu();
    delete pCustomGamePlayers;
    if(ret != MENU_QUIT_DEFAULT) {
        quit(ret);
    }
}

void CustomGameMenu::onCancel()
{
    quit();
}

void CustomGameMenu::onLoad()
{
    char tmp[FILENAME_MAX];
    fnkdat("mpsave/", tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
    std::string savepath(tmp);
    openWindow(LoadSaveWindow::create(false, _("Load Game"), savepath, "dls"));
}

void CustomGameMenu::onGameOptions()
{
    openWindow(GameOptionsWindow::create(currentGameOptions));
}

void CustomGameMenu::onMapTypeChange(int buttonID)
{
    singleplayerMapsButton.setToggleState(buttonID == 0);
    singleplayerUserMapsButton.setToggleState(buttonID == 1);
    multiplayerMapsButton.setToggleState(buttonID == 2);
    multiplayerUserMapsButton.setToggleState(buttonID == 3);

    switch(buttonID) {
        case 0: {
            currentMapDirectory = std::string(DUNELEGACY_DATADIR) + std::string("/maps/singleplayer/");
        } break;
        case 1: {
            char tmp[FILENAME_MAX];
            fnkdat("maps/singleplayer/", tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
            currentMapDirectory = tmp;
        } break;
        case 2: {
            currentMapDirectory = std::string(DUNELEGACY_DATADIR) + std::string("/maps/multiplayer/");
        } break;
        case 3: {
            char tmp[FILENAME_MAX];
            fnkdat("maps/multiplayer/", tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
            currentMapDirectory = tmp;
        } break;
    }

    mapList.clearAllEntries();

    std::list<std::string> filesList = getFileNamesList(currentMapDirectory, "ini", true, FileListOrder_Name_CaseInsensitive_Asc);

    std::list<std::string>::iterator iter;
    for(iter = filesList.begin(); iter != filesList.end(); ++iter) {
        mapList.addEntry(iter->substr(0, iter->length() - 4));
    }

    if(filesList.empty() == false) {
        mapList.setSelectedItem(0);
    } else {
        minimap.setSurface( GUIStyle::getInstance().createButtonSurface(130,130,_("No map available"), true, false), true);
        mapPropertySize.setText("");
        mapPropertyPlayers.setText("");
        mapPropertyAuthors.setText("");
        mapPropertyLicense.setText("");
    }
}

void CustomGameMenu::onMapListSelectionChange(bool bInteractive)
{
    nextButton.setEnabled(true);

    if(mapList.getSelectedIndex() < 0) {
        return;
    }

    std::string mapFilename = currentMapDirectory + mapList.getSelectedEntry() + ".ini";
    getCaseInsensitiveFilename(mapFilename);

    std::shared_ptr<INIFile> pMap(new INIFile(mapFilename));

    int sizeX = 0;
    int sizeY = 0;

    if(pMap->hasKey("MAP","Seed")) {
        // old map format with seed value
        int mapscale = pMap->getIntValue("BASIC", "MapScale", -1);

        switch(mapscale) {
            case 0: {
                sizeX = 62;
                sizeY = 62;
            } break;

            case 1: {
                sizeX = 32;
                sizeY = 32;
            } break;

            case 2: {
                sizeX = 21;
                sizeY = 21;
            } break;

            default: {
                sizeX = 64;
                sizeY = 64;
            }
        }
    } else {
        // new map format with saved map
        sizeX = pMap->getIntValue("MAP","SizeX", 0);
        sizeY = pMap->getIntValue("MAP","SizeY", 0);
    }

    mapPropertySize.setText(stringify(sizeX) + " x " + stringify(sizeY));

    SDL_Surface* pMapSurface = nullptr;
    try {
        INIMapPreviewCreator mapPreviewCreator(pMap);
        pMapSurface = mapPreviewCreator.createMinimapImageOfMap(1, DuneStyle::buttonBorderColor);
    } catch(...) {
        pMapSurface = GUIStyle::getInstance().createButtonSurface(130, 130, "Error", true, false);
        loadButton.setEnabled(false);
    }
    minimap.setSurface(pMapSurface, true);

    int numPlayers = 0;
    if(pMap->hasSection("Atreides")) numPlayers++;
    if(pMap->hasSection("Ordos")) numPlayers++;
    if(pMap->hasSection("Harkonnen")) numPlayers++;
    if(pMap->hasSection("Fremen")) numPlayers++;
    if(pMap->hasSection("Mercenary")) numPlayers++;
    if(pMap->hasSection("Sardaukar")) numPlayers++;
    if(pMap->hasSection("Player1")) numPlayers++;
    if(pMap->hasSection("Player2")) numPlayers++;
    if(pMap->hasSection("Player3")) numPlayers++;
    if(pMap->hasSection("Player4")) numPlayers++;
    if(pMap->hasSection("Player5")) numPlayers++;
    if(pMap->hasSection("Player6")) numPlayers++;

    mapPropertyPlayers.setText(stringify(numPlayers));



    std::string authors = pMap->getStringValue("BASIC","Author", "-");
    if(authors.size() > 11) {
        authors = authors.substr(0,9) + "...";
    }
    mapPropertyAuthors.setText(authors);


    mapPropertyLicense.setText(pMap->getStringValue("BASIC","License", "-"));

}
