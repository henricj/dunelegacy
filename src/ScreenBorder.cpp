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

#include "misc/Random.h"
#include <misc/InputStream.h>
#include <misc/OutputStream.h>

#include <globals.h>

#include <algorithm>

ScreenBorder::ScreenBorder(const SDL_FRect& gameBoardRect) : gameBoardRect_(gameBoardRect) {

    bottomRightCorner_.x         = static_cast<int>(std::ceil(gameBoardRect.w));
    bottomRightCorner_.y         = static_cast<int>(std::ceil(gameBoardRect.h));
    topLeftCornerOnScreen_.x     = gameBoardRect.x;
    topLeftCornerOnScreen_.y     = gameBoardRect.y;
    bottomRightCornerOnScreen_.x = gameBoardRect.x + gameBoardRect.w;
    bottomRightCornerOnScreen_.y = gameBoardRect.y + gameBoardRect.h;
}

ScreenBorder::~ScreenBorder() = default;

void ScreenBorder::load(InputStream& stream) {
    Coord center;
    center.x = stream.readSint16();
    center.y = stream.readSint16();

    setNewScreenCenter(center);
}

void ScreenBorder::save(OutputStream& stream) const {
    const auto center = getCurrentCenter();
    stream.writeSint16(static_cast<Sint16>(center.x));
    stream.writeSint16(static_cast<Sint16>(center.y));
}

void ScreenBorder::setNewScreenCenter(const Coord& newPosition) {
    const Coord currentBorderSize = bottomRightCorner_ - topLeftCorner_;

    topLeftCorner_     = newPosition - currentBorderSize / 2;
    bottomRightCorner_ = newPosition + currentBorderSize / 2;

    const int worldSizeX = mapSizeX_ * TILESIZE;
    const int worldSizeY = mapSizeY_ * TILESIZE;

    if ((topLeftCorner_.x < 0) && (bottomRightCorner_.x >= worldSizeX)) {
        // screen is wider than map
        topLeftCorner_.x     = 0;
        bottomRightCorner_.x = worldSizeX;
    } else if (topLeftCorner_.x < 0) {
        // leaving the map at the left border
        bottomRightCorner_.x -= topLeftCorner_.x;
        topLeftCorner_.x = 0;
    } else if (bottomRightCorner_.x >= worldSizeX) {
        // leaving the map at the right border
        topLeftCorner_.x -= (bottomRightCorner_.x - worldSizeX);
        bottomRightCorner_.x = worldSizeX;
    }

    if ((topLeftCorner_.y < 0) && (bottomRightCorner_.y >= worldSizeY)) {
        // screen is higher than map
        topLeftCorner_.y     = 0;
        bottomRightCorner_.y = worldSizeY;
    } else if (topLeftCorner_.y < 0) {
        // leaving the map at the top border
        bottomRightCorner_.y -= topLeftCorner_.y;
        topLeftCorner_.y = 0;
    } else if (bottomRightCorner_.y >= worldSizeY) {
        // leaving the map at the bottom border
        topLeftCorner_.y -= (bottomRightCorner_.y - worldSizeY);
        bottomRightCorner_.y = worldSizeY;
    }
}

bool ScreenBorder::scrollLeft() {
    if (topLeftCorner_.x > 0) {
        const int scrollAmount = std::min(dune::globals::settings.general.scrollSpeed, topLeftCorner_.x);
        topLeftCorner_.x -= scrollAmount;
        bottomRightCorner_.x -= scrollAmount;
        return true;
    }
    return false;
}

bool ScreenBorder::scrollRight() {
    if (bottomRightCorner_.x < mapSizeX_ * TILESIZE - 1) {
        const int scrollAmount =
            std::min(dune::globals::settings.general.scrollSpeed, mapSizeX_ * TILESIZE - 1 - bottomRightCorner_.x);
        topLeftCorner_.x += scrollAmount;
        bottomRightCorner_.x += scrollAmount;
        return true;
    }
    return false;
}

