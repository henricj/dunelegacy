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

#include "GUI/VBox.h"

VBox::VBox() = default;

VBox::~VBox() = default;

void VBox::addWidget(Widget* newWidget, int32_t fixedHeight) {
    if (newWidget == nullptr)
        return;

    containedWidgets.push_back(VBox_WidgetData(newWidget, fixedHeight));
    newWidget->setParent(this);
    resizeAll();
}

void VBox::addWidget(Widget* newWidget, double weight) {
    if (newWidget != nullptr) {
        containedWidgets.push_back(VBox_WidgetData(newWidget, weight));
        newWidget->setParent(this);
        Widget::resizeAll();
    }
}

Point VBox::getMinimumSize() const {
    Point p(0, 0);
    for (const VBox_WidgetData& widgetData : containedWidgets) {
        p.x = std::max(p.x, widgetData.pWidget->getMinimumSize().x);
        if (widgetData.fixedHeight > 0) {
            p.y += widgetData.fixedHeight;
        } else {
            p.y += widgetData.pWidget->getMinimumSize().y;
        }
    }
    return p;
}

void VBox::resize(uint32_t width, uint32_t height) {
    int32_t availableHeight = height;

    int numRemainingWidgets = containedWidgets.size();

    // Find objects that are not allowed to be resized or have a fixed width
    // also find the sum of all weights
    double weightSum = 0.0;
    for (const VBox_WidgetData& widgetData : containedWidgets) {
        if (widgetData.pWidget->resizingYAllowed() == false) {
            availableHeight = availableHeight - widgetData.pWidget->getSize().y;
            numRemainingWidgets--;
        } else if (widgetData.fixedHeight > 0) {
            availableHeight = availableHeight - widgetData.fixedHeight;
            numRemainingWidgets--;
        } else {
            weightSum += widgetData.weight;
        }
    }

    // Under the resizeable widgets find all objects that are oversized (minimum size > AvailableHeight*weight)
    // also calculate the weight sum of all the resizeable widgets that are not oversized
    int32_t neededOversizeHeight = 0;
    double notOversizedWeightSum = 0.0;
    for (const VBox_WidgetData& widgetData : containedWidgets) {
        if (widgetData.pWidget->resizingYAllowed() == true && widgetData.fixedHeight <= 0) {
            if (static_cast<double>(widgetData.pWidget->getMinimumSize().y)
                > availableHeight * (widgetData.weight / weightSum)) {
                neededOversizeHeight += widgetData.pWidget->getMinimumSize().y;
            } else {
                notOversizedWeightSum += widgetData.weight;
            }
        }
    }

    const int32_t totalAvailableHeight = availableHeight;
    for (const VBox_WidgetData& widgetData : containedWidgets) {
        int32_t widgetWidth;
        if (widgetData.pWidget->resizingXAllowed() == true) {
            widgetWidth = width;
        } else {
            widgetWidth = widgetData.pWidget->getMinimumSize().x;
        }

        if (widgetData.pWidget->resizingYAllowed() == true) {
            int32_t WidgetHeight = 0;

            if (widgetData.fixedHeight <= 0) {
                if (numRemainingWidgets <= 1) {
                    WidgetHeight = availableHeight;
                } else if (static_cast<double>(widgetData.pWidget->getMinimumSize().y)
                           > totalAvailableHeight * (widgetData.weight / weightSum)) {
                    WidgetHeight = widgetData.pWidget->getMinimumSize().y;
                } else {
                    WidgetHeight = static_cast<int32_t>((totalAvailableHeight - neededOversizeHeight)
                                                        * (widgetData.weight / notOversizedWeightSum));
                }
                availableHeight -= WidgetHeight;
                numRemainingWidgets--;
            } else {
                WidgetHeight = widgetData.fixedHeight;
            }

            widgetData.pWidget->resize(widgetWidth, WidgetHeight);
        } else {
            widgetData.pWidget->resize(widgetWidth, widgetData.pWidget->getSize().y);
        }
    }

    Container<VBox_WidgetData>::resize(width, height);
}

VBox* VBox::create() {
    VBox* vbox = new VBox();

    vbox->pAllocated = true;
    return vbox;
}

Point VBox::getPosition(const VBox_WidgetData& widgetData) const {
    Point p(0, 0);
    for (const VBox_WidgetData& tmpWidgetData : containedWidgets) {
        if (widgetData.pWidget == tmpWidgetData.pWidget) {
            return p;
        }
        p.y = p.y + tmpWidgetData.pWidget->getSize().y;
    }

    // should not happen
    return {0, 0};
}
