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

#include <Menu/MainMenu.h>

#include <globals.h>

#include "misc/DrawingRectHelper.h"
#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/music/MusicPlayer.h>

#include <MapEditor/MapEditor.h>

#include <Menu/AboutMenu.h>
#include <Menu/MultiPlayerMenu.h>
#include <Menu/OptionsMenu.h>
#include <Menu/SinglePlayerMenu.h>

MainMenu::MainMenu() {
    // set up window

    MainMenu::setWindowWidget(&windowWidget);

    const auto* const gfx = dune::globals::pGFXManager.get();

    // set up pictures in the background
    const auto* const pPlanetBackground = gfx->getUIGraphic(UI_PlanetBackground);
    planetPicture.setTexture(pPlanetBackground);
    auto dest1 = calcAlignedDrawingRect(pPlanetBackground);
    dest1.y    = dest1.y - getHeight(pPlanetBackground) / 2 + 10;
    windowWidget.addWidget(&planetPicture, dest1);

    const auto* const pDuneLegacy = gfx->getUIGraphic(UI_DuneLegacy);
    duneLegacy.setTexture(pDuneLegacy);
    auto dest2 = calcAlignedDrawingRect(pDuneLegacy);
    dest2.y    = dest2.y + getHeight(pDuneLegacy) / 2 + 28;
    windowWidget.addWidget(&duneLegacy, dest2);

    const auto* const pMenuButtonBorder = gfx->getUIGraphic(UI_MenuButtonBorder);
    buttonBorder.setTexture(pMenuButtonBorder);
    auto dest3 = calcAlignedDrawingRect(pMenuButtonBorder);
    dest3.y    = dest3.y + getHeight(pMenuButtonBorder) / 2 + 59;
    windowWidget.addWidget(&buttonBorder, dest3);

    // set up menu buttons
    windowWidget.addWidget(&MenuButtons, Point((getRendererWidth() - 160) / 2, getRendererHeight() / 2 + 64),
                           Point(160, 111));

    singlePlayerButton.setText(_("SINGLE PLAYER"));
    singlePlayerButton.setOnClick([this] { onSinglePlayer(); });
    MenuButtons.addWidget(&singlePlayerButton);
    singlePlayerButton.setActive();

    MenuButtons.addWidget(VSpacer::create(3));

    multiPlayerButton.setText(_("MULTIPLAYER"));
    multiPlayerButton.setOnClick([this] { onMultiPlayer(); });
    MenuButtons.addWidget(&multiPlayerButton);

    MenuButtons.addWidget(VSpacer::create(3));

    //    MenuButtons.addWidget(VSpacer::create(16));
    mapEditorButton.setText(_("MAP EDITOR"));
    mapEditorButton.setOnClick([this] { onMapEditor(); });
    MenuButtons.addWidget(&mapEditorButton);

    MenuButtons.addWidget(VSpacer::create(3));

    optionsButton.setText(_("OPTIONS"));
    optionsButton.setOnClick([this] { onOptions(); });
    MenuButtons.addWidget(&optionsButton);

    MenuButtons.addWidget(VSpacer::create(3));

    aboutButton.setText(_("ABOUT"));
    aboutButton.setOnClick([this] { onAbout(); });
    MenuButtons.addWidget(&aboutButton);

    MenuButtons.addWidget(VSpacer::create(3));

    quitButton.setText(_("QUIT"));
    quitButton.setOnClick([this] { onQuit(); });
    MenuButtons.addWidget(&quitButton);
}

MainMenu::~MainMenu() = default;

void MainMenu::resize(uint32_t width, uint32_t height) {
    parent::resize(width, height);

    const auto iWidth  = static_cast<int>(width);
    const auto iHeight = static_cast<int>(height);

    const auto planet_size = planetPicture.getSize();
    windowWidget.setWidgetGeometry(&planetPicture, {(iWidth - planet_size.x) / 2, iHeight / 2 - planet_size.y + 10},
                                   planet_size);

    const auto dune_size = duneLegacy.getSize();
    windowWidget.setWidgetGeometry(&duneLegacy, {(iWidth - dune_size.x) / 2, iHeight / 2 + 28}, dune_size);

    const auto border_size = buttonBorder.getSize();
    windowWidget.setWidgetGeometry(&buttonBorder, {(iWidth - border_size.x) / 2, iHeight / 2 + 59}, border_size);

    const auto menu_size = MenuButtons.getSize();
    windowWidget.setWidgetGeometry(&MenuButtons, {(iWidth - 160) / 2, iHeight / 2 + 64}, menu_size);
}

int MainMenu::showMenuImpl() {
    dune::globals::musicPlayer->changeMusic(MUSIC_MENU);

    return parent::showMenuImpl();
}

void MainMenu::onSinglePlayer() {
    SinglePlayerMenu singlePlayerMenu;
    singlePlayerMenu.showMenu([&](const auto& e) { doInput(e); });
}

void MainMenu::onMultiPlayer() {
    MultiPlayerMenu multiPlayerMenu;
    multiPlayerMenu.showMenu([&](const auto& e) { doInput(e); });
}

void MainMenu::onMapEditor() {
    MapEditor mapEditor;
    mapEditor.RunEditor([&](const auto& e) { doInput(e); });
}

void MainMenu::onOptions() {
    OptionsMenu optionsMenu;
    const int ret = optionsMenu.showMenu([&](const auto& e) { doInput(e); });

    if (ret == MENU_QUIT_REINITIALIZE) {
        quit(MENU_QUIT_REINITIALIZE);
    }
}

void MainMenu::onAbout() {
    AboutMenu myAbout;
    myAbout.showMenu([&](const auto& e) { doInput(e); });
}

void MainMenu::onQuit() {
    quit();
}
