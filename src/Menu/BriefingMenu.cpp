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

#include <Menu/BriefingMenu.h>

#include <globals.h>

#include <mmath.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/music/MusicPlayer.h>

BriefingMenu::BriefingMenu(HOUSETYPE newHouse,int mission,int type) : MentatMenu(newHouse) {
    this->mission = mission;
    this->type = type;

    const auto* const pMentatProceed = pGFXManager->getUIGraphic(UI_MentatProceed);
    const auto* const pMentatProceedPressed = pGFXManager->getUIGraphic(UI_MentatProceed_Pressed);
    proceedButton.setTextures(pMentatProceed, pMentatProceedPressed);
    proceedButton.setEnabled(false);
    proceedButton.setVisible(false);
    proceedButton.setOnClick([&] { onProceed(); });
    windowWidget.addWidget(&proceedButton, Point(350,340), getTextureSize(pMentatProceed));

    const auto* const pMentatRepeat = pGFXManager->getUIGraphic(UI_MentatRepeat);
    const auto* const pMentatRepeatPressed = pGFXManager->getUIGraphic(UI_MentatRepeat_Pressed);
    repeatButton.setTextures(pMentatRepeat, pMentatRepeatPressed);
    repeatButton.setEnabled(false);
    repeatButton.setVisible(false);
    repeatButton.setOnClick([&] { onRepeat(); });
    windowWidget.addWidget(&repeatButton,Point(500,340), getTextureSize(pMentatRepeat));

    int mission_number;
    if(mission != 22) {
        mission_number = ((mission+1)/3)+1;
    } else {
        mission_number = 9;
    }

    Animation* anim = nullptr;

    switch(type) {
        case DEBRIEFING_WIN: {
            anim = pGFXManager->getAnimation(getRandomBool() ? Anim_Win1 : Anim_Win2);
            text = pTextManager->getBriefingText(mission_number,MISSION_WIN,house);
        } break;
        case DEBRIEFING_LOST: {
            anim = pGFXManager->getAnimation(getRandomBool() ? Anim_Lose1 : Anim_Lose2);
            text = pTextManager->getBriefingText(mission_number,MISSION_LOSE,house);
        } break;
        default:
        case BRIEFING: {
            anim = pGFXManager->getAnimation(getMissionSpecificAnim(mission_number));
            text = pTextManager->getBriefingText(mission_number,MISSION_DESCRIPTION,house);
        } break;
    }
    setText(text);
    animation.setAnimation(anim);
    windowWidget.addWidget(&animation,Point(256,96),animation.getMinimumSize());
}

BriefingMenu::~BriefingMenu() = default;

void BriefingMenu::onMentatTextFinished() {
    proceedButton.setEnabled(true);
    proceedButton.setVisible(true);
    repeatButton.setEnabled(true);
    repeatButton.setVisible(true);
}

int BriefingMenu::showMenu()
{
    switch(type) {
        case DEBRIEFING_WIN:
        {
            switch(house) {
                case HOUSETYPE::HOUSE_HARKONNEN:
                case HOUSETYPE::HOUSE_SARDAUKAR: {
                    musicPlayer->changeMusic(MUSIC_WIN_H);
                } break;

                case HOUSETYPE::HOUSE_ATREIDES:
                case HOUSETYPE::HOUSE_FREMEN: {
                    musicPlayer->changeMusic(MUSIC_WIN_A);
                } break;

                case HOUSETYPE::HOUSE_ORDOS:
                case HOUSETYPE::HOUSE_MERCENARY: {
                    musicPlayer->changeMusic(MUSIC_WIN_O);
                } break;
            }
        } break;

        case DEBRIEFING_LOST:
        {
            switch(house) {
                case HOUSETYPE::HOUSE_HARKONNEN:
                case HOUSETYPE::HOUSE_SARDAUKAR: {
                    musicPlayer->changeMusic(MUSIC_LOSE_H);
                } break;

                case HOUSETYPE::HOUSE_ATREIDES:
                case HOUSETYPE::HOUSE_FREMEN: {
                    musicPlayer->changeMusic(MUSIC_LOSE_A);
                } break;

                case HOUSETYPE::HOUSE_ORDOS:
                case HOUSETYPE::HOUSE_MERCENARY: {
                    musicPlayer->changeMusic(MUSIC_LOSE_O);
                } break;
            }
        } break;

        case BRIEFING:
        {
            switch(house) {
                case HOUSETYPE::HOUSE_HARKONNEN:
                case HOUSETYPE::HOUSE_SARDAUKAR: {
                    musicPlayer->changeMusic(MUSIC_BRIEFING_H);
                } break;

                case HOUSETYPE::HOUSE_ATREIDES:
                case HOUSETYPE::HOUSE_FREMEN: {
                    musicPlayer->changeMusic(MUSIC_BRIEFING_A);
                } break;

                case HOUSETYPE::HOUSE_ORDOS:
                case HOUSETYPE::HOUSE_MERCENARY: {
                    musicPlayer->changeMusic(MUSIC_BRIEFING_O);
                } break;
            }
        } break;
    }

    return MentatMenu::showMenu();
}

void BriefingMenu::onRepeat() {
    setText(text);

    proceedButton.setEnabled(false);
    proceedButton.setVisible(false);
    repeatButton.setEnabled(false);
    repeatButton.setVisible(false);
}

void BriefingMenu::onProceed() {
    quit();
}
