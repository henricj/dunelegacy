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
    bFreeUnpressedTexture = false;
    bFreePressedTexture = false;
    bFreeActiveTexture = false;
}

Button::~Button() {
    invalidateTextures();
}

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

    if((isEnabled() == false) || (isVisible() == false)) {
        return true;
    }

    if(pressed == true) {
        // button pressed
        bPressed = true;
        if(!bToggleButton) {
            soundPlayer->playSound(Sound_ButtonClick);
        }
    } else {
        // button released
        if(bPressed == true) {
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
    if((isVisible() == false) || (isEnabled() == false) || (isActive() == false)) {
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

    if((bToggleButton == false) && (SDL_GetModState() == KMOD_NONE) && (key.keysym.sym == SDLK_RETURN)) {
        soundPlayer->playSound(Sound_ButtonClick);

        if(pOnClick) {
            pOnClick();
        }
    }

    return true;
}

void Button::draw(Point position) {
    if(isVisible() == false) {
        return;
    }

    updateTextures();

    SDL_Texture* tex;
    if(bToggleState == true) {
        if(pPressedTexture != nullptr) {
            tex = pPressedTexture.get();
        } else {
            if(isActive() && pActiveTexture != nullptr) {
                tex = pActiveTexture.get();
            } else {
                tex = pUnpressedTexture.get();
            }
        }
    } else {
        if(bPressed == true) {
            if(pPressedTexture != nullptr) {
                tex = pPressedTexture.get();
            } else {
                if(isActive() && pActiveTexture != nullptr) {
                    tex = pActiveTexture.get();
                } else {
                    tex = pUnpressedTexture.get();
                }
            }
        } else {
            if((isActive() || bHover) && pActiveTexture != nullptr) {
                tex = pActiveTexture.get();
            } else {
                tex = pUnpressedTexture.get();
            }
        }
    }

    if(tex == nullptr) {
        return;
    }

    SDL_Rect dest = calcDrawingRect(tex, position.x, position.y);
    SDL_RenderCopy(renderer, tex, nullptr, &dest);
}

void Button::drawOverlay(Point position) {
    if(!isVisible() || !isEnabled() || bHover != true || tooltipTexture == nullptr) return;

    if(SDL_GetTicks() - tooltipLastMouseMotion <= 750) return;

    SDL_Rect renderRect = getRendererSize();
    SDL_Rect dest = calcDrawingRect(tooltipTexture.get(), drawnMouseX, drawnMouseY, HAlign::Left, VAlign::Bottom);
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

    SDL_RenderCopy(renderer, tooltipTexture.get(), nullptr, &dest);
}

void Button::setSurfaces(   SDL_Surface* pUnpressedSurface,bool bFreeUnpressedSurface,
                            SDL_Surface* pPressedSurface,bool bFreePressedSurface,
                            SDL_Surface* pActiveSurface,bool bFreeActiveSurface) {

    setTextures(    convertSurfaceToTexture(pUnpressedSurface, bFreeUnpressedSurface).release(), true,
                    convertSurfaceToTexture(pPressedSurface, bFreePressedSurface).release(), true,
                    convertSurfaceToTexture(pActiveSurface, bFreeActiveSurface).release(), true);
}

void Button::setTextures(   SDL_Texture* pUnpressedTexture, bool bFreeUnpressedTexture,
                            SDL_Texture* pPressedTexture, bool bFreePressedTexture,
                            SDL_Texture* pActiveTexture, bool bFreeActiveTexture) {
    invalidateTextures();
    this->pUnpressedTexture = sdl2::texture_ptr{ pUnpressedTexture };
    this->bFreeUnpressedTexture = bFreeUnpressedTexture;
    this->pPressedTexture = sdl2::texture_ptr{ pPressedTexture};
    this->bFreePressedTexture = bFreePressedTexture;
    this->pActiveTexture = sdl2::texture_ptr{ pActiveTexture };
    this->bFreeActiveTexture = bFreeActiveTexture;
}

void Button::invalidateTextures() {
    if (pUnpressedTexture != nullptr) {
        if ((bFreeUnpressedTexture == true)) {
            pUnpressedTexture.reset();
            bFreeUnpressedTexture = false;
        } else {
            pUnpressedTexture.release();
        }
    }

    if ((pPressedTexture != nullptr)) {
        if ((bFreePressedTexture == true)) {
            pPressedTexture.reset();
            bFreePressedTexture = false;
        } else {
            pPressedTexture.release();
        }
    }

    if ((pActiveTexture != nullptr)) {
        if ((bFreeActiveTexture == true) && (pActiveTexture != nullptr)) {
            pActiveTexture.reset();
            bFreeActiveTexture = false;
        } else {
            pActiveTexture.release();
        }
    }
}
