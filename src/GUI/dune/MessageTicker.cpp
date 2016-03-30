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
	messageTextures.push(std::shared_ptr<SDL_Texture>(pFontManager->createTextureWithText(msg, COLOR_BLACK, FONT_STD12), SDL_DestroyTexture));
}

void MessageTicker::draw(SDL_Surface* screen, Point position) {
	if(isVisible() == false) {
		return;
	}

	// draw message
	if(!messageTextures.empty()) {
		if(timer++ == (MESSAGETIME/3)) {
			timer = -MESSAGETIME/2;
			// delete first message
			messageTextures.pop();

			// if no more messages leave
			if(messageTextures.empty()) {
				timer = -MESSAGETIME/2;
				return;
			};
		};

		//draw text
		SDL_Rect textLocation = { position.x + 21, position.y + 15, 0, 0 };

		if(timer>0) {
			textLocation.y -= SLOWDOWN;
		}

		SDL_Texture *tex = messageTextures.front().get();

		SDL_Rect cut = { 0, 0, 0, 0 };

		if(timer>0) {
			cut.y = 3*SLOWDOWN;
		}

		textLocation.w = cut.w = getWidth(tex);
		textLocation.h = cut.h = getHeight(tex) - cut.y;
		SDL_RenderCopy(renderer, tex, &cut, &textLocation);
	};
}
