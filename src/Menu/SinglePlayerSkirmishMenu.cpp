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

#include <Menu/SinglePlayerSkirmishMenu.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>

#include <GameInitSettings.h>
#include <sand.h>

static const int houseOrder[] = { HOUSE_ATREIDES, HOUSE_ORDOS, HOUSE_HARKONNEN, HOUSE_MERCENARY, HOUSE_FREMEN, HOUSE_SARDAUKAR };

SinglePlayerSkirmishMenu::SinglePlayerSkirmishMenu() : MenuBase()
{
    currentHouseChoiceScrollPos = 0;
    selectedButton = 1;
	mission = 1;

	// set up window
	SDL_Surface *surf, *surfPressed;
	surf = pGFXManager->getUIGraphic(UI_MenuBackground);

	setBackground(surf,false);
	resize(surf->w,surf->h);

	setWindowWidget(&windowWidget);

	// set up pictures in the background
	surf = pGFXManager->getUIGraphic(UI_DuneLegacy);
	duneLegacy.setSurface(surf,false);
	windowWidget.addWidget(&duneLegacy,
							Point((screen->w - surf->w)/2, screen->h/2 + 28),
							Point(surf->w,surf->h));

	surf = pGFXManager->getUIGraphic(UI_MenuButtonBorder);
	buttonBorder.setSurface(surf,false);
	windowWidget.addWidget(&buttonBorder,
							Point((screen->w - surf->w)/2, screen->h/2 + 59),
							Point(surf->w,surf->h));

	// set up menu buttons
	windowWidget.addWidget(&menuButtonsVBox,Point((screen->w - 160)/2,screen->h/2 + 64),
										Point(160,111));

	startButton.setText(_("Start"));
	startButton.setOnClick(std::bind(&SinglePlayerSkirmishMenu::onStart, this));
	menuButtonsVBox.addWidget(&startButton);
	startButton.setActive();

	menuButtonsVBox.addWidget(VSpacer::create(79));

	backButton.setText(_("Back"));
	backButton.setOnClick(std::bind(&SinglePlayerSkirmishMenu::onCancel, this));
	menuButtonsVBox.addWidget(&backButton);

	// set up house choice

	surf = pGFXManager->getUIGraphic(UI_HouseSelect);
	windowWidget.addWidget(&houseChoiceContainer,
							Point((screen->w - surf->w)/2,screen->h/2 - surf->h + 10),
							Point(surf->w,surf->h));

	heraldPicture.setSurface(surf,false);
	houseChoiceContainer.addWidget(&heraldPicture, Point(0,0), Point(surf->w,surf->h));


	// House1 button
	houseChoiceContainer.addWidget(	&house1Picture, Point(21,54), Point(83,91));
	houseChoiceContainer.addWidget(	&house1SelectedPicture, Point(20,53), Point(83,91));

	house1Button.setOnClick(std::bind(&SinglePlayerSkirmishMenu::onSelectHouseButton, this, 0));
	houseChoiceContainer.addWidget(	&house1Button, Point(20,53), Point(83,91));

	// House2 button
	houseChoiceContainer.addWidget(	&house2Picture, Point(117,54), Point(83,91));
	houseChoiceContainer.addWidget(	&house2SelectedPicture, Point(116,53), Point(83,91));

	house2Button.setOnClick(std::bind(&SinglePlayerSkirmishMenu::onSelectHouseButton, this, 1));
	houseChoiceContainer.addWidget(	&house2Button, Point(116,53), Point(83,91));

	// House3 button
	houseChoiceContainer.addWidget(	&house3Picture, Point(215,54), Point(83,91));
	houseChoiceContainer.addWidget(	&house3SelectedPicture, Point(214,53), Point(83,91));

	house3Button.setOnClick(std::bind(&SinglePlayerSkirmishMenu::onSelectHouseButton, this, 2));
	houseChoiceContainer.addWidget(	&house3Button, Point(214,53), Point(83,91));

    surf = pGFXManager->getUIGraphic(UI_Herald_ArrowLeft);
    surfPressed = pGFXManager->getUIGraphic(UI_Herald_ArrowLeftHighlight);
    houseLeftButton.setSurfaces(surf, false, surf, false, surfPressed, false);
	houseLeftButton.setOnClick(std::bind(&SinglePlayerSkirmishMenu::onHouseLeft, this));
	houseLeftButton.setVisible(false);
	houseChoiceContainer.addWidget(	&houseLeftButton, Point(houseChoiceContainer.getSize().x/2 - surf->w - 85, 160), Point(surf->w,surf->h));

    surf = pGFXManager->getUIGraphic(UI_Herald_ArrowRight);
    surfPressed = pGFXManager->getUIGraphic(UI_Herald_ArrowRightHighlight);
    houseRightButton.setSurfaces(surf, false, surf, false, surfPressed, false);
	houseRightButton.setOnClick(std::bind(&SinglePlayerSkirmishMenu::onHouseRight, this));
	houseChoiceContainer.addWidget(	&houseRightButton, Point(houseChoiceContainer.getSize().x/2 + 85, 160), Point(surf->w,surf->h));

    updateHouseChoice();

	onSelectHouseButton(1);


	// setup +/- Buttons to select misson

	missionCounter.setCount(mission);
	windowWidget.addWidget(	&missionCounter,
							Point(((screen->w/4)*3 + 160/4) - 83/2,screen->h/2 + 89),
							missionCounter.getMinimumSize());



	surf = pGFXManager->getUIGraphic(UI_Plus);
	surfPressed = pGFXManager->getUIGraphic(UI_Plus_Pressed);
	missionPlusButton.setSurfaces(surf,false,surfPressed,false);
	missionPlusButton.setOnClick(std::bind(&SinglePlayerSkirmishMenu::onMissionIncrement, this));
	windowWidget.addWidget(	&missionPlusButton,
							Point(((screen->w/4)*3 + 160/4) - surf->w/2 + 72,screen->h/2 + 96),
							Point(surf->w,surf->h));


	surf = pGFXManager->getUIGraphic(UI_Minus);
	surfPressed = pGFXManager->getUIGraphic(UI_Minus_Pressed);
	missionMinusButton.setSurfaces(surf,false,surfPressed,false);
	missionMinusButton.setOnClick(std::bind(&SinglePlayerSkirmishMenu::onMissionDecrement, this));
	windowWidget.addWidget(	&missionMinusButton,
							Point(((screen->w/4)*3 + 160/4) - surf->w/2 + 72,screen->h/2 + 109),
							Point(surf->w,surf->h));

}

