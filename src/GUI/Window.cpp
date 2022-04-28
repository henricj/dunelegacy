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

#include "misc/dune_events.h"
#include <GUI/GUIStyle.h>
#include <GUI/Window.h>
#include <globals.h>

Window::Window(uint32_t x, uint32_t y, uint32_t w, uint32_t h) : position(x, y) {

    Window::resize(w, h);
}

Window::~Window() {
    if (pChildWindow != nullptr) {
        Window::closeChildWindow();
        processChildWindowOpenCloses();
    }

    if (pWindowWidget != nullptr) {
        pWindowWidget->destroy();
    }
}

void Window::openWindow(Window* pChildWindow) {
    if (pChildWindow == nullptr) {
        return;
    }

    if (this->pChildWindow != nullptr) {
        queuedChildWindows.push(pChildWindow);
        closeChildWindow();
    } else {
        this->pChildWindow = pChildWindow;
        this->pChildWindow->setParent(this);
        pChildWindowAlreadyClosed = false;
    }
}

void Window::closeChildWindow() {
    if (pChildWindow != nullptr && !pChildWindowAlreadyClosed) {
        closeChildWindowCounter++;
        pChildWindow->setEnabled(false);
        pChildWindow->setVisible(false);
        pChildWindowAlreadyClosed = true;
    }
}

void Window::setCurrentPosition(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    position.x = x;
    position.y = y;
    resize(w, h);
}

