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

#include "Widget.h"
#include <misc/draw_util.h>
#include <misc/string_util.h>
#include <misc/SDL2pp.h>
#include <string>

#include <stdio.h>

/// A class for a text box
class TextBox : public Widget {
public:

    /// default constructor
    TextBox() : Widget() {
        fontSize = 14;
        textcolor = COLOR_DEFAULT;
        textshadowcolor = COLOR_DEFAULT;
        maxTextLength = -1;
        pTextureWithoutCarret = nullptr;
        pTextureWithCarret = nullptr;
        lastCarretTime = SDL_GetTicks();
        enableResizing(true,false);
        resize(getMinimumSize().x,getMinimumSize().y);
    }

    /// destructor
    virtual ~TextBox() {
        invalidateTextures();
    }

    /**
        Returns true.
        \return true = activatable, false = not activatable
    */
    inline bool isActivatable() const override { return isEnabled(); };

    /**
        This method sets a new text for this text box.
        \param  Text The new text for this text box
    */
    virtual void setText(const std::string& text) {
        setText(text, false);
    }

    /**
        Get the text of this text box.
        \return the text of this text box
    */
    inline const std::string& getText() const { return text; };

    /**
        Sets a font size for this text box. Default font size of a text box is 14
        \param  fontSize      the size of the new font
    */
    virtual inline void setTextFontSize(int fontSize) {
        this->fontSize = fontSize;
        resize(getSize().x, getSize().y);
    }

    /**
        Gets the font size of this text box. Default font size of a text box is 14
        \return the font size of this text box
    */
    virtual inline int getTextFontSize() const {
       return fontSize;
    }

    /**
        Sets the text color for this text box.
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color)
        \param  textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
    */
    virtual inline void setTextColor(Uint32 textcolor, Uint32 textshadowcolor = COLOR_DEFAULT) {
        this->textcolor = textcolor;
        this->textshadowcolor = textshadowcolor;
        invalidateTextures();
    }

    /**
        Sets the maximum length of the typed text
        \param  maxTextLength   the maximum length, -1 = unlimited
    */
    virtual inline void setMaximumTextLength(int maxTextLength) {
        this->maxTextLength = maxTextLength;
    }

    /**
        Gets the maximum length of the typed text
    */
    virtual inline int getMaximumTextLength() const {
        return maxTextLength;
    }

    /**
        Sets the set of allowed characters for this text box.
        \param  allowedChars    the set of allowed chars or an empty string if everything is allowed
    */
    virtual inline void setAllowedChars(const std::string& allowedChars = "") {
        this->allowedChars = allowedChars;
    }

    /**
        Sets the set of forbidden characters for this text box.
        \param  forbiddenChars    the set of forbidden chars or an empty string if everything is allowed
    */
    virtual inline void setForbiddenChars(const std::string& forbiddenChars = "") {
        this->forbiddenChars = forbiddenChars;
    }

    /**
        Sets the function that should be called when the text of this text box changes.
        \param  pOnTextChange   A function to call on text change
    */
    inline void setOnTextChange(std::function<void (bool)> pOnTextChange) {
        this->pOnTextChange = pOnTextChange;
    }


    /**
        Sets the method that should be called when return is pressed
        \param  pOnReturn   A function to call on pressing return
    */
    inline void setOnReturn(std::function<void ()> pOnReturn) {
        this->pOnReturn = pOnReturn;
    }


    /**
        Returns the minimum size of this text box. The text box should not
        resized to a size smaller than this. If the text box is not resizeable
        in a direction this method returns the size in that direction.
        \return the minimum size of this text box
    */
    Point getMinimumSize() const override
    {
        return GUIStyle::getInstance().getMinimumTextBoxSize(fontSize);
    }

    /**
        This method resizes the text box. This method should only
        called if the new size is a valid size for this text box (See getMinumumSize).
        \param  newSize the new size of this progress bar
    */
    void resize(Point newSize) override
    {
        resize(newSize.x,newSize.y);
    }

    /**
        This method resizes the text box to width and height. This method should only be
        called if the new size is a valid size for this text box (See resizingXAllowed,
        resizingYAllowed, getMinumumSize).
        \param  width   the new width of this text box
        \param  height  the new height of this text box
    */
    void resize(Uint32 width, Uint32 height) override
    {
        Widget::resize(width,height);
        invalidateTextures();
    }

    /**
        This method updates all surfaces for this text box. This method will be called
        if this text box is resized or the text changes.
    */
    void updateTextures() override
    {
        if(pTextureWithoutCarret == nullptr || pTextureWithCarret == nullptr) {
            invalidateTextures();

            pTextureWithoutCarret = convertSurfaceToTexture(GUIStyle::getInstance().createTextBoxSurface(getSize().x, getSize().y, text, false, fontSize,  Alignment_Left, textcolor, textshadowcolor));
            pTextureWithCarret = convertSurfaceToTexture(GUIStyle::getInstance().createTextBoxSurface(getSize().x, getSize().y, text, true, fontSize, Alignment_Left, textcolor, textshadowcolor));
        }
    }

