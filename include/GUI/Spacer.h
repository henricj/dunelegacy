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

#ifndef SPACER_H
#define SPACER_H

#include "Widget.h"

/// A variable spacer class. This spacer fills in between widgets
class Spacer final : public Widget {
public:
    /// default constructor
    Spacer();

    /// destructor
    ~Spacer() override;

    /**
        Returns the minimum size of this spacer. That is (0,0).
        \return the minimum size of this widget
    */
    [[nodiscard]] Point getMinimumSize() const override;
};

/// A horizontal fixed-size spacer class
class HSpacer final : public Widget {
public:
    /// default constructor
    HSpacer();

    /**
        Constructor.
        \param width    Width of this spacer.
    */
    explicit HSpacer(int32_t width);

    /// destructor
    ~HSpacer() override;

    /**
        Returns the minimum size of this spacer. The returned size is (width,0).
        \return the minimum size of this spacer
    */
    [[nodiscard]] Point getMinimumSize() const override;

private:
    int32_t width_{}; ///< width of this spacer
};

/// A vertical fixed-size spacer class
class VSpacer final : public Widget {
public:
    /// default constructor
    VSpacer();

    /**
        Constructor.
        \param height   Height of this spacer.
    */
    explicit VSpacer(int32_t height);

    /// destructor
    ~VSpacer() override;

    /**
        Returns the minimum size of this spacer. The returned size is (0,height).
        \return the minimum size of this spacer
    */
    [[nodiscard]] Point getMinimumSize() const override;

private:
    int32_t height_{}; ///< height of this spacer
};

#endif // SPACER_H
