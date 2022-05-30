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

#include <misc/IFileStream.h>

#include <misc/exceptions.h>

#include <SDL2/SDL_endian.h>

#include <cmath>

#ifdef _WIN32
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <Windows.h>
#endif

IFileStream::IFileStream() : fp(nullptr) { }

IFileStream::~IFileStream() {
    close();
}

bool IFileStream::open(const std::filesystem::path& filename) {
    close();

    const auto normal = filename.lexically_normal().make_preferred();

#ifdef _WIN32
    fp = _wfsopen(normal.c_str(), L"rbS", _SH_DENYWR);
#else
    fp = fopen(normal.c_str(), "rb");
#endif

    return fp != nullptr;
}

void IFileStream::close() {
    if (fp != nullptr) {
        fclose(fp);
        fp = nullptr;
    }
}

std::string IFileStream::readString() {
    uint32_t length = 0;

    length = readUint32();

    if (length == 0) {
        return "";
    }
    std::string str;

    str.resize(length);

    if (fread(str.data(), length, 1, fp) != 1) {
        if (feof(fp) != 0) {
            THROW(InputStream::eof, "IFileStream::readString(): End-of-File reached!");
        }
        THROW(InputStream::error, "IFileStream::readString(): An I/O-Error occurred!");
    }

    return str;
}

uint8_t IFileStream::readUint8() {
    uint8_t tmp = 0;
    if (fread(&tmp, sizeof(uint8_t), 1, fp) != 1) {
        if (feof(fp) != 0) {
            THROW(InputStream::eof, "IFileStream::readUint8(): End-of-File reached!");
        }
        THROW(InputStream::error, "IFileStream::readUint8(): An I/O-Error occurred!");
    }

    return tmp;
}

uint16_t IFileStream::readUint16() {
    uint16_t tmp = 0;
    if (fread(&tmp, sizeof(uint16_t), 1, fp) != 1) {
        if (feof(fp) != 0) {
            THROW(InputStream::eof, "IFileStream::readUint16(): End-of-File reached!");
        }
        THROW(InputStream::error, "IFileStream::readUint16(): An I/O-Error occurred!");
    }

    return SDL_SwapLE16(tmp);
}

uint32_t IFileStream::readUint32() {
    uint32_t tmp = 0;
    if (fread(&tmp, sizeof(uint32_t), 1, fp) != 1) {
        if (feof(fp) != 0) {
            THROW(InputStream::eof, "IFileStream::readUint32(): End-of-File reached!");
        }
        THROW(InputStream::error, "IFileStream::readUint32(): An I/O-Error occurred!");
    }

    return SDL_SwapLE32(tmp);
}

uint64_t IFileStream::readUint64() {
    uint64_t tmp = 0;
    if (fread(&tmp, sizeof(uint64_t), 1, fp) != 1) {
        if (feof(fp) != 0) {
            THROW(InputStream::eof, "IFileStream::readUint64(): End-of-File reached!");
        }
        THROW(InputStream::error, "IFileStream::readUint64(): An I/O-Error occurred!");
    }
    return SDL_SwapLE64(tmp);
}

bool IFileStream::readBool() {
    return readUint8() == 1;
}

float IFileStream::readFloat() {
    // We could use std::bit_cast from <bit>, but it is unclear
    // if Xcode supports it yet.
    // return std::bit_cast<float>(readUint32());
    const uint32_t tmp = readUint32();
    float tmp2         = NAN;
    memcpy(&tmp2, &tmp, sizeof(uint32_t)); // workaround for a strange optimization in gcc 4.1
    return tmp2;
}
