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

#ifndef CLICKMAP_H
#define CLICKMAP_H

#include "Widget.h"

#include <functional>

/// This widget reports the coordinate where the user clicked on
class ClickMap final : public Widget {
public:
    /// default constructor
    ClickMap() {
        enableResizing(true, true);
    }

    /// destructor
    ~ClickMap() override = default;

    /**
        Sets the function that should be called when this click map is clicked on.
        \param  pOnClick    A function to call when this map is clicked on
    */
    void setOnClick(std::function<void(int, int)> pOnClick) {
        this->pOnClick = pOnClick;
    }

    /**
        Handles a left mouse click.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the widget, false = click was not processed by the widget
    */
    bool handleMouseLeft(int32_t x, int32_t y, bool pressed) override {
        if ((x < 0) || (x >= getSize().x) || (y < 0) || (y >= getSize().y)) {
            return false;
        }

        if ((!isEnabled()) || (!isVisible())) {
            return true;
        }

        if (pressed && pOnClick) {
            pOnClick(x, y);
        }

        return true;
    }

private:
    std::function<void(int, int)> pOnClick; ///< function that is called when this click map is clicked
};

#endif // CLICKMAP_H
