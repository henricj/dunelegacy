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

#ifndef VBOX_H
#define VBOX_H

#include "Container.h"

class VBox_WidgetData {
public:
    VBox_WidgetData() = default;
    VBox_WidgetData(Widget* _pWidget, int32_t _fixedHeight) : pWidget(_pWidget), fixedHeight(_fixedHeight) { }
    VBox_WidgetData(Widget* _pWidget, double _weight) : pWidget(_pWidget), fixedHeight(-1), weight(_weight) { }

    VBox_WidgetData(const VBox_WidgetData&)            = default;
    VBox_WidgetData(VBox_WidgetData&&)                 = default;
    VBox_WidgetData& operator=(const VBox_WidgetData&) = default;
    VBox_WidgetData& operator=(VBox_WidgetData&&)      = default;

    Widget* pWidget     = nullptr;
    int32_t fixedHeight = 0;
    double weight       = 0.0;
};

/// A container class for vertical aligned widgets.
class VBox final : public Container<VBox_WidgetData> {
    using parent = Container<VBox_WidgetData>;

public:
    /// default constructor
    VBox();

    /// destructor
    ~VBox() override;

    /**
        This method adds a new widget to this container.
        \param newWidget    Widget to add
        \param fixedHeight  a fixed height for this widget (must be greater than the minimum size)
    */
    void addWidget(Widget* newWidget, int32_t fixedHeight);

    /**
        This method adds a new widget to this container.
        \param newWidget    Widget to add
        \param weight       The weight for this widget (default=1.0)
    */
    void addWidget(Widget* newWidget, double weight = 1.0);

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
        This static method creates a dynamic VBox object.
        The idea behind this method is to simply create a new VBox on the fly and
        add it to a container. If the container gets destroyed also this VBox will be freed.
        \return The new created VBox (will be automatically destroyed when it's parent widget is destroyed)
    */
    static VBox* create();

protected:
    /**
        This method must be overwritten by all container classes. It should return
        the position of the specified widget.
        \param widgetData   the widget data to get the position from.
        \return The position of the left upper corner
    */
    Point getPosition(const VBox_WidgetData& widgetData) const override;
};

#endif // VBOX_H
