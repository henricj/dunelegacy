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

#ifndef RADIOBUTTON_H
#define RADIOBUTTON_H

#include "Button.h"
#include "GUIStyle.h"

#include "RadioButtonManager.h"

#include <string>

/// A class for a radio button implemented as a toggle button
class RadioButton : public Button {
public:
	/// Default constructor
	RadioButton() : Button() {
        textcolor = COLOR_DEFAULT;
	    textshadowcolor = COLOR_DEFAULT;

		enableResizing(true,false);
		setToggleButton(true);
        pCheckedActiveSurface = NULL;
        bFreeCheckedActiveSurface = false;

		pRadioButtonManager = NULL;
	}

	/// destructor
	virtual ~RadioButton() {
        if((bFreeCheckedActiveSurface == true) && (pCheckedActiveSurface != NULL)) {
            SDL_FreeSurface(pCheckedActiveSurface);
            pCheckedActiveSurface = NULL;
            bFreeCheckedActiveSurface = false;
        }

		unregisterFromRadioButtonManager();
	}


	void registerRadioButtonManager(RadioButtonManager* pNewRadioButtonManager) {
		if(pNewRadioButtonManager != pRadioButtonManager) {
			unregisterFromRadioButtonManager();
			pRadioButtonManager = pNewRadioButtonManager;
			if(pRadioButtonManager->isRegistered(this) == false) {
				pRadioButtonManager->registerRadioButton(this);
			}
		}
	}

	void unregisterFromRadioButtonManager() {
		if(pRadioButtonManager != NULL) {
			RadioButtonManager* pOldRadioButtonManager = pRadioButtonManager;
			pRadioButtonManager = NULL;
			if(pOldRadioButtonManager->isRegistered(this)) {
				pOldRadioButtonManager->unregisterRadioButton(this);
			}
		}
	}

	/**
		This method sets a new text for this radio button and resizes it
		to fit this text.
		\param	text The new text for this radio button
	*/
	virtual inline void setText(std::string text) {
		this->text = text;
		resizeAll();
	}

	/**
		Get the text of this radio button.
		\return the text of this radio button
	*/
	inline std::string getText() { return text; };

    /**
		Sets the text color for this radio button.
		\param	textcolor	    the color of the text (COLOR_DEFAULT = default color)
        \param	textshadowcolor	the color of the shadow of the text (COLOR_DEFAULT = default color)
	*/
	virtual inline void setTextColor(Uint32 textcolor, Uint32 textshadowcolor = COLOR_DEFAULT) {
		this->textcolor = textcolor;
		this->textshadowcolor = textshadowcolor;
		resize(getSize().x, getSize().y);
	}


	/**
		This method sets the current toggle state. On radio buttons this is only
		effective for bToggleState == true, so you can only set a radio button to be selected
		but cannot deselect it
		\param bToggleState	true = toggled, false = untoggled
	*/
	virtual void setToggleState(bool bToggleState) {
		if(bToggleState != true) {
			return;
		}

		if(bToggleState != getToggleState()) {
			if(pRadioButtonManager != NULL) {
				pRadioButtonManager->setChecked(this);
			}
		}
	}

	/**
		This method sets this radio button to checked or unchecked. It does the same as setToggleState().
		\param bChecked	true = checked, false = unchecked
	*/
	inline void setChecked(bool bChecked) {
		setToggleState(bChecked);
	}

	/**
		This method returns whether this radio button is checked. It is the same as getToggleState().
		\return	true = checked, false = unchecked
	*/
	inline bool isChecked() const {
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
		This method resized the radio button to width and height. This method should only
		called if the new size is a valid size for this checkbox (See getMinimumSize).
		\param	width	the new width of this radio button
		\param	height	the new height of this radio button
	*/
	virtual void resize(Uint32 width, Uint32 height) {
		setSurfaces(GUIStyle::getInstance().createRadioButtonSurface(width, height, text, false, false, textcolor, textshadowcolor),true,
                    GUIStyle::getInstance().createRadioButtonSurface(width, height, text, true, false, textcolor, textshadowcolor),true,
                    GUIStyle::getInstance().createRadioButtonSurface(width, height, text, false, true, textcolor, textshadowcolor),true);

        if((bFreeCheckedActiveSurface == true) && (pCheckedActiveSurface != NULL)) {
            SDL_FreeSurface(pCheckedActiveSurface);
            pCheckedActiveSurface = NULL;
            bFreeCheckedActiveSurface = false;
        }

        pCheckedActiveSurface = GUIStyle::getInstance().createRadioButtonSurface(width, height, text, true, true, textcolor, textshadowcolor);
        bFreeCheckedActiveSurface = true;

		Widget::resize(width,height);
	}

	/**
		Returns the minimum size of this button. The button should not
		resized to a size smaller than this.
		\return the minimum size of this button
	*/
	virtual Point getMinimumSize() const {
		return GUIStyle::getInstance().getMinimumRadioButtonSize(text);
	}

private:
    Uint32 textcolor;                       ///< Text color
    Uint32 textshadowcolor;                 ///< Text shadow color
	std::string text;		                ///< Text of this radio button
	SDL_Surface* pCheckedActiveSurface;		///< Surface that is shown when the radio button is activated by keyboard or by mouse hover
	bool bFreeCheckedActiveSurface;			///< Should pActiveSurface be freed if this button is destroyed?

	RadioButtonManager*	pRadioButtonManager;///< The Manager for managing the toggle states
};

#endif // RADIOBUTTON_H
