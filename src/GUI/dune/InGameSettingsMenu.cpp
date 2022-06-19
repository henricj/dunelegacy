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

#include "misc/Fullscreen.h"
#include <FileClasses/DuneConfig.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/music/MusicPlayer.h>
#include <House.h>
#include <SoundPlayer.h>

#include <GUI/Spacer.h>

InGameSettingsMenu::InGameSettingsMenu() : Window(0, 0, 0, 0) {
    const auto* const gfx = dune::globals::pGFXManager.get();
    const auto& palette   = dune::globals::palette;

    const auto houseID = dune::globals::pLocalHouse->getHouseID();

    // set up window
    const auto* pBackground = gfx->getUIGraphic(UI_OptionsMenu, houseID);
    setBackground(pBackground);

    InGameSettingsMenu::setCurrentPosition(calcAlignedDrawingRect(pBackground, HAlign::Center, VAlign::Center));

    InGameSettingsMenu::setWindowWidget(&windowWidget);

    const auto palette_index = dune::globals::houseToPaletteIndex[static_cast<int>(houseID)];

    const Uint32 color1 = SDL2RGB(palette[palette_index + 2]);
    const Uint32 color2 = SDL2RGB(palette[palette_index + 3]);

    // Game speed
    gameSpeedMinus.setTextures(gfx->getUIGraphic(UI_Minus, houseID), gfx->getUIGraphic(UI_Minus_Pressed, houseID));
    gameSpeedMinus.setOnClick([this] { onGameSpeedMinus(); });
    windowWidget.addWidget(&gameSpeedMinus, Point(5, 52), gameSpeedMinus.getSize());

    gameSpeedBar.setColor(color1);
    windowWidget.addWidget(&gameSpeedBar, Point(23, 56), Point(146, 6));

    gameSpeedPlus.setTextures(gfx->getUIGraphic(UI_Plus, houseID), gfx->getUIGraphic(UI_Plus_Pressed, houseID));
    gameSpeedPlus.setOnClick([this] { onGameSpeedPlus(); });
    windowWidget.addWidget(&gameSpeedPlus, Point(172, 52), gameSpeedPlus.getSize());

    // Volume
    volumeMinus.setTextures(gfx->getUIGraphic(UI_Minus, houseID), gfx->getUIGraphic(UI_Minus_Pressed, houseID));
    volumeMinus.setOnClick([this] { onVolumeMinus(); });
    windowWidget.addWidget(&volumeMinus, Point(5, 83), volumeMinus.getSize());

    volumeBar.setColor(color1);
    windowWidget.addWidget(&volumeBar, Point(23, 87), Point(146, 6));

    volumePlus.setTextures(gfx->getUIGraphic(UI_Plus, houseID), gfx->getUIGraphic(UI_Plus_Pressed, houseID));
    volumePlus.setOnClick([this] { onVolumePlus(); });
    windowWidget.addWidget(&volumePlus, Point(172, 83), volumePlus.getSize());

    // Scroll speed
    scrollSpeedMinus.setTextures(gfx->getUIGraphic(UI_Minus, houseID), gfx->getUIGraphic(UI_Minus_Pressed, houseID));
    scrollSpeedMinus.setOnClick([this] { onScrollSpeedMinus(); });
    windowWidget.addWidget(&scrollSpeedMinus, Point(5, 114), scrollSpeedMinus.getSize());

    scrollSpeedBar.setColor(color1);
    windowWidget.addWidget(&scrollSpeedBar, Point(23, 118), Point(146, 6));

    scrollSpeedPlus.setTextures(gfx->getUIGraphic(UI_Plus, houseID), gfx->getUIGraphic(UI_Plus_Pressed, houseID));
    scrollSpeedPlus.setOnClick([this] { onScrollSpeedPlus(); });
    windowWidget.addWidget(&scrollSpeedPlus, Point(172, 114), scrollSpeedPlus.getSize());

    // buttons
    okButton.setText(_("OK"));
    okButton.setTextColor(color2);
    okButton.setOnClick([this] { onOK(); });
    windowWidget.addWidget(&okButton, Point(12, 134), Point(79, 15));

    cancelButton.setText(_("Cancel"));
    cancelButton.setTextColor(color2);
    cancelButton.setOnClick([this] { onCancel(); });
    windowWidget.addWidget(&cancelButton, Point(101, 134), Point(79, 15));

    init();
}

