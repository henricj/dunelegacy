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

#ifndef CONTAINER_H
#define CONTAINER_H

#include "Widget.h"

#include <algorithm>
#include <ranges>
#include <vector>

/// The abstract base class for container widgets
/**
    WidgetData is used for storing information for every widget added to the container. It must contain a pointer
   pWidget which stores the contained widget and can contain any additional data needed by the derived container class
   for managing the widget size and position (like weight factors, margins, etc)
*/
template<class WidgetData>
class Container : public Widget {
    using parent = Widget;

protected:
    using WidgetList = std::vector<WidgetData>;

    /// default constructor
    Container() : Widget() {
        pActiveChildWidget = nullptr;
        Container::enableResizing(true, true);
    }

public:
    /// destructor
    ~Container() override { clearWidgetList(); }

    /**
        This method removes a widget from this container. Everything
        will be resized afterwards.
        \param pChildWidget Widget to remove
    */
    void removeChildWidget(Widget* pChildWidget) override {
        auto it = std::ranges::find_if(containedWidgets,
                                       [pChildWidget](const auto& wd) { return wd.pWidget == pChildWidget; });

        if (it == std::end(containedWidgets))
            return;

        auto* widget = it->pWidget;

        setActiveChildWidget(false, widget);

        containedWidgets.erase(it);

        widget->setParent(nullptr);
        widget->destroy();

        resizeAll();
    }

    /**
        This method will remove all contained widgets in this container. Everything
        will be resized afterwards.
    */
    virtual void removeAllChildWidgets() { clearWidgetList(); }

    /**
        Handles a mouse movement.
        \param  x               x-coordinate (relative to the left top corner of the container)
        \param  y               y-coordinate (relative to the left top corner of the container)
        \param  insideOverlay   true, if (x,y) is inside an overlay and this container may be behind it, false otherwise
    */
    void handleMouseMovement(int32_t x, int32_t y, bool insideOverlay) override {
        for (const auto& widgetData : containedWidgets) {
            const Point pos = getPosition(widgetData);
            widgetData.pWidget->handleMouseMovement(x - pos.x, y - pos.y, insideOverlay);
        }
    }

    /**
        Handles a left mouse click.
        \param  x x-coordinate (relative to the left top corner of the container)
        \param  y y-coordinate (relative to the left top corner of the container)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the container, false = click was not processed by the container
    */
    bool handleMouseLeft(int32_t x, int32_t y, bool pressed) override {
        if (isEnabled() == false || isVisible() == false)
            return false;

        bool bWidgetFound = false;

        for (const auto& widgetData : containedWidgets) {
            const Point pos = getPosition(widgetData);
            if (widgetData.pWidget->isEnabled() && widgetData.pWidget->isVisible()) {
                bWidgetFound |= widgetData.pWidget->handleMouseLeft(x - pos.x, y - pos.y, pressed);
            }
        }

        return bWidgetFound;
    }

    /**
        Handles a right mouse click.
        \param  x x-coordinate (relative to the left top corner of the container)
        \param  y y-coordinate (relative to the left top corner of the container)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the container, false = click was not processed by the container
    */
    bool handleMouseRight(int32_t x, int32_t y, bool pressed) override {
        if (isEnabled() == false || isVisible() == false)
            return false;

        bool bWidgetFound = false;

        for (const auto& widgetData : containedWidgets) {
            const Point pos = getPosition(widgetData);
            bWidgetFound |= widgetData.pWidget->handleMouseRight(x - pos.x, y - pos.y, pressed);
        }

        return bWidgetFound;
    }

