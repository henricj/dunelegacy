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

#ifndef MAPMIRROR_H
#define MAPMIRROR_H

#include <DataTypes.h>

typedef enum { MirrorModeNone, MirrorModeHorizontal, MirrorModeVertical, MirrorModeBoth, MirrorModePoint } MirrorMode;

class MapMirror {
public:
    MapMirror(int mapsizeX, int mapsizeY);

    virtual ~MapMirror();

    [[nodiscard]] virtual int getSize() const = 0;

    [[nodiscard]] virtual bool mirroringPossible(Coord coord, Coord objectSize = Coord(1, 1)) const = 0;

    [[nodiscard]] virtual Coord getCoord(Coord originalCoord, int i, Coord objectSize = Coord(1, 1)) const = 0;

    [[nodiscard]] virtual ANGLETYPE getAngle(ANGLETYPE angle, int i) const = 0;

    static std::unique_ptr<MapMirror> createMapMirror(MirrorMode mirrorMode, int mapsizeX, int mapsizeY);

protected:
    int mapsizeX;
    int mapsizeY;
};

class MapMirrorNone final : public MapMirror {
public:
    MapMirrorNone(int mapsizeX, int mapsizeY);

    [[nodiscard]] int getSize() const override { return 1; }

    [[nodiscard]] bool
    mirroringPossible([[maybe_unused]] Coord coord, [[maybe_unused]] Coord objectSize = Coord(1, 1)) const override {
        return true;
    }

    [[nodiscard]] Coord getCoord(Coord originalCoord, int i, Coord objectSize = Coord(1, 1)) const override;

    [[nodiscard]] ANGLETYPE getAngle(ANGLETYPE angle, int i) const override;
};

class MapMirrorHorizontal final : public MapMirror {
public:
    MapMirrorHorizontal(int mapsizeX, int mapsizeY);

    [[nodiscard]] int getSize() const override { return 2; }

    [[nodiscard]] bool mirroringPossible(Coord coord, Coord objectSize = Coord(1, 1)) const override {
        return !(coord.x < mapsizeX / 2 && coord.x + objectSize.x - 1 >= mapsizeX / 2);
    }

    [[nodiscard]] Coord getCoord(Coord originalCoord, int i, Coord objectSize = Coord(1, 1)) const override;

    [[nodiscard]] ANGLETYPE getAngle(ANGLETYPE angle, int i) const override;
};

class MapMirrorVertical final : public MapMirror {
public:
    MapMirrorVertical(int mapsizeX, int mapsizeY);

    [[nodiscard]] int getSize() const override { return 2; }

    [[nodiscard]] bool mirroringPossible(Coord coord, Coord objectSize = Coord(1, 1)) const override {
        return !(coord.y < mapsizeY / 2 && coord.y + objectSize.y - 1 >= mapsizeY / 2);
    }

    [[nodiscard]] Coord getCoord(Coord originalCoord, int i, Coord objectSize = Coord(1, 1)) const override;

    [[nodiscard]] ANGLETYPE getAngle(ANGLETYPE angle, int i) const override;
};

class MapMirrorBoth final : public MapMirror {
public:
    MapMirrorBoth(int mapsizeX, int mapsizeY);

    [[nodiscard]] int getSize() const override { return 4; }

    [[nodiscard]] bool mirroringPossible(Coord coord, Coord objectSize = Coord(1, 1)) const override {
        return !((coord.x < mapsizeX / 2 && coord.x + objectSize.x - 1 >= mapsizeX / 2)
                 || (coord.y < mapsizeY / 2 && coord.y + objectSize.y - 1 >= mapsizeY / 2));
    }

    [[nodiscard]] Coord getCoord(Coord originalCoord, int i, Coord objectSize = Coord(1, 1)) const override;

    [[nodiscard]] ANGLETYPE getAngle(ANGLETYPE angle, int i) const override;
};

class MapMirrorPoint final : public MapMirror {
public:
    MapMirrorPoint(int mapsizeX, int mapsizeY);

    [[nodiscard]] int getSize() const override { return 2; }

    [[nodiscard]] bool mirroringPossible(Coord coord, Coord objectSize = Coord(1, 1)) const override {
        return !((coord.x < mapsizeX / 2 && coord.x + objectSize.x - 1 >= mapsizeX / 2)
                 && (coord.y < mapsizeY / 2 && coord.y + objectSize.y - 1 >= mapsizeY / 2));
    }

    [[nodiscard]] Coord getCoord(Coord originalCoord, int i, Coord objectSize = Coord(1, 1)) const override;

    [[nodiscard]] ANGLETYPE getAngle(ANGLETYPE angle, int i) const override;
};

#endif // MAPMIRROR_H
