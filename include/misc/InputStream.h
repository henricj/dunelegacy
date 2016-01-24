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

#include <libfixmath/FixPoint.h>

#include <SDL.h>
#include <string>
#include <list>
#include <vector>
#include <set>
#include <exception>

class InputStream
{
public:
	InputStream() { ; };
	virtual ~InputStream() { ; };

	/**
		readString reads in a strings from the stream.
        \return the read string
	*/
	virtual std::string readString() = 0;

	virtual Uint8 readUint8() = 0;
	virtual Uint16 readUint16() = 0;
	virtual Uint32 readUint32() = 0;
	virtual Uint64 readUint64() = 0;
	virtual bool readBool() = 0;
	virtual float readFloat() = 0;

	/**
        Reads in a Sint8 value.
        \return the read value
	*/
	Sint8 readSint8() {
	    Uint8 tmp = readUint8();
        return *((Sint8*) &tmp);
	}

	/**
        Reads in a Sint16 value.
        \return the read value
	*/
	Sint16 readSint16() {
	    Uint16 tmp = readUint16();
        return *((Sint16*) &tmp);
	}

	/**
        Reads in a Sint32 value.
        \return the read value
	*/
	Sint32 readSint32() {
	    Uint32 tmp = readUint32();
        return *((Sint32*) &tmp);
	}

	/**
        Reads in a Sint64 value.
        \return the read value
	*/
	Sint64 readSint64() {
	    Uint64 tmp = readUint64();
        return *((Sint64*) &tmp);
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
	void readBools(bool* pVal1 = NULL, bool* pVal2 = NULL, bool* pVal3 = NULL, bool* pVal4 = NULL, bool* pVal5 = NULL, bool* pVal6 = NULL, bool* pVal7 = NULL, bool* pVal8 = NULL) {
        Uint8 val = readUint8();

        if(pVal1 != NULL)   *pVal1 = ((val & 0x01) != 0);
        if(pVal2 != NULL)   *pVal2 = ((val & 0x02) != 0);
        if(pVal3 != NULL)   *pVal3 = ((val & 0x04) != 0);
        if(pVal4 != NULL)   *pVal4 = ((val & 0x08) != 0);
        if(pVal5 != NULL)   *pVal5 = ((val & 0x10) != 0);
        if(pVal6 != NULL)   *pVal6 = ((val & 0x20) != 0);
        if(pVal7 != NULL)   *pVal7 = ((val & 0x40) != 0);
        if(pVal8 != NULL)   *pVal8 = ((val & 0x80) != 0);
	}

	/**
		Reads a list of Uint32 written by writeUint32List().
		\return	the read list
	*/
	std::list<Uint32> readUint32List() {
		std::list<Uint32> List;
		Uint32 size = readUint32();
		for(unsigned int i=0; i < size; i++) {
			List.push_back(readUint32());
		}
		return List;
	}

	/**
		Reads a vector of Uint32 written by writeUint32Vector().
		\return	the read vector
	*/
	std::vector<Uint32> readUint32Vector() {
		std::vector<Uint32> vec;
		Uint32 size = readUint32();
		for(unsigned int i=0; i < size; i++) {
			vec.push_back(readUint32());
		}
		return vec;
	}

    /**
		Reads a set of Uint32 written by writeUint32Set().
		\return	the read set
	*/
	std::set<Uint32> readUint32Set() {
		std::set<Uint32> retSet;
		Uint32 size = readUint32();
		for(unsigned int i=0; i < size; i++) {
			retSet.insert(readUint32());
		}
		return retSet;
	}

	class exception : public std::exception {
	public:
		exception() throw () { ; };
		virtual ~exception() throw () { ; };
	};

	class eof : public InputStream::exception {
	public:
		eof(const std::string& str) throw () { this->str = str; };
		virtual ~eof() throw () { ; };

		virtual const char* what() const throw () { return str.c_str(); };

	private:
		std::string str;
	};

	class error : public InputStream::exception {
	public:
		error(const std::string& str) throw () { this->str = str; };
		virtual ~error() throw () { ; };

		virtual const char* what() const throw () { return str.c_str(); };

	private:
		std::string str;
	};
};

#endif // INPUTSTREAM_H
