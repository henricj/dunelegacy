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

#include <Menu/MentatHelp.h>

#include <globals.h>

#include <sand.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>

#include <misc/draw_util.h>

#include <Game.h>

MentatHelp::MentatHelp(int newHouse, int techLevel, int mission) : MentatMenu(newHouse), mission(mission) {
    mentatEntries = pTextManager->getAllMentatEntries(newHouse, techLevel);

    if(mission == 0) {
        std::vector<MentatTextFile::MentatEntry>::iterator iter = mentatEntries.begin();
        while(iter != mentatEntries.end()) {
            if(iter->numMenuEntry == 0) {
                iter = mentatEntries.erase(iter);
            } else {
                ++iter;
            }
        }

    }

	setClearScreen(false);

	backgroundLabel.setTextColor(-1, -1, 116);
	windowWidget.addWidget(&backgroundLabel,Point(256,96),Point(368,224));

    std::vector<MentatTextFile::MentatEntry>::iterator iter;
    for(iter = mentatEntries.begin(); iter != mentatEntries.end(); ++iter) {
        if(iter->menuLevel == 0) {
            mentatTopicsList.addEntry("     " + iter->title + " :");
        } else {
            mentatTopicsList.addEntry("        " + iter->title);
        }
    }
    mentatTopicsList.setHighlightSelectedElement(false);
	mentatTopicsList.setOnSingleClick(std::bind(&MentatHelp::onListBoxClick, this));
	mentatTopicsList.setColor(houseColor[newHouse]+3);
    windowWidget.addWidget(&mentatTopicsList,Point(256+7, 96+7),Point(368 - 14, 224 - 14));

    windowWidget.addWidget(&animation,Point(256,96),Point(368,224));
    animation.setVisible(false);
    animation.setEnabled(false);
    itemDescriptionLabel.setTextFont(FONT_STD10);
    itemDescriptionLabel.setAlignment((Alignment_Enum) (Alignment_Left | Alignment_Top));
    itemDescriptionLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
    windowWidget.addWidget(&itemDescriptionLabel,Point(256 + 4, 96 + 4),Point(368 - 8, 224 - 8));
    itemDescriptionLabel.setVisible(false);

	SDL_Surface* surf = pGFXManager->getUIGraphic(UI_MentatExit);
	SDL_Surface* surfPressed = pGFXManager->getUIGraphic(UI_MentatExit_Pressed);
	exitButton.setSurfaces(surf,false,surfPressed,false);

	exitButton.setOnClick(std::bind(&MentatHelp::onExit, this));
	windowWidget.addWidget(&exitButton,Point(370,340),Point(surf->w,surf->h));
}

MentatHelp::~MentatHelp() {
}

void MentatHelp::drawSpecificStuff()
{
    MentatMenu::drawSpecificStuff();

    int x1 = getPosition().x;
    int x2 = getPosition().x + getSize().x - 1;
    int y1 = getPosition().y;
    int y2 = getPosition().y + getSize().y - 1;

	drawRect(screen, x1, y1, x2, y2, 229);
	drawHLine(screen, x1 + 1, y1 + 1, x2 - 1, 108);
	drawHLine(screen, x1 + 2, y1 + 2, x2 - 2, 108);
	drawHLine(screen, x1 + 3, y1 + 3, x2 - 3, 108);

	drawVLine(screen, x1 + 1, y1 + 1, y2 - 1, 108);
	drawVLine(screen, x1 + 2, y1 + 2, y2 - 2, 108);
	drawVLine(screen, x1 + 3, y1 + 3, y2 - 3, 108);

	drawHLine(screen, x1 + 1, y2 - 1, x2 - 1, 226);
	drawHLine(screen, x1 + 2, y2 - 2, x2 - 2, 226);
	drawHLine(screen, x1 + 3, y2 - 3, x2 - 3, 226);

	drawVLine(screen, x2 - 1, y1 + 1, y2 - 1, 226);
	drawVLine(screen, x2 - 2, y1 + 2, y2 - 2, 226);
	drawVLine(screen, x2 - 3, y1 + 3, y2 - 3, 226);
}

bool MentatHelp::doInput(SDL_Event &event)
{
    if((mentatTopicsList.isVisible() == false) && (event.type == SDL_MOUSEBUTTONDOWN)) {
        showNextMentatText();
        return true;
    } else {
        return MentatMenu::doInput(event);
    }
}

void MentatHelp::onMentatTextFinished() {
    animation.setVisible(false);
    animation.setEnabled(false);
    itemDescriptionLabel.setVisible(false);
    mentatTopicsList.setVisible(true);
    mentatTopicsList.setEnabled(true);
    exitButton.setVisible(true);
    exitButton.setEnabled(true);

    setText("");
    itemDescriptionLabel.setText("");
}

void MentatHelp::onExit() {
    if(mentatTopicsList.isVisible()) {
        currentGame->resumeGame();
        quit();
    } else {
        onMentatTextFinished();
    }
}

void MentatHelp::onListBoxClick() {
    int index = mentatTopicsList.getSelectedIndex();

    if(index < 0) {
        return;
    }

    MentatTextFile::MentatEntry& mentatEntry = mentatEntries[index];

    if(mentatEntry.menuLevel != 1) {
        return;
    }

    int animID;
    std::string text;
    std::string name;

	int missionnumber = ((mission+1)/3)+1;

    if(mentatEntry.filename == "0") {
        animID = getMissionSpecificAnim(missionnumber);
		text = pTextManager->getBriefingText(missionnumber, MISSION_DESCRIPTION, house);
    } else if(mentatEntry.filename == "3") {
        animID = getMissionSpecificAnim(missionnumber);
		text = pTextManager->getBriefingText(missionnumber, MISSION_ADVICE, house);
    } else {
        animID = getAnimByFilename(mentatEntry.filename);
        text = mentatEntry.content;
        name = mentatEntry.name;
    }

    animation.setAnimation(pGFXManager->getAnimation(animID));
    setText(text);
    itemDescriptionLabel.setText(name);

    animation.setVisible(true);
    animation.setEnabled(true);
    itemDescriptionLabel.setVisible(true);

    mentatTopicsList.setVisible(false);
    mentatTopicsList.setEnabled(false);
    exitButton.setVisible(false);
    exitButton.setEnabled(false);
}
