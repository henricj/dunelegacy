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

#include <GUI/dune/NewsTicker.h>

#include <globals.h>

#include <FileClasses/FontManager.h>
#include <FileClasses/GFXManager.h>

#define MESSAGESCROLLSPEED 16
#define MESSAGESCROLLTIME  (16 * MESSAGESCROLLSPEED)
#define MESSAGETIME        (16 * MESSAGESCROLLSPEED)

NewsTicker::NewsTicker() {
    Widget::enableResizing(false, false);

    timer                  = -MESSAGETIME;
    pBackground            = pGFXManager->getUIGraphic(UI_MessageBox);
    pCurrentMessageTexture = nullptr;

    Widget::resize(getTextureSize(pBackground));
}

NewsTicker::~NewsTicker() = default;

void NewsTicker::addMessage(const std::string& msg) {
    if (messages.contains(msg) || messages.size() >= 3)
        return;

    messages.push(msg);
}

void NewsTicker::addUrgentMessage(const std::string& msg) {
    messages.clear();

    messages.push(msg);
}

void NewsTicker::draw(Point position) {
    if (!isVisible()) {
        return;
    }

    // draw background
    if (pBackground == nullptr) {
        return;
    }

    pBackground->draw(renderer, position.x, position.y);

    // draw message
    if (!messages.empty()) {
        if (timer++ == MESSAGESCROLLTIME) {
            timer = -MESSAGETIME;
            // delete first message
            messages.pop();

            // if no more messages leave
            if (messages.empty()) {
                return;
            };
        };

        // draw text
        if (currentMessage != messages.front()) {
            currentMessage         = messages.front();
            pCurrentMessageTexture = pFontManager->createTextureWithText(currentMessage, COLOR_BLACK, 12);
        }

        if (pCurrentMessageTexture != nullptr) {
            SDL_FRect textLocation {static_cast<float>(position.x + 10), static_cast<float>(position.y + 6), 0.f, 0.f};
            SDL_Rect cut {0, 0, 0, 0};

            if (timer > 0) {
                // start scrolling the text
                const auto newsTickerInnerEdgeY = position.y + 4;
                textLocation.y -= (timer / MESSAGESCROLLSPEED);
                if (textLocation.y < newsTickerInnerEdgeY) {
                    cut.y          = newsTickerInnerEdgeY - textLocation.y;
                    textLocation.y = newsTickerInnerEdgeY;
                }
            }

            textLocation.w = cut.w = getWidth(pCurrentMessageTexture.get());
            textLocation.h = cut.h = getHeight(pCurrentMessageTexture.get()) - cut.y;
            Dune_RenderCopyF(renderer, pCurrentMessageTexture.get(), &cut, &textLocation);
        }
    };
}
