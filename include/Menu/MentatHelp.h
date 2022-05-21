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

#ifndef MENTATHELP_H
#define MENTATHELP_H

#include "MentatMenu.h"

#include <FileClasses/MentatTextFile.h>

#include <GUI/Label.h>
#include <GUI/ListBox.h>
#include <GUI/PictureButton.h>
#include <GUI/dune/AnimationLabel.h>

class MentatHelp final : public MentatMenu {
    using parent = MentatMenu;

public:
    MentatHelp(HOUSETYPE newHouse, int techLevel, int mission);
    ~MentatHelp() override;

    void drawSpecificStuff() override;

    void onMentatTextFinished() override;

protected:
    void doInputImpl(const SDL_Event& event) override;

private:
    void onExit();
    void onListBoxClick();

    int mission;
    std::vector<MentatTextFile::MentatEntry> mentatEntries;

    Label backgroundLabel;
    Label itemDescriptionLabel;
    PictureButton exitButton;
    AnimationLabel animation;
    ListBox mentatTopicsList;
};

#endif // MENTATHELP_H
