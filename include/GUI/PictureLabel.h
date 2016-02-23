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

#ifndef PICTURELABEL_H
#define PICTURELABEL_H

#include "Widget.h"
#include <SDL.h>

/// A class for showning a static picture
class PictureLabel : public Widget {
public:

	/// default constructor
	PictureLabel() : Widget() {
		pSurface = NULL;
		bFreeSurface = false;
	}

	/// destructor
	virtual ~PictureLabel() {
		if((bFreeSurface == true) && (pSurface != NULL)) {
			SDL_FreeSurface(pSurface);
			pSurface = NULL;
		}
	}

	/**
		This method sets the surface for this picture label.
		\param	pSurface	This surface is shown
		\param	bFreeSurface	Should pSurface be freed if this picture label is destroyed?
	*/
	virtual void setSurface(SDL_Surface* pSurface,bool bFreeSurface) {
		if((this->bFreeSurface == true) && (this->pSurface != NULL)) {
			SDL_FreeSurface(this->pSurface);
			this->pSurface = NULL;
		}

		this->pSurface = pSurface;
		this->bFreeSurface = bFreeSurface;

		if(this->pSurface != NULL) {
			resize(this->pSurface->w,this->pSurface->h);
		} else {
			resize(0,0);
		}
	}

	/**
		Returns the minimum size of this picture label. The picture label should not
		be resized to a size smaller than this.
		\return the minimum size of this picture label
	*/
	virtual Point getMinimumSize() const {
		if(pSurface != NULL) {
			return Point((Sint32) pSurface->w, (Sint32) pSurface->h);
		} else {
			return Point(0,0);
		}
	}

	/**
		Draws this button to screen. This method is called before drawOverlay().
		\param	screen	Surface to draw on
		\param	position	Position to draw the button to
	*/
	void draw(SDL_Surface* screen, Point position) {
		if(isVisible() == false) {
			return;
		}

		if(pSurface == NULL) {
			return;
		}

		SDL_Rect dest = calcDrawingRect(pSurface, position.x, position.y);
		SDL_BlitSurface(pSurface,NULL,screen,&dest);
	}


private:
	SDL_Surface* pSurface;	///< Surface that is shown
	bool bFreeSurface;		///< Should pSurface be freed if this picture label is destroyed?
};

#endif // PICTURELABEL_H
