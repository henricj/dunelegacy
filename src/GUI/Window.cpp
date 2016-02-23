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

#include <GUI/Window.h>
#include <GUI/GUIStyle.h>

Window::Window(Uint32 x, Uint32 y, Uint32 w, Uint32 h) : Widget() {
    closeChildWindowCounter = 0;
	pChildWindow = NULL;
	pChildWindowAlreadyClosed = false;
	pWindowWidget = NULL;

	bTransparentBackground = false;
	pBackground = NULL;
	bFreeBackground = false;
	bSelfGeneratedBackground = true;

	position = Point(x,y);
	Widget::resize(w,h);
}

Window::~Window() {
	if(pChildWindow != NULL) {
		closeChildWindow();
		processChildWindowOpenCloses();
	}

	if(pWindowWidget != NULL) {
		pWindowWidget->destroy();
	}

	if(((bSelfGeneratedBackground == true) || (bFreeBackground == true)) && (pBackground != NULL)) {
		SDL_FreeSurface(pBackground);
		pBackground = NULL;
	}
}

void Window::openWindow(Window* pChildWindow) {
    if(pChildWindow == NULL) {
        return;
    }

    if(this->pChildWindow != NULL) {
        queuedChildWindows.push(pChildWindow);
        closeChildWindow();
    } else {
        this->pChildWindow = pChildWindow;
        this->pChildWindow->setParent(this);
        pChildWindowAlreadyClosed = false;
    }
}

void Window::closeChildWindow() {
    if(pChildWindow != NULL && !pChildWindowAlreadyClosed) {
        closeChildWindowCounter++;
        pChildWindow->setEnabled(false);
        pChildWindow->setVisible(false);
        pChildWindowAlreadyClosed = true;
    }
}

void Window::setCurrentPosition(Uint32 x, Uint32 y, Uint32 w, Uint32 h) {
	position.x = x; position.y = y;
	resize(w,h);
}

bool Window::processChildWindowOpenCloses() {
    bool bClosed = false;

    while(closeChildWindowCounter > 0) {
        onChildWindowClose(pChildWindow);
        pChildWindow->destroy();
        closeChildWindowCounter--;
        bClosed = true;

        if(!queuedChildWindows.empty()) {
            pChildWindow = queuedChildWindows.front();
            queuedChildWindows.pop();
            pChildWindow->setParent(this);
            pChildWindowAlreadyClosed = false;
        } else {
            pChildWindow = NULL;
        }

    }

    return bClosed;
}

void Window::handleInput(SDL_Event& event) {
	if(pChildWindow != NULL) {
		pChildWindow->handleInput(event);

        if(processChildWindowOpenCloses()) {
            // Small hack: simulate mouse movement to get rid of tooltips
            handleMouseMovement(0, 0);
        }

		return;
	}

	switch(event.type) {
		case SDL_KEYDOWN: {
			handleKeyPress(event.key);
		} break;

		case SDL_MOUSEMOTION: {
			handleMouseMovement(event.motion.x,event.motion.y);
		} break;

		case SDL_MOUSEBUTTONDOWN: {
			switch(event.button.button) {
				case SDL_BUTTON_LEFT: {
					handleMouseLeft(event.button.x,event.button.y,true);
				} break;

				case SDL_BUTTON_RIGHT: {
					handleMouseRight(event.button.x,event.button.y,true);
				} break;
			}
		} break;

		case SDL_MOUSEWHEEL: {
            if(event.wheel.y != 0) {
				handleMouseWheel(event.wheel.x,event.wheel.y,(event.wheel.y > 0));
            }
		} break;

		case SDL_MOUSEBUTTONUP: {
			switch(event.button.button) {
				case SDL_BUTTON_LEFT: {
					handleMouseLeft(event.button.x,event.button.y,false);
				} break;

				case SDL_BUTTON_RIGHT: {
					handleMouseRight(event.button.x,event.button.y,false);
				} break;

			}
		} break;
	}
}

void Window::handleMouseMovement(Sint32 x, Sint32 y, bool insideOverlay) {
	if(pChildWindow != NULL) {
		pChildWindow->handleMouseMovement(x, y);
		return;
	}

	if(isEnabled() && (pWindowWidget != NULL)) {
	    bool insideOverlay = pWindowWidget->handleMouseMovementOverlay(x - getPosition().x, y - getPosition().y);
		pWindowWidget->handleMouseMovement(x - getPosition().x, y - getPosition().y, insideOverlay);
	}
}

