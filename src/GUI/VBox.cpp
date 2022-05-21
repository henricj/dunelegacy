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

    containedWidgets.emplace_back(newWidget, fixedHeight);
    newWidget->setParent(this);
    resizeAll();
}

void VBox::addWidget(Widget* newWidget, double weight) {
    if (newWidget != nullptr) {
        containedWidgets.emplace_back(newWidget, weight);
        newWidget->setParent(this);

        resizeAll();
    }
}

Point VBox::getMinimumSize() const {
    Point p(0, 0);

    for (const auto& widgetData : containedWidgets) {
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
    auto availableHeight = static_cast<int>(height);

    auto numRemainingWidgets = static_cast<int>(containedWidgets.size());

    // Find objects that are not allowed to be resized or have a fixed width
    // also find the sum of all weights
    auto weightSum = 0.0;
    for (const auto& widgetData : containedWidgets) {
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
    auto neededOversizeHeight  = 0;
    auto notOversizedWeightSum = 0.0;
    for (const auto& widgetData : containedWidgets) {
        if (widgetData.pWidget->resizingYAllowed() != true || widgetData.fixedHeight > 0)
            continue;

        if (static_cast<double>(widgetData.pWidget->getMinimumSize().y)
            > availableHeight * (widgetData.weight / weightSum)) {
            neededOversizeHeight += widgetData.pWidget->getMinimumSize().y;
        } else {
            notOversizedWeightSum += widgetData.weight;
        }
    }

    const auto totalAvailableHeight = availableHeight;
    for (const auto& widgetData : containedWidgets) {
        auto* const widget = widgetData.pWidget;

        assert(widget);

        const auto widgetWidth = widget->resizingXAllowed() ? width : widget->getMinimumSize().x;

        if (widget->resizingYAllowed() == true) {
            auto WidgetHeight = 0;

            if (widgetData.fixedHeight <= 0) {
                if (numRemainingWidgets <= 1) {
                    WidgetHeight = availableHeight;
                } else if (static_cast<double>(widget->getMinimumSize().y)
                           > totalAvailableHeight * (widgetData.weight / weightSum)) {
                    WidgetHeight = widget->getMinimumSize().y;
                } else {
                    const auto new_height =
                        (totalAvailableHeight - neededOversizeHeight) * (widgetData.weight / notOversizedWeightSum);

                    WidgetHeight = static_cast<int32_t>(std::round(new_height));
                }
                availableHeight -= WidgetHeight;
                numRemainingWidgets--;
            } else {
                WidgetHeight = widgetData.fixedHeight;
            }

            widget->resize(widgetWidth, WidgetHeight);
        } else {
            widget->resize(widgetWidth, widget->getSize().y);
        }
    }

    parent::resize(width, height);
}

VBox* VBox::create() {
    VBox* vbox = new VBox();

    vbox->pAllocated_ = true;
    return vbox;
}

Point VBox::getPosition(const VBox_WidgetData& widgetData) const {
    Point p(0, 0);

    for (const auto& tmpWidgetData : containedWidgets) {
        if (widgetData.pWidget == tmpWidgetData.pWidget)
            return p;

        p.y = p.y + tmpWidgetData.pWidget->getSize().y;
    }

    // should not happen
    return {0, 0};
}
