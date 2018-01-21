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
#include <stdlib.h>
#include <string>

class OMemoryStream : public OutputStream
{
public:
    OMemoryStream()
     : currentPos(0), bufferSize(0), pBuffer(nullptr) {
        ;
    }

    ~OMemoryStream() {
        free(pBuffer);
    }

    void open() {
        free(pBuffer);

        currentPos = 0;
        bufferSize = 4;
        pBuffer = (char*) malloc(4);
        if(pBuffer == nullptr) {
            THROW(OMemoryStream::error, "OMemoryStream::open(): malloc failed!");
        }
    }

    const char* getData() const {
        return pBuffer;
    }

    size_t getDataLength() const {
        return (bReadOnly == true) ? bufferSize : currentPos;
    }

    void flush() override
    {
        ;
    }

    void writeString(const std::string& str) override
    {
        ensureBufferSize(currentPos + str.length() + sizeof(Uint32));

        writeUint32(str.length());

        if(!str.empty()) {
            memcpy(pBuffer + currentPos, str.c_str(), str.length());
            currentPos += str.length();
        }
    }


    void writeUint8(Uint8 x) override
    {
        ensureBufferSize(currentPos + sizeof(Uint8));
        *((Uint8*) (pBuffer + currentPos)) = x;
        currentPos += sizeof(Uint8);
    }

    void writeUint16(Uint16 x) override
    {
        ensureBufferSize(currentPos + sizeof(Uint16));
        x = SDL_SwapLE16(x);
        *((Uint16*) (pBuffer + currentPos)) = x;
        currentPos += sizeof(Uint16);
    }

    void writeUint32(Uint32 x) override
    {
        ensureBufferSize(currentPos + sizeof(Uint32));
        x = SDL_SwapLE32(x);
        *((Uint32*) (pBuffer + currentPos)) = x;
        currentPos += sizeof(Uint32);
    }

    void writeUint64(Uint64 x) override
    {
        ensureBufferSize(currentPos + sizeof(Uint64));
        x = SDL_SwapLE64(x);
        *((Uint64*) (pBuffer + currentPos)) = x;
        currentPos += sizeof(Uint64);
    }

    void writeBool(bool x) override
    {
        writeUint8(x == true ? 1 : 0);
    }

    void writeFloat(float x) override
    {
        if(sizeof(float) != sizeof(Uint32)) {
            THROW(OMemoryStream::error, "OMemoryStream::writeFloat(): sizeof(float) != sizeof(Uint32). Cannot save floats on such systems.");
        }
        Uint32 tmp;
        memcpy(&tmp,&x,sizeof(Uint32)); // workaround for a strange optimization in gcc 4.1
        writeUint32(tmp);
    }

    void ensureBufferSize(size_t minBufferSize) {
        if(minBufferSize < bufferSize) {
            return;
        }

        size_t newBufferSize = ((bufferSize * 3) / 2);
        if(newBufferSize < minBufferSize) {
            newBufferSize = minBufferSize;
        }

        char* pNewBuffer = (char*) realloc(pBuffer, newBufferSize);
        if(pNewBuffer == nullptr) {
            THROW(OMemoryStream::error, "OMemoryStream::ensureBufferSize(): realloc failed!");
        } else {
            pBuffer = pNewBuffer;
            bufferSize = newBufferSize;
        }
    }

private:
    size_t  currentPos;
    size_t  bufferSize;
    char*   pBuffer;
};

#endif // OMEMORYSTREAM_H
