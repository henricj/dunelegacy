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

#include <GUI/Button.h>

#include <globals.h>

#include <SoundPlayer.h>

#include "misc/DrawingRectHelper.h"
#include "misc/dune_clock.h"

#include <chrono>

Button::Button()  = default;
Button::~Button() = default;

void Button::handleMouseMovement(int32_t x, int32_t y, bool insideOverlay) {
    if ((x < 0) || (x >= getSize().x) || (y < 0) || (y >= getSize().y)) {
        bPressed = false;
        bHover   = false;
    } else if (isEnabled() && !insideOverlay) {
        bHover                 = true;
        tooltipLastMouseMotion = dune::dune_clock::now();
    }
}

bool Button::handleMouseLeft(int32_t x, int32_t y, bool pressed) {
    if ((x < 0) || (x >= getSize().x) || (y < 0) || (y >= getSize().y)) {
        return false;
    }

    if ((!isEnabled()) || (!isVisible())) {
        return true;
    }

    if (pressed) {
        // button pressed
        bPressed = true;
        if (!bToggleButton) {
            soundPlayer->playSound(Sound_enum::Sound_ButtonClick);
        }
    } else {
        // button released
        if (bPressed) {
            bPressed = false;
            if (bToggleButton) {
                const bool oldState = getToggleState();
                setToggleState(!bToggleState);
                if (getToggleState() != oldState) {
                    soundPlayer->playSound(Sound_enum::Sound_ButtonClick);
                }
            }

            if (pOnClick) {
                pOnClick();
            }
        }
    }
    return true;
}

bool Button::handleKeyPress(SDL_KeyboardEvent& key) {
    if ((!isVisible()) || (!isEnabled()) || (!isActive())) {
        return true;
    }

    if (key.keysym.sym == SDLK_TAB) {
        setInactive();
        return true;
    }

    if (key.keysym.sym == SDLK_SPACE) {
        if (bToggleButton) {
            const bool oldState = getToggleState();
            setToggleState(!bToggleState);
            if (getToggleState() != oldState) {
                soundPlayer->playSound(Sound_enum::Sound_ButtonClick);
            }
        } else {
            soundPlayer->playSound(Sound_enum::Sound_ButtonClick);
        }

        if (pOnClick) {
            pOnClick();
        }
    }

    if ((!bToggleButton) && (SDL_GetModState() == KMOD_NONE) && (key.keysym.sym == SDLK_RETURN)) {
        soundPlayer->playSound(Sound_enum::Sound_ButtonClick);

        if (pOnClick) {
            pOnClick();
        }
    }

    return true;
}

void Button::draw(Point position) {
    if (!isVisible()) {
        return;
    }

    updateTextures();

    const DuneTexture* tex = nullptr;
    if (bToggleState) {
        if (pPressedTexture) {
            tex = pPressedTexture;
        } else {
            if (isActive() && pActiveTexture) {
                tex = pActiveTexture;
            } else {
                tex = pUnpressedTexture;
            }
        }
    } else {
        if (bPressed) {
            if (pPressedTexture) {
                tex = pPressedTexture;
            } else {
                if (isActive() && pActiveTexture) {
                    tex = pActiveTexture;
                } else {
                    tex = pUnpressedTexture;
                }
            }
        } else {
            if ((isActive() || bHover) && pActiveTexture) {
                tex = pActiveTexture;
            } else {
                tex = pUnpressedTexture;
            }
        }
    }

    const auto& gui = GUIStyle::getInstance();

    const auto& hw = getSize();
    const SDL_FRect dest{static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(hw.x),
                         static_cast<float>(hw.y)};

    gui.RenderButton(renderer, dest, tex, bPressed);
}

