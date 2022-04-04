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

#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "Button.h"
#include "GUIStyle.h"

#include <string>

/// A class for a checkbox implemented as a toggle button
class Checkbox final : public Button {
public:
    /// Default constructor
    Checkbox() {
        Checkbox::enableResizing(true, false);
        setToggleButton(true);
    }

    /// destructor
    ~Checkbox() override { Checkbox::invalidateTextures(); }

    /**
        This method sets a new text for this checkbox and resizes it
        to fit this text.
        \param  text The new text for this checkbox
    */
    virtual void setText(const std::string& text) {
        this->text = text;
        resizeAll();
    }

    /**
        Get the text of this checkbox.
        \return the text of this checkbox
    */
    [[nodiscard]] const std::string& getText() const noexcept { return text; }

    /**
        Sets the text color for this checkbox.
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color)
        \param  textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
    */
    virtual void setTextColor(uint32_t textcolor, Uint32 textshadowcolor = COLOR_DEFAULT) {
        this->textcolor       = textcolor;
        this->textshadowcolor = textshadowcolor;
        invalidateTextures();
    }

    /**
        This method sets this checkbox to checked or unchecked. It does the same as setToggleState().
        \param bChecked true = checked, false = unchecked
    */
    void setChecked(bool bChecked) { setToggleState(bChecked); }

    /**
        This method returns whether this checkbox is checked. It is the same as getToggleState().
        \return true = checked, false = unchecked
    */
    [[nodiscard]] bool isChecked() const noexcept { return getToggleState(); }

    /**
        Draws this button to screen. This method is called before drawOverlay().
        \param  position    Position to draw the button to
    */
    void draw(Point position) override {
        if (!isVisible()) {
            return;
        }

        updateTextures();

        DuneTexture tex;
        if (isChecked()) {
            if ((isActive() || bHover) && pCheckedActiveTexture) {
                tex = DuneTexture{pCheckedActiveTexture.get()};
            } else {
                tex = *pPressedTexture;
            }
        } else {
            if ((isActive() || bHover) && pActiveTexture) {
                tex = *pActiveTexture;
            } else {
                tex = *pUnpressedTexture;
            }
        }

        if (!tex) {
            return;
        }

        tex.draw(renderer, position.x, position.y);
    }

    /**
        This method resizes the checkbox. This method should only
        called if the new size is a valid size for this progress bar (See getMinimumSize).
        \param  newSize the new size of this progress bar
    */
    void resize(Point newSize) override { resize(newSize.x, newSize.y); }

    /**
        This method resizes the checkbox to width and height. This method should only
        called if the new size is a valid size for this checkbox (See getMinimumSize).
        \param  width   the new width of this checkbox
        \param  height  the new height of this checkbox
    */
    void resize(uint32_t width, uint32_t height) override {
        invalidateTextures();
        Widget::resize(width, height);
    }

    /**
        Returns the minimum size of this button. The button should not
        resized to a size smaller than this.
        \return the minimum size of this button
    */
    [[nodiscard]] Point getMinimumSize() const override { return GUIStyle::getInstance().getMinimumCheckboxSize(text); }

protected:
    /**
        This method is called whenever the textures of this widget are needed, e.g. before drawing. This method
        should be overwritten by subclasses if they like to defer texture creation as long as possible.
        This method should first check whether a renewal of the textures is necessary.
    */
    void updateTextures() override {
        Button::updateTextures();

        if (!pUnpressedTexture) {
            invalidateTextures();

            setSurfaces(GUIStyle::getInstance().createCheckboxSurface(getSize().x, getSize().y, text, false, false,
                                                                      textcolor, textshadowcolor),
                        GUIStyle::getInstance().createCheckboxSurface(getSize().x, getSize().y, text, true, false,
                                                                      textcolor, textshadowcolor),
                        GUIStyle::getInstance().createCheckboxSurface(getSize().x, getSize().y, text, false, true,
                                                                      textcolor, textshadowcolor));

            pCheckedActiveTexture = convertSurfaceToTexture(GUIStyle::getInstance().createCheckboxSurface(
                getSize().x, getSize().y, text, true, true, textcolor, textshadowcolor));
        }
    }

    /**
        This method frees all textures that are used by this checkbox
    */
    void invalidateTextures() override {
        Button::invalidateTextures();

        pCheckedActiveTexture.reset();
    }

private:
    uint32_t textcolor{COLOR_DEFAULT};       ///< Text color
    uint32_t textshadowcolor{COLOR_DEFAULT}; ///< Text shadow color
    std::string text;                        ///< Text of this checkbox
    sdl2::texture_ptr
        pCheckedActiveTexture; ///< Texture that is shown when the checkbox is activated by keyboard or by mouse hover
};

#endif // CHECKBOX_H
