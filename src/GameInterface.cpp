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

#include <GameInterface.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>

#include <ObjectBase.h>
#include <GUI/ObjectInterfaces/ObjectInterface.h>
#include <GUI/ObjectInterfaces/MultiUnitInterface.h>

#include <misc/draw_util.h>

#include <SDL.h>

GameInterface::GameInterface() : Window(0,0,0,0) {
	pObjectContainer = NULL;
	objectID = NONE;

	setTransparentBackground(true);

	setCurrentPosition(0,0,screen->w,screen->h);

	setWindowWidget(&windowWidget);

	SDL_Surface* surf, *surfPressed;

	// top bar
	surf = pGFXManager->getUIGraphic(UI_TopBar, pLocalHouse->getHouseID());
	topBar.setSurface(surf,false);
	windowWidget.addWidget(&topBar,Point(0,0),Point(surf->w,surf->h - 12));

	// side bar
	surf = pGFXManager->getUIGraphic(UI_SideBar, pLocalHouse->getHouseID());
	sideBar.setSurface(surf,false);
	windowWidget.addWidget(&sideBar,Point(screen->w - surf->w,0),Point(surf->w,surf->h));

	// add buttons
	windowWidget.addWidget(&topBarHBox,Point(5,5),
							Point(screen->w - sideBar.getSize().x, topBar.getSize().y - 10));

	topBarHBox.addWidget(&newsticker);

	topBarHBox.addWidget(Spacer::create());

	surf = pGFXManager->getUIGraphic(UI_Options, pLocalHouse->getHouseID());
	surfPressed = pGFXManager->getUIGraphic(UI_Options_Pressed, pLocalHouse->getHouseID());

	optionsButton.setSurfaces(surf, false, surfPressed, false);
	optionsButton.setOnClick(std::bind(&Game::onOptions, currentGame));
	topBarHBox.addWidget(&optionsButton);

	topBarHBox.addWidget(Spacer::create());

	surf = pGFXManager->getUIGraphic(UI_Mentat, pLocalHouse->getHouseID());
	surfPressed = pGFXManager->getUIGraphic(UI_Mentat_Pressed, pLocalHouse->getHouseID());

	mentatButton.setSurfaces(surf, false, surfPressed, false);
	mentatButton.setOnClick(std::bind(&Game::onMentat, currentGame));
	topBarHBox.addWidget(&mentatButton);

	topBarHBox.addWidget(Spacer::create());

	// add radar
	windowWidget.addWidget(&radarView,Point(screen->w-sideBar.getSize().x+SIDEBAR_COLUMN_WIDTH, 0),radarView.getMinimumSize());
	radarView.setOnRadarClick(std::bind(&Game::onRadarClick, currentGame, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	// add chat manager
	windowWidget.addWidget(&chatManager, Point(20, 60), Point(screen->w - sideBar.getSize().x, 360));
}

GameInterface::~GameInterface() {
	removeOldContainer();
}

void GameInterface::draw(SDL_Surface* screen, Point position) {
	Window::draw(screen,position);

	// draw Power Indicator and Spice indicator

	SDL_Rect PowerIndicatorPos = {	screen->w - sideBar.getSize().x + 14, 146,
									4, screen->h - 146 - 2 };
	SDL_FillRect(screen,&PowerIndicatorPos,0);

	SDL_Rect SpiceIndicatorPos = {	screen->w - sideBar.getSize().x + 20, 146,
									4, screen->h - 146 - 2 };
	SDL_FillRect(screen,&SpiceIndicatorPos,0);

	int xCount = 0, yCount = 0;
	int	yCount2 = 0;

	if (!SDL_MUSTLOCK(screen) || (SDL_LockSurface(screen) == 0)) {
		//draw power level indicator
		if (pLocalHouse->getPowerRequirement() == 0)	{
			if (pLocalHouse->getProducedPower() > 0) {
				yCount2 = PowerIndicatorPos.h + 1;
			} else {
				yCount2 = PowerIndicatorPos.h/2;
			}
		} else {
			yCount2 = lround((double)pLocalHouse->getProducedPower()/(double)pLocalHouse->getPowerRequirement()*(double)(PowerIndicatorPos.h/2));
		}

		if (yCount2 > PowerIndicatorPos.h + 1) {
			yCount2 = PowerIndicatorPos.h + 1;
		}

		for (yCount = 0; yCount < yCount2; yCount++) {
			for (xCount = 1; xCount < PowerIndicatorPos.w - 1; xCount++) {
				if(((yCount/2) % 3) != 0) {
					putPixel(screen, xCount + PowerIndicatorPos.x, PowerIndicatorPos.y + PowerIndicatorPos.h - yCount, COLOR_GREEN);
				}
			}
		}

		//draw spice level indicator
		if (pLocalHouse->getCapacity() == 0) {
			yCount2 = 0;
		} else {
			yCount2 = lround((double)pLocalHouse->getStoredCredits()/(double)pLocalHouse->getCapacity()*(double)SpiceIndicatorPos.h);
		}

		if (yCount2 > SpiceIndicatorPos.h + 1) {
			yCount2 = SpiceIndicatorPos.h + 1;
		}

		for (yCount = 0; yCount < yCount2; yCount++) {
			for (xCount = 1; xCount < SpiceIndicatorPos.w - 1; xCount++) {
				if(((yCount/2) % 3) != 0) {
					putPixel(screen, xCount + SpiceIndicatorPos.x, SpiceIndicatorPos.y + SpiceIndicatorPos.h - yCount, COLOR_ORANGE);
				}
			}
		}

		if (SDL_MUSTLOCK(screen)) {
				SDL_UnlockSurface(screen);
		}
	}

	//draw credits
	char CreditsBuffer[10];
	int credits = pLocalHouse->getCredits();
	sprintf(CreditsBuffer, "%d", (credits < 0) ? 0 : credits);
	int NumDigits = strlen(CreditsBuffer);
	SDL_Surface* surface = pGFXManager->getUIGraphic(UI_CreditsDigits);



	for(int i=NumDigits-1; i>=0; i--) {
	    SDL_Rect source = { (CreditsBuffer[i] - '0')*(surface->w/10), 0,
                            surface->w/10, surface->h };
        SDL_Rect dest = {   (screen->w - sideBar.getSize().x + 49) + (6 - NumDigits + i)*10, 135,
                            surface->w/10, surface->h};
		SDL_BlitSurface(surface, &source, screen, &dest);
	}
}

void GameInterface::updateObjectInterface() {
	if(currentGame->getSelectedList().size() == 1) {
		ObjectBase* pObject = currentGame->getObjectManager().getObject( *(currentGame->getSelectedList().begin()));
		Uint32 newObjectID = pObject->getObjectID();

		if(newObjectID != objectID) {
			removeOldContainer();

			pObjectContainer = pObject->getInterfaceContainer();

			if(pObjectContainer != NULL) {
				objectID = newObjectID;

				windowWidget.addWidget(pObjectContainer,
										Point(screen->w - sideBar.getSize().x + 24, 146),
										Point(sideBar.getSize().x - 25,screen->h - 148));

			}

		} else {
			if(pObjectContainer->update() == false) {
				removeOldContainer();
			}
		}
	} else if(currentGame->getSelectedList().size() > 1) {

	    if((pObjectContainer == NULL) || (objectID != NONE)) {
	        // either there was nothing selected before or exactly one unit

            if(pObjectContainer != NULL) {
                removeOldContainer();
            }

            pObjectContainer = MultiUnitInterface::create();

            windowWidget.addWidget(pObjectContainer,
                                    Point(screen->w - sideBar.getSize().x + 24, 146),
                                    Point(sideBar.getSize().x - 25,screen->h - 148));
	    } else {
	        if(pObjectContainer->update() == false) {
				removeOldContainer();
			}
	    }
	} else {
		removeOldContainer();
	}
}

void GameInterface::removeOldContainer() {
	if(pObjectContainer != NULL) {
		delete pObjectContainer;
		pObjectContainer = NULL;
		objectID = NONE;
	}
}
