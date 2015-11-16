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
class Checkbox : public Button {
public:
	/// Default constructor
	Checkbox() : Button() {
        textcolor = -1;
	    textshadowcolor = -1;

		enableResizing(true,false);
		setToggleButton(true);
        pCheckedActiveSurface = NULL;
        bFreeCheckedActiveSurface = false;
	}

	/// destructor
	virtual ~Checkbox() {
        if((bFreeCheckedActiveSurface == true) && (pCheckedActiveSurface != NULL)) {
            SDL_FreeSurface(pCheckedActiveSurface);
            pCheckedActiveSurface = NULL;
            bFreeCheckedActiveSurface = false;
        }
	}

	/**
		This method sets a new text for this checkbox and resizes it
		to fit this text.
		\param	Text The new text for this checkbox
	*/
	virtual inline void setText(std::string text) {
		this->text = text;
		resizeAll();
	}

	/**
		Get the text of this checkbox.
		\return the text of this checkbox
	*/
	inline std::string getText() { return text; };

    /**
		Sets the text color for this checkbox.
		\param	textcolor	    the color of the text (-1 = default color)
        \param	textshadowcolor	the color of the shadow of the text (-1 = default color)
	*/
	virtual inline void setTextColor(int textcolor, int textshadowcolor = -1) {
		this->textcolor = textcolor;
		this->textshadowcolor = textshadowcolor;
		resize(getSize().x, getSize().y);
	}

	/**
		This method sets this checkbox to checked or unchecked. It does the same as setToggleState().
		\param bChecked	true = checked, false = unchecked
	*/
	inline void setChecked(bool bChecked) {
		setToggleState(bChecked);
	}

	/**
		This method returns whether this checkbox is checked. It is the same as getToggleState().
		\return	true = checked, false = unchecked
	*/
	inline bool isChecked() {
		return getToggleState();
	}

	/**
		Draws this button to screen. This method is called before drawOverlay().
		\param	screen	Surface to draw on
		\param	position	Position to draw the button to
	*/
	virtual void draw(SDL_Surface* screen, Point position) {
        if(isVisible() == false) {
            return;
        }

        SDL_Surface* surf;
        if(isChecked()) {
            if((isActive() || bHover) && pCheckedActiveSurface != NULL) {
                surf = pCheckedActiveSurface;
            } else {
                surf = pPressedSurface;
            }
        } else {
            if((isActive() || bHover) && pActiveSurface != NULL) {
                surf = pActiveSurface;
            } else {
                surf = pUnpressedSurface;
            }
        }

        if(surf == NULL) {
            return;
        }

        SDL_Rect dest = { static_cast<Sint16>(position.x), static_cast<Sint16>(position.y), static_cast<Uint16>(surf->w), static_cast<Uint16>(surf->h) };
        SDL_BlitSurface(surf,NULL,screen,&dest);
    }

	/**
		This method resized the checkbox to width and height. This method should only
		called if the new size is a valid size for this checkbox (See getMinimumSize).
		\param	width	the new width of this checkbox
		\param	height	the new height of this checkbox
	*/
	virtual void resize(Uint32 width, Uint32 height) {
		setSurfaces(GUIStyle::getInstance().createCheckboxSurface(width, height, text, false, false, textcolor, textshadowcolor),true,
                    GUIStyle::getInstance().createCheckboxSurface(width, height, text, true, false, textcolor, textshadowcolor),true,
                    GUIStyle::getInstance().createCheckboxSurface(width, height, text, false, true, textcolor, textshadowcolor),true);

        if((bFreeCheckedActiveSurface == true) && (pCheckedActiveSurface != NULL)) {
            SDL_FreeSurface(pCheckedActiveSurface);
            pCheckedActiveSurface = NULL;
            bFreeCheckedActiveSurface = false;
        }

        pCheckedActiveSurface = GUIStyle::getInstance().createCheckboxSurface(width, height, text, true, true, textcolor, textshadowcolor);
        bFreeCheckedActiveSurface = true;

		Widget::resize(width,height);
	}

	/**
		Returns the minimum size of this button. The button should not
		resized to a size smaller than this.
		\return the minimum size of this button
	*/
	virtual Point getMinimumSize() const {
		return GUIStyle::getInstance().getMinimumCheckboxSize(text);
	}

private:
    int textcolor;                          ///< Text color
    int textshadowcolor;                    ///< Text shadow color
	std::string text;		                ///< Text of this checkbox
	SDL_Surface* pCheckedActiveSurface;		///< Surface that is shown when the checkbox is activated by keyboard or by mouse hover
	bool bFreeCheckedActiveSurface;			///< Should pActiveSurface be freed if this button is destroyed?
};

#endif // CHECKBOX_H
