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

#include "GUI/ProgressBar.h"

ProgressBar::ProgressBar() {
    ProgressBar::enableResizing(true, true);
}

ProgressBar::~ProgressBar() {
    ProgressBar::invalidateTextures();
}

void ProgressBar::draw(Point position) {
    if (isVisible() == false) {
        return;
    }

    updateTextures();

    parent::draw(position);

    if (!pForeground)
        return;

    auto* const renderer = dune::globals::renderer.get();

    const auto dest = calcDrawingRect(pForeground.get(), position.x, position.y);
    if (bDrawShadow) {
        const SDL_Rect dest2 = {position.x + 2, position.y + 2, static_cast<int>(lround(percent * (dest.w / 100.0))),
                                dest.h};
        renderFillRect(renderer, &dest2, COLOR_BLACK);
    }

    Dune_RenderCopy(renderer, pForeground.get(), nullptr, &dest);
}

void ProgressBar::updateTextures() {
    parent::updateTextures();

    if (!pForeground) {
        pForeground = convertSurfaceToTexture(
            GUIStyle::getInstance().createProgressBarOverlay(getSize().x, getSize().y, percent, color));
    }
}

void ProgressBar::invalidateTextures() {
    pForeground.reset();

    parent::invalidateTextures();
}

TextProgressBar::TextProgressBar()  = default;
TextProgressBar::~TextProgressBar() = default;

void TextProgressBar::setText(const std::string& text) {
    if (this->text != text) {
        this->text = text;
        resizeAll();
    }
}

void TextProgressBar::updateTextures() {
    parent::updateTextures();

    if (!getBackground()) {
        const auto& gui = GUIStyle::getInstance();

        setBackground(
            gui.createButtonSurface(getSize().x, getSize().y, text, true, false, textcolor, textshadowcolor).get());
    }
}

PictureProgressBar::PictureProgressBar() {
    parent::enableResizing(false, false);
}
PictureProgressBar::~PictureProgressBar() = default;

Point PictureProgressBar::getMinimumSize() const {
    if (const auto* const background = getBackground()) {
        return getTextureSize(background);
    }

    return {4, 4};
}
