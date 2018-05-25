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
#include <misc/draw_util.h>
#include <globals.h>

Window::Window(Uint32 x, Uint32 y, Uint32 w, Uint32 h) : position(x,y) {
    closeChildWindowCounter = 0;
    pChildWindow = nullptr;
    pChildWindowAlreadyClosed = false;
    pWindowWidget = nullptr;

    bTransparentBackground = false;
    pBackground = nullptr;
    bSelfGeneratedBackground = true;

    resize(w,h);
}

Window::~Window() {
    if(pChildWindow != nullptr) {
        closeChildWindow();
        processChildWindowOpenCloses();
    }

    if(pWindowWidget != nullptr) {
        pWindowWidget->destroy();
    }
}

void Window::openWindow(Window* pChildWindow) {
    if(pChildWindow == nullptr) {
        return;
    }

    if(this->pChildWindow != nullptr) {
        queuedChildWindows.push(pChildWindow);
        closeChildWindow();
    } else {
        this->pChildWindow = pChildWindow;
        this->pChildWindow->setParent(this);
        pChildWindowAlreadyClosed = false;
    }
}

void Window::closeChildWindow() {
    if(pChildWindow != nullptr && !pChildWindowAlreadyClosed) {
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
            pChildWindow = nullptr;
        }

    }

    return bClosed;
}

void Window::handleInput(SDL_Event& event) {
    if(pChildWindow != nullptr) {
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

        case SDL_TEXTINPUT: {
            handleTextInput(event.text);
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
                handleMouseWheel(drawnMouseX,drawnMouseY,(event.wheel.y > 0));
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
    if(pChildWindow != nullptr) {
        pChildWindow->handleMouseMovement(x, y);
        return;
    }

    if(isEnabled() && (pWindowWidget != nullptr)) {
        bool insideOverlay = pWindowWidget->handleMouseMovementOverlay(x - getPosition().x, y - getPosition().y);
        pWindowWidget->handleMouseMovement(x - getPosition().x, y - getPosition().y, insideOverlay);
    }
}

bool Window::handleMouseLeft(Sint32 x, Sint32 y, bool pressed) {
    if(pChildWindow != nullptr) {
        return pChildWindow->handleMouseLeft(x, y, pressed);
    }

    if(isEnabled() && (pWindowWidget != nullptr)) {
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
    if(pChildWindow != nullptr) {
        return pChildWindow->handleMouseRight(x, y, pressed);
    }

    if(isEnabled() && (pWindowWidget != nullptr)) {
        return pWindowWidget->handleMouseRightOverlay(x - getPosition().x, y - getPosition().y, pressed)
                || pWindowWidget->handleMouseRight(x - getPosition().x, y - getPosition().y, pressed);
    } else {
        return false;
    }
}

bool Window::handleMouseWheel(Sint32 x, Sint32 y, bool up)  {
    if(pChildWindow != nullptr) {
        return pChildWindow->handleMouseWheel(x, y, up);
    }

    if(isEnabled() && (pWindowWidget != nullptr)) {
        return pWindowWidget->handleMouseWheelOverlay(x - getPosition().x, y - getPosition().y, up)
                || pWindowWidget->handleMouseWheel(x - getPosition().x, y - getPosition().y, up);
    } else {
        return false;
    }
}

bool Window::handleKeyPress(SDL_KeyboardEvent& key) {
    if(pChildWindow != nullptr) {
        return pChildWindow->handleKeyPress(key);
    }

    if(isEnabled() && (pWindowWidget != nullptr)) {
        return pWindowWidget->handleKeyPressOverlay(key) || pWindowWidget->handleKeyPress(key);
    } else {
        return false;
    }
}

bool Window::handleTextInput(SDL_TextInputEvent& textInput) {
    if(pChildWindow != nullptr) {
        return pChildWindow->handleTextInput(textInput);
    }

    if(isEnabled() && (pWindowWidget != nullptr)) {
        return pWindowWidget->handleTextInputOverlay(textInput) || pWindowWidget->handleTextInput(textInput);
    } else {
        return false;
    }
}

void Window::draw(Point position) {
    if(isVisible()) {
        if(bTransparentBackground == false) {

            if(bSelfGeneratedBackground && !pBackground) {
                pBackground = convertSurfaceToTexture(GUIStyle::getInstance().createBackground(getSize().x,getSize().y));
            }

            if(pBackground) {
                // Draw background
                SDL_Rect dest = calcDrawingRect(pBackground.get(), getPosition().x + getSize().x/2, getPosition().y + getSize().y/2, HAlign::Center, VAlign::Center);
                SDL_RenderCopy(renderer, pBackground.get(), nullptr, &dest);
            }
        }

        if(pWindowWidget != nullptr) {
            pWindowWidget->draw(Point(position.x+getPosition().x,position.y+getPosition().y));
        }
    }

    if(pChildWindow != nullptr) {
        pChildWindow->draw();
    }
}

void Window::drawOverlay(Point position) {
    if(pChildWindow != nullptr) {
        pChildWindow->drawOverlay();
    } else if(isVisible() && (pWindowWidget != nullptr)) {
        pWindowWidget->drawOverlay(Point(position.x+getPosition().x,position.y+getPosition().y));
    }
}

void Window::resize(Uint32 width, Uint32 height) {
    Widget::resize(width,height);
    if(pWindowWidget != nullptr) {
        pWindowWidget->resize(width,height);
    }

    if(bSelfGeneratedBackground == true) {
        pBackground.reset();

        // the new background is created when the window is drawn next time
    }
}

void Window::setBackground(sdl2::surface_unique_or_nonowning_ptr pBackground) {
    if(!pBackground) {
        setBackground(sdl2::texture_unique_or_nonowning_ptr(nullptr));
    } else {
        setBackground(convertSurfaceToTexture(pBackground.get()));
    }
}

void Window::setBackground(sdl2::texture_unique_or_nonowning_ptr pBackground) {
    if(!pBackground) {
        bSelfGeneratedBackground = true;
        this->pBackground = nullptr;
    } else {
        bSelfGeneratedBackground = false;
        this->pBackground = std::move(pBackground);
    }
}

void Window::setTransparentBackground(bool bTransparent) {
    bTransparentBackground = bTransparent;
}
