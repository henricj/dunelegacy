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

#include "ListBox.h"
#include "PictureButton.h"
#include "Widget.h"
#include <misc/SDL2pp.h>

#include <functional>
#include <string>
#include <utility>

/// A class for a dropdown box widget
class DropDownBox final : public Widget {
    using parent = Widget;

public:
    using index_type                    = ListBox::index_type;
    static constexpr auto invalid_index = std::numeric_limits<index_type>::max();

    /// default constructor
    DropDownBox();

    /// destructor
    ~DropDownBox() override;

    /**
        Handles a mouse movement. This method is for example needed for the tooltip.
        \param  x               x-coordinate (relative to the left top corner of the widget)
        \param  y               y-coordinate (relative to the left top corner of the widget)
        \param  insideOverlay   true, if (x,y) is inside an overlay and this widget may be behind it, false otherwise
    */
    void handleMouseMovement(int32_t x, int32_t y, bool insideOverlay) override;

    /**
        Handles mouse movement in overlays.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \return true if (x,y) is in overlay of this widget, false otherwise
    */
    bool handleMouseMovementOverlay(int32_t x, int32_t y) override;

    /**
        Handles a left mouse click.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the widget, false = click was not processed by the widget
    */
    bool handleMouseLeft(int32_t x, int32_t y, bool pressed) override;

    /**
        Handles a left mouse click in overlays.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the widget, false = click was not processed by the widget
    */
    bool handleMouseLeftOverlay(int32_t x, int32_t y, bool pressed) override;

    /**
        Handles mouse wheel scrolling.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  up  true = mouse wheel up, false = mouse wheel down
        \return true = the mouse wheel scrolling was processed by the widget, false = mouse wheel scrolling was not
       processed by the widget
    */
    bool handleMouseWheel(int32_t x, int32_t y, bool up) override;

    /**
        Handles mouse wheel scrolling in overlays.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  up  true = mouse wheel up, false = mouse wheel down
        \return true = the mouse wheel scrolling was processed by the widget, false = mouse wheel scrolling was not
       processed by the widget
    */
    bool handleMouseWheelOverlay(int32_t x, int32_t y, bool up) override;

    /**
        Handles a key stroke. This method is necessary for controlling an application
        without a mouse.
        \param  key the key that was pressed or released.
        \return true = key stroke was processed by the widget, false = key stroke was not processed by the widget
    */
    bool handleKeyPress(const SDL_KeyboardEvent& key) override;

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
        This method resizes the dropdownbox to width and height. This method should only
        called if the new size is a valid size for this dropdownbox (See getMinimumSize).
        \param  width   the new width of this scroll bar
        \param  height  the new height of this scroll bar
    */
    void resize(uint32_t width, uint32_t height) override;

    using parent::resize;

    /**
        Returns the minimum size of this scroll bar. The scroll bar should not
        resized to a size smaller than this.
        \return the minimum size of this scroll bar
    */
    [[nodiscard]] Point getMinimumSize() const override {
        return {listBox_.getMinimumSize().x, openListBoxButton_.getSize().y + 2};
    }

    /**
        Returns whether this widget can be set active.
        \return true = activatable, false = not activatable
    */
    [[nodiscard]] bool isActivatable() const override { return isEnabled(); }

    /**
        Adds a new entry to this dropdown box
        \param  text    the text to be added to the list
        \param  data    an integer value that is assigned to this entry (see getEntryIntData)
    */
    void addEntry(std::string text, int data = 0);

    /**
        Adds a new entry to this dropdown box
        \param  text    the text to be added to the list
        \param  data    an pointer value that is assigned to this entry (see getEntryPtrData)
    */
    void addEntry(std::string text, void* data);

    /**
        Adds a new entry to this dropdown box
        \param  text    the text to be added to the list
        \param  data    an integer value that is assigned to this entry (see getEntryIntData)
    */
    void addEntry(std::string_view text, int data = 0) { addEntry(std::string{text}, data); }

