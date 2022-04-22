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

#ifndef CENTERINGBOX_H
#define CENTERINGBOX_H

#include "Widget.h"
#include "Container.h"

class CenteringContainer_WidgetData {
public:
    CenteringContainer_WidgetData() : pWidget(nullptr) { }

    explicit CenteringContainer_WidgetData(Widget* pWidget) : pWidget(pWidget) { }

    Widget* pWidget;
    Point position;
    Point size;

    bool operator==(const CenteringContainer_WidgetData& op) const { return pWidget == op.pWidget; }
};

/// A container class for horizontal aligned widgets.
class CenteringBox final : public Container<CenteringContainer_WidgetData> {
    using parent = Widget;

public:
    /// default constructor
    CenteringBox() = default;

    /// destructor
    ~CenteringBox() override = default;

    /**
        This method sets the widget.
        \param newWidget    Widget to set
    */
    virtual void setWidget(Widget* newWidget) {
        if (newWidget != nullptr) {
            containedWidgets.push_back(CenteringContainer_WidgetData(newWidget));
            newWidget->setParent(this);
            Widget::resizeAll();
        }
    }

    /**
        This method sets the border widget.
        \param newWidget    Widget to set
    */
    virtual void setBorderWidget(Widget* newWidget) {
        if (newWidget != nullptr) {
            containedWidgets.push_front(CenteringContainer_WidgetData(newWidget));
            newWidget->setParent(this);
            Widget::resizeAll();
        }
    }

    /**
        Returns the minimum size of this container. The container should not
        resized to a size smaller than this. If the container is not resizeable
        in a direction this method returns the size in that direction.
        \return the minimum size of this container
    */
    Point getMinimumSize() const override {
        Point p(0, 0);

        for (const auto& widgetData : containedWidgets) {
            const auto& size = widgetData.size;

            if (size.x > p.x)
                p.x = size.x;
            if (size.y > p.y)
                p.y = size.y;
        }

        return p;
    }

    /**
        This method resizes the container. This method should only
        called if the new size is a valid size for this container (See getMinumumSize).
        \param  newSize the new size of this progress bar
    */
    void resize(Point newSize) override { resize(newSize.x, newSize.y); }

    /**
        This method resizes the widget to width and height. This method should only be
        called if the new size is a valid size for this container (See resizingXAllowed,
        resizingYAllowed, getMinimumSize). It also resizes all child widgets.
        \param  width   the new width of this container
        \param  height  the new height of this container
    */
    void resize(uint32_t width, uint32_t height) override {
        const auto width0  = width;
        const auto height0 = height;

        if (auto* border_widget = get_border_widget()) {
            border_widget->pWidget->resize(width, height);

            const auto actual = border_widget->pWidget->getSize();

            if (actual.x > width)
                width = actual.x;
            if (actual.y > height)
                height = actual.y;

            border_widget->size = actual;

            border_widget->position.x = actual.x >= width0 ? 0 : (width0 - actual.x) / 2;
            border_widget->position.y = actual.y >= height0 ? 0 : (height0 - actual.y) / 2;
        }

        if (auto* widget = get_widget()) {
            widget->pWidget->resize(width, height);

            const auto actual = widget->pWidget->getSize();

            widget->size = actual;

            widget->position.x = actual.x >= width0 ? 0 : (width0 - actual.x) / 2;
            widget->position.y = actual.y >= height0 ? 0 : (height0 - actual.y) / 2;
        }

        parent::resize(width0, height0);
    }

protected:
    /**
        This method must be overwritten by all container classes. It should return
        the position of the specified widget.
        \param widgetData   the widget data to get the position from.
        \return The position of the left upper corner
    */
    Point getPosition(const CenteringContainer_WidgetData& widgetData) const override { return widgetData.position; }

    void setWidgetGeometry(Widget* pWidget, Point position, Point size) {
        auto* pWidgetData = getWidgetDataFromWidget(pWidget);
        if (pWidgetData != nullptr) {
            pWidgetData->position = position;
            pWidgetData->size     = size;
            pWidget->resize(size.x, size.y);
        }
    }

    CenteringContainer_WidgetData* get_widget() {
        return containedWidgets.empty() ? nullptr : &containedWidgets.back();
    }
    CenteringContainer_WidgetData* get_border_widget() {
        return containedWidgets.empty() ? nullptr : &containedWidgets.front();
    }
};

#endif // CENTERINGBOX_H
