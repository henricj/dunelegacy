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

#include "misc/DrawingRectHelper.h"
#include "misc/dune_events.h"
#include <GUI/GUIStyle.h>
#include <globals.h>

Window::Window(uint32_t x, uint32_t y, uint32_t w, uint32_t h) : position_(x, y) {

    Window::resize(w, h);
}

Window::~Window() {
    if (pChildWindow_ != nullptr) {
        Window::closeChildWindow();
        processChildWindowOpenCloses();
    }

    if (pWindowWidget_ != nullptr) {
        pWindowWidget_->destroy();
    }
}

void Window::openWindow(Window* pChildWindow) {
    if (pChildWindow == nullptr) {
        return;
    }

    if (this->pChildWindow_ != nullptr) {
        queuedChildWindows_.push(pChildWindow);
        closeChildWindow();
    } else {
        this->pChildWindow_ = pChildWindow;
        this->pChildWindow_->setParent(this);
        pChildWindowAlreadyClosed_ = false;
    }
}

void Window::closeChildWindow() {
    if (pChildWindow_ != nullptr && !pChildWindowAlreadyClosed_) {
        closeChildWindowCounter_++;
        pChildWindow_->setEnabled(false);
        pChildWindow_->setVisible(false);
        pChildWindowAlreadyClosed_ = true;
    }
}

void Window::setCurrentPosition(int32_t x, int32_t y) {
    position_.x = x;
    position_.y = y;
}

void Window::setCurrentPosition(int32_t x, int32_t y, int32_t w, int32_t h) {
    position_.x = x;
    position_.y = y;
    if (size_.x != w || size_.y != h)
        resize(w, h);
}

bool Window::processChildWindowOpenCloses() {
    bool bClosed = false;

    while (closeChildWindowCounter_ > 0) {
        onChildWindowClose(pChildWindow_);
        pChildWindow_->destroy();
        closeChildWindowCounter_--;
        bClosed = true;

        if (!queuedChildWindows_.empty()) {
            pChildWindow_ = queuedChildWindows_.front();
            queuedChildWindows_.pop();
            pChildWindow_->setParent(this);
            pChildWindowAlreadyClosed_ = false;
        } else {
            pChildWindow_ = nullptr;
        }
    }

    return bClosed;
}

namespace {
const auto event_type_filter =
    std::unordered_set<Uint32>{SDL_WINDOWEVENT, SDL_DISPLAYEVENT_ORIENTATION, SDL_RENDER_DEVICE_RESET};
}

bool Window::isBroadcastEventType(Uint32 type) {
    return event_type_filter.contains(type);
}

void Window::handleInput(const SDL_Event& event) {
    if (pChildWindow_ != nullptr) {
        pChildWindow_->handleInput(event);

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

void Window::handleMouseMovement(int32_t x, int32_t y, [[maybe_unused]] bool insideOverlay) {
    if (pChildWindow_ != nullptr) {
        pChildWindow_->handleMouseMovement(x, y);
        return;
    }

    if (isEnabled() && (pWindowWidget_ != nullptr)) {
        const bool insideOverlay2 =
            pWindowWidget_->handleMouseMovementOverlay(x - getPosition().x, y - getPosition().y);
        pWindowWidget_->handleMouseMovement(x - getPosition().x, y - getPosition().y, insideOverlay2);
    }
}

bool Window::handleMouseLeft(int32_t x, int32_t y, bool pressed) {
    if (pChildWindow_ != nullptr) {
        return pChildWindow_->handleMouseLeft(x, y, pressed);
    }

    if (isEnabled() && (pWindowWidget_ != nullptr)) {
        const bool bProcessed =
            pWindowWidget_->handleMouseLeftOverlay(x - getPosition().x, y - getPosition().y, pressed)
            || pWindowWidget_->handleMouseLeft(x - getPosition().x, y - getPosition().y, pressed);
        if (pressed && (!bProcessed)) {
            pWindowWidget_->setActive(false);
            pWindowWidget_->setActive(true);
        }
        return bProcessed;
    }

    return false;
}

bool Window::handleMouseRight(int32_t x, int32_t y, bool pressed) {
    if (pChildWindow_ != nullptr) {
        return pChildWindow_->handleMouseRight(x, y, pressed);
    }

    if (isEnabled() && (pWindowWidget_ != nullptr)) {
        return pWindowWidget_->handleMouseRightOverlay(x - getPosition().x, y - getPosition().y, pressed)
            || pWindowWidget_->handleMouseRight(x - getPosition().x, y - getPosition().y, pressed);
    }
    return false;
}

bool Window::handleMouseWheel(int32_t x, int32_t y, bool up) {
    if (pChildWindow_ != nullptr) {
        return pChildWindow_->handleMouseWheel(x, y, up);
    }

    if (isEnabled() && (pWindowWidget_ != nullptr)) {
        return pWindowWidget_->handleMouseWheelOverlay(x - getPosition().x, y - getPosition().y, up)
            || pWindowWidget_->handleMouseWheel(x - getPosition().x, y - getPosition().y, up);
    }
    return false;
}

bool Window::handleKeyPress(const SDL_KeyboardEvent& key) {
    if (pChildWindow_ != nullptr) {
        return pChildWindow_->handleKeyPress(key);
    }

    if (isEnabled() && (pWindowWidget_ != nullptr)) {
        return pWindowWidget_->handleKeyPressOverlay(key) || pWindowWidget_->handleKeyPress(key);
    }
    return false;
}

bool Window::handleTextInput(const SDL_TextInputEvent& textInput) {
    if (pChildWindow_ != nullptr) {
        return pChildWindow_->handleTextInput(textInput);
    }

    if (isEnabled() && (pWindowWidget_ != nullptr)) {
        return pWindowWidget_->handleTextInputOverlay(textInput) || pWindowWidget_->handleTextInput(textInput);
    }
    return false;
}

void Window::draw(Point position) {
    if (isVisible()) {
        parent::draw(getPosition());

        if (pWindowWidget_ != nullptr) {
            pWindowWidget_->draw(Point(position.x + getPosition().x, position.y + getPosition().y));
        }
    }

    if (pChildWindow_ != nullptr) {
        pChildWindow_->draw();
    }
}

void Window::drawOverlay(Point position) {
    if (pChildWindow_ != nullptr) {
        pChildWindow_->drawOverlay();
    } else if (isVisible() && (pWindowWidget_ != nullptr)) {
        pWindowWidget_->drawOverlay(Point(position.x + getPosition().x, position.y + getPosition().y));
    }
}

void Window::setWindowWidget(Widget* widget) {
    this->pWindowWidget_ = widget;

    if (widget == nullptr)
        return;

    widget->setParent(this);
    widget->resize(getSize().x, getSize().y);
    widget->setActive();
}

void Window::resize(uint32_t width, uint32_t height) {
    parent::resize(width, height);

    if (pWindowWidget_ != nullptr)
        pWindowWidget_->resize(width, height);
}

void Window::resizeAll() {
    // Windows do not get bigger if content changes

    const auto size = getSize();

    resize(size.x, size.y);
}
