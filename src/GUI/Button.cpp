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
	tooltipSurface = NULL;

	bPressed = false;
	bHover = false;
	bToggleButton = false;
	bToggleState = false;

	pUnpressedSurface = NULL;
	pPressedSurface = NULL;
	pActiveSurface = NULL;
	bFreeUnpressedSurface = false;
	bFreePressedSurface = false;
	bFreeActiveSurface = false;
}

Button::~Button() {
	freeSurfaces();

	if(tooltipSurface != NULL) {
		SDL_FreeSurface(tooltipSurface);
		tooltipSurface = NULL;
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
			soundPlayer->playSound(ButtonClick);
		}
	} else {
		// button released
		if(bPressed == true) {
			bPressed = false;
			if(bToggleButton) {
				bool oldState = getToggleState();
				setToggleState(!bToggleState);
				if(getToggleState() != oldState) {
					soundPlayer->playSound(ButtonClick);
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
				soundPlayer->playSound(ButtonClick);
			}
		} else {
			soundPlayer->playSound(ButtonClick);
		}

		if(pOnClick) {
			pOnClick();
		}
	}

	if((bToggleButton == false) && (SDL_GetModState() == KMOD_NONE) && (key.keysym.sym == SDLK_RETURN)) {
		soundPlayer->playSound(ButtonClick);

		if(pOnClick) {
			pOnClick();
		}
	}

	return true;
}

void Button::draw(SDL_Surface* screen, Point position) {
	if(isVisible() == false) {
		return;
	}

	SDL_Surface* surf;
	if(bToggleState == true) {
		if(pPressedSurface != NULL) {
			surf = pPressedSurface;
		} else {
			if(isActive() && pActiveSurface != NULL) {
				surf = pActiveSurface;
			} else {
				surf = pUnpressedSurface;
			}
		}
	} else {
		if(bPressed == true) {
			if(pPressedSurface != NULL) {
				surf = pPressedSurface;
			} else {
				if(isActive() && pActiveSurface != NULL) {
					surf = pActiveSurface;
				} else {
					surf = pUnpressedSurface;
				}
			}
		} else {
			if((isActive() || bHover) && pActiveSurface != NULL) {
				surf = pActiveSurface;
			} else {
				surf = pUnpressedSurface;
			}
		}
	}

	if(surf == NULL) {
		return;
	}

	SDL_Rect dest = { static_cast<Sint16>(position.x), static_cast<Sint16>(position.y), static_cast<Uint16>(surf->w), static_cast<Uint16>(surf->h) };
	SDL_BlitSurface(surf,NULL,screen,&dest);
}

void Button::drawOverlay(SDL_Surface* screen, Point Pos) {
	if(isVisible() && isEnabled() && (bHover == true)) {
		if(tooltipSurface != NULL) {
			if((SDL_GetTicks() - tooltipLastMouseMotion) > 750) {
				int x,y;
				SDL_GetMouseState(&x,&y);
				int win_w, win_h;
				SDL_GetWindowSize(window, &win_w, &win_h);
				SDL_Rect vp;
				SDL_RenderGetViewport(renderer, &vp);
				x = (x*vp.w)/win_w-vp.x;
				y = (y*vp.h)/win_h-vp.y;
				SDL_Rect dest = { static_cast<Sint16>(x), static_cast<Sint16>(y - tooltipSurface->h), static_cast<Uint16>(tooltipSurface->w), static_cast<Uint16>(tooltipSurface->h) };
				if(dest.x + dest.w >= screen->w) {
				    // do not draw tooltip outside screen
                    dest.x = screen->w - dest.w;
				}

				if(dest.y < 0) {
				    // do not draw tooltip outside screen
                    dest.y = 0;
				} else if(dest.y + dest.h >= screen->h) {
				    // do not draw tooltip outside screen
                    dest.y = screen->h - dest.h;
				}

				SDL_BlitSurface(tooltipSurface,NULL,screen,&dest);
			}
		}
	}
}

void Button::setSurfaces(	SDL_Surface* pUnpressedSurface,bool bFreeUnpressedSurface,
							SDL_Surface* pPressedSurface,bool bFreePressedSurface,
							SDL_Surface* pActiveSurface,bool bFreeActiveSurface) {
	freeSurfaces();
	this->pUnpressedSurface = pUnpressedSurface;
	this->bFreeUnpressedSurface = bFreeUnpressedSurface;
	this->pPressedSurface = pPressedSurface;
	this->bFreePressedSurface = bFreePressedSurface;
	this->pActiveSurface = pActiveSurface;
	this->bFreeActiveSurface = bFreeActiveSurface;
}

void Button::freeSurfaces() {
	if((bFreeUnpressedSurface == true) && (pUnpressedSurface != NULL)) {
		SDL_FreeSurface(pUnpressedSurface);
		pUnpressedSurface = NULL;
		bFreeUnpressedSurface = false;
	}

	if((bFreePressedSurface == true) && (pPressedSurface != NULL)) {
		SDL_FreeSurface(pPressedSurface);
		pPressedSurface = NULL;
		bFreePressedSurface = false;
	}

	if((bFreeActiveSurface == true) && (pActiveSurface != NULL)) {
		SDL_FreeSurface(pActiveSurface);
		pActiveSurface = NULL;
		bFreeActiveSurface = false;
	}
}