bool Window::handleMouseLeft(Sint32 x, Sint32 y, bool pressed) {
	if(pChildWindow != NULL) {
		return pChildWindow->handleMouseLeft(x, y, pressed);
	}

	if(isEnabled() && (pWindowWidget != NULL)) {
	    bool bProcessed = pWindowWidget->handleMouseLeftOverlay(x - getPosition().x, y - getPosition().y, pressed)
                            || pWindowWidget->handleMouseLeft(x - getPosition().x, y - getPosition().y, pressed);
        if(pressed && (bProcessed == false)) {
            pWindowWidget->setActive(false);
            pWindowWidget->setActive(true);
        }
        return bProcessed;
	} else {
		return false;
	}
}

bool Window::handleMouseRight(Sint32 x, Sint32 y, bool pressed) {
	if(pChildWindow != NULL) {
		return pChildWindow->handleMouseRight(x, y, pressed);
	}

	if(isEnabled() && (pWindowWidget != NULL)) {
		return pWindowWidget->handleMouseRightOverlay(x - getPosition().x, y - getPosition().y, pressed)
				|| pWindowWidget->handleMouseRight(x - getPosition().x, y - getPosition().y, pressed);
	} else {
		return false;
	}
}

bool Window::handleMouseWheel(Sint32 x, Sint32 y, bool up)  {
	if(pChildWindow != NULL) {
		return pChildWindow->handleMouseWheel(x, y, up);
	}

	if(isEnabled() && (pWindowWidget != NULL)) {
		return pWindowWidget->handleMouseWheelOverlay(x - getPosition().x, y - getPosition().y, up)
				|| pWindowWidget->handleMouseWheel(x - getPosition().x, y - getPosition().y, up);
	} else {
		return false;
	}
}

bool Window::handleKeyPress(SDL_KeyboardEvent& key) {
	if(pChildWindow != NULL) {
		return pChildWindow->handleKeyPress(key);
	}

	if(isEnabled() && (pWindowWidget != NULL)) {
		return pWindowWidget->handleKeyPressOverlay(key) || pWindowWidget->handleKeyPress(key);
	} else {
		return false;
	}
}

void Window::draw(SDL_Surface* screen, Point position) {
	if(isVisible()) {
		if(bTransparentBackground == false) {

			if((bSelfGeneratedBackground == true) && (pBackground == NULL)) {
				pBackground = GUIStyle::getInstance().createBackground(getSize().x,getSize().y);

			}

			if(pBackground != NULL) {
				// Draw background
				SDL_Rect dest = calcDrawingRect(pBackground, getPosition().x + getSize().x/2, getPosition().y + getSize().y/2, HAlign::Center, VAlign::Center);
				SDL_BlitSurface(pBackground,NULL,screen,&dest);
			}
		}

		if(pWindowWidget != NULL) {
			pWindowWidget->draw(screen, Point(position.x+getPosition().x,position.y+getPosition().y));
		}
	}

	if(pChildWindow != NULL) {
		pChildWindow->draw(screen);
	}
}

void Window::drawOverlay(SDL_Surface* screen, Point position) {
	if(pChildWindow != NULL) {
		pChildWindow->drawOverlay(screen);
	} else if(isVisible() && (pWindowWidget != NULL)) {
		pWindowWidget->drawOverlay(screen, Point(position.x+getPosition().x,position.y+getPosition().y));
	}
}

void Window::resize(Uint32 width, Uint32 height) {
	Widget::resize(width,height);
	if(pWindowWidget != NULL) {
		pWindowWidget->resize(width,height);
	}

	if(bSelfGeneratedBackground == true) {
		if(pBackground != NULL) {
			SDL_FreeSurface(pBackground);
			pBackground = NULL;
		}

		// the new background is created when the window is drawn next time
	}
}

void Window::setBackground(SDL_Surface* pBackground, bool bFreeBackground) {
	if(((bSelfGeneratedBackground == true) || (this->bFreeBackground == true)) && (this->pBackground != NULL)) {
		SDL_FreeSurface(this->pBackground);
		this->pBackground = NULL;
	}

	if(pBackground == NULL) {
		bSelfGeneratedBackground = true;
		this->bFreeBackground = false;
		this->pBackground = NULL;
	} else {
		bSelfGeneratedBackground = false;
		this->pBackground = pBackground;
		this->bFreeBackground = bFreeBackground;
	}
}

void Window::setTransparentBackground(bool bTransparent) {
	bTransparentBackground = bTransparent;
}
