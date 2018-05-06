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
    #include <windows.h>
#endif

OFileStream::OFileStream()
{
    fp = nullptr;
}

OFileStream::~OFileStream()
{
    close();
}

bool OFileStream::open(const char* filename)
{
    if(fp != nullptr) {
        fclose(fp);
    }

    const char* pFilename = filename;

    #ifdef _WIN32

    // on win32 we need an ansi-encoded filepath
    WCHAR szwPath[MAX_PATH];
    char szPath[MAX_PATH];

    if(MultiByteToWideChar(CP_UTF8, 0, filename, -1, szwPath, MAX_PATH) == 0) {
        return false;
    }

    if(WideCharToMultiByte(CP_ACP, 0, szwPath, -1, szPath, MAX_PATH, nullptr, nullptr) == 0) {
        return false;
    }

    pFilename = szPath;

    #endif

    if( (fp = fopen(pFilename,"wb")) == nullptr) {
        return false;
    } else {
        return true;
    }
}

bool OFileStream::open(const std::string& filename)
{
    return open(filename.c_str());
}

void OFileStream::close()
{
    if(fp != nullptr) {
        fclose(fp);
        fp = nullptr;
    }
}

void OFileStream::flush() {
    if(fp != nullptr) {
        fflush(fp);
    }
}

void OFileStream::writeString(const std::string& str)
{
    writeUint32(str.length());

    if(!str.empty()) {
        if(fwrite(str.c_str(),str.length(),1,fp) != 1) {
            THROW(OutputStream::error, "OFileStream::writeString(): An I/O-Error occurred!");
        }
    }
}

void OFileStream::writeUint8(Uint8 x)
{
    if(fwrite(&x,sizeof(Uint8),1,fp) != 1) {
        THROW(OutputStream::error, "OFileStream::writeUint8(): An I/O-Error occurred!");
    }
}

void OFileStream::writeUint16(Uint16 x)
{
    x = SDL_SwapLE16(x);

    if(fwrite(&x,sizeof(Uint16),1,fp) != 1) {
        THROW(OutputStream::error, "OFileStream::writeUint16(): An I/O-Error occurred!");
    }
}

void OFileStream::writeUint32(Uint32 x)
{
    x = SDL_SwapLE32(x);

    if(fwrite(&x,sizeof(Uint32),1,fp) != 1) {
        THROW(OutputStream::error, "OFileStream::writeUint32(): An I/O-Error occurred!");
    }
}

void OFileStream::writeUint64(Uint64 x)
{
    x = SDL_SwapLE64(x);
    if(fwrite(&x,sizeof(Uint64),1,fp) != 1) {
        THROW(OutputStream::error, "OFileStream::writeUint64(): An I/O-Error occurred!");
    }
}

void OFileStream::writeBool(bool x)
{
    writeUint8(x == true ? 1 : 0);
}

void OFileStream::writeFloat(float x)
{
    Uint32 tmp;
    memcpy(&tmp,&x,sizeof(Uint32)); // workaround for a strange optimization in gcc 4.1
    writeUint32(tmp);
}
