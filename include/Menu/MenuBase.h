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

#ifndef MENUBASE_H
#define MENUBASE_H

#include <GUI/Window.h>
#include <misc/SDL2pp.h>

#define MENU_QUIT_DEFAULT   (-1)

class MenuBase: public Window
{
public:

    MenuBase();
    virtual ~MenuBase();

    MenuBase(const MenuBase &) = delete;
    MenuBase(MenuBase &&) = delete;
    MenuBase& operator=(const MenuBase &) = delete;
    MenuBase& operator=(MenuBase &&) = delete;

    virtual int showMenu();
    virtual void quit(int returnVal = MENU_QUIT_DEFAULT);
    virtual bool isQuiting() { return quiting; };
    void disableQuiting(bool disable) { bAllowQuiting = !disable; };

    virtual void update() { };

    virtual void drawSpecificStuff();

    void draw() override;
    virtual bool doInput(SDL_Event &event);

    void setClearScreen(bool bClearScreen) {
        this->bClearScreen = bClearScreen;
    };

private:
    bool bClearScreen;
    bool bAllowQuiting;
    bool quiting;
    int  retVal;
};

#endif // MENUBASE_H
