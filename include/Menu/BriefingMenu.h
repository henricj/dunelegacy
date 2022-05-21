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

#ifndef BRIEFINGMENU_H
#define BRIEFINGMENU_H

#include "MentatMenu.h"
#include <GUI/PictureButton.h>
#include <GUI/dune/AnimationLabel.h>

inline constexpr auto BRIEFING        = 0;
inline constexpr auto DEBRIEFING_WIN  = 1;
inline constexpr auto DEBRIEFING_LOST = 2;

class BriefingMenu final : public MentatMenu {
    using parent = MentatMenu;

public:
    BriefingMenu(HOUSETYPE newHouse, int mission, int type);
    ~BriefingMenu() override;

    void onMentatTextFinished() override;

protected:
    int showMenuImpl() override;

private:
    std::string text;

    void onRepeat();
    void onProceed();
    int mission;
    int type;
    PictureButton proceedButton;
    PictureButton repeatButton;
    AnimationLabel animation;
};

#endif // BRIEFINGMENU_H
