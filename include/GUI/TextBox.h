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
#include <string>
#include <SDL.h>

#include <stdio.h>

/// A class for a text box
class TextBox : public Widget {
public:

	/// default constructor
	TextBox() : Widget() {
	    fontID = FONT_STD12;
        textcolor = COLOR_DEFAULT;
	    textshadowcolor = COLOR_DEFAULT;
	    maxTextLength = -1;
		pTextureWithoutCarret = NULL;
		pTextureWithCarret = NULL;
		lastCarretTime = SDL_GetTicks();
		enableResizing(true,false);
		resize(getMinimumSize().x,getMinimumSize().y);
	}

	/// destructor
	virtual ~TextBox() {
		if(pTextureWithoutCarret != NULL) {
			SDL_DestroyTexture(pTextureWithoutCarret);
			pTextureWithoutCarret = NULL;
		}

		if(pTextureWithCarret != NULL) {
			SDL_DestroyTexture(pTextureWithCarret);
			pTextureWithCarret = NULL;
		}
	}

	/**
		Returns true.
		\return	true = activatable, false = not activatable
	*/
	virtual inline bool isActivatable() const { return isEnabled(); };

	/**
		This method sets a new text for this text box.
		\param	Text The new text for this text box
	*/
	virtual void setText(std::string text) {
	    setText(text, false);
	}

	/**
		Get the text of this text box.
		\return the text of this text box
	*/
	inline std::string getText() const { return text; };

    /**
		Sets a font for this text box. Default font of a text box is FONT_STD12
		\param	fontID	    the ID of the new font
	*/
	virtual inline void setTextFont(int fontID) {
	    this->fontID = fontID;
	    resize(getSize().x, getSize().y);
	}

    /**
		Gets the font of this text box. Default font of a text box is FONT_STD12
		\return the font ID of this text box
	*/
	virtual inline int getTextFont() const {
	   return fontID;
	}

    /**
		Sets the text color for this text box.
		\param	textcolor	    the color of the text (COLOR_DEFAULT = default color)
        \param	textshadowcolor	the color of the shadow of the text (COLOR_DEFAULT = default color)
	*/
	virtual inline void setTextColor(Uint32 textcolor, Uint32 textshadowcolor = COLOR_DEFAULT) {
		this->textcolor = textcolor;
		this->textshadowcolor = textshadowcolor;
		resize(getSize().x, getSize().y);
	}

	/**
        Sets the maximum length of the typed text
        \param  maxTextLength   the maximum length, -1 = unlimited
	*/
	virtual inline void setMaximumTextLength(int maxTextLength) {
        this->maxTextLength = maxTextLength;
	}


	/**
        Sets the set of allowed characters for this text box.
        \param  allowedChars    the set of allowed chars or an empty string if everything is allowed
	*/
	virtual inline void setAllowedChars(std::string allowedChars = "") {
        this->allowedChars = allowedChars;
	}

	/**
		Sets the function that should be called when the text of this text box changes.
		\param	pOnTextChange	A function to call on text change
	*/
	inline void setOnTextChange(std::function<void (bool)> pOnTextChange) {
		this->pOnTextChange = pOnTextChange;
	}


	/**
		Sets the method that should be called when return is pressed
		\param	pOnReturn	A function to call on pressing return
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
	virtual Point getMinimumSize() const {
		return GUIStyle::getInstance().getMinimumTextBoxSize(fontID);
	}

	/**
		This method resized the text box to width and height. This method should only be
		called if the new size is a valid size for this text box (See resizingXAllowed,
		resizingYAllowed, getMinumumSize).
		\param	width	the new width of this text box
		\param	height	the new height of this text box
	*/
	virtual void resize(Uint32 width, Uint32 height) {
		Widget::resize(width,height);
		updateSurfaces();
	}

	/**
		This method updates all surfaces for this text box. This method will be called
		if this text box is resized or the text changes.
	*/
	virtual void updateSurfaces() {
		if(pTextureWithoutCarret != NULL) {
			SDL_DestroyTexture(pTextureWithoutCarret);
			pTextureWithoutCarret = NULL;
		}

		if(pTextureWithCarret != NULL) {
			SDL_DestroyTexture(pTextureWithCarret);
			pTextureWithCarret = NULL;
		}

		pTextureWithoutCarret = convertSurfaceToTexture(GUIStyle::getInstance().createTextBoxSurface(getSize().x, getSize().y, text, false, fontID,  Alignment_Left, textcolor, textshadowcolor), true);
		pTextureWithCarret = convertSurfaceToTexture(GUIStyle::getInstance().createTextBoxSurface(getSize().x, getSize().y, text, true, fontID, Alignment_Left, textcolor, textshadowcolor), true);
	}

