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

#ifndef STATICCONTAINER_H
#define STATICCONTAINER_H

#include "Container.h"

class StaticContainer_WidgetData {
public:
    StaticContainer_WidgetData() = default;

    StaticContainer_WidgetData(const StaticContainer_WidgetData&)            = default;
    StaticContainer_WidgetData(StaticContainer_WidgetData&&)                 = default;
    StaticContainer_WidgetData& operator=(const StaticContainer_WidgetData&) = default;
    StaticContainer_WidgetData& operator=(StaticContainer_WidgetData&&)      = default;

    StaticContainer_WidgetData(Widget* pWidget, Point position, Point size)
        : pWidget(pWidget), position(position), size(size) { }

    Widget* pWidget = nullptr;
    Point position{0, 0};
    Point size{0, 0};

    bool operator==(const StaticContainer_WidgetData& op) const = default;
};

/// A container class of explicit placed widgets
class StaticContainer : public Container<StaticContainer_WidgetData> {
    using parent = Container<StaticContainer_WidgetData>;

public:
    /// default constructor
    StaticContainer();

    /// default destructor
    ~StaticContainer() override;

    /**
        This method adds a new widget to this container.
        \param newWidget    Widget to add
        \param position     Position of the new Widget
        \param size         Size of the new widget
    */
    virtual void addWidget(Widget* newWidget, Point position, Point size);

    /**
        This method adds a new widget to this container.
        \param newWidget    Widget to add
        \param rect         Position and size of the new Widget
    */
    void addWidget(Widget* newWidget, const SDL_Rect& rect);

    /**
        This method adds a new widget to this container.
        \param newWidget    Widget to add
        \param rect         Position and size of the new Widget
    */
    void addWidget(Widget* newWidget, const SDL_FRect& rect);

    /**
        Returns the minimum size of this container. The container should not
        resized to a size smaller than this. If the container is not resizeable
        in a direction this method returns the size in that direction.
        \return the minimum size of this container
    */
    Point getMinimumSize() const override;

    /**
        This method resized the container to width and height. This method should only be
        called if the new size is a valid size for this container (See resizingXAllowed,
        resizingYAllowed, getMinimumSize). It also resizes all child widgets.
        \param  width   the new width of this container
        \param  height  the new height of this container
    */
    void resize(uint32_t width, uint32_t height) override;

    using parent::resize;

    /**
        This method must be overwritten by all container classes. It should return
        the position of the specified widget.
        \param widgetData   the widget data to get the position from.
        \return The position of the left upper corner
    */
    Point getPosition(const StaticContainer_WidgetData& widgetData) const override { return widgetData.position; }

    /**
        Update the given widget's position and size.
        \param pWidget      the widget (must already be in the container)
        \param position     the position
        \param size         the size
     */
    void setWidgetGeometry(Widget* pWidget, Point position, Point size);
};

#endif // STATICCONTAINER_H
