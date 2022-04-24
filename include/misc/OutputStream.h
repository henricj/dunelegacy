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

#ifndef OUTPUTSTREAM_H
#define OUTPUTSTREAM_H

#include "DataTypes.h"
#include <fixmath/FixPoint.h>
#include <misc/SDL2pp.h>

#include <exception>
#include <list>
#include <string>
#include <string_view>

class OutputStream {
public:
    OutputStream();
    virtual ~OutputStream();

    /**
        This method flushes all caches and finally writes out all cached output.
    */
    virtual void flush() = 0;

    // write operations

    virtual void writeString(std::string_view str) = 0;

    virtual void writeUint8(uint8_t x)   = 0;
    virtual void writeUint16(uint16_t x) = 0;
    virtual void writeUint32(uint32_t x) = 0;
    virtual void writeUint64(uint64_t x) = 0;
    virtual void writeBool(bool x)       = 0;
    virtual void writeFloat(float x)     = 0;

    /**
        Writes out a Sint8 value.
        \param x    the value to write out
    */
    void writeSint8(int8_t x);

    /**
        Writes out a Sint16 value.
        \param x    the value to write out
    */
    void writeSint16(int16_t x);

    /**
        Writes out a Sint32 value.
        \param x    the value to write out
    */
    void writeSint32(int32_t x);

    /**
        Writes out a Sint64 value.
        \param x    the value to write out
    */
    void writeSint64(int64_t x);

    /**
        Writes out a FixPoint value.
        \param x    the value to write out
    */
    void writeFixPoint(FixPoint x);

    /**
        Writes out up to 8 boolean values into a single byte.
        \param  val1    the 1st boolean value
        \param  val2    the 2nd boolean value
        \param  val3    the 3rd boolean value
        \param  val4    the 4th boolean value
        \param  val5    the 5th boolean value
        \param  val6    the 6th boolean value
        \param  val7    the 7th boolean value
        \param  val8    the 8th boolean value

    */
    void writeBools(bool val1 = false, bool val2 = false, bool val3 = false, bool val4 = false, bool val5 = false,
                    bool val6 = false, bool val7 = false, bool val8 = false);

    /**
        Writes out a complete vector of Uint8
        \param  dataVector the vector to write
    */
    void writeUint8Vector(std::span<const uint8_t> dataVector);

    /**
        Writes out a complete list of Uint32
        \param  dataList    the list to write
    */
    void writeUint32List(const std::list<uint32_t>& dataList);

    /**
        Writes out a complete vector of Uint32
        \param  dataVector the vector to write
    */
    void writeUint32Vector(std::span<const uint32_t> dataVector);

    /**
        Writes out a complete vector of Uint32
        \param  dataVector the vector to write
    */
    template<typename T>
    void writeUint32Vector(std::span<const T> dataVector) {
        writeUint32(static_cast<uint32_t>(dataVector.size()));
        for (const auto data : dataVector) {
            writeUint32(static_cast<uint32_t>(data));
        }
    }

    /**
        Writes out a complete set of Uint32
        \param  dataSet   the set to write
    */
    void writeUint32Set(const Dune::selected_set_type& dataSet);

    class exception : public std::exception {
    public:
        exception() noexcept           = default;
        ~exception() noexcept override = default;
    };

    class eof : public OutputStream::exception {
    public:
        explicit eof(const std::string& str) noexcept : str(str) { }
        ~eof() noexcept override = default;

        [[nodiscard]] const char* what() const noexcept override { return str.c_str(); }

    private:
        std::string str;
    };

    class error : public OutputStream::exception {
    public:
        explicit error(const std::string& str) noexcept : str(str) { }
        ~error() noexcept override = default;

        [[nodiscard]] const char* what() const noexcept override { return str.c_str(); }

    private:
        std::string str;
    };
};

#endif // OUTPUTSTREAM_H
