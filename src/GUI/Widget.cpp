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

#include <GUI/Widget.h>

Widget::Widget() = default;

Widget::~Widget() {
    pAllocated = false;
    if (parent != nullptr) {
        parent->removeChildWidget(this);
    }
}

void Widget::setActive() {
    active = true;
    if (parent != nullptr) {
        parent->setActiveChildWidget(true, this);
    }
}

void Widget::setInactive() {
    active = false;
    if (parent != nullptr) {
        parent->setActiveChildWidget(false, this);
    }
}

void Widget::resizeAll() {
    if (parent != nullptr) {
        parent->resizeAll();
    } else {
        resize(std::max(getMinimumSize().x, getSize().x), std::max(getMinimumSize().y, getSize().y));
    }
}

void Widget::destroy() {
    if (pAllocated == true) {
        pAllocated = false;
        delete this;
    } else {
        if (parent != nullptr) {
            parent->removeChildWidget(this);
        }
    }
}

void Widget::setActive(bool bActive) {
    const auto oldActive = active;
    active               = bActive;

    if (oldActive != bActive) {
        if (active && pOnGainFocus) {
            pOnGainFocus();
        } else if (!active && pOnLostFocus) {
            pOnLostFocus();
        }
    }
}
