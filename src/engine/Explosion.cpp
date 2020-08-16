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

#include <Explosion.h>

#include <Game.h>

namespace Dune::Engine {

Explosion::Explosion() : explosionID(NONE_ID), house(HOUSETYPE::HOUSE_HARKONNEN) {
}

Explosion::Explosion(uint32_t explosionID, const Coord& position, HOUSETYPE house)
    : explosionID(explosionID), position(position), house(house) {

}

Explosion::Explosion(InputStream& stream) {
    explosionID  = stream.readUint32();
    position.x   = stream.readSint16();
    position.y   = stream.readSint16();
    house        = static_cast<HOUSETYPE>(stream.readUint32());
    timer        = stream.readUint32();
}


void Explosion::save(OutputStream& stream) const {
    stream.writeUint32(explosionID);
    stream.writeSint16(position.x);
    stream.writeSint16(position.y);
    stream.writeUint32(static_cast<uint32_t>(house));
    stream.writeUint32(timer);
}

bool Explosion::update() {
    if(0 == timer) return true;

    --timer;

    return false;
}

} // namespace Dune::Engine
