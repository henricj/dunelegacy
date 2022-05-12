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

#ifndef DATABUF_H
#define DATABUF_H

#include "utils.h"

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <memory>
#include <string>
#include <vector>

class ODataSource;

/**
 * Abstract input base class.
 */
class IDataSource {
public:
    IDataSource()                                  = default;
    IDataSource(const IDataSource&)                = delete;
    IDataSource& operator=(const IDataSource&)     = delete;
    IDataSource(IDataSource&&) noexcept            = default;
    IDataSource& operator=(IDataSource&&) noexcept = default;
    virtual ~IDataSource() noexcept                = default;

    virtual uint32_t peek() = 0;

    virtual uint32_t read1()         = 0;
    virtual uint16_t read2()         = 0;
    virtual uint16_t read2high()     = 0;
    virtual uint32_t read4()         = 0;
    virtual uint32_t read4high()     = 0;
    virtual void read(void*, size_t) = 0;
    virtual void read(std::string& s, size_t len) {
        s.resize(len);
        read(s.data(), s.size());
    }
    std::unique_ptr<unsigned char[]> readN(size_t N) {
        auto ptr = std::make_unique<unsigned char[]>(N);
        read(ptr.get(), N);
        return ptr;
    }
    virtual std::unique_ptr<IDataSource> makeSource(size_t) = 0;

    virtual void seek(size_t)         = 0;
    virtual void skip(std::streamoff) = 0;
    virtual size_t getSize() const    = 0;
    virtual size_t getPos() const     = 0;
    size_t getAvail() const {
        size_t const msize = getSize();
        size_t const mpos  = getPos();
        return msize >= mpos ? msize - mpos : 0;
    }
    virtual bool eof() const = 0;
    virtual bool good() const { return true; }
    virtual void clear_error() { }

    virtual void copy_to(ODataSource& dest);

    void readline(std::string& str) {
        str.erase();
        while (!eof()) {
            char character = static_cast<char>(read1());
            if (character == '\r') {
                continue; // Skip cr
            }
            if (character == '\n') {
                break; // break on line feed
            }
            str += character;
        }
    }
};

/**
 * Stream-based input data source which does not own the stream.
 */
class IStreamDataSource : public IDataSource {
protected:
    std::istream* in;

public:
    explicit IStreamDataSource(std::istream* data_stream) : in(data_stream) { }

    uint32_t peek() final { return in->peek(); }

    uint32_t read1() final { return Read1(in); }

    uint16_t read2() final { return Read2(in); }

    uint16_t read2high() final { return Read2high(in); }

    uint32_t read4() final { return Read4(in); }

    uint32_t read4high() final { return Read4high(in); }

    void read(void* b, size_t len) final { in->read(static_cast<char*>(b), len); }

    std::unique_ptr<IDataSource> makeSource(size_t len) final;

    void seek(size_t pos) final { in->seekg(pos); }

    void skip(std::streamoff pos) final { in->seekg(pos, std::ios::cur); }

    size_t getSize() const final { return get_file_size(*in); }

    size_t getPos() const final { return in->tellg(); }

    bool eof() const final {
        in->get();
        bool ret = in->eof();
        if (!ret) {
            in->unget();
        }
        return ret;
    }
    bool good() const final { return in && in->good(); }
    void clear_error() final { in->clear(); }
};

/**
 * Buffer-based input data source which does not own the buffer.
 */
class IBufferDataView : public IDataSource {
protected:
    const unsigned char* buf;
    const unsigned char* buf_ptr;
    std::size_t size;

public:
    IBufferDataView(const void* data, size_t len)
        : buf(static_cast<const unsigned char*>(data)), buf_ptr(buf), size(len) {
        // data can be nullptr if len is also 0
        assert(data != nullptr || len == 0);
    }
    IBufferDataView(const std::unique_ptr<unsigned char[]>& data_, size_t len) : IBufferDataView(data_.get(), len) { }

    // Prevent use after free.
    IBufferDataView(std::unique_ptr<unsigned char[]>&& data_, size_t len) = delete;

    uint32_t peek() final { return *buf_ptr; }

    uint32_t read1() final { return Read1(buf_ptr); }

    uint16_t read2() final { return Read2(buf_ptr); }

    uint16_t read2high() final { return Read2high(buf_ptr); }

    uint32_t read4() final { return Read4(buf_ptr); }

    uint32_t read4high() final { return Read4high(buf_ptr); }

    void read(void* b, size_t len) final {
        std::memcpy(b, buf_ptr, len);
        buf_ptr += len;
    }

    void read(std::string& s, size_t len) final {
        s = std::string(reinterpret_cast<const char*>(buf_ptr), len);
        buf_ptr += len;
    }

    std::unique_ptr<IDataSource> makeSource(size_t len) final;

    void seek(size_t pos) final { buf_ptr = buf + pos; }

    void skip(std::streamoff pos) final { buf_ptr += pos; }

    size_t getSize() const final { return size; }

    size_t getPos() const final { return buf_ptr - buf; }

    const unsigned char* getPtr() { return buf_ptr; }

    bool eof() const final { return buf_ptr >= buf + size; }

    bool good() const final { return (buf != nullptr) && (size != 0U); }

    void copy_to(ODataSource& dest) final;
};

/**
 * Buffer-based input data source which owns the stream.
 */
class IBufferDataSource : public IBufferDataView {
protected:
    std::unique_ptr<unsigned char[]> data;

public:
    IBufferDataSource(void* data_, size_t len)
        : IBufferDataView(data_, len), data(static_cast<unsigned char*>(data_)) { }
    IBufferDataSource(std::unique_ptr<unsigned char[]> data_, size_t len)
        : IBufferDataView(data_, len), data(std::move(data_)) { }
    auto steal_data(size_t& len) {
        len = size;
        return std::move(data);
    }
};

