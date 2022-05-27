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

#ifndef DIGITSTEXTBOX_H
#define DIGITSTEXTBOX_H

#include <globals.h>

#include <FileClasses/GFXManager.h>

#include <GUI/HBox.h>
#include <GUI/Spacer.h>
#include <GUI/TextBox.h>
#include <GUI/VBox.h>

#include <misc/string_util.h>

#include <limits>

/// A class for a text box
class DigitsTextBox final : public HBox {

public:
    DigitsTextBox() {
        textBox_.setText("0");
        textBox_.setAllowedChars("-0123456789");
        textBox_.setOnLostFocus([this] { onTextBoxLostFocus(); });
        DigitsTextBox::addWidget(&textBox_);

        buttonVBox_.addWidget(Widget::create<Spacer>().release());

        plusButton_.setOnClick([this] { onIncrement(); });
        buttonVBox_.addWidget(&plusButton_, 0.0);

        minusButton_.setOnClick([this] { onDecrement(); });
        buttonVBox_.addWidget(&minusButton_, 0.0);

        buttonVBox_.addWidget(Widget::create<Spacer>().release());

        updateSurfaces();

        DigitsTextBox::addWidget(&buttonVBox_, 0.0);
    }

    ~DigitsTextBox() override {
        // we don't want to get notified because we are then already gone
        textBox_.setOnLostFocus(std::function<void()>());
    }

    /**
       Sets the text color for this text box.
       \param   house           the house, used for the button colors
       \param   textcolor       the color of the text (COLOR_DEFAULT = default color)
       \param   textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
    */
    virtual void setColor(HOUSETYPE house, uint32_t textcolor, Uint32 textshadowcolor = COLOR_DEFAULT) {
        this->house_ = house;
        updateSurfaces();
        textBox_.setTextColor(textcolor, textshadowcolor);
    }

    void setMinMax(int newMinValue, int newMaxValue) {
        minValue_ = newMinValue;
        maxValue_ = newMaxValue;

        const int currentValue = getValue();

        if (currentValue < minValue_) {
            setValue(minValue_, false);
        } else if (currentValue > maxValue_) {
            setValue(maxValue_, false);
        }

        textBox_.setMaximumTextLength(std::to_string(maxValue_).length()
                                      + (((minValue_ < 0) && (maxValue_ >= 0)) ? 1 : 0));

        resizeAll();
    }

    void setIncrementValue(int newIncrementValue) { incrementValue_ = newIncrementValue; }

    void setValue(int newValue) { setValue(newValue, false); }

    [[nodiscard]] int getValue() const {
        int x = 0;
        if (parseString(textBox_.getText(), x)) {
            return x;
        }
        return 0;
    }

    /**
        Sets the function that should be called when the value of this digit text box changes.
        \param  pOnValueChange  A function to call on value change
    */
    void setOnValueChange(std::function<void(bool)> pOnValueChange) {
        textBox_.setOnTextChange(pOnValueChange);
        this->pOnValueChange_ = pOnValueChange;
    }

    /**
        Handles mouse wheel scrolling.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  up  true = mouse wheel up, false = mouse wheel down
        \return true = the mouse wheel scrolling was processed by the widget, false = mouse wheel scrolling was not
       processed by the widget
    */
    bool handleMouseWheel(int32_t x, int32_t y, bool up) override {
        if ((!isEnabled()) || (!isVisible())) {
            return true;
        }

        if (x >= 0 && x < getSize().x && y >= 0 && y < getSize().y) {
            if (up) {
                onDecrement();
            } else {
                onIncrement();
            }

            return true;
        }
        return true;
    }

    /**
        Returns the minimum size of this text box. The text box should not
        resized to a size smaller than this. If the text box is not resizeable
        in a direction this method returns the size in that direction.
        \return the minimum size of this text box
    */
    [[nodiscard]] Point getMinimumSize() const override {
        if (textBox_.getParent() != this || buttonVBox_.getParent() != this) {
            // we are about to be destroyed
            return {0, 0};
        }

        const Point textBoxMinimumSize    = textBox_.getMinimumSize();
        const Point buttonVBoxMinimumSize = buttonVBox_.getMinimumSize();

        const std::string testString(std::max(1, textBox_.getMaximumTextLength()), '9');

        return {textBoxMinimumSize.x + buttonVBoxMinimumSize.x
                    + static_cast<int>(
                        GUIStyle::getInstance().getTextWidth(testString.c_str(), textBox_.getTextFontSize())),
                std::max(textBoxMinimumSize.y, buttonVBoxMinimumSize.y)};
    }

protected:
    void setValue(int newValue, bool bInteractive) {
        textBox_.setText(std::to_string(newValue));
        if (bInteractive && pOnValueChange_) {
            pOnValueChange_(true);
        }
    }

private:
    void onTextBoxLostFocus() {
        int x = 0;
        if (parseString(textBox_.getText(), x)) {
            if (x < minValue_) {
                setValue(minValue_, true);
            } else if (x > maxValue_) {
                setValue(maxValue_, true);
            }
        } else {
            setValue((0 >= minValue_ && 0 <= maxValue_) ? 0 : minValue_, true);
        }
    }

    void onIncrement() {
        int currentValue = getValue();
        currentValue += incrementValue_;
        if (currentValue < minValue_) {
            setValue(minValue_, true);
        } else if (currentValue <= maxValue_) {
            setValue(currentValue, true);
        } else {
            setValue(maxValue_, true);
        }
    }

    void onDecrement() {
        int currentValue = getValue();
        currentValue -= incrementValue_;
        if (currentValue > maxValue_) {
            setValue(maxValue_, true);
        } else if (currentValue >= minValue_) {
            setValue(currentValue, true);
        } else {
            setValue(minValue_, true);
        }
    }

    void updateSurfaces() {
        auto* const gfx = dune::globals::pGFXManager.get();

        plusButton_.setTextures(gfx->getUIGraphic(UI_Plus, house_), gfx->getUIGraphic(UI_Plus_Pressed, house_),
                                gfx->getUIGraphic(UI_Plus_Active, house_));

        minusButton_.setTextures(gfx->getUIGraphic(UI_Minus, house_), gfx->getUIGraphic(UI_Minus_Pressed, house_),
                                 gfx->getUIGraphic(UI_Minus_Active, house_));
    }

    TextBox textBox_;
    VBox buttonVBox_;
    PictureButton plusButton_;
    PictureButton minusButton_;

    std::function<void(bool)> pOnValueChange_;

    int minValue_ = std::numeric_limits<int>::min();
    int maxValue_ = std::numeric_limits<int>::max();

    int incrementValue_ = 1;

    HOUSETYPE house_{HOUSETYPE::HOUSE_HARKONNEN};
};

#endif // DIGITSTEXTBOX_H
