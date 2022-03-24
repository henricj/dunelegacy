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

static constexpr HOUSETYPE houseOrder[] = {HOUSETYPE::HOUSE_ATREIDES,  HOUSETYPE::HOUSE_ORDOS,
                                           HOUSETYPE::HOUSE_HARKONNEN, HOUSETYPE::HOUSE_MERCENARY,
                                           HOUSETYPE::HOUSE_FREMEN,    HOUSETYPE::HOUSE_SARDAUKAR};

HouseChoiceMenu::HouseChoiceMenu() : currentHouseChoiceScrollPos(0) {

    // set up window
    const int xpos = std::max(0, (getRendererWidth() - 640) / 2);
    const int ypos = std::max(0, (getRendererHeight() - 400) / 2);

    setCurrentPosition(xpos, ypos, 640, 400);

    setTransparentBackground(true);

    setWindowWidget(&windowWidget);

    selectYourHouseLabel.setTexture(pGFXManager->getUIGraphic(UI_SelectYourHouseLarge));
    windowWidget.addWidget(&selectYourHouseLabel, Point(0, 0), Point(100, 640));

    // set up buttons
    house1Button.setOnClick([&] { onHouseButton(0); });
    windowWidget.addWidget(&house1Button, Point(40, 108), Point(168, 182));

    house2Button.setOnClick([&] { onHouseButton(1); });
    windowWidget.addWidget(&house2Button, Point(235, 108), Point(168, 182));

    house3Button.setOnClick([&] { onHouseButton(2); });
    windowWidget.addWidget(&house3Button, Point(430, 108), Point(168, 182));

    const auto* const pArrowLeft          = pGFXManager->getUIGraphic(UI_Herald_ArrowLeftLarge);
    const auto* const pArrowLeftHighlight = pGFXManager->getUIGraphic(UI_Herald_ArrowLeftHighlightLarge);
    houseLeftButton.setTextures(pArrowLeft, pArrowLeft, pArrowLeftHighlight);
    houseLeftButton.setOnClick([&] { onHouseLeft(); });
    houseLeftButton.setVisible(false);
    windowWidget.addWidget(&houseLeftButton, Point(320 - getWidth(pArrowLeft) - 85, 360), getTextureSize(pArrowLeft));

    const auto* const pArrowRight          = pGFXManager->getUIGraphic(UI_Herald_ArrowRightLarge);
    const auto* const pArrowRightHighlight = pGFXManager->getUIGraphic(UI_Herald_ArrowRightHighlightLarge);
    houseRightButton.setTextures(pArrowRight, pArrowRight, pArrowRightHighlight);
    houseRightButton.setOnClick([&] { onHouseRight(); });
    windowWidget.addWidget(&houseRightButton, Point(320 + 85, 360), getTextureSize(pArrowRight));

    updateHouseChoice();
}

HouseChoiceMenu::~HouseChoiceMenu() = default;

void HouseChoiceMenu::onHouseButton(int button) {
    const auto selectedHouse = houseOrder[currentHouseChoiceScrollPos + button];

    // clang-format off
    switch(selectedHouse) {
        case HOUSETYPE::HOUSE_HARKONNEN:    soundPlayer->playVoice(Voice_enum::HouseHarkonnen, selectedHouse); break;
        case HOUSETYPE::HOUSE_ATREIDES:     soundPlayer->playVoice(Voice_enum::HouseAtreides, selectedHouse); break;
        case HOUSETYPE::HOUSE_ORDOS:        soundPlayer->playVoice(Voice_enum::HouseOrdos, selectedHouse); break;
        default:                /* no sounds for the other houses avail.*/  break;
    }
    // clang-format on

    const int ret = HouseChoiceInfoMenu(selectedHouse).showMenu();
    quit(ret == MENU_QUIT_DEFAULT ? MENU_QUIT_DEFAULT : static_cast<int>(selectedHouse));
}

void HouseChoiceMenu::updateHouseChoice() {
    // House1 button
    house1Button.setTextures(
        pGFXManager->getUIGraphic(UI_Herald_ColoredLarge, houseOrder[currentHouseChoiceScrollPos + 0]));

    // House2 button
    house2Button.setTextures(
        pGFXManager->getUIGraphic(UI_Herald_ColoredLarge, houseOrder[currentHouseChoiceScrollPos + 1]));

    // House3 button
    house3Button.setTextures(
        pGFXManager->getUIGraphic(UI_Herald_ColoredLarge, houseOrder[currentHouseChoiceScrollPos + 2]));
}

void HouseChoiceMenu::onHouseLeft() {
    if (currentHouseChoiceScrollPos > 0) {
        currentHouseChoiceScrollPos--;
        updateHouseChoice();

        houseLeftButton.setVisible((currentHouseChoiceScrollPos > 0));
        houseRightButton.setVisible(true);
    }
}

void HouseChoiceMenu::onHouseRight() {
    if (currentHouseChoiceScrollPos < 3) {
        currentHouseChoiceScrollPos++;
        updateHouseChoice();

        houseLeftButton.setVisible(true);
        houseRightButton.setVisible((currentHouseChoiceScrollPos < 3));
    }
}
