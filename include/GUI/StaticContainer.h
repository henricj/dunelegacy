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

#ifndef STATICCONTAINER_H
#define STATICCONTAINER_H

#include "Container.h"

class StaticContainer_WidgetData {
public:
	StaticContainer_WidgetData() {
		pWidget = nullptr;
		position = Point(0,0);
		size = Point(0,0);
	}

	StaticContainer_WidgetData(Widget* pWidget, Point position, Point size) {
		this->pWidget = pWidget;
		this->position = position;
		this->size = size;
	}

	Widget* pWidget;
	Point position;
	Point size;

	inline bool operator==(const StaticContainer_WidgetData& op) const {
		return (pWidget == op.pWidget) && (position == op.position) && (size == op.size);
	}
};

/// A container class of explicit placed widgets
class StaticContainer : public Container<StaticContainer_WidgetData> {
public:


	/// default constructor
	StaticContainer() : Container<StaticContainer_WidgetData>() {
		;
	}

	/// default destructor
	virtual ~StaticContainer() {
	}

	/**
		This method adds a new widget to this container.
		\param newWidget	Widget to add
		\param position		Position of the new Widget
		\param size			Size of the new widget
	*/
	virtual void addWidget(Widget* newWidget, Point position, Point size) {
		if(newWidget != nullptr) {
			containedWidgets.push_back(StaticContainer_WidgetData(newWidget,position,size));
			newWidget->resize(size.x, size.y);
			newWidget->setParent(this);
			Widget::resizeAll();
		}
	}

	/**
		This method adds a new widget to this container.
		\param newWidget	Widget to add
		\param rect		    Position and size of the new Widget
	*/
	virtual void addWidget(Widget* newWidget, const SDL_Rect& rect) {
        addWidget(newWidget, Point(rect.x, rect.y), Point(rect.w, rect.h));
	}

	/**
		Returns the minimum size of this container. The container should not
		resized to a size smaller than this. If the container is not resizeable
		in a direction this method returns the size in that direction.
		\return the minimum size of this container
	*/
	virtual Point getMinimumSize() const {
		Point p(0,0);
		WidgetList::const_iterator iter;
		for(iter = containedWidgets.begin(); iter != containedWidgets.end(); ++iter) {
			const StaticContainer_WidgetData curWidgetData = *iter;
			p.x = std::max(p.x , curWidgetData.position.x + curWidgetData.size.x);
			p.y = std::max(p.y , curWidgetData.position.y + curWidgetData.size.y);
		}
		return p;
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
		This method resized the container to width and height. This method should only be
		called if the new size is a valid size for this container (See resizingXAllowed,
		resizingYAllowed, getMinumumSize). It also resizes all child widgets.
		\param	width	the new width of this container
		\param	height	the new height of this container
	*/
	virtual void resize(Uint32 width, Uint32 height) {
		WidgetList::const_iterator iter;
		for(iter = containedWidgets.begin(); iter != containedWidgets.end(); ++iter) {
			const StaticContainer_WidgetData curWidgetData = *iter;
			curWidgetData.pWidget->resize(curWidgetData.size.x,curWidgetData.size.y);
		}
		Container<StaticContainer_WidgetData>::resize(width,height);
	}

protected:
	/**
		This method must be overwritten by all container classes. It should return
		the position of the specified widget.
		\param widgetData	the widget data to get the position from.
		\return The position of the left upper corner
	*/
	virtual Point getPosition(const StaticContainer_WidgetData& widgetData) const {
		return widgetData.position;
	}

	void setWidgetGeometry(Widget* pWidget, Point position, Point size) {
		StaticContainer_WidgetData* pWidgetData = getWidgetDataFromWidget(pWidget);
		if(pWidgetData != 0) {
			pWidgetData->position = position;
			pWidgetData->size = size;
			pWidget->resize(size.x, size.y);
		}
	}
};


#endif // STATICCONTAINER_H

