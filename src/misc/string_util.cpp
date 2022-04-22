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

#include <charconv>
#include <regex>
#include <vector>

std::vector<std::string> splitStringToStringVector(const std::string& parseString, const std::string& delimRegex) {
    const std::regex rgx(delimRegex);
    return {std::sregex_token_iterator(parseString.begin(), parseString.end(), rgx, -1), std::sregex_token_iterator()};
}

/**
    Replaces multiple strings at one. The mapping is specified by replacementMap
    \param  str the string to apply the replacement to
    \param  replacementMap  a map of replacements
    \return the modified strings
*/
std::string replaceAll(const std::string& str, const std::unordered_map<std::string, std::string>& replacementMap) {

    std::string result = str;
    size_t currentPos  = 0;

    while (true) {

        size_t bestNextPos = std::string::npos;
        std::string bestNextKey;
        std::string bestNextValue;

        for (const auto& replacement : replacementMap) {

            std::string nextKey  = replacement.first;
            const size_t nextPos = result.find(nextKey, currentPos);

            if (nextPos != std::string::npos
                && (nextPos < bestNextPos || nextPos == bestNextPos && nextKey.length() > bestNextKey.length())) {

                // best match so far (either smaller position or same position but longer match)
                bestNextPos   = nextPos;
                bestNextKey   = nextKey;
                bestNextValue = replacement.second;
            }
        }

        if (bestNextPos == std::string::npos) {
            break;
        }

        result.replace(bestNextPos, bestNextKey.length(), bestNextValue);

        currentPos = bestNextPos + bestNextValue.length();
    }

    return result;
}

std::string_view trim(std::string_view str) {
    const auto firstChar = str.find_first_not_of(" \t");
    const auto lastChar  = str.find_last_not_of(" \t");

    if (firstChar == std::string::npos || lastChar == std::string::npos)
        return {};

    return str.substr(firstChar, lastChar - firstChar + 1);
}

size_t utf8Length(std::string_view str) {
    size_t resultLen = 0;

    auto iter = str.cbegin();
    while (iter != str.cend()) {
        const auto c = static_cast<unsigned char>(*iter);

        if ((c & 0x80) == 0) {
            // 1 byte: 0xxxxxxx
            iter += 1;
        } else if ((c & 0xE0) == 0xC0) {
            // 2 byte: 110xxxxx 10xxxxxx
            iter += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // 3 byte: 1110xxxx 10xxxxxx 10xxxxxx
            iter += 3;
        } else if ((c & 0xF8) == 0xF0) {
            // 4 byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            iter += 4;
        } else {
            // invalid => skip
            iter += 1;
        }

        resultLen += 1u;
    }

    return resultLen;
}

std::string utf8Substr(std::string_view str, size_t pos, size_t len) {
    std::string result;
    const size_t estimatedLength = len == std::string::npos ? str.length() - pos : len;
    result.reserve(estimatedLength);

    auto iter = str.cbegin();

    size_t currentPos = 0;
    while (iter != str.cend() && currentPos != pos) {
        auto c = static_cast<unsigned char>(*iter);

        if ((c & 0x80) == 0) {
            // 1 byte: 0xxxxxxx
            iter += 1;
        } else if ((c & 0xE0) == 0xC0) {
            // 2 byte: 110xxxxx 10xxxxxx
            iter += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // 3 byte: 1110xxxx 10xxxxxx 10xxxxxx
            iter += 3;
        } else if ((c & 0xF8) == 0xF0) {
            // 4 byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            iter += 4;
        } else {
            // invalid => skip
            iter += 1;
        }
        currentPos++;
    }

    size_t resultLen = 0;
    while (iter != str.cend() && resultLen != len) {
        auto c = static_cast<unsigned char>(*iter);

        size_t numBytes = 0;
        if ((c & 0x80) == 0) {
            // 1 byte: 0xxxxxxx
            numBytes = 1;
        } else if ((c & 0xE0) == 0xC0) {
            // 2 byte: 110xxxxx 10xxxxxx
            numBytes = 2;
        } else if ((c & 0xF0) == 0xE0) {
            // 3 byte: 1110xxxx 10xxxxxx 10xxxxxx
            numBytes = 3;
        } else if ((c & 0xF8) == 0xF0) {
            // 4 byte: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            numBytes = 4;
        } else {
            // invalid => skip
            numBytes = 1;
        }

        while (iter < str.cend() && numBytes > 0) {
            result += *iter;
            numBytes--;
            ++iter;
        }

        resultLen += 1;
    }

    return result;
}

