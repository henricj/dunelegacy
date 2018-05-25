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

ScrollBar::ScrollBar() : Widget() {
    color = COLOR_DEFAULT;
    minValue = 1;
    maxValue = 1;
    currentValue = 1;
    bigStepSize = 10;
    bDragSlider = false;

    enableResizing(false,true);

    updateArrowButtonSurface();

    arrow1.setOnClick(std::bind(&ScrollBar::onArrow1, this));
    arrow2.setOnClick(std::bind(&ScrollBar::onArrow2, this));

    pBackground = nullptr;

    resize(ScrollBar::getMinimumSize());
}

ScrollBar::~ScrollBar() = default;

void ScrollBar::handleMouseMovement(Sint32 x, Sint32 y, bool insideOverlay) {
    arrow1.handleMouseMovement(x,y,insideOverlay);
    arrow2.handleMouseMovement(x,y - getSize().y + arrow2.getSize().y,insideOverlay);

    if(bDragSlider) {
        int SliderAreaHeight = getSize().y - arrow1.getSize().y - arrow2.getSize().y;
        int Range = (maxValue - minValue + 1);

        double OneTickHeight = static_cast<double>(SliderAreaHeight - sliderButton.getSize().y) / static_cast<double>(Range - 1);

        setCurrentValue(static_cast<int>((y - dragPositionFromSliderTop - arrow1.getSize().y) / OneTickHeight));
    }
}

bool ScrollBar::handleMouseLeft(Sint32 x, Sint32 y, bool pressed) {
    if(pressed == false) {
        bDragSlider = false;
    }

    if(x >= 0 && x < getSize().x && y >= 0 && y < getSize().y) {
        if(arrow1.handleMouseLeft(x, y, pressed) || arrow2.handleMouseLeft(x, y - getSize().y + arrow2.getSize().y, pressed)) {
            // one of the arrow buttons clicked
            return true;
        } else {
            if(pressed) {
                if(y < sliderPosition.y) {
                    // between up arrow and slider
                    setCurrentValue(currentValue-bigStepSize);
                } else if(y > sliderPosition.y + sliderButton.getSize().y) {
                    // between slider and down button
                    setCurrentValue(currentValue+bigStepSize);
                } else {
                    // slider button
                    bDragSlider = true;
                    dragPositionFromSliderTop = y - sliderPosition.y;
                }
            }
            return true;
        }
    } else {
        return false;
    }
}

bool ScrollBar::handleMouseWheel(Sint32 x, Sint32 y, bool up)  {
    if((x >= 0) && (x < getSize().x) && (y >= 0) && (y < getSize().y)) {
        if(up == true) {
            setCurrentValue(currentValue-1);
        } else {
            setCurrentValue(currentValue+1);
        }
        return true;
    } else {
        return false;
    }
}

bool ScrollBar::handleKeyPress(SDL_KeyboardEvent& key) {
    return true;
}

void ScrollBar::draw(Point position) {
    if(isVisible() == false) {
        return;
    }

    updateTextures();

    if(pBackground != nullptr) {
        SDL_Rect dest = calcDrawingRect(pBackground.get(), position.x, position.y);
        SDL_RenderCopy(renderer, pBackground.get(), nullptr, &dest);
    }

    arrow1.draw(position);
    Point p = position;
    p.y = p.y + getSize().y - arrow2.getSize().y;
    arrow2.draw(p);
    sliderButton.draw(position+sliderPosition);
}

void ScrollBar::resize(Uint32 width, Uint32 height) {
    Widget::resize(width,height);

    invalidateTextures();

    updateSliderButton();
}

void ScrollBar::updateSliderButton() {
    double Range = static_cast<double>(maxValue - minValue + 1);
    int ArrowHeight = GUIStyle::getInstance().getMinimumScrollBarArrowButtonSize().y;
    double SliderAreaHeight = static_cast<double>(getSize().y - 2 * ArrowHeight);

    if(SliderAreaHeight < 0.0) {
        SliderAreaHeight = GUIStyle::getInstance().getMinimumScrollBarArrowButtonSize().y;
    }

    double SliderButtonHeight;
    double OneTickHeight;

    if(Range <= 1) {
        SliderButtonHeight = SliderAreaHeight;
        OneTickHeight = 0;
    } else {
        SliderButtonHeight = (SliderAreaHeight*bigStepSize) / (Range+bigStepSize);
        if(SliderButtonHeight <= 7) {
            SliderButtonHeight = 7;
        }
        OneTickHeight = (SliderAreaHeight - SliderButtonHeight)/(Range-1);
    }

    sliderButton.resize(getSize().x, lround(SliderButtonHeight));
    sliderPosition.x = 0;
    sliderPosition.y = ArrowHeight +  lround((currentValue - minValue)*OneTickHeight);
}

void ScrollBar::updateArrowButtonSurface() {
    arrow1.setSurfaces( GUIStyle::getInstance().createScrollBarArrowButton(false,false,false,color),
                        GUIStyle::getInstance().createScrollBarArrowButton(false,true,true,color),
                        GUIStyle::getInstance().createScrollBarArrowButton(false,false,true,color));

    arrow2.setSurfaces( GUIStyle::getInstance().createScrollBarArrowButton(true,false,false,color),
                        GUIStyle::getInstance().createScrollBarArrowButton(true,true,true,color),
                        GUIStyle::getInstance().createScrollBarArrowButton(true,false,true,color));
}


