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

#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include "Widget.h"
#include "ScrollBar.h"
#include <misc/SDL2pp.h>

#include <vector>
#include <string>

/// A class for a multiline text view widget
class TextView : public Widget {
public:
    /// default constructor
    TextView();

    /// destructor
    virtual ~TextView();

    /**
        Sets a font size for this text view. Default font size of a text view is 14
        \param  fontSize      the size of the new font
    */
    virtual void setTextFontSize(int fontSize) {
        this->fontSize = fontSize;
        resize(getSize().x, getSize().y);
    }

    /**
        Gets the font size of this text view. Default font size of a text view is 14
        \return the font size of this text view
    */
    virtual int getTextFontSize() const {
       return fontSize;
    }

    /**
        Sets the text color for this label.
        \param  textcolor       the color of the text (COLOR_DEFAULT = default color)
        \param  textshadowcolor the color of the shadow of the text (COLOR_DEFAULT = default color)
        \param  backgroundcolor the color of the label background (COLOR_TRANSPARENT = transparent)
    */
    virtual void setTextColor(Uint32 textcolor, Uint32 textshadowcolor = COLOR_DEFAULT, Uint32 backgroundcolor = COLOR_TRANSPARENT) {
        this->textcolor = textcolor;
        this->textshadowcolor = textshadowcolor;
        this->backgroundcolor = backgroundcolor;
        invalidateTextures();
    }

    /**
        Sets the alignment of the text in this label.
        \param alignment Combination of (Alignment_HCenter, Alignment_Left or Alignment_Right) and (Alignment_VCenter, Alignment_Top or Alignment_Bottom)
    */
    virtual void setAlignment(Alignment_Enum alignment) {
        this->alignment = alignment;
        invalidateTextures();
    }

    /**
        Returns the alignment of the text in this label.
        \return Combination of (Alignment_HCenter, Alignment_Left or Alignment_Right) and (Alignment_VCenter, Alignment_Top or Alignment_Bottom)
    */
    virtual inline Alignment_Enum getAlignment() const {
        return alignment;
    }

    /**
        This method sets a new text for this label and resizes this label
        to fit this text.
        \param  text The new text for this button
    */
    virtual void setText(const std::string& text) {
        this->text = text;
        resizeAll();
    }

    /**
        Get the text of this label.
        \return the text of this button
    */
    const std::string& getText() const noexcept { return text; };

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
        This method resizes the text view. This method should only
        called if the new size is a valid size for this text view (See getMinumumSize).
        \param  newSize the new size of this progress bar
    */
    void resize(Point newSize) override
    {
        resize(newSize.x,newSize.y);
    }

    /**
        This method resizes the text view to width and height. This method should only
        called if the new size is a valid size for this text view (See getMinumumSize).
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
        Returns whether this widget can be set active.
        \return true = activatable, false = not activatable
    */
    inline bool isActivatable() const override { return isEnabled(); };

    /**
        Is the scrollbar always shown or is it hidden if not needed
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
        Scrolls the text box to the start.
    */
    void scrollToStart() {
        scrollbar.setCurrentValue(0);
    }

    /**
        Scrolls the text box to the last line.
    */
    void scrollToEnd() {
        scrollbar.setCurrentValue(scrollbar.getRangeMax());
    }

protected:
    /**
        This method is called whenever the textures of this widget are needed, e.g. before drawing. This method
        should be overwritten by subclasses if they like to defer texture creation as long as possible.
        This method should first check whether a renewal of the textures is necessary.
    */
    void updateTextures() override;

    /**
        This method frees all textures that are used by this text view
    */
    void invalidateTextures() override
    {
        pBackground.reset();
        pForeground.reset();
    }

private:

    int fontSize = 14;                  ///< the size of the font to use
    Uint32 textcolor = COLOR_DEFAULT;           ///< the text color
    Uint32 textshadowcolor = COLOR_DEFAULT;     ///< the color of the shadow of the text
    Uint32 backgroundcolor = COLOR_TRANSPARENT; ///< the color of the label background
    std::string text;                           ///< the text of this label

    Alignment_Enum alignment = static_cast<Alignment_Enum>(Alignment_Left | Alignment_Top);   ///< the alignment of this label

    sdl2::texture_ptr pBackground = nullptr;
    sdl2::texture_ptr pForeground = nullptr;
    ScrollBar scrollbar;

    bool bAutohideScrollbar = true;     ///< hide the scrollbar if not needed (default = true)
};

#endif //TEXTVIEW_H
