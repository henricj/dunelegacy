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

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <misc/fnkdat.h>
#include <Game.h>
#include <GameInitSettings.h>

#include <GUI/MsgBox.h>
#include <GUI/QstBox.h>
#include <GUI/dune/InGameSettingsMenu.h>
#include <GUI/dune/LoadSaveWindow.h>


InGameMenu::InGameMenu(bool bMultiplayer, int color)
 : Window(0,0,0,0), bMultiplayer(bMultiplayer), color(color) {
	// set up window
	SDL_Surface *surf;
	surf = pGFXManager->getUIGraphic(UI_GameMenu);

	setBackground(surf,false);

	int xpos = std::max(0,(screen->w - surf->w)/2);
	int ypos = std::max(0,(screen->h - surf->h)/2);

	setCurrentPosition(xpos,ypos,surf->w,surf->h);

	setWindowWidget(&mainHBox);

	mainHBox.addWidget(HSpacer::create(22));
	mainHBox.addWidget(&mainVBox);
	mainHBox.addWidget(HSpacer::create(22));


	mainVBox.addWidget(VSpacer::create(34));

	resumeButton.setText(_("Resume Game"));
	resumeButton.setTextColor(color);
	resumeButton.setOnClick(std::bind(&InGameMenu::onResume, this));
	mainVBox.addWidget(&resumeButton);

	mainVBox.addWidget(VSpacer::create(3));

	saveGameButton.setText(_("Save Game"));
	saveGameButton.setTextColor(color);
	saveGameButton.setOnClick(std::bind(&InGameMenu::onSave, this));
	mainVBox.addWidget(&saveGameButton);

	mainVBox.addWidget(VSpacer::create(3));

	loadGameButton.setText(_("Load Game"));
	loadGameButton.setTextColor(color);
	loadGameButton.setOnClick(std::bind(&InGameMenu::onLoad, this));
	loadGameButton.setVisible(bMultiplayer == false);
	loadGameButton.setEnabled(bMultiplayer == false);
	mainVBox.addWidget(&loadGameButton);

	mainVBox.addWidget(VSpacer::create(3));

	gameSettingsButton.setText(_("Game Settings"));
	gameSettingsButton.setTextColor(color);
	gameSettingsButton.setOnClick(std::bind(&InGameMenu::onSettings, this));
	gameSettingsButton.setVisible(bMultiplayer == false);
	gameSettingsButton.setEnabled(bMultiplayer == false);
	mainVBox.addWidget(&gameSettingsButton);

	mainVBox.addWidget(VSpacer::create(3));

	restartGameButton.setText(_("Restart Game"));
	restartGameButton.setTextColor(color);
	restartGameButton.setOnClick(std::bind(&InGameMenu::onRestart, this));
	restartGameButton.setVisible(bMultiplayer == false);
	restartGameButton.setEnabled(bMultiplayer == false);
	mainVBox.addWidget(&restartGameButton);

	mainVBox.addWidget(VSpacer::create(3));

	quitButton.setText(_("Quit to Menu"));
	quitButton.setTextColor(color);
	quitButton.setOnClick(std::bind(&InGameMenu::onQuit, this));
	mainVBox.addWidget(&quitButton);

	mainVBox.addWidget(VSpacer::create(6));
}

InGameMenu::~InGameMenu()
{
	;
}

bool InGameMenu::handleKeyPress(SDL_KeyboardEvent& key) {
	switch( key.keysym.sym ) {
		case SDLK_ESCAPE:
		{
			currentGame->resumeGame();
		} break;

		case SDLK_RETURN:
            if(SDL_GetModState() & KMOD_ALT) {
                SDL_SetWindowFullscreen(window, (SDL_GetWindowFlags(window) ^ SDL_WINDOW_FULLSCREEN_DESKTOP));
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

void InGameMenu::onChildWindowClose(Window* pChildWindow) {
	LoadSaveWindow* pLoadSaveWindow = dynamic_cast<LoadSaveWindow*>(pChildWindow);
	if(pLoadSaveWindow != NULL) {
        std::string FileName = pLoadSaveWindow->getFilename();
		bool bSave = pLoadSaveWindow->isSaveWindow();

        if(FileName != "") {
            if(bSave == false) {
                // load window
                try {
                    currentGame->setNextGameInitSettings(GameInitSettings(FileName));
                } catch (std::exception& e) {
                    // most probably the savegame file is not valid or from a different dune legacy version
                    openWindow(MsgBox::create(e.what()));
                }

                currentGame->resumeGame();
                currentGame->quitGame();

            } else {
                // save window
                currentGame->saveGame(FileName);

                currentGame->resumeGame();
            }
        }
	} else {
        QstBox* pQstBox = dynamic_cast<QstBox*>(pChildWindow);
		if(pQstBox != NULL) {
			if(pQstBox->getPressedButtonID() == QSTBOX_BUTTON2) {
			    if(pQstBox->getText() == _("Do you really want to quit this game?")) {
                    // quit
                    currentGame->quitGame();
			    } else {
			        // restart
			    	// set new current init settings as init info for next game
                    currentGame->setNextGameInitSettings(currentGame->getGameInitSettings());

                    // quit current game
                    currentGame->resumeGame();
                    currentGame->quitGame();
			    }
			}
		}
	}
}

void InGameMenu::onResume()
{
	currentGame->resumeGame();
}

void InGameMenu::onSettings()
{
	openWindow(InGameSettingsMenu::create());
}

void InGameMenu::onSave()
{
	char tmp[FILENAME_MAX];
	fnkdat(bMultiplayer ? "mpsave/" : "save/", tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
	std::string savepath(tmp);
	openWindow(LoadSaveWindow::create(true, _("Save Game"), savepath, "dls", "", color));
}

void InGameMenu::onLoad()
{
	char tmp[FILENAME_MAX];
	fnkdat("save/", tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
	std::string savepath(tmp);
	openWindow(LoadSaveWindow::create(false, _("Load Game"), savepath, "dls", "", color));
}

void InGameMenu::onRestart()
{
    QstBox* pQstBox = QstBox::create(   _("Do you really want to restart this game?"),
                                        _("No"),
                                        _("Yes"),
                                        QSTBOX_BUTTON1);

    pQstBox->setTextColor(color);

    openWindow(pQstBox);
}

void InGameMenu::onQuit()
{
    QstBox* pQstBox = QstBox::create(   _("Do you really want to quit this game?"),
                                        _("No"),
                                        _("Yes"),
                                        QSTBOX_BUTTON1);

    pQstBox->setTextColor(color);

    openWindow(pQstBox);
}
