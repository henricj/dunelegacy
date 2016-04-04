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

#include <misc/string_util.h>

#include <stdarg.h>
#include <stdio.h>
#include <algorithm>

/**
	Splits a string into several substrings. This strings are separated with ','.
        Example:<br>
        String first, second;<br>
        SplitString("abc,xyz",2,&first, &second);<br>
	\param parseString  the string to parse
	\param numStringPointers    the number of pointers to strings following after this parameter
	\return true if successful, false otherwise.
*/
bool splitString(const std::string& parseString, unsigned int numStringPointers, ...) {
	va_list arg_ptr;
	va_start(arg_ptr, numStringPointers);

	std::string** pStr;

	if(numStringPointers == 0)
		return false;

    pStr = new std::string*[numStringPointers];

	for(unsigned int i = 0; i < numStringPointers; i++) {
		pStr[i] = va_arg(arg_ptr, std::string* );
	}
	va_end(arg_ptr);

	int startpos = 0;
	unsigned int index = 0;

	for(unsigned int i = 0; i < parseString.size(); i++) {
		if(parseString[i] == ',') {
			*(pStr[index]) = parseString.substr(startpos,i-startpos);
			startpos = i + 1;
			index++;
			if(index >= numStringPointers) {
				delete [] pStr;
				return false;
			}
		}
	}

	*(pStr[index]) = parseString.substr(startpos,parseString.size()-startpos);
	delete [] pStr;
	return true;
}

/*
	Splits a string into several substrings. This strings are separated with ',' by default.
*/
std::vector<std::string> splitString(const std::string& parseString, const std::string& delim, bool keepDelim) {
	std::vector<std::string> retVector;
	if(delim.empty()) {
        retVector.push_back(parseString);
        return retVector;
	}

	std::string::const_iterator iter = parseString.begin();

	while(true) {
        std::string::const_iterator foundPos = std::search(iter, parseString.end(), delim.begin(), delim.end());
        std::string part(iter, (foundPos == parseString.end()) ? foundPos : foundPos + (keepDelim ? delim.length() : 0));
        retVector.push_back(part);

        if(foundPos == parseString.end()) {
            break;
        }

        iter = foundPos + delim.length();
	}

	return retVector;
}

/**
	Replaces multiple strings at one. The mapping is specified by replacementMap
	\param	str	the string to apply the replacement to
	\param	replacementMap	a map of replacements
	\return	the modified strings
*/
std::string replaceAll(std::string str, const std::map<std::string, std::string>& replacementMap) {

	size_t currentPos = 0;

	while(true) {

		size_t bestNextPos = std::string::npos;
		std::string bestNextKey;
		std::string bestNextValue;

		std::map<std::string, std::string>::const_iterator iter;
		for(iter = replacementMap.begin(); iter != replacementMap.end(); ++iter) {

			std::string nextKey = iter->first;
			size_t nextPos = str.find(nextKey, currentPos);

			if((nextPos != std::string::npos)
				&& ( (nextPos < bestNextPos)
					 || ((nextPos == bestNextPos) && (nextKey.length() > bestNextKey.length())) ) ) {

				// best match so far (either smaller position or same position but longer match)
				bestNextPos = nextPos;
				bestNextKey = nextKey;
				bestNextValue = iter->second;
			}

		}

		if(bestNextPos == std::string::npos) {
			break;
		}

		str.replace(bestNextPos, bestNextKey.length(), bestNextValue);

		currentPos = bestNextPos + bestNextValue.length();
	}


	return str;
}

std::string strprintf(const std::string fmt, ...) {
    // Note that fmt is not passed by reference as this is not allowed for the last parameter before ...
	va_list arg_ptr;
	va_start(arg_ptr, fmt);

	int length = vsnprintf(nullptr, 0, fmt.c_str(), arg_ptr);
	if(length < 0) {
        throw std::runtime_error("strprintf(): vsnprintf() failed!");
	}

	char* tmpBuffer = new char[length+1];

	va_end(arg_ptr);

    va_start(arg_ptr, fmt);
	if(vsnprintf(tmpBuffer, length+1, fmt.c_str(), arg_ptr) < 0) {
	    delete [] tmpBuffer;
        throw std::runtime_error("strprintf(): vsnprintf() failed!");
	}

	std::string formatedString(tmpBuffer);

	delete [] tmpBuffer;

	va_end(arg_ptr);

	return formatedString;
}

