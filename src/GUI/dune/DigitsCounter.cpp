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

#include "GUI/dune/DigitsCounter.h"

#include "GUI/Widget.h"
#include <FileClasses/GFXManager.h>

#include <charconv>

DigitsCounter::DigitsCounter() {
    parent::enableResizing(false, false);
}

DigitsCounter::~DigitsCounter() = default;

void DigitsCounter::draw(Point position) {
    const auto* const gfx = dune::globals::pGFXManager.get();
    auto* const renderer  = dune::globals::renderer.get();

    const auto* const tex = gfx->getUIGraphic(UI_MissionSelect);

    tex->draw(renderer, position.x, position.y);

    const auto* const digitsTex = gfx->getUIGraphic(UI_CreditsDigits);

    char creditsBuffer[3];
    const auto& [ptr, ec] = std::to_chars(std::begin(creditsBuffer), std::end(creditsBuffer), count);
    if (ec == std::errc{}) {
        const auto digits = static_cast<int>(ptr - std::begin(creditsBuffer));

        for (auto i = digits - 1; i >= 0; i--) {
            const auto source = calcSpriteSourceRect(digitsTex, creditsBuffer[i] - '0', 10);
            const auto dest2 =
                calcSpriteDrawingRect(digitsTex, position.x + 40 + (6 - digits + i) * 10, position.y + 16, 10);
            Dune_RenderCopyF(renderer, digitsTex, &source, &dest2);
        }
    }
}

Point DigitsCounter::getMinimumSize() const {
    const auto* const tex = dune::globals::pGFXManager->getUIGraphic(UI_MissionSelect);
    if (tex != nullptr) {
        return getTextureSize(tex);
    }
    return {0, 0};
}