bool Window::processChildWindowOpenCloses() {
    bool bClosed = false;

    while (closeChildWindowCounter > 0) {
        onChildWindowClose(pChildWindow);
        pChildWindow->destroy();
        closeChildWindowCounter--;
        bClosed = true;

        if (!queuedChildWindows.empty()) {
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
    if (pChildWindow != nullptr) {
        pChildWindow->handleInput(event);

        if (processChildWindowOpenCloses()) {
            // Small hack: simulate mouse movement to get rid of tooltips
            handleMouseMovement(0, 0);
        }

        return;
    }

    switch (event.type) {
        case SDL_KEYDOWN: {
            handleKeyPress(event.key);
        } break;

        case SDL_TEXTINPUT: {
            handleTextInput(event.text);
        } break;

        case SDL_MOUSEMOTION: {
            handleMouseMovement(event.motion.x, event.motion.y);
        } break;

        case SDL_MOUSEBUTTONDOWN: {
            switch (event.button.button) {
                case SDL_BUTTON_LEFT: {
                    handleMouseLeft(event.button.x, event.button.y, true);
                } break;

                case SDL_BUTTON_RIGHT: {
                    handleMouseRight(event.button.x, event.button.y, true);
                } break;
                default: break;
            }
        } break;

        case SDL_MOUSEWHEEL: {
            if (event.wheel.y != 0) {
                handleMouseWheel(dune::globals::drawnMouseX, dune::globals::drawnMouseY, (event.wheel.y > 0));
            }
        } break;

        case SDL_MOUSEBUTTONUP: {
            switch (event.button.button) {
                case SDL_BUTTON_LEFT: {
                    handleMouseLeft(event.button.x, event.button.y, false);
                } break;

                case SDL_BUTTON_RIGHT: {
                    handleMouseRight(event.button.x, event.button.y, false);
                } break;

                default: break;
            }
        } break;

        case SDL_WINDOWEVENT: {
            switch (event.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED: {
                    auto& gui = GUIStyle::getInstance();

                    gui.setLogicalSize(dune::globals::renderer.get(), event.window.data1, event.window.data2);

                    const auto actual = getRendererSize();

                    resize(actual.w, actual.h);

                    const auto size = getMinimumSize();

                    if (size.x > 0 && size.y > 0)
                        SDL_SetWindowMinimumSize(dune::globals::window.get(), size.x, size.y);
                } break;

                case SDL_WINDOWEVENT_DISPLAY_CHANGED: {
                    invalidateTextures();
                } break;

                default: break;
            }
        } break;

        case SDL_DISPLAYEVENT_ORIENTATION: [[fallthrough]];
        case SDL_RENDER_DEVICE_RESET: {
            invalidateTextures();
        } break;

        default: break;
    }
}

void Window::handleMouseMovement(int32_t x, int32_t y, bool insideOverlay) {
    if (pChildWindow != nullptr) {
        pChildWindow->handleMouseMovement(x, y);
        return;
    }

    if (isEnabled() && (pWindowWidget != nullptr)) {
        const bool insideOverlay = pWindowWidget->handleMouseMovementOverlay(x - getPosition().x, y - getPosition().y);
        pWindowWidget->handleMouseMovement(x - getPosition().x, y - getPosition().y, insideOverlay);
    }
}

bool Window::handleMouseLeft(int32_t x, int32_t y, bool pressed) {
    if (pChildWindow != nullptr) {
        return pChildWindow->handleMouseLeft(x, y, pressed);
    }

    if (isEnabled() && (pWindowWidget != nullptr)) {
        const bool bProcessed = pWindowWidget->handleMouseLeftOverlay(x - getPosition().x, y - getPosition().y, pressed)
                             || pWindowWidget->handleMouseLeft(x - getPosition().x, y - getPosition().y, pressed);
        if (pressed && (!bProcessed)) {
            pWindowWidget->setActive(false);
            pWindowWidget->setActive(true);
        }
        return bProcessed;
    }

    return false;
}

bool Window::handleMouseRight(int32_t x, int32_t y, bool pressed) {
    if (pChildWindow != nullptr) {
        return pChildWindow->handleMouseRight(x, y, pressed);
    }

    if (isEnabled() && (pWindowWidget != nullptr)) {
        return pWindowWidget->handleMouseRightOverlay(x - getPosition().x, y - getPosition().y, pressed)
            || pWindowWidget->handleMouseRight(x - getPosition().x, y - getPosition().y, pressed);
    }
    return false;
}

bool Window::handleMouseWheel(int32_t x, int32_t y, bool up) {
    if (pChildWindow != nullptr) {
        return pChildWindow->handleMouseWheel(x, y, up);
    }

    if (isEnabled() && (pWindowWidget != nullptr)) {
        return pWindowWidget->handleMouseWheelOverlay(x - getPosition().x, y - getPosition().y, up)
            || pWindowWidget->handleMouseWheel(x - getPosition().x, y - getPosition().y, up);
    }
    return false;
}

bool Window::handleKeyPress(SDL_KeyboardEvent& key) {
    if (pChildWindow != nullptr) {
        return pChildWindow->handleKeyPress(key);
    }

    if (isEnabled() && (pWindowWidget != nullptr)) {
        return pWindowWidget->handleKeyPressOverlay(key) || pWindowWidget->handleKeyPress(key);
    }
    return false;
}

bool Window::handleTextInput(SDL_TextInputEvent& textInput) {
    if (pChildWindow != nullptr) {
        return pChildWindow->handleTextInput(textInput);
    }

    if (isEnabled() && (pWindowWidget != nullptr)) {
        return pWindowWidget->handleTextInputOverlay(textInput) || pWindowWidget->handleTextInput(textInput);
    }
    return false;
}

void Window::draw(Point position) {
    if (isVisible()) {
        parent::draw(getPosition());

        if (pWindowWidget != nullptr) {
            pWindowWidget->draw(Point(position.x + getPosition().x, position.y + getPosition().y));
        }
    }

    if (pChildWindow != nullptr) {
        pChildWindow->draw();
    }
}

void Window::drawOverlay(Point position) {
    if (pChildWindow != nullptr) {
        pChildWindow->drawOverlay();
    } else if (isVisible() && (pWindowWidget != nullptr)) {
        pWindowWidget->drawOverlay(Point(position.x + getPosition().x, position.y + getPosition().y));
    }
}

void Window::setWindowWidget(Widget* widget) {
    this->pWindowWidget = widget;

    if (widget == nullptr)
        return;

    widget->setParent(this);
    widget->resize(getSize().x, getSize().y);
    widget->setActive();
}

void Window::resize(uint32_t width, uint32_t height) {
    parent::resize(width, height);

    if (pWindowWidget != nullptr)
        pWindowWidget->resize(width, height);
}

void Window::resizeAll() {
    // Windows do not get bigger if content changes

    const auto size = getSize();

    resize(size.x, size.y);
}