/**
 * Abstract output base class.
 */
class ODataSource {
public:
    ODataSource()                                  = default;
    ODataSource(const ODataSource&)                = delete;
    ODataSource& operator=(const ODataSource&)     = delete;
    ODataSource(ODataSource&&) noexcept            = default;
    ODataSource& operator=(ODataSource&&) noexcept = default;
    virtual ~ODataSource() noexcept                = default;

    virtual void write1(uint32_t)           = 0;
    virtual void write2(uint16_t)           = 0;
    virtual void write2high(uint16_t)       = 0;
    virtual void write4(uint32_t)           = 0;
    virtual void write4high(uint32_t)       = 0;
    virtual void write(const void*, size_t) = 0;
    virtual void write(const std::string& s) { write(s.data(), s.size()); }

    virtual void seek(size_t)         = 0;
    virtual void skip(std::streamoff) = 0;
    virtual size_t getSize() const    = 0;
    virtual size_t getPos() const     = 0;
    virtual void flush() { }
    virtual bool good() const { return true; }
    virtual void clear_error() { }
};

/**
 * Stream-based output data source which does not own the stream.
 */
class OStreamDataSource : public ODataSource {
protected:
    std::ostream* out;

public:
    explicit OStreamDataSource(std::ostream* data_stream) : out(data_stream) { }

    void write1(uint32_t val) final { Write1(out, static_cast<uint16_t>(val)); }

    void write2(uint16_t val) final { Write2(out, val); }

    void write2high(uint16_t val) final { Write2high(out, val); }

    void write4(uint32_t val) final { Write4(out, val); }

    void write4high(uint32_t val) final { Write4high(out, val); }

    void write(const void* b, size_t len) final { out->write(static_cast<const char*>(b), len); }

    void write(const std::string& s) final { out->write(&s[0], s.size()); }

    void seek(size_t pos) final { out->seekp(pos); }

    void skip(std::streamoff pos) final { out->seekp(pos, std::ios::cur); }

    size_t getSize() const final { return out->tellp(); }

    size_t getPos() const final { return out->tellp(); }

    void flush() final { out->flush(); }
    bool good() const final { return out->good(); }
    void clear_error() final { out->clear(); }
};

/**
 * Buffer-based output data source which does not own the buffer.
 */
class OBufferDataSpan : public ODataSource {
protected:
    unsigned char* buf;
    unsigned char* buf_ptr;
    std::size_t size;

public:
    OBufferDataSpan(void* data, size_t len) : buf(static_cast<unsigned char*>(data)), buf_ptr(buf), size(len) {
        // data can be nullptr if len is also 0
        assert(data != nullptr || len == 0);
    }

    OBufferDataSpan(const std::unique_ptr<unsigned char[]>& data_, size_t len) : OBufferDataSpan(data_.get(), len) { }

    // Prevent use after free.
    OBufferDataSpan(std::unique_ptr<unsigned char[]>&& data_, size_t len) = delete;

    void write1(uint32_t val) final { Write1(buf_ptr, val); }

    void write2(uint16_t val) final { Write2(buf_ptr, val); }

    void write2high(uint16_t val) final { Write2high(buf_ptr, val); }

    void write4(uint32_t val) final { Write4(buf_ptr, val); }

    void write4high(uint32_t val) final { Write4high(buf_ptr, val); }

    void write(const void* b, size_t len) final {
        std::memcpy(buf_ptr, b, len);
        buf_ptr += len;
    }

    void write(const std::string& s) final { write(&s[0], s.size()); }

    void seek(size_t pos) final { buf_ptr = buf + pos; }

    void skip(std::streamoff pos) final { buf_ptr += pos; }

    size_t getSize() const final { return size; }

    size_t getPos() const final { return buf_ptr - buf; }

    unsigned char* getPtr() { return buf_ptr; }
};

/**
 * Buffer-based output data source which owns the buffer.
 */
class OBufferDataSource : public OBufferDataSpan {
    std::unique_ptr<unsigned char[]> data;

public:
    explicit OBufferDataSource(size_t len) : OBufferDataSpan(nullptr, 0), data(std::make_unique<unsigned char[]>(len)) {
        assert(len != 0);
        buf_ptr = buf = data.get();
        size          = len;
    }
    OBufferDataSource(std::unique_ptr<unsigned char[]> data_, size_t len)
        : OBufferDataSpan(data_, len), data(std::move(data_)) { }
    OBufferDataSource(void* data_, size_t len)
        : OBufferDataSpan(data_, len), data(static_cast<unsigned char*>(data_)) { }
};

inline void IDataSource::copy_to(ODataSource& dest) {
    size_t len = getSize();
    auto data  = readN(len);
    dest.write(data.get(), len);
}

inline std::unique_ptr<IDataSource> IStreamDataSource::makeSource(size_t len) {
    return std::make_unique<IBufferDataSource>(readN(len), len);
}

inline std::unique_ptr<IDataSource> IBufferDataView::makeSource(size_t len) {
    size_t avail = getAvail();
    if (avail < len) {
        len = avail;
    }
    const unsigned char* ptr = getPtr();
    skip(len);
    return std::make_unique<IBufferDataView>(ptr, len);
}

inline void IBufferDataView::copy_to(ODataSource& dest) {
    size_t len = getAvail();
    dest.write(getPtr(), len);
    skip(len);
}

#endif // DATABUF_H
