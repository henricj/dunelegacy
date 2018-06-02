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

Button::~Button() {
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
        if(pPressedTexture) {
            tex = pPressedTexture.get();
        } else {
            if(isActive() && pActiveTexture) {
                tex = pActiveTexture.get();
            } else {
                tex = pUnpressedTexture.get();
            }
        }
    } else {
        if(bPressed == true) {
            if(pPressedTexture) {
                tex = pPressedTexture.get();
            } else {
                if(isActive() && pActiveTexture) {
                    tex = pActiveTexture.get();
                } else {
                    tex = pUnpressedTexture.get();
                }
            }
        } else {
            if((isActive() || bHover) && pActiveTexture) {
                tex = pActiveTexture.get();
            } else {
                tex = pUnpressedTexture.get();
            }
        }
    }

    if(!tex) {
        return;
    }

    SDL_Rect dest = calcDrawingRect(tex, position.x, position.y);
    SDL_RenderCopy(renderer, tex, nullptr, &dest);
}

void Button::drawOverlay(Point position) {
    if(!isVisible() || !isEnabled() || bHover != true || !tooltipTexture) return;

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

void Button::invalidateTextures() {
    pUnpressedTexture.reset();
    pPressedTexture.reset();
    pActiveTexture.reset();
}

void Button::setSurfaces(   sdl2::surface_unique_or_nonowning_ptr pUnpressedSurface,
                            sdl2::surface_unique_or_nonowning_ptr pPressedSurface,
                            sdl2::surface_unique_or_nonowning_ptr pActiveSurface) {

    setTextures(    convertSurfaceToTexture(pUnpressedSurface.get()),
                    convertSurfaceToTexture(pPressedSurface.get()),
                    convertSurfaceToTexture(pActiveSurface.get()) );
}

void Button::setTextures(   sdl2::texture_unique_or_nonowning_ptr pUnpressedTexture,
                            sdl2::texture_unique_or_nonowning_ptr pPressedTexture,
                            sdl2::texture_unique_or_nonowning_ptr pActiveTexture) {
    this->pUnpressedTexture = std::move(pUnpressedTexture);
    this->pPressedTexture = std::move(pPressedTexture);
    this->pActiveTexture = std::move(pActiveTexture);
}
