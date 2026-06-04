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

#include "misc/OHashStream.h"

#include <misc/SDL2pp.h>

#include <cstring>

OHashStream::OHashStream() = default;

OHashStream::~OHashStream() = default;

void OHashStream::flush() { }

void OHashStream::writeString(std::string_view str) {
    writeUint32(static_cast<uint32_t>(str.size()));
    if (!str.empty())
        hasher_.absorb(reinterpret_cast<const uint8_t*>(str.data()), str.size());
}

void OHashStream::writeUint8(uint8_t x) {
    hasher_.absorb(&x, 1);
}

void OHashStream::writeUint16(uint16_t x) {
    x = SDL_Swap16LE(x);
    hasher_.absorb(reinterpret_cast<const uint8_t*>(&x), sizeof(x));
}

void OHashStream::writeUint32(uint32_t x) {
    x = SDL_Swap32LE(x);
    hasher_.absorb(reinterpret_cast<const uint8_t*>(&x), sizeof(x));
}

void OHashStream::writeUint64(uint64_t x) {
    x = SDL_Swap64LE(x);
    hasher_.absorb(reinterpret_cast<const uint8_t*>(&x), sizeof(x));
}

void OHashStream::writeBool(bool x) {
    writeUint8(x ? 1 : 0);
}

void OHashStream::writeFloat(float x) {
    uint32_t tmp = 0;
    memcpy(&tmp, &x, sizeof(tmp));
    writeUint32(tmp);
}

std::array<uint8_t, 48> OHashStream::finish() const {
    std::array<uint8_t, 48> result{};
    hasher_.digest(result.data(), result.size());
    return result;
}

std::string OHashStream::hexdigest() const {
    return hasher_.hexdigest();
}
