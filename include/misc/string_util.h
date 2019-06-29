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

#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <misc/exceptions.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <sstream>


#if defined(_MSC_VER) && (_MSC_VER >= 1600) /* VS 2010 and above */
#include <sal.h>
#define PRINTF_FORMAT_STRING _Printf_format_string_
#define PRINTF_VARARG_FUNC( fmtargnumber )
#elif defined(__GNUC__)
#define PRINTF_FORMAT_STRING
#define PRINTF_VARARG_FUNC( fmtargnumber ) __attribute__ (( format( __printf__, fmtargnumber, fmtargnumber+1 )))
#endif

/**
    Splits a string into several substrings. These strings are separated with ','.
        Example:<br>
        std::string first, second;<br>
        SplitString("abc,xyz",first, second);<br>
    \param parseString  the string to parse
    \return true if successful, false otherwise (e.g. a mismatch between the number of args and the number of delimited substrings).
*/
template<class ...Args>
bool splitString(const std::string& parseString, Args & ... args) {
    std::vector<std::string*> pStrings = { &args... };

	std::istringstream iss(parseString);
	for(std::string* pString : pStrings) {
		if(!std::getline(iss, *pString, ',')) {
			return false;
		}
	}

	if(!iss.eof()) {
		return false;
	}

    return true;
}

/**
    Splits a string into several substrings.
    \param parseString  the string to parse
    \param delim  regular expression to find the delimiter; default is ','
    \return a vector of the splitted parseString
*/
std::vector<std::string> splitStringToStringVector(const std::string& parseString, const std::string& delim = ",");

std::string replaceAll(const std::string& str, const std::map<std::string, std::string>& replacementMap);


template<typename T>
inline bool parseString(const std::string& str, T& t) {
    std::istringstream is(str);
    is >> t;
    return !is.fail();
}


inline void convertToLower(std::string& str) {
    std::transform(str.begin(),str.end(), str.begin(), (int(*)(int)) tolower);
}

inline std::string strToLower(const std::string& str) {
    std::string result = str;
    convertToLower(result);
    return result;
}

inline void convertToUpper(std::string& str) {
    std::transform(str.begin(),str.end(), str.begin(), (int(*)(int)) toupper);
}

inline std::string strToUpper(const std::string& str) {
    std::string result = str;
    convertToUpper(result);
    return result;
}

inline std::string trim(const std::string& str) {
    size_t firstChar = str.find_first_not_of(" \t");
    size_t lastChar = str.find_last_not_of(" \t");

    if((firstChar == std::string::npos) || (lastChar == std::string::npos)) {
        return "";
    } else {
        return str.substr(firstChar, lastChar - firstChar + 1);
    }
}

inline int stringCaseInsensitiveCompare(const std::string& str1, const std::string& str2) {
    const char* pStr1 = str1.c_str();
    const char* pStr2 = str2.c_str();

    while((*pStr1 != '\0') && (*pStr2 != '\0')) {
        char c1 = tolower(*pStr1);
        char c2 = tolower(*pStr2);

        if(c1 != c2) {
            return c1 - c2;
        }
    }

    return tolower(*pStr1) - tolower(*pStr2);
}

inline bool utf8IsStartByte(unsigned char c) {
    return ( ((c & 0x80) == 0) || ((c & 0xC0) == 0xC0) );
}

/**
    Returns the number of characters (=code points) in the supplied string
    \param  str an utf-8 string
    \return the number of code points
*/
inline size_t utf8Length(const std::string& str) {
    size_t resultLen = 0;

    auto iter = str.cbegin();
    while( iter < str.cend() ) {
        unsigned char c = static_cast<unsigned char>( *iter );

        if( (c & 0x80) == 0) {
            // 1 byte: 0xxxxxxx
            iter += 1;
        } else if( (c & 0xE0) == 0xC0) {
            // 2 bvte: 110xxxxx 10xxxxxx
            iter += 2;
        } else if( (c & 0xF0) == 0xE0) {
            // 3 byte: 1110xxxx 10xxxxxx 10xxxxxx
            iter += 3;
        } else if( (c & 0xF8) == 0xF0) {
            // 4 byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            iter += 4;
        } else {
            // invalid => skip
            iter += 1;
        }

        resultLen += 1;
    }

    return resultLen;
}

/**
    Returns a substring of the specified string with len characters (=code points). If len goes past the end of
    the string a truncated string is returned.
    \param  str an utf-8 string
    \param  pos the position to start with the substring specified in characters (=code points)
    \param  len the length of the substring to copy specified in characters (=code points)
    \return the utf8 substring
*/
std::string utf8Substr(const std::string& str, size_t pos, size_t len = std::string::npos);

/**
    This function splits a text into multiple lines such that each line is no longer than linewidth pixels. The function pGetTextWidth is
    used to determine how width a given text will be in pixels.
    \param  text            the text to split; any hard line breaks '\n' are also considered
    \param  linewidth       the maximum width of a line in pixel
    \param  pGetTextWidth   this function is used to determine the width in pixels of a given string. Its return value shall specify the width in pixels of its parameter.
    \return the returned vector contains the complete text, split into multiple lines.
*/
std::vector<std::string> greedyWordWrap(const std::string& text, int linewidth, std::function<int (const std::string&)> pGetTextWidth);

std::string convertCP850ToUTF8(const std::string& text);

/// This function decodes a string to CP850 Code.
/**
    The parameter text is decoded to CP850 Code and returned
    \param text Text to decode
    \return The decoded text
*/
std::string decodeString(const std::string& text);

#endif // STRING_UTIL_H