    /**
        Handles mouse wheel scrolling.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  up  true = mouse wheel up, false = mouse wheel down
        \return true = the mouse wheel scrolling was processed by the widget, false = mouse wheel scrolling was not
       processed by the widget
    */
    bool handleMouseWheel(int32_t x, int32_t y, bool up) override {
        if (isEnabled() == false || isVisible() == false)
            return false;

        bool bProcessed = false;
        for (const auto& widgetData : containedWidgets) {
            const Point pos = getPosition(widgetData);

            int childX = x - pos.x;
            int childY = y - pos.y;

            auto* widget = widgetData.pWidget;

            if (childX < 0 || childX >= widget->getSize().x || childY < 0 || childY >= widget->getSize().y)
                continue;

            bProcessed = widget->handleMouseWheel(childX, childY, up);
            if (bProcessed)
                return true;
        }

        if (pActiveChildWidget != nullptr) {
            const Point pos = getPosition(*getWidgetDataFromWidget(pActiveChildWidget));
            return pActiveChildWidget->handleMouseWheel(x - pos.x, y - pos.y, up);
        }

        return bProcessed;
    }

    /**
        Handles a key stroke.
        \param  key the key that was pressed or released.
        \return true = key stroke was processed by the container, false = key stroke was not processed by the container
    */
    bool handleKeyPress(const SDL_KeyboardEvent& key) override {
        if (isEnabled() == false || isVisible() == false || isActive() == false)
            return false;

        if (pActiveChildWidget != nullptr)
            return pActiveChildWidget->handleKeyPress(key);

        if (key.keysym.sym == SDLK_TAB) {
            activateFirstActivatableWidget();
            return true;
        }

        return false;
    }

    /**
        Handles a text input event.
        \param  textInput the text input that was performed.
        \return true = text input was processed by the container, false = text input was not processed by the container
    */
    bool handleTextInput(const SDL_TextInputEvent& textInput) override {
        if (isEnabled() == false || isVisible() == false || isActive() == false)
            return false;

        if (pActiveChildWidget != nullptr)
            return pActiveChildWidget->handleTextInput(textInput);

        return false;
    }

    /**
        Handles mouse movement in overlays.
        \param  x x-coordinate (relative to the left top corner of the container)
        \param  y y-coordinate (relative to the left top corner of the container)
        \return true if (x,y) is in overlay of this container, false otherwise
    */
    bool handleMouseMovementOverlay(int32_t x, int32_t y) override {
        bool insideOverlay = false;

        for (const auto& widgetData : containedWidgets) {
            const Point pos = getPosition(widgetData);
            insideOverlay |= widgetData.pWidget->handleMouseMovementOverlay(x - pos.x, y - pos.y);
        }

        return insideOverlay;
    }

    /**
        Handles a left mouse click in overlays.
        \param  x x-coordinate (relative to the left top corner of the container)
        \param  y y-coordinate (relative to the left top corner of the container)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the container, false = click was not processed by the container
    */
    bool handleMouseLeftOverlay(int32_t x, int32_t y, bool pressed) override {
        if (isEnabled() == false || isVisible() == false)
            return false;

        bool bWidgetFound = false;
        for (const auto& widgetData : containedWidgets) {
            const Point pos = getPosition(widgetData);
            bWidgetFound |= widgetData.pWidget->handleMouseLeftOverlay(x - pos.x, y - pos.y, pressed);
        }

        return bWidgetFound;
    }

    /**
        Handles a right mouse click in overlays.
        \param  x x-coordinate (relative to the left top corner of the container)
        \param  y y-coordinate (relative to the left top corner of the container)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the container, false = click was not processed by the container
    */
    bool handleMouseRightOverlay(int32_t x, int32_t y, bool pressed) override {
        if (isEnabled() == false || isVisible() == false)
            return false;

        bool bWidgetFound = false;

        for (const auto& widgetData : containedWidgets) {
            const Point pos = getPosition(widgetData);
            bWidgetFound |= widgetData.pWidget->handleMouseRightOverlay(x - pos.x, y - pos.y, pressed);
        }

        return bWidgetFound;
    }

