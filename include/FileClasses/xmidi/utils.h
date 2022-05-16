/*
 *  utils.h - Common utility routines.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2022  The Exult Team
 *
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

#ifndef UTILS_H
#define UTILS_H

#include <cstring>
#include <functional>
#include <iosfwd>
#include <iostream>
#include <limits>
#include <memory>
#include <string>

#ifndef ATTR_PRINTF
#    ifdef __GNUC__
#        define ATTR_PRINTF(x, y) __attribute__((format(printf, (x), (y))))
#    else
#        define ATTR_PRINTF(x, y)
#    endif
#endif

/*
 *  Read a 1-byte value.
 */

inline uint8_t Read1(std::istream& in) {
    return static_cast<uint8_t>(in.get());
}

inline uint8_t Read1(std::istream* in) {
    return static_cast<uint8_t>(in->get());
}

inline uint8_t Read1(const uint8_t*& in) {
    return *in++;
}

/*
 *  Read a 2-byte value, lsb first.
 */

template<typename Source>
inline uint16_t Read2(Source& in) {
    uint16_t b0 = Read1(in);
    uint16_t b1 = Read1(in);
    return (b1 << 8) | b0;
}

/*
 *  Read a 2-byte value, hsb first.
 */

template<typename Source>
inline uint16_t Read2high(Source& in) {
    uint16_t b0 = Read1(in);
    uint16_t b1 = Read1(in);
    return (b0 << 8) | b1;
}

/*
 *  Read a 4-byte long value, lsb first.
 */

template<typename Source>
inline uint32_t Read4(Source& in) {
    uint32_t b0 = Read1(in);
    uint32_t b1 = Read1(in);
    uint32_t b2 = Read1(in);
    uint32_t b3 = Read1(in);
    return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}

/*
 *  Read a 4-byte long value, hsb first.
 */
template<typename Source>
inline uint32_t Read4high(Source& in) {
    uint32_t b0 = Read1(in);
    uint32_t b1 = Read1(in);
    uint32_t b2 = Read1(in);
    uint32_t b3 = Read1(in);
    return (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
}

/*
 *  Read a 2-byte value, lsb first, unsigned.
 */
template<typename Source>
inline int16_t Read2s(Source& in) {
    return static_cast<int16_t>(Read2(in));
}

/*
 *  Read a 4-byte value, lsb first, unsigned.
 */
template<typename Source>
inline int32_t Read4s(Source& in) {
    return static_cast<int32_t>(Read4(in));
}

/*
 *  Read a N-byte long value, lsb first.
 */

template<typename T, typename Source>
inline T ReadN(Source& in) {
    T val = 0;
    for (size_t i = 0; i < sizeof(T); i++)
        val |= static_cast<T>(T(Read1(in)) << (8 * i));
    return val;
}

/*
 *  Read a N-byte long value, hsb first.
 */

template<typename T, typename Source>
inline T ReadNhigh(Source& in) {
    T val = 0;
    for (int i = sizeof(T) - 1; i >= 0; i--)
        val |= static_cast<T>(T(Read1(in)) << (8 * i));
    return val;
}

inline int ReadInt(std::istream& in, int def = 0) {
    int num;
    if (in.eof())
        return def;
    in >> num;
    if (in.fail())
        return def;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '/');
    return num;
}

inline unsigned int ReadUInt(std::istream& in, int def = 0) {
    unsigned int num;
    if (in.eof())
        return def;
    in >> num;
    if (in.fail())
        return def;
    in.ignore(std::numeric_limits<std::streamsize>::max(), '/');
    return num;
}

inline void WriteInt(std::ostream& out, int num, bool final = false) {
    out << num;
    if (final)
        out << std::endl;
    else
        out << '/';
}

inline void WriteInt(std::ostream& out, unsigned int num, bool final = false) {
    out << num;
    if (final)
        out << std::endl;
    else
        out << '/';
}

inline std::string ReadStr(char*& eptr, int off = 1) {
    eptr += off;
    char* pos = std::strchr(eptr, '/');
    std::string retval(eptr, pos - eptr);
    eptr = pos;
    return retval;
}

inline std::string ReadStr(std::istream& in) {
    std::string retval;
    std::getline(in, retval, '/');
    return retval;
}

inline void WriteStr(std::ostream& out, const std::string& str, bool final = false) {
    out << str;
    if (final)
        out << std::endl;
    else
        out << '/';
}

/*
 *  Read a 1-byte value from mutable data.
 */

inline uint8_t MRead1(uint8_t*& in) {
    return *in++;
}

/*
 *  Read a 2-byte value, lsb first, from mutable data.
 */

inline uint16_t MRead2(uint8_t*& in) {
    uint16_t b0 = MRead1(in);
    uint16_t b1 = MRead1(in);
    return (b1 << 8) | b0;
}

/*
 *  Write a 1-byte value.
 */

inline void Write1(std::ostream& out, uint8_t val) {
    out.put(static_cast<char>(val));
}

inline void Write1(std::ostream* out, uint8_t val) {
    out->put(static_cast<char>(val));
}

inline void Write1(uint8_t*& out, uint8_t val) {
    *out++ = val;
}

/*
 *  Write a 2-byte value, lsb first.
 */

template<typename Dest>
inline void Write2(Dest& out, uint16_t val) {
    Write1(out, static_cast<uint8_t>(val));
    Write1(out, static_cast<uint8_t>(val >> 8));
}

/*
 *  Write a 2-byte value, msb first.
 */

template<typename Dest>
inline void Write2high(Dest& out, uint16_t val) {
    Write1(out, static_cast<uint8_t>(val >> 8));
    Write1(out, static_cast<uint8_t>(val));
}

/*
 *  Write a 4-byte value, lsb first.
 */

template<typename Dest>
inline void Write4(Dest& out, uint32_t val) {
    Write1(out, static_cast<uint8_t>(val));
    Write1(out, static_cast<uint8_t>(val >> 8));
    Write1(out, static_cast<uint8_t>(val >> 16));
    Write1(out, static_cast<uint8_t>(val >> 24));
}

/*
 *  Write a 4-byte value, msb first.
 */

template<typename Dest>
inline void Write4high(Dest& out, uint32_t val) {
    Write1(out, static_cast<uint8_t>(val >> 24));
    Write1(out, static_cast<uint8_t>(val >> 16));
    Write1(out, static_cast<uint8_t>(val >> 8));
    Write1(out, static_cast<uint8_t>(val));
}

/*
 *  Write a signed 4-byte value to a stream, lsb first.
 */

template<typename Dest>
inline void Write4s(Dest& out, int32_t val) {
    Write4(out, static_cast<uint32_t>(val));
}

/*
 *  Write a N-byte value, lsb first.
 */

template<typename T, typename Dest>
inline void WriteN(Dest& out, T val) {
    for (size_t i = 0; i < sizeof(T); i++)
        Write1(out, static_cast<uint8_t>(val >> (8 * i)));
}

/*
 *  Write a N-byte value, msb first.
 */

template<typename T, typename Dest>
inline void WriteNhigh(Dest& out, T val) {
    for (int i = sizeof(T) - 1; i >= 0; i--)
        Write1(out, static_cast<uint8_t>(val >> (8 * i)));
}

/*
 *  Get file size without undefined behavior.
 */
inline size_t get_file_size(std::istream& in) {
    const auto start = in.tellg();
    in.seekg(0);
    in.ignore(std::numeric_limits<std::streamsize>::max());
    size_t len = in.gcount();
    in.seekg(start);
    return len;
}

#endif /* _UTILS_H_ */