std::vector<std::string>
greedyWordWrap(std::string_view text, float linewidth, std::function<float(std::string_view)> pGetTextWidth) {
    // split text into single lines at every '\n'
    std::vector<std::string_view> hardLines;

    for (auto pos = decltype(text)::size_type{};;) {
        const auto next = text.find('\n', pos);

        if (next == std::string::npos) {
            hardLines.emplace_back(text.substr(pos, text.length() - pos));
            break;
        }

        hardLines.emplace_back(text.substr(pos, next - pos));
        pos = next + 1;
    }

    std::vector<std::string> textLines;
    for (const auto& hardLine : hardLines) {
        if (hardLine.empty()) {
            textLines.emplace_back(" ");
            continue;
        }

        bool bEndOfLine   = false;
        size_t warppos    = 0;
        size_t oldwarppos = 0;
        size_t lastwarp   = 0;

        while (!bEndOfLine) {
            while (true) {
                warppos = hardLine.find(' ', oldwarppos);
                std::string_view tmp;
                if (warppos == std::string::npos) {
                    tmp        = hardLine.substr(lastwarp, hardLine.length() - lastwarp);
                    warppos    = hardLine.length();
                    bEndOfLine = true;
                } else {
                    tmp = hardLine.substr(lastwarp, warppos - lastwarp);
                }

                if (pGetTextWidth(tmp) > linewidth) {
                    // this line would be too big => in oldwarppos is the last correct word warp pos
                    bEndOfLine = false;
                    break;
                }
                if (bEndOfLine) {
                    oldwarppos = warppos;
                    break;
                }
                oldwarppos = warppos + 1;
            }

            if (oldwarppos == lastwarp) {
                // linewidth is too small for the next word => split the word

                warppos = lastwarp;
                while (true) {
                    if (warppos > lastwarp) {
                        const auto tmp = hardLine.substr(lastwarp, warppos - lastwarp);
                        if (pGetTextWidth(tmp) > linewidth) {
                            // this line would be too big => in oldwarppos is the last correct warp pos
                            break;
                        }
                    }
                    oldwarppos = warppos;

                    do {
                        warppos++;
                    } while (warppos < hardLine.length() && !utf8IsStartByte(hardLine[warppos]));

                    if (warppos >= hardLine.length()) {
                        oldwarppos = hardLine.length();
                        break;
                    }
                }

                if (warppos != lastwarp) {
                    if (oldwarppos > lastwarp)
                        textLines.emplace_back(hardLine.substr(lastwarp, oldwarppos - lastwarp));
                    lastwarp = oldwarppos;
                } else {
                    // linewidth is too small for the next character => create a dummy entry
                    textLines.emplace_back(" ");
                    lastwarp++;
                    oldwarppos++;
                }
            } else {
                if (oldwarppos > lastwarp)
                    textLines.emplace_back(hardLine.substr(lastwarp, oldwarppos - lastwarp));
                lastwarp = oldwarppos;
            }
        }
    }

    return textLines;
}

std::string convertCP850ToUTF8(std::string_view text) {
    // contains the upper half of cp850 (128 - 255)
    static constexpr unsigned char cp850toISO8859_1[] = {
        0xc7, 0xfc, 0xe9, 0xe2, 0xe4, 0xe0, 0xe5, 0xe7, 0xea, 0xeb, 0xe8, 0xef, 0xee, 0xec, 0xc4, 0xc5,
        0xc9, 0xe6, 0xc6, 0xf4, 0xf6, 0xf2, 0xfb, 0xf9, 0xff, 0xd6, 0xdc, 0xf8, 0xa3, 0xd8, 0xd7, 0x3f,
        0xe1, 0xed, 0xf3, 0xfa, 0xf1, 0xd1, 0xaa, 0xba, 0xbf, 0xae, 0xac, 0xbd, 0xbc, 0xa1, 0xab, 0xbb,
        0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0xc1, 0xc2, 0xc0, 0xa9, 0x3f, 0x3f, 0x3f, 0x3f, 0xa2, 0xa5, 0x3f,
        0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0xe3, 0xc3, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0xa4,
        0xf0, 0xd0, 0xca, 0xcb, 0xc8, 0x3f, 0xcd, 0xce, 0xcf, 0x3f, 0x3f, 0x3f, 0x3f, 0xa6, 0xcc, 0x3f,
        0xd3, 0xdf, 0xd4, 0xd2, 0xf5, 0xd5, 0xb5, 0xfe, 0xde, 0xda, 0xdb, 0xd9, 0xfd, 0xdd, 0xaf, 0xb4,
        0xad, 0xb1, 0x3f, 0xbe, 0xb6, 0xa7, 0xf7, 0xb8, 0xb0, 0xa8, 0xb7, 0xb9, 0xb3, 0xb2, 0x3f, 0xa0};

    std::string result;
    result.reserve(text.length());
    for (const char i : text) {
        auto c = static_cast<unsigned char>(i);
        if (c == 0x0D) {
            result += "\n";
        } else if (c < 128) {
            result += c;
        } else {
            c = cp850toISO8859_1[c - 128];
            result += 0xC0 | c >> 6;
            result += 0x80 | c & 0x3F;
        }
    }
    return result;
}

