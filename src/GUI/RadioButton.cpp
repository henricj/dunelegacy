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

#include "GUI/RadioButton.h"

RadioButton::RadioButton() {
    Widget::enableResizing(true, false);
    setToggleButton(true);
}

RadioButton::~RadioButton() {
    RadioButton::invalidateTextures();

    unregisterFromRadioButtonManager();
}

void RadioButton::registerRadioButtonManager(RadioButtonManager* pNewRadioButtonManager) {
    if (pNewRadioButtonManager != pRadioButtonManager) {
        unregisterFromRadioButtonManager();
        pRadioButtonManager = pNewRadioButtonManager;
        if (!pRadioButtonManager->isRegistered(this)) {
            pRadioButtonManager->registerRadioButton(this);
        }
    }
}

void RadioButton::unregisterFromRadioButtonManager() {
    if (pRadioButtonManager != nullptr) {
        RadioButtonManager* pOldRadioButtonManager = pRadioButtonManager;
        pRadioButtonManager                        = nullptr;
        if (pOldRadioButtonManager->isRegistered(this)) {
            pOldRadioButtonManager->unregisterRadioButton(this);
        }
    }
}

void RadioButton::setTextColor(uint32_t textcolor, Uint32 textshadowcolor) {
    this->textcolor       = textcolor;
    this->textshadowcolor = textshadowcolor;
    invalidateTextures();
}

void RadioButton::setToggleState(bool bToggleState) {
    if (!bToggleState) {
        return;
    }

    if (bToggleState != getToggleState()) {
        if (pRadioButtonManager != nullptr) {
            pRadioButtonManager->setChecked(this);
        }
    }
}

void RadioButton::draw(Point position) {
    if (!isVisible()) {
        return;
    }

    updateTextures();

    DuneTexture tex;
    if (isChecked()) {
        if ((isActive() || bHover) && pCheckedActiveTexture) {
            tex = DuneTexture{pCheckedActiveTexture.get()};
        } else {
            tex = *pPressedTexture;
        }
    } else {
        if ((isActive() || bHover) && pActiveTexture) {
            tex = *pActiveTexture;
        } else {
            tex = *pUnpressedTexture;
        }
    }

    if (!tex) {
        return;
    }

    tex.draw(renderer, position.x, position.y);
}

void RadioButton::resize(uint32_t width, uint32_t height) {
    invalidateTextures();
    Widget::resize(width, height);
}

void RadioButton::updateTextures() {
    parent::updateTextures();

    if (!pUnpressedTexture) {
        invalidateTextures();

        const auto& gui = GUIStyle::getInstance();

        const auto size = getSize();

        setTextures(gui.createRadioButtonSurface(size.x, size.y, text, false, false, textcolor, textshadowcolor)
                        .createTexture(renderer),
                    gui.createRadioButtonSurface(size.x, size.y, text, true, false, textcolor, textshadowcolor)
                        .createTexture(renderer),
                    gui.createRadioButtonSurface(size.x, size.y, text, false, true, textcolor, textshadowcolor)
                        .createTexture(renderer));

        pCheckedActiveTexture =
            gui.createRadioButtonSurface(size.x, size.y, text, true, true, textcolor, textshadowcolor)
                .createTexture(renderer);
    }
}

void RadioButton::invalidateTextures() {
    pCheckedActiveTexture.reset();

    parent::invalidateTextures();
}
