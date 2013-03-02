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

#ifndef MENTATMENU_H
#define MENTATMENU_H

#include "MenuBase.h"
#include <GUI/StaticContainer.h>
#include <GUI/Button.h>
#include <GUI/dune/AnimationLabel.h>
#include <GUI/Label.h>

#include <misc/string_util.h>

#include <stdio.h>

#include <SDL.h>
#include <string>

class MentatMenu : public MenuBase {
public:
	MentatMenu(int newHouse);
	virtual ~MentatMenu();

	virtual void drawSpecificStuff();

	virtual void update();

    virtual bool doInput(SDL_Event &event) {
        if(event.type == SDL_MOUSEBUTTONDOWN) {
            showNextMentatText();
        }

        return MenuBase::doInput(event);
    }

	void setText(std::string text) {
	    mentatTexts = splitString(text, ". ", true);

        mouthAnim.getAnimation()->setNumLoops(mentatTexts[0].empty() ? 0 : mentatTexts[0].length()/25 + 1);
	    textLabel.setText(mentatTexts[0]);
        textLabel.setVisible(true);
        textLabel.resize(620,240);

	    currentMentatTextIndex = 0;
        nextMentatTextSwitch = SDL_GetTicks() + mentatTexts[0].length() * 75 + 1000;
	}

	void showNextMentatText() {
        nextMentatTextSwitch = 0;
	}

	virtual void onMentatTextFinished() { }

	int getMissionSpecificAnim(int missionnumber) const;

protected:
    Uint32  nextSpecialAnimation;

    std::vector<std::string> mentatTexts;
    int currentMentatTextIndex;
    Uint32 nextMentatTextSwitch;
	int house;

	StaticContainer	windowWidget;
	AnimationLabel	eyesAnim;
	AnimationLabel	mouthAnim;
	AnimationLabel	specialAnim;
	AnimationLabel	shoulderAnim;
	Label			textLabel;

};

#endif // MENTATMENU_H
