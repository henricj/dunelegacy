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

#ifndef LISTBOX_H
#define LISTBOX_H

#include "Widget.h"
#include "ScrollBar.h"
#include <misc/SDL2pp.h>

#include <vector>
#include <string>
#include <functional>

class DropDownBox;

/// A class for a list box widget
class ListBox : public Widget {
public:
    friend class DropDownBox;

    /// default constructor
    ListBox();

    /// destructor
    virtual ~ListBox();

    /**
        Handles a mouse movement. This method is for example needed for the tooltip.
        \param  x               x-coordinate (relative to the left top corner of the widget)
        \param  y               y-coordinate (relative to the left top corner of the widget)
        \param  insideOverlay   true, if (x,y) is inside an overlay and this widget may be behind it, false otherwise
    */
    void handleMouseMovement(Sint32 x, Sint32 y, bool insideOverlay) override;

    /**
        Handles a left mouse click.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the widget, false = click was not processed by the widget
    */
    bool handleMouseLeft(Sint32 x, Sint32 y, bool pressed) override;

    /**
        Handles mouse wheel scrolling.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  up  true = mouse wheel up, false = mouse wheel down
        \return true = the mouse wheel scrolling was processed by the widget, false = mouse wheel scrolling was not processed by the widget
    */
    bool handleMouseWheel(Sint32 x, Sint32 y, bool up) override;

    /**
        Handles a key stroke. This method is neccessary for controlling an application
        without a mouse.
        \param  key the key that was pressed or released.
        \return true = key stroke was processed by the widget, false = key stroke was not processed by the widget
    */
    bool handleKeyPress(SDL_KeyboardEvent& key) override;

    /**
        Draws this scroll bar to screen. This method is called before drawOverlay().
        \param  position    Position to draw the scroll bar to
    */
    void draw(Point position) override;

    /**
        This method resizes the list box. This method should only
        called if the new size is a valid size for this list box (See getMinumumSize).
        \param  newSize the new size of this progress bar
    */
    void resize(Point newSize) override
    {
        resize(newSize.x,newSize.y);
    }
    /**
        This method resizes the list box to width and height. This method should only
        called if the new size is a valid size for this list box (See getMinumumSize).
        \param  width   the new width of this scroll bar
        \param  height  the new height of this scroll bar
    */
    void resize(Uint32 width, Uint32 height) override;

    /**
        Returns the minimum size of this scroll bar. The scroll bar should not
        resized to a size smaller than this.
        \return the minimum size of this scroll bar
    */
    Point getMinimumSize() const override
    {
        Point tmp = scrollbar.getMinimumSize();
        tmp.x += 30;
        return tmp;
    }

    /**
        Sets this widget active. The parent widgets are also activated and the
        currently active widget is set to inactive.
    */
    void setActive() override;

    /**
        Returns whether this widget can be set active.
        \return true = activatable, false = not activatable
    */
    inline bool isActivatable() const override { return isEnabled(); };

    /**
        Adds a new entry to this list box
        \param  text    the text to be added to the list
        \param  data    an integer value that is assigned to this entry (see getEntryIntData)
    */
    void addEntry(const std::string& text, int data = 0) {
        entries.emplace_back(text,data);
        updateList();
    }

    /**
        Adds a new entry to this list box
        \param  text    the text to be added to the list
        \param  data    an pointer value that is assigned to this entry (see getEntryPtrData)
    */
    void addEntry(const std::string& text, void* data) {
        entries.push_back(ListEntry(text,data));
        updateList();
    }

    /**
        Insert a new entry to this list box at the specified index
        \param  index   the index this entry should be inserted before.
        \param  text    the text to be added to the list
        \param  data    an integer value that is assigned to this entry (see getEntryIntData)
    */
    void insertEntry(int index, const std::string& text, int data = 0) {
        if(index <= selectedElement) selectedElement++;

        entries.insert(entries.begin() + index, ListEntry(text,data));
        updateList();
    }

    /**
        Insert a new entry to this list box at the specified index
        \param  index   the index this entry should be inserted before.
        \param  text    the text to be added to the list
        \param  data    an pointer value that is assigned to this entry (see getEntryPtrData)
    */
    void insertEntry(int index, const std::string& text, void* data) {
        if(index <= selectedElement) selectedElement++;

        entries.insert(entries.begin() + index, ListEntry(text,data));
        updateList();
    }

