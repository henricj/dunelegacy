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


#include "math.h"


#ifdef _WIN32
    #include <windows.h>
#endif

IFileStream::IFileStream()
{
    fp = nullptr;
}

IFileStream::~IFileStream()
{
    close();
}

bool IFileStream::open(const std::filesystem::path& filename)
{
    close();

    const auto normal = std::filesystem::canonical(filename);

#ifdef _WIN32
    fp = _wfsopen(normal.c_str(), L"rbS", _SH_DENYWR);
#else
    fp = fopen(normal.c_str(), "rb");
#endif

    if( fp == nullptr) {
        return false;
    } else {
        return true;
    }
}

void IFileStream::close()
{
    if(fp != nullptr) {
        fclose(fp);
        fp = nullptr;
    }
}

std::string IFileStream::readString()
{
    Uint32 length = 0;

    length = readUint32();

    if(length == 0) {
        return "";
    } else {
        std::string str;

        str.resize(length);

        if(fread(&str[0],length,1,fp) != 1) {
            if(feof(fp) != 0) {
                THROW(InputStream::eof, "IFileStream::readString(): End-of-File reached!");
            } else {
                THROW(InputStream::error, "IFileStream::readString(): An I/O-Error occurred!");
            }
        }

        return str;
    }
}

Uint8 IFileStream::readUint8()
{
    Uint8 tmp = 0;
    if(fread(&tmp,sizeof(Uint8),1,fp) != 1) {
        if(feof(fp) != 0) {
            THROW(InputStream::eof, "IFileStream::readUint8(): End-of-File reached!");
        } else {
            THROW(InputStream::error, "IFileStream::readUint8(): An I/O-Error occurred!");
        }
    }

    return tmp;
}

Uint16 IFileStream::readUint16()
{
    Uint16 tmp = 0;
    if(fread(&tmp,sizeof(Uint16),1,fp) != 1) {
        if(feof(fp) != 0) {
            THROW(InputStream::eof, "IFileStream::readUint16(): End-of-File reached!");
        } else {
            THROW(InputStream::error, "IFileStream::readUint16(): An I/O-Error occurred!");
        }
    }

    return SDL_SwapLE16(tmp);
}

Uint32 IFileStream::readUint32()
{
    Uint32 tmp = 0;
    if(fread(&tmp,sizeof(Uint32),1,fp) != 1) {
        if(feof(fp) != 0) {
            THROW(InputStream::eof, "IFileStream::readUint32(): End-of-File reached!");
        } else {
            THROW(InputStream::error, "IFileStream::readUint32(): An I/O-Error occurred!");
        }
    }

    return SDL_SwapLE32(tmp);
}

Uint64 IFileStream::readUint64()
{
    Uint64 tmp = 0;
    if(fread(&tmp,sizeof(Uint64),1,fp) != 1) {
        if(feof(fp) != 0) {
            THROW(InputStream::eof, "IFileStream::readUint64(): End-of-File reached!");
        } else {
            THROW(InputStream::error, "IFileStream::readUint64(): An I/O-Error occurred!");
        }
    }
    return SDL_SwapLE64(tmp);
}

bool IFileStream::readBool()
{
    return (readUint8() == 1 ? true : false);
}

float IFileStream::readFloat()
{
    Uint32 tmp = readUint32();
    float tmp2 = NAN;
    memcpy(&tmp2,&tmp,sizeof(Uint32)); // workaround for a strange optimization in gcc 4.1
    return tmp2;
}