    /**
        Draws this text box to screen.
        \param  Position    Position to draw the text box to
    */
    void draw(Point position) override
    {
        if(isVisible() == false) {
            return;
        }

        updateTextures();

        if((pTextureWithoutCarret == nullptr) || (pTextureWithCarret == nullptr)) {
            return;
        }

        SDL_Rect dest = calcDrawingRect(pTextureWithoutCarret.get(), position.x, position.y);

        if(isActive()) {
            if((SDL_GetTicks() - lastCarretTime) < 500) {
                SDL_RenderCopy(renderer, pTextureWithCarret.get(), nullptr, &dest);
            } else {
                SDL_RenderCopy(renderer, pTextureWithoutCarret.get(), nullptr, &dest);
            }

            if(SDL_GetTicks() - lastCarretTime >= 1000) {
                lastCarretTime = SDL_GetTicks();
            }
        } else {
            SDL_RenderCopy(renderer, pTextureWithoutCarret.get(), nullptr, &dest);
        }
    }

    /**
        Handles a left mouse click.
        \param  x x-coordinate (relative to the left top corner of the text box)
        \param  y y-coordinate (relative to the left top corner of the text box)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the widget, false = click was not processed by the text box
    */
    bool handleMouseLeft(Sint32 x, Sint32 y, bool pressed) override
    {
        if((x < 0) || (x >= getSize().x) || (y < 0) || (y >= getSize().y)) {
            return false;
        }

        if((isEnabled() == false) || (isVisible() == false)) {
            return true;
        }

        if(pressed == true) {
            setActive();
            lastCarretTime = SDL_GetTicks();
        }
        return true;
    }

    /**
        Handles a key stroke.
        \param  key the key that was pressed or released.
        \return true = key stroke was processed by the text box, false = key stroke was not processed by the text box
    */
    bool handleKeyPress(SDL_KeyboardEvent& key) override
    {
        if((isVisible() == false) || (isEnabled() == false) || (isActive() == false)) {
            return true;
        }

        if(key.keysym.sym == SDLK_TAB) {
            setInactive();
            return true;
        }

        if(key.keysym.sym == SDLK_BACKSPACE) {
            if(text.size() != 0) {
                text = utf8Substr(text, 0, utf8Length(text)-1);
                if(pOnTextChange) {
                    pOnTextChange(true);
                }
            }
        } else if(key.keysym.sym == SDLK_RETURN) {
            if(pOnReturn) {
                pOnReturn();
            }
        }

        invalidateTextures();

        return true;
    }


    /**
        Handles a text input event.
        \param  textInput the text input that was performed.
        \return true = text input was processed by the widget, false = text input was not processed by the widget
    */
    bool handleTextInput(SDL_TextInputEvent& textInput) override
    {
        if((isVisible() == false) || (isEnabled() == false) || (isActive() == false)) {
            return true;
        }

        std::string newText = textInput.text;

        bool bChanged = false;
        for(char c : newText) {
            if(((maxTextLength < 0) || ((int) utf8Length(text) < maxTextLength))
                && (allowedChars.empty() || allowedChars.find(c) != std::string::npos)
                && (forbiddenChars.find(c) == std::string::npos)) {
                text += c;
                bChanged = true;
            }
        }

        if(bChanged && pOnTextChange) {
            pOnTextChange(true);
        }

        return true;
    }

protected:
    /**
        This method sets a new text for this text box.
        \param  text            The new text for this text box
        \param  bInteractive    Was this text change initiated by the user?
    */
    virtual void setText(const std::string& text, bool bInteractive) {
        bool bChanged = (text != this->text);
        this->text = text;
        invalidateTextures();
        if(bChanged && pOnTextChange) {
            pOnTextChange(bInteractive);
        }
    }

    /**
        This method frees all textures that are used by this text box
    */
    void invalidateTextures() override
    {
        pTextureWithoutCarret.reset();
        pTextureWithCarret.reset();
    }

private:
    int fontSize;                               ///< the size of the font to use
    Uint32 textcolor;                           ///< Text color
    Uint32 textshadowcolor;                     ///< Text shadow color
    std::string text;                           ///< text in this text box
    int maxTextLength;                          ///< the maximum length of the typed text

    std::string allowedChars;                   ///< a set of allowed characters, empty string for everything allowed
    std::string forbiddenChars;                 ///< a set of forbidden characters, empty string for everything allowed

    Uint32 lastCarretTime;                      ///< Last time the carret changes from off to on or vise versa

    std::function<void (bool)> pOnTextChange;   ///< function that is called when the text of this text box changes
    std::function<void ()> pOnReturn;           ///< function that is called when return is pressed

    sdl2::texture_ptr pTextureWithoutCarret;         ///< Texture with carret off
    sdl2::texture_ptr pTextureWithCarret;            ///< Texture with carret on
};

#endif // TEXTBOX_H
