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
    pAllocated_ = false;
    if (parent_ != nullptr) {
        parent_->removeChildWidget(this);
    }
}

void Widget::setActive() {
    active_ = true;
    if (parent_ != nullptr) {
        parent_->setActiveChildWidget(true, this);
    }
}

void Widget::setInactive() {
    active_ = false;
    if (parent_ != nullptr) {
        parent_->setActiveChildWidget(false, this);
    }
}

void Widget::resizeAll() {
    if (parent_ != nullptr) {
        parent_->resizeAll();
    } else {
        const auto minimum_size = getMinimumSize();
        const auto current_size = getSize();

        resize(std::max(minimum_size.x, current_size.x), std::max(minimum_size.y, current_size.y));
    }
}

void Widget::destroy() {
    if (pAllocated_ == true) {
        pAllocated_ = false;
        delete this;
    } else {
        if (parent_ != nullptr) {
            parent_->removeChildWidget(this);
        }
    }
}

void Widget::setActive(bool bActive) {
    const auto oldActive = active_;
    active_              = bActive;

    if (oldActive != bActive) {
        if (active_ && pOnGainFocus_) {
            pOnGainFocus_();
        } else if (!active_ && pOnLostFocus_) {
            pOnLostFocus_();
        }
    }
}
