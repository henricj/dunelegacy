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

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "Widget.h"

#include <string>
#include <SDL.h>

#include <cmath>


/// A class for a progress bar widget
class ProgressBar : public Widget {
public:

	/// default constructor
	ProgressBar() : Widget() {
		percent = 0.0;
		color = COLOR_DEFAULT;
		bDrawShadow = false;
		pBackground = NULL;
		pForeground = NULL;
		bFreeBackground = true;
		enableResizing(true,true);
	}

	/// destructor
	virtual ~ProgressBar() {
		if((bFreeBackground == true) && (pBackground != NULL)) {
			SDL_FreeSurface(pBackground);
		}

		if(pForeground != NULL) {
			SDL_FreeSurface(pForeground);
		}
	}

	/**
		Sets the progress of this progress bar.
		\param percent	Should be between 0.0 and 100.0
	*/
	void setProgress(double percent) {
		if(percent != this->percent) {
			this->percent = percent;
			if(percent < 0.0) {
				percent = 0.0;
			} else if(percent > 100.0) {
				percent = 100.0;
			}

			if(pForeground != NULL) {
				SDL_FreeSurface(pForeground);
				pForeground = NULL;
			}

			pForeground = GUIStyle::getInstance().createProgressBarOverlay(getSize().x, getSize().y, percent, color);
		}
	}

	/**
		Return the current progress.
		\return	the current progress in percent
	*/
	double getProgress() {
		return percent;
	}

	/**
		Sets the color of the progress bar overlay
		\param	color	the new color (COLOR_DEFAULT = default)
	*/
	inline void setColor(Uint32 color = COLOR_DEFAULT) {
		this->color = color;
		resizeAll();
	}

	/**
        Specifies if a shadow is drawn or not.
        \param  bDrawShadow if true, a shadow is drawn
	*/
	inline void setDrawShadow(bool bDrawShadow) {
        this->bDrawShadow = bDrawShadow;
	}

	/**
		This method resized the progress bar to width and height. This method should only
		called if the new size is a valid size for this progress bar (See getMinumumSize).
		\param	width	the new width of this progress bar
		\param	height	the new height of this progress bar
	*/
	virtual void resize(Uint32 width, Uint32 height) {
		Widget::resize(width,height);

        if(pForeground != NULL) {
            SDL_FreeSurface(pForeground);
            pForeground = NULL;
        }

        pForeground = GUIStyle::getInstance().createProgressBarOverlay(getSize().x, getSize().y, percent, color);
	}

	/**
		Draws this progress bar to screen
		\param	screen	Surface to draw on
		\param	position	Position to draw the progress bar to
	*/
	virtual void draw(SDL_Surface* screen, Point position) {
		if(isVisible() == false) {
			return;
		}

		if(pBackground != NULL) {
            SDL_Rect dest = calcDrawingRect(pBackground, position.x, position.y);
            SDL_BlitSurface(pBackground,NULL,screen,&dest);
		}

		if(pForeground != NULL) {
            SDL_Rect dest = calcDrawingRect(pForeground, position.x, position.y);
		    if(bDrawShadow) {
		        SDL_Rect dest2 = { position.x + 2, position.y + 2, lround(percent*(dest.w/100.0)), dest.h };
                SDL_FillRect(screen, &dest2, COLOR_BLACK);
		    }

			SDL_BlitSurface(pForeground,NULL,screen,&dest);
		}
	}

protected:
	SDL_Surface*	pBackground;
	bool			bFreeBackground;
	SDL_Surface*	pForeground;

	double percent;				///< Percent from 0.0 to 100.0
	Uint32 color;			    ///< The color of the progress overlay
	bool bDrawShadow;           ///< Draw shadow under the foreground surface
};

class TextProgressBar : public ProgressBar {
public:
	TextProgressBar() : ProgressBar() {
		text = "";
		bFreeBackground = true;
        textcolor = COLOR_DEFAULT;
	    textshadowcolor = COLOR_DEFAULT;
	}

	virtual ~TextProgressBar() { ; };

	/**
		This method sets a new text for this progress bar and resizes it
		to fit this text.
		\param	text The new text for this progress bar
	*/
	virtual inline void setText(std::string text) {
		if(this->text != text) {
			this->text = text;
			resizeAll();
		}
	}

	/**
		Get the text of this progress bar.
		\return the text of this button
	*/
	inline std::string getText() { return text; };

    /**
		Sets the text color for this progress bar.
		\param	textcolor	    the color of the text (COLOR_DEFAULT = default color)
        \param	textshadowcolor	the color of the shadow of the text (COLOR_DEFAULT = default color)
	*/
	virtual inline void setTextColor(Uint32 textcolor, Uint32 textshadowcolor = COLOR_DEFAULT) {
		this->textcolor = textcolor;
		this->textshadowcolor = textshadowcolor;
		resize(getSize().x, getSize().y);
	}

	/**
		This method resized the progress bar to width and height. This method should only
		called if the new size is a valid size for this progress bar (See getMinumumSize).
		\param	width	the new width of this progress bar
		\param	height	the new height of this progress bar
	*/
	virtual void resize(Uint32 width, Uint32 height) {
		Widget::resize(width,height);

		if(pBackground != NULL) {
			SDL_FreeSurface(pBackground);
			pBackground = NULL;
		}

		if(pForeground != NULL) {
			SDL_FreeSurface(pForeground);
			pForeground = NULL;
		}

		pBackground = GUIStyle::getInstance().createButtonSurface(width, height, text, true, false, textcolor, textshadowcolor);
		pForeground = GUIStyle::getInstance().createProgressBarOverlay(width, height, percent, color);

	}

	/**
		Returns the minimum size of this progress bar. The progress bar should not
		resized to a size smaller than this.
		\return the minimum size of this progress bar
	*/
	virtual Point getMinimumSize() const {
		if(text == "") {
			return Point(4,4);
		} else {
			return GUIStyle::getInstance().getMinimumButtonSize(text);
		}
	}

protected:
	std::string text;			///< Text of this progress bar

    Uint32 textcolor;
    Uint32 textshadowcolor;
};

class PictureProgressBar: public ProgressBar {
public:
	PictureProgressBar() : ProgressBar() {
		enableResizing(false,false);
	}

	virtual ~PictureProgressBar() { ; }

	void setSurface(SDL_Surface* pBackground, bool bFreeBackground) {
		if((this->bFreeBackground == true) && (this->pBackground != NULL)) {
			SDL_FreeSurface(this->pBackground);
		}

		this->pBackground = pBackground;
		this->bFreeBackground = bFreeBackground;

		if(this->pBackground != NULL) {
			resize(this->pBackground->w,this->pBackground->h);
		} else {
			resize(4,4);
		}

		resizeAll();
	}

	/**
		This method resized the progress bar to width and height. This method should only
		called if the new size is a valid size for this progress bar (See getMinumumSize).
		\param	width	the new width of this progress bar
		\param	height	the new height of this progress bar
	*/
	virtual void resize(Uint32 width, Uint32 height) {
		Widget::resize(width,height);
	}

	/**
		Returns the minimum size of this progress bar. The progress bar should not
		resized to a size smaller than this.
		\return the minimum size of this progress bar
	*/
	virtual Point getMinimumSize() const {
		if(pBackground == NULL) {
			return Point(4,4);
		} else {
			return Point(pBackground->w,pBackground->h);
		}
	}
};

#endif // PROGRESSBAR_H
