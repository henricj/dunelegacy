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

#ifndef RADARVIEWBASE_H
#define RADARVIEWBASE_H

#include <GUI/Widget.h>

#include <DataTypes.h>

#include <functional>

inline constexpr auto NUM_STATIC_FRAMES     = 21;
inline constexpr auto NUM_STATIC_FRAME_TIME = 5;

inline constexpr auto RADARVIEW_BORDERTHICKNESS = 2;
inline constexpr auto RADARWIDTH                = 128;
inline constexpr auto RADARHEIGHT               = 128;

/// This class manages the mini map at the top right corner of the screen
class RadarViewBase : public Widget {
public:
    /**
        Constructor
    */
    RadarViewBase();

    /**
        Destructor
    */
    ~RadarViewBase() override;

    /**
        Get the map size in x direction
        \return map width
    */
    [[nodiscard]] virtual int getMapSizeX() const = 0;

    /**
        Get the map size in y direction
        \return map height
    */
    [[nodiscard]] virtual int getMapSizeY() const = 0;

    /**
        Draws the radar to screen. This method is called before drawOverlay().
        \param  position    Position to draw the radar to
    */
    void draw(Point position) override { }

    /**
        This method checks if position is inside the radar view
        \param mouseX the x-coordinate to check (relative to the top left corner of the radar)
        \param mouseY the y-coordinate to check (relative to the top left corner of the radar)
        \return true, if inside the radar view; false otherwise
    */
    [[nodiscard]] bool isOnRadar(int mouseX, int mouseY) const;

    /**
        This method returns the corresponding world coordinates for a point on the radar
        \param mouseX  the position on the radar screen (relative to the top left corner of the radar)
        \param mouseY  the position on the radar screen (relative to the top left corner of the radar)
        \return the world coordinates
    */
    [[nodiscard]] Coord getWorldCoords(int mouseX, int mouseY) const;

    /**
        This method calculates the scale and the offsets that are necessary to show a minimap centered inside a 128x128
       rectangle.
        \param  MapSizeX    The width of the map in tiles
        \param  MapSizeY    The height of the map in tiles
        \param  scale       The scale factor is saved here
        \param  offsetX     The offset in x direction is saved here
        \param  offsetY     The offset in y direction is saved here
    */
    static void calculateScaleAndOffsets(int MapSizeX, int MapSizeY, int& scale, int& offsetX, int& offsetY);

    /**
        Returns the minimum size of this widget. The widget should not
        resized to a size smaller than this. If the widget is not resizeable
        in a direction this method returns the size in that direction.
        \return the minimum size of this widget
    */
    [[nodiscard]] Point getMinimumSize() const override {
        return Point(RADARWIDTH + (2 * RADARVIEW_BORDERTHICKNESS), RADARHEIGHT + (2 * RADARVIEW_BORDERTHICKNESS));
    }

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
        Sets the function that should be called when the radar view is clicked.
        \param  pOnRadarClick   A function to be called on click
    */
    void setOnRadarClick(std::function<bool(Coord, bool, bool)> pOnRadarClick) { this->pOnRadarClick = std::move(pOnRadarClick); }

protected:
    std::function<bool(Coord, bool, bool)>
        pOnRadarClick; ///< this function is called when the user clicks on the radar (1st parameter is world
                       ///< coordinate; 2nd parameter is whether the right mouse button was pressed; 3rd parameter is
                       ///< whether the mouse was moved while being pressed, e.g. dragging; return value shall be true
                       ///< if dragging should start or continue)

    bool bRadarInteraction; ///< currently dragging on the radar? (e.g. moving the view rectangle on the radar)
};

#endif // RADARVIEWBASE_H
