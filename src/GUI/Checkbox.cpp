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
    this->textcolor       = textcolor;
    this->textshadowcolor = textshadowcolor;
    invalidateTextures();
}

void Checkbox::draw(Point position) {
    if (!isVisible()) {
        return;
    }

    updateTextures();

    DuneTexture tex;
    if (isChecked()) {
        if ((isActive() || bHover) && pCheckedActiveTexture) {
            tex = pCheckedActiveTexture.as_dune_texture();
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

        setTextures(gui.createCheckboxSurface(size.x, size.y, text, false, false, textcolor, textshadowcolor)
                        .createTexture(renderer),
                    gui.createCheckboxSurface(size.x, size.y, text, true, false, textcolor, textshadowcolor)
                        .createTexture(renderer),
                    gui.createCheckboxSurface(size.x, size.y, text, false, true, textcolor, textshadowcolor)
                        .createTexture(renderer));

        pCheckedActiveTexture = gui.createCheckboxSurface(size.x, size.y, text, true, true, textcolor, textshadowcolor)
                                    .createTexture(renderer);
    }
}

void Checkbox::invalidateTextures() {
    pCheckedActiveTexture.reset();

    parent::invalidateTextures();
}
