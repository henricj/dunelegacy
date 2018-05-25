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
#include <main.h>
#include <House.h>
#include <SoundPlayer.h>
#include <ScreenBorder.h>

#include <GUI/Spacer.h>


InGameSettingsMenu::InGameSettingsMenu() : Window(0,0,0,0) {
    int houseID = pLocalHouse->getHouseID();
    Uint32 color1 = SDL2RGB(palette[houseToPaletteIndex[houseID]+2]);
    Uint32 color2 = SDL2RGB(palette[houseToPaletteIndex[houseID]+3]);

    // set up window
    SDL_Texture *pBackground = pGFXManager->getUIGraphic(UI_OptionsMenu, houseID);
    setBackground(pBackground);

    setCurrentPosition(calcAlignedDrawingRect(pBackground, HAlign::Center, VAlign::Center));

    setWindowWidget(&windowWidget);

    // Game speed
    gameSpeedMinus.setTextures(pGFXManager->getUIGraphic(UI_Minus, houseID), pGFXManager->getUIGraphic(UI_Minus_Pressed, houseID));
    gameSpeedMinus.setOnClick(std::bind(&InGameSettingsMenu::onGameSpeedMinus, this));
    windowWidget.addWidget(&gameSpeedMinus, Point(5,52), gameSpeedMinus.getSize());

    gameSpeedBar.setColor(color1);
    windowWidget.addWidget(&gameSpeedBar, Point(23,56), Point(146,6));

    gameSpeedPlus.setTextures(pGFXManager->getUIGraphic(UI_Plus, houseID), pGFXManager->getUIGraphic(UI_Plus_Pressed, houseID));
    gameSpeedPlus.setOnClick(std::bind(&InGameSettingsMenu::onGameSpeedPlus, this));
    windowWidget.addWidget(&gameSpeedPlus, Point(172,52), gameSpeedPlus.getSize());

    // Volume
    volumeMinus.setTextures(pGFXManager->getUIGraphic(UI_Minus, houseID), pGFXManager->getUIGraphic(UI_Minus_Pressed, houseID));
    volumeMinus.setOnClick(std::bind(&InGameSettingsMenu::onVolumeMinus, this));
    windowWidget.addWidget(&volumeMinus, Point(5,83), volumeMinus.getSize());

    volumeBar.setColor(color1);
    windowWidget.addWidget(&volumeBar, Point(23,87), Point(146,6));

    volumePlus.setTextures(pGFXManager->getUIGraphic(UI_Plus, houseID), pGFXManager->getUIGraphic(UI_Plus_Pressed, houseID));
    volumePlus.setOnClick(std::bind(&InGameSettingsMenu::onVolumePlus, this));
    windowWidget.addWidget(&volumePlus, Point(172,83), volumePlus.getSize());


    // Scroll speed
    scrollSpeedMinus.setTextures(pGFXManager->getUIGraphic(UI_Minus, houseID), pGFXManager->getUIGraphic(UI_Minus_Pressed, houseID));
    scrollSpeedMinus.setOnClick(std::bind(&InGameSettingsMenu::onScrollSpeedMinus, this));
    windowWidget.addWidget(&scrollSpeedMinus, Point(5,114), scrollSpeedMinus.getSize());

    scrollSpeedBar.setColor(color1);
    windowWidget.addWidget(&scrollSpeedBar, Point(23,118), Point(146,6));

    scrollSpeedPlus.setTextures(pGFXManager->getUIGraphic(UI_Plus, houseID), pGFXManager->getUIGraphic(UI_Plus_Pressed, houseID));
    scrollSpeedPlus.setOnClick(std::bind(&InGameSettingsMenu::onScrollSpeedPlus, this));
    windowWidget.addWidget(&scrollSpeedPlus, Point(172,114), scrollSpeedPlus.getSize());


    // buttons
    okButton.setText(_("OK"));
    okButton.setTextColor(color2);
    okButton.setOnClick(std::bind(&InGameSettingsMenu::onOK, this));
    windowWidget.addWidget(&okButton, Point(12,134), Point(79,15));

    cancelButton.setText(_("Cancel"));
    cancelButton.setTextColor(color2);
    cancelButton.setOnClick(std::bind(&InGameSettingsMenu::onCancel, this));
    windowWidget.addWidget(&cancelButton, Point(101,134), Point(79,15));

    init();
}

InGameSettingsMenu::~InGameSettingsMenu() = default;

void InGameSettingsMenu::init() {
    newGamespeed = settings.gameOptions.gameSpeed;
    gameSpeedBar.setProgress(100.0 - ((newGamespeed-GAMESPEED_MIN)*100.0)/(GAMESPEED_MAX - GAMESPEED_MIN));

    previousVolume = volume = soundPlayer->getSfxVolume();
    volumeBar.setProgress((100.0*volume)/MIX_MAX_VOLUME);

    scrollSpeed = settings.general.scrollSpeed;
    scrollSpeedBar.setProgress(scrollSpeed);
}

bool InGameSettingsMenu::handleKeyPress(SDL_KeyboardEvent& key) {
    switch( key.keysym.sym ) {
        case SDLK_RETURN:
            if(SDL_GetModState() & KMOD_ALT) {
                toogleFullscreen();
            }
            break;

        case SDLK_TAB:
            if(SDL_GetModState() & KMOD_ALT) {
                SDL_MinimizeWindow(window);
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
    if(pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void InGameSettingsMenu::onOK() {
    settings.general.scrollSpeed = scrollSpeed;
    settings.audio.sfxVolume = soundPlayer->getSfxVolume();
    settings.audio.musicVolume = musicPlayer->getMusicVolume();
    settings.gameOptions.gameSpeed = newGamespeed;

    INIFile myINIFile(getConfigFilepath());
    myINIFile.setIntValue("General","Scroll Speed", settings.general.scrollSpeed);
    myINIFile.setIntValue("Audio","Music Volume", settings.audio.musicVolume);
    myINIFile.setIntValue("Audio","SFX Volume", settings.audio.sfxVolume);
    myINIFile.setIntValue("Game Options","Game Speed", settings.gameOptions.gameSpeed);
    myINIFile.saveChangesTo(getConfigFilepath());

    Window* pParentWindow = dynamic_cast<Window*>(getParent());
    if(pParentWindow != nullptr) {
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
    scrollSpeed = std::min(scrollSpeed+4, 100);
    scrollSpeedBar.setProgress(scrollSpeed);
}

void InGameSettingsMenu::onScrollSpeedMinus() {
    scrollSpeed = std::max(scrollSpeed-4, 1);
    scrollSpeedBar.setProgress(scrollSpeed);
}