    /**
        Adds a new entry to this dropdown box
        \param  text    the text to be added to the list
        \param  data    an pointer value that is assigned to this entry (see getEntryPtrData)
    */
    void addEntry(std::string_view text, void* data) { addEntry(std::string{text}, data); }

    /**
        Insert a new entry to this list box at the specified index
        \param  index   the index this entry should be inserted before.
        \param  text    the text to be added to the list
        \param  data    an integer value that is assigned to this entry (see getEntryIntData)
    */
    void insertEntry(index_type index, std::string text, int data = 0);

    /**
        Insert a new entry to this list box at the specified index
        \param  index   the index this entry should be inserted before.
        \param  text    the text to be added to the list
        \param  data    an pointer value that is assigned to this entry (see getEntryPtrData)
    */
    void insertEntry(index_type index, std::string text, void* data);

    /**
        Returns the number of entries in this dropdown box
        \return number of entries
    */
    [[nodiscard]] auto getNumEntries() const { return listBox_.getNumEntries(); }

    /**
        Returns the text of the entry specified by index.
        \param  index   the zero-based index of the entry
        \return the text of the entry
    */
    [[nodiscard]] std::string getEntry(index_type index) const { return listBox_.getEntry(index); }

    /**
        Sets the text of the entry specified by index.
        \param  index   the zero-based index of the entry
        \param  text    the text to set
    */
    void setEntry(index_type index, const std::string& text);

    /**
        Returns the data assigned to the entry specified by index.
        \param  index   the zero-based index of the entry
        \return the data of the entry
    */
    [[nodiscard]] int getEntryIntData(index_type index) const { return listBox_.getEntryIntData(index); }

    /**
        Sets the data assigned to the entry specified by index.
        \param  index   the zero-based index of the entry
        \param  value    the value to set
    */
    void setEntryIntData(index_type index, int value) { listBox_.setEntryIntData(index, value); }

    /**
        Returns the data assigned to the entry specified by index.
        \param  index   the zero-based index of the entry
        \return the data of the entry
    */
    [[nodiscard]] void* getEntryPtrData(index_type index) const { return listBox_.getEntryPtrData(index); }

    /**
        Sets the data assigned to the entry specified by index.
        \param  index   the zero-based index of the entry
        \param  data    the data to set
    */
    void setEntryPtrData(index_type index, void* data) { listBox_.setEntryPtrData(index, data); }

    /**
        Returns the text of the selected entry.
        \return the text of the entry ("" if non is selected)
    */
    [[nodiscard]] std::string getSelectedEntry() const { return listBox_.getSelectedEntry(); }

    /**
        Returns the data assigned to the selected entry.
        \return the data of the entry (-1 if non is selected)
    */
    [[nodiscard]] int getSelectedEntryIntData() const { return listBox_.getSelectedEntryIntData(); }

    /**
        Returns the data assigned to the selected entry.
        \return the data of the entry (nullptr if non is selected)
    */
    [[nodiscard]] void* getSelectedEntryPtrData() const { return listBox_.getSelectedEntryPtrData(); }

    /**
        Returns the zero-based index of the current selected entry.
        \return the index of the selected element (invalid_index if none is selected)
    */
    [[nodiscard]] index_type getSelectedIndex() const { return listBox_.getSelectedIndex(); }

    /**
        Sets the selected item. The user is informed about this switch
        by calling pOnSelectionChange(false)
        \param index    the new index (-1 == select nothing)
    */
    void setSelectedItem(index_type index) { setSelectedItem(index, false); }

    /**
        Sets the number of visible entries when the list is opened
        \param  newNumVisibleEntries    the number of entries (default=7)
    */
    void setNumVisibleEntries(index_type newNumVisibleEntries) { numVisibleEntries_ = newNumVisibleEntries; }

    /**
        Removes the entry which is specified by index
        \param  index   the zero-based index of the element to remove
    */
    void removeEntry(index_type index);

    /**
            Deletes all entries in the list.
    */
    void clearAllEntries();

