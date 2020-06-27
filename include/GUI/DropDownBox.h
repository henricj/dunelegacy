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

#ifndef DROPDOWNBOX_H
#define DROPDOWNBOX_H

#include "Widget.h"
#include "PictureButton.h"
#include "ListBox.h"
#include <misc/SDL2pp.h>

#include <vector>
#include <string>
#include <functional>

/// A class for a dropdown box widget
class DropDownBox : public Widget {
public:
    /// default constructor
    DropDownBox();

    /// destructor
    virtual ~DropDownBox();

    /**
        Handles a mouse movement. This method is for example needed for the tooltip.
        \param  x               x-coordinate (relative to the left top corner of the widget)
        \param  y               y-coordinate (relative to the left top corner of the widget)
        \param  insideOverlay   true, if (x,y) is inside an overlay and this widget may be behind it, false otherwise
    */
    void handleMouseMovement(Sint32 x, Sint32 y, bool insideOverlay) override;

    /**
        Handles mouse movement in overlays.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \return true if (x,y) is in overlay of this widget, false otherwise
    */
    bool handleMouseMovementOverlay(Sint32 x, Sint32 y) override;

    /**
        Handles a left mouse click.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the widget, false = click was not processed by the widget
    */
    bool handleMouseLeft(Sint32 x, Sint32 y, bool pressed) override;

    /**
        Handles a left mouse click in overlays.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the widget, false = click was not processed by the widget
    */
    bool handleMouseLeftOverlay(Sint32 x, Sint32 y, bool pressed) override;

    /**
        Handles mouse wheel scrolling.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  up  true = mouse wheel up, false = mouse wheel down
        \return true = the mouse wheel scrolling was processed by the widget, false = mouse wheel scrolling was not processed by the widget
    */
    bool handleMouseWheel(Sint32 x, Sint32 y, bool up) override;

    /**
        Handles mouse wheel scrolling in overlays.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  up  true = mouse wheel up, false = mouse wheel down
        \return true = the mouse wheel scrolling was processed by the widget, false = mouse wheel scrolling was not processed by the widget
    */
    bool handleMouseWheelOverlay(Sint32 x, Sint32 y, bool up) override;

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
        This method draws the parts of this widget that must be drawn after all the other
        widgets are drawn (e.g. tooltips). This method is called after draw().
        \param  position    Position to draw the widget to
    */
    void drawOverlay(Point position) override;

    /**
        This method resizes the dropdownbox. This method should only
        called if the new size is a valid size for this dropdownbox (See getMinumumSize).
        \param  newSize the new size of this progress bar
    */
    void resize(Point newSize) override
    {
        resize(newSize.x,newSize.y);
    }

    /**
        This method resizes the dropdownbox to width and height. This method should only
        called if the new size is a valid size for this dropdownbox (See getMinumumSize).
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
        return Point(listBox.getMinimumSize().x,openListBoxButton.getSize().y+2);
    }

    /**
        Returns whether this widget can be set active.
        \return true = activatable, false = not activatable
    */
    inline bool isActivatable() const override { return isEnabled(); };

    /**
        Adds a new entry to this dropdown box
        \param  text    the text to be added to the list
        \param  data    an integer value that is assigned to this entry (see getEntryIntData)
    */
    void addEntry(const std::string& text, int data = 0) {
        listBox.addEntry(text, data);
        resizeListBox();
    }

    /**
        Adds a new entry to this dropdown box
        \param  text    the text to be added to the list
        \param  data    an pointer value that is assigned to this entry (see getEntryPtrData)
    */
    void addEntry(const std::string& text, void* data) {
        listBox.addEntry(text, data);
        resizeListBox();
    }

    /**
        Insert a new entry to this list box at the specified index
        \param  index   the index this entry should be inserted before.
        \param  text    the text to be added to the list
        \param  data    an integer value that is assigned to this entry (see getEntryIntData)
    */
    void insertEntry(int index, const std::string& text, int data = 0) {
        listBox.insertEntry(index, text, data);
        resizeListBox();
    }

