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

/// A class for a text button
class TextButton final : public Button {
    using parent = Button;

public:
    /// Default constructor
    TextButton();

    TextButton(const TextButton&) = delete;
    TextButton(TextButton&&) noexcept;
    TextButton& operator=(const TextButton&) = delete;
    TextButton& operator=(TextButton&&) noexcept;

    /// destructor
    ~TextButton() override;

    /**
        This method sets a new text for this button and resizes this button
        to fit this text.
        \param  text The new text for this button
    */
    void setText(std::string text);

    /**
        This method sets a new text for this button and resizes this button
        to fit this text.
        \param  text The new text for this button
    */
    void setText(std::string_view text);

    /**
        Get the text of this button.
        \return the text of this button
    */
    [[nodiscard]] const std::string& getText() const noexcept { return text_; }

    /**
        Sets the text color for this button.
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color)
        \param  textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
    */
    void setTextColor(uint32_t textcolor, Uint32 textshadowcolor = COLOR_DEFAULT) {
        this->textcolor_       = textcolor;
        this->textshadowcolor_ = textshadowcolor;
        invalidateTextures();
    }

    /**
        This method resizes the button to width and height. This method should only
        called if the new size is a valid size for this button (See getMinimumSize).
        \param  width   the new width of this button
        \param  height  the new height of this button
    */
    void resize(uint32_t width, uint32_t height) override;

    using parent::resize;

    /**
        Returns the minimum size of this button. The button should not
        resized to a size smaller than this.
        \return the minimum size of this button
    */
    [[nodiscard]] Point getMinimumSize() const override { return GUIStyle::getInstance().getMinimumButtonSize(text_); }

protected:
    /**
        This method is called whenever the textures of this widget are needed, e.g. before drawing. This method
        should be overwritten by subclasses if they like to defer texture creation as long as possible.
        This method should first check whether a renewal of the textures is necessary.
    */
    void updateTextures() override;

private:
    uint32_t textcolor_       = COLOR_DEFAULT;
    uint32_t textshadowcolor_ = COLOR_DEFAULT;

    std::string text_; ///< Text of this button
};

#endif // TEXTBUTTON_H
