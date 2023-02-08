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
    DigitsTextBox();

    ~DigitsTextBox() override;

    /**
       Sets the text color for this text box.
       \param   house           the house, used for the button colors
       \param   textcolor       the color of the text (COLOR_DEFAULT = default color)
       \param   textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
    */
    void setColor(HOUSETYPE house, uint32_t textcolor, Uint32 textshadowcolor = COLOR_DEFAULT);

    void setMinMax(int newMinValue, int newMaxValue);

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
    bool handleMouseWheel(int32_t x, int32_t y, bool up) override;

    /**
        Returns the minimum size of this text box. The text box should not
        resized to a size smaller than this. If the text box is not resizeable
        in a direction this method returns the size in that direction.
        \return the minimum size of this text box
    */
    [[nodiscard]] Point getMinimumSize() const override;

protected:
    void setValue(int newValue, bool bInteractive);

private:
    void onTextBoxLostFocus();

    void onIncrement();

    void onDecrement();

    void updateSurfaces();

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