    /**
        Handles mouse wheel scrolling in overlays.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  up  true = mouse wheel up, false = mouse wheel down
        \return true = the mouse wheel scrolling was processed by the widget, false = mouse wheel scrolling was not
       processed by the widget
    */
    bool handleMouseWheelOverlay(int32_t x, int32_t y, bool up) override {
        if (isEnabled() == false || isVisible() == false)
            return false;

        bool bProcessed = false;

        for (const auto& widgetData : containedWidgets) {
            const Point pos = getPosition(widgetData);

            const auto childX = x - pos.x;
            const auto childY = y - pos.y;

            bProcessed = widgetData.pWidget->handleMouseWheelOverlay(childX, childY, up);
            if (bProcessed)
                break;
        }

        if (bProcessed == false && pActiveChildWidget != nullptr) {
            const Point pos = getPosition(*getWidgetDataFromWidget(pActiveChildWidget));
            return pActiveChildWidget->handleMouseWheelOverlay(x - pos.x, y - pos.y, up);
        }

        return bProcessed;
    }

    /**
        Handles a key stroke in overlays.
        \param  key the key that was pressed or released.
        \return true = key stroke was processed by the container, false = key stroke was not processed by the container
    */
    bool handleKeyPressOverlay(const SDL_KeyboardEvent& key) override {
        if (isEnabled() == false || isVisible() == false || isActive() == false)
            return false;

        if (pActiveChildWidget != nullptr)
            return pActiveChildWidget->handleKeyPressOverlay(key);

        return false;
    }

    /**
        Handles a text input event in overlays.
        \param  textInput the text input that was performed.
        \return true = text input was processed by the container, false = text input was not processed by the container
    */
    bool handleTextInputOverlay(const SDL_TextInputEvent& textInput) override {
        if (isEnabled() == false || isVisible() == false || isActive() == false)
            return false;

        if (pActiveChildWidget != nullptr)
            return pActiveChildWidget->handleTextInputOverlay(textInput);

        return false;
    }

    /**
        Draws this container and it's children to screen. This method is called before drawOverlay().
        \param  position    Position to draw the container to
    */
    void draw(Point position) override {
        if (isVisible() == false)
            return;

        for (const auto& widgetData : containedWidgets) {
            widgetData.pWidget->draw(position + getPosition(widgetData));
        }
    }

    /**
        This method draws the parts of this container that must be drawn after all the other
        widgets are drawn (e.g. tooltips). This method is called after draw().
        \param  position    Position to draw the container to
    */
    void drawOverlay(Point position) override {
        if (isVisible() == false)
            return;

        for (const auto& widgetData : containedWidgets)
            widgetData.pWidget->drawOverlay(position + getPosition(widgetData));
    }

    /**
        Returns the position of widget relative to the top left corner of this container
        \param widget   the widget data to get the position from.
        \return The position of the left upper corner of widget or (-1,-1) if widget cannot be found in this container
    */
    virtual Point getWidgetPosition(const Widget* widget) {
        auto* widgetData = getWidgetDataFromWidget(widget);
        if (widgetData == nullptr)
            return {-1, -1};

        return getPosition(*widgetData);
    }

    /**
        Sets this container and its children active. The parent widgets are also activated and the
        currently widget is set to inactive.
    */
    void setActive() override {
        if (pActiveChildWidget == nullptr)
            activateFirstActivatableWidget();

        parent::setActive();
    }

    /**
        Sets this container and its children inactive. The next activatable widget is activated.
    */
    void setInactive() override {
        if (pActiveChildWidget != nullptr) {
            pActiveChildWidget->setActive(false);
            pActiveChildWidget = nullptr;
        }

        parent::setInactive();
    }

    /**
        Returns whether one of this container's children can be set active.
        \return true = activatable, false = not activatable
    */
    [[nodiscard]] bool isActivatable() const override {
        if (isEnabled() == false)
            return false;

        for (const auto& widgetData : containedWidgets) {
            if (widgetData.pWidget->isActivatable())
                return true;
        }

        return false;
    }

    /**
        Returns whether this widget is an container.
        \return true = container, false = any other widget
    */
    [[nodiscard]] bool isContainer() const override { return true; }

