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
    scrollbar.handleMouseMovement(x - getSize().x + scrollbar.getSize().x, y, insideOverlay);
}

bool TextView::handleMouseLeft(int32_t x, int32_t y, bool pressed) {
    return scrollbar.handleMouseLeft(x - getSize().x + scrollbar.getSize().x, y, pressed);
}

bool TextView::handleMouseWheel(int32_t x, int32_t y, bool up) {
    // forward mouse wheel event to scrollbar
    return scrollbar.handleMouseWheel(0, 0, up);
}

bool TextView::handleKeyPress(const SDL_KeyboardEvent& key) {
    Widget::handleKeyPress(key);

    scrollbar.handleKeyPress(key);
    return true;
}

void TextView::draw(Point position) {
    if (!isVisible()) {
        return;
    }

    updateTextures();

    auto* const renderer = dune::globals::renderer.get();

    if (pBackground)
        pBackground.draw(renderer, position.x, position.y);

    if (pForeground) {
        const auto& gui = GUIStyle::getInstance();

        const auto lineHeight = gui.getTextHeight(fontSize) + 2.f;

        const auto src_y = static_cast<int>(std::round(static_cast<float>(scrollbar.getCurrentValue()) * lineHeight));

        const auto height =
            std::min(getHeight(pForeground.get()),
                     static_cast<int>(std::round(static_cast<int>(getSize().y) - 2 * gui.getActualScale())));

        const SDL_Rect src{0, src_y, getWidth(pForeground.get()), height};

        const SDL_FRect dest = {static_cast<float>(position.x) + 2.f, static_cast<float>(position.y) + 1.f,
                                pForeground.width_,
                                std::min(pForeground.height_, static_cast<float>(getSize().y) - 2.f)};

        Dune_RenderCopyF(renderer, pForeground.get(), &src, &dest);
    }

    Point scrollBarPos = position;
    scrollBarPos.x += getSize().x - scrollbar.getSize().x;

    if (!bAutohideScrollbar || (scrollbar.getRangeMin() != scrollbar.getRangeMax())) {
        scrollbar.draw(scrollBarPos);
    }
}

void TextView::resize(uint32_t width, uint32_t height) {
    invalidateTextures();

    scrollbar.resize(scrollbar.getMinimumSize().x, height);

    auto& gui = GUIStyle::getInstance();

    const std::vector<std::string> textLines =
        greedyWordWrap(text, static_cast<float>(getSize().x - scrollbar.getSize().x - 4),
                       [&gui, font = fontSize](std::string_view tmp) {
                           return static_cast<float>(gui.getMinimumLabelSize(tmp, font).x - 4);
                       });

    const auto lineHeight = gui.getTextHeight(fontSize) + 2.f;

    const auto numVisibleLines = static_cast<int>(std::floor(static_cast<int>(height) / lineHeight));
    scrollbar.setRange(0, std::max(0, static_cast<int>(textLines.size()) - numVisibleLines));
    scrollbar.setBigStepSize(std::max(1, numVisibleLines - 1));

    Widget::resize(width, height);
}

void TextView::updateTextures() {
    parent::updateTextures();

    const auto& gui = GUIStyle::getInstance();

    auto* const renderer = dune::globals::renderer.get();

    if (!pBackground)
        pBackground = gui.createWidgetBackground(getSize().x, getSize().y).createTexture(renderer);

    if (!pForeground) {
        const auto textLines = greedyWordWrap(text, static_cast<float>(getSize().x - scrollbar.getSize().x - 4),
                                              [&gui, font = fontSize](std::string_view tmp) {
                                                  return static_cast<float>(gui.getMinimumLabelSize(tmp, font).x - 4);
                                              });

        const auto lineHeight  = gui.getTextHeight(fontSize) + 2.f;
        const auto labelHeight = static_cast<int>(std::ceil(lineHeight * static_cast<float>(textLines.size()) + 2.f));

        pForeground = gui.createLabel(renderer, getSize().x - 4, labelHeight, textLines, fontSize, alignment, textcolor,
                                      textshadowcolor, backgroundcolor);
    }
}

void TextView::invalidateTextures() {
    pBackground.reset();
    pForeground.reset();

    parent::invalidateTextures();
}