SinglePlayerSkirmishMenu::~SinglePlayerSkirmishMenu()
{
	;
}

void SinglePlayerSkirmishMenu::onDifficulty()
{
}


void SinglePlayerSkirmishMenu::onStart()
{
    HOUSETYPE houseChoice = (HOUSETYPE) houseOrder[currentHouseChoiceScrollPos + selectedButton];

	GameInitSettings init(houseChoice, mission, settings.gameOptions);

    for(int houseID = 0; houseID < NUM_HOUSES; houseID++) {
	    if(houseID == houseChoice) {
	        GameInitSettings::HouseInfo humanHouseInfo(houseChoice, 1);
	        humanHouseInfo.addPlayerInfo( GameInitSettings::PlayerInfo(settings.general.playerName, HUMANPLAYERCLASS) );
            init.addHouseInfo(humanHouseInfo);
	    } else {
            GameInitSettings::HouseInfo aiHouseInfo((HOUSETYPE) houseID, 2);
	        aiHouseInfo.addPlayerInfo( GameInitSettings::PlayerInfo(getHouseNameByNumber( (HOUSETYPE) houseID), settings.ai.campaignAI) );
            init.addHouseInfo(aiHouseInfo);
	    }
	}

	startSinglePlayerGame(init);

	quit();
}

void SinglePlayerSkirmishMenu::onCancel()
{
	quit();
}

void SinglePlayerSkirmishMenu::onSelectHouseButton(int button)
{
    selectedButton = button;

    house1Button.setEnabled( (selectedButton != 0) );
    house2Button.setEnabled( (selectedButton != 1) );
    house3Button.setEnabled( (selectedButton != 2) );

    house1Picture.setVisible( (selectedButton != 0) );
    house2Picture.setVisible( (selectedButton != 1) );
    house3Picture.setVisible( (selectedButton != 2) );

    house1SelectedPicture.setVisible( (selectedButton == 0) );
    house2SelectedPicture.setVisible( (selectedButton == 1) );
    house3SelectedPicture.setVisible( (selectedButton == 2) );
}

void SinglePlayerSkirmishMenu::onHouseLeft()
{
    if(currentHouseChoiceScrollPos > 0) {
        currentHouseChoiceScrollPos--;
        selectedButton++;
        onSelectHouseButton(selectedButton);
        updateHouseChoice();

        houseLeftButton.setVisible( (currentHouseChoiceScrollPos > 0) );
        houseRightButton.setVisible(true);
    }
}

void SinglePlayerSkirmishMenu::onHouseRight()
{
    if(currentHouseChoiceScrollPos < 3) {
        currentHouseChoiceScrollPos++;
        selectedButton--;
        onSelectHouseButton(selectedButton);
        updateHouseChoice();

        houseLeftButton.setVisible(true);
        houseRightButton.setVisible( (currentHouseChoiceScrollPos < 3) );
    }
}

void SinglePlayerSkirmishMenu::onMissionIncrement()
{
	mission++;
	if(mission > 22) {
		mission = 1;
	}
	missionCounter.setCount(mission);
}

void SinglePlayerSkirmishMenu::onMissionDecrement()
{
	mission--;
	if(mission < 1) {
		mission = 22;
	}
	missionCounter.setCount(mission);
}

void SinglePlayerSkirmishMenu::updateHouseChoice()
{
    // House1 button
	house1Picture.setSurface(pGFXManager->getUIGraphic(UI_Herald_Grey, houseOrder[currentHouseChoiceScrollPos+0]),false);
	house1SelectedPicture.setSurface(pGFXManager->getUIGraphic(UI_Herald_Colored, houseOrder[currentHouseChoiceScrollPos+0]),false);

	// House2 button
	house2Picture.setSurface(pGFXManager->getUIGraphic(UI_Herald_Grey, houseOrder[currentHouseChoiceScrollPos+1]),false);
	house2SelectedPicture.setSurface(pGFXManager->getUIGraphic(UI_Herald_Colored, houseOrder[currentHouseChoiceScrollPos+1]),false);

	// House3 button
	house3Picture.setSurface(pGFXManager->getUIGraphic(UI_Herald_Grey, houseOrder[currentHouseChoiceScrollPos+2]),false);
	house3SelectedPicture.setSurface(pGFXManager->getUIGraphic(UI_Herald_Colored, houseOrder[currentHouseChoiceScrollPos+2]),false);
}
