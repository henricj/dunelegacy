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

#include <iostream>

/// A variable spacer class. This spacer fills in between widgets
class Spacer final : public Widget {
public:
    /// default constructor
    Spacer() : Widget() { Spacer::enableResizing(true, true); }

    /// destructor
    ~Spacer() override = default;

    /**
        Returns the minimum size of this spacer. That is (0,0).
        \return the minimum size of this widget
    */
    [[nodiscard]] Point getMinimumSize() const override { return {0, 0}; }

    /**
        This static method creates a dynamic spacer object.
        The idea behind this method is to simply create a new spacer on the fly and
        add it to a container. If the container gets destroyed also this spacer will be freed.
        \return The new created spacer (will be automatically destroyed when it's parent widget is destroyed)
    */
    static Spacer* create() {
        Spacer* sp     = new Spacer();
        sp->pAllocated = true;
        return sp;
    }
};

/// A horizontal fixed-size spacer class
class HSpacer final : public Widget {
public:
    /// default constructor
    HSpacer() : Widget(), width{} {
        HSpacer::resize(0, 0);
        HSpacer::enableResizing(false, false);
    }

    /**
        Constructor.
        \param width    Width of this spacer.
    */
    explicit HSpacer(uint32_t width) : Widget(), width{width} {
        HSpacer::resize(width, 0);
        HSpacer::enableResizing(false, false);
    }

    /// destructor
    ~HSpacer() override = default;

    /**
        Returns the minimum size of this spacer. The returned size is (width,0).
        \return the minimum size of this spacer
    */
    [[nodiscard]] Point getMinimumSize() const override { return Point(width, 0); }

    /**
        This static method creates a dynamic horizontal spacer object.
        The idea behind this method is to simply create a new spacer on the fly and
        add it to a container. If the container gets destroyed also this spacer will be freed.
        \param width width of this spacer
        \return The new created spacer (will be automatically destroyed when it's parent widget is destroyed)
    */
    static HSpacer* create(uint32_t width) {
        HSpacer* sp    = new HSpacer(width);
        sp->pAllocated = true;
        return sp;
    }

private:
    uint32_t width; ///< width of this spacer
};

/// A vertical fixed-size spacer class
class VSpacer final : public Widget {
public:
    /// default constructor
    VSpacer() : Widget(), height{} {
        VSpacer::resize(0, 0);
        VSpacer::enableResizing(false, false);
    }

    /**
        Constructor.
        \param height   Height of this spacer.
    */
    explicit VSpacer(uint32_t height) : Widget(), height{height} {
        VSpacer::resize(0, height);
        VSpacer::enableResizing(false, false);
    }

    /// destructor
    ~VSpacer() override = default;

    /**
        Returns the minimum size of this spacer. The returned size is (0,height).
        \return the minimum size of this spacer
    */
    [[nodiscard]] Point getMinimumSize() const override { return Point(0, height); }

    /**
        This static method creates a dynamic vertical spacer object.
        The idea behind this method is to simply create a new spacer on the fly and
        add it to a container. If the container gets destroyed also this spacer will be freed.
        \param height height of this spacer
        \return The new created spacer (will be automatically destroyed when it's parent widget is destroyed)
    */
    static VSpacer* create(uint32_t height) {
        VSpacer* sp    = new VSpacer(height);
        sp->pAllocated = true;
        return sp;
    }

private:
    uint32_t height; ///< height of this spacer
};

#endif // SPACER_H
