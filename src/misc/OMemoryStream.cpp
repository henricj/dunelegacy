#include "misc/OMemoryStream.h"

#include <cstdlib>

OMemoryStream::OMemoryStream() = default;

OMemoryStream::~OMemoryStream() {
    free(pBuffer);
}

void OMemoryStream::open() {
    free(pBuffer);

    currentPos = 0;
    bufferSize = 4;
    pBuffer    = static_cast<char*>(malloc(4));
    if(pBuffer == nullptr) {
        THROW(OMemoryStream::error, "OMemoryStream::open(): malloc failed!");
    }
}

void OMemoryStream::writeUint8(Uint8 x) {
    ensureBufferSize(currentPos + sizeof(Uint8));
    *reinterpret_cast<Uint8*>(pBuffer + currentPos) = x;
    currentPos += sizeof(Uint8);
}

void OMemoryStream::writeFloat(float x) {
    if constexpr(sizeof(float) != sizeof(Uint32)) {
        THROW(OMemoryStream::error,
              "OMemoryStream::writeFloat(): sizeof(float) != sizeof(Uint32). Cannot save floats on such systems.");
    }
    Uint32 tmp;
    memcpy(&tmp, &x, sizeof(Uint32)); // workaround for a strange optimization in gcc 4.1
    writeUint32(tmp);
}

void OMemoryStream::ensureBufferSize(size_t minBufferSize) {
    if(minBufferSize < bufferSize) {
        return;
    }

    size_t newBufferSize = ((bufferSize * 3) / 2);
    if(newBufferSize < minBufferSize) {
        newBufferSize = minBufferSize;
    }

    auto* const pNewBuffer = static_cast<char*>(realloc(pBuffer, newBufferSize));
    if(pNewBuffer == nullptr) {
        THROW(OMemoryStream::error, "OMemoryStream::ensureBufferSize(): realloc failed!");
    }
    pBuffer    = pNewBuffer;
    bufferSize = newBufferSize;
}

void OMemoryStream::flush() { }

void OMemoryStream::writeString(const std::string& str) {
    ensureBufferSize(currentPos + str.length() + sizeof(Uint32));

    writeUint32(str.length());

    if(!str.empty()) {
        memcpy(pBuffer + currentPos, str.c_str(), str.length());
        currentPos += str.length();
    }
}

void OMemoryStream::writeUint16(Uint16 x) {
    ensureBufferSize(currentPos + sizeof(Uint16));

    x = SDL_SwapLE16(x);

    *reinterpret_cast<Uint16*>(pBuffer + currentPos) = x;
    currentPos += sizeof(Uint16);
}

void OMemoryStream::writeUint32(Uint32 x) {
    ensureBufferSize(currentPos + sizeof(Uint32));

    x = SDL_SwapLE32(x);

    *reinterpret_cast<Uint32*>(pBuffer + currentPos) = x;
    currentPos += sizeof(Uint32);
}

void OMemoryStream::writeUint64(Uint64 x) {
    ensureBufferSize(currentPos + sizeof(Uint64));

    x = SDL_SwapLE64(x);

    *reinterpret_cast<Uint64*>(pBuffer + currentPos) = x;
    currentPos += sizeof(Uint64);
}

void OMemoryStream::writeBool(bool x) {
    writeUint8(x == true ? 1 : 0);
}
