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
    RadioButton::enableResizing(true, false);
    setToggleButton(true);
}

RadioButton::~RadioButton() {
    RadioButton::invalidateTextures();

    unregisterFromRadioButtonManager();
}

void RadioButton::registerRadioButtonManager(RadioButtonManager* pNewRadioButtonManager) {
    if (pNewRadioButtonManager != pRadioButtonManager_) {
        unregisterFromRadioButtonManager();
        pRadioButtonManager_ = pNewRadioButtonManager;
        if (!pRadioButtonManager_->isRegistered(this)) {
            pRadioButtonManager_->registerRadioButton(this);
        }
    }
}

void RadioButton::unregisterFromRadioButtonManager() {
    if (pRadioButtonManager_ != nullptr) {
        RadioButtonManager* pOldRadioButtonManager = pRadioButtonManager_;
        pRadioButtonManager_                       = nullptr;
        if (pOldRadioButtonManager->isRegistered(this)) {
            pOldRadioButtonManager->unregisterRadioButton(this);
        }
    }
}

void RadioButton::setText(const std::string& text) {
    if (text == text_)
        return;
    this->text_ = text;
    resizeAll();
}

void RadioButton::setText(std::string_view text) {
    if (text == text_)
        return;
    setText(std::string{text});
}

void RadioButton::setTextColor(uint32_t textcolor, Uint32 textshadowcolor) {
    this->text_color_        = textcolor;
    this->text_shadow_color_ = textshadowcolor;
    invalidateTextures();
}

void RadioButton::setToggleState(bool bToggleState) {
    if (!bToggleState) {
        return;
    }

    if (bToggleState != getToggleState()) {
        if (pRadioButtonManager_ != nullptr) {
            pRadioButtonManager_->setChecked(this);
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
        if ((isActive() || bHover_) && pCheckedActiveTexture_) {
            tex = DuneTexture{pCheckedActiveTexture_.get()};
        } else {
            tex = *pPressedTexture_;
        }
    } else {
        if ((isActive() || bHover_) && pActiveTexture_) {
            tex = *pActiveTexture_;
        } else {
            tex = *pUnpressedTexture_;
        }
    }

    if (!tex) {
        return;
    }

    tex.draw(dune::globals::renderer.get(), position.x, position.y);
}

void RadioButton::resize(uint32_t width, uint32_t height) {
    invalidateTextures();
    Widget::resize(width, height);
}

void RadioButton::updateTextures() {
    parent::updateTextures();

    if (!pUnpressedTexture_) {
        invalidateTextures();

        const auto& gui      = GUIStyle::getInstance();
        auto* const renderer = dune::globals::renderer.get();

        const auto size = getSize();

        setTextures(gui.createRadioButtonSurface(size.x, size.y, text_, false, false, text_color_, text_shadow_color_)
                        .createTexture(renderer),
                    gui.createRadioButtonSurface(size.x, size.y, text_, true, false, text_color_, text_shadow_color_)
                        .createTexture(renderer),
                    gui.createRadioButtonSurface(size.x, size.y, text_, false, true, text_color_, text_shadow_color_)
                        .createTexture(renderer));

        pCheckedActiveTexture_ =
            gui.createRadioButtonSurface(size.x, size.y, text_, true, true, text_color_, text_shadow_color_)
                .createTexture(renderer);
    }
}

void RadioButton::invalidateTextures() {
    pCheckedActiveTexture_.reset();

    parent::invalidateTextures();
}
