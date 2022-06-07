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

#include "GUI/QstBox.h"

void QstBox::setText(std::string text) {
    textLabel.setText(std::move(text));
    resize(std::max(vbox.getMinimumSize().x, 120), vbox.getMinimumSize().y);
}

void QstBox::setTextColor(uint32_t textcolor, Uint32 textshadowcolor) {
    textLabel.setTextColor(textcolor, textshadowcolor);
    button1.setTextColor(textcolor, textshadowcolor);
    button2.setTextColor(textcolor, textshadowcolor);
}

void QstBox::resize(uint32_t width, uint32_t height) {
    Window::resize(width, height);
    position_.x = (getRendererWidth() - getSize().x) / 2;
    position_.y = (getRendererHeight() - getSize().y) / 2;
}

void QstBox::resizeAll() {
    // QstBox should get bigger if content changes
    if (pWindowWidget_ != nullptr) {
        auto newSize = pWindowWidget_->getMinimumSize();
        newSize.x    = std::max(newSize.x, 120);
        newSize.y    = std::max(newSize.y, 30);
        resize(newSize.x, newSize.y);
    }
}

int QstBox::getPressedButtonID() const {
    return pressedButtonID;
}

QstBox* QstBox::create(std::string text, std::string button1Text, std::string button2Text, int defaultFocus) {
    auto* qstbox        = new QstBox(std::move(text), std::move(button1Text), std::move(button2Text), defaultFocus);
    qstbox->pAllocated_ = true;
    return qstbox;
}

QstBox::QstBox(std::string text, std::string button1Text, std::string button2Text, int defaultFocus)
    : Window(50, 50, 50, 50), pressedButtonID(QSTBOX_BUTTON_INVALID) {
    setWindowWidget(&vbox);
    vbox.addWidget(Widget::create<VSpacer>(6).release());
    vbox.addWidget(&textLabel);
    vbox.addWidget(Widget::create<VSpacer>(3).release());
    vbox.addWidget(&hbox);
    vbox.addWidget(Widget::create<VSpacer>(6).release());
    hbox.addWidget(Widget::create<Spacer>().release(), 0.2);
    hbox.addWidget(&vbox2, 0.6);
    vbox2.addWidget(Widget::create<VSpacer>(4).release());
    button1.setText(std::move(button1Text));
    button1.setOnClick([this] { onButton(QSTBOX_BUTTON1); });
    hbox2.addWidget(&button1);
    hbox2.addWidget(Widget::create<HSpacer>(6).release());
    button2.setText(std::move(button2Text));
    button2.setOnClick([this] { onButton(QSTBOX_BUTTON2); });
    hbox2.addWidget(&button2);
    vbox2.addWidget(&hbox2);
    vbox2.addWidget(Widget::create<VSpacer>(4).release());
    hbox.addWidget(Widget::create<Spacer>().release(), 0.2);
    setText(std::move(text));
    textLabel.setAlignment(Alignment_HCenter);
    if (defaultFocus == QSTBOX_BUTTON1) {
        button1.setActive();
    } else if (defaultFocus == QSTBOX_BUTTON2) {
        button2.setActive();
    }
}

QstBox::~QstBox() = default;

void QstBox::onButton(int btnID) {
    pressedButtonID = btnID;

    auto* pParentWindow = dynamic_cast<Window*>(getParent());
    if (pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}
