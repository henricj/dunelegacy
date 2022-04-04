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

Checkbox::Checkbox() {
    Checkbox::enableResizing(true, false);
    setToggleButton(true);
}

Checkbox::~Checkbox() {
    Checkbox::invalidateTextures();
}

void Checkbox::draw(Point position) {
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

void Checkbox::resize(uint32_t width, uint32_t height) {
    invalidateTextures();
    Widget::resize(width, height);
}

void Checkbox::updateTextures() {
    parent::updateTextures();

    if (!pUnpressedTexture) {
        invalidateTextures();

        auto& gui = GUIStyle::getInstance();

        setSurfaces(gui.createCheckboxSurface(getSize().x, getSize().y, text, false, false, textcolor, textshadowcolor),
                    gui.createCheckboxSurface(getSize().x, getSize().y, text, true, false, textcolor, textshadowcolor),
                    gui.createCheckboxSurface(getSize().x, getSize().y, text, false, true, textcolor, textshadowcolor));

        pCheckedActiveTexture = convertSurfaceToTexture(
            gui.createCheckboxSurface(getSize().x, getSize().y, text, true, true, textcolor, textshadowcolor));
    }
}

void Checkbox::invalidateTextures() {
    pCheckedActiveTexture.reset();

    parent::invalidateTextures();
}
