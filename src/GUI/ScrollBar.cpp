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

#include <GUI/ScrollBar.h>

#include <cmath>

ScrollBar::ScrollBar() {
    color        = COLOR_DEFAULT;
    minValue     = 1;
    maxValue     = 1;
    currentValue = 1;
    bigStepSize  = 10;
    bDragSlider  = false;

    Widget::enableResizing(false, true);

    updateArrowButtonSurface();

    arrow1.setOnClick(std::bind(&ScrollBar::onArrow1, this));
    arrow2.setOnClick(std::bind(&ScrollBar::onArrow2, this));

    pBackground = nullptr;

    resize(ScrollBar::getMinimumSize());
}

ScrollBar::~ScrollBar() = default;

void ScrollBar::handleMouseMovement(int32_t x, int32_t y, bool insideOverlay) {
    arrow1.handleMouseMovement(x, y, insideOverlay);
    arrow2.handleMouseMovement(x, y - getSize().y + arrow2.getSize().y, insideOverlay);

    if (bDragSlider) {
        const auto SliderAreaHeight = getSize().y - arrow1.getSize().y - arrow2.getSize().y;
        const auto Range            = (maxValue - minValue + 1);

        double OneTickHeight = static_cast<double>(SliderAreaHeight - sliderButton.getSize().y) / static_cast<double>(Range - 1);

        setCurrentValue(static_cast<int>((y - dragPositionFromSliderTop - arrow1.getSize().y) / OneTickHeight));
    }
}

bool ScrollBar::handleMouseLeft(int32_t x, int32_t y, bool pressed) {
    if (!pressed) {
        bDragSlider = false;
    }

    if (x >= 0 && x < getSize().x && y >= 0 && y < getSize().y) {
        if (arrow1.handleMouseLeft(x, y, pressed) || arrow2.handleMouseLeft(x, y - getSize().y + arrow2.getSize().y, pressed)) {
            // one of the arrow buttons clicked
            return true;
        }
        if (pressed) {

            if (y < sliderPosition.y) {

                // between up arrow and slider

                setCurrentValue(currentValue - bigStepSize);

            } else if (y > sliderPosition.y + sliderButton.getSize().y) {

                // between slider and down button

                setCurrentValue(currentValue + bigStepSize);

            } else {

                // slider button

                bDragSlider = true;

                dragPositionFromSliderTop = y - sliderPosition.y;
            }
        }

        return true;

    } else {
        return false;
    }
}

bool ScrollBar::handleMouseWheel(int32_t x, int32_t y, bool up) {
    if ((x >= 0) && (x < getSize().x) && (y >= 0) && (y < getSize().y)) {
        if (up) {
            setCurrentValue(currentValue - 1);
        } else {
            setCurrentValue(currentValue + 1);
        }
        return true;
    }
    return false;
}

bool ScrollBar::handleKeyPress(SDL_KeyboardEvent& key) {
    return true;
}

void ScrollBar::draw(Point position) {
    if (!isVisible()) {
        return;
    }

    updateTextures();

    if (pBackground != nullptr) {
        SDL_Rect dest = calcDrawingRect(pBackground.get(), position.x, position.y);
        Dune_RenderCopy(renderer, pBackground.get(), nullptr, &dest);
    }

    arrow1.draw(position);
    auto p = position;
    p.y    = p.y + getSize().y - arrow2.getSize().y;
    arrow2.draw(p);
    sliderButton.draw(position + sliderPosition);
}

void ScrollBar::resize(uint32_t width, uint32_t height) {
    Widget::resize(width, height);

    invalidateTextures();

    updateSliderButton();
}

void ScrollBar::updateSliderButton() {
    auto Range            = static_cast<double>(maxValue - minValue + 1);
    int ArrowHeight       = GUIStyle::getInstance().getMinimumScrollBarArrowButtonSize().y;
    auto SliderAreaHeight = static_cast<double>(getSize().y - 2 * ArrowHeight);

    if (SliderAreaHeight < 0.0) {
        SliderAreaHeight = GUIStyle::getInstance().getMinimumScrollBarArrowButtonSize().y;
    }

    double SliderButtonHeight = NAN;
    double OneTickHeight      = NAN;

    if (Range <= 1) {
        SliderButtonHeight = SliderAreaHeight;
        OneTickHeight      = 0;
    } else {
        SliderButtonHeight = (SliderAreaHeight * bigStepSize) / (Range + bigStepSize);
        if (SliderButtonHeight <= 7) {
            SliderButtonHeight = 7;
        }
        OneTickHeight = (SliderAreaHeight - SliderButtonHeight) / (Range - 1);
    }

    sliderButton.resize(getSize().x, lround(SliderButtonHeight));
    sliderPosition.x = 0;
    sliderPosition.y = ArrowHeight + lround((currentValue - minValue) * OneTickHeight);
}

void ScrollBar::updateArrowButtonSurface() {
    arrow1.setSurfaces(GUIStyle::getInstance().createScrollBarArrowButton(false, false, false, color),
                       GUIStyle::getInstance().createScrollBarArrowButton(false, true, true, color),
                       GUIStyle::getInstance().createScrollBarArrowButton(false, false, true, color));

    arrow2.setSurfaces(GUIStyle::getInstance().createScrollBarArrowButton(true, false, false, color),
                       GUIStyle::getInstance().createScrollBarArrowButton(true, true, true, color),
                       GUIStyle::getInstance().createScrollBarArrowButton(true, false, true, color));
}
