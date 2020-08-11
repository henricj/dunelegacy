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

#ifndef INPUTSTREAM_H
#define INPUTSTREAM_H

#include <fixmath/FixPoint.h>

#include <string>
#include <list>
#include <utility>
#include <vector>
#include <exception>

class InputStream
{
public:
    InputStream() = default;
    virtual ~InputStream() = default;

    /**
        readString reads in a strings from the stream.
        \return the read string
    */
    virtual std::string readString() = 0;

    virtual uint8_t readUint8() = 0;
    virtual uint16_t readUint16() = 0;
    virtual uint32_t readUint32() = 0;
    virtual uint64_t readUint64() = 0;
    virtual bool readBool() = 0;
    virtual float readFloat() = 0;

    /**
        Reads in a int8_t value.
        \return the read value
    */
    int8_t readSint8() {
        uint8_t tmp = readUint8();
        return *((int8_t*) &tmp);
    }

    /**
        Reads in a int16_t value.
        \return the read value
    */
    int16_t readSint16() {
        uint16_t tmp = readUint16();
        return *((int16_t*) &tmp);
    }

    /**
        Reads in a int32_t value.
        \return the read value
    */
    int32_t readSint32() {
        uint32_t tmp = readUint32();
        return *((int32_t*) &tmp);
    }

    /**
        Reads in a int64_t value.
        \return the read value
    */
    int64_t readSint64() {
        uint64_t tmp = readUint64();
        return *((int64_t*) &tmp);
    }

    /**
        Reads in a FixPoint value.
        \return the read value
    */
    FixPoint readFixPoint() {
        return FixPoint::FromRawValue(readSint64());
    }

    /**
        Reads in up to 8 boolean values from a single byte
        \param  pVal1   the 1st boolean value
        \param  pVal2   the 2nd boolean value
        \param  pVal3   the 3rd boolean value
        \param  pVal4   the 4th boolean value
        \param  pVal5   the 5th boolean value
        \param  pVal6   the 6th boolean value
        \param  pVal7   the 7th boolean value
        \param  pVal8   the 8th boolean value

    */
    void readBools(bool* pVal1 = nullptr, bool* pVal2 = nullptr, bool* pVal3 = nullptr, bool* pVal4 = nullptr, bool* pVal5 = nullptr, bool* pVal6 = nullptr, bool* pVal7 = nullptr, bool* pVal8 = nullptr) {
        uint8_t val = readUint8();

        if(pVal1 != nullptr)   *pVal1 = ((val & 0x01) != 0);
        if(pVal2 != nullptr)   *pVal2 = ((val & 0x02) != 0);
        if(pVal3 != nullptr)   *pVal3 = ((val & 0x04) != 0);
        if(pVal4 != nullptr)   *pVal4 = ((val & 0x08) != 0);
        if(pVal5 != nullptr)   *pVal5 = ((val & 0x10) != 0);
        if(pVal6 != nullptr)   *pVal6 = ((val & 0x20) != 0);
        if(pVal7 != nullptr)   *pVal7 = ((val & 0x40) != 0);
        if(pVal8 != nullptr)   *pVal8 = ((val & 0x80) != 0);
    }

    /**
        Reads a vector of uint8_t written by writeUint8Vector().
        \return the read vector
    */
    std::vector<uint8_t> readUint8Vector() {
        std::vector<uint8_t> vec;
        readUint8Vector(vec);

        return vec;
    }

    /**
        Reads a vector of uint8_t written by writeUint8Vector().
        \param vec operate in place (reduce heap thrashing)
    */
    void readUint8Vector(std::vector<uint8_t>& vec) {
        vec.clear();
        const auto size = readUint32();
        vec.reserve(size);
        for(unsigned int i = 0; i < size; i++) {
            vec.push_back(readUint8());
        }
    }

    /**
        Reads a list of uint32_t written by writeUint32List().
        \return the read list
    */
    std::list<uint32_t> readUint32List() {
        std::list<uint32_t> List;
        const auto size = readUint32();
        for(unsigned int i=0; i < size; i++) {
            List.push_back(readUint32());
        }
        return List;
    }

    /**
        Reads a vector of uint32_t written by writeUint32Vector().
        \param vec operate in place (reduce heap thrashing)
    */
    void readUint32Vector(std::vector<uint32_t>& vec)
    {
        vec.clear();
        const auto size = readUint32();
        vec.reserve(size);
        for(unsigned int i=0; i < size; i++) {
            vec.push_back(readUint32());
        }
    }

    /**
        Reads a vector of uint32_t written by writeUint32Vector().
        \param vec operate in place (reduce heap thrashing)
    */
    template<typename T>
    void readUint32Vector(std::vector<T>& vec) {
        vec.clear();
        const auto size = readUint32();
        vec.reserve(size);
        for(unsigned int i = 0; i < size; i++) {
            vec.push_back(static_cast<T>(readUint32()));
        }
    }

    /**
        Reads a vector of uint32_t written by writeUint32Vector().
        \return the read vector
    */
    std::vector<uint32_t> readUint32Vector() {
        std::vector<uint32_t> vec;
        readUint32Vector(vec);

        return vec;
    }

    /**
        Reads a vector of uint32_t written by writeUint32Vector().
        \return the read vector
    */
    template<typename T>
    std::vector<T> readUint32Vector() {
        std::vector<T> vec;
        readUint32Vector<T>(vec);

        return vec;
    }

    /**
        Reads a set of uint32_t written by writeUint32Set().
        \return the read set
    */
    template<typename SetType>
    SetType readUint32Set() {
        SetType retSet;
        const auto size = readUint32();
        retSet.reserve(size);
        for(auto i = decltype(size){0}; i < size; i++) {
            retSet.insert(readUint32());
        }
        return retSet;
    }

    class exception : public std::exception {
    public:
        exception() noexcept = default;
        exception(const exception &) = default;
        exception(exception &&) = default;
        virtual ~exception() noexcept = default;

        exception& operator=(const exception &) = default;
        exception& operator=(exception &&) = default;
    };

    class eof : public InputStream::exception {
    public:
        explicit eof(std::string str) noexcept : str(std::move(str)) { }
        eof(const eof &) = default;
        eof(eof &&) = default;
        virtual ~eof() noexcept = default;

        eof& operator=(const eof &) = default;
        eof& operator=(eof &&) = default;

        [[nodiscard]] const char* what() const noexcept override { return str.c_str(); }

    private:
        std::string str;
    };

    class error : public InputStream::exception {
    public:
        explicit error(std::string str) noexcept : str(std::move(str)) { };
        error(const error &) = default;
        error(error &&) = default;
        virtual ~error() noexcept = default;

        error& operator=(const error &) = default;
        error& operator=(error &&) = default;

        [[nodiscard]] const char* what() const noexcept override { return str.c_str(); };

    private:
        std::string str;
    };
};

#endif // INPUTSTREAM_H
