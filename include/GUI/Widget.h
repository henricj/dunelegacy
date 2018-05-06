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

#ifndef WIDGET_H
#define WIDGET_H

#include <misc/DrawingRectHelper.h>
#include <misc/SDL2pp.h>

#include <functional>
#include <iostream>
#include <algorithm>

/// A point class for representing a point.
class Point {
public:
    /// Default constructor.
    Point() : x(0) , y(0) { ; };

    /** A constructor that initializes the point with x and y
        \param x the x coordinate
        \param y the y coordinate
    */
    Point(Sint32 x, Sint32 y) : x(x) , y(y) { ; };

    /** operator for adding two points.
        \param p the point to add
        \return the sum of both points
    */
    inline Point operator+(const Point& p) const {
        return Point(x+p.x,y+p.y);
    };

    /** operator for subtracting two points.
        \param p the point to add
        \return this point minus p
    */
    inline Point operator-(const Point& p) const {
        return Point(x-p.x,y-p.y);
    };

    /** operator for multiplying every both coordinates with an integer value.
        \param c the scalar to multiply with
        \return this point times c
    */
    inline Point operator*(Sint32 c) const {
        return Point(x*c,y*c);
    };

    /** operator for dividing every both coordinates by an integer value.
        \param c the scalar to devide by
        \return this point divided by c
    */
    inline Point operator/(Sint32 c) const {
        return Point(x/c,y/c);
    };

    /** operator for comparing two Points.
        \param op the other operator
        \return true if both coordinates are equal, false otherwise
    */
    inline bool operator==(const Point& op) const {
            return (x == op.x) && (y == op.y);
    }

    /// The x coordinate
    Sint32 x;

    /// The y coordinate
    Sint32 y;
};

/**
    Return the dimensions of the given surface
    \param  pSurface    the surface to measure
    \return the size of pSurface
*/
inline Point getSurfaceSize(SDL_Surface* pSurface) {
    return Point(pSurface->w, pSurface->h);
}

/**
    Return the dimensions of the given texture
    \param  pTexture    the texture to measure
    \return the size of pTexture
*/
inline Point getTextureSize(SDL_Texture* pTexture) {
    Point p;
    if(pTexture != nullptr) {
        SDL_QueryTexture(pTexture, nullptr, nullptr, &p.x, &p.y);
    }
    return p;
}

// forward declarations
class Window;
template<class WidgetData> class Container;

/// The abstract base class for all widgets
class Widget
{
public:
    template<class WidgetData> friend class Container;

    friend class Window;

    /** The default constructor.
    */
    Widget() {
        parent = nullptr;
        enabled = true;
        visible = true;
        active = false;
        size.x = size.y = 0;
        resizeX = resizeY = false;
        pAllocated = false;
    }

    /** Destructor
        Removes this widget from its parent.
    */
    virtual ~Widget() {
        pAllocated = false;
        if(parent != nullptr) {
            parent->removeChildWidget(this);
        }
    }

    Widget(const Widget &) = default;
    Widget(Widget &&) = default;
    Widget& operator=(const Widget &) = delete;
    Widget& operator=(Widget &&) = default;

    /**
        Sets the parent of this widget.
        \param pParent new parent (nullptr is also possible)
    */
    inline void setParent(Widget* pParent) { parent = pParent; };

    /**
        Returns the parent of this widget
        \return the parent of this widget or nullptr if it has no parent.
    */
    inline Widget* getParent() const { return parent; };

    /**
        Enable or disable this widget. A disabled widget is not responding
        to clicks and key strokes and might look different.
        \param  bEnabled    true = enable widget, false = disable widget
    */
    virtual inline void setEnabled(bool bEnabled) {
        if((bEnabled == false) && (isActive() == true)) {
            setInactive();
        }
        enabled = bEnabled;
    };

    /**
        Returns whether this widget is enabled or not.
        \return true = widget is enabled, false = widget is disabled
    */
    inline bool isEnabled() const { return enabled; };

    /**
        Sets this widget visible or invisible. An invisible widget does not
        responding to clicks and key presses.
        \return bVisible    true = visible, false = invisible
    */
    virtual inline void setVisible(bool bVisible) { visible = bVisible; };

    /**
        Returns whether this widget is visible or not.
        \return true = visible, false = invisible
    */
    inline bool isVisible() const { return visible; };

    /**
        Sets this widget active. The parent widgets are also activated and the
        currently active widget is set to inactive.
    */
    virtual void setActive();

    /**
        Sets this widget inactive. The next activatable widget is activated.
    */
    virtual void setInactive();

    /**
        Returns whether this widget is currently active.
        \return true = currently active, false = currently inactive
    */
    inline bool isActive() const { return active; };

