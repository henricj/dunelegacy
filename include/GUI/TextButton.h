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

#ifndef TEXTBUTTON_H
#define TEXTBUTTON_H

#include "Button.h"
#include "GUIStyle.h"

#include <string>

#include <iostream>

/// A class for a text button
class TextButton : public Button {
public:
    /// Default contructor
    TextButton() : Button() {
        textcolor = COLOR_DEFAULT;
        textshadowcolor = COLOR_DEFAULT;

        enableResizing(true,true);
    }

    /// destructor
    virtual ~TextButton() {
    }

    /**
        This method sets a new text for this button and resizes this button
        to fit this text.
        \param  text The new text for this button
    */
    virtual inline void setText(std::string text) {
        this->text = text;
        resizeAll();
    }

    /**
        Get the text of this button.
        \return the text of this button
    */
    inline std::string getText() { return text; };

    /**
        Sets the text color for this button.
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color)
        \param  textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
    */
    virtual inline void setTextColor(Uint32 textcolor, Uint32 textshadowcolor = COLOR_DEFAULT) {
        this->textcolor = textcolor;
        this->textshadowcolor = textshadowcolor;
        invalidateTextures();
    }

    /**
        This method resizes the button. This method should only
        called if the new size is a valid size for this button (See getMinumumSize).
        \param  newSize the new size of this progress bar
    */
    virtual void resize(Point newSize) {
        resize(newSize.x,newSize.y);
    }

    /**
        This method resizes the button to width and height. This method should only
        called if the new size is a valid size for this button (See getMinumumSize).
        \param  width   the new width of this button
        \param  height  the new height of this button
    */
    virtual void resize(Uint32 width, Uint32 height) {
        invalidateTextures();
        Button::resize(width,height);
    }

    /**
        Returns the minimum size of this button. The button should not
        resized to a size smaller than this.
        \return the minimum size of this button
    */
    virtual Point getMinimumSize() const {
        return GUIStyle::getInstance().getMinimumButtonSize(text);
    }

protected:
    /**
        This method is called whenever the textures of this widget are needed, e.g. before drawing. This method
        should be overwritten by subclasses if they like to defer texture creation as long as possible.
        This method should first check whether a renewal of the textures is necessary.
    */
    virtual void updateTextures() {
        Button::updateTextures();

        if(pUnpressedTexture == nullptr) {
            invalidateTextures();

            setSurfaces(    GUIStyle::getInstance().createButtonSurface(getSize().x, getSize().y, text, false, false, textcolor, textshadowcolor),true,
                            GUIStyle::getInstance().createButtonSurface(getSize().x, getSize().y, text, true, true, textcolor, textshadowcolor),true,
                            GUIStyle::getInstance().createButtonSurface(getSize().x, getSize().y, text, false, true, textcolor, textshadowcolor),true);
        }
    }

private:
    Uint32 textcolor;
    Uint32 textshadowcolor;

    std::string text;       ///< Text of this button
};

#endif // TEXTBUTTON_H
