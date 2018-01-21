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

#ifndef INGAMESETTINGSMENU
#define INGAMESETTINGSMENU

#include <GUI/Window.h>
#include <GUI/StaticContainer.h>
#include <GUI/TextButton.h>
#include <GUI/PictureButton.h>
#include <GUI/ProgressBar.h>
#include "Definitions.h"

class InGameSettingsMenu : public Window
{
public:
    InGameSettingsMenu();
    virtual ~InGameSettingsMenu();

    void init();

    /**
        Handles a key stroke.
        \param  key the key that was pressed or released.
        \return true = key stroke was processed by the window, false = key stroke was not processed by the window
    */
    bool handleKeyPress(SDL_KeyboardEvent& key) override;

    /**
        This static method creates a dynamic settings menu object.
        The idea behind this method is to simply create a new dialog on the fly and
        add it as a child window of some other window. If the window gets closed it will be freed.
        \return The new dialog box (will be automatically destroyed when it's closed)
    */
    static InGameSettingsMenu* create() {
        InGameSettingsMenu* dlg = new InGameSettingsMenu();
        dlg->pAllocated = true;
        return dlg;
    }

private:
    void onOK();
    void onCancel();

    void onGameSpeedPlus();
    void onGameSpeedMinus();

    void onVolumePlus();
    void onVolumeMinus();

    void onScrollSpeedPlus();
    void onScrollSpeedMinus();

    StaticContainer windowWidget;

    TextButton      cancelButton;
    TextButton      okButton;

    PictureButton   gameSpeedPlus;
    PictureButton   gameSpeedMinus;
    ProgressBar     gameSpeedBar;

    PictureButton   volumePlus;
    PictureButton   volumeMinus;
    ProgressBar     volumeBar;

    PictureButton   scrollSpeedPlus;
    PictureButton   scrollSpeedMinus;
    ProgressBar     scrollSpeedBar;

    int     newGamespeed;
    int     previousVolume;
    int     volume;
    int     scrollSpeed;

};

#endif // INGAMESETTINGSMENU
