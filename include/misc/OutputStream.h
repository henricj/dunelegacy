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


#include <SDL.h>
#include <string>
#include <list>
#include <vector>
#include <set>
#include <exception>

class OutputStream
{
public:
	OutputStream() { ; };
	virtual ~OutputStream() { ; };

	/**
        This method flushes all caches and finally writes out all cached output.
	*/
	virtual void flush() = 0;

	// write operations

	virtual void writeString(const std::string& str) = 0;

	virtual void writeUint8(Uint8 x) = 0;
	virtual void writeUint16(Uint16 x) = 0;
	virtual void writeUint32(Uint32 x) = 0;
	virtual void writeUint64(Uint64 x) = 0;
	virtual void writeBool(bool x) = 0;
	virtual void writeFloat(float x) = 0;

    /**
        Writes out a Sint8 value.
        \param x    the value to write out
	*/
	void writeSint8(Sint8 x) {
        Uint8 tmp = *((Uint8*) &x);
        writeUint8(tmp);
	}

    /**
        Writes out a Sint16 value.
        \param x    the value to write out
	*/
	void writeSint16(Sint16 x) {
        Uint16 tmp = *((Uint16*) &x);
        writeUint16(tmp);
	}

	/**
        Writes out a Sint32 value.
        \param x    the value to write out
	*/
	void writeSint32(Sint32 x) {
        Uint32 tmp = *((Uint32*) &x);
        writeUint32(tmp);
	}

	/**
        Writes out a Sint64 value.
        \param x    the value to write out
	*/
	void writeSint64(Sint64 x) {
        Uint64 tmp = *((Uint64*) &x);
        writeUint64(tmp);
	}

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
	void writeBools(bool val1 = false, bool val2 = false, bool val3 = false, bool val4 = false, bool val5 = false, bool val6 = false, bool val7 = false, bool val8 = false) {
        Uint8 val = (val1) | (val2 << 1) | (val3 << 2) | (val4 << 3) | (val5 << 4) | (val6 << 5) | (val7 << 6) | (val8 << 7);
        writeUint8(val);
	}

	/**
		Writes out a complete list of Uint32
		\param	List	the list to write
	*/
	void writeUint32List(const std::list<Uint32>& List) {
		writeUint32((Uint32) List.size());
		std::list<Uint32>::const_iterator iter;
		for(iter=List.begin(); iter != List.end(); ++iter) {
			writeUint32(*iter);
		}
	}

	/**
		Writes out a complete vector of Uint32
		\param	vec	the vector to write
	*/
	void writeUint32Vector(const std::vector<Uint32>& vec) {
		writeUint32((Uint32) vec.size());
		std::vector<Uint32>::const_iterator iter;
		for(iter=vec.begin(); iter != vec.end(); ++iter) {
			writeUint32(*iter);
		}
	}

    /**
		Writes out a complete set of Uint32
		\param	set2write	the set to write
	*/
	void writeUint32Set(const std::set<Uint32>& set2write) {
		writeUint32((Uint32) set2write.size());
		std::set<Uint32>::const_iterator iter;
		for(iter=set2write.begin(); iter != set2write.end(); ++iter) {
			writeUint32(*iter);
		}
	}

	class exception : public std::exception {
	public:
		exception() throw () { ; };
		virtual ~exception() throw () { ; };
	};

	class eof : public OutputStream::exception {
	public:
		eof(const std::string& str) throw () { this->str = str; };
		virtual ~eof() throw () { ; };

		virtual const char* what() const throw () { return str.c_str(); };

	private:
		std::string str;
	};

	class error : public OutputStream::exception {
	public:
		error(const std::string& str) throw () { this->str = str; };
		virtual ~error() throw () { ; };

		virtual const char* what() const throw () { return str.c_str(); };

	private:
		std::string str;
	};
};

#endif // OUTPUTSTREAM_H
