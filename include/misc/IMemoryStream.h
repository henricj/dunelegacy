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
#include "math.h"


#include <misc/exceptions.h>

#include <cstdlib>
#include <memory.h>
#include <string>

class IMemoryStream : public InputStream
{
public:
    IMemoryStream()
      {
        ;
    }

    IMemoryStream(const char* data, int length)
     :  bufferSize(length), pBuffer(data) {
        ;
    }

    ~IMemoryStream() override = default;

    void open(const char* data, int length) {
        currentPos = 0;
        bufferSize = length;
        pBuffer = data;
    }

    std::string readString() override
    {
        uint32_t length = readUint32();

        if(currentPos + length > bufferSize) {
            THROW(InputStream::eof, "IMemoryStream::readString(): End-of-File reached!");
        }

        std::string resultString(pBuffer + currentPos, length);
        currentPos += length;
        return resultString;
    }

    uint8_t readUint8() override {
        if(currentPos + sizeof(uint8_t) > bufferSize) {
            THROW(InputStream::eof, "IMemoryStream::readUint8(): End-of-File reached!");
        }

        uint8_t tmp = *((const uint8_t*)(pBuffer + currentPos));
        currentPos += sizeof(uint8_t);
        return tmp;
    }

    uint16_t readUint16() override {
        if(currentPos + sizeof(uint16_t) > bufferSize) {
            THROW(InputStream::eof, "IMemoryStream::readUint16(): End-of-File reached!");
        }

        uint16_t tmp = *((const uint16_t*)(pBuffer + currentPos));
        currentPos += sizeof(uint16_t);
        return SDL_SwapLE16(tmp);
    }

    uint32_t readUint32() override {
        if(currentPos + sizeof(uint32_t) > bufferSize) {
            THROW(InputStream::eof, "IMemoryStream::readUint32(): End-of-File reached!");
        }

        uint32_t tmp = *((const uint32_t*)(pBuffer + currentPos));
        currentPos += sizeof(uint32_t);
        return SDL_SwapLE32(tmp);
    }

    uint64_t readUint64() override {
        if(currentPos + sizeof(uint64_t) > bufferSize) {
            THROW(InputStream::eof, "IMemoryStream::readUint64(): End-of-File reached!");
        }

        uint64_t tmp = *((const uint64_t*)(pBuffer + currentPos));
        currentPos += sizeof(uint64_t);
        return SDL_SwapLE64(tmp);
    }

    bool readBool() override
    {
        return (readUint8() == 1 ? true : false);
    }

    float readFloat() override
    {
        uint32_t tmp  = readUint32();
        float    tmp2 = NAN;
        memcpy(&tmp2,&tmp,sizeof(uint32_t)); // workaround for a strange optimization in gcc 4.1
        return tmp2;
    }

private:
    size_t      currentPos{0};
    size_t      bufferSize{0};
    const char* pBuffer{nullptr};
};

#endif // IMEMORYSTREAM_H
