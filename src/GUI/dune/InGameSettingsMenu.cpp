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

#include <GUI/dune/InGameSettingsMenu.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/music/MusicPlayer.h>
#include <Game.h>
#include <House.h>
#include <SoundPlayer.h>
#include <ScreenBorder.h>

#include <GUI/Spacer.h>


InGameSettingsMenu::InGameSettingsMenu() : Window(0,0,0,0) {
    int houseID = pLocalHouse->getHouseID();
	int color = houseColor[houseID];

	// set up window
	SDL_Surface *surf,*surfPressed;
	surf = pGFXManager->getUIGraphic(UI_OptionsMenu, houseID);

	setBackground(surf,false);

	int xpos = std::max(0,(screen->w - surf->w)/2);
	int ypos = std::max(0,(screen->h - surf->h)/2);

	setCurrentPosition(xpos,ypos,surf->w,surf->h);

	setWindowWidget(&windowWidget);

	// Game speed
    surf = pGFXManager->getUIGraphic(UI_Minus, houseID);
	surfPressed = pGFXManager->getUIGraphic(UI_Minus_Pressed, houseID);
    gameSpeedMinus.setSurfaces(surf,false,surfPressed,false);
	gameSpeedMinus.setOnClick(std::bind(&InGameSettingsMenu::onGameSpeedMinus, this));
	windowWidget.addWidget(&gameSpeedMinus, Point(5,52), gameSpeedMinus.getSize());

    gameSpeedBar.setColor(color + 2);
	windowWidget.addWidget(&gameSpeedBar, Point(23,56), Point(146,6));

	surf = pGFXManager->getUIGraphic(UI_Plus, houseID);
	surfPressed = pGFXManager->getUIGraphic(UI_Plus_Pressed, houseID);
	gameSpeedPlus.setSurfaces(surf,false,surfPressed,false);
	gameSpeedPlus.setOnClick(std::bind(&InGameSettingsMenu::onGameSpeedPlus, this));
	windowWidget.addWidget(&gameSpeedPlus, Point(172,52), gameSpeedPlus.getSize());

	// Volume
	surf = pGFXManager->getUIGraphic(UI_Minus, houseID);
	surfPressed = pGFXManager->getUIGraphic(UI_Minus_Pressed, houseID);
	volumeMinus.setSurfaces(surf,false,surfPressed,false);
	volumeMinus.setOnClick(std::bind(&InGameSettingsMenu::onVolumeMinus, this));
	windowWidget.addWidget(&volumeMinus, Point(5,83), volumeMinus.getSize());

	volumeBar.setColor(color + 2);
	windowWidget.addWidget(&volumeBar, Point(23,87), Point(146,6));

	surf = pGFXManager->getUIGraphic(UI_Plus, houseID);
	surfPressed = pGFXManager->getUIGraphic(UI_Plus_Pressed, houseID);
	volumePlus.setSurfaces(surf,false,surfPressed,false);
	volumePlus.setOnClick(std::bind(&InGameSettingsMenu::onVolumePlus, this));
	windowWidget.addWidget(&volumePlus, Point(172,83), volumePlus.getSize());


	// Scroll speed
	surf = pGFXManager->getUIGraphic(UI_Minus, houseID);
	surfPressed = pGFXManager->getUIGraphic(UI_Minus_Pressed, houseID);
	scrollSpeedMinus.setSurfaces(surf,false,surfPressed,false);
	scrollSpeedMinus.setOnClick(std::bind(&InGameSettingsMenu::onScrollSpeedMinus, this));
	windowWidget.addWidget(&scrollSpeedMinus, Point(5,114), scrollSpeedMinus.getSize());

	scrollSpeedBar.setColor(color + 2);
	windowWidget.addWidget(&scrollSpeedBar, Point(23,118), Point(146,6));

	surf = pGFXManager->getUIGraphic(UI_Plus, houseID);
	surfPressed = pGFXManager->getUIGraphic(UI_Plus_Pressed, houseID);
	scrollSpeedPlus.setSurfaces(surf,false,surfPressed,false);
	scrollSpeedPlus.setOnClick(std::bind(&InGameSettingsMenu::onScrollSpeedPlus, this));
	windowWidget.addWidget(&scrollSpeedPlus, Point(172,114), scrollSpeedPlus.getSize());


    // buttons
	okButton.setText(_("OK"));
	okButton.setTextColor(color+3);
	okButton.setOnClick(std::bind(&InGameSettingsMenu::onOK, this));
	windowWidget.addWidget(&okButton, Point(12,134), Point(79,15));

	cancelButton.setText(_("Cancel"));
	cancelButton.setTextColor(color+3);
	cancelButton.setOnClick(std::bind(&InGameSettingsMenu::onCancel, this));
	windowWidget.addWidget(&cancelButton, Point(101,134), Point(79,15));

	init();
}