std::string decodeString(std::string_view text) {
    static constexpr char decodeTable1[16]    = {' ', 'e', 't', 'a', 'i', 'n', 'o', 's',
                                                 'r', 'l', 'h', 'c', 'd', 'u', 'p', 'm'};
    static constexpr char decodeTable2[16][9] = {
        {'t', 'a', 's', 'i', 'o', ' ', 'w', 'b'}, {' ', 'r', 'n', 's', 'd', 'a', 'l', 'm'},
        {'h', ' ', 'i', 'e', 'o', 'r', 'a', 's'}, {'n', 'r', 't', 'l', 'c', ' ', 's', 'y'},
        {'n', 's', 't', 'c', 'l', 'o', 'e', 'r'}, {' ', 'd', 't', 'g', 'e', 's', 'i', 'o'},
        {'n', 'r', ' ', 'u', 'f', 'm', 's', 'w'}, {' ', 't', 'e', 'p', '.', 'i', 'c', 'a'},
        {'e', ' ', 'o', 'i', 'a', 'd', 'u', 'r'}, {' ', 'l', 'a', 'e', 'i', 'y', 'o', 'd'},
        {'e', 'i', 'a', ' ', 'o', 't', 'r', 'u'}, {'e', 't', 'o', 'a', 'k', 'h', 'l', 'r'},
        {' ', 'e', 'i', 'u', ',', '.', 'o', 'a'}, {'n', 's', 'r', 'c', 't', 'l', 'a', 'i'},
        {'l', 'e', 'o', 'i', 'r', 'a', 't', 'p'}, {'e', 'a', 'o', 'i', 'p', ' ', 'b', 'm'}};

    std::string out;
    out.reserve(text.length());

    for (unsigned int i = 0; i < text.length(); i++) {
        unsigned char databyte = text[i];

        if (databyte & 0x80) {
            const unsigned char index1 = databyte >> 3u & 0xFu;
            const unsigned char index2 = databyte & 0x7u;

            out += decodeTable1[index1];
            out += decodeTable2[index1][index2];
        } else {
            if (databyte == 0x1B) {
                // special character
                // These characters are encoded as CP850 but from the actual CP850 code 0x7F is subtracted

                i++;
                if (i == text.length()) {
                    THROW(std::invalid_argument, "decodeString(): Special character escape sequence at end of string!");
                }

                const unsigned char special = text[i] + 0x7Fu;

                out += special;
            } else if (databyte == '\r') {
                out += '\n';
            } else if (databyte == 0x0Cu) {
                out += '\n';
            } else if (databyte == 0x1Fu) {
                out += '.';
            } else {
                out += databyte;
            }
        }
    }
    return out;
}

std::string to_hex(gsl::span<const uint8_t> data) {
    std::string s;
    s.reserve(data.size() * 2 + 1);

    auto count = -1;
    char buffer[2];
    for (const auto n : data) {
        if (++count > 7) {
            count = 0;
            s.append(1, '-');
        }

        auto [p, ec] = std::to_chars(std::begin(buffer), std::end(buffer), n, 16);
        if (ec != std::errc{}) {
            THROW(std::runtime_error, "Unable to convert to hex");
        }
        const auto length = p - std::begin(buffer);
        if (length > 1)
            s.append(buffer, 2);
        else {
            s.append(1, '0');
            s.append(std::begin(buffer), 1);
        }
    }

    return s;
}

std::string to_hex(gsl::span<const uint32_t> data) {
    std::string s;
    s.reserve(data.size() * 9 + 1);

    auto first = true;
    char buffer[8];
    for (const auto n : data) {
        if (first) {
            first = false;
        } else
            s.append(1, '-');

        auto [p, ec] = std::to_chars(std::begin(buffer), std::end(buffer), n, 16);

        if (ec != std::errc{}) {
            THROW(std::runtime_error, "Unable to convert to hex");
        }

        const auto length = p - std::begin(buffer);
        if (length > 7)
            s.append(buffer, 8);
        else {
            s.append(8 - length, '0');
            s.append(std::begin(buffer), length);
        }
    }

    return s;
}

std::string to_hex(gsl::span<const uint64_t> data) {
    std::string s;
    s.reserve(data.size() * 17 + 1);

    auto first = true;
    char buffer[16];
    for (const auto n : data) {
        if (first) {
            first = false;
        } else
            s.append(1, '-');

        auto [p, ec] = std::to_chars(std::begin(buffer), std::end(buffer), n, 16);

        if (ec != std::errc{}) {
            THROW(std::runtime_error, "Unable to convert to hex");
        }

        const auto length = p - std::begin(buffer);
        if (length > 15)
            s.append(buffer, 16);
        else {
            s.append(16 - length, '0');
            s.append(std::begin(buffer), length);
        }
    }

    return s;
}
