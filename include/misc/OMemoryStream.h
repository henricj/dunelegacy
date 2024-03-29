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

#ifndef OMEMORYSTREAM_H
#define OMEMORYSTREAM_H

#include "OutputStream.h"

#include <string>

class OMemoryStream final : public OutputStream {
public:
    OMemoryStream();

    ~OMemoryStream() override;

    void open();

    [[nodiscard]] const char* getData() const { return pBuffer; }

    [[nodiscard]] size_t getDataLength() const { return (bReadOnly == true) ? bufferSize : currentPos; }

    void flush() override;

    void writeString(const std::string& str) override;

    void writeUint8(Uint8 x) override;

    void writeUint16(Uint16 x) override;

    void writeUint32(Uint32 x) override;

    void writeUint64(Uint64 x) override;

    void writeBool(bool x) override;

    void writeFloat(float x) override;

    void ensureBufferSize(size_t minBufferSize);

private:
    size_t currentPos{};
    size_t bufferSize{};
    char* pBuffer{};
};

#endif // OMEMORYSTREAM_H