    /**
        Returns the number of entries in this list box
        \return number of entries
    */
    int getNumEntries() const {
        return entries.size();
    }

    /**
        Returns the text of the entry specified by index.
        \param  index   the zero-based index of the entry
        \return the text of the entry
    */
    std::string getEntry(unsigned int index) const {
        if(index < entries.size()) {
            return entries.at(index).text;
        } else {
            return "";
        }
    }

    /**
        Sets the text of the entry specified by index.
        \param  index   the zero-based index of the entry
        \param  text    the text to set
    */
    void setEntry(unsigned int index, const std::string& text) {
        if(index >= entries.size()) {
            return;
        }

        entries.at(index).text = text;
        updateList();
    }

    /**
        Returns the data assigned to the entry specified by index.
        \param  index   the zero-based index of the entry
        \return the data of the entry
    */
    int getEntryIntData(unsigned int index) const {
        if(index < entries.size()) {
            return entries.at(index).data.intData;
        } else {
            return 0;
        }
    }

    /**
        Sets the data assigned to the entry specified by index.
        \param  index   the zero-based index of the entry
        \param  value    the value to set
    */
    void setEntryIntData(unsigned int index, int value) {
        if(index >= entries.size()) {
            return;
        }

        entries.at(index).data.intData = value;
    }

    /**
        Returns the data assigned to the entry specified by index.
        \param  index   the zero-based index of the entry
        \return the data of the entry
    */
    void* getEntryPtrData(unsigned int index) const {
        if(index < entries.size()) {
            return entries.at(index).data.ptrData;
        } else {
            return 0;
        }
    }

    /**
        Sets the data assigned to the entry specified by index.
        \param  index   the zero-based index of the entry
        \param  data    the data to set
    */
    void setEntryPtrData(unsigned int index, void* data) {
        if(index >= entries.size()) {
            return;
        }

        entries.at(index).data.ptrData = data;
    }

    /**
        Returns the text of the selected entr.
        \return the text of the entry ("" if non is selected)
    */
    std::string getSelectedEntry() const {
        return ((selectedElement >= 0) ? getEntry(selectedElement) : "");
    }

    /**
        Returns the data assigned to the selected entry.
        \return the data of the entry (-1 if non is selected)
    */
    int getSelectedEntryIntData() const {
        return ((selectedElement >= 0) ? getEntryIntData(selectedElement) : -1);
    }

    /**
        Returns the data assigned to the selected entry.
        \return the data of the entry (nullptr if non is selected)
    */
    void* getSelectedEntryPtrData() const {
        return ((selectedElement >= 0) ? getEntryPtrData(selectedElement) : nullptr);
    }

    /**
        Returns the zero-based index of the current selected entry.
        \return the index of the selected element (-1 if none is selected)
    */
    int getSelectedIndex() const {
        return selectedElement;
    }

    /**
        Sets the selected item and scrolls the scrollbar to that position. The user
        is informed about this switch by calling pOnSelectionChange(false)
        \param index    the new index (-1 == select nothing)
    */
    void setSelectedItem(int index) {
        setSelectedItem(index, false);
    }

    /**
        Removes the entry which is specified by index
        \param  index   the zero-based index of the element to remove
    */
    void removeEntry(int index) {
        std::vector<ListEntry>::iterator iter = entries.begin() + index;
        entries.erase(iter);
        if(index == selectedElement) {
            selectedElement = -1;
        } else if(index < selectedElement) {
            selectedElement--;
        }

        if(index < firstVisibleElement) firstVisibleElement--;

        updateList();
    }

    /**
            Deletes all entries in the list.
    */
    void clearAllEntries() {
        entries.clear();
        selectedElement = -1;
        updateList();
    }

    /**
        Sets the function that should be called when the selection in this list box changes.
        \param  pOnSelectionChange  A function to be called on selection change
    */
    inline void setOnSelectionChange(std::function<void (bool)> pOnSelectionChange) {
        this->pOnSelectionChange = pOnSelectionChange;
    }

