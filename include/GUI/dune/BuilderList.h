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

#ifndef BUILDERLIST_H
#define BUILDERLIST_H

#include "data.h"
#include <GUI/PictureButton.h>
#include <GUI/StaticContainer.h>
#include <GUI/TextButton.h>
#include <misc/SDL2pp.h>

inline constexpr auto ARROWBTN_WIDTH     = 48;
inline constexpr auto ARROWBTN_HEIGHT    = 16;
inline constexpr auto BUILDERBTN_HEIGHT  = 55;
inline constexpr auto BUILDERBTN_WIDTH   = 91;
inline constexpr auto BUILDERBTN_SPACING = 5;
inline constexpr auto ORDERBTN_HEIGHT    = 16;

inline constexpr auto WIDGET_WIDTH = BUILDERBTN_SPACING + BUILDERBTN_WIDTH + BUILDERBTN_SPACING;

class BuilderList final : public StaticContainer {
    using parent = StaticContainer;

public:
    /**
        Handles a mouse movement.
        \param  x               x-coordinate (relative to the left top corner of the widget)
        \param  y               y-coordinate (relative to the left top corner of the widget)
        \param  insideOverlay   true, if (x,y) is inside an overlay and this widget may be behind it, false otherwise
    */
    void handleMouseMovement(int32_t x, int32_t y, bool insideOverlay) override;

    /**
        Handles a left mouse click.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the widget, false = click was not processed by the widget
    */
    bool handleMouseLeft(int32_t x, int32_t y, bool pressed) override;

    /**
        Handles a right mouse click.
        \param  x x-coordinate (relative to the left top corner of the widget)
        \param  y y-coordinate (relative to the left top corner of the widget)
        \param  pressed true = mouse button pressed, false = mouse button released
        \return true = click was processed by the widget, false = click was not processed by the widget
    */
    bool handleMouseRight(int32_t x, int32_t y, bool pressed) override;

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
        Handles a key stroke.
        \param  key the key that was pressed or released.
        \return true = key stroke was processed by the widget, false = key stroke was not processed by the widget
    */
    bool handleKeyPress(SDL_KeyboardEvent& key) override;

    /**
        Draws this widget to screen. This method is called before drawOverlay().
        \param  Position    Position to draw the widget to
    */
    void draw(Point position) override;

    /**
        This method draws the parts of this window that must be drawn after all the other
        widgets are drawn (e.g. tooltips). This method is called after draw().
        \param  Position    Position to draw the window to. The position of the window is added to this.
    */
    void drawOverlay(Point position) override;

    /**
        This method resizes the builder list to width and height. This method should only be
        called if the new size is a valid size for this builder list (See resizingXAllowed,
        resizingYAllowed, getMinimumSize).
        \param  width   the new width of this widget
        \param  height  the new height of this widget
    */
    void resize(uint32_t width, uint32_t height) override;

    using parent::resize;

    /**
        Returns the minimum size of this widget. The widget should not
        resized to a size smaller than this. If the widget is not resizeable
        in a direction this method returns the size in that direction.
        \return the minimum size of this widget
    */
    Point getMinimumSize() const override {
        return {WIDGET_WIDTH, BUILDERBTN_HEIGHT * 3 + (ARROWBTN_HEIGHT + BUILDERBTN_SPACING) * 2
                                  + BUILDERBTN_SPACING * 4 + ORDERBTN_HEIGHT + BUILDERBTN_SPACING};
    }

    static BuilderList* create(uint32_t builderObjectID) {
        auto* tmp       = new BuilderList(builderObjectID);
        tmp->pAllocated = true;
        return tmp;
    }

    BuilderList(const BuilderList&)            = delete;
    BuilderList(BuilderList&&)                 = delete;
    BuilderList& operator=(const BuilderList&) = delete;
    BuilderList& operator=(BuilderList&&)      = delete;

private:
    explicit BuilderList(uint32_t builderObjectID);
    ~BuilderList() override;

    static int getRealHeight(int height);
    static int getNumButtons(int height);
    static Point getButtonPosition(int BtnNumber);
    int getButton(int x, int y) const;
    ItemID_enum getItemIDFromIndex(int i) const;

    void onUp();
    void onDown();
    void onOrder() const;
    void onCancel() const;

    int currentListPos = 0;
    PictureButton upButton;
    PictureButton downButton;
    uint32_t builderObjectID;

    TextButton orderButton;

    int mouseLeftButton  = -1;
    int mouseRightButton = -1;

    DuneTextureOwned pSoldOutTextTexture;
    DuneTextureOwned pAlreadyBuiltTextTexture;
    DuneTextureOwned pPlaceItTextTexture;
    DuneTextureOwned pOnHoldTextTexture;
    DuneTextureOwned pUnitLimitReachedTextTexture;

    DuneTextureOwned pLastTooltip;
    std::string tooltipText;
    dune::dune_clock::time_point lastMouseMovement = dune::dune_clock::time_point::max();
    Point lastMousePos;
};

#endif // BUILDERLIST_H
