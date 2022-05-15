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

#ifndef MEMORYDATASOURCE_H
#define MEMORYDATASOURCE_H

#include "databuf.h"

#include <vector>

// namespace dune {

struct MemoryDataSourceBuffer {
    size_t position_{};
    std::vector<uint8_t> buffer_;
};

//}

inline void Write1(MemoryDataSourceBuffer& buffer, uint8_t value) {
    if (buffer.position_ < buffer.buffer_.size()) {
        buffer.buffer_[buffer.position_++] = value;
        return;
    }

    buffer.buffer_.push_back(value);
    buffer.position_ = buffer.buffer_.size();
}

// namespace dune {

class OMemoryDataSource final : public ODataSource {
public:
    void write1(uint32_t val) override { Write1(buffer_, static_cast<uint8_t>(val)); }

    void write2(uint16_t val) override { Write2(buffer_, val); }

    void write2high(uint16_t val) override { Write2high(buffer_, val); }

    void write4(uint32_t val) override { Write4(buffer_, val); }

    void write4high(uint32_t val) override { Write4high(buffer_, val); }

    void write(const void* b, size_t len) override;

    void seek(size_t pos) override;

    void skip(std::streamoff pos) override;

    [[nodiscard]] size_t getSize() const override { return buffer_.buffer_.size(); }

    [[nodiscard]] size_t getPos() const override { return buffer_.position_; }

    std::vector<uint8_t> takeBuffer() { return std::move(buffer_.buffer_); }

private:
    MemoryDataSourceBuffer buffer_;
};

//}

#endif // MEMORYDATASOURCE_H