    /**
        Returns whether this widget can be set active.
        \return true = activatable, false = not activatable
    */
    virtual inline bool isActivatable() const { return false; };


    /**
        Returns whether this widget is an container.
        \return true = container, false = any other widget
    */
    virtual inline bool isContainer() const { return false; };

    /**
        Returns the current size of this widget.
        \return current size of this widget
    */
    inline const Point& getSize() const { return size; };

    /**
        Returns the minimum size of this widget. The widget should not
        resized to a size smaller than this. If the widget is not resizeable
        in a direction this method returns the size in that direction.
        \return the minimum size of this widget
    */
    virtual Point getMinimumSize() const { return Point(0,0); };

    /**
        Returns whether this widget is allowed to be resized in X direction.
        You should not resize a widget, that is not resizeable.
        \return true = resizeable in X direction, false = not resizeable in X direction
    */
    inline bool resizingXAllowed() const { return resizeX; };

    /**
        Returns whether this widget is allowed to be resized in Y direction.
        You should not resize a widget, that is not resizeable.
        \return true = resizeable in Y direction, false = not resizeable in Y direction
    */
    inline bool resizingYAllowed() const { return resizeY; };

    /**
        This method resizes the widget. This method should only be
        called if the new size is a valid size for this widget (See resizingXAllowed,
        resizingYAllowed, getMinumumSize).
        \param  newSize the new size of this widget
    */
    virtual inline void resize(Point newSize) {
        resize(newSize.x, newSize.y);
    };

    /**
        This method resizes the widget to width and height. This method should only be
        called if the new size is a valid size for this widget (See resizingXAllowed,
        resizingYAllowed, getMinumumSize).
        \param  width   the new width of this widget
        \param  height  the new height of this widget
    */
    virtual inline void resize(Uint32 width, Uint32 height) {
        size.x = width;
        size.y = height;
    };

    /**
        This method resizes the widget and its parent (the surrounding container).
    */
    virtual void resizeAll() {
        if(parent != nullptr) {
            parent->resizeAll();
        } else {
            resize( std::max(getMinimumSize().x,getSize().x),
                    std::max(getMinimumSize().y,getSize().y));
        }
    };

    /**
        Handles a mouse movement.
        \param  x               x-coordinate (relative to the left top corner of the widget)
        \param  y               y-coordinate (relative to the left top corner of the widget)
        \param  insideOverlay   true, if (x,y) is inside an overlay and this widget may be behind it, false otherwise
    */
    virtual inline void handleMouseMovement(Sint32 x, Sint32 y, bool insideOverlay) { ; };

    /**
        Handles a left mouse click.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the widget, false = click was not processed by the widget
    */
    virtual inline bool handleMouseLeft(Sint32 x, Sint32 y, bool pressed) { return false; };

    /**
        Handles a right mouse click.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the widget, false = click was not processed by the widget
    */
    virtual inline bool handleMouseRight(Sint32 x, Sint32 y, bool pressed) { return false; };

    /**
        Handles mouse wheel scrolling.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  up  true = mouse wheel up, false = mouse wheel down
        \return true = the mouse wheel scrolling was processed by the widget, false = mouse wheel scrolling was not processed by the widget
    */
    virtual inline bool handleMouseWheel(Sint32 x, Sint32 y, bool up) { return false; };

    /**
        Handles a key stroke.
        \param  key the key that was pressed or released.
        \return true = key stroke was processed by the widget, false = key stroke was not processed by the widget
    */
    virtual inline bool handleKeyPress(SDL_KeyboardEvent& key) {
        if(isActive() && (key.keysym.sym == SDLK_TAB)) {
            setInactive();
        }
        return false;
    }

    /**
        Handles a text input event.
        \param  textInput the text input that was performed.
        \return true = text input was processed by the widget, false = text input was not processed by the widget
    */
    virtual inline bool handleTextInput(SDL_TextInputEvent& textInput) { return false; }

    /**
        Handles mouse movement in overlays.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \return true if (x,y) is in overlay of this widget, false otherwise
    */
    virtual inline bool handleMouseMovementOverlay(Sint32 x, Sint32 y) { return false; };

    /**
        Handles a left mouse click in overlays.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the widget, false = click was not processed by the widget
    */
    virtual inline bool handleMouseLeftOverlay(Sint32 x, Sint32 y, bool pressed) { return false; };

    /**
        Handles a right mouse click in overlays.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the widget, false = click was not processed by the widget
    */
    virtual inline bool handleMouseRightOverlay(Sint32 x, Sint32 y, bool pressed) { return false; };

