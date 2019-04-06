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
#include <GUI/VBox.h>
#include <GUI/TextBox.h>
#include <GUI/Spacer.h>

#include <misc/string_util.h>
#include <misc/draw_util.h>

#include <limits>

/// A class for a text box
class DigitsTextBox : public HBox {

public:
    DigitsTextBox() : house(HOUSE_HARKONNEN) {
        minValue = std::numeric_limits<int>::min();
        maxValue = std::numeric_limits<int>::max();

        incrementValue = 1;

        textBox.setText("0");
        textBox.setAllowedChars("-0123456789");
        textBox.setOnLostFocus(std::bind(&DigitsTextBox::onTextBoxLostFocus, this));
        addWidget(&textBox);

        buttonVBox.addWidget(Spacer::create());

        plusButton.setOnClick(std::bind(&DigitsTextBox::onIncrement, this));
        buttonVBox.addWidget(&plusButton, 0.0);

        minusButton.setOnClick(std::bind(&DigitsTextBox::onDecrement, this));
        buttonVBox.addWidget(&minusButton, 0.0);

        buttonVBox.addWidget(Spacer::create());

        updateSurfaces();

        addWidget(&buttonVBox,0.0);
    }

    virtual ~DigitsTextBox() {
        // we don't want to get notified because we are then already gone
        textBox.setOnLostFocus(std::function<void ()>());
    }

    /**
       Sets the text color for this text box.
       \param   house           the house, used for the button colors
       \param   textcolor       the color of the text (COLOR_DEFAULT = default color)
       \param   textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
    */
    virtual inline void setColor(HOUSETYPE house, Uint32 textcolor, Uint32 textshadowcolor = COLOR_DEFAULT) {
        this->house = house;
        updateSurfaces();
        textBox.setTextColor(textcolor, textshadowcolor);
    }

    void setMinMax(int newMinValue, int newMaxValue) {
        minValue = newMinValue;
        maxValue = newMaxValue;

        int currentValue = getValue();

        if(currentValue < minValue) {
            setValue(minValue, false);
        } else if(currentValue > maxValue) {
            setValue(maxValue, false);
        }

        textBox.setMaximumTextLength(std::to_string(maxValue).length() + (((minValue < 0) && (maxValue >= 0)) ? 1 : 0));

        resizeAll();
    }

    void setIncrementValue(int newIncrementValue) {
        incrementValue = newIncrementValue;
    }

    void setValue(int newValue) {
        setValue(newValue, false);
    }

    int getValue() const {
        int x = 0;
        if(parseString(textBox.getText(), x)) {
            return x;
        } else {
            return 0;
        }
    }

    /**
        Sets the function that should be called when the value of this digit text box changes.
        \param  pOnValueChange  A function to call on value change
    */
    inline void setOnValueChange(std::function<void (bool)> pOnValueChange) {
        textBox.setOnTextChange(pOnValueChange);
        this->pOnValueChange = pOnValueChange;
    }

    /**
        Handles mouse wheel scrolling.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  up  true = mouse wheel up, false = mouse wheel down
        \return true = the mouse wheel scrolling was processed by the widget, false = mouse wheel scrolling was not processed by the widget
    */
    inline bool handleMouseWheel(Sint32 x, Sint32 y, bool up) override
    {
        if((isEnabled() == false) || (isVisible() == false)) {
            return true;
        }

        if(x >= 0 && x < getSize().x && y >= 0 && y < getSize().y) {
            if(up) {
                onDecrement();
            } else {
                onIncrement();
            }

            return true;
        } else {
            return true;
        }
    }

    /**
        Returns the minimum size of this text box. The text box should not
        resized to a size smaller than this. If the text box is not resizeable
        in a direction this method returns the size in that direction.
        \return the minimum size of this text box
    */
    Point getMinimumSize() const override
    {
        if(textBox.getParent() != this || buttonVBox.getParent() != this) {
            // we are about to be destroyed
            return Point(0,0);
        }

        Point textBoxMinimumSize = textBox.getMinimumSize();
        Point buttonVBoxMinimumSize = buttonVBox.getMinimumSize();

        std::string testString(std::max(1,textBox.getMaximumTextLength()), '9');

        return Point(   textBoxMinimumSize.x + buttonVBoxMinimumSize.x + GUIStyle::getInstance().getTextWidth(testString.c_str(), textBox.getTextFontSize()),
                        std::max(textBoxMinimumSize.y,buttonVBoxMinimumSize.y) );
    }

protected:
    void setValue(int newValue, bool bInteractive) {
        textBox.setText(std::to_string(newValue));
        if(bInteractive && pOnValueChange) {
            pOnValueChange(true);
        }
    }

private:

    void onTextBoxLostFocus() {
        int x = 0;
        if(parseString(textBox.getText(), x)) {
            if(x < minValue) {
                setValue(minValue, true);
            } else if(x > maxValue) {
                setValue(maxValue, true);
            }
        } else {
            setValue((0 >= minValue && 0 <= maxValue) ? 0 : minValue, true);
        }
    }

    void onIncrement() {
        int currentValue = getValue();
        currentValue += incrementValue;
        if(currentValue < minValue) {
            setValue(minValue, true);
        } else if(currentValue <= maxValue) {
            setValue(currentValue, true);
        } else {
            setValue(maxValue, true);
        }
    }

    void onDecrement() {
        int currentValue = getValue();
        currentValue -= incrementValue;
        if(currentValue > maxValue) {
            setValue(maxValue, true);
        } else if(currentValue >= minValue) {
            setValue(currentValue, true);
        } else {
            setValue(minValue, true);
        }
    }


    void updateSurfaces() {
        plusButton.setTextures( pGFXManager->getUIGraphic(UI_Plus, house),
                                pGFXManager->getUIGraphic(UI_Plus_Pressed, house),
                                pGFXManager->getUIGraphic(UI_Plus_Active, house));

        minusButton.setTextures(pGFXManager->getUIGraphic(UI_Minus, house),
                                pGFXManager->getUIGraphic(UI_Minus_Pressed, house),
                                pGFXManager->getUIGraphic(UI_Minus_Active, house));
    }


    TextBox         textBox;
    VBox            buttonVBox;
    PictureButton   plusButton;
    PictureButton   minusButton;

    std::function<void (bool)> pOnValueChange;

    int             minValue;
    int             maxValue;

    int             incrementValue;

    HOUSETYPE       house;
};


#endif // DIGITSTEXTBOX_H
