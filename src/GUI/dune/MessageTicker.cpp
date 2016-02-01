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

#include <GUI/dune/MessageTicker.h>

#include <globals.h>

#include <FileClasses/FontManager.h>

#define MESSAGETIME (16*13)
#define SLOWDOWN timer/16

MessageTicker::MessageTicker() : Widget() {
	enableResizing(false,false);

 	timer = -MESSAGETIME/2;

	resize(0,0);
}

MessageTicker::~MessageTicker() {

}

void MessageTicker::addMessage(const std::string& msg)
{
	messages.push(msg);
}

void MessageTicker::draw(SDL_Surface* screen, Point position) {
	if(isVisible() == false) {
		return;
	}

	// draw message
	if(!messages.empty()) {
		if(timer++ == (MESSAGETIME/3)) {
			timer = -MESSAGETIME/2;
			// delete first message
			messages.pop();

			// if no more messages leave
			if(messages.empty()) {
				timer = -MESSAGETIME/2;
				return;
			};
		};

		//draw text
		SDL_Rect textLocation = { static_cast<Sint16>(position.x + 21), static_cast<Sint16>(position.y + 15), 0, 0 };

		if(timer>0) {
			textLocation.y -= SLOWDOWN;
		}

		SDL_Surface *surface = pFontManager->createSurfaceWithText(messages.front(), COLOR_BLACK, FONT_STD12);

		SDL_Rect cut = { 0, 0, 0, 0 };

		if(timer>0) {
			cut.y = 3*SLOWDOWN;
		}

		cut.h = surface->h - cut.y;
		cut.w = surface->w;
		SDL_BlitSurface(surface, &cut, screen, &textLocation);
		SDL_FreeSurface(surface);
	};
}
