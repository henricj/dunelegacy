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

#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include "Widget.h"
#include "ScrollBar.h"

#include <SDL.h>
#include <vector>
#include <string>

/// A class for a multiline text view widget
class TextView : public Widget {
public:
	/// default constructor
	TextView();

	/// destructor
	virtual ~TextView();

    /**
		Sets a font for this label. Default font of a label is FONT_STD12
		\param	fontID	    the ID of the new font
	*/
	virtual inline void setTextFont(int fontID) {
	    this->fontID = fontID;
	    resize(getSize().x, getSize().y);
	}

    /**
		Gets the font of this label. Default font of a label is FONT_STD12
		\return the font ID of this label
	*/
	virtual inline int getTextFont() const {
	   return fontID;
	}

	/**
		Sets the text color for this label.
		\param	textcolor	    the color of the text (-1 = default color)
        \param	textshadowcolor	the color of the shadow of the text (-1 = default color)
        \param  backgroundcolor the color of the label background (0 = transparent)
	*/
	virtual inline void setTextColor(int textcolor, int textshadowcolor = -1, int backgroundcolor = 0) {
		this->textcolor = textcolor;
		this->textshadowcolor = textshadowcolor;
		this->backgroundcolor = backgroundcolor;
		resize(getSize().x, getSize().y);
	}

	/**
		Sets the alignment of the text in this label.
		\param alignment Combination of (Alignment_HCenter, Alignment_Left or Alignment_Right) and (Alignment_VCenter, Alignment_Top or Alignment_Bottom)
	*/
	virtual inline void setAlignment(Alignment_Enum alignment) {
		this->alignment = alignment;
		resize(getSize().x, getSize().y);
	}

	/**
		Returns the alignment of the text in this label.
		\return Combination of (Alignment_HCenter, Alignment_Left or Alignment_Right) and (Alignment_VCenter, Alignment_Top or Alignment_Bottom)
	*/
	virtual inline Alignment_Enum getAlignment() const {
		return alignment;
	}

	/**
		This method sets a new text for this label and resizes this label
		to fit this text.
		\param	text The new text for this button
	*/
	virtual inline void setText(std::string text) {
		this->text = text;
		resizeAll();
	}

	/**
		Get the text of this label.
		\return the text of this button
	*/
	inline std::string getText() const { return text; };

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
		Handles mouse wheel scrolling.
		\param	x x-coordinate (relative to the left top corner of the widget)
		\param	y y-coordinate (relative to the left top corner of the widget)
		\param	up	true = mouse wheel up, false = mouse wheel down
		\return	true = the mouse wheel scrolling was processed by the widget, false = mouse wheel scrolling was not processed by the widget
	*/
	virtual bool handleMouseWheel(Sint32 x, Sint32 y, bool up);

	/**
		Handles a key stroke. This method is neccessary for controlling an application
		without a mouse.
		\param	key the key that was pressed or released.
		\return	true = key stroke was processed by the widget, false = key stroke was not processed by the widget
	*/
	virtual bool handleKeyPress(SDL_KeyboardEvent& key);

	/**
		Draws this scroll bar to screen. This method is called before drawOverlay().
		\param	screen	Surface to draw on
		\param	position	Position to draw the scroll bar to
	*/
	virtual void draw(SDL_Surface* screen, Point position);

	/**
		This method resized the scroll bar to width and height. This method should only
		called if the new size is a valid size for this scroll bar (See getMinumumSize).
		\param	width	the new width of this scroll bar
		\param	height	the new height of this scroll bar
	*/
	virtual void resize(Uint32 width, Uint32 height);

	/**
		Returns the minimum size of this scroll bar. The scroll bar should not
		resized to a size smaller than this.
		\return the minimum size of this scroll bar
	*/
	virtual Point getMinimumSize() const {
		Point tmp = scrollbar.getMinimumSize();
		tmp.x += 30;
		return tmp;
	}

	/**
		Returns whether this widget can be set active.
		\return	true = activatable, false = not activatable
	*/
	virtual inline bool isActivatable() const { return isEnabled(); };

    /**
		Is the scrollbar always shown or is it hidden if not needed
		\return	true if scrollbar is hidden if not needed
	*/
	bool getAutohideScrollbar() const { return bAutohideScrollbar; }

    /**
		Set if the scrollbar shall be hidden if not needed
		\param  bAutohideScrollbar	true = hide scrollbar, false = show always
	*/
    void setAutohideScrollbar(bool bAutohideScrollbar) {
        this->bAutohideScrollbar = bAutohideScrollbar;
    }

	/**
		Scrolls the text box to the start.
	*/
	void scrollToStart() {
		scrollbar.setCurrentValue(0);
	}

	/**
		Scrolls the text box to the last line.
	*/
	void scrollToEnd() {
		scrollbar.setCurrentValue(scrollbar.getRangeMax());
	}

private:
    int fontID;                 ///< the ID of the font to use
	int textcolor;				///< the text color
	int textshadowcolor;        ///< the color of the shadow of the text
	int backgroundcolor;        ///< the color of the label background
	std::string text;			///< the text of this label

	Alignment_Enum alignment;	///< the alignment of this label

	SDL_Surface* pBackground;
	SDL_Surface* pForeground;
	ScrollBar scrollbar;

    bool bAutohideScrollbar;    ///< hide the scrollbar if not needed (default = true)
};

#endif //TEXTVIEW_H