InGameSettingsMenu::~InGameSettingsMenu() {
}

void InGameSettingsMenu::init() {
	newGamespeed = currentGame->gamespeed;
	gameSpeedBar.setProgress(100.0 - ((newGamespeed-GAMESPEED_MIN)*100.0)/(GAMESPEED_MAX - GAMESPEED_MIN));

	previousVolume = volume = soundPlayer->getSfxVolume();
	volumeBar.setProgress((100.0*volume)/MIX_MAX_VOLUME);

    scrollSpeed = screenborder->getScrollSpeed();
	scrollSpeedBar.setProgress((scrollSpeed-2.0)*2.0);
}

bool InGameSettingsMenu::handleKeyPress(SDL_KeyboardEvent& key) {
	switch( key.keysym.sym ) {
		case SDLK_RETURN:
            if(SDL_GetModState() & KMOD_ALT) {
                SDL_WM_ToggleFullScreen(screen);
            }
            break;

        case SDLK_TAB:
            if(SDL_GetModState() & KMOD_ALT) {
                SDL_WM_IconifyWindow();
            }
            break;

		default:
			break;
	}

	return Window::handleKeyPress(key);
}

void InGameSettingsMenu::onCancel() {
	soundPlayer->setSfxVolume(previousVolume);
	musicPlayer->setMusicVolume(previousVolume);

	Window* pParentWindow = dynamic_cast<Window*>(getParent());
	if(pParentWindow != NULL) {
		pParentWindow->closeChildWindow();
	}
}

void InGameSettingsMenu::onOK() {
	currentGame->gamespeed = newGamespeed;
    screenborder->setScrollSpeed(scrollSpeed);

	Window* pParentWindow = dynamic_cast<Window*>(getParent());
	if(pParentWindow != NULL) {
		pParentWindow->closeChildWindow();
	}
}

void InGameSettingsMenu::onGameSpeedPlus() {
	if(newGamespeed > GAMESPEED_MIN)
		newGamespeed -= 1;

	gameSpeedBar.setProgress(100 - ((newGamespeed-GAMESPEED_MIN)*100)/(GAMESPEED_MAX - GAMESPEED_MIN));
}

void InGameSettingsMenu::onGameSpeedMinus() {
	if(newGamespeed < GAMESPEED_MAX)
		newGamespeed += 1;

	gameSpeedBar.setProgress(100 - ((newGamespeed-GAMESPEED_MIN)*100)/(GAMESPEED_MAX - GAMESPEED_MIN));
}

void InGameSettingsMenu::onVolumePlus() {
	if(volume <= MIX_MAX_VOLUME - 4) {
		volume += 4;
		volumeBar.setProgress((100*volume)/MIX_MAX_VOLUME);
        soundPlayer->setSfxVolume(volume);
        musicPlayer->setMusicVolume(volume);
	}
}

void InGameSettingsMenu::onVolumeMinus() {
	if(volume >= 4) {
		volume -= 4;
		volumeBar.setProgress((100*volume)/MIX_MAX_VOLUME);
        soundPlayer->setSfxVolume(volume);
        musicPlayer->setMusicVolume(volume);
	}
}

void InGameSettingsMenu::onScrollSpeedPlus() {
    if(scrollSpeed < 51) {
        scrollSpeed += 2;
        scrollSpeedBar.setProgress((scrollSpeed-2)*2.0);
    }
}

void InGameSettingsMenu::onScrollSpeedMinus() {
    if(scrollSpeed > 3) {
        scrollSpeed -= 2;
        scrollSpeedBar.setProgress((scrollSpeed-2)*2.0);
    }
}
