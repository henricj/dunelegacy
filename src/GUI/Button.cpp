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

Button::Button() {
    bPressed = false;
    bHover = false;
    bToggleButton = false;
    bToggleState = false;

    pUnpressedTexture = nullptr;
    pPressedTexture = nullptr;
    pActiveTexture = nullptr;
}

Button::~Button() = default;

void Button::handleMouseMovement(Sint32 x, Sint32 y, bool insideOverlay) {
    if((x < 0) || (x >= getSize().x) || (y < 0) || (y >= getSize().y)) {
        bPressed = false;
        bHover = false;
    } else if(isEnabled() && !insideOverlay) {
        bHover = true;
        tooltipLastMouseMotion = SDL_GetTicks();
    }
}

bool Button::handleMouseLeft(Sint32 x, Sint32 y, bool pressed) {
    if((x < 0) || (x >= getSize().x) || (y < 0) || (y >= getSize().y)) {
        return false;
    }

    if((!isEnabled()) || (!isVisible())) {
        return true;
    }

    if(pressed) {
        // button pressed
        bPressed = true;
        if(!bToggleButton) {
            soundPlayer->playSound(Sound_ButtonClick);
        }
    } else {
        // button released
        if(bPressed) {
            bPressed = false;
            if(bToggleButton) {
                bool oldState = getToggleState();
                setToggleState(!bToggleState);
                if(getToggleState() != oldState) {
                    soundPlayer->playSound(Sound_ButtonClick);
                }
            }

            if(pOnClick) {
                pOnClick();
            }
        }
    }
    return true;
}

bool Button::handleKeyPress(SDL_KeyboardEvent& key) {
    if((!isVisible()) || (!isEnabled()) || (!isActive())) {
        return true;
    }

    if(key.keysym.sym == SDLK_TAB) {
        setInactive();
        return true;
    }

    if(key.keysym.sym == SDLK_SPACE) {
        if(bToggleButton) {
            bool oldState = getToggleState();
            setToggleState(!bToggleState);
            if(getToggleState() != oldState) {
                soundPlayer->playSound(Sound_ButtonClick);
            }
        } else {
            soundPlayer->playSound(Sound_ButtonClick);
        }

        if(pOnClick) {
            pOnClick();
        }
    }

    if((!bToggleButton) && (SDL_GetModState() == KMOD_NONE) && (key.keysym.sym == SDLK_RETURN)) {
        soundPlayer->playSound(Sound_ButtonClick);

        if(pOnClick) {
            pOnClick();
        }
    }

    return true;
}

void Button::draw(Point position) {
    if(!isVisible()) {
        return;
    }

    updateTextures();

    const DuneTexture* tex = nullptr;
    if(bToggleState) {
        if(pPressedTexture) {
            tex = pPressedTexture;
        } else {
            if(isActive() && pActiveTexture) {
                tex = pActiveTexture;
            } else {
                tex = pUnpressedTexture;
            }
        }
    } else {
        if(bPressed) {
            if(pPressedTexture) {
                tex = pPressedTexture;
            } else {
                if(isActive() && pActiveTexture) {
                    tex = pActiveTexture;
                } else {
                    tex = pUnpressedTexture;
                }
            }
        } else {
            if((isActive() || bHover) && pActiveTexture) {
                tex = pActiveTexture;
            } else {
                tex = pUnpressedTexture;
            }
        }
    }

    if(!tex) {
        return;
    }

    const auto dest = calcDrawingRect(tex, position.x, position.y);
    Dune_RenderCopy(renderer, tex, nullptr, &dest);
}

void Button::drawOverlay(Point position) {
    if(!isVisible() || !isEnabled() || !bHover || !tooltipTexture) return;

    if(SDL_GetTicks() - tooltipLastMouseMotion <= 750) return;

    const auto renderRect = getRendererSize();
    auto dest = calcDrawingRect(tooltipTexture.get(), drawnMouseX, drawnMouseY, HAlign::Left, VAlign::Bottom);
    if(dest.x + dest.w >= renderRect.w) {
        // do not draw tooltip outside screen
        dest.x = renderRect.w - dest.w;
    }

    if(dest.y < 0) {
        // do not draw tooltip outside screen
        dest.y = 0;
    } else if(dest.y + dest.h >= renderRect.h) {
        // do not draw tooltip outside screen
        dest.y = renderRect.h - dest.h;
    }

    Dune_RenderCopy(renderer, tooltipTexture.get(), nullptr, &dest);
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
    this->pUnpressedTexture = pUnpressedTexture;
    this->pPressedTexture = pPressedTexture;
    this->pActiveTexture = pActiveTexture;
}
