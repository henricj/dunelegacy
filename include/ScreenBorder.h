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

#ifndef SCREENBORDER_H
#define SCREENBORDER_H

#include <DataTypes.h>

#include <misc/InputStream.h>
#include <misc/OutputStream.h>
#include <fixmath/FixPoint.h>
#include <mmath.h>
#include <globals.h>

#include <algorithm>

#define SCROLLBORDER 3

/// This class manages everything that is related to the current view onto the map.
class ScreenBorder
{
public:
    /**
        Constructor
        \param gameBoardRect    this SDL_Rect specifies the rectangle on the screen where the map is shown
    */
    explicit ScreenBorder(const SDL_Rect& gameBoardRect)
     : mapSizeX(0), mapSizeY(0) {

        this->gameBoardRect = gameBoardRect;

        bottomRightCorner.x = gameBoardRect.w;
        bottomRightCorner.y = gameBoardRect.h;
        topLeftCornerOnScreen.x = gameBoardRect.x;
        topLeftCornerOnScreen.y = gameBoardRect.y;
        bottomRightCornerOnScreen.x = gameBoardRect.x + gameBoardRect.w;
        bottomRightCornerOnScreen.y = gameBoardRect.y + gameBoardRect.h;

        numShakingCycles = 0;
    };

    /**
        Destructor
    */
    ~ScreenBorder() { }

    /**
        Loads the current position on the map from a stream
        \param stream the stream to load from
    */
    void load(InputStream& stream) {
        Coord center;
        center.x = stream.readSint16();
        center.y = stream.readSint16();

        setNewScreenCenter(center);
    }

    /**
        Saves the current map position to a stream
        \param stream the stream to save to
    */
    void save(OutputStream& stream) const
    {
        Coord center = getCurrentCenter();
        stream.writeSint16(center.x);
        stream.writeSint16(center.y);
    }

    /**
        Get the current center point of the view in world coordinates.
        \return current center point in world coordinates.
    */
    inline Coord getCurrentCenter() const
    {
        return topLeftCorner + shakingOffset + (bottomRightCorner - topLeftCorner)/2;
    }

    /**
        Returns the top left corner of the view in world coordinates.
        \return current top left corner in world coordinates.
    */
    inline Coord getTopLeftCorner() const
    {
        return topLeftCorner + shakingOffset;
    }

    /**
        Returns the bottom right corner of the view in world coordinates.
        \return current bottom right corner in world coordinates.
    */
    inline Coord getBottomRightCorner() const
    {
        return bottomRightCorner + shakingOffset;
    }

    /**
        Returns the position of the left edge of the view in world coordinates.
        \return current left edge in world coordinates.
    */
    inline short getLeft() const
    {
        return topLeftCorner.x + shakingOffset.x;
    }

    /**
        Returns the position of the top edge of the view in world coordinates.
        \return current top edge in world coordinates.
    */
    inline short getTop() const
    {
        return topLeftCorner.y + shakingOffset.y;
    }

    /**
        Returns the position of the right edge of the view in world coordinates.
        \return current right edge in world coordinates.
    */
    inline short getRight() const
    {
        return bottomRightCorner.x + shakingOffset.x;
    }

    /**
        Returns the position of the bottom edge of the view in world coordinates.
        \return current bottom edge in world coordinates.
    */
    inline short getBottom() const
    {
        return bottomRightCorner.y + shakingOffset.y;
    }

    /**
        Returns the map coordinate of the top left corner of the current view.
        \return map coordinate of the top left corner
    */
    inline Coord getTopLeftTile() const
    {
        return (topLeftCorner + shakingOffset) / TILESIZE;
    }

    /**
        Returns the map coordinate of the bottom right corner of the current view.
        \return map coordinate of the bottom right corner
    */
    inline Coord getBottomRightTile() const
    {
        return (bottomRightCorner + shakingOffset) / TILESIZE;
    }

    /**
        There may be a part of the tile at the top left corner that is outside the screen.
        This method returns how much is outside the screen (in world coordinates).
        \return the part of the tile that lies outside the screen.
    */
    inline Coord getCornerOffset() const
    {
        return ((topLeftCorner + shakingOffset) / TILESIZE) * TILESIZE - (topLeftCorner + shakingOffset);
    }

    /**
        This method checks if some object is (partly) inside or completely outside the current view.
        \param objectPosition   object position in world coordinates
        \param objectSize       the size of the object (in world coordinates)
        \return true if (partly) inside, false if completly outside
    */
    inline bool isInsideScreen(const Coord& objectPosition, const Coord& objectSize) const
    {
        return (((objectPosition.x + objectSize.x/2) >= topLeftCorner.x + shakingOffset.x)
                && ((objectPosition.x - objectSize.x/2) <= bottomRightCorner.x + shakingOffset.x)
                && ((objectPosition.y + objectSize.y/2) >= topLeftCorner.y + shakingOffset.y)
                && ((objectPosition.y - objectSize.y/2) <= bottomRightCorner.y + shakingOffset.y) );
    }

    /**
        This method checks if a tile is (partly) inside the current view.
        \param tileLocation the location of the tile in map coordinates
        \return true if (partly) inside, false if completly outside
    */
    inline bool isTileInsideScreen(const Coord& tileLocation) const
    {
        return isInsideScreen(tileLocation*TILESIZE + Coord(TILESIZE/2, TILESIZE/2), Coord(TILESIZE,TILESIZE));
    }

    /**
        Sets the current view to a new position.
        \param newPosition  the center of the new view in world coordinates
    */
    void setNewScreenCenter(const Coord& newPosition);

