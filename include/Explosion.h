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

#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <DataTypes.h>
#include <misc/InputStream.h>
#include <misc/OutputStream.h>
#include <misc/SDL2pp.h>

class Explosion
{
public:
    Explosion();
    Explosion(Uint32 explosionID, const Coord& position, int house = HOUSE_HARKONNEN);
    explicit Explosion(InputStream& stream);
    ~Explosion();

    void init();

    void save(OutputStream& stream) const;

    void blitToScreen() const;

    void update();

private:
    Uint32 explosionID;
    Coord position;
    int house;
    zoomable_texture graphic{};
    int numFrames = 0;
    int currentFrame;
    int frameTimer;
};


#endif // EXPLOSION_H