    /**
        Handles mouse wheel scrolling in overlays.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  up  true = mouse wheel up, false = mouse wheel down
        \return true = the mouse wheel scrolling was processed by the widget, false = mouse wheel scrolling was not processed by the widget
    */
    virtual inline bool handleMouseWheelOverlay(Sint32 x, Sint32 y, bool up) { return false; };

    /**
        Handles a key stroke in overlays.
        \param  key the key that was pressed or released.
        \return true = key stroke was processed by the widget, false = key stroke was not processed by the widget
    */
    virtual inline bool handleKeyPressOverlay(SDL_KeyboardEvent& key) { return false; }

    /**
        Handles a text input event in overlays.
        \param  textInput the text input that was performed.
        \return true = text input was processed by the widget, false = text input was not processed by the widget
    */
    virtual inline bool handleTextInputOverlay(SDL_TextInputEvent& textInput) { return false; }


    /**
        Draws this widget to screen. This method is called before drawOverlay().
        \param  Position    Position to draw the widget to
    */
    virtual inline void draw(Point position) { ; };

    /**
        This method draws the parts of this widget that must be drawn after all the other
        widgets are drawn (e.g. tooltips). This method is called after draw().
        \param  Position    Position to draw the widget to
    */
    virtual inline void drawOverlay(Point position) { ; };


    /**
        This method is called if a child widget is destroyed (see Widget::~Widget).
        \param  pChildWidget    widget to remove
    */
    virtual void removeChildWidget(Widget* pChildWidget) {
        ;
    }

    /**
        This method is called if the widget is removed from a container. If
        the widget was created via a named constructor (static create method) then this
        method automatically frees the memory of this object.
    */
    virtual void destroy() {
        if(pAllocated == true) {
            pAllocated = false;
            delete this;
        } else {
            if(parent != nullptr) {
                parent->removeChildWidget(this);
            }
        }
    };

    /**
        Sets the function that should be called when this widget gains focus.
        \param  pOnGainFocus    A function to call on focus gain
    */
    inline void setOnGainFocus(std::function<void ()> pOnGainFocus) {
        this->pOnGainFocus = pOnGainFocus;
    }

    /**
        Sets the function that should be called when this widget loses focus.
        \param  pOnLostFocus    A function to call on focus loss
    */
    inline void setOnLostFocus(std::function<void ()> pOnLostFocus) {
        this->pOnLostFocus = pOnLostFocus;
    }

protected:
    /**
        This method is called by containers to enable a widget or disable a widget explicitly.
        It is the responsibility of the container to take care that there is only one active
        widget.
        \param  bActive true = activate this widget, false = deactiviate this widget
    */
    virtual void inline setActive(bool bActive) {
        bool oldActive = active;
        active = bActive;

        if(oldActive != bActive) {
            if(active && pOnGainFocus) {
                pOnGainFocus();
            } else if(!active && pOnLostFocus) {
                pOnLostFocus();
            }
        }
    };

    /**
        This method enables or disables resizing of a widget. By default resizing in both directions is not allowed
        and this method should be called by all constructors of a concrete widget that is resizible.
        \param  bResizeX    true = resizing in x direction allowed, false = resizing in x direction not allowed
        \param  bResizeY    true = resizing in y direction allowed, false = resizing in y direction not allowed
    */
    virtual inline void enableResizing(bool bResizeX, bool bResizeY) {
        resizeX = bResizeX;
        resizeY = bResizeY;
    };

    /**
        This method activates or deactivates one specific widget in this widget/container. It is mainly used
        by widgets that are activated/deactivated and have to inform their parent container.
        \param  active  true = activate, false = deactivate
        \param  childWidget the widget to activate/deactivate
    */
    virtual void setActiveChildWidget(bool active, Widget* childWidget) {
    }

    /**
        This method is called whenever the textures of this widget are needed, e.g. before drawing. This method
        should be overwritten by subclasses if they like to defer texture creation as long as possible.
        This method should first check whether a renewal of the textures is necessary.
    */
    virtual void updateTextures() {
    }

    /**
        This method frees all textures that are used by this widget
    */
    virtual void invalidateTextures() {
    }

    /// If this widget is created via a named constructor (static create method) then bAllocated is true
    bool pAllocated;

private:
    bool visible;   ///< Is this widget visible?
    bool enabled;   ///< Is this widget enabled?
    bool active;    ///< Is this widget active?
    bool resizeX;   ///< Is this widget resizable in X direction?
    bool resizeY;   ///< Is this widget resizable in Y direction?
    Point size;     ///< The size of this widget
    Widget* parent; ///< The parent widget

    std::function<void ()> pOnGainFocus;    ///< function that is called when this widget gains focus
    std::function<void ()> pOnLostFocus;    ///< function that is called when this widget loses focus

};


#endif //WIDGET_H