    /**
        Sets the function that should be called when a list entry is single clicked.
        \param  pOnSingleClick  A function to be called on single click
    */
    inline void setOnSingleClick(std::function<void ()> pOnSingleClick) {
        this->pOnSingleClick = pOnSingleClick;
    }

    /**
        Sets the function that should be called when a list entry is double clicked.
        \param  pOnDoubleClick  A function to be called on double click
    */
    inline void setOnDoubleClick(std::function<void ()> pOnDoubleClick) {
        this->pOnDoubleClick = pOnDoubleClick;
    }

    /**
        Sets the color for this list box.
        \param  color   the color (COLOR_DEFAULT = default color)
    */
    virtual inline void setColor(Uint32 color) {
        this->color = color;
        updateList();
        scrollbar.setColor(color);
    }

    /**
        Is the scrollbar always shown or is it hidden if not needed?
        \return true if scrollbar is hidden if not needed
    */
    bool getAutohideScrollbar() const { return bAutohideScrollbar; }

    /**
        Set if the scrollbar shall be hidden if not needed
        \param  bAutohideScrollbar  true = hide scrollbar, false = show always
    */
    void setAutohideScrollbar(bool bAutohideScrollbar) {
        this->bAutohideScrollbar = bAutohideScrollbar;
    }

    /**
        Checks if the scrollbar is currently visible.
        \return true, if visible, false otherwise
    */
    bool isScrollbarVisible() const {
        return (!bAutohideScrollbar || (scrollbar.getRangeMin() != scrollbar.getRangeMax()));
    }

    /**
        Is the selected element highlighted?
        \return true if the selected element is highlighted
    */
    bool getHighlightSelectedElement() const { return bHighlightSelectedElement; }

    /**
        Set if the selected element shall be highlighted
        \param  bHighlightSelectedElement   true = highlight, false = do not highlight
    */
    void setHighlightSelectedElement(bool bHighlightSelectedElement) {
        this->bHighlightSelectedElement = bHighlightSelectedElement;
        updateList();
    }

protected:
    /**
        Sets the selected item and scrolls the scrollbar to that position. The user
        is informed about this switch by calling pOnSelectionChange(bInteractive)
        \param index        the new index (-1 == select nothing)
        \param bInteractive true = interactive change of the selection, false = changed by calling setSelectedEntry()
    */
    void setSelectedItem(int index, bool bInteractive);

protected:
    /**
        This method is called whenever the textures of this widget are needed, e.g. before drawing. This method
        should be overwritten by subclasses if they like to defer texture creation as long as possible.
        This method should first check whether a renewal of the textures is necessary.
    */
    void updateTextures() override;

    /**
        This method frees all textures that are used by this list box
    */
    void invalidateTextures() override
    {
        pBackground.reset();
        pForeground.reset();
    }

private:
    void updateList();

    void onScrollbarChange() {
        firstVisibleElement = scrollbar.getCurrentValue();
        updateList();
    }

    class ListEntry {
    public:
        ListEntry(const std::string& text, int intData) : text(text) {
            data.intData = intData;
        }

        ListEntry(const std::string& text, void* ptrData) : text(text) {
            data.ptrData = ptrData;
        }

        std::string text;
        union {
            int intData;
            void*   ptrData;
        } data;
    };

    std::vector<ListEntry> entries;
    sdl2::texture_ptr pBackground;
    sdl2::texture_ptr pForeground;
    ScrollBar scrollbar;

    std::function<void (bool)> pOnSelectionChange;  ///< this function is called when the selection changes
    std::function<void ()> pOnSingleClick;          ///< this function is called when a list entry is single clicked
    std::function<void ()> pOnDoubleClick;          ///< this function is called when a list entry is double clicked

    Uint32 color;                                   ///< the color
    bool bAutohideScrollbar;                        ///< hide the scrollbar if not needed (default = true)
    bool bHighlightSelectedElement;                 ///< highlight selected element (default = true);
    int firstVisibleElement;                        ///< the index of the first shown element in the list
    int selectedElement;                            ///< the selected element
    Uint32 lastClickTime;                           ///< the time an element was clicked on the last time (needed for double clicking)
};

#endif //LISTBOX_H
