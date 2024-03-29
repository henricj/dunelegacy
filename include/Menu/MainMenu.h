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

#ifndef MAINMENU_H
#define MAINMENU_H

#include "MenuBase.h"

#include "Renderer/DuneTexture.h"
#include <GUI/PictureLabel.h>
#include <GUI/StaticContainer.h>
#include <GUI/TextButton.h>
#include <GUI/VBox.h>

class MainMenu final : public MainMenuBase {
    using parent = MainMenuBase;

public:
    MainMenu();
    ~MainMenu() override;

    MainMenu(const MainMenu&)            = delete;
    MainMenu(MainMenu&&)                 = delete;
    MainMenu& operator=(const MainMenu&) = delete;
    MainMenu& operator=(MainMenu&&)      = delete;

    /**
        This method resizes the window to width and height.
        \param  width   the new width of this widget
        \param  height  the new height of this widget
    */
    void resize(uint32_t width, uint32_t height) override;

    using parent::resize;

protected:
    int showMenuImpl() override;

private:
    void onSinglePlayer();
    void onMultiPlayer();
    void onMapEditor();
    void onOptions();
    void onAbout();
    void onQuit();

    StaticContainer windowWidget;
    VBox MenuButtons;

    TextButton singlePlayerButton;
    TextButton multiPlayerButton;
    TextButton mapEditorButton;
    TextButton optionsButton;
    TextButton aboutButton;
    TextButton quitButton;

    PictureLabel planetPicture;
    PictureLabel duneLegacy;
    PictureLabel buttonBorder;

    DuneTextureOwned mainBackgroundOwned;
    DuneTexture mainBackground;
};

#endif // MAINMENU_H
