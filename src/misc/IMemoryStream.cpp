#include "misc/IMemoryStream.h"

#include <cmath>
#include <cstdlib>
#include <memory.h>

IMemoryStream::IMemoryStream()  = default;
IMemoryStream::~IMemoryStream() = default;

std::string IMemoryStream::readString() {
    const uint32_t length = readUint32();

    if (currentPos + length > bufferSize) {
        THROW(InputStream::eof, "IMemoryStream::readString(): End-of-File reached!");
    }

    std::string resultString(pBuffer + currentPos, length);
    currentPos += length;
    return resultString;
}

uint8_t IMemoryStream::readUint8() {
    if (currentPos + sizeof(uint8_t) > bufferSize) {
        THROW(InputStream::eof, "IMemoryStream::readUint8(): End-of-File reached!");
    }

    const uint8_t tmp = *reinterpret_cast<const uint8_t*>(pBuffer + currentPos);
    currentPos += sizeof(uint8_t);
    return tmp;
}

uint16_t IMemoryStream::readUint16() {
    if (currentPos + sizeof(uint16_t) > bufferSize) {
        THROW(InputStream::eof, "IMemoryStream::readUint16(): End-of-File reached!");
    }

    const uint16_t tmp = *reinterpret_cast<const uint16_t*>(pBuffer + currentPos);
    currentPos += sizeof(uint16_t);
    return SDL_SwapLE16(tmp);
}

uint32_t IMemoryStream::readUint32() {
    if (currentPos + sizeof(uint32_t) > bufferSize) {
        THROW(InputStream::eof, "IMemoryStream::readUint32(): End-of-File reached!");
    }

    const uint32_t tmp = *reinterpret_cast<const uint32_t*>(pBuffer + currentPos);
    currentPos += sizeof(uint32_t);
    return SDL_SwapLE32(tmp);
}

uint64_t IMemoryStream::readUint64() {
    if (currentPos + sizeof(uint64_t) > bufferSize) {
        THROW(InputStream::eof, "IMemoryStream::readUint64(): End-of-File reached!");
    }

    const uint64_t tmp = *reinterpret_cast<const uint64_t*>(pBuffer + currentPos);
    currentPos += sizeof(uint64_t);
    return SDL_SwapLE64(tmp);
}

bool IMemoryStream::readBool() {
    return readUint8() == 1 ? true : false;
}

float IMemoryStream::readFloat() {
    const uint32_t tmp = readUint32();
    float tmp2         = NAN;
    memcpy(&tmp2, &tmp, sizeof(uint32_t)); // workaround for a strange optimization in gcc 4.1
    return tmp2;
}
