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

#include <GUI/WidgetWithBackground.h>

#include "GUI/GUIStyle.h"
#include "misc/draw_util.h"

#include "globals.h"

WidgetWithBackground::~WidgetWithBackground() = default;

void WidgetWithBackground::setTransparentBackground(bool bTransparent) {
    bTransparentBackground = bTransparent;
}

void WidgetWithBackground::setBackground(const DuneTexture* pBackground) {
    setBackground(static_cast<SDL_Surface*>(nullptr));

    if (!pBackground || !*pBackground) {
        bSelfGeneratedBackground = true;
        this->pBackground        = nullptr;
    } else {
        bSelfGeneratedBackground = false;
        this->pBackground        = pBackground;
    }
}

void WidgetWithBackground::setBackground(DuneTextureOwned background) {
    localDuneTexture_ = background.as_dune_texture();
    localTexture_     = std::move(background.texture_);

    setBackground(&localDuneTexture_);
}

void WidgetWithBackground::resize(uint32_t width, uint32_t height) {
    parent::resize(width, height);

    if (bSelfGeneratedBackground) {
        localDuneTexture_.reset();
        localTexture_.reset();
        pBackground = nullptr;
    }
}

void WidgetWithBackground::draw_background(Point position) {
    auto* const renderer = dune::globals::renderer.get();

    const auto* background = getBackground();
    if (!background) {
        if (bSelfGeneratedBackground) {
            auto& size = getSize();

            const SDL_FRect dest{static_cast<float>(position.x), static_cast<float>(position.y),
                                 static_cast<float>(size.x), static_cast<float>(size.y)};

            GUIStyle::getInstance().drawBackground(renderer, dest);
        }

        return;
    }

    SDL_FRect dst{static_cast<float>(position.x), static_cast<float>(position.y), background->width_,
                  background->height_};

    const auto size = getSize();

    const auto x = static_cast<float>(size.x);
    const auto y = static_cast<float>(size.y);

    if (dst.w != x) // NOLINT(clang-diagnostic-float-equal)
        dst.x += (x - dst.w) / 2;
    if (dst.h != y) // NOLINT(clang-diagnostic-float-equal)
        dst.y += (y - dst.h) / 2;

    background->draw(renderer, dst.x, dst.y);
}

void WidgetWithBackground::draw(Point position) {
    if (bTransparentBackground)
        return;

    draw_background(position);
}

void WidgetWithBackground::invalidateTextures() {
    pBackground = nullptr;

    localDuneTexture_.reset();
    localTexture_.reset();

    parent::invalidateTextures();
}

DuneSurfaceOwned WidgetWithBackground::createBackground() {
    return {};
}

const DuneTexture* WidgetWithBackground::getBackground() {
    if (bTransparentBackground)
        return nullptr;

    if (bSelfGeneratedBackground
        && (!pBackground || !*pBackground || pBackground->source_.w != getSize().x
            || pBackground->source_.h != getSize().y)) {
        const auto surface = createBackground();

        setBackground(surface.get());
    }

    return pBackground && *pBackground ? pBackground : nullptr;
}

void WidgetWithBackground::setBackground(SDL_Surface* surface) {
    if (surface) {
        localTexture_     = convertSurfaceToTexture(surface);
        localDuneTexture_ = DuneTexture{localTexture_.get()};
        pBackground       = &localDuneTexture_;
    } else {
        localDuneTexture_.reset();
        localTexture_.reset();
        pBackground = nullptr;
    }
}

// void WidgetWithBackground::setBackground(SDL_Texture* texture) {
//     localTexture_.reset();
//
//     if (texture) {
//         localDuneTexture_ = DuneTexture{texture};
//         pBackground       = &localDuneTexture_;
//     } else {
//         localDuneTexture_ = DuneTexture{};
//         pBackground       = nullptr;
//     }
// }
