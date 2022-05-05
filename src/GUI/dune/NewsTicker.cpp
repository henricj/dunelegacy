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

#include <FileClasses/GFXManager.h>

inline constexpr auto MESSAGESCROLLSPEED = 16;
inline constexpr auto MESSAGESCROLLTIME  = (16 * MESSAGESCROLLSPEED);
inline constexpr auto MESSAGETIME        = (16 * MESSAGESCROLLSPEED);

NewsTicker::NewsTicker()
    : pBackground(dune::globals::pGFXManager->getUIGraphic(UI_MessageBox)), pCurrentMessageTexture(nullptr),
      timer(-MESSAGETIME) {
    parent::enableResizing(false, false);

    parent::resize(getTextureSize(pBackground));
}

NewsTicker::~NewsTicker() = default;

void NewsTicker::addMessage(std::string msg) {
    if (messages.contains(msg) || messages.size() >= 3)
        return;

    messages.push(std::move(msg));
}

void NewsTicker::addUrgentMessage(std::string msg) {
    messages.clear();

    messages.push(std::move(msg));
}

void NewsTicker::draw(Point position) {
    if (!isVisible())
        return;

    auto* const renderer = dune::globals::renderer.get();

    // draw background
    if (pBackground)
        pBackground->draw(renderer, static_cast<float>(position.x), static_cast<float>(position.y));

    // draw message
    if (messages.empty())
        return;

    if (timer++ == MESSAGESCROLLTIME) {
        timer = -MESSAGETIME;
        // delete first message
        messages.pop();

        // if no more messages leave
        if (messages.empty()) {
            return;
        }
    }

    // draw text

    if (const auto& message = messages.front(); currentMessage != message) {
        const auto& gui = GUIStyle::getInstance();

        currentMessage         = message;
        pCurrentMessageTexture = gui.createText(renderer, currentMessage, COLOR_BLACK, 12);
    }

    if (!pCurrentMessageTexture)
        return;

    const auto x = static_cast<float>(position.x) + 10.f;
    auto y       = static_cast<float>(position.y) + 5.f;

    if (timer > 0) {
        // start scrolling the text
        constexpr auto scale = 1.f / static_cast<float>(MESSAGESCROLLSPEED);
        y -= static_cast<float>(timer) * scale;
    }

    const auto size = getSize();
    const SDL_Rect clip{position.x + 8, position.y + 4, size.x - 16, size.y - 8};

    dune::RenderClip clipping{renderer, clip};

    pCurrentMessageTexture.draw(renderer, x, y);
}
