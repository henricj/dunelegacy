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

#include <GUI/dune/InGameMenu.h>

#include <globals.h>

#include "misc/Fullscreen.h"
#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <Game.h>
#include <GameInitSettings.h>
#include <misc/fnkdat.h>

#include <GUI/MsgBox.h>
#include <GUI/QstBox.h>
#include <GUI/dune/InGameSettingsMenu.h>
#include <GUI/dune/LoadSaveWindow.h>

InGameMenu::InGameMenu(bool bMultiplayer, int color) : Window(0, 0, 0, 0), bMultiplayer(bMultiplayer), color(color) {
    // set up window
    const auto* pBackground = dune::globals::pGFXManager->getUIGraphic(UI_GameMenu);
    setBackground(pBackground);

    InGameMenu::setCurrentPosition(calcAlignedDrawingRect(pBackground, HAlign::Center, VAlign::Center));

    InGameMenu::setWindowWidget(&mainHBox);

    mainHBox.addWidget(Widget::create<HSpacer>(22).release());
    mainHBox.addWidget(&mainVBox);
    mainHBox.addWidget(Widget::create<HSpacer>(22).release());

    mainVBox.addWidget(Widget::create<VSpacer>(34).release());

    resumeButton.setText(_("Resume Game"));
    resumeButton.setTextColor(color);
    resumeButton.setOnClick([] { onResume(); });
    mainVBox.addWidget(&resumeButton);

    mainVBox.addWidget(Widget::create<VSpacer>(3).release());

    saveGameButton.setText(_("Save Game"));
    saveGameButton.setTextColor(color);
    saveGameButton.setOnClick([this] { onSave(); });
    mainVBox.addWidget(&saveGameButton);

    mainVBox.addWidget(Widget::create<VSpacer>(3).release());

    loadGameButton.setText(_("Load Game"));
    loadGameButton.setTextColor(color);
    loadGameButton.setOnClick([this] { onLoad(); });
    loadGameButton.setVisible(!bMultiplayer);
    loadGameButton.setEnabled(!bMultiplayer);
    mainVBox.addWidget(&loadGameButton);

    mainVBox.addWidget(Widget::create<VSpacer>(3).release());

    gameSettingsButton.setText(_("Game Settings"));
    gameSettingsButton.setTextColor(color);
    gameSettingsButton.setOnClick([this] { onSettings(); });
    gameSettingsButton.setVisible(!bMultiplayer);
    gameSettingsButton.setEnabled(!bMultiplayer);
    mainVBox.addWidget(&gameSettingsButton);

    mainVBox.addWidget(Widget::create<VSpacer>(3).release());

    restartGameButton.setText(_("Restart Game"));
    restartGameButton.setTextColor(color);
    restartGameButton.setOnClick([this] { onRestart(); });
    restartGameButton.setVisible(!bMultiplayer);
    restartGameButton.setEnabled(!bMultiplayer);
    mainVBox.addWidget(&restartGameButton);

    mainVBox.addWidget(Widget::create<VSpacer>(3).release());

    quitButton.setText(_("Quit to Menu"));
    quitButton.setTextColor(color);
    quitButton.setOnClick([this] { onQuit(); });
    mainVBox.addWidget(&quitButton);

    mainVBox.addWidget(Widget::create<VSpacer>(6).release());
}

InGameMenu::~InGameMenu() = default;

bool InGameMenu::handleKeyPress(const SDL_KeyboardEvent& key) {
    switch (key.keysym.sym) {
        case SDLK_ESCAPE: {
            dune::globals::currentGame->resumeGame();
        } break;

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

    return Window::handleKeyPress(key);
}

void InGameMenu::onChildWindowClose(Window* pChildWindow) {
    auto* const game = dune::globals::currentGame.get();

    if (const auto* pLoadSaveWindow = dynamic_cast<LoadSaveWindow*>(pChildWindow)) {
        auto FileName    = pLoadSaveWindow->getFilename();
        const bool bSave = pLoadSaveWindow->isSaveWindow();

        if (!FileName.empty()) {
            if (!bSave) {
                // load window
                try {
                    game->setNextGameInitSettings(GameInitSettings(std::move(FileName)));
                } catch (std::exception& e) {
                    // most probably the savegame file is not valid or from a different dune legacy version
                    openWindow(MsgBox::create(std::string{e.what()}));
                }

                game->resumeGame();
                game->quitGame();

            } else {
                // save window
                game->saveGame(FileName);

                game->resumeGame();
            }
        }
    } else if (auto* const pQstBox = dynamic_cast<QstBox*>(pChildWindow)) {
        if (pQstBox->getPressedButtonID() == QSTBOX_BUTTON1) {
            if (pQstBox->getText() == _("Do you really want to quit this game?")) {
                // quit
                game->quitGame();
            } else {
                // restart
                // set new current init settings as init info for next game
                game->setNextGameInitSettings(game->getGameInitSettings());

                // quit current game
                game->resumeGame();
                game->quitGame();
            }
        }
    }
}

void InGameMenu::onResume() {
    dune::globals::currentGame->resumeGame();
}

void InGameMenu::onSettings() {
    openWindow(InGameSettingsMenu::create());
}

void InGameMenu::onSave() {
    auto [ok, savepath] = fnkdat(bMultiplayer ? "mpsave/" : "save/", FNKDAT_USER | FNKDAT_CREAT);
    openWindow(
        LoadSaveWindow::create(true, std::string{_("Save Game")}, std::move(savepath), "dls", "", color).release());
}

void InGameMenu::onLoad() {
    auto [ok, savepath] = fnkdat("save/", FNKDAT_USER | FNKDAT_CREAT);
    openWindow(
        LoadSaveWindow::create(false, std::string{_("Load Game")}, std::move(savepath), "dls", "", color).release());
}

void InGameMenu::onRestart() {
    QstBox* pQstBox = QstBox::create(_("Do you really want to restart this game?"), _("Yes"), _("No"), QSTBOX_BUTTON2);

    pQstBox->setTextColor(color);

    openWindow(pQstBox);
}

void InGameMenu::onQuit() {
    QstBox* pQstBox = QstBox::create(_("Do you really want to quit this game?"), _("Yes"), _("No"), QSTBOX_BUTTON2);

    pQstBox->setTextColor(color);

    openWindow(pQstBox);
}
