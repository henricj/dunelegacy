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

#include <algorithm>

class VBox_WidgetData {
public:
    VBox_WidgetData() : pWidget(nullptr), fixedHeight(0), weight(0.0) { };
    VBox_WidgetData(Widget* _pWidget, Sint32 _fixedHeight) : pWidget(_pWidget), fixedHeight(_fixedHeight), weight(0.0) { };
    VBox_WidgetData(Widget* _pWidget, double _weight) : pWidget(_pWidget), fixedHeight(-1), weight(_weight) { };

    Widget* pWidget;
    Sint32 fixedHeight;
    double weight;
};

/// A container class for vertical aligned widgets.
class VBox : public Container<VBox_WidgetData> {
public:
    /// default constructor
    VBox() : Container<VBox_WidgetData>() {
        ;
    }

    /// destructor
    virtual ~VBox() {
        ;
    }

    /**
        This method adds a new widget to this container.
        \param newWidget    Widget to add
        \param fixedHeight  a fixed height for this widget (must be greater than the minimum size)
    */
    virtual void addWidget(Widget* newWidget, Sint32 fixedHeight) {
        if(newWidget != nullptr) {
            containedWidgets.push_back(VBox_WidgetData(newWidget, fixedHeight));
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
            containedWidgets.push_back(VBox_WidgetData(newWidget, weight));
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
        for(const VBox_WidgetData& widgetData : containedWidgets) {
            p.x = std::max(p.x,widgetData.pWidget->getMinimumSize().x);
            if(widgetData.fixedHeight > 0) {
                p.y += widgetData.fixedHeight;
            } else {
                p.y += widgetData.pWidget->getMinimumSize().y;
            }
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
        Sint32 availableHeight = height;

        int numRemainingWidgets = containedWidgets.size();

        // Find objects that are not allowed to be resized or have a fixed width
        // also find the sum of all weights
        double weightSum = 0.0;
        for(const VBox_WidgetData& widgetData : containedWidgets) {
            if(widgetData.pWidget->resizingYAllowed() == false) {
                availableHeight = availableHeight - widgetData.pWidget->getSize().y;
                numRemainingWidgets--;
            } else if(widgetData.fixedHeight > 0) {
                availableHeight = availableHeight - widgetData.fixedHeight;
                numRemainingWidgets--;
            } else {
                weightSum += widgetData.weight;
            }
        }

        // Under the resizeable widgets find all objects that are oversized (minimum size > AvailableHeight*weight)
        // also calculate the weight sum of all the resizeable widgets that are not oversized
        Sint32 neededOversizeHeight = 0;
        double notOversizedWeightSum = 0.0;
        for(const VBox_WidgetData& widgetData : containedWidgets) {
            if(widgetData.pWidget->resizingYAllowed() == true && widgetData.fixedHeight <= 0) {
                if((double) widgetData.pWidget->getMinimumSize().y > availableHeight * (widgetData.weight/weightSum)) {
                    neededOversizeHeight += widgetData.pWidget->getMinimumSize().y;
                } else {
                    notOversizedWeightSum += widgetData.weight;
                }
            }
        }

        Sint32 totalAvailableHeight = availableHeight;
        for(const VBox_WidgetData& widgetData : containedWidgets) {
            Sint32 widgetWidth;
            if(widgetData.pWidget->resizingXAllowed() == true) {
                widgetWidth = width;
            } else {
                widgetWidth = widgetData.pWidget->getMinimumSize().x;
            }

            if(widgetData.pWidget->resizingYAllowed() == true) {
                Sint32 WidgetHeight = 0;

                if(widgetData.fixedHeight <= 0) {
                    if(numRemainingWidgets <= 1) {
                        WidgetHeight = availableHeight;
                    } else if((double) widgetData.pWidget->getMinimumSize().y > totalAvailableHeight * (widgetData.weight/weightSum)) {
                        WidgetHeight = widgetData.pWidget->getMinimumSize().y;
                    } else {
                        WidgetHeight = (Sint32) ((totalAvailableHeight-neededOversizeHeight) * (widgetData.weight/notOversizedWeightSum));
                    }
                    availableHeight -= WidgetHeight;
                    numRemainingWidgets--;
                } else {
                    WidgetHeight = widgetData.fixedHeight;
                }

                widgetData.pWidget->resize(widgetWidth,WidgetHeight);
            } else {
                widgetData.pWidget->resize(widgetWidth, widgetData.pWidget->getSize().y);
            }
        }

        Container<VBox_WidgetData>::resize(width,height);
    }

    /**
        This static method creates a dynamic VBox object.
        The idea behind this method is to simply create a new VBox on the fly and
        add it to a container. If the container gets destroyed also this VBox will be freed.
        \return The new created VBox (will be automatically destroyed when it's parent widget is destroyed)
    */
    static VBox* create() {
        VBox* vbox = new VBox();
        vbox->pAllocated = true;
        return vbox;
    }

protected:
    /**
        This method must be overwritten by all container classes. It should return
        the position of the specified widget.
        \param widgetData   the widget data to get the position from.
        \return The position of the left upper corner
    */
    Point getPosition(const VBox_WidgetData& widgetData) const override
    {
        Point p(0,0);
        for(const VBox_WidgetData& tmpWidgetData : containedWidgets) {
            if(widgetData.pWidget == tmpWidgetData.pWidget) {
                return p;
            } else {
                p.y = p.y + tmpWidgetData.pWidget->getSize().y;
            }
        }

        //should not happen
        return Point(0,0);
    }
};

#endif // VBOX_H