	/**
		Draws this text box to screen.
		\param	screen	Surface to draw on
		\param	Position	Position to draw the text box to
	*/
	virtual void draw(SDL_Surface* screen, Point position) {
		if((isVisible() == false) || (pTextureWithoutCarret == NULL) || (pTextureWithCarret == NULL)) {
			return;
		}

		SDL_Rect dest = calcDrawingRect(pTextureWithoutCarret, position.x, position.y);

		if(isActive()) {
			if((SDL_GetTicks() - lastCarretTime) < 500) {
				SDL_RenderCopy(renderer, pTextureWithCarret, NULL, &dest);
			} else {
				SDL_RenderCopy(renderer, pTextureWithoutCarret, NULL, &dest);
			}

			if(SDL_GetTicks() - lastCarretTime >= 1000) {
				lastCarretTime = SDL_GetTicks();
			}
		} else {
			SDL_RenderCopy(renderer, pTextureWithoutCarret, NULL, &dest);
		}
	}

	/**
		Handles a left mouse click.
		\param	x x-coordinate (relative to the left top corner of the text box)
		\param	y y-coordinate (relative to the left top corner of the text box)
		\param	pressed	true = mouse button pressed, false = mouse button released
		\return	true = click was processed by the widget, false = click was not processed by the text box
	*/
	virtual bool handleMouseLeft(Sint32 x, Sint32 y, bool pressed) {
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
		\param	key the key that was pressed or released.
		\return	true = key stroke was processed by the text box, false = key stroke was not processed by the text box
	*/
	virtual bool handleKeyPress(SDL_KeyboardEvent& key) {
		if((isVisible() == false) || (isEnabled() == false) || (isActive() == false)) {
			return true;
		}

		if(key.keysym.sym == SDLK_TAB) {
			setInactive();
			return true;
		}

		if(key.keysym.sym == SDLK_BACKSPACE) {
			if(text.size() != 0) {
				text.erase(text.size() - 1);
				if(pOnTextChange) {
					pOnTextChange(true);
				}
			}
		} else if(key.keysym.sym == SDLK_RETURN) {
			if(pOnReturn) {
				pOnReturn();
			}
		} else if((key.keysym.sym <= 0xFF) && (key.keysym.sym != 0) && ((maxTextLength < 0) || ((int) text.length() < maxTextLength))) {
		    char newChar = (char) key.keysym.sym;

		    if((key.keysym.mod & KMOD_LSHIFT) || (key.keysym.mod & KMOD_RSHIFT)) {
                newChar = std::toupper(newChar);
            }

		    if(allowedChars.empty() || allowedChars.find(newChar) != std::string::npos) {
                text += newChar;

                if(pOnTextChange) {
                    pOnTextChange(true);
                }
		    }
		}

		updateSurfaces();

		return true;
	}

protected:
	/**
		This method sets a new text for this text box.
		\param	text            The new text for this text box
		\param  bInteractive    Was this text change initiated by the user?
	*/
	virtual void setText(std::string text, bool bInteractive) {
	    bool bChanged = (text != this->text);
		this->text = text;
		updateSurfaces();
		if(bChanged && pOnTextChange) {
            pOnTextChange(bInteractive);
		}
	}

private:
    int fontID;                                 ///< the ID of the font to use
    Uint32 textcolor;                           ///< Text color
    Uint32 textshadowcolor;                     ///< Text shadow color
	std::string text;                           ///< text in this text box
	int maxTextLength;                          ///< the maximum length of the typed text

	std::string allowedChars;                   ///< a set of allowed characters, empty string for everything allowed

	Uint32 lastCarretTime;                      ///< Last time the carret changes from off to on or vise versa

	std::function<void (bool)> pOnTextChange;	///< function that is called when the text of this text box changes
	std::function<void ()> pOnReturn;		    ///< function that is called when return is pressed

	SDL_Texture* pTextureWithoutCarret;		    ///< Texture with carret off
	SDL_Texture* pTextureWithCarret;		    ///< Texture with carret on
};

#endif // TEXTBOX_H