    /**
        Sets the function that should be called when the selection in this dropdown box changes.
        \param  pOnSelectionChange  A function this is called on selection change
    */
    void setOnSelectionChange(std::function<void(bool)> pOnSelectionChange) {
        this->pOnSelectionChange_ = std::move(pOnSelectionChange);
    }

    /**
        Sets the function that should be called when the drop down box is clicked. If no function is set,
        the default behaviour of opening the drop down list is activated.
        Otherwise the drop down list is not opened and the specified function is called. Furthermore
        a on mouse hover effect is only active if a OnClick function is set.
        \param  pOnClick    A function to be called on click
    */
    void setOnClick(std::function<void()> pOnClick) { this->pOnClick_ = std::move(pOnClick); }

    /**
        Sets this widget active. The parent widgets are also activated and the
        currently active widget is set to inactive.
    */
    void setActive() override;

    /**
        Sets the color for this drop down box.
        \param  color   the color (COLOR_DEFAULT = default color)
    */
    void setColor(uint32_t color);

    /**
        Enable or disable this widget. A disabled widget is not responding
        to clicks and key strokes and might look different.
        \param  bEnabled    true = enable widget, false = disable widget
    */
    void setEnabled(bool bEnabled) override;

    /**
        Enable or disable the onClick event for this dropdownbox.
        \param  bOnClickEnabled true = enable the onClick event for this dropdownbox, false = disable onClick event
    */
    void setOnClickEnabled(bool bOnClickEnabled) { this->bOnClickEnabled_ = bOnClickEnabled; }

    /**
        Returns whether the onClick event for this dropdownbox is enabled.
        \return true = onClick event is enabled, false = onClick event is disabled
    */
    [[nodiscard]] bool isOnClickEnabled() const { return bOnClickEnabled_; }

    /**
        This static method creates a dynamic dropdown box object.
        The idea behind this method is to simply create a new dropdown box on the fly and
        add it to a container. If the container gets destroyed also this dropdown box will be freed.
        \return The new created dropdown box (will be automatically destroyed when it's parent widget is destroyed)
    */
    static std::unique_ptr<DropDownBox> create();

protected:
    /**
        This method is called by containers to enable a widget or disable a widget explicitly.
        It is the responsibility of the container to take care that there is only one active
        widget.
        \param  bActive true = activate this widget, false = deactivate this widget
    */
    void setActive(bool bActive) override;

    /**
        Sets the selected item. The user is informed about this switch
        by calling pOnSelectionChange(true)
        \param index    the new index (-1 == select nothing)
        \param bInteractive true = interactive change of the selection, false = changed by calling setSelectedEntry()
    */
    void setSelectedItem(index_type index, bool bInteractive) { listBox_.setSelectedItem(index, bInteractive); }

private:
    void onSelectionChange(bool bInteractive);

    void resizeListBox();

    void updateButtonSurface();

    void invalidateForeground();

    void updateForeground();

    void invalidateBackground();

    void updateBackground();

    void onOpenListBoxButton() { bShowListBox_ = !bShowListBox_; }

    bool bShowListBox_                       = false;
    bool bListBoxAbove_                      = false;
    bool bAutocloseListBoxOnSelectionChange_ = true; ///< This is a small hack to allow the list box to be open while
                                                     ///< selection with up/down keys
    bool bOnClickEnabled_ = true;                    ///< Is the onClick event enabled for this widget?

    DuneTextureOwned pBackground_;
    DuneTextureOwned pForeground_;
    DuneTextureOwned pActiveForeground_; ///< Ís shown while the mouse cursor is over this drop down box

    PictureButton openListBoxButton_;
    ListBox listBox_;

    std::function<void(bool)> pOnSelectionChange_; ///< this function is called when the selection changes
    std::function<void()> pOnClick_;               ///< function that is called when this drop down box is clicked

    index_type numVisibleEntries_ = 7; ///< the number of entries visible when the list is opened (default=7)

    uint32_t color_{COLOR_DEFAULT}; ///< the color
    bool bHover_ = false;           ///< true = currently mouse hover, false = currently no mouse hover
};

#endif // DROPDOWNBOX_H