InGameSettingsMenu::~InGameSettingsMenu() = default;

void InGameSettingsMenu::init() {
    const auto& settings = dune::globals::settings;

    newGamespeed = settings.gameOptions.gameSpeed;
    update_speed_bar();

    previousVolume = volume = dune::globals::soundPlayer->getSfxVolume();
    update_volume_bar();

    scrollSpeed = settings.general.scrollSpeed;
    scrollSpeedBar.setProgress(static_cast<float>(scrollSpeed));
}

bool InGameSettingsMenu::handleKeyPress(const SDL_KeyboardEvent& key) {
    switch (key.keysym.sym) {
        case SDLK_RETURN:
            if (SDL_GetModState() & KMOD_ALT) {
                toggleFullscreen();
            }
            break;

        case SDLK_TAB:
            if (SDL_GetModState() & KMOD_ALT) {
                SDL_MinimizeWindow(dune::globals::window.get());
            }
            break;

        default: break;
    }

    return parent::handleKeyPress(key);
}

void InGameSettingsMenu::onCancel() {
    dune::globals::soundPlayer->setSfxVolume(previousVolume);
    dune::globals::musicPlayer->setMusicVolume(previousVolume);

    auto* pParentWindow = dynamic_cast<Window*>(getParent());
    if (pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void InGameSettingsMenu::onOK() {
    auto& settings = dune::globals::settings;

    settings.general.scrollSpeed   = scrollSpeed;
    settings.audio.sfxVolume       = dune::globals::soundPlayer->getSfxVolume();
    settings.audio.musicVolume     = dune::globals::musicPlayer->getMusicVolume();
    settings.gameOptions.gameSpeed = newGamespeed;

    INIFile myINIFile(getConfigFilepath());
    myINIFile.setIntValue("General", "Scroll Speed", settings.general.scrollSpeed);
    myINIFile.setIntValue("Audio", "Music Volume", settings.audio.musicVolume);
    myINIFile.setIntValue("Audio", "SFX Volume", settings.audio.sfxVolume);
    myINIFile.setIntValue("Game Options", "Game Speed", settings.gameOptions.gameSpeed);
    if (!myINIFile.saveChangesTo(getConfigFilepath())) {
        sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Unable to save configuration file %s",
                        reinterpret_cast<const char*>(getConfigFilepath().u8string().c_str()));
    }

    auto* pParentWindow = dynamic_cast<Window*>(getParent());
    if (pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void InGameSettingsMenu::onGameSpeedPlus() {
    if (newGamespeed > GAMESPEED_MIN)
        newGamespeed -= 1;

    update_speed_bar();
}

void InGameSettingsMenu::onGameSpeedMinus() {
    if (newGamespeed < GAMESPEED_MAX)
        newGamespeed += 1;

    update_speed_bar();
}

void InGameSettingsMenu::onVolumePlus() {
    if (volume <= MIX_MAX_VOLUME - 4) {
        volume += 4;
        update_volume_bar();
        dune::globals::soundPlayer->setSfxVolume(volume);
        dune::globals::musicPlayer->setMusicVolume(volume);
    }
}

void InGameSettingsMenu::onVolumeMinus() {
    if (volume >= 4) {
        volume -= 4;
        update_volume_bar();
        dune::globals::soundPlayer->setSfxVolume(volume);
        dune::globals::musicPlayer->setMusicVolume(volume);
    }
}

void InGameSettingsMenu::onScrollSpeedPlus() {
    scrollSpeed = std::min(scrollSpeed + 4, 100);
    scrollSpeedBar.setProgress(static_cast<float>(scrollSpeed));
}

void InGameSettingsMenu::onScrollSpeedMinus() {
    scrollSpeed = std::max(scrollSpeed - 4, 1);
    scrollSpeedBar.setProgress(static_cast<float>(scrollSpeed));
}

void InGameSettingsMenu::update_speed_bar() {
    static constexpr auto scale = 100.f / static_cast<float>(GAMESPEED_MAX - GAMESPEED_MIN);

    const auto percent = 100.f - scale * static_cast<float>(newGamespeed - GAMESPEED_MIN);
    gameSpeedBar.setProgress(percent);
}

void InGameSettingsMenu::update_volume_bar() {
    static constexpr auto scale = 100.0f / static_cast<float>(MIX_MAX_VOLUME);

    volumeBar.setProgress(scale * static_cast<float>(volume));
}