void Button::drawOverlay(Point position) {
    using namespace std::chrono_literals;

    if (!isVisible() || !isEnabled() || !bHover || !tooltipTexture)
        return;

    if (dune::dune_clock::now() - tooltipLastMouseMotion <= 750ms)
        return;

    const auto renderRect = getRendererSize();
    const auto render_w   = static_cast<float>(renderRect.w);
    const auto render_h   = static_cast<float>(renderRect.h);

    auto dest = calcDrawingRect(tooltipTexture, drawnMouseX, drawnMouseY, HAlign::Left, VAlign::Bottom);
    if (dest.x + dest.w >= render_w) {
        // do not draw tooltip outside screen
        dest.x = render_w - dest.w;
    }

    if (dest.y < 0) {
        // do not draw tooltip outside screen
        dest.y = 0;
    } else if (dest.y + dest.h >= render_h) {
        // do not draw tooltip outside screen
        dest.y = render_h - dest.h;
    }

    Dune_RenderCopyF(renderer, tooltipTexture.get(), nullptr, &dest);
}

void Button::invalidateTextures() {
    pUnpressedTexture = nullptr;
    pPressedTexture   = nullptr;
    pActiveTexture    = nullptr;

    localDuneUnpressed_ = DuneTexture{};
    localDunePressed_   = DuneTexture{};
    localDuneActive_    = DuneTexture{};

    localUnpressed_.reset();
    localPressed_.reset();
    localActive_.reset();

    parent::invalidateTextures();
}

void Button::setSurfaces(sdl2::surface_ptr pUnpressedSurface, sdl2::surface_ptr pPressedSurface,
                         sdl2::surface_ptr pActiveSurface) {
    localUnpressed_ = convertSurfaceToTexture(pUnpressedSurface.get());
    localPressed_   = convertSurfaceToTexture(pPressedSurface.get());
    localActive_    = convertSurfaceToTexture(pActiveSurface.get());

    localDuneUnpressed_ = DuneTexture{localUnpressed_.get()};
    localDunePressed_   = DuneTexture{localPressed_.get()};
    localDuneActive_    = DuneTexture{localActive_.get()};

    setTextures(&localDuneUnpressed_, &localDunePressed_, localActive_ ? &localDuneActive_ : nullptr);
}

void Button::setTextures(const DuneTexture* pUnpressedTexture, const DuneTexture* pPressedTexture,
                         const DuneTexture* pActiveTexture) {
    if (!pUnpressedTexture || localUnpressed_.get() != pUnpressedTexture->texture_)
        localUnpressed_.reset();
    if (!pPressedTexture || localPressed_.get() != pPressedTexture->texture_)
        localPressed_.reset();
    if (!pActiveTexture || localActive_.get() != pActiveTexture->texture_)
        localActive_.reset();

    this->pUnpressedTexture = pUnpressedTexture;
    this->pPressedTexture   = pPressedTexture;
    this->pActiveTexture    = pActiveTexture;
}

void Button::setTextures(DuneTextureOwned pUnpressedTexture, DuneTextureOwned pPressedTexture,
                         DuneTextureOwned pActiveTexture) {

    localUnpressed_ = std::move(pUnpressedTexture.texture_);
    localPressed_   = std::move(pPressedTexture.texture_);
    localActive_    = std::move(pActiveTexture.texture_);

    localDuneUnpressed_         = DuneTexture{localUnpressed_.get()};
    localDuneUnpressed_.width_  = pUnpressedTexture.width_;
    localDuneUnpressed_.height_ = pUnpressedTexture.height_;

    localDunePressed_         = DuneTexture{localPressed_.get()};
    localDunePressed_.width_  = pPressedTexture.width_;
    localDunePressed_.height_ = pPressedTexture.height_;

    localDuneActive_         = DuneTexture{localActive_.get()};
    localDuneActive_.width_  = pActiveTexture.width_;
    localDuneActive_.height_ = pActiveTexture.height_;

    setTextures(&localDuneUnpressed_, &localDunePressed_, localActive_ ? &localDuneActive_ : nullptr);
}
