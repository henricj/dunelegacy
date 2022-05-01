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

#include "GUI/GUIStyle.h"
#include "misc/DrawingRectHelper.h"

#include <SDL2/SDL.h>

#include <string>
#include <utility>

ProgressBar::ProgressBar() {
    ProgressBar::enableResizing(true, true);
}

ProgressBar::~ProgressBar() {
    ProgressBar::invalidateTextures();
}

void ProgressBar::setProgress(float newPercent) {
    if (percent == newPercent)
        return;

    percent = newPercent;
    if (percent < 0.f)
        percent = 0.f;
    else if (percent > 100.f)
        percent = 100.f;
}

void ProgressBar::draw(Point position) {
    if (isVisible() == false) {
        return;
    }

    updateTextures();

    parent::draw(position);

    const auto size = getSize();

    auto* const renderer = dune::globals::renderer.get();
    auto& gui            = GUIStyle::getInstance();

    if (bDrawShadow) {
        const SDL_FRect dest2{static_cast<float>(position.x + 2), static_cast<float>(position.y + 2),
                              percent * 0.01f * static_cast<float>(size.x), static_cast<float>(size.y)};
        renderFillRectF(renderer, &dest2, COLOR_BLACK);
    }

    const auto render_button = [&](const DuneTexture* foreground) {
        gui.RenderButton(renderer,
                         {static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(size.x),
                          static_cast<float>(size.y)},
                         foreground, true);
    };

    if (std::holds_alternative<DuneTextureOwned>(pContent)) {
        const auto foreground = std::get<DuneTextureOwned>(pContent).as_dune_texture();

        render_button(foreground ? &foreground : nullptr);
    } else if (std::holds_alternative<const DuneTexture*>(pContent)) {
        const auto* foreground = std::get<const DuneTexture*>(pContent);

        foreground->draw(renderer, position.x, position.y);

    } else
        render_button(nullptr);

    if (color == COLOR_DEFAULT) {
        // default color

        auto const full_width = static_cast<float>(size.x) - 4.f;

        auto width = percent * 0.01f * full_width;

        if (width < 0)
            width = 0;
        else if (width > full_width)
            width = full_width;

        const SDL_FRect dest{static_cast<float>(position.x) + 2, static_cast<float>(position.y) + 2, width,
                             static_cast<float>(size.y) - 4};

        setRenderDrawColor(renderer, COLOR_HALF_TRANSPARENT);
        SDL_RenderFillRectF(renderer, &dest);
    } else {
        const auto full_width = static_cast<float>(size.x);

        auto width = percent * 0.01f * full_width;

        if (width < 0)
            width = 0;
        else if (width > full_width)
            width = full_width;

        const SDL_FRect dest = {static_cast<float>(position.x), static_cast<float>(position.y), width,
                                static_cast<float>(size.y)};
        setRenderDrawColor(renderer, color);
        SDL_RenderFillRectF(renderer, &dest);
    }
}

TextProgressBar::TextProgressBar()  = default;
TextProgressBar::~TextProgressBar() = default;

void TextProgressBar::setText(std::string text) {
    if (this->text != text) {
        this->text = std::move(text);
        resizeAll();
    }
}

Point TextProgressBar::getMinimumSize() const {
    if (text.empty()) {
        return {4, 4};
    }
    return GUIStyle::getInstance().getMinimumButtonSize(text);
}

void TextProgressBar::updateTextures() {
    parent::updateTextures();

    if (std::holds_alternative<DuneTextureOwned>(pContent))
        return;

    auto* const renderer = dune::globals::renderer.get();
    const auto& gui      = GUIStyle::getInstance();

    const auto size = getSize();

    const auto surface = gui.createButtonText(size.x, size.y, text, false, textcolor, textshadowcolor);

    pContent = surface.createTexture(renderer);
}

void TextProgressBar::invalidateTextures() {
    parent::invalidateTextures();

    pContent = std::monostate{};
}

PictureProgressBar::PictureProgressBar() {
    parent::enableResizing(false, false);
}
PictureProgressBar::~PictureProgressBar() = default;

void PictureProgressBar::setTexture(const DuneTexture* content) {
    if (content && *content)
        pContent = content;
    else
        pContent = std::monostate{};

    resize(getMinimumSize());

    resizeAll();
}

Point PictureProgressBar::getMinimumSize() const {
    if (std::holds_alternative<const DuneTexture*>(pContent))
        return (getTextureSize(std::get<const DuneTexture*>(pContent)));

    return {4, 4};
}
