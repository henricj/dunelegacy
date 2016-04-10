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

Button::Button() : Widget() {
	tooltipText = "";
	tooltipTexture = nullptr;

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

	if(tooltipTexture != nullptr) {
		SDL_DestroyTexture(tooltipTexture);
		tooltipTexture = nullptr;
	}
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
			tex = pPressedTexture;
		} else {
			if(isActive() && pActiveTexture != nullptr) {
				tex = pActiveTexture;
			} else {
				tex = pUnpressedTexture;
			}
		}
	} else {
		if(bPressed == true) {
			if(pPressedTexture != nullptr) {
				tex = pPressedTexture;
			} else {
				if(isActive() && pActiveTexture != nullptr) {
					tex = pActiveTexture;
				} else {
					tex = pUnpressedTexture;
				}
			}
		} else {
			if((isActive() || bHover) && pActiveTexture != nullptr) {
				tex = pActiveTexture;
			} else {
				tex = pUnpressedTexture;
			}
		}
	}

	if(tex == nullptr) {
		return;
	}

	SDL_Rect dest = calcDrawingRect(tex, position.x, position.y);
	SDL_RenderCopy(renderer, tex, nullptr, &dest);
}

void Button::drawOverlay(Point Pos) {
	if(isVisible() && isEnabled() && (bHover == true)) {
		if(tooltipTexture != nullptr) {
			if((SDL_GetTicks() - tooltipLastMouseMotion) > 750) {
				int x,y;
				SDL_GetMouseState(&x,&y);
				int win_w, win_h;
				SDL_GetWindowSize(window, &win_w, &win_h);
				SDL_Rect vp;
				SDL_RenderGetViewport(renderer, &vp);
				x = (x*vp.w)/win_w-vp.x;
				y = (y*vp.h)/win_h-vp.y;
				SDL_Rect renderRect = getRendererSize();
				SDL_Rect dest = calcDrawingRect(tooltipTexture, x, y, HAlign::Left, VAlign::Bottom);
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

				SDL_RenderCopy(renderer, tooltipTexture, nullptr, &dest);
			}
		}
	}
}

void Button::setSurfaces(	SDL_Surface* pUnpressedSurface,bool bFreeUnpressedSurface,
							SDL_Surface* pPressedSurface,bool bFreePressedSurface,
							SDL_Surface* pActiveSurface,bool bFreeActiveSurface) {

    setTextures(    convertSurfaceToTexture(pUnpressedSurface, bFreeUnpressedSurface), true,
                    convertSurfaceToTexture(pPressedSurface, bFreePressedSurface), true,
                    convertSurfaceToTexture(pActiveSurface, bFreeActiveSurface), true);
}

void Button::setTextures(	SDL_Texture* pUnpressedTexture, bool bFreeUnpressedTexture,
							SDL_Texture* pPressedTexture, bool bFreePressedTexture,
							SDL_Texture* pActiveTexture, bool bFreeActiveTexture) {
	invalidateTextures();
	this->pUnpressedTexture = pUnpressedTexture;
	this->bFreeUnpressedTexture = bFreeUnpressedTexture;
	this->pPressedTexture = pPressedTexture;
	this->bFreePressedTexture = bFreePressedTexture;
	this->pActiveTexture = pActiveTexture;
	this->bFreeActiveTexture = bFreeActiveTexture;
}

void Button::invalidateTextures() {
	if((bFreeUnpressedTexture == true) && (pUnpressedTexture != nullptr)) {
		SDL_DestroyTexture(pUnpressedTexture);
		pUnpressedTexture = nullptr;
		bFreeUnpressedTexture = false;
	}

	if((bFreePressedTexture == true) && (pPressedTexture != nullptr)) {
		SDL_DestroyTexture(pPressedTexture);
		pPressedTexture = nullptr;
		bFreePressedTexture = false;
	}

	if((bFreeActiveTexture == true) && (pActiveTexture != nullptr)) {
		SDL_DestroyTexture(pActiveTexture);
		pActiveTexture = nullptr;
		bFreeActiveTexture = false;
	}
}
