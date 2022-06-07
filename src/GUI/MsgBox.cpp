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

#include "GUI/MsgBox.h"

#include "GUI/Spacer.h"

#include <algorithm>
#include <string>
#include <utility>

void MsgBox::setText(std::string text) {
    textLabel.setText(std::move(text));
    resize(std::max(vbox.getMinimumSize().x, 120), vbox.getMinimumSize().y);
}

void MsgBox::setTextColor(uint32_t textcolor, Uint32 textshadowcolor) {
    textLabel.setTextColor(textcolor, textshadowcolor);
    okbutton.setTextColor(textcolor, textshadowcolor);
}

void MsgBox::resize(uint32_t width, uint32_t height) {
    parent::resize(width, height);
    position_.x = (getRendererWidth() - getSize().x) / 2;
    position_.y = (getRendererHeight() - getSize().y) / 2;
}

void MsgBox::resizeAll() {
    // MsgBox should get bigger if content changes
    if (pWindowWidget_ != nullptr) {
        Point newSize = pWindowWidget_->getMinimumSize();
        newSize.x     = std::max(newSize.x, 120);
        newSize.y     = std::max(newSize.y, 30);
        resize(newSize.x, newSize.y);
    }
}

MsgBox* MsgBox::create(std::string text) {
    auto* msgbox        = new MsgBox(std::move(text));
    msgbox->pAllocated_ = true;
    return msgbox;
}

MsgBox::MsgBox(std::string text) : Window(50, 50, 50, 50) {
    using namespace std::literals;

    MsgBox::setWindowWidget(&vbox);
    vbox.addWidget(Widget::create<VSpacer>(6).release());
    vbox.addWidget(&textLabel);
    vbox.addWidget(Widget::create<VSpacer>(3).release());
    vbox.addWidget(&hbox);
    vbox.addWidget(Widget::create<VSpacer>(6).release());
    hbox.addWidget(Widget::create<Spacer>().release());
    hbox.addWidget(&vbox2);
    vbox2.addWidget(Widget::create<VSpacer>(4).release());
    okbutton.setText("OK"sv);
    okbutton.setOnClick([this] { onOK(); });
    vbox2.addWidget(&okbutton);
    vbox2.addWidget(Widget::create<VSpacer>(4).release());
    hbox.addWidget(Widget::create<Spacer>().release());
    setText(std::move(text));
    textLabel.setAlignment(Alignment_HCenter);
    okbutton.setActive();
}

MsgBox::~MsgBox() = default;

void MsgBox::onOK() {
    auto* pParentWindow = dynamic_cast<Window*>(getParent());
    if (pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}
