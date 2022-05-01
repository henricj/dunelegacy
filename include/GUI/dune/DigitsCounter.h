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

#ifndef DIGITSCOUNTER_H
#define DIGITSCOUNTER_H

#include <GUI/Widget.h>

#include <algorithm>

/// A widget for showing digits (like the credits in dune are shown)
class DigitsCounter final : public Widget {
    using parent = Widget;

public:
    /// default constructor
    DigitsCounter();

    /// destructor
    ~DigitsCounter() override;

    /**
        Get the current count of this digits counter
        \return the number that this digits counter currently shows
    */
    [[nodiscard]] unsigned int getCount() const { return count; }

    /**
        Set the count of this digits counter
        \param  newCount    the new number to show
    */
    void setCount(unsigned int newCount) { count = std::min(99u, newCount); }

    /**
        Draws this widget to screen. This method is called before drawOverlay().
        \param  position    Position to draw the widget to
    */
    void draw(Point position) override;

    /**
        Returns the minimum size of this digits counter. The widget should not
        be resized to a size smaller than this.
        \return the minimum size of this digits counter
    */
    [[nodiscard]] Point getMinimumSize() const override;

private:
    unsigned int count = 0;
};

#endif // DIGITSCOUNTER_H