std::string convertCP850ToISO8859_1(const std::string& text)
{
    // contains the upper half of cp850 (128 - 255)
    static const unsigned char cp850toISO8859_1[] = {
        0xc7, 0xfc, 0xe9, 0xe2, 0xe4, 0xe0, 0xe5, 0xe7, 0xea, 0xeb, 0xe8, 0xef, 0xee, 0xec, 0xc4, 0xc5,
        0xc9, 0xe6, 0xc6, 0xf4, 0xf6, 0xf2, 0xfb, 0xf9, 0xff, 0xd6, 0xdc, 0xf8, 0xa3, 0xd8, 0xd7, 0x3f,
        0xe1, 0xed, 0xf3, 0xfa, 0xf1, 0xd1, 0xaa, 0xba, 0xbf, 0xae, 0xac, 0xbd, 0xbc, 0xa1, 0xab, 0xbb,
        0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0xc1, 0xc2, 0xc0, 0xa9, 0x3f, 0x3f, 0x3f, 0x3f, 0xa2, 0xa5, 0x3f,
        0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0xe3, 0xc3, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0xa4,
        0xf0, 0xd0, 0xca, 0xcb, 0xc8, 0x3f, 0xcd, 0xce, 0xcf, 0x3f, 0x3f, 0x3f, 0x3f, 0xa6, 0xcc, 0x3f,
        0xd3, 0xdf, 0xd4, 0xd2, 0xf5, 0xd5, 0xb5, 0xfe, 0xde, 0xda, 0xdb, 0xd9, 0xfd, 0xdd, 0xaf, 0xb4,
        0xad, 0xb1, 0x3f, 0xbe, 0xb6, 0xa7, 0xf7, 0xb8, 0xb0, 0xa8, 0xb7, 0xb9, 0xb3, 0xb2, 0x3f, 0xa0
    };

    std::string result;
    for(unsigned int i = 0; i < text.size(); i++) {
        unsigned char c = (unsigned char) text[i];
        if(c == 0x0D) {
            result += "\n";
        } else if(c < 128) {
            result += c;
        } else {
            result += cp850toISO8859_1[c-128];
        }
    }
    return result;
}

std::string convertUTF8ToISO8859_1(const std::string& text)
{
    unsigned char savedChar = 0;

    std::string result;
    for(unsigned int i = 0; i < text.size(); i++) {
        unsigned char c = (unsigned char) text[i];

        if(savedChar != 0) {
            unsigned char newChar = ((savedChar & 0x1F) << 6) | (c & 0x3F);
            result += newChar;
            savedChar = 0;
        } else {

            if(c < 128) {
                // 1 byte
                result += c;
            } else if( (c & 0xE0) == 0xC0) {
                // 2 byte
                savedChar = c;
            } else if( (c & 0xF0) == 0xE0) {
                // 3 byte
                result += 0x01; // cannot handle this
                i += 2;
            } else if( (c & 0xF8) == 0xF0) {
                // 4 byte
                result += 0x01; // cannot handle this
                i += 3;
            } else if( (c & 0xFC) == 0xF8) {
                // 5 byte
                result += 0x01; // cannot handle this
                i += 4;
            } else if( (c & 0xFE) == 0xFC) {
                // 6 byte
                result += 0x01; // cannot handle this
                i += 5;
            }
        }
    }
    return result;
}

std::string decodeString(std::string text) {
	std::string out = "";

	static const char decodeTable1[16] = { ' ','e','t','a','i','n','o','s','r','l','h','c','d','u','p','m' };
	static const char decodeTable2[16][9] = {	{ 't','a','s','i','o',' ','w','b' },
												{ ' ','r','n','s','d','a','l','m' },
												{ 'h',' ','i','e','o','r','a','s' },
												{ 'n','r','t','l','c',' ','s','y' },
												{ 'n','s','t','c','l','o','e','r' },
												{ ' ','d','t','g','e','s','i','o' },
												{ 'n','r',' ','u','f','m','s','w' },
												{ ' ','t','e','p','.','i','c','a' },
												{ 'e',' ','o','i','a','d','u','r' },
												{ ' ','l','a','e','i','y','o','d' },
												{ 'e','i','a',' ','o','t','r','u' },
												{ 'e','t','o','a','k','h','l','r' },
												{ ' ','e','i','u',',','.','o','a' },
												{ 'n','s','r','c','t','l','a','i' },
												{ 'l','e','o','i','r','a','t','p' },
												{ 'e','a','o','i','p',' ','b','m' } };

	for(unsigned int i = 0; i < text.length(); i++) {
		unsigned char databyte = text[i];

		if(databyte & 0x80) {
			unsigned char index1 = (databyte >> 3) & 0xF;
			unsigned char index2 = databyte & 0x7;

			out += decodeTable1[index1];
			out += decodeTable2[index1][index2];
		} else {
			if(databyte == 0x1B) {
				// special character
				// These characters are encoded as CP850 but from the actual CP850 code 0x7F is subtracted

				i++;
				if(i == text.length()) {
				    throw std::invalid_argument("decodeString(): Special character escape sequence at end of string!");
				}

				unsigned char special = text[i] + 0x7F;

				out += special;
			} else if(databyte == '\r') {
				out += '\n';
			} else if(databyte == 0x0C) {
				out += '\n';
			} else if(databyte == 0x1F) {
				out += '.';
			} else {
				out += databyte;
			}
		}
	}
	return out;
}
