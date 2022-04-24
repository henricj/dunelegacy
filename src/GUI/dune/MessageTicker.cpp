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

inline constexpr auto MESSAGESCROLLSPEED = 5;
inline constexpr auto MESSAGESCROLLTIME  = (20 * MESSAGESCROLLSPEED);
inline constexpr auto MESSAGETIME        = (15 * MESSAGESCROLLSPEED);

MessageTicker::MessageTicker() : timer(-MESSAGETIME) {
    MessageTicker::enableResizing(false, false);

    MessageTicker::resize(0, 0);
}

MessageTicker::~MessageTicker() = default;

void MessageTicker::addMessage(const std::string& msg) {
    messageTextures.emplace(
        SDL_CreateTextureFromSurface(renderer, pFontManager->getFont(14)->createTextSurface(msg, COLOR_BLACK).get()));
}

void MessageTicker::draw(Point position) {
    if (!isVisible())
        return;

    // draw message
    if (messageTextures.empty())
        return;

    if (timer++ == MESSAGESCROLLTIME) {
        timer = -MESSAGETIME;
        // delete first message
        messageTextures.pop();

        // if no more messages leave
        if (messageTextures.empty()) {
            return;
        }
    }

    SDL_Texture* tex = messageTextures.front().get();

    // draw text
    SDL_Rect textLocation = {position.x + 21, position.y + 17, 0, 0};
    SDL_Rect cut          = {0, 0, 0, 0};

    if (timer > 0) {
        // start scrolling the text
        const int newsTickerInnerEdgeY = position.y + 10;
        textLocation.y -= (timer / MESSAGESCROLLSPEED);
        if (textLocation.y < newsTickerInnerEdgeY) {
            cut.y          = newsTickerInnerEdgeY - textLocation.y;
            textLocation.y = newsTickerInnerEdgeY;
        }
    }

    textLocation.w = cut.w = getWidth(tex);
    textLocation.h = cut.h = getHeight(tex) - cut.y;
    Dune_RenderCopy(renderer, tex, &cut, &textLocation);
}