    /**
        Insert a new entry to this list box at the specified index
        \param  index   the index this entry should be inserted before.
        \param  text    the text to be added to the list
        \param  data    an pointer value that is assigned to this entry (see getEntryPtrData)
    */
    void insertEntry(int index, const std::string& text, void* data) {
        listBox.insertEntry(index, text, data);
        resizeListBox();
    }

    /**
        Returns the number of entries in this dropdown box
        \return number of entries
    */
    int getNumEntries() const {
        return listBox.getNumEntries();
    }

    /**
        Returns the text of the entry specified by index.
        \param  index   the zero-based index of the entry
        \return the text of the entry
    */
    std::string getEntry(unsigned int index) const {
        return listBox.getEntry(index);
    }

    /**
        Sets the text of the entry specified by index.
        \param  index   the zero-based index of the entry
        \param  text    the text to set
    */
    void setEntry(unsigned int index, const std::string& text) {
        listBox.setEntry(index, text);
        invalidateForeground();
        resizeListBox();
    }

    /**
        Returns the data assigned to the entry specified by index.
        \param  index   the zero-based index of the entry
        \return the data of the entry
    */
    int getEntryIntData(unsigned int index) const {
        return listBox.getEntryIntData(index);
    }

    /**
        Sets the data assigned to the entry specified by index.
        \param  index   the zero-based index of the entry
        \param  value    the value to set
    */
    void setEntryIntData(unsigned int index, int value) {
        listBox.setEntryIntData(index, value);
    }

    /**
        Returns the data assigned to the entry specified by index.
        \param  index   the zero-based index of the entry
        \return the data of the entry
    */
    void* getEntryPtrData(unsigned int index) const {
        return listBox.getEntryPtrData(index);
    }

    /**
        Sets the data assigned to the entry specified by index.
        \param  index   the zero-based index of the entry
        \param  data    the data to set
    */
    void setEntryPtrData(unsigned int index, void* data) {
        listBox.setEntryPtrData(index, data);
    }

    /**
        Returns the text of the selected entry.
        \return the text of the entry ("" if non is selected)
    */
    std::string getSelectedEntry() const {
        return listBox.getSelectedEntry();
    }

    /**
        Returns the data assigned to the selected entry.
        \return the data of the entry (-1 if non is selected)
    */
    int getSelectedEntryIntData() const {
        return listBox.getSelectedEntryIntData();
    }

    /**
        Returns the data assigned to the selected entry.
        \return the data of the entry (nullptr if non is selected)
    */
    void* getSelectedEntryPtrData() const {
        return listBox.getSelectedEntryPtrData();
    }

    /**
        Returns the zero-based index of the current selected entry.
        \return the index of the selected element (-1 if none is selected)
    */
    int getSelectedIndex() const {
        return listBox.getSelectedIndex();
    }

    /**
        Sets the selected item. The user is informed about this switch
        by calling pOnSelectionChange(false)
        \param index    the new index (-1 == select nothing)
    */
    void setSelectedItem(int index) {
        setSelectedItem(index, false);
    }

    /**
        Sets the number of visible entries when the list is opened
        \param  newNumVisibleEntries    the number of entries (default=7)
    */
    void setNumVisibleEntries(int newNumVisibleEntries) {
        numVisibleEntries = newNumVisibleEntries;
    }

    /**
        Removes the entry which is specified by index
        \param  index   the zero-based index of the element to remove
    */
    void removeEntry(int index) {
        listBox.removeEntry(index);
        if(listBox.getSelectedIndex() < 0) {
            invalidateForeground();
        }
        resizeListBox();
    }

    /**
            Deletes all entries in the list.
    */
    void clearAllEntries() {
        listBox.clearAllEntries();
        invalidateForeground();
        resizeListBox();
    }

    /**
        Sets the function that should be called when the selection in this dropdown box changes.
        \param  pOnSelectionChange  A function this is called on selection change
    */
    inline void setOnSelectionChange(std::function<void (bool)> pOnSelectionChange) {
        this->pOnSelectionChange = pOnSelectionChange;
    }

