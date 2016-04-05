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

#include <ScreenBorder.h>

#include <globals.h>

#include <algorithm>

void ScreenBorder::setNewScreenCenter(const Coord& newPosition) {
    Coord currentBorderSize = bottomRightCorner - topLeftCorner;

    topLeftCorner = newPosition - currentBorderSize/2;
    bottomRightCorner = newPosition + currentBorderSize/2;

    int worldSizeX = mapSizeX * TILESIZE;
    int worldSizeY = mapSizeY * TILESIZE;

    if((topLeftCorner.x < 0) && (bottomRightCorner.x >= worldSizeX)) {
        // screen is wider than map
        topLeftCorner.x = 0;
        bottomRightCorner.x = worldSizeX;
    } else if(topLeftCorner.x < 0) {
        // leaving the map at the left border
        bottomRightCorner.x -= topLeftCorner.x;
        topLeftCorner.x = 0;
    } else if(bottomRightCorner.x >= worldSizeX) {
        // leaving the map at the right border
        topLeftCorner.x -= (bottomRightCorner.x - worldSizeX);
        bottomRightCorner.x = worldSizeX;
    }

    if((topLeftCorner.y < 0) && (bottomRightCorner.y >= worldSizeY)) {
        // screen is higher than map
        topLeftCorner.y = 0;
        bottomRightCorner.y = worldSizeY;
    } else if(topLeftCorner.y < 0) {
        // leaving the map at the top border
        bottomRightCorner.y -= topLeftCorner.y;
        topLeftCorner.y = 0;
    } else if(bottomRightCorner.y >= worldSizeY) {
        // leaving the map at the bottom border
        topLeftCorner.y -= (bottomRightCorner.y - worldSizeY);
        bottomRightCorner.y = worldSizeY;
    }
}

bool ScreenBorder::scrollLeft() {
    if(topLeftCorner.x > 0) {
        int scrollAmount = std::min(settings.general.scrollSpeed, topLeftCorner.x);
        topLeftCorner.x -= scrollAmount;
        bottomRightCorner.x -= scrollAmount;
        return true;
    } else {
        return false;
    }
}

bool ScreenBorder::scrollRight() {
    if(bottomRightCorner.x < mapSizeX*TILESIZE-1) {
        int scrollAmount = std::min(settings.general.scrollSpeed,mapSizeX*TILESIZE-1-bottomRightCorner.x);
        topLeftCorner.x += scrollAmount;
        bottomRightCorner.x += scrollAmount;
        return true;
    } else {
        return false;
    }
}

bool ScreenBorder::scrollUp() {
    if(topLeftCorner.y > 0) {
        int scrollAmount = std::min(settings.general.scrollSpeed, topLeftCorner.y);
        topLeftCorner.y -= scrollAmount;
        bottomRightCorner.y -= scrollAmount;
        return true;
    } else {
        return false;
    }
}

bool ScreenBorder::scrollDown() {
    if(bottomRightCorner.y < mapSizeY*TILESIZE-1) {
        int scrollAmount = std::min(settings.general.scrollSpeed,mapSizeY*TILESIZE-1-bottomRightCorner.y);
        topLeftCorner.y += scrollAmount;
        bottomRightCorner.y += scrollAmount;
        return true;
    } else {
        return false;
    }
}

void ScreenBorder::adjustScreenBorderToMapsize(int newMapSizeX, int newMapSizeY) {
    mapSizeX = newMapSizeX;
    mapSizeY = newMapSizeY;

    int worldSizeX = mapSizeX * TILESIZE;
    int worldSizeY = mapSizeY * TILESIZE;

    if(worldSizeX >= zoomedWorld2world(gameBoardRect.w)) {
        topLeftCorner.x = 0;
        bottomRightCorner.x = zoomedWorld2world(gameBoardRect.w);
        topLeftCornerOnScreen.x = zoomedWorld2world(gameBoardRect.x);
        bottomRightCornerOnScreen.x = zoomedWorld2world(gameBoardRect.x + gameBoardRect.w);
    } else {
        topLeftCorner.x = 0;
        bottomRightCorner.x = worldSizeX;
        topLeftCornerOnScreen.x = zoomedWorld2world(gameBoardRect.x) + (zoomedWorld2world(gameBoardRect.w) - worldSizeX) / 2;
        bottomRightCornerOnScreen.x = zoomedWorld2world(gameBoardRect.x) + (zoomedWorld2world(gameBoardRect.w) - worldSizeX) / 2 + worldSizeX;
    }

    if(worldSizeY >= zoomedWorld2world(gameBoardRect.h)) {
        topLeftCorner.y = 0;
        bottomRightCorner.y = zoomedWorld2world(gameBoardRect.h);
        topLeftCornerOnScreen.y = zoomedWorld2world(gameBoardRect.y);
        bottomRightCornerOnScreen.y = zoomedWorld2world(gameBoardRect.y + gameBoardRect.h);
    } else {
        topLeftCorner.y = 0;
        bottomRightCorner.y = worldSizeY;
        topLeftCornerOnScreen.y = zoomedWorld2world(gameBoardRect.y) + (zoomedWorld2world(gameBoardRect.h) - worldSizeY) / 2;
        bottomRightCornerOnScreen.y = zoomedWorld2world(gameBoardRect.y) + (zoomedWorld2world(gameBoardRect.h) - worldSizeY) / 2 + worldSizeY;
    }
}
