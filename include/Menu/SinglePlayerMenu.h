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

#ifndef SINGLEPLAYERMENU_H
#define SINGLEPLAYERMENU_H

#include "MenuBase.h"
#include <GUI/PictureLabel.h>
#include <GUI/Spacer.h>
#include <GUI/StaticContainer.h>
#include <GUI/TextButton.h>
#include <GUI/VBox.h>

class SinglePlayerMenu final : public MainMenuBase {
    using parent = MainMenuBase;

public:
    SinglePlayerMenu();
    ~SinglePlayerMenu() override;

    /**
        This method is called, when the child window is about to be closed.
        This child window will be closed after this method returns.
        \param  pChildWindow    The child window that will be closed
    */
    void onChildWindowClose(Window* pChildWindow) override;

    /**
        This method resizes the window to width and height.
        \param  width   the new width of this widget
        \param  height  the new height of this widget
    */
    void resize(uint32_t width, uint32_t height) override;

    using parent::resize;

private:
    void onCampaign();
    void onCustom();
    void onSkirmish();
    void onLoadSavegame();
    void onLoadReplay();
    void onCancel();

    StaticContainer windowWidget;
    VBox menuButtonsVBox;

    TextButton campaignButton;
    TextButton customButton;
    TextButton skirmishButton;
    TextButton loadSavegameButton;
    TextButton loadReplayButton;
    TextButton cancelButton;

    PictureLabel planetPicture;
    PictureLabel duneLegacy;
    PictureLabel buttonBorder;
};

#endif // SINGLEPLAYERMENU_H
