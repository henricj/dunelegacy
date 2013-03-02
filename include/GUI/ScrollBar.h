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

#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include "Widget.h"
#include "PictureButton.h"
#include "TextButton.h"

#include <misc/functional.h>

/// A class for a scroll bar
class ScrollBar : public Widget {
public:
	/// default constructor
	ScrollBar();

	/// destructor
	virtual ~ScrollBar();

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
		Point tmp = GUIStyle::getInstance().getMinimumScrollBarArrowButtonSize();
		tmp.y = tmp.y * 3;
		return tmp;
	}

	/**
		Sets the range of this ScrollBar.
		\param	minValue	the minimum value
		\param	MaxValue	the maximum value
	*/
	void setRange(int minValue, int maxValue) {
		if(minValue > maxValue) {
			return;
		}

		this->minValue = minValue;
		this->maxValue = maxValue;

		if(currentValue < minValue) {
			currentValue = minValue;
		} else if(currentValue > maxValue) {
			currentValue = maxValue;
		}

		updateSliderButton();
	}

    /**
		Gets the range start of this ScrollBar.
		\return the start value of the range
	*/
	int getRangeMin() const { return minValue; }

    /**
		Gets the range end of this ScrollBar.
		\return the end value of the range
	*/
	int getRangeMax() const { return maxValue; }

	/**
		Sets the big step size that is used when clicking between the arrows and the slider.
		This value is also used to determine the size of the slider.
		\param	stepSize	the new step size to advance scollbar when clicking inbetween
	*/
	void setBigStepSize(int stepSize) {
		bigStepSize = stepSize;
		updateSliderButton();
	}

	/**
		Returns the current position
		\return	the current value
	*/
	int getCurrentValue() {
		return currentValue;
	}

	/**
		Sets the current position. Should be in range
		\param newValue the new position
	*/
	void setCurrentValue(int newValue) {
		currentValue = newValue;
		if(currentValue < minValue) {
			currentValue = minValue;
		} else if(currentValue > maxValue) {
			currentValue = maxValue;
		}

		updateSliderButton();

		if(pOnChange) {
			pOnChange();
		}
	}

	/**
		Sets the function that should be called when this scroll bar changes its position.
		\param	pOnChange	A function to be called on change
	*/
	inline void setOnChange(std::function<void ()> pOnChange) {
		this->pOnChange = pOnChange;
	}

    /**
		Sets the color for this scrollbar.
		\param	color   the color (-1 = default color)
	*/
	virtual inline void setColor(int color) {
		this->color = color;
		updateArrowButtonSurface();
	}

private:
	void updateSliderButton();

	void updateArrowButtonSurface();

	void onArrow1() {
		setCurrentValue(currentValue-1);
	}

	void onArrow2() {
		setCurrentValue(currentValue+1);
	}

	SDL_Surface* pBackground;
	PictureButton arrow1;
	PictureButton arrow2;
	TextButton sliderButton;

	std::function<void ()> pOnChange;		///< function that is called when this scrollbar changes its position

	int currentValue;
	int minValue;
	int maxValue;
	int bigStepSize;						///< the step size when clicking between the arrows and the slider
	Point sliderPosition;

	bool bDragSlider;
	int dragPositionFromSliderTop;

    int color;				                ///< the color
};

#endif // SCROLLBAR_H
