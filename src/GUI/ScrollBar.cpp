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

ScrollBar::ScrollBar() : color_(COLOR_DEFAULT) {

    Widget::enableResizing(false, true);

    updateArrowButtonSurface();

    arrow1_.setOnClick([this] { onArrow1(); });
    arrow2_.setOnClick([this] { onArrow2(); });

    resize(ScrollBar::getMinimumSize());
}

ScrollBar::~ScrollBar() = default;

void ScrollBar::handleMouseMovement(int32_t x, int32_t y, bool insideOverlay) {
    arrow1_.handleMouseMovement(x, y, insideOverlay);
    arrow2_.handleMouseMovement(x, y - getSize().y + arrow2_.getSize().y, insideOverlay);

    if (bDragSlider_) {
        const auto SliderAreaHeight = getSize().y - arrow1_.getSize().y - arrow2_.getSize().y;
        const auto Range            = (maxValue_ - minValue_ + 1);

        const double OneTickHeight =
            static_cast<double>(SliderAreaHeight - sliderButton_.getSize().y) / static_cast<double>(Range - 1);

        setCurrentValue(static_cast<int>((y - dragPositionFromSliderTop_ - arrow1_.getSize().y) / OneTickHeight));
    }
}

bool ScrollBar::handleMouseLeft(int32_t x, int32_t y, bool pressed) {
    if (!pressed) {
        bDragSlider_ = false;
    }

    if (x >= 0 && x < getSize().x && y >= 0 && y < getSize().y) {
        if (arrow1_.handleMouseLeft(x, y, pressed)
            || arrow2_.handleMouseLeft(x, y - getSize().y + arrow2_.getSize().y, pressed)) {
            // one of the arrow buttons clicked
            return true;
        }
        if (pressed) {

            if (y < sliderPosition_.y) {

                // between up arrow and slider

                setCurrentValue(currentValue_ - bigStepSize_);

            } else if (y > sliderPosition_.y + sliderButton_.getSize().y) {

                // between slider and down button

                setCurrentValue(currentValue_ + bigStepSize_);

            } else {

                // slider button

                bDragSlider_ = true;

                dragPositionFromSliderTop_ = y - sliderPosition_.y;
            }
        }

        return true;
    }
    return false;
}

bool ScrollBar::handleMouseWheel(int32_t x, int32_t y, bool up) {
    if ((x >= 0) && (x < getSize().x) && (y >= 0) && (y < getSize().y)) {
        if (up) {
            setCurrentValue(currentValue_ - 1);
        } else {
            setCurrentValue(currentValue_ + 1);
        }
        return true;
    }
    return false;
}

bool ScrollBar::handleKeyPress(const SDL_KeyboardEvent& key) {
    return true;
}

void ScrollBar::draw(Point position) {
    if (!isVisible()) {
        return;
    }

    updateTextures();

    if (pBackground_)
        pBackground_.draw(dune::globals::renderer.get(), position.x, position.y);

    arrow1_.draw(position);
    auto p = position;
    p.y    = p.y + getSize().y - arrow2_.getSize().y;
    arrow2_.draw(p);
    sliderButton_.draw(position + sliderPosition_);
}

void ScrollBar::resize(uint32_t width, uint32_t height) {
    Widget::resize(width, height);

    invalidateTextures();

    updateSliderButton();
}

void ScrollBar::updateSliderButton() {
    const auto Range      = static_cast<double>(maxValue_ - minValue_ + 1);
    const int ArrowHeight = GUIStyle::getInstance().getMinimumScrollBarArrowButtonSize().y;
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
        SliderButtonHeight = (SliderAreaHeight * bigStepSize_) / (Range + bigStepSize_);
        if (SliderButtonHeight <= 7) {
            SliderButtonHeight = 7;
        }
        OneTickHeight = (SliderAreaHeight - SliderButtonHeight) / (Range - 1);
    }

    sliderButton_.resize(getSize().x, lround(SliderButtonHeight));
    sliderPosition_.x = 0;
    sliderPosition_.y = ArrowHeight + lround((currentValue_ - minValue_) * OneTickHeight);
}

void ScrollBar::updateArrowButtonSurface() {
    arrow1_.setSurfaces(GUIStyle::getInstance().createScrollBarArrowButton(false, false, false, color_),
                        GUIStyle::getInstance().createScrollBarArrowButton(false, true, true, color_),
                        GUIStyle::getInstance().createScrollBarArrowButton(false, false, true, color_));

    arrow2_.setSurfaces(GUIStyle::getInstance().createScrollBarArrowButton(true, false, false, color_),
                        GUIStyle::getInstance().createScrollBarArrowButton(true, true, true, color_),
                        GUIStyle::getInstance().createScrollBarArrowButton(true, false, true, color_));
}
