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

#ifndef VBOX_H
#define VBOX_H

#include "Container.h"

class VBox_WidgetData {
public:
	VBox_WidgetData() : pWidget(NULL), fixedHeight(0), weight(0.0) { };
	VBox_WidgetData(Widget* _pWidget, Sint32 _fixedHeight) : pWidget(_pWidget), fixedHeight(_fixedHeight), weight(0.0) { };
	VBox_WidgetData(Widget* _pWidget, double _weight) : pWidget(_pWidget), fixedHeight(-1), weight(_weight) { };

	Widget* pWidget;
	Sint32 fixedHeight;
	double weight;
};

/// A container class for vertical aligned widgets.
class VBox : public Container<VBox_WidgetData> {
public:
	/// default constructor
	VBox() : Container<VBox_WidgetData>() {
		;
	}

	/// destructor
	virtual ~VBox() {
		;
	}

	/**
		This method adds a new widget to this container.
		\param newWidget	Widget to add
		\param fixedHeight	a fixed height for this widget (must be greater than the minimum size)
	*/
	virtual void addWidget(Widget* newWidget, Sint32 fixedHeight) {
		if(newWidget != NULL) {
			containedWidgets.push_back(VBox_WidgetData(newWidget, fixedHeight));
			newWidget->setParent(this);
			Widget::resizeAll();
		}
	}

	/**
		This method adds a new widget to this container.
		\param newWidget	Widget to add
		\param weight		The weight for this widget (default=1.0)
	*/
	virtual void addWidget(Widget* newWidget, double weight = 1.0) {
		if(newWidget != NULL) {
			containedWidgets.push_back(VBox_WidgetData(newWidget, weight));
			newWidget->setParent(this);
			Widget::resizeAll();
		}
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
			Widget* curWidget = iter->pWidget;
			p.x = std::max(p.x,curWidget->getMinimumSize().x);
			if(iter->fixedHeight > 0) {
                p.y += iter->fixedHeight;
			} else {
                p.y += curWidget->getMinimumSize().y;
			}
		}
		return p;
	}

	/**
		This method resizes the container to width and height. This method should only be
		called if the new size is a valid size for this container (See resizingXAllowed,
		resizingYAllowed, getMinumumSize). It also resizes all child widgets.
		\param	width	the new width of this container
		\param	height	the new height of this container
	*/
	virtual void resize(Uint32 width, Uint32 height) {
		Sint32 availableHeight = height;

		int numRemainingWidgets = containedWidgets.size();

		// Find objects that are not allowed to be resized or have a fixed width
		// also find the sum of all weights
		double weightSum = 0.0;
		WidgetList::const_iterator iter;
		for(iter = containedWidgets.begin(); iter != containedWidgets.end(); ++iter) {
			Widget* curWidget = iter->pWidget;
			if(curWidget->resizingYAllowed() == false) {
				availableHeight = availableHeight - curWidget->getSize().y;
				numRemainingWidgets--;
			} else if(iter->fixedHeight > 0) {
				availableHeight = availableHeight - iter->fixedHeight;
				numRemainingWidgets--;
			} else {
				weightSum += iter->weight;
			}
		}

		// Under the resizeable widgets find all objects that are oversized (minimum size > AvailableHeight*weight)
		// also calculate the weight sum of all the resizeable widgets that are not oversized
		Sint32 neededOversizeHeight = 0;
		double notOversizedWeightSum = 0.0;
		for(iter = containedWidgets.begin(); iter != containedWidgets.end(); ++iter) {
			Widget* curWidget = iter->pWidget;
			if(curWidget->resizingYAllowed() == true && iter->fixedHeight <= 0) {
				if((double) curWidget->getMinimumSize().y > availableHeight * (iter->weight/weightSum)) {
					neededOversizeHeight += curWidget->getMinimumSize().y;
				} else {
					notOversizedWeightSum += iter->weight;
				}
			}
		}


		Sint32 totalAvailableHeight = availableHeight;
		for(iter = containedWidgets.begin(); iter != containedWidgets.end(); ++iter) {
			Widget* curWidget = iter->pWidget;
			Sint32 WidgetWidth;
			if(curWidget->resizingXAllowed() == true) {
				WidgetWidth = width;
			} else {
				WidgetWidth = curWidget->getMinimumSize().x;
			}

			if(curWidget->resizingYAllowed() == true) {
				Sint32 WidgetHeight = 0;

				if(iter->fixedHeight <= 0) {
					if(numRemainingWidgets <= 1) {
						WidgetHeight = availableHeight;
					} else if((double) curWidget->getMinimumSize().y > totalAvailableHeight * (iter->weight/weightSum)) {
						WidgetHeight = curWidget->getMinimumSize().y;
					} else {
						WidgetHeight = (Sint32) ((totalAvailableHeight-neededOversizeHeight) * (iter->weight/notOversizedWeightSum));
					}
					availableHeight -= WidgetHeight;
					numRemainingWidgets--;
				} else {
					WidgetHeight = iter->fixedHeight;
				}

				curWidget->resize(WidgetWidth,WidgetHeight);
			} else {
				curWidget->resize(WidgetWidth, curWidget->getSize().y);
			}
		}

		Container<VBox_WidgetData>::resize(width,height);
	}

    /**
		This static method creates a dynamic VBox object.
		The idea behind this method is to simply create a new VBox on the fly and
		add it to a container. If the container gets destroyed also this VBox will be freed.
		\return	The new created VBox (will be automatically destroyed when it's parent widget is destroyed)
	*/
	static VBox* create() {
		VBox* vbox = new VBox();
		vbox->pAllocated = true;
		return vbox;
	}

protected:
	/**
		This method must be overwritten by all container classes. It should return
		the position of the specified widget.
		\param widgetData	the widget data to get the position from.
		\return The position of the left upper corner
	*/
	virtual Point getPosition(const VBox_WidgetData& widgetData) const {
		Point p(0,0);
		WidgetList::const_iterator iter;
		for(iter = containedWidgets.begin(); iter != containedWidgets.end(); ++iter) {
			Widget* curWidget = iter->pWidget;
			if(widgetData.pWidget == curWidget) {
				return p;
			} else {
				p.y = p.y + curWidget->getSize().y;
			}
		}

		//should not happen
		return Point(0,0);
	}
};

#endif // VBOX_H

