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

#ifndef IFILESTREAM_H
#define IFILESTREAM_H

#include "InputStream.h"
#include <stdlib.h>
#include <string>

class IFileStream : public InputStream
{
public:
    IFileStream();
    ~IFileStream();

    bool open(const char* filename);
    bool open(std::string filename);
    void close();

    std::string readString();

    Uint8 readUint8();
    Uint16 readUint16();
    Uint32 readUint32();
    Uint64 readUint64();
    bool readBool();
    float readFloat();

private:
    FILE* fp;
};

#endif // IFILESTREAM_H
