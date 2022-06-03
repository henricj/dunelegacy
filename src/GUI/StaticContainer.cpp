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

#include "GUI/StaticContainer.h"

StaticContainer::StaticContainer() = default;

StaticContainer::~StaticContainer() = default;

void StaticContainer::addWidget(Widget* newWidget, Point position, Point size) {
    if (newWidget == nullptr)
        return;

    containedWidgets.emplace_back(newWidget, position, size);
    newWidget->resize(size.x, size.y);
    newWidget->setParent(this);

    resizeAll();
}

void StaticContainer::addWidget(Widget* newWidget, const SDL_Rect& rect) {
    addWidget(newWidget, {rect.x, rect.y}, {rect.w, rect.h});
}

void StaticContainer::addWidget(Widget* newWidget, const SDL_FRect& rect) {
    addWidget(newWidget, {static_cast<int>(std::ceil(rect.x)), static_cast<int>(std::ceil(rect.y))},
              {static_cast<int>(std::ceil(rect.w)), static_cast<int>(std::ceil(rect.h))});
}

Point StaticContainer::getMinimumSize() const {
    Point p(0, 0);

    for (const auto& widgetData : containedWidgets) {
        p.x = std::max(p.x, widgetData.position.x + widgetData.size.x);
        p.y = std::max(p.y, widgetData.position.y + widgetData.size.y);
    }

    return p;
}

void StaticContainer::resize(uint32_t width, uint32_t height) {
    for (const auto& widgetData : containedWidgets)
        widgetData.pWidget->resize(widgetData.size.x, widgetData.size.y);

    parent::resize(width, height);
}

void StaticContainer::setWidgetGeometry(Widget* pWidget, Point position, Point size) {
    auto* const pWidgetData = getWidgetDataFromWidget(pWidget);
    if (pWidgetData == nullptr)
        return;

    pWidgetData->position = position;
    pWidgetData->size     = size;

    pWidget->resize(size.x, size.y);
}

void StaticContainer::setWidgetPosition(Widget* pWidget, Point position) {
    auto* const pWidgetData = getWidgetDataFromWidget(pWidget);
    if (pWidgetData == nullptr)
        return;

    pWidgetData->position = position;
}
