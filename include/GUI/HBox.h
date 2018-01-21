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
    HBox_WidgetData() : pWidget(nullptr), fixedWidth(0), weight(0.0) { };
    HBox_WidgetData(Widget* _pWidget, Sint32 _fixedWidth) : pWidget(_pWidget), fixedWidth(_fixedWidth), weight(0.0) { };
    HBox_WidgetData(Widget* _pWidget, double _weight) : pWidget(_pWidget), fixedWidth(-1), weight(_weight) { };

    Widget* pWidget;
    Sint32 fixedWidth;
    double weight;
};

/// A container class for horizontal aligned widgets.
class HBox : public Container<HBox_WidgetData> {
public:
    /// default constructor
    HBox() : Container<HBox_WidgetData>() {
        ;
    }

    /// destructor
    virtual ~HBox() {
        ;
    }

    /**
        This method adds a new widget to this container.
        \param newWidget    Widget to add
        \param fixedWidth   a fixed width for this widget (must be greater than the minimum size)
    */
    virtual void addWidget(Widget* newWidget, Sint32 fixedWidth) {
        if(newWidget != nullptr) {
            containedWidgets.push_back(HBox_WidgetData(newWidget, fixedWidth));
            newWidget->setParent(this);
            Widget::resizeAll();
        }
    }

    /**
        This method adds a new widget to this container.
        \param newWidget    Widget to add
        \param weight       The weight for this widget (default=1.0)
    */
    virtual void addWidget(Widget* newWidget, double weight = 1.0) {
        if(newWidget != nullptr) {
            containedWidgets.push_back(HBox_WidgetData(newWidget, weight));
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
    Point getMinimumSize() const override
    {
        Point p(0,0);
        for(const HBox_WidgetData& widgetData : containedWidgets) {
            if(widgetData.fixedWidth > 0) {
                p.x += widgetData.fixedWidth;
            } else {
                p.x += widgetData.pWidget->getMinimumSize().x;
            }
            p.y = std::max(p.y,widgetData.pWidget->getMinimumSize().y);
        }
        return p;
    }

    /**
        This method resizes the container. This method should only
        called if the new size is a valid size for this container (See getMinumumSize).
        \param  newSize the new size of this progress bar
    */
    void resize(Point newSize) override
    {
        resize(newSize.x,newSize.y);
    }

    /**
        This method resizes the container to width and height. This method should only be
        called if the new size is a valid size for this container (See resizingXAllowed,
        resizingYAllowed, getMinumumSize). It also resizes all child widgets.
        \param  width   the new width of this container
        \param  height  the new height of this container
    */
    void resize(Uint32 width, Uint32 height) override
    {
        Sint32 availableWidth = width;

        int numRemainingWidgets = containedWidgets.size();

        // Find objects that are not allowed to be resized or have a fixed width
        // also find the sum of all weights
        double weightSum = 0.0;
        for(const HBox_WidgetData& widgetData : containedWidgets) {
            if(widgetData.pWidget->resizingXAllowed() == false) {
                availableWidth = availableWidth - widgetData.pWidget->getSize().x;
                numRemainingWidgets--;
            } else if(widgetData.fixedWidth > 0) {
                availableWidth = availableWidth - widgetData.fixedWidth;
                numRemainingWidgets--;
            } else {
                weightSum += widgetData.weight;
            }
        }

        // Under the resizeable widgets find all objects that are oversized (minimum size > availableWidth*weight)
        // also calculate the weight sum of all the resizeable widgets that are not oversized
        Sint32 neededOversizeWidth = 0;
        double notOversizedWeightSum = 0.0;
        for(const HBox_WidgetData& widgetData : containedWidgets) {
            if(widgetData.pWidget->resizingXAllowed() == true && widgetData.fixedWidth <= 0) {
                if((double) widgetData.pWidget->getMinimumSize().x > availableWidth * (widgetData.weight/weightSum)) {
                    neededOversizeWidth += widgetData.pWidget->getMinimumSize().x;
                } else {
                    notOversizedWeightSum += widgetData.weight;
                }
            }
        }

        Sint32 totalAvailableWidth = availableWidth;
        for(const HBox_WidgetData& widgetData : containedWidgets) {
            Sint32 widgetHeight;
            if(widgetData.pWidget->resizingYAllowed() == true) {
                widgetHeight = height;
            } else {
                widgetHeight = widgetData.pWidget->getMinimumSize().y;
            }

            if(widgetData.pWidget->resizingXAllowed() == true) {
                Sint32 widgetWidth = 0;

                if(widgetData.fixedWidth <= 0) {
                    if(numRemainingWidgets <= 1) {
                        widgetWidth = availableWidth;
                    } else if((double) widgetData.pWidget->getMinimumSize().x > totalAvailableWidth * (widgetData.weight/weightSum)) {
                        widgetWidth = widgetData.pWidget->getMinimumSize().x;
                    } else {
                        widgetWidth = (Sint32) ((totalAvailableWidth-neededOversizeWidth) * (widgetData.weight/notOversizedWeightSum));
                    }
                    availableWidth -= widgetWidth;
                    numRemainingWidgets--;
                } else {
                    widgetWidth = widgetData.fixedWidth;
                }

                widgetData.pWidget->resize(widgetWidth,widgetHeight);
            } else {
                widgetData.pWidget->resize(widgetData.pWidget->getSize().x,widgetHeight);
            }
        }

        Container<HBox_WidgetData>::resize(width,height);
    }

    /**
        This static method creates a dynamic HBox object.
        The idea behind this method is to simply create a new HBox on the fly and
        add it to a container. If the container gets destroyed also this HBox will be freed.
        \return The new created HBox (will be automatically destroyed when it's parent widget is destroyed)
    */
    static HBox* create() {
        HBox* hbox = new HBox();
        hbox->pAllocated = true;
        return hbox;
    }

protected:
    /**
        This method must be overwritten by all container classes. It should return
        the position of the specified widget.
        \param widgetData   the widget data to get the position from.
        \return The position of the left upper corner
    */
    Point getPosition(const HBox_WidgetData& widgetData) const override
    {
        Point p(0,0);
        for(const HBox_WidgetData& tmpWidgetData : containedWidgets) {
            if(widgetData.pWidget == tmpWidgetData.pWidget) {
                p.y = (getSize().y - tmpWidgetData.pWidget->getSize().y)/2;
                return p;
            } else {
                p.x = p.x + tmpWidgetData.pWidget->getSize().x;
            }
        }

        //should not happen
        return Point(0,0);
    }
};

#endif // HBOX_H

