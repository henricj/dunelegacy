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

#include <Menu/HouseChoiceMenu.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <Menu/HouseChoiceInfoMenu.h>
#include <SoundPlayer.h>


static const int houseOrder[] = { HOUSE_ATREIDES, HOUSE_ORDOS, HOUSE_HARKONNEN, HOUSE_MERCENARY, HOUSE_FREMEN, HOUSE_SARDAUKAR };

HouseChoiceMenu::HouseChoiceMenu() : MenuBase()
{
    currentHouseChoiceScrollPos = 0;

	// set up window
	int xpos = std::max(0,(screen->w - 640)/2);
	int ypos = std::max(0,(screen->h - 400)/2);

	setCurrentPosition(xpos,ypos,640,400);

	setTransparentBackground(true);

	setWindowWidget(&windowWidget);


	selectYourHouseLabel.setSurface(pGFXManager->getUIGraphic(UI_SelectYourHouseLarge), false);
	windowWidget.addWidget(&selectYourHouseLabel, Point(0,0), Point(100, 640));

	// set up buttons
	house1Button.setOnClick(std::bind(&HouseChoiceMenu::onHouseButton, this, 0));
	windowWidget.addWidget(&house1Button, Point(40,108),	Point(168,182));

	house2Button.setOnClick(std::bind(&HouseChoiceMenu::onHouseButton, this, 1));
	windowWidget.addWidget(&house2Button, Point(235,108),	Point(168,182));

	house3Button.setOnClick(std::bind(&HouseChoiceMenu::onHouseButton, this, 2));
	windowWidget.addWidget(&house3Button, Point(430,108),	Point(168,182));

	SDL_Surface* surf;
	SDL_Surface* surfPressed;

    surf = pGFXManager->getUIGraphic(UI_Herald_ArrowLeftLarge);
    surfPressed = pGFXManager->getUIGraphic(UI_Herald_ArrowLeftHighlightLarge);
    houseLeftButton.setSurfaces(surf, false, surf, false, surfPressed, false);
	houseLeftButton.setOnClick(std::bind(&HouseChoiceMenu::onHouseLeft, this));
	houseLeftButton.setVisible(false);
	windowWidget.addWidget(	&houseLeftButton, Point(320 - surf->w - 85, 360), Point(surf->w,surf->h));

    surf = pGFXManager->getUIGraphic(UI_Herald_ArrowRightLarge);
    surfPressed = pGFXManager->getUIGraphic(UI_Herald_ArrowRightHighlightLarge);
    houseRightButton.setSurfaces(surf, false, surf, false, surfPressed, false);
	houseRightButton.setOnClick(std::bind(&HouseChoiceMenu::onHouseRight, this));
	windowWidget.addWidget(	&houseRightButton, Point(320 + 85, 360), Point(surf->w,surf->h));

	updateHouseChoice();
}

HouseChoiceMenu::~HouseChoiceMenu() {
}

void HouseChoiceMenu::onHouseButton(int button) {
    int selectedHouse = houseOrder[currentHouseChoiceScrollPos+button];

    switch(selectedHouse) {
        case HOUSE_HARKONNEN:   soundPlayer->playSound(HouseHarkonnen);     break;
        case HOUSE_ATREIDES:    soundPlayer->playSound(HouseAtreides);      break;
        case HOUSE_ORDOS:       soundPlayer->playSound(HouseOrdos);         break;
        default:                /* no sounds for the other houses avail.*/  break;

    }

	HouseChoiceInfoMenu* myHouseChoiceInfoMenu = new HouseChoiceInfoMenu(selectedHouse);
	if(myHouseChoiceInfoMenu->showMenu() == MENU_QUIT_DEFAULT) {
		quit(MENU_QUIT_DEFAULT);
	} else {
		quit(selectedHouse);
	}
	delete myHouseChoiceInfoMenu;
}


void HouseChoiceMenu::updateHouseChoice() {
    // House1 button
	house1Button.setSurfaces(pGFXManager->getUIGraphic(UI_Herald_ColoredLarge, houseOrder[currentHouseChoiceScrollPos+0]),false);

	// House2 button
	house2Button.setSurfaces(pGFXManager->getUIGraphic(UI_Herald_ColoredLarge, houseOrder[currentHouseChoiceScrollPos+1]),false);

	// House3 button
	house3Button.setSurfaces(pGFXManager->getUIGraphic(UI_Herald_ColoredLarge, houseOrder[currentHouseChoiceScrollPos+2]),false);
}

void HouseChoiceMenu::onHouseLeft()
{
    if(currentHouseChoiceScrollPos > 0) {
        currentHouseChoiceScrollPos--;
        updateHouseChoice();

        houseLeftButton.setVisible( (currentHouseChoiceScrollPos > 0) );
        houseRightButton.setVisible(true);
    }
}

void HouseChoiceMenu::onHouseRight()
{
    if(currentHouseChoiceScrollPos < 3) {
        currentHouseChoiceScrollPos++;
        updateHouseChoice();

        houseLeftButton.setVisible(true);
        houseRightButton.setVisible( (currentHouseChoiceScrollPos < 3) );
    }
}
