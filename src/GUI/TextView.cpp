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

#include "GUI/TextView.h"

#include "misc/DrawingRectHelper.h"
#include "misc/string_util.h"

#include <algorithm>

TextView::TextView() {
    Widget::enableResizing(true, true);

    resize(TextView::getMinimumSize().x, TextView::getMinimumSize().y);
}

TextView::~TextView() = default;

void TextView::handleMouseMovement(int32_t x, int32_t y, bool insideOverlay) {
    scrollbar_.handleMouseMovement(x - getSize().x + scrollbar_.getSize().x, y, insideOverlay);
}

bool TextView::handleMouseLeft(int32_t x, int32_t y, bool pressed) {
    return scrollbar_.handleMouseLeft(x - getSize().x + scrollbar_.getSize().x, y, pressed);
}

bool TextView::handleMouseWheel(int32_t x, int32_t y, bool up) {
    // forward mouse wheel event to scrollbar
    return scrollbar_.handleMouseWheel(0, 0, up);
}

bool TextView::handleKeyPress(const SDL_KeyboardEvent& key) {
    Widget::handleKeyPress(key);

    scrollbar_.handleKeyPress(key);
    return true;
}

void TextView::draw(Point position) {
    if (!isVisible()) {
        return;
    }

    updateTextures();

    auto* const renderer = dune::globals::renderer.get();

    if (pBackground_)
        pBackground_.draw(renderer, position.x, position.y);

    if (pForeground_) {
        const auto& gui = GUIStyle::getInstance();

        const auto lineHeight = gui.getTextHeight(fontSize_) + 2.f;

        const auto src_y = static_cast<int>(std::round(static_cast<float>(scrollbar_.getCurrentValue()) * lineHeight));

        const auto height =
            std::min(getHeight(pForeground_.get()),
                     static_cast<int>(std::round(static_cast<int>(getSize().y) - 2 * gui.getActualScale())));

        const SDL_Rect src{0, src_y, getWidth(pForeground_.get()), height};

        const SDL_FRect dest = {static_cast<float>(position.x) + 2.f, static_cast<float>(position.y) + 1.f,
                                pForeground_.width_,
                                std::min(pForeground_.height_, static_cast<float>(getSize().y) - 2.f)};

        Dune_RenderCopyF(renderer, pForeground_.get(), &src, &dest);
    }

    Point scrollBarPos = position;
    scrollBarPos.x += getSize().x - scrollbar_.getSize().x;

    if (!bAutohideScrollbar_ || (scrollbar_.getRangeMin() != scrollbar_.getRangeMax())) {
        scrollbar_.draw(scrollBarPos);
    }
}

void TextView::resize(uint32_t width, uint32_t height) {
    invalidateTextures();

    scrollbar_.resize(scrollbar_.getMinimumSize().x, height);

    auto& gui = GUIStyle::getInstance();

    const std::vector<std::string> textLines =
        greedyWordWrap(text_, static_cast<float>(getSize().x - scrollbar_.getSize().x - 4),
                       [&gui, font = fontSize_](std::string_view tmp) {
                           return static_cast<float>(gui.getMinimumLabelSize(tmp, font).x - 4);
                       });

    const auto lineHeight = gui.getTextHeight(fontSize_) + 2.f;

    const auto numVisibleLines = static_cast<int>(std::floor(static_cast<int>(height) / lineHeight));
    scrollbar_.setRange(0, std::max(0, static_cast<int>(textLines.size()) - numVisibleLines));
    scrollbar_.setBigStepSize(std::max(1, numVisibleLines - 1));

    Widget::resize(width, height);
}

void TextView::updateTextures() {
    parent::updateTextures();

    const auto& gui = GUIStyle::getInstance();

    auto* const renderer = dune::globals::renderer.get();

    if (!pBackground_)
        pBackground_ = gui.createWidgetBackground(getSize().x, getSize().y).createTexture(renderer);

    if (!pForeground_) {
        const auto textLines = greedyWordWrap(text_, static_cast<float>(getSize().x - scrollbar_.getSize().x - 4),
                                              [&gui, font = fontSize_](std::string_view tmp) {
                                                  return static_cast<float>(gui.getMinimumLabelSize(tmp, font).x - 4);
                                              });

        const auto lineHeight  = gui.getTextHeight(fontSize_) + 2.f;
        const auto labelHeight = static_cast<int>(std::ceil(lineHeight * static_cast<float>(textLines.size()) + 2.f));

        pForeground_ = gui.createLabel(renderer, getSize().x - 4, labelHeight, textLines, fontSize_, alignment_,
                                       text_color_, text_shadow_color_, background_color_);
    }
}

void TextView::invalidateTextures() {
    pBackground_.reset();
    pForeground_.reset();

    parent::invalidateTextures();
}
