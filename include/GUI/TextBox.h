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

#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "GUIStyle.h"
#include "Widget.h"
#include "misc/dune_clock.h"
#include <misc/SDL2pp.h>

#include <functional>
#include <string>
#include <string_view>

/// A class for a text box
class TextBox final : public Widget {
    using parent = Widget;

public:
    /// default constructor
    TextBox();

    /// destructor
    ~TextBox() override;

    /**
        Returns true.
        \return true = activatable, false = not activatable
    */
    [[nodiscard]] bool isActivatable() const override { return isEnabled(); }

    /**
        This method sets a new text for this text box.
        \param  Text The new text for this text box
    */
    void setText(std::string_view text) { setText(text, false); }

    /**
        Get the text of this text box.
        \return the text of this text box
    */
    [[nodiscard]] std::string_view getText() const { return text_; }

    /**
        Sets a font size for this text box. Default font size of a text box is 14
        \param  fontSize      the size of the new font
    */
    void setTextFontSize(int fontSize) {
        this->fontSize_ = fontSize;
        resize(getSize().x, getSize().y);
    }

    /**
        Gets the font size of this text box. Default font size of a text box is 14
        \return the font size of this text box
    */
    [[nodiscard]] int getTextFontSize() const { return fontSize_; }

    /**
        Sets the text color for this text box.
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color)
        \param  textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
    */
    void setTextColor(uint32_t textcolor, Uint32 textshadowcolor = COLOR_DEFAULT);

    /**
        Sets the maximum length of the typed text
        \param  maxTextLength   the maximum length, -1 = unlimited
    */
    void setMaximumTextLength(int maxTextLength) { this->maxTextLength_ = maxTextLength; }

    /**
        Gets the maximum length of the typed text
    */
    [[nodiscard]] int getMaximumTextLength() const { return maxTextLength_; }

    /**
        Sets the set of allowed characters for this text box.
        \param  allowedChars    the set of allowed chars or an empty string if everything is allowed
    */
    void setAllowedChars(std::string allowedChars = {}) { this->allowedChars_ = std::move(allowedChars); }

    /**
        Sets the set of forbidden characters for this text box.
        \param  forbiddenChars    the set of forbidden chars or an empty string if everything is allowed
    */
    void setForbiddenChars(std::string forbiddenChars = {}) {
        this->forbiddenChars_ = std::move(forbiddenChars);
    }

    /**
        Sets the function that should be called when the text of this text box changes.
        \param  pOnTextChange   A function to call on text change
    */
    void setOnTextChange(std::function<void(bool)> pOnTextChange) { this->pOnTextChange_ = pOnTextChange; }

    /**
        Sets the method that should be called when return is pressed
        \param  pOnReturn   A function to call on pressing return
    */
    void setOnReturn(std::function<void()> pOnReturn) { this->pOnReturn_ = pOnReturn; }

    /**
        Returns the minimum size of this text box. The text box should not
        resized to a size smaller than this. If the text box is not resizeable
        in a direction this method returns the size in that direction.
        \return the minimum size of this text box
    */
    [[nodiscard]] Point getMinimumSize() const override {
        return GUIStyle::getInstance().getMinimumTextBoxSize(fontSize_);
    }

    /**
        This method resizes the text box to width and height. This method should only be
        called if the new size is a valid size for this text box (See resizingXAllowed,
        resizingYAllowed, getMinimumSize).
        \param  width   the new width of this text box
        \param  height  the new height of this text box
    */
    void resize(uint32_t width, uint32_t height) override {
        parent::resize(width, height);
        invalidateTextures();
    }

    using parent::resize;

    /**
        This method updates all surfaces for this text box. This method will be called
        if this text box is resized or the text changes.
    */
    void updateTextures() override;

    /**
        Draws this text box to screen.
        \param  position    Position to draw the text box to
    */
    void draw(Point position) override;

    /**
        Handles a left mouse click.
        \param  x x-coordinate (relative to the left top corner of the text box)
        \param  y y-coordinate (relative to the left top corner of the text box)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the widget, false = click was not processed by the text box
    */
    bool handleMouseLeft(int32_t x, int32_t y, bool pressed) override;

    /**
        Handles a key stroke.
        \param  key the key that was pressed or released.
        \return true = key stroke was processed by the text box, false = key stroke was not processed by the text box
    */
    bool handleKeyPress(const SDL_KeyboardEvent& key) override;

    /**
        Handles a text input event.
        \param  textInput the text input that was performed.
        \return true = text input was processed by the widget, false = text input was not processed by the widget
    */
    bool handleTextInput(const SDL_TextInputEvent& textInput) override;

protected:
    /**
        This method sets a new text for this text box.
        \param  text            The new text for this text box
        \param  bInteractive    Was this text change initiated by the user?
    */
    void setText(std::string_view text, bool bInteractive);

    /**
        This method frees all textures that are used by this text box
    */
    void invalidateTextures() override;

private:
    int fontSize_               = 14;            ///< the size of the font to use
    uint32_t text_color_        = COLOR_DEFAULT; ///< Text color
    uint32_t text_shadow_color_ = COLOR_DEFAULT; ///< Text shadow color
    std::string text_;                           ///< text in this text box
    int maxTextLength_ = -1;                     ///< the maximum length of the typed text

    std::string allowedChars_;   ///< a set of allowed characters, empty string for everything allowed
    std::string forbiddenChars_; ///< a set of forbidden characters, empty string for everything allowed

    dune::dune_clock::time_point lastCaretTime_ =
        dune::dune_clock::now(); ///< Last time the caret changes from off to on or vise versa

    std::function<void(bool)> pOnTextChange_; ///< function that is called when the text of this text box changes
    std::function<void()> pOnReturn_;         ///< function that is called when return is pressed

    DuneTextureOwned pTextureWithoutCaret_; ///< Texture with caret off
    DuneTextureOwned pTextureWithCaret_;    ///< Texture with caret on
};

#endif // TEXTBOX_H
