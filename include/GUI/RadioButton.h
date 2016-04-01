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
        pCheckedActiveTexture = NULL;
        bFreeCheckedActiveTexture = false;

		pRadioButtonManager = NULL;
	}

	/// destructor
	virtual ~RadioButton() {
        invalidateTextures();

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
		invalidateTextures();
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
		\param	position	Position to draw the button to
	*/
	virtual void draw(Point position) {
        if(isVisible() == false) {
            return;
        }

        updateTextures();

        SDL_Texture* tex;
        if(isChecked()) {
            if((isActive() || bHover) && pCheckedActiveTexture != NULL) {
                tex = pCheckedActiveTexture;
            } else {
                tex = pPressedTexture;
            }
        } else {
            if((isActive() || bHover) && pActiveTexture != NULL) {
                tex = pActiveTexture;
            } else {
                tex = pUnpressedTexture;
            }
        }

        if(tex == NULL) {
            return;
        }

        SDL_Rect dest = calcDrawingRect(tex, position.x, position.y);
        SDL_RenderCopy(renderer, tex, NULL, &dest);
    }

    /**
		This method resizes the radio button. This method should only
		called if the new size is a valid size for this radio button (See getMinumumSize).
		\param	newSize	the new size of this progress bar
	*/
	virtual void resize(Point newSize) {
		resize(newSize.x,newSize.y);
	}

	/**
		This method resizes the radio button to width and height. This method should only
		called if the new size is a valid size for this radio button (See getMinimumSize).
		\param	width	the new width of this radio button
		\param	height	the new height of this radio button
	*/
	virtual void resize(Uint32 width, Uint32 height) {
        invalidateTextures();
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
protected:
	/**
        This method is called whenever the textures of this widget are needed, e.g. before drawing. This method
        should be overwritten by subclasses if they like to defer texture creation as long as possible.
        This method should first check whether a renewal of the textures is necessary.
	*/
	virtual void updateTextures() {
        Button::updateTextures();

        if(pUnpressedTexture == NULL) {
            invalidateTextures();

            setSurfaces(GUIStyle::getInstance().createRadioButtonSurface(getSize().x, getSize().y, text, false, false, textcolor, textshadowcolor),true,
                        GUIStyle::getInstance().createRadioButtonSurface(getSize().x, getSize().y, text, true, false, textcolor, textshadowcolor),true,
                        GUIStyle::getInstance().createRadioButtonSurface(getSize().x, getSize().y, text, false, true, textcolor, textshadowcolor),true);

            pCheckedActiveTexture = convertSurfaceToTexture(GUIStyle::getInstance().createRadioButtonSurface(getSize().x, getSize().y, text, true, true, textcolor, textshadowcolor), true);
            bFreeCheckedActiveTexture = true;
        }
	}

	/**
		This method frees all textures that are used by this radio button
	*/
	virtual void invalidateTextures() {
        Button::invalidateTextures();

        if((bFreeCheckedActiveTexture == true) && (pCheckedActiveTexture != NULL)) {
            SDL_DestroyTexture(pCheckedActiveTexture);
            bFreeCheckedActiveTexture = false;
        }
        pCheckedActiveTexture = NULL;
	}

private:
    Uint32 textcolor;                       ///< Text color
    Uint32 textshadowcolor;                 ///< Text shadow color
	std::string text;		                ///< Text of this radio button
	SDL_Texture* pCheckedActiveTexture;		///< Texture that is shown when the radio button is activated by keyboard or by mouse hover
	bool bFreeCheckedActiveTexture;			///< Should pActiveSurface be freed if this button is destroyed?

	RadioButtonManager*	pRadioButtonManager;///< The Manager for managing the toggle states
};

#endif // RADIOBUTTON_H
