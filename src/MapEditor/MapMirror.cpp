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

#include <MapEditor/MapMirror.h>

#include <mmath.h>

MapMirror::MapMirror(int mapsizeX, int mapsizeY)
 : mapsizeX(mapsizeX), mapsizeY(mapsizeY) {
}

MapMirror::~MapMirror() = default;

std::unique_ptr<MapMirror> MapMirror::createMapMirror(MirrorMode mirrorMode, int mapsizeX, int mapsizeY) {
    // clang-format off
    switch(mirrorMode) {
        case MirrorModeNone:        return std::make_unique<MapMirrorNone>(mapsizeX, mapsizeY);          break;
        case MirrorModeHorizontal:  return std::make_unique<MapMirrorHorizontal>(mapsizeX, mapsizeY);    break;
        case MirrorModeVertical:    return std::make_unique<MapMirrorVertical>(mapsizeX, mapsizeY);      break;
        case MirrorModeBoth:        return std::make_unique<MapMirrorBoth>(mapsizeX, mapsizeY);          break;
        case MirrorModePoint:       return std::make_unique<MapMirrorPoint>(mapsizeX, mapsizeY);         break;
        default:                    return std::unique_ptr<MapMirror>();
    }
    // clang-format off
}



MapMirrorNone::MapMirrorNone(int mapsizeX, int mapsizeY)
 : MapMirror(mapsizeX, mapsizeY) {
}

Coord MapMirrorNone::getCoord(Coord originalCoord, int i, Coord objectSize) const {
    return originalCoord;
}

ANGLETYPE MapMirrorNone::getAngle(ANGLETYPE angle, int i) const {
    return angle;
}



MapMirrorHorizontal::MapMirrorHorizontal(int mapsizeX, int mapsizeY)
 : MapMirror(mapsizeX, mapsizeY) {
}

Coord MapMirrorHorizontal::getCoord(Coord originalCoord, int i, Coord objectSize) const {
    switch(i%2) {
        case 0:     return originalCoord;
        case 1:     return Coord(mapsizeX - originalCoord.x - objectSize.x, originalCoord.y);
        default:    return originalCoord;
    }
}

ANGLETYPE MapMirrorHorizontal::getAngle(ANGLETYPE angle, int i) const {
    switch(i%2) {
        case 0:     return angle;
        case 1:     return mirrorAngleHorizontal(angle);
        default:    return angle;
    }
}



MapMirrorVertical::MapMirrorVertical(int mapsizeX, int mapsizeY)
 : MapMirror(mapsizeX, mapsizeY) {
}

Coord MapMirrorVertical::getCoord(Coord originalCoord, int i, Coord objectSize) const {
    switch(i%2) {
        case 0:     return originalCoord;
        case 1:     return Coord(originalCoord.x, mapsizeY - originalCoord.y - objectSize.y);
        default:    return originalCoord;
    }
}

ANGLETYPE MapMirrorVertical::getAngle(ANGLETYPE angle, int i) const {
    switch(i%2) {
        case 0:     return angle;
        case 1:     return mirrorAngleVertical(angle);
        default:    return angle;
    }
}



MapMirrorBoth::MapMirrorBoth(int mapsizeX, int mapsizeY)
 : MapMirror(mapsizeX, mapsizeY) {
}

Coord MapMirrorBoth::getCoord(Coord originalCoord, int i, Coord objectSize) const {
    switch(i%4) {
        case 0:     return originalCoord;
        case 1:     return Coord(mapsizeX - originalCoord.x - objectSize.x, originalCoord.y);
        case 2:     return Coord(mapsizeX - originalCoord.x - objectSize.x, mapsizeY - originalCoord.y - objectSize.y);
        case 3:     return Coord(originalCoord.x, mapsizeY - originalCoord.y - objectSize.y);
        default:    return originalCoord;
    }
}

ANGLETYPE MapMirrorBoth::getAngle(ANGLETYPE angle, int i) const {
    switch(i%4) {
        case 0:     return angle;
        case 1:     return mirrorAngleHorizontal(angle);
        case 2:     return mirrorAngleHorizontal(mirrorAngleVertical(angle));
        case 3:     return mirrorAngleVertical(angle);
        default:    return angle;
    }
}



MapMirrorPoint::MapMirrorPoint(int mapsizeX, int mapsizeY)
 : MapMirror(mapsizeX, mapsizeY) {
}

Coord MapMirrorPoint::getCoord(Coord originalCoord, int i, Coord objectSize) const {
    switch(i%2) {
        case 0:     return originalCoord;
        case 1:     return Coord(mapsizeX - originalCoord.x - objectSize.x, mapsizeY - originalCoord.y - objectSize.y);
        default:    return originalCoord;
    }
}

ANGLETYPE MapMirrorPoint::getAngle(ANGLETYPE angle, int i) const {
    switch(i%2) {
        case 0:     return angle;
        case 1: {
            auto int_angle = static_cast<int>(angle) + static_cast<int>(ANGLETYPE::NUM_ANGLES) / 2;

            return normalizeAngle(static_cast<ANGLETYPE>(int_angle));
        }
        default:    return angle;
    }
}