    /**
        This method scrolls left
    */
    bool scrollLeft();

    /**
        This method scrolls right
    */
    bool scrollRight();

        /**
        This method scrolls up
    */
    bool scrollUp();

    /**
        This method scrolls down
    */
    bool scrollDown();

    /**
        This method converts from world to screen coordinates.
        \param x    the x position in world coordinates
        \return the x-coordinate on the screen
    */
    inline int world2screenX(int x) const
    {
        return world2zoomedWorld(x - topLeftCorner.x + shakingOffset.x + topLeftCornerOnScreen.x);
    }

    /**
        This method converts from world to screen coordinates.
        \param x    the x position in world coordinates
        \return the x-coordinate on the screen
    */
    inline int world2screenX(float x) const
    {
        return world2zoomedWorld(x - (float) topLeftCorner.x + (float) shakingOffset.x + (float) topLeftCornerOnScreen.x);
    }

    /**
        This method converts from world to screen coordinates.
        \param x    the x position in world coordinates
        \return the x-coordinate on the screen
    */
    inline int world2screenX(FixPoint x) const
    {
        return world2zoomedWorld(x.toFloat() - (float) topLeftCorner.x + (float) shakingOffset.x + (float) topLeftCornerOnScreen.x);
    }

    /**
        This method converts from world to screen coordinates.
        \param y    the y position in world coordinates
        \return the y-coordinate on the screen
    */
    inline int world2screenY(int y) const
    {
        return world2zoomedWorld(y - topLeftCorner.y + shakingOffset.y + topLeftCornerOnScreen.y);
    }

    /**
        This method converts from world to screen coordinates.
        \param y    the y position in world coordinates
        \return the y-coordinate on the screen
    */
    inline int world2screenY(float y) const
    {
        return world2zoomedWorld(y - (float) topLeftCorner.y + (float) shakingOffset.y + (float) topLeftCornerOnScreen.y);
    }

    /**
        This method converts from world to screen coordinates.
        \param y    the y position in world coordinates
        \return the y-coordinate on the screen
    */
    inline int world2screenY(FixPoint y) const
    {
        return world2zoomedWorld(y.toFloat() - (float) topLeftCorner.y + (float) shakingOffset.y + (float) topLeftCornerOnScreen.y);
    }

    /**
        This method converts from screen to world coordinates.
        \param x    the x coordinate on the screen
        \return the x-position in world coordinates
    */
    inline int screen2worldX(int x) const
    {
        return zoomedWorld2world(x) - topLeftCornerOnScreen.x + topLeftCorner.x + shakingOffset.x;
    }

    /**
        This method converts from screen to world coordinates.
        \param y    the y coordinate on the screen
        \return the y-position in world coordinates
    */
    inline int screen2worldY(int y) const
    {
        return zoomedWorld2world(y) - topLeftCornerOnScreen.y + topLeftCorner.y + shakingOffset.y;
    }

    /**
        This method converts from screen to map coordinates.
        \param x    the x coordinate on the screen
        \return the x-coordinate of the tile at position x in map coordinates
    */
    inline int screen2MapX(int x) const
    {
        return screen2worldX(x)/TILESIZE;
    };

    /**
        This method converts from screen to map coordinates.
        \param y    the y coordinate on the screen
        \return the y-coordinate of the tile at position y in map coordinates
    */
    inline int screen2MapY(int y) const
    {
        return screen2worldY(y)/TILESIZE;
    };

    /**
        This method checks if the specified x,y coordinate is within the map
        \param  x the x coordinate in screen coordinates
        \param  y the y coordinate in screen coordinates
        \return true, if inside, false otherwise
    */
    inline bool isScreenCoordInsideMap(int x, int y) const {
        return (zoomedWorld2world(x) >= topLeftCornerOnScreen.x  && zoomedWorld2world(x) < bottomRightCornerOnScreen.x
                && zoomedWorld2world(y) >= topLeftCornerOnScreen.y  && zoomedWorld2world(y) < bottomRightCornerOnScreen.y);
    }

    /**
        This method adjusts the screen border to the current map size.
        \param newMapSizeX         the number of map tiles in x direction
        \param newMapSizeY         the number of map tiles in y direction
    */
    void adjustScreenBorderToMapsize(int newMapSizeX, int newMapSizeY);


    void shakeScreen(int numShakingCycles) {
        this->numShakingCycles += numShakingCycles;
        if(this->numShakingCycles > 2*numShakingCycles) {
            this->numShakingCycles = 2*numShakingCycles;
        }
    }

    void update() {
        if(numShakingCycles > 0) {
            int offsetMax = std::min(TILESIZE-1,numShakingCycles);

            shakingOffset.x = getRandomInt(-offsetMax/2, offsetMax/2);
            shakingOffset.y = getRandomInt(-offsetMax/2, offsetMax/2);

            numShakingCycles--;
        }
    }

private:
    SDL_Rect gameBoardRect;         ///< the complete game board rectangle

    int mapSizeX;                   ///< The number of tiles in x direction
    int mapSizeY;                   ///< The number of tiles in y direction

    Coord topLeftCorner;            ///< the position of the top left corner in world coordinates
    Coord bottomRightCorner;        ///< the position of the bottom right corner in world coordinates

    Coord shakingOffset;            ///< this offset is added while shaking the screen (e.g. when a death hand explodes)

    Coord topLeftCornerOnScreen;    ///< the position of the top left corner in screen coordinates
    Coord bottomRightCornerOnScreen;///< the position of the bottom right corner in screen coordinates

    int numShakingCycles;           ///< the number of cycles the screen will shake
};

#endif //SCREENBORDER
