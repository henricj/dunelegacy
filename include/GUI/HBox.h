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

#ifndef HBOX_H
#define HBOX_H

#include "Container.h"

#include <algorithm>

class HBox_WidgetData {
public:
    HBox_WidgetData() : pWidget(nullptr), fixedWidth(0), weight(0.0) { }
    HBox_WidgetData(Widget* _pWidget, int32_t _fixedWidth) : pWidget(_pWidget), fixedWidth(_fixedWidth), weight(0.0) { }
    HBox_WidgetData(Widget* _pWidget, double _weight) : pWidget(_pWidget), fixedWidth(-1), weight(_weight) { }

    Widget* pWidget;
    int32_t fixedWidth;
    double weight;
};

/// A container class for horizontal aligned widgets.
class HBox : public Container<HBox_WidgetData> {
    using parent = Container<HBox_WidgetData>;

public:
    /// default constructor
    HBox();

    /// destructor
    ~HBox() override;

    /**
        This method adds a new widget to this container.
        \param newWidget    Widget to add
        \param fixedWidth   a fixed width for this widget (must be greater than the minimum size)
    */
    virtual void addWidget(Widget* newWidget, int32_t fixedWidth);

    /**
        This method adds a new widget to this container.
        \param newWidget    Widget to add
        \param weight       The weight for this widget (default=1.0)
    */
    virtual void addWidget(Widget* newWidget, double weight = 1.0);

    /**
        Returns the minimum size of this container. The container should not
        resized to a size smaller than this. If the container is not resizeable
        in a direction this method returns the size in that direction.
        \return the minimum size of this container
    */
    Point getMinimumSize() const override;

    /**
        This method resizes the container to width and height. This method should only be
        called if the new size is a valid size for this container (See resizingXAllowed,
        resizingYAllowed, getMinimumSize). It also resizes all child widgets.
        \param  width   the new width of this container
        \param  height  the new height of this container
    */
    void resize(uint32_t width, uint32_t height) override;

    using parent::resize;

    /**
        This static method creates a dynamic HBox object.
        The idea behind this method is to simply create a new HBox on the fly and
        add it to a container. If the container gets destroyed also this HBox will be freed.
        \return The new created HBox (will be automatically destroyed when it's parent widget is destroyed)
    */
    static HBox* create();

protected:
    /**
        This method must be overwritten by all container classes. It should return
        the position of the specified widget.
        \param widgetData   the widget data to get the position from.
        \return The position of the left upper corner
    */
    Point getPosition(const HBox_WidgetData& widgetData) const override;
};

#endif // HBOX_H