bool ScreenBorder::scrollUp() {
    if (topLeftCorner_.y > 0) {
        const int scrollAmount = std::min(dune::globals::settings.general.scrollSpeed, topLeftCorner_.y);
        topLeftCorner_.y -= scrollAmount;
        bottomRightCorner_.y -= scrollAmount;
        return true;
    }
    return false;
}

bool ScreenBorder::scrollDown() {
    if (bottomRightCorner_.y < mapSizeY_ * TILESIZE - 1) {
        const int scrollAmount =
            std::min(dune::globals::settings.general.scrollSpeed, mapSizeY_ * TILESIZE - 1 - bottomRightCorner_.y);
        topLeftCorner_.y += scrollAmount;
        bottomRightCorner_.y += scrollAmount;
        return true;
    }
    return false;
}

void ScreenBorder::adjustScreenBorderToMapsize(int newMapSizeX, int newMapSizeY) {
    mapSizeX_ = newMapSizeX;
    mapSizeY_ = newMapSizeY;

    const int worldSizeX = mapSizeX_ * TILESIZE;
    const int worldSizeY = mapSizeY_ * TILESIZE;

    if (worldSizeX >= zoomedWorld2world(gameBoardRect_.w)) {
        topLeftCorner_.x             = 0;
        bottomRightCorner_.x         = zoomedWorld2world(gameBoardRect_.w);
        topLeftCornerOnScreen_.x     = zoomedWorld2worldF(gameBoardRect_.x);
        bottomRightCornerOnScreen_.x = zoomedWorld2worldF(gameBoardRect_.x + gameBoardRect_.w);
    } else {
        topLeftCorner_.x     = 0;
        bottomRightCorner_.x = worldSizeX;
        topLeftCornerOnScreen_.x =
            zoomedWorld2worldF(gameBoardRect_.x) + (zoomedWorld2worldF(gameBoardRect_.w) - worldSizeX) / 2;
        bottomRightCornerOnScreen_.x =
            zoomedWorld2worldF(gameBoardRect_.x) + (zoomedWorld2worldF(gameBoardRect_.w) - worldSizeX) / 2 + worldSizeX;
    }

    if (worldSizeY >= zoomedWorld2world(gameBoardRect_.h)) {
        topLeftCorner_.y             = 0;
        bottomRightCorner_.y         = zoomedWorld2world(gameBoardRect_.h);
        topLeftCornerOnScreen_.y     = zoomedWorld2worldF(gameBoardRect_.y);
        bottomRightCornerOnScreen_.y = zoomedWorld2worldF(gameBoardRect_.y + gameBoardRect_.h);
    } else {
        topLeftCorner_.y     = 0;
        bottomRightCorner_.y = worldSizeY;
        topLeftCornerOnScreen_.y =
            zoomedWorld2worldF(gameBoardRect_.y) + (zoomedWorld2worldF(gameBoardRect_.h) - worldSizeY) / 2;
        bottomRightCornerOnScreen_.y =
            zoomedWorld2worldF(gameBoardRect_.y) + (zoomedWorld2worldF(gameBoardRect_.h) - worldSizeY) / 2 + worldSizeY;
    }
}

void ScreenBorder::shakeScreen(int numShakingCycles) {
    this->numShakingCycles_ += numShakingCycles;
    if (this->numShakingCycles_ > 2 * numShakingCycles) {
        this->numShakingCycles_ = 2 * numShakingCycles;
    }
}

void ScreenBorder::update(Random& uiRandom) {
    if (numShakingCycles_ <= 0)
        return;

    const auto offsetMax = std::min(TILESIZE - 1, numShakingCycles_);

    shakingOffset_.x = uiRandom.rand(-offsetMax / 2, offsetMax / 2);
    shakingOffset_.y = uiRandom.rand(-offsetMax / 2, offsetMax / 2);

    numShakingCycles_--;
}
