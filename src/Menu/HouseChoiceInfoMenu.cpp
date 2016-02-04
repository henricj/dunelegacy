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

#include <Menu/HouseChoiceInfoMenu.h>

#include <globals.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/Palfile.h>

#include <string>

HouseChoiceInfoMenu::HouseChoiceInfoMenu(int newHouse) : MentatMenu(HOUSE_INVALID)
{
	disableQuiting(true);

	house = newHouse;

	Animation* anim = NULL;

	switch(house) {
		case HOUSE_HARKONNEN:   anim = pGFXManager->getAnimation(Anim_HarkonnenPlanet); break;
		case HOUSE_ATREIDES:    anim = pGFXManager->getAnimation(Anim_AtreidesPlanet);  break;
		case HOUSE_ORDOS:       anim = pGFXManager->getAnimation(Anim_OrdosPlanet);     break;
		case HOUSE_FREMEN:      anim = pGFXManager->getAnimation(Anim_FremenPlanet);    break;
		case HOUSE_SARDAUKAR:   anim = pGFXManager->getAnimation(Anim_SardaukarPlanet); break;
		case HOUSE_MERCENARY:   anim = pGFXManager->getAnimation(Anim_MercenaryPlanet); break;
		default:                                                                        break;
	}

	planetAnimation.setAnimation(anim);
	windowWidget.addWidget(&planetAnimation, Point(256,96), Point(anim->getFrame()->w, anim->getFrame()->h));

	SDL_Surface* pQuestionSurface = pGFXManager->getUIGraphic(UI_MentatHouseChoiceInfoQuestion, newHouse);
	questionLabel.setSurface(pQuestionSurface, false);
	windowWidget.addWidget(&questionLabel, Point(0,0), Point(pQuestionSurface->w, pQuestionSurface->h));
	questionLabel.setVisible(false);

	// init textbox but skip first line (this line contains "House ???")
	std::string desc = pTextManager->getBriefingText(0,MISSION_DESCRIPTION,house);
	int linebreak = desc.find("\n",0) + 1;
	setText(desc.substr(linebreak,desc.length()-linebreak));

	SDL_Surface* surf;
	SDL_Surface* surfPressed;

	surf = pGFXManager->getUIGraphic(UI_MentatNo);
	surfPressed = pGFXManager->getUIGraphic(UI_MentatNo_Pressed);

	noButton.setSurfaces(surf,false,surfPressed,false);
	noButton.setEnabled(false);
	noButton.setVisible(false);
	noButton.setOnClick(std::bind(&HouseChoiceInfoMenu::onNo, this));
	windowWidget.addWidget(&noButton,Point(370,340),Point(surf->w,surf->h));

	surf = pGFXManager->getUIGraphic(UI_MentatYes);
	surfPressed = pGFXManager->getUIGraphic(UI_MentatYes_Pressed);

	yesButton.setSurfaces(surf,false,surfPressed,false);
	yesButton.setEnabled(false);
	yesButton.setVisible(false);
	yesButton.setOnClick(std::bind(&HouseChoiceInfoMenu::onYes, this));
	windowWidget.addWidget(&yesButton,Point(480,340),Point(surf->w,surf->h));
}

HouseChoiceInfoMenu::~HouseChoiceInfoMenu() {
    SDL_FillRect(screen, NULL, COLOR_BLACK);
}

void HouseChoiceInfoMenu::onMentatTextFinished() {
	yesButton.setEnabled(true);
	yesButton.setVisible(true);
    noButton.setEnabled(true);
	noButton.setVisible(true);

	questionLabel.setVisible(true);
}

void HouseChoiceInfoMenu::drawSpecificStuff() {
	MentatMenu::drawSpecificStuff();
}

void HouseChoiceInfoMenu::onYes() {
	quit(MENU_QUIT_HOUSECHOICE_YES);
}

void HouseChoiceInfoMenu::onNo() {
	quit(MENU_QUIT_DEFAULT);
}
