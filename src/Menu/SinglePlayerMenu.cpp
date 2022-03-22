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

#include <misc/exceptions.h>
#include <misc/fnkdat.h>
#include <misc/string_util.h>

#include <Menu/CustomGameMenu.h>
#include <Menu/HouseChoiceMenu.h>
#include <Menu/SinglePlayerSkirmishMenu.h>

#include <GUI/MsgBox.h>
#include <GUI/dune/LoadSaveWindow.h>

#include <Game.h>
#include <GameInitSettings.h>
#include <sand.h>

SinglePlayerMenu::SinglePlayerMenu() {
    // set up window
    const auto* const pBackground = pGFXManager->getUIGraphic(UI_MenuBackground);
    setBackground(pBackground);
    resize(getTextureSize(pBackground));

    setWindowWidget(&windowWidget);

    // set up pictures in the background
    const auto* const pPlanetBackground = pGFXManager->getUIGraphic(UI_PlanetBackground);
    planetPicture.setTexture(pPlanetBackground);
    auto dest1 = calcAlignedDrawingRect(pPlanetBackground);
    dest1.y    = dest1.y - getHeight(pPlanetBackground) / 2 + 10;
    windowWidget.addWidget(&planetPicture, dest1);

    const auto* const pDuneLegacy = pGFXManager->getUIGraphic(UI_DuneLegacy);
    duneLegacy.setTexture(pDuneLegacy);
    auto dest2 = calcAlignedDrawingRect(pDuneLegacy);
    dest2.y    = dest2.y + getHeight(pDuneLegacy) / 2 + 28;
    windowWidget.addWidget(&duneLegacy, dest2);

    const auto* pMenuButtonBorder = pGFXManager->getUIGraphic(UI_MenuButtonBorder);
    buttonBorder.setTexture(pMenuButtonBorder);
    auto dest3 = calcAlignedDrawingRect(pMenuButtonBorder);
    dest3.y    = dest3.y + getHeight(pMenuButtonBorder) / 2 + 59;
    windowWidget.addWidget(&buttonBorder, dest3);

    // set up menu buttons
    windowWidget.addWidget(&menuButtonsVBox, Point((getRendererWidth() - 160) / 2, getRendererHeight() / 2 + 64), Point(160, 111));

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
    skirmishButton.setOnClick([] { onSkirmish(); });
    menuButtonsVBox.addWidget(&skirmishButton);

    menuButtonsVBox.addWidget(VSpacer::create(3));

    loadSavegameButton.setText(_("LOAD GAME"));
    loadSavegameButton.setOnClick([this] { onLoadSavegame(); });
    menuButtonsVBox.addWidget(&loadSavegameButton);

    menuButtonsVBox.addWidget(VSpacer::create(3));

    loadReplayButton.setText(_("LOAD REPLAY"));
    loadReplayButton.setOnClick([this] { onLoadReplay(); });
    menuButtonsVBox.addWidget(&loadReplayButton);

    menuButtonsVBox.addWidget(VSpacer::create(3));

    cancelButton.setText(_("BACK"));
    cancelButton.setOnClick([this] { onCancel(); });
    menuButtonsVBox.addWidget(&cancelButton);
}

SinglePlayerMenu::~SinglePlayerMenu() = default;

void SinglePlayerMenu::onCampaign() {
    int player = HouseChoiceMenu().showMenu();

    if (player < 0) {
        return;
    }

    GameInitSettings init((HOUSETYPE)player, settings.gameOptions);

    for_each_housetype([&](const auto houseID) {
        if (houseID == static_cast<HOUSETYPE>(player)) {
            GameInitSettings::HouseInfo humanHouseInfo((HOUSETYPE)player, 1);
            humanHouseInfo.addPlayerInfo(GameInitSettings::PlayerInfo(settings.general.playerName, HUMANPLAYERCLASS));
            init.addHouseInfo(humanHouseInfo);
        } else {
            GameInitSettings::HouseInfo aiHouseInfo(houseID, 2);
            aiHouseInfo.addPlayerInfo(GameInitSettings::PlayerInfo(getHouseNameByNumber(houseID), settings.ai.campaignAI));
            init.addHouseInfo(aiHouseInfo);
        }
    });

    startSinglePlayerGame(init);

    quit();
}

void SinglePlayerMenu::onCustom() {
    CustomGameMenu(false).showMenu();
}

void SinglePlayerMenu::onSkirmish() {
    SinglePlayerSkirmishMenu().showMenu();
}

void SinglePlayerMenu::onLoadSavegame() {
    auto [ok, savepath] = fnkdat("save/", FNKDAT_USER | FNKDAT_CREAT);
    openWindow(LoadSaveWindow::create(false, _("Load Game"), savepath, "dls"));
}

void SinglePlayerMenu::onLoadReplay() {
    auto [ok, replaypath] = fnkdat("replay/", FNKDAT_USER | FNKDAT_CREAT);
    openWindow(LoadSaveWindow::create(false, _("Load Replay"), replaypath, "rpl"));
}

void SinglePlayerMenu::onCancel() {
    quit();
}

void SinglePlayerMenu::onChildWindowClose(Window* pChildWindow) {
    std::filesystem::path filename;
    std::string extension;
    auto* pLoadSaveWindow = dynamic_cast<LoadSaveWindow*>(pChildWindow);
    if (pLoadSaveWindow != nullptr) {
        filename  = pLoadSaveWindow->getFilename();
        extension = pLoadSaveWindow->getExtension();
    }

    if (filename != "") {
        if (extension == "dls") {

            try {
                startSinglePlayerGame(GameInitSettings(std::move(filename)));
            } catch (std::exception& e) {
                // most probably the savegame file is not valid or from a different dune legacy version
                openWindow(MsgBox::create(e.what()));
            }
        } else if (extension == "rpl") {
            startReplay(filename);
        }
    }
}
