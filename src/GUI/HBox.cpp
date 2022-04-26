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

#include "GUI/HBox.h"

HBox::HBox() = default;

HBox::~HBox() = default;

void HBox::addWidget(Widget* newWidget, int32_t fixedWidth) {
    if (newWidget == nullptr)
        return;

    containedWidgets.push_back(HBox_WidgetData(newWidget, fixedWidth));
    newWidget->setParent(this);
    resizeAll();
}

void HBox::addWidget(Widget* newWidget, double weight) {
    if (newWidget == nullptr)
        return;
    containedWidgets.push_back(HBox_WidgetData(newWidget, weight));
    newWidget->setParent(this);
    resizeAll();
}

Point HBox::getMinimumSize() const {
    Point p(0, 0);
    for (const HBox_WidgetData& widgetData : containedWidgets) {
        if (widgetData.fixedWidth > 0) {
            p.x += widgetData.fixedWidth;
        } else {
            p.x += widgetData.pWidget->getMinimumSize().x;
        }
        p.y = std::max(p.y, widgetData.pWidget->getMinimumSize().y);
    }
    return p;
}

void HBox::resize(uint32_t width, uint32_t height) {
    int32_t availableWidth = width;

    int numRemainingWidgets = containedWidgets.size();

    // Find objects that are not allowed to be resized or have a fixed width
    // also find the sum of all weights
    double weightSum = 0.0;
    for (const HBox_WidgetData& widgetData : containedWidgets) {
        if (widgetData.pWidget->resizingXAllowed() == false) {
            availableWidth = availableWidth - widgetData.pWidget->getSize().x;
            numRemainingWidgets--;
        } else if (widgetData.fixedWidth > 0) {
            availableWidth = availableWidth - widgetData.fixedWidth;
            numRemainingWidgets--;
        } else {
            weightSum += widgetData.weight;
        }
    }

    // Under the resizeable widgets find all objects that are oversized (minimum size > availableWidth*weight)
    // also calculate the weight sum of all the resizeable widgets that are not oversized
    int32_t neededOversizeWidth  = 0;
    double notOversizedWeightSum = 0.0;
    for (const HBox_WidgetData& widgetData : containedWidgets) {
        if (widgetData.pWidget->resizingXAllowed() == true && widgetData.fixedWidth <= 0) {
            if (static_cast<double>(widgetData.pWidget->getMinimumSize().x)
                > availableWidth * (widgetData.weight / weightSum)) {
                neededOversizeWidth += widgetData.pWidget->getMinimumSize().x;
            } else {
                notOversizedWeightSum += widgetData.weight;
            }
        }
    }

    const int32_t totalAvailableWidth = availableWidth;
    for (const HBox_WidgetData& widgetData : containedWidgets) {
        int32_t widgetHeight;
        if (widgetData.pWidget->resizingYAllowed() == true) {
            widgetHeight = height;
        } else {
            widgetHeight = widgetData.pWidget->getMinimumSize().y;
        }

        if (widgetData.pWidget->resizingXAllowed() == true) {
            int32_t widgetWidth = 0;

            if (widgetData.fixedWidth <= 0) {
                if (numRemainingWidgets <= 1) {
                    widgetWidth = availableWidth;
                } else if (static_cast<double>(widgetData.pWidget->getMinimumSize().x)
                           > totalAvailableWidth * (widgetData.weight / weightSum)) {
                    widgetWidth = widgetData.pWidget->getMinimumSize().x;
                } else {
                    widgetWidth = static_cast<int32_t>((totalAvailableWidth - neededOversizeWidth)
                                                       * (widgetData.weight / notOversizedWeightSum));
                }
                availableWidth -= widgetWidth;
                numRemainingWidgets--;
            } else {
                widgetWidth = widgetData.fixedWidth;
            }

            widgetData.pWidget->resize(widgetWidth, widgetHeight);
        } else {
            widgetData.pWidget->resize(widgetData.pWidget->getSize().x, widgetHeight);
        }
    }

    Container<HBox_WidgetData>::resize(width, height);
}

HBox* HBox::create() {
    HBox* hbox       = new HBox();
    hbox->pAllocated = true;
    return hbox;
}

Point HBox::getPosition(const HBox_WidgetData& widgetData) const {
    Point p(0, 0);
    for (const HBox_WidgetData& tmpWidgetData : containedWidgets) {
        if (widgetData.pWidget == tmpWidgetData.pWidget) {
            p.y = (getSize().y - tmpWidgetData.pWidget->getSize().y) / 2;
            return p;
        }
        p.x = p.x + tmpWidgetData.pWidget->getSize().x;
    }

    // should not happen
    return {0, 0};
}
