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

#include <misc/OFileStream.h>

#include <misc/exceptions.h>

#include <SDL2/SDL_endian.h>

#ifdef _WIN32
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <Windows.h>
#endif

OFileStream::OFileStream() = default;

OFileStream::~OFileStream() {
    close();
}

bool OFileStream::open(const std::filesystem::path& filename) {
    close();

    const auto normal = filename.lexically_normal().make_preferred();

#ifdef _WIN32
    fp = _wfsopen(normal.c_str(), L"wbS", _SH_DENYRW);
#else
    fp = fopen(normal.c_str(), "wb");
#endif

    return fp != nullptr;
}

void OFileStream::close() {
    if (fp != nullptr) {
        fclose(fp);
        fp = nullptr;
    }
}

void OFileStream::flush() {
    if (fp != nullptr) {
        fflush(fp);
    }
}

void OFileStream::writeString(std::string_view str) {
    writeUint32(str.length());

    if (!str.empty()) {
        if (fwrite(str.data(), str.length(), 1, fp) != 1) {
            THROW(OutputStream::error, "OFileStream::writeString(): An I/O-Error occurred!");
        }
    }
}

void OFileStream::writeUint8(uint8_t x) {
    if (fwrite(&x, sizeof(uint8_t), 1, fp) != 1) {
        THROW(OutputStream::error, "OFileStream::writeUint8(): An I/O-Error occurred!");
    }
}

void OFileStream::writeUint16(uint16_t x) {
    x = SDL_SwapLE16(x);

    if (fwrite(&x, sizeof(uint16_t), 1, fp) != 1) {
        THROW(OutputStream::error, "OFileStream::writeUint16(): An I/O-Error occurred!");
    }
}

void OFileStream::writeUint32(uint32_t x) {
    x = SDL_SwapLE32(x);

    if (fwrite(&x, sizeof(uint32_t), 1, fp) != 1) {
        THROW(OutputStream::error, "OFileStream::writeUint32(): An I/O-Error occurred!");
    }
}

void OFileStream::writeUint64(uint64_t x) {
    x = SDL_SwapLE64(x);
    if (fwrite(&x, sizeof(uint64_t), 1, fp) != 1) {
        THROW(OutputStream::error, "OFileStream::writeUint64(): An I/O-Error occurred!");
    }
}

void OFileStream::writeBool(bool x) {
    writeUint8(x ? 1 : 0);
}

void OFileStream::writeFloat(float x) {
    uint32_t tmp = 0;
    memcpy(&tmp, &x, sizeof(uint32_t)); // workaround for a strange optimization in gcc 4.1
    writeUint32(tmp);
}
