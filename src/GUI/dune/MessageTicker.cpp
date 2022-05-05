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

#include "Renderer/DuneRenderer.h"
#include "misc/DrawingRectHelper.h"
#include <FileClasses/FontManager.h>

#include <globals.h>

inline constexpr auto MESSAGESCROLLSPEED = 5;
inline constexpr auto MESSAGESCROLLTIME  = (20 * MESSAGESCROLLSPEED);
inline constexpr auto MESSAGETIME        = (15 * MESSAGESCROLLSPEED);

MessageTicker::MessageTicker() : timer(-MESSAGETIME) {
    MessageTicker::enableResizing(false, false);

    MessageTicker::resize(0, 0);
}

MessageTicker::~MessageTicker() = default;

void MessageTicker::addMessage(std::string msg) {
    messages.emplace(std::move(msg));
}

void MessageTicker::draw(Point position) {
    if (!isVisible())
        return;

    // draw message
    if (messages.empty())
        return;

    if (timer++ == MESSAGESCROLLTIME) {
        timer = -MESSAGETIME;
        // delete first message
        messages.pop();

        texture_.reset();

        // if no more messages leave
        if (messages.empty()) {
            return;
        }
    }

    auto* const renderer = dune::globals::renderer.get();

    if (!texture_) {
        const auto& message = messages.front();

        if (trim(message).empty())
            return;

        const auto& gui = GUIStyle::getInstance();

        texture_ = gui.createText(renderer, messages.front(), COLOR_BLACK, 14);

        if (!texture_)
            return;
    }

    const auto size = getSize();

    const SDL_Rect clip{position.x + 17, position.y + 8, size.x - 33, size.y - 16};

    // draw text
    const auto x = static_cast<float>(position.x) + 21.f;
    auto y       = static_cast<float>(position.y) + 17.f;

    if (timer > 0) {
        // start scrolling the text
        static constexpr auto scale = 1.f / static_cast<float>(MESSAGESCROLLSPEED);
        y -= static_cast<float>(timer) * scale;
    }

    dune::RenderClip clipping{renderer, clip};

    texture_.draw(renderer, x, y);
}

void MessageTicker::invalidateTextures() {
    parent::invalidateTextures();

    texture_.reset();
}
