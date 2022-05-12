/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "FileClasses/xmidi/databuf.h"

#include <cassert>
#include <memory>

IDataSource::~IDataSource() noexcept = default;

std::unique_ptr<unsigned char[]> IDataSource::readN(size_t N) {
    auto ptr = std::make_unique<unsigned char[]>(N);
    read(ptr.get(), N);
    return ptr;
}

void IDataSource::copy_to(ODataSource& dest) {
    const auto len  = getSize();
    const auto data = readN(len);
    dest.write(data.get(), len);
}

void IDataSource::readline(std::string& str) {
    str.erase();
    while (!eof()) {
        const char character = static_cast<char>(read1());
        if (character == '\r') {
            continue; // Skip cr
        }
        if (character == '\n') {
            break; // break on line feed
        }
        str += character;
    }
}

IStreamDataSource::IStreamDataSource(std::istream* data_stream) : in(data_stream) { }

IStreamDataSource::~IStreamDataSource() = default;

std::unique_ptr<IDataSource> IStreamDataSource::makeSource(size_t len) {
    return std::make_unique<IBufferDataSource>(readN(len), len);
}

bool IStreamDataSource::eof() const {
    in->get();
    const bool ret = in->eof();
    if (!ret) {
        in->unget();
    }
    return ret;
}

IBufferDataView::IBufferDataView(const void* data, size_t len)
    : buf(static_cast<const unsigned char*>(data)), buf_ptr(buf), size(len) {
    // data can be nullptr if len is also 0
    assert(data != nullptr || len == 0);
}

IBufferDataView::~IBufferDataView() = default;

void IBufferDataView::read(void* b, size_t len) {
    std::memcpy(b, buf_ptr, len);
    buf_ptr += len;
}

void IBufferDataView::read(std::string& s, size_t len) {
    s = std::string(reinterpret_cast<const char*>(buf_ptr), len);
    buf_ptr += len;
}

std::unique_ptr<IDataSource> IBufferDataView::makeSource(size_t len) {
    const auto avail = getAvail();
    if (avail < len) {
        len = avail;
    }

    const unsigned char* ptr = getPtr();
    skip(len);

    return std::make_unique<IBufferDataView>(ptr, len);
}

void IBufferDataView::copy_to(ODataSource& dest) {
    const size_t len = getAvail();
    dest.write(getPtr(), len);
    skip(len);
}

IBufferDataSource::IBufferDataSource(void* data_, size_t len)
    : IBufferDataView(data_, len), data(static_cast<unsigned char*>(data_)) { }

IBufferDataSource::IBufferDataSource(std::unique_ptr<unsigned char[]> data_, size_t len)
    : IBufferDataView(data_, len), data(std::move(data_)) { }

IBufferDataSource::~IBufferDataSource() = default;

ODataSource::~ODataSource() noexcept = default;

void ODataSource::write(const std::string& s) {
    write(s.data(), s.size());
}

OStreamDataSource::OStreamDataSource(std::ostream* data_stream) : out(data_stream) { }

OStreamDataSource::~OStreamDataSource() = default;

OBufferDataSpan::OBufferDataSpan(void* data, size_t len)
    : buf(static_cast<unsigned char*>(data)), buf_ptr(buf), size(len) {
    // data can be nullptr if len is also 0
    assert(data != nullptr || len == 0);
}

OBufferDataSpan::OBufferDataSpan(const std::unique_ptr<unsigned char[]>& data_, size_t len)
    : OBufferDataSpan(data_.get(), len) { }

OBufferDataSpan::~OBufferDataSpan() = default;

void OBufferDataSpan::write(const void* b, size_t len) {
    std::memcpy(buf_ptr, b, len);
    buf_ptr += len;
}

OBufferDataSource::OBufferDataSource(size_t len)
    : OBufferDataSpan(nullptr, 0), data(std::make_unique<unsigned char[]>(len)) {
    assert(len != 0);

    buf_ptr = buf = data.get();

    size = len;
}

OBufferDataSource::OBufferDataSource(std::unique_ptr<unsigned char[]> data_, size_t len)
    : OBufferDataSpan(data_, len), data(std::move(data_)) { }

OBufferDataSource::OBufferDataSource(void* data_, size_t len)
    : OBufferDataSpan(data_, len), data(static_cast<unsigned char*>(data_)) { }

OBufferDataSource::~OBufferDataSource() = default;
