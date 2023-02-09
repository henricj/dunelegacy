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

#include <filesystem>
#include <string>

class IFileStream final : public InputStream {
public:
    IFileStream();
    ~IFileStream() override;

    bool open(const std::filesystem::path& filename);
    void close();

    std::string readString() override;

    uint8_t readUint8() override;
    uint16_t readUint16() override;
    uint32_t readUint32() override;
    uint64_t readUint64() override;
    bool readBool() override;
    float readFloat() override;

    size_t bytesLeft() const override { return sizeBytes_ - bytePos_; }

private:
    FILE* fp;
    long sizeBytes_;
    long bytePos_;
};

#endif // IFILESTREAM_H