    /**
        Sets the function that should be called when the drop down box is clicked. If no function is set,
        the default behaviour of opening the drop down list is activated.
        Otherwise the drop down list is not opened and the specified function is called. Furthermore
        a on mouse hover effect is only active if a OnClick function is set.
        \param  pOnClick    A function to be called on click
    */
    inline void setOnClick(std::function<void ()> pOnClick) {
        this->pOnClick = pOnClick;
    }

    /**
        Sets this widget active. The parent widgets are also activated and the
        currently active widget is set to inactive.
    */
    void setActive() override
    {
        openListBoxButton.setActive();
        Widget::setActive();
    }

    /**
        Sets the color for this drop down box.
        \param  color   the color (COLOR_DEFAULT = default color)
    */
    virtual inline void setColor(Uint32 color) {
        this->color = color;
        updateButtonSurface();
        invalidateForeground();
        listBox.setColor(color);
    }

    /**
        Enable or disable this widget. A disabled widget is not responding
        to clicks and key strokes and might look different.
        \param  bEnabled    true = enable widget, false = disable widget
    */
    inline void setEnabled(bool bEnabled) override
    {
        openListBoxButton.setEnabled(bEnabled);

        Widget::setEnabled(bEnabled);
    };

    /**
        Enable or disable the onClick event for this dropdownbox.
        \param  bOnClickEnabled true = enable the onClick event for this dropdownbox, false = disable onClick event
    */
    virtual inline void setOnClickEnabled(bool bOnClickEnabled) {
        this->bOnClickEnabled = bOnClickEnabled;
    };

    /**
        Returns whether the onClick event for this dropdownbox is enabled.
        \return true = onClick event is enabled, false = onClick event is disabled
    */
    inline bool isOnClickEnabled() const { return bOnClickEnabled; };

    /**
        This static method creates a dynamic dropdown box object.
        The idea behind this method is to simply create a new dropdown box on the fly and
        add it to a container. If the container gets destroyed also this dropdown box will be freed.
        \return The new created dropdown box (will be automatically destroyed when it's parent widget is destroyed)
    */
    static DropDownBox* create() {
        DropDownBox* dropDownBox = new DropDownBox();
        dropDownBox->pAllocated = true;
        return dropDownBox;
    }

protected:
    /**
        This method is called by containers to enable a widget or disable a widget explicitly.
        It is the responsibility of the container to take care that there is only one active
        widget.
        \param  bActive true = activate this widget, false = deactiviate this widget
    */
    void setActive(bool bActive) override;

    /**
        Sets the selected item. The user is informed about this switch
        by calling pOnSelectionChange(true)
        \param index    the new index (-1 == select nothing)
        \param bInteractive true = interactive change of the selection, false = changed by calling setSelectedEntry()
    */
    void setSelectedItem(int index, bool bInteractive) {
        listBox.setSelectedItem(index, bInteractive);
    }

private:

    void onSelectionChange(bool bInteractive);

    void resizeListBox();

    void updateButtonSurface();

    void invalidateForeground();

    void updateForeground();

    void invalidateBackground();

    void updateBackground();

    void onOpenListBoxButton() {
        bShowListBox = !bShowListBox;
    }

    bool bShowListBox;
    bool bListBoxAbove;
    bool bAutocloseListBoxOnSelectionChange;        ///< This is a small hack to allow the list box to be open while selection with up/down keys
    bool bOnClickEnabled;                           ///< Is the onClick event enabled for this widget?

    sdl2::texture_ptr pBackground;
    sdl2::texture_ptr pForeground;
    sdl2::texture_ptr pActiveForeground;                 ///< Ís shown while the mouse cursor is over this drop down box

    PictureButton openListBoxButton;
    ListBox listBox;

    std::function<void (bool)> pOnSelectionChange;  ///< this function is called when the selection changes
    std::function<void ()> pOnClick;                ///< function that is called when this drop down box is clicked

    int numVisibleEntries;                          ///< the number of entries visible when the list is opened (default=7)

    Uint32 color;                                   ///< the color
    bool bHover;                                    ///< true = currenlty mouse hover, false = currently no mouse hover
};

#endif //DROPDOWNBOX_H
