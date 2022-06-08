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

#ifndef IMEMORYSTREAM_H
#define IMEMORYSTREAM_H

#include "InputStream.h"

#include <misc/exceptions.h>

#include <string>

class IMemoryStream final : public InputStream {
public:
    IMemoryStream();

    IMemoryStream(const char* data, size_t length) : bufferSize(length), pBuffer(data) { }

    ~IMemoryStream() override;

    void open(const char* data, size_t length) {
        currentPos = 0;
        bufferSize = length;
        pBuffer    = data;
    }

    std::string readString() override;

    uint8_t readUint8() override;

    uint16_t readUint16() override;

    uint32_t readUint32() override;

    uint64_t readUint64() override;

    bool readBool() override;

    float readFloat() override;

    long bytesLeft() const override { return bufferSize - currentPos; }

private:
    size_t currentPos{};
    size_t bufferSize{};
    const char* pBuffer{};
};

#endif // IMEMORYSTREAM_H
