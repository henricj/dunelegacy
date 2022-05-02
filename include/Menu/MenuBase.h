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

#include "misc/dune_clock.h"

#include <SDL2/SDL_events.h>

inline constexpr auto MENU_QUIT_DEFAULT = -1;

class MenuBase : public Window {
    using parent = Window;

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
    virtual bool isQuitting() { return quitting; }
    void disableQuitting(bool disable) { bAllowQuitting = !disable; }

    virtual void update() { }

    virtual void drawSpecificStuff();

    void draw() override;
    virtual bool doInput(SDL_Event& event);

    void setClearScreen(bool bClearScreen) { this->bClearScreen = bClearScreen; }

protected:
    bool doEventsUntil(dune::dune_clock::time_point until);

private:
    bool bClearScreen   = true;
    bool bAllowQuitting = true;
    bool quitting       = false;
    int retVal{MENU_QUIT_DEFAULT};
};

class DefaultWindowBase : public Window {
    using parent = Window;

protected:
    /**
        Constructor for creating a window
        \param  x   x position of this window
        \param  y   y position of this window
        \param  w   width of this window
        \param  h   height of this window
    */
    DefaultWindowBase(uint32_t x, uint32_t y, uint32_t w, uint32_t h);

public:
    ~DefaultWindowBase() override;

protected:
    void draw_background(Point position) override;
};

class TopMenuBase : public MenuBase {
    using parent = MenuBase;

protected:
    TopMenuBase();

public:
    ~TopMenuBase() override;
};

class MainMenuBase : public TopMenuBase {
    using parent = TopMenuBase;

protected:
    MainMenuBase();

public:
    ~MainMenuBase() override;

protected:
    void draw_background(Point position) override;
};

#endif // MENUBASE_H
