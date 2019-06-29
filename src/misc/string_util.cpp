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
#include <vector>
#include <regex>
#include <algorithm>

std::vector<std::string> splitStringToStringVector(const std::string& parseString, const std::string& delimRegex) {
    std::regex rgx(delimRegex);
    return std::vector<std::string>(std::sregex_token_iterator(parseString.begin(), parseString.end(), rgx, -1), std::sregex_token_iterator());
}

/**
    Replaces multiple strings at one. The mapping is specified by replacementMap
    \param  str the string to apply the replacement to
    \param  replacementMap  a map of replacements
    \return the modified strings
*/
std::string replaceAll(const std::string& str, const std::map<std::string, std::string>& replacementMap) {

    std::string result = str;
    size_t currentPos = 0;

    while(true) {

        size_t bestNextPos = std::string::npos;
        std::string bestNextKey;
        std::string bestNextValue;

        for(const auto& replacement : replacementMap) {

            std::string nextKey = replacement.first;
            size_t nextPos = result.find(nextKey, currentPos);

            if((nextPos != std::string::npos)
                && ( (nextPos < bestNextPos)
                     || ((nextPos == bestNextPos) && (nextKey.length() > bestNextKey.length())) ) ) {

                // best match so far (either smaller position or same position but longer match)
                bestNextPos = nextPos;
                bestNextKey = nextKey;
                bestNextValue = replacement.second;
            }

        }

        if(bestNextPos == std::string::npos) {
            break;
        }

        result.replace(bestNextPos, bestNextKey.length(), bestNextValue);

        currentPos = bestNextPos + bestNextValue.length();
    }


    return result;
}

std::string utf8Substr(const std::string& str, size_t pos, size_t len) {
    std::string result;
    size_t estimatedLength = (len == std::string::npos) ? (str.length() - pos) : len;
    result.reserve(estimatedLength);

    auto iter = str.cbegin();

    size_t currentPos = 0;
    while( (iter < str.cend()) && (currentPos != pos) ) {
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
        currentPos++;
    }

    size_t resultLen = 0;
    while( (iter < str.cend()) && (resultLen != len) ) {
        unsigned char c = static_cast<unsigned char>( *iter );

        size_t numBytes = 0;
        if( (c & 0x80) == 0) {
            // 1 byte: 0xxxxxxx
            numBytes = 1;
        } else if( (c & 0xE0) == 0xC0) {
            // 2 bvte: 110xxxxx 10xxxxxx
            numBytes = 2;
        } else if( (c & 0xF0) == 0xE0) {
            // 3 byte: 1110xxxx 10xxxxxx 10xxxxxx
            numBytes= 3;
        } else if( (c & 0xF8) == 0xF0) {
            // 4 byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            numBytes = 4;
        } else {
            // invalid => skip
            numBytes = 1;
        }

        while((iter < str.cend()) && (numBytes > 0)) {
            result += *iter;
            numBytes--;
            iter++;
        }

        resultLen += 1;
    }

    return result;
}


std::vector<std::string> greedyWordWrap(const std::string& text, int linewidth, std::function<int (const std::string&)> pGetTextWidth) {
    //split text into single lines at every '\n'
    size_t startpos = 0;
    size_t nextpos;
    std::vector<std::string> hardLines;
    do {
        nextpos = text.find("\n",startpos);
        if(nextpos == std::string::npos) {
            hardLines.push_back(text.substr(startpos,text.length()-startpos));
        } else {
            hardLines.push_back(text.substr(startpos,nextpos-startpos));
            startpos = nextpos+1;
        }
    } while(nextpos != std::string::npos);

    std::vector<std::string> textLines;
    for(const std::string& hardLine : hardLines) {
        if(hardLine == "") {
            textLines.emplace_back(" ");
            continue;
        }

        bool bEndOfLine = false;
        size_t warppos = 0;
        size_t oldwarppos = 0;
        size_t lastwarp = 0;

        while(bEndOfLine == false) {
            while(true) {
                warppos = hardLine.find(" ", oldwarppos);
                std::string tmp;
                if(warppos == std::string::npos) {
                    tmp = hardLine.substr(lastwarp,hardLine.length()-lastwarp);
                    warppos = hardLine.length();
                    bEndOfLine = true;
                } else {
                    tmp = hardLine.substr(lastwarp,warppos-lastwarp);
                }

                if( pGetTextWidth(tmp) > linewidth) {
                    // this line would be too big => in oldwarppos is the last correct word warp pos
                    bEndOfLine = false;
                    break;
                } else {
                    if(bEndOfLine == true) {
                        oldwarppos = warppos;
                        break;
                    } else {
                        oldwarppos = warppos + 1;
                    }
                }
            }

            if(oldwarppos == lastwarp) {
                // linewidth is too small for the next word => split the word

                warppos = lastwarp;
                while(true) {
                    std::string tmp = hardLine.substr(lastwarp,warppos-lastwarp);
                    if( pGetTextWidth(tmp) > linewidth) {
                        // this line would be too big => in oldwarppos is the last correct warp pos
                        break;
                    } else {
                        oldwarppos = warppos;
                    }

                    do {
                        warppos++;
                    } while((warppos < hardLine.length()) && !utf8IsStartByte(hardLine[warppos]));

                    if(warppos >= hardLine.length()) {
                        oldwarppos = hardLine.length();
                        break;
                    }
                }

                if(warppos != lastwarp) {
                    textLines.push_back(hardLine.substr(lastwarp,oldwarppos-lastwarp));
                    lastwarp = oldwarppos;
                } else {
                    // linewidth is too small for the next character => create a dummy entry
                    textLines.emplace_back(" ");
                    lastwarp++;
                    oldwarppos++;
                }
            } else {
                textLines.push_back(hardLine.substr(lastwarp,oldwarppos-lastwarp));
                lastwarp = oldwarppos;
            }
        }
    }

    return textLines;
}


std::string convertCP850ToUTF8(const std::string& text)
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
    result.reserve(text.length());
    for(unsigned int i = 0; i < text.size(); i++) {
        unsigned char c = (unsigned char) text[i];
        if(c == 0x0D) {
            result += "\n";
        } else if(c < 128) {
            result += c;
        } else {
            c = cp850toISO8859_1[c-128];
            result += (0xC0 | (c >> 6));
            result += (0x80 | (c & 0x3F));
        }
    }
    return result;
}


std::string decodeString(const std::string& text) {
    std::string out = "";

    static const char decodeTable1[16] = { ' ','e','t','a','i','n','o','s','r','l','h','c','d','u','p','m' };
    static const char decodeTable2[16][9] = {   { 't','a','s','i','o',' ','w','b' },
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
                    THROW(std::invalid_argument, "decodeString(): Special character escape sequence at end of string!");
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
