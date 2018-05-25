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
    int xpos = std::max(0,(getRendererWidth() - 640)/2);
    int ypos = std::max(0,(getRendererHeight() - 400)/2);

    setCurrentPosition(xpos,ypos,640,400);

    setTransparentBackground(true);

    setWindowWidget(&windowWidget);


    selectYourHouseLabel.setTexture(pGFXManager->getUIGraphic(UI_SelectYourHouseLarge));
    windowWidget.addWidget(&selectYourHouseLabel, Point(0,0), Point(100, 640));

    // set up buttons
    house1Button.setOnClick(std::bind(&HouseChoiceMenu::onHouseButton, this, 0));
    windowWidget.addWidget(&house1Button, Point(40,108),    Point(168,182));

    house2Button.setOnClick(std::bind(&HouseChoiceMenu::onHouseButton, this, 1));
    windowWidget.addWidget(&house2Button, Point(235,108),   Point(168,182));

    house3Button.setOnClick(std::bind(&HouseChoiceMenu::onHouseButton, this, 2));
    windowWidget.addWidget(&house3Button, Point(430,108),   Point(168,182));

    SDL_Texture *pArrowLeft = pGFXManager->getUIGraphic(UI_Herald_ArrowLeftLarge);
    SDL_Texture *pArrowLeftHighlight = pGFXManager->getUIGraphic(UI_Herald_ArrowLeftHighlightLarge);
    houseLeftButton.setTextures(pArrowLeft, pArrowLeft, pArrowLeftHighlight);
    houseLeftButton.setOnClick(std::bind(&HouseChoiceMenu::onHouseLeft, this));
    houseLeftButton.setVisible(false);
    windowWidget.addWidget( &houseLeftButton, Point(320 - getWidth(pArrowLeft) - 85, 360), getTextureSize(pArrowLeft));

    SDL_Texture *pArrowRight = pGFXManager->getUIGraphic(UI_Herald_ArrowRightLarge);
    SDL_Texture *pArrowRightHighlight = pGFXManager->getUIGraphic(UI_Herald_ArrowRightHighlightLarge);
    houseRightButton.setTextures(pArrowRight, pArrowRight, pArrowRightHighlight);
    houseRightButton.setOnClick(std::bind(&HouseChoiceMenu::onHouseRight, this));
    windowWidget.addWidget( &houseRightButton, Point(320 + 85, 360), getTextureSize(pArrowRight));

    updateHouseChoice();
}

HouseChoiceMenu::~HouseChoiceMenu() = default;

void HouseChoiceMenu::onHouseButton(int button) {
    int selectedHouse = houseOrder[currentHouseChoiceScrollPos+button];

    switch(selectedHouse) {
        case HOUSE_HARKONNEN:   soundPlayer->playVoice(HouseHarkonnen, selectedHouse);     break;
        case HOUSE_ATREIDES:    soundPlayer->playVoice(HouseAtreides, selectedHouse);      break;
        case HOUSE_ORDOS:       soundPlayer->playVoice(HouseOrdos, selectedHouse);         break;
        default:                /* no sounds for the other houses avail.*/  break;

    }

    int ret = HouseChoiceInfoMenu(selectedHouse).showMenu();
    quit(ret == MENU_QUIT_DEFAULT ? MENU_QUIT_DEFAULT : selectedHouse);
}


void HouseChoiceMenu::updateHouseChoice() {
    // House1 button
    house1Button.setTextures(pGFXManager->getUIGraphic(UI_Herald_ColoredLarge, houseOrder[currentHouseChoiceScrollPos+0]));

    // House2 button
    house2Button.setTextures(pGFXManager->getUIGraphic(UI_Herald_ColoredLarge, houseOrder[currentHouseChoiceScrollPos+1]));

    // House3 button
    house3Button.setTextures(pGFXManager->getUIGraphic(UI_Herald_ColoredLarge, houseOrder[currentHouseChoiceScrollPos+2]));
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
