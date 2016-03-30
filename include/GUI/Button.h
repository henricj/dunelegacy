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

#ifndef BUTTON_H
#define BUTTON_H

#include "Widget.h"
#include "GUIStyle.h"
#include "misc/draw_util.h"
#include <SDL.h>

#include <string>
#include <functional>

/// A abstract base class for all buttons
class Button : public Widget {
public:
	/// Default contructor
	Button();

	/// desctructor
	virtual ~Button();

	/**
		Returns that this button can be set active.
		\return	true(= activatable)
	*/
	virtual inline bool isActivatable() const { return isEnabled(); };

	/**
		Enable or disable this button. A disabled button is not responding
		to clicks and key strokes and might look different.
		\param	bEnabled	true = enable button, false = disable button
	*/
	virtual inline void setEnabled(bool bEnabled) {
		Widget::setEnabled(bEnabled);
		if(bEnabled == false) {
			bPressed = false;
		}
	};


	/**
		Sets a tooltip text. This text is shown when the mouse remains a short time over this button.
		\param	text	The text for this tooltip
	*/
	inline void setTooltipText(std::string text) {
		tooltipText = text;

		if(tooltipTexture != NULL) {
			SDL_DestroyTexture(tooltipTexture);
			tooltipTexture = NULL;
		}

		if(tooltipText != "") {
			tooltipTexture = convertSurfaceToTexture(GUIStyle::getInstance().createToolTip(tooltipText), true);
		}
	}

	/**
		Returns the current tooltip text.
		\return	the current tooltip text
	*/
	inline std::string getTooltipText() {
		return tooltipText;
	}

	/**
		Sets the function that should be called when this button is clicked on.
		\param	pOnClick	A function to call on click
	*/
	inline void setOnClick(std::function<void ()> pOnClick) {
		this->pOnClick = pOnClick;
	}

	/**
		Sets whether this button is a toggle button.
		\param bToggleButton	true = toggle button, false = normal button
	*/
	inline void setToggleButton(bool bToggleButton) {
		this->bToggleButton = bToggleButton;
	}

	/**
		Returns whether this button is a toggle button
		\return true = toggle button, false = normal button
	*/
	inline bool isToggleButton() {
		return bToggleButton;
	}

	/**
		This method sets the current toggle state. If this button is no
		toggle button this method has no effect.
		\param bToggleState	true = toggled, false = untoggled
	*/
	virtual void setToggleState(bool bToggleState) {
		if(isToggleButton()) {
			this->bToggleState = bToggleState;
		}
	}

	/**
		This method returns whether this button is currently toggled or not.
		\return	true = toggled, false = untoggled
	*/
	inline bool getToggleState() const {
		return bToggleState;
	}

	/**
		Handles a mouse movement. This method is for example needed for the tooltip.
		\param	x               x-coordinate (relative to the left top corner of the widget)
		\param	y               y-coordinate (relative to the left top corner of the widget)
		\param  insideOverlay   true, if (x,y) is inside an overlay and this widget may be behind it, false otherwise
	*/
	virtual void handleMouseMovement(Sint32 x, Sint32 y, bool insideOverlay);

	/**
		Handles a left mouse click.
		\param	x x-coordinate (relative to the left top corner of the widget)
		\param	y y-coordinate (relative to the left top corner of the widget)
		\param	pressed	true = mouse button pressed, false = mouse button released
		\return	true = click was processed by the widget, false = click was not processed by the widget
	*/
	virtual bool handleMouseLeft(Sint32 x, Sint32 y, bool pressed);

	/**
		Handles a key stroke. This method is neccessary for controlling an application
		without a mouse.
		\param	key the key that was pressed or released.
		\return	true = key stroke was processed by the widget, false = key stroke was not processed by the widget
	*/
	virtual bool handleKeyPress(SDL_KeyboardEvent& key);

	/**
		Draws this button to screen. This method is called before drawOverlay().
		\param	screen	Surface to draw on
		\param	position	Position to draw the button to
	*/
	virtual void draw(SDL_Surface* screen, Point position);

	/**
		This method draws the tooltip for this button. This method is called after draw().
		\param	screen	Surface to draw on
		\param	position	Position to draw the widget to
	*/
	virtual void drawOverlay(SDL_Surface* screen, Point position);

protected:
	/**
		This method is used for setting the different surfaces for this button.
		\param	pUnpressedSurface		This surface is normally shown
		\param	bFreeUnpressedSurface	Should pUnpressedSurface be freed if this button is destroyed?
		\param	pPressedSurface			This surface is shown when the button is pressed
		\param	bFreePressedSurface		Should pPressedSurface be freed if this button is destroyed?
		\param	pActiveSurface			This surface is shown when the button is activated by keyboard or by mouse hover
		\param	bFreeActiveSurface		Should pActiveSurface be freed if this button is destroyed?
	*/
	virtual void setSurfaces(	SDL_Surface* pUnpressedSurface,bool bFreeUnpressedSurface,
								SDL_Surface* pPressedSurface,bool bFreePressedSurface,
								SDL_Surface* pActiveSurface = NULL,bool bFreeActiveSurface = false);

	/**
		This method is used for setting the different textures for this button.
		\param	pUnpressedTexture		This texture is normally shown
		\param	bFreeUnpressedTexture	Should pUnpressedTexture be freed if this button is destroyed?
		\param	pPressedTexture			This texture is shown when the button is pressed
		\param	bFreePressedTexture		Should pPressedTexture be freed if this button is destroyed?
		\param	pActiveTexture			This texture is shown when the button is activated by keyboard or by mouse hover
		\param	bFreeActiveTexture		Should pActiveTexture be freed if this button is destroyed?
	*/
	virtual void setTextures(	SDL_Texture* pUnpressedTexture,bool bFreeUnpressedTexture,
								SDL_Texture* pPressedTexture,bool bFreePressedTexture,
								SDL_Texture* pActiveTexture = NULL,bool bFreeActiveTexture = false);

	SDL_Texture* pUnpressedTexture;		///< Texture that is normally shown
	SDL_Texture* pPressedTexture;		///< Texture that is shown when the button is pressed
	SDL_Texture* pActiveTexture;		///< Texture that is shown when the button is activated by keyboard or by mouse hover
	bool bFreeUnpressedTexture;			///< Should pUnpressedTexture be freed if this button is destroyed?
	bool bFreePressedTexture;			///< Should pPressedTexture be freed if this button is destroyed?
	bool bFreeActiveTexture;			///< Should pActiveTexture be freed if this button is destroyed?

	/**
		This method frees all textures that should be freed by this button
	*/
	void freeTextures();

	std::string tooltipText;			///< the tooltip text
	SDL_Texture* tooltipTexture;		///< the tooltip texture
	Uint32 tooltipLastMouseMotion;		///< the last time the mouse was moved

	std::function<void ()> pOnClick;	///< function that is called when this button is clicked
	bool bPressed;						///< true = currently pressed, false = currently unpressed
	bool bHover;						///< true = currently mouse hover, false = currently no mouse hover
	bool bToggleButton;					///< true = toggle button, false = normal button
	bool bToggleState;					///< true = currently toggled, false = currently not toggled
};


#endif //BUTTON_H
