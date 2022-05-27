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

#include "GUI/Checkbox.h"

#include "globals.h"

Checkbox::Checkbox() {
    Checkbox::enableResizing(true, false);
    setToggleButton(true);
}

Checkbox::~Checkbox() {
    Checkbox::invalidateTextures();
}

void Checkbox::setTextColor(uint32_t textcolor, Uint32 textshadowcolor) {
    this->text_color_        = textcolor;
    this->text_shadow_color_ = textshadowcolor;
    invalidateTextures();
}

void Checkbox::draw(Point position) {
    if (!isVisible()) {
        return;
    }

    updateTextures();

    DuneTexture tex;
    if (isChecked()) {
        if ((isActive() || bHover) && pCheckedActiveTexture_) {
            tex = pCheckedActiveTexture_.as_dune_texture();
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

    if (!tex)
        return;

    tex.draw(dune::globals::renderer.get(), position.x, position.y);
}

void Checkbox::resize(uint32_t width, uint32_t height) {
    invalidateTextures();
    Widget::resize(width, height);
}

void Checkbox::updateTextures() {
    parent::updateTextures();

    if (!pUnpressedTexture) {
        invalidateTextures();

        const auto& gui      = GUIStyle::getInstance();
        auto* const renderer = dune::globals::renderer.get();

        const auto size = getSize();

        setTextures(gui.createCheckboxSurface(size.x, size.y, text_, false, false, text_color_, text_shadow_color_)
                        .createTexture(renderer),
                    gui.createCheckboxSurface(size.x, size.y, text_, true, false, text_color_, text_shadow_color_)
                        .createTexture(renderer),
                    gui.createCheckboxSurface(size.x, size.y, text_, false, true, text_color_, text_shadow_color_)
                        .createTexture(renderer));

        pCheckedActiveTexture_ =
            gui.createCheckboxSurface(size.x, size.y, text_, true, true, text_color_, text_shadow_color_)
                .createTexture(renderer);
    }
}

void Checkbox::invalidateTextures() {
    pCheckedActiveTexture_.reset();

    parent::invalidateTextures();
}