    /**
        This method frees all textures that are used by contained widgets
    */
    void invalidateTextures() override {
        for (const auto& wd : containedWidgets)
            wd.pWidget->invalidateTextures();

        parent::invalidateTextures();
    }

protected:
    /**
        This method is called by other containers to enable this container or disable this container explicitly.
        It is the responsibility of the calling container to take care that there is only one active
        widget.
        \param  bActive true = activate this widget, false = deactivate this widget
    */
    void setActive(bool bActive) override {
        if (pActiveChildWidget != nullptr) {
            pActiveChildWidget->setActive(bActive);
            if (bActive == false)
                pActiveChildWidget = nullptr;
        }

        parent::setActive(bActive);
    }

    /**
        This method activates or deactivates one specific widget in this container. It is mainly used
        by widgets that are activated/deactivated and have to inform their parent container.
        \param  active  true = activate, false = deactivate
        \param  childWidget the widget to activate/deactivate
    */
    void setActiveChildWidget(bool active, Widget* childWidget) override {
        if (childWidget == nullptr)
            return;

        if (active == true) {
            // deactivate current active widget
            if (pActiveChildWidget != nullptr && pActiveChildWidget != childWidget) {
                pActiveChildWidget->setActive(false);
                pActiveChildWidget = childWidget;
            } else {
                pActiveChildWidget = childWidget;

                // activate this container and upper containers
                parent::setActive();
            }
        } else {
            if (childWidget != pActiveChildWidget)
                return;

            // deactivate current active widget
            if (pActiveChildWidget != nullptr)
                pActiveChildWidget->setActive(false);

            // find childWidget in the widget list
            auto iter =
                std::ranges::find_if(containedWidgets, [childWidget](auto& wd) { return wd.pWidget == childWidget; });

            if (iter != std::end(containedWidgets)) {
                ++iter;

                for (; iter != containedWidgets.end(); ++iter) {
                    auto* const widget = iter->pWidget;
                    assert(widget);

                    if (!widget->isActivatable())
                        continue;

                    // activate next widget
                    pActiveChildWidget = widget;
                    pActiveChildWidget->setActive();
                    return;
                }
            }

            // we are at the end of the list
            if (getParent() != nullptr && getParent()->isContainer()) {
                pActiveChildWidget = nullptr;
                setInactive();
            } else {
                activateFirstActivatableWidget();
                setActive();
            }
        }
    }

    /**
        This method activates the first activatable widget in this container.
    */
    void activateFirstActivatableWidget() {
        for (const auto& widgetData : containedWidgets) {
            auto* widget = widgetData.pWidget;
            assert(widget);

            if (widget->isActivatable() != true)
                continue;

            // activate next widget
            pActiveChildWidget = widget;
            pActiveChildWidget->setActive();
            return;
        }
        pActiveChildWidget = nullptr;
    }

    /**
        This method must be overwritten by all container classes. It should return
        the position of the specified widget.
        \param widgetData   the widget data to get the position from.
        \return The position of the left upper corner
    */
    virtual Point getPosition(const WidgetData& widgetData) const = 0;

    /**
        This method returns from a Widget* the corresponding WidgetData.
        \param  pWidget the widget to look for
        \return a pointer to the WidgetData, nullptr if not found
    */
    WidgetData* getWidgetDataFromWidget(const Widget* pWidget) {
        auto it = std::ranges::find_if(containedWidgets, [pWidget](const auto& wd) { return wd.pWidget == pWidget; });

        if (it == std::end(containedWidgets))
            return nullptr;

        return &*it;
    }

    void clearWidgetList() {
        WidgetList widgets;

        containedWidgets.swap(widgets);

        for (auto& wd : widgets)
            wd.pWidget->destroy();
    }

    WidgetList containedWidgets;  ///< List of widgets
    Widget* pActiveChildWidget{}; ///< currently active widget
};

#endif // CONTAINER_H
