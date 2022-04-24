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

#include <SDL2/SDL_events.h>

#define MENU_QUIT_DEFAULT (-1)

class MenuBase : public Window {
protected:
    MenuBase();

public:
    ~MenuBase() override;

    MenuBase(const MenuBase&)            = delete;
    MenuBase(MenuBase&&)                 = delete;
    MenuBase& operator=(const MenuBase&) = delete;
    MenuBase& operator=(MenuBase&&)      = delete;

    virtual int showMenu();
    virtual void quit(int returnVal = MENU_QUIT_DEFAULT);
    virtual bool isQuiting() { return quiting; }
    void disableQuiting(bool disable) { bAllowQuiting = !disable; }

    virtual void update() { }

    virtual void drawSpecificStuff();

    void draw() override;
    virtual bool doInput(SDL_Event& event);

    void setClearScreen(bool bClearScreen) { this->bClearScreen = bClearScreen; }

protected:
    bool doEventsUntil(dune::dune_clock::time_point until);

private:
    bool bClearScreen  = true;
    bool bAllowQuiting = true;
    bool quiting       = false;
    int retVal{MENU_QUIT_DEFAULT};
};

class TopMenuBase : public MenuBase {
protected:
    TopMenuBase();

public:
    ~TopMenuBase() override;

protected:
    void draw_background(Point position) override;
};

#endif // MENUBASE_H
