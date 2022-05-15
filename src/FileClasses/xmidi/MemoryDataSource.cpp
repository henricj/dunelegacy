/*
Copyright (C) 2000, 2001  The Exult Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "FileClasses/xmidi/MemoryDataSource.h"

#include <algorithm>

void OMemoryDataSource::write(const void* b, size_t len) {
    const auto tail = buffer_.position_ + len;

    if (tail > buffer_.buffer_.size())
        buffer_.buffer_.resize(tail);

    std::copy_n(static_cast<const uint8_t*>(b), len, buffer_.buffer_.data() + buffer_.position_);
    buffer_.position_ += len;
}

void OMemoryDataSource::seek(size_t pos) {
    if (pos > buffer_.buffer_.size())
        buffer_.buffer_.resize(pos);

    buffer_.position_ = pos;
}

void OMemoryDataSource::skip(std::streamoff pos) {
    seek(pos + buffer_.position_);
}
