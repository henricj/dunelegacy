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
#include <FileClasses/Palfile.h>
#include <FileClasses/TextManager.h>

HouseChoiceInfoMenu::HouseChoiceInfoMenu(HOUSETYPE newHouse) : MentatMenu(HOUSETYPE::HOUSE_INVALID) {
    disableQuiting(true);

    house = newHouse;

    Animation* anim = nullptr;

    // clang-format off
    switch(house) {
        case HOUSETYPE::HOUSE_HARKONNEN:    anim = pGFXManager->getAnimation(Anim_HarkonnenPlanet); break;
        case HOUSETYPE::HOUSE_ATREIDES:     anim = pGFXManager->getAnimation(Anim_AtreidesPlanet);  break;
        case HOUSETYPE::HOUSE_ORDOS:        anim = pGFXManager->getAnimation(Anim_OrdosPlanet);     break;
        case HOUSETYPE::HOUSE_FREMEN:       anim = pGFXManager->getAnimation(Anim_FremenPlanet);    break;
        case HOUSETYPE::HOUSE_SARDAUKAR:    anim = pGFXManager->getAnimation(Anim_SardaukarPlanet); break;
        case HOUSETYPE::HOUSE_MERCENARY:    anim = pGFXManager->getAnimation(Anim_MercenaryPlanet); break;
        default: {
            THROW(std::invalid_argument, "HouseChoiceInfoMenu::HouseChoiceInfoMenu(): Invalid house id '%d'.", static_cast<int>(newHouse));
        }
    }
    // clang-format on

    planetAnimation.setAnimation(anim);
    windowWidget.addWidget(&planetAnimation, Point(256, 96), planetAnimation.getMinimumSize());

    const auto* const pQuestionTexture = pGFXManager->getUIGraphic(UI_MentatHouseChoiceInfoQuestion, newHouse);
    questionLabel.setTexture(pQuestionTexture);
    windowWidget.addWidget(&questionLabel, Point(0, 0), getTextureSize(pQuestionTexture));
    questionLabel.setVisible(false);

    // init textbox but skip first line (this line contains "House ???")
    std::string desc    = pTextManager->getBriefingText(0, MISSION_DESCRIPTION, house);
    const int linebreak = desc.find("\n", 0) + 1;
    setText(desc.substr(linebreak, desc.length() - linebreak));

    const auto* const pMentatYes        = pGFXManager->getUIGraphic(UI_MentatYes);
    const auto* const pMentatYesPressed = pGFXManager->getUIGraphic(UI_MentatYes_Pressed);

    yesButton.setTextures(pMentatYes, pMentatYesPressed);
    yesButton.setEnabled(false);
    yesButton.setVisible(false);
    yesButton.setOnClick([this] { onYes(); });
    windowWidget.addWidget(&yesButton, Point(370, 340), getTextureSize(pMentatYes));

    const auto* const pMentatNo        = pGFXManager->getUIGraphic(UI_MentatNo);
    const auto* const pMentatNoPressed = pGFXManager->getUIGraphic(UI_MentatNo_Pressed);

    noButton.setTextures(pMentatNo, pMentatNoPressed);
    noButton.setEnabled(false);
    noButton.setVisible(false);
    noButton.setOnClick([&] { onNo(); });
    windowWidget.addWidget(&noButton, Point(480, 340), getTextureSize(pMentatNo));
}

HouseChoiceInfoMenu::~HouseChoiceInfoMenu() = default;

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
