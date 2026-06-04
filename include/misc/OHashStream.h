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

#ifndef OHASHSTREAM_H
#define OHASHSTREAM_H

#include "OutputStream.h"

#include <digestpp/algorithm/sha2.hpp>

#include <array>
#include <cstdint>
#include <string>

/// Streaming OutputStream that accumulates all written bytes into a SHA-384 digest.
/// Byte encoding matches OFileStream (little-endian multibyte integers, length-prefixed strings),
/// so the hash can be independently reproduced from saved game data.
/// Call finish() to retrieve the 48-byte digest without resetting state.
class OHashStream final : public OutputStream {
public:
    OHashStream();
    ~OHashStream() override;

    void flush() override;

    void writeString(std::string_view str) override;

    void writeUint8(uint8_t x) override;
    void writeUint16(uint16_t x) override;
    void writeUint32(uint32_t x) override;
    void writeUint64(uint64_t x) override;
    void writeBool(bool x) override;
    void writeFloat(float x) override;

    /// Returns the SHA-384 digest of all bytes written so far.
    /// Does not modify the stream state; safe to call multiple times.
    [[nodiscard]] std::array<uint8_t, 48> finish() const;

    /// Returns the SHA-384 digest as a lowercase hex string.
    /// Does not modify the stream state; safe to call multiple times.
    [[nodiscard]] std::string hexdigest() const;

private:
    digestpp::sha384 hasher_;
};

#endif // OHASHSTREAM_H
