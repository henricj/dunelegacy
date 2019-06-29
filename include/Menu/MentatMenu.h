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
#include <misc/SDL2pp.h>

#include <stdio.h>
#include <string>

class MentatMenu : public MenuBase {
public:
    explicit MentatMenu(int newHouse);
    virtual ~MentatMenu();

    void setText(const std::string& text);

    void drawSpecificStuff() override;

    void update() override;

    bool doInput(SDL_Event &event) override
    {
        if(event.type == SDL_MOUSEBUTTONDOWN) {
            showNextMentatText();
        }

        return MenuBase::doInput(event);
    }

    void showNextMentatText() {
        nextMentatTextSwitch = 0;
    }

    virtual void onMentatTextFinished() { }

    int getMissionSpecificAnim(int missionnumber) const;

protected:
    enum MentatEyes {
        MentatEyesNormal = 0,
        MentatEyesLeft = 1,
        MentatEyesRight = 2,
        MentatEyesDown = 3,
        MentatEyesClosed = 4
    };


    enum MentatMouth {
        MentatMouthClosed = 0,
        MentatMouthOpen1 = 1,
        MentatMouthOpen2 = 2,
        MentatMouthOpen3 = 3,
        MentatMouthOpen4 = 4
    };

    Uint32  nextSpecialAnimation;

    std::vector<std::string> mentatTexts;
    int currentMentatTextIndex;
    Uint32 nextMentatTextSwitch;
    int house;

    StaticContainer windowWidget;
    AnimationLabel  eyesAnim;
    AnimationLabel  mouthAnim;
    AnimationLabel  specialAnim;
    AnimationLabel  shoulderAnim;
    Label           textLabel;

};

#endif // MENTATMENU_H
