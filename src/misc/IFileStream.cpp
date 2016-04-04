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

#include <string.h>
#include <stdio.h>
#include <SDL_endian.h>

#ifdef _WIN32
    #include <windows.h>
#endif

IFileStream::IFileStream()
{
	fp = NULL;
}

IFileStream::~IFileStream()
{
	close();
}

bool IFileStream::open(const char* filename)
{
	if(fp != NULL) {
		fclose(fp);
	}

	const char* pFilename = filename;

    #if defined (_WIN32)

    // on win32 we need an ansi-encoded filepath
    WCHAR szwPath[MAX_PATH];
    char szPath[MAX_PATH];

    if(MultiByteToWideChar(CP_UTF8, 0, filename, -1, szwPath, MAX_PATH) == 0) {
        return false;
    }

    if(WideCharToMultiByte(CP_ACP, 0, szwPath, -1, szPath, MAX_PATH, NULL, NULL) == 0) {
        return false;
    }

    pFilename = szPath;

    #endif

	if( (fp = fopen(pFilename,"rb")) == NULL) {
		return false;
	} else {
		return true;
	}
}

bool IFileStream::open(std::string filename)
{
	return open(filename.c_str());
}

void IFileStream::close()
{
	if(fp != NULL) {
		fclose(fp);
		fp = NULL;
	}
}

std::string IFileStream::readString()
{
	Uint32 length;

	length = readUint32();

    if(length == 0) {
        return "";
    } else {
        std::string str;

        str.resize(length);

        if(fread(&str[0],length,1,fp) != 1) {
            if(feof(fp) != 0) {
                throw InputStream::eof("IFileStream::readString(): End-of-File reached!");
            } else {
                throw InputStream::error("IFileStream::readString(): An I/O-Error occurred!");
            }
        }

        return str;
    }
}

Uint8 IFileStream::readUint8()
{
	Uint8 tmp;
	if(fread(&tmp,sizeof(Uint8),1,fp) != 1) {
		if(feof(fp) != 0) {
			throw InputStream::eof("IFileStream::readUint8(): End-of-File reached!");
		} else {
			throw InputStream::error("IFileStream::readUint8(): An I/O-Error occurred!");
		}
	}

	return tmp;
}

Uint16 IFileStream::readUint16()
{
	Uint16 tmp;
	if(fread(&tmp,sizeof(Uint16),1,fp) != 1) {
		if(feof(fp) != 0) {
			throw InputStream::eof("IFileStream::readUint16(): End-of-File reached!");
		} else {
			throw InputStream::error("IFileStream::readUint16(): An I/O-Error occurred!");
		}
	}

	return SDL_SwapLE16(tmp);
}

Uint32 IFileStream::readUint32()
{
	Uint32 tmp;
	if(fread(&tmp,sizeof(Uint32),1,fp) != 1) {
		if(feof(fp) != 0) {
			throw InputStream::eof("IFileStream::readUint32(): End-of-File reached!");
		} else {
			throw InputStream::error("IFileStream::readUint32(): An I/O-Error occurred!");
		}
	}

	return SDL_SwapLE32(tmp);
}

Uint64 IFileStream::readUint64()
{
	Uint64 tmp;
	if(fread(&tmp,sizeof(Uint64),1,fp) != 1) {
		if(feof(fp) != 0) {
			throw InputStream::eof("IFileStream::readUint64(): End-of-File reached!");
		} else {
			throw InputStream::error("IFileStream::readUint64(): An I/O-Error occurred!");
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
	float tmp2;
	memcpy(&tmp2,&tmp,sizeof(Uint32)); // workaround for a strange optimization in gcc 4.1
	return tmp2;
}
