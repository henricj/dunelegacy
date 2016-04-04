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
#include <misc/RobustList.h>

/// The abstract base class for container widgets
/**
	WidgetData is used for storing information for every widget added to the container. It must contain a pointer pWidget which stores the contained widget and can contain any
	additional data needed by the derived container class for managing the widget size and position (like weight factors, margins, etc)
*/
template<class WidgetData> class Container : public Widget {
protected:
	typedef RobustList<WidgetData> WidgetList;

public:

	/// default constructor
	Container() : Widget() {
		pActiveChildWidget = nullptr;
		enableResizing(true,true);
	}

	/// destructor
	virtual ~Container() {
		while(containedWidgets.begin() != containedWidgets.end()) {
			Widget* curWidget = containedWidgets.front().pWidget;
			curWidget->destroy();
		}
	}

	/**
		This method removes a widget from this container. Everything
		will be resized afterwards.
		\param pChildWidget	Widget to remove
	*/
	virtual void removeChildWidget(Widget* pChildWidget) {
		for(typename WidgetList::iterator iter = containedWidgets.begin(); iter != containedWidgets.end(); ++iter) {
			if(iter->pWidget == pChildWidget) {
			    setActiveChildWidget(false, pChildWidget);
				containedWidgets.erase(iter);
                pChildWidget->setParent(nullptr);
                pChildWidget->destroy();
				break;
			}
		}
		resizeAll();
	}


    /**
		This method will remove all contained widgets in this container. Everything
		will be resized afterwards.
	*/
	virtual void removeAllChildWidgets() {
        while(containedWidgets.empty() == false) {
            removeChildWidget(containedWidgets.begin()->pWidget);
        }
	}

	/**
		Handles a mouse movement.
		\param	x               x-coordinate (relative to the left top corner of the container)
		\param	y               y-coordinate (relative to the left top corner of the container)
		\param  insideOverlay   true, if (x,y) is inside an overlay and this container may be behind it, false otherwise
	*/
	virtual void handleMouseMovement(Sint32 x, Sint32 y, bool insideOverlay) {

		typename WidgetList::const_iterator iter;
		for(iter = containedWidgets.begin(); iter != containedWidgets.end(); ++iter) {
			Widget* curWidget = iter->pWidget;
			Point pos = getPosition(*iter);
			curWidget->handleMouseMovement(x - pos.x, y - pos.y, insideOverlay);
		}
	}

	/**
		Handles a left mouse click.
		\param	x x-coordinate (relative to the left top corner of the container)
		\param	y y-coordinate (relative to the left top corner of the container)
		\param	pressed	true = mouse button pressed, false = mouse button released
		\return	true = click was processed by the container, false = click was not processed by the container
	*/
	virtual bool handleMouseLeft(Sint32 x, Sint32 y, bool pressed) {
		if((isEnabled() == false) || (isVisible() == false)) {
			return false;
		}

		bool bWidgetFound = false;
		typename WidgetList::const_iterator iter;
		for(iter = containedWidgets.begin(); iter != containedWidgets.end(); ++iter) {
			Widget* curWidget = iter->pWidget;
			Point pos = getPosition(*iter);
			bWidgetFound |= curWidget->handleMouseLeft(x - pos.x, y - pos.y, pressed);
		}
		return bWidgetFound;
	}

	/**
		Handles a right mouse click.
		\param	x x-coordinate (relative to the left top corner of the container)
		\param	y y-coordinate (relative to the left top corner of the container)
		\param	pressed	true = mouse button pressed, false = mouse button released
		\return	true = click was processed by the container, false = click was not processed by the container
	*/
	virtual bool handleMouseRight(Sint32 x, Sint32 y, bool pressed) {
		if((isEnabled() == false) || (isVisible() == false)) {
			return false;
		}

		bool bWidgetFound = false;
		typename WidgetList::const_iterator iter;
		for(iter = containedWidgets.begin(); iter != containedWidgets.end(); ++iter) {
			Widget* curWidget = iter->pWidget;
			Point pos = getPosition(*iter);
			bWidgetFound |= curWidget->handleMouseRight(x - pos.x, y - pos.y, pressed);
		}
		return bWidgetFound;
	}

	/**
		Handles mouse wheel scrolling.
		\param	x x-coordinate (relative to the left top corner of the widget)
		\param	y y-coordinate (relative to the left top corner of the widget)
		\param	up	true = mouse wheel up, false = mouse wheel down
		\return	true = the mouse wheel scrolling was processed by the widget, false = mouse wheel scrolling was not processed by the widget
	*/
	virtual inline bool handleMouseWheel(Sint32 x, Sint32 y, bool up)  {
		if((isEnabled() == false) || (isVisible() == false)) {
			return false;
		}

		bool bProcessed = false;
		typename WidgetList::const_iterator iter;
		for(iter = containedWidgets.begin(); iter != containedWidgets.end(); ++iter) {
			Widget* curWidget = iter->pWidget;
			Point pos = getPosition(*iter);

			int childX = x - pos.x;
			int childY = y - pos.y;

			if((childX >= 0) && (childX < curWidget->getSize().x)
				&& (childY >= 0) && (childY < curWidget->getSize().y)) {
				bProcessed = curWidget->handleMouseWheel(childX, childY, up);
				if(bProcessed) {
                    break;
				}
			}
		}

		if((bProcessed == false) && (pActiveChildWidget != nullptr)) {
			Point pos = getPosition(*getWidgetDataFromWidget(pActiveChildWidget));
			return pActiveChildWidget->handleMouseWheel(x - pos.x, y - pos.y, up);
		}

		return bProcessed;
	}

	/**
		Handles a key stroke.
		\param	key the key that was pressed or released.
		\return	true = key stroke was processed by the container, false = key stroke was not processed by the container
	*/
	virtual bool handleKeyPress(SDL_KeyboardEvent& key) {
		if((isEnabled() == false) || (isVisible() == false) || (isActive() == false)) {
			return false;
		}

		if(pActiveChildWidget != nullptr) {
			return pActiveChildWidget->handleKeyPress(key);
		} else {
			if(key.keysym.sym == SDLK_TAB) {
				activateFirstActivatableWidget();
				return true;
			} else {
                return false;
			}
		}
	}

	/**
		Handles a text input event.
		\param	textInput the text input that was performed.
		\return	true = text input was processed by the container, false = text input was not processed by the container
	*/
	virtual inline bool handleTextInput(SDL_TextInputEvent& textInput) {
		if((isEnabled() == false) || (isVisible() == false) || (isActive() == false)) {
			return false;
		}

		if(pActiveChildWidget != nullptr) {
			return pActiveChildWidget->handleTextInput(textInput);
		}
		return false;
	}

	/**
		Handles mouse movement in overlays.
		\param	x x-coordinate (relative to the left top corner of the container)
		\param	y y-coordinate (relative to the left top corner of the container)
		\return true if (x,y) is in overlay of this container, false otherwise
	*/
	virtual inline bool handleMouseMovementOverlay(Sint32 x, Sint32 y) {
	    bool insideOverlay = false;

		typename WidgetList::const_iterator iter;
		for(iter = containedWidgets.begin(); iter != containedWidgets.end(); ++iter) {
			Widget* curWidget = iter->pWidget;
			Point pos = getPosition(*iter);
			insideOverlay |= curWidget->handleMouseMovementOverlay(x - pos.x, y - pos.y);
		}

		return insideOverlay;
    };

	/**
		Handles a left mouse click in overlays.
		\param	x x-coordinate (relative to the left top corner of the container)
		\param	y y-coordinate (relative to the left top corner of the container)
		\param	pressed	true = mouse button pressed, false = mouse button released
		\return	true = click was processed by the container, false = click was not processed by the container
	*/
	virtual bool handleMouseLeftOverlay(Sint32 x, Sint32 y, bool pressed) {
		if((isEnabled() == false) || (isVisible() == false)) {
			return false;
		}

		bool bWidgetFound = false;
		typename WidgetList::const_iterator iter;
		for(iter = containedWidgets.begin(); iter != containedWidgets.end(); ++iter) {
			Widget* curWidget = iter->pWidget;
			Point pos = getPosition(*iter);
			bWidgetFound |= curWidget->handleMouseLeftOverlay(x - pos.x, y - pos.y, pressed);
		}
		return bWidgetFound;
	}

	/**
		Handles a right mouse click in overlays.
		\param	x x-coordinate (relative to the left top corner of the container)
		\param	y y-coordinate (relative to the left top corner of the container)
		\param	pressed	true = mouse button pressed, false = mouse button released
		\return	true = click was processed by the container, false = click was not processed by the container
	*/
	virtual bool handleMouseRightOverlay(Sint32 x, Sint32 y, bool pressed) {
		if((isEnabled() == false) || (isVisible() == false)) {
			return false;
		}

		bool bWidgetFound = false;
		typename WidgetList::const_iterator iter;
		for(iter = containedWidgets.begin(); iter != containedWidgets.end(); ++iter) {
			Widget* curWidget = iter->pWidget;
			Point pos = getPosition(*iter);
			bWidgetFound |= curWidget->handleMouseRightOverlay(x - pos.x, y - pos.y, pressed);
		}
		return bWidgetFound;
	}

	/**
		Handles mouse wheel scrolling in overlays.
		\param	x x-coordinate (relative to the left top corner of the widget)
		\param	y y-coordinate (relative to the left top corner of the widget)
		\param	up	true = mouse wheel up, false = mouse wheel down
		\return	true = the mouse wheel scrolling was processed by the widget, false = mouse wheel scrolling was not processed by the widget
	*/
	virtual inline bool handleMouseWheelOverlay(Sint32 x, Sint32 y, bool up)  {
		if((isEnabled() == false) || (isVisible() == false)) {
			return false;
		}

		bool bProcessed = false;
		typename WidgetList::const_iterator iter;
		for(iter = containedWidgets.begin(); iter != containedWidgets.end(); ++iter) {
			Widget* curWidget = iter->pWidget;
			Point pos = getPosition(*iter);

			int childX = x - pos.x;
			int childY = y - pos.y;

            bProcessed = curWidget->handleMouseWheelOverlay(childX, childY, up);
            if(bProcessed) {
                break;
            }
		}

		if((bProcessed == false) && (pActiveChildWidget != nullptr)) {
			Point pos = getPosition(*getWidgetDataFromWidget(pActiveChildWidget));
			return pActiveChildWidget->handleMouseWheelOverlay(x - pos.x, y - pos.y, up);
		}

		return bProcessed;
	}

	/**
		Handles a key stroke in overlays.
		\param	key the key that was pressed or released.
		\return	true = key stroke was processed by the container, false = key stroke was not processed by the container
	*/
	virtual bool handleKeyPressOverlay(SDL_KeyboardEvent& key) {
		if((isEnabled() == false) || (isVisible() == false) || (isActive() == false)) {
			return false;
		}

		if(pActiveChildWidget != nullptr) {
			return pActiveChildWidget->handleKeyPressOverlay(key);
		}
		return false;
	}

	/**
		Handles a text input event in overlays.
		\param	textInput the text input that was performed.
		\return	true = text input was processed by the container, false = text input was not processed by the container
	*/
	virtual inline bool handleTextInputOverlay(SDL_TextInputEvent& textInput) {
		if((isEnabled() == false) || (isVisible() == false) || (isActive() == false)) {
			return false;
		}

		if(pActiveChildWidget != nullptr) {
			return pActiveChildWidget->handleTextInputOverlay(textInput);
		}
		return false;
	}

	/**
		Draws this container and it's children to screen. This method is called before drawOverlay().
		\param	position	Position to draw the container to
	*/
	virtual void draw(Point position) {
		if(isVisible() == false) {
			return;
		}

		typename WidgetList::const_iterator iter;
		for(iter = containedWidgets.begin(); iter != containedWidgets.end(); ++iter) {
			Widget* curWidget = iter->pWidget;
			Point pos = getPosition(*iter);
			curWidget->draw(position+pos);
		}
	}

	/**
		This method draws the parts of this container that must be drawn after all the other
		widgets are drawn (e.g. tooltips). This method is called after draw().
		\param	position	Position to draw the container to
	*/
	virtual void drawOverlay(Point position) {
		if(isVisible() == false) {
			return;
		}

		typename WidgetList::const_iterator iter;
		for(iter = containedWidgets.begin(); iter != containedWidgets.end(); ++iter) {
			Widget* curWidget = iter->pWidget;
			Point pos = getPosition(*iter);
			curWidget->drawOverlay(position+pos);
		}
	}

    /**
		This method resizes the container. This method should only
		called if the new size is a valid size for this container (See getMinumumSize).
		\param	newSize	the new size of this progress bar
	*/
	virtual void resize(Point newSize) {
		resize(newSize.x,newSize.y);
	}

	/**
		This method resizes the container to width and height. This method should only be
		called if the new size is a valid size for this container (See getMinumumSize).
		\param	width	the new width of this container
		\param	height	the new height of this container
	*/
	virtual void resize(Uint32 width, Uint32 height) {
		Widget::resize(width,height);
	}

	/**
		Returns the position of widget relative to the top left corner of this container
		\param widgetData	the widget data to get the position from.
		\return The position of the left upper corner of widget or (-1,-1) if widget cannot be found in this container
	*/
	virtual Point getWidgetPosition(const Widget* widget) {
        WidgetData* widgetData = getWidgetDataFromWidget(widget);
        if(widgetData == nullptr) {
            return Point(-1,-1);
        } else {
            return getPosition(*widgetData);
        }
	}

	/**
		Sets this container and its children active. The parent widgets are also activated and the
		currently widget is set to inactive.
	*/
	virtual void setActive() {
		if(pActiveChildWidget == nullptr) {
            activateFirstActivatableWidget();
		}

		Widget::setActive();
	}

	/**
		Sets this container and its children inactive. The next activatable widget is activated.
	*/
	virtual void setInactive() {
		if(pActiveChildWidget != nullptr) {
			pActiveChildWidget->setActive(false);
			pActiveChildWidget = nullptr;
		}

		Widget::setInactive();
	}

	/**
		Returns whether one of this container's children can be set active.
		\return	true = activatable, false = not activatable
	*/
	virtual inline bool isActivatable() const {
	    if(isEnabled() == false) {
            return false;
	    }

		typename WidgetList::const_iterator iter = containedWidgets.begin();
		while(iter != containedWidgets.end()) {
			if(iter->pWidget->isActivatable() == true) {
				return true;
			}
			++iter;
		}
		return false;
	}

    /**
		Returns whether this widget is an container.
		\return	true = container, false = any other widget
	*/
	virtual inline bool isContainer() const { return true; }

protected:
	/**
		This method is called by other containers to enable this container or disable this container explicitly.
		It is the responsibility of the calling container to take care that there is only one active
		widget.
		\param	bActive	true = activate this widget, false = deactiviate this widget
	*/
	virtual void setActive(bool bActive) {
		if(pActiveChildWidget != nullptr) {
			pActiveChildWidget->setActive(bActive);
			if(bActive == false) {
                pActiveChildWidget = nullptr;
			}
		}
		Widget::setActive(bActive);
	}

	/**
		This method activates or deactivates one specific widget in this container. It is mainly used
		by widgets that are activated/deactivated and have to inform their parent container.
		\param	active	true = activate, false = deactivate
		\param	childWidget	the widget to activate/deactivate
	*/
	virtual void setActiveChildWidget(bool active, Widget* childWidget) {
		if(childWidget == nullptr) {
			return;
		}

		if(active == true) {
			// deactivate current active widget
			if((pActiveChildWidget != nullptr) && (pActiveChildWidget != childWidget)) {
				pActiveChildWidget->setActive(false);
				pActiveChildWidget = childWidget;
			} else {
				pActiveChildWidget = childWidget;

				// activate this container and upper containers
				Widget::setActive();
			}
		} else {
			if(childWidget != pActiveChildWidget) {
				return;
			}

			// deactivate current active widget
			if(pActiveChildWidget != nullptr) {
				pActiveChildWidget->setActive(false);
			}

			// find childWidget in the widget list
			typename WidgetList::const_iterator iter = containedWidgets.begin();
			while((iter != containedWidgets.end()) && (iter->pWidget != childWidget)) {
				++iter;
			}

			++iter;

			while(iter != containedWidgets.end()) {
				if(iter->pWidget->isActivatable() == true) {
					// activate next widget
					pActiveChildWidget = iter->pWidget;
					pActiveChildWidget->setActive();
					return;
				}
				++iter;
			}

			// we are at the end of the list
			if((getParent() != nullptr) && getParent()->isContainer()) {
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
		typename WidgetList::const_iterator iter = containedWidgets.begin();
		while(iter != containedWidgets.end()) {
			if(iter->pWidget->isActivatable() == true) {
				// activate next widget
				pActiveChildWidget = iter->pWidget;
				pActiveChildWidget->setActive();
				return;
			}
			++iter;
		}
		pActiveChildWidget = nullptr;
	}

	/**
		This method must be overwritten by all container classes. It should return
		the position of the specified widget.
		\param widgetData	the widget data to get the position from.
		\return The position of the left upper corner
	*/
	virtual Point getPosition(const WidgetData& widgetData) const = 0;

	/**
		This method returns from a Widget* the corresponding WidgetData.
		\param	pWidget	the widget to look for
		\return	a pointer to the WidgetData, nullptr if not found
	*/
	WidgetData* getWidgetDataFromWidget(const Widget* pWidget) {
		typename WidgetList::iterator iter = containedWidgets.begin();
		while(iter != containedWidgets.end()) {
			if(iter->pWidget == pWidget) {
				return &(*iter);
			}
			++iter;
		}

		return nullptr;
	}


	WidgetList containedWidgets;				///< List of widgets
	Widget* pActiveChildWidget;				///< currently active widget
};

#endif // CONTAINER_H
