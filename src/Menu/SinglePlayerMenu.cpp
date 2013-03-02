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

#include <Menu/SinglePlayerMenu.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <misc/fnkdat.h>
#include <misc/string_util.h>

#include <Menu/CustomGameMenu.h>
#include <Menu/SinglePlayerSkirmishMenu.h>
#include <Menu/HouseChoiceMenu.h>

#include <GUI/dune/LoadSaveWindow.h>
#include <GUI/MsgBox.h>

#include <Game.h>
#include <GameInitSettings.h>
#include <sand.h>

#include <stdexcept>

SinglePlayerMenu::SinglePlayerMenu() : MenuBase() {
	// set up window
	SDL_Surface *surf;
	surf = pGFXManager->getUIGraphic(UI_MenuBackground);

	setBackground(surf,false);
	resize(surf->w,surf->h);

	setWindowWidget(&windowWidget);

	// set up pictures in the background
	surf = pGFXManager->getUIGraphic(UI_PlanetBackground);
	planetPicture.setSurface(surf,false);
	windowWidget.addWidget(&planetPicture,
							Point((screen->w - surf->w)/2,screen->h/2 - surf->h + 10),
							Point(surf->w,surf->h));

	surf = pGFXManager->getUIGraphic(UI_DuneLegacy);
	duneLegacy.setSurface(surf,false);
	windowWidget.addWidget(&duneLegacy,
							Point((screen->w - surf->w)/2, screen->h/2 + 28),
							Point(surf->w,surf->h));

	surf = pGFXManager->getUIGraphic(UI_MenuButtonBorder);
	buttonBorder.setSurface(surf,false);
	windowWidget.addWidget(&buttonBorder,
							Point((screen->w - surf->w)/2, screen->h/2 + 59),
							Point(surf->w,surf->h));

	// set up menu buttons
	windowWidget.addWidget(&menuButtonsVBox,Point((screen->w - 160)/2,screen->h/2 + 64),
										Point(160,111));

	campaignButton.setText(_("CAMPAIGN"));
	campaignButton.setOnClick(std::bind(&SinglePlayerMenu::onCampaign, this));
	menuButtonsVBox.addWidget(&campaignButton);
	campaignButton.setActive();

	menuButtonsVBox.addWidget(VSpacer::create(3));

	customButton.setText(_("CUSTOM GAME"));
	customButton.setOnClick(std::bind(&SinglePlayerMenu::onCustom, this));
	menuButtonsVBox.addWidget(&customButton);

	menuButtonsVBox.addWidget(VSpacer::create(3));

	skirmishButton.setText(_("SKIRMISH"));
	skirmishButton.setOnClick(std::bind(&SinglePlayerMenu::onSkirmish, this));
	menuButtonsVBox.addWidget(&skirmishButton);

	menuButtonsVBox.addWidget(VSpacer::create(3));

	loadSavegameButton.setText(_("LOAD GAME"));
	loadSavegameButton.setOnClick(std::bind(&SinglePlayerMenu::onLoadSavegame, this));
	menuButtonsVBox.addWidget(&loadSavegameButton);

	menuButtonsVBox.addWidget(VSpacer::create(3));

	loadReplayButton.setText(_("LOAD REPLAY"));
	loadReplayButton.setOnClick(std::bind(&SinglePlayerMenu::onLoadReplay, this));
	menuButtonsVBox.addWidget(&loadReplayButton);

	menuButtonsVBox.addWidget(VSpacer::create(3));

	cancelButton.setText(_("BACK"));
	cancelButton.setOnClick(std::bind(&SinglePlayerMenu::onCancel, this));
	menuButtonsVBox.addWidget(&cancelButton);
}

SinglePlayerMenu::~SinglePlayerMenu() {
}

void SinglePlayerMenu::onCampaign() {
	HouseChoiceMenu* pHouseChoiceMenu = new HouseChoiceMenu();
	int player = pHouseChoiceMenu->showMenu();
	delete pHouseChoiceMenu;

	if(player < 0) {
		return;
	}

	GameInitSettings init((HOUSETYPE) player, settings.gameOptions);

    for(int houseID = 0; houseID < NUM_HOUSES; houseID++) {
	    if(houseID == player) {
	        GameInitSettings::HouseInfo humanHouseInfo((HOUSETYPE) player, 1);
	        humanHouseInfo.addPlayerInfo( GameInitSettings::PlayerInfo(settings.general.playerName, HUMANPLAYERCLASS) );
            init.addHouseInfo(humanHouseInfo);
	    } else {
            GameInitSettings::HouseInfo aiHouseInfo((HOUSETYPE) houseID, 2);
	        aiHouseInfo.addPlayerInfo( GameInitSettings::PlayerInfo(getHouseNameByNumber( (HOUSETYPE) houseID), settings.ai.campaignAI) );
            init.addHouseInfo(aiHouseInfo);
	    }
	}

	startSinglePlayerGame(init);

	quit();
}

void SinglePlayerMenu::onCustom() {
    CustomGameMenu* pCustomGameMenu = new CustomGameMenu(false);
    pCustomGameMenu->showMenu();
    delete pCustomGameMenu;
}

void SinglePlayerMenu::onSkirmish() {
	SinglePlayerSkirmishMenu* pSinglePlayerSkirmishMenu = new SinglePlayerSkirmishMenu();
	pSinglePlayerSkirmishMenu->showMenu();
	delete pSinglePlayerSkirmishMenu;
}

void SinglePlayerMenu::onLoadSavegame() {
	char tmp[FILENAME_MAX];
	fnkdat("save/", tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
	std::string savepath(tmp);
	openWindow(LoadSaveWindow::create(false, _("Load Game"), savepath, "dls"));
}

void SinglePlayerMenu::onLoadReplay() {
	char tmp[FILENAME_MAX];
	fnkdat("replay/", tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
	std::string replaypath(tmp);
	openWindow(LoadSaveWindow::create(false, _("Load Replay"), replaypath, "rpl"));
}

void SinglePlayerMenu::onCancel() {
	quit();
}

void SinglePlayerMenu::onChildWindowClose(Window* pChildWindow) {
	std::string filename = "";
	std::string extension = "";
	LoadSaveWindow* pLoadSaveWindow = dynamic_cast<LoadSaveWindow*>(pChildWindow);
	if(pLoadSaveWindow != NULL) {
		filename = pLoadSaveWindow->getFilename();
		extension = pLoadSaveWindow->getExtension();
	}

	if(filename != "") {
		if(extension == "dls") {

		    try {
                startSinglePlayerGame( GameInitSettings(filename) );
		    } catch (std::exception& e) {
		        // most probably the savegame file is not valid or from a different dune legacy version
                openWindow(MsgBox::create(e.what()));
            }
		} else if(extension == "rpl") {
			startReplay(filename);
		}
	}
}
