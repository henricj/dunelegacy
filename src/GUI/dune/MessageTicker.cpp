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

#define MESSAGESCROLLSPEED  5
#define MESSAGESCROLLTIME   (20*MESSAGESCROLLSPEED)
#define MESSAGETIME         (15*MESSAGESCROLLSPEED)

MessageTicker::MessageTicker() {
    enableResizing(false,false);

    timer = -MESSAGETIME;

    resize(0,0);
}

MessageTicker::~MessageTicker() = default;

void MessageTicker::addMessage(const std::string& msg)
{
    messageTextures.emplace(pFontManager->createTextureWithText(msg, COLOR_BLACK, 14));
}

void MessageTicker::draw(Point position) {
    if(isVisible() == false)
        return;

    // draw message
    if(messageTextures.empty()) return;

    if(timer++ == MESSAGESCROLLTIME) {
        timer = -MESSAGETIME;
        // delete first message
        messageTextures.pop();

        // if no more messages leave
        if(messageTextures.empty()) {
            return;
        }
    }

    SDL_Texture *tex = messageTextures.front().get();

    //draw text
    SDL_Rect textLocation = { position.x + 21, position.y + 17, 0, 0 };
    SDL_Rect cut = { 0, 0, 0, 0 };

    if(timer>0) {
        // start scrolling the text
        int newsTickerInnerEdgeY = position.y + 10;
        textLocation.y -= (timer / MESSAGESCROLLSPEED);
        if(textLocation.y < newsTickerInnerEdgeY) {
            cut.y = newsTickerInnerEdgeY - textLocation.y;
            textLocation.y = newsTickerInnerEdgeY;
        }
    }

    textLocation.w = cut.w = getWidth(tex);
    textLocation.h = cut.h = getHeight(tex) - cut.y;
    SDL_RenderCopy(renderer, tex, &cut, &textLocation);
}
