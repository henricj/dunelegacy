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

#ifndef DIGITSCOUNTER_H
#define DIGITSCOUNTER_H

#include <GUI/Widget.h>

#include <FileClasses/GFXManager.h>

extern GFXManager* pGFXManager;


/// A widget for showing digits (like the credits in dune are shown)
class DigitsCounter : public Widget
{
public:

	/// default constructor
	DigitsCounter() {
		enableResizing(false,false);
		count = 0;
	}

	/// destructor
	virtual ~DigitsCounter() { ; };

	/**
		Get the current count of this digits counter
		\return	the number that this digits counter currently shows
	*/
	inline unsigned int getCount() { return count; }

	/**
		Set the count of this digits counter
		\param	newCount	the new number to show
	*/
	inline void setCount(unsigned int newCount) { count = newCount; }

	/**
		Draws this widget to screen. This method is called before drawOverlay().
		\param	screen	Surface to draw on
		\param	position	Position to draw the widget to
	*/
	virtual inline void draw(SDL_Surface* screen, Point position) {
		SDL_Surface* surface = pGFXManager->getUIGraphic(UI_MissionSelect);

		SDL_Rect dest = { position.x, position.y, surface->w, surface->h } ;
		SDL_BlitSurface(surface, NULL, screen, &dest);

		SDL_Surface* digitsSurface = pGFXManager->getUIGraphic(UI_CreditsDigits);

		char creditsBuffer[3];
		sprintf(creditsBuffer, "%d", count);
		int digits = strlen(creditsBuffer);

		for(int i=digits-1; i>=0; i--) {
            SDL_Rect source = { (creditsBuffer[i] - '0')*(digitsSurface->w/10), 0, digitsSurface->w/10, digitsSurface->h };
            SDL_Rect dest2 = { position.x + 40 + (6 - digits + i)*10, position.y + 16, digitsSurface->w/10, digitsSurface->h } ;
			SDL_BlitSurface(digitsSurface, &source, screen, &dest2);
		}

	};

	/**
		Returns the minimum size of this digits counter. The widget should not
		be resized to a size smaller than this.
		\return the minimum size of this digits counter
	*/
	virtual Point getMinimumSize() const {
		SDL_Surface* surface = pGFXManager->getUIGraphic(UI_MissionSelect);
		if(surface != NULL) {
			return Point((Sint32) surface->w, (Sint32) surface->h);
		} else {
			return Point(0,0);
		}
	}

private:

	unsigned int count;
};

#endif // DIGITSCOUNTER_H
