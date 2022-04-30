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

#include <algorithm>
#include <charconv>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <gsl/gsl>

#if defined(_MSC_VER) && (_MSC_VER >= 1600) /* VS 2010 and above */
#    include <sal.h>
#    define PRINTF_FORMAT_STRING _Printf_format_string_
#    define PRINTF_VARARG_FUNC(fmtargnumber)
#elif defined(__GNUC__)
#    define PRINTF_FORMAT_STRING
#    define PRINTF_VARARG_FUNC(fmtargnumber) __attribute__((format(__printf__, fmtargnumber, fmtargnumber + 1)))
#endif

/**
    Splits a string_view into several substrings. These strings are separated with ','.
        Example:<br>
        std::string first, second;<br>
        SplitString("abc,xyz",first, second);<br>
    \param parseString  the string to parse
    \return true if successful, false otherwise (e.g. a mismatch between the number of args and the number of delimited
   substrings).
*/
template<class... Args>
bool splitString(std::string_view parseString, std::string& arg0, Args&... args) {
    std::array<std::string*, sizeof...(Args) + 1> pStrings = {&arg0, &args...};

    auto** p                 = &pStrings[0];
    auto** const strings_end = p + pStrings.size();

    for (auto previous = parseString.data(), current = previous, end = previous + parseString.size();
         current != end && previous != end; previous = current + 1) {
        current = std::find(previous, end, ',');

        if (previous != current) {
            **p = std::string{previous, static_cast<std::string::size_type>(current - previous)};
            if (++p == strings_end)
                return true;
        }
    }

    return false;
}

/**
    Splits a string_view into several substrings. These strings are separated with ','.
        Example:<br>
        std::string_view* first, second;<br>
        SplitString("abc,xyz", &first, &second);<br>
    \param parseString  the string to parse
    \return true if successful, false otherwise (e.g. a mismatch between the number of args and the number of delimited
   substrings).
*/
template<class... Args>
bool splitString(std::string_view parseString, std::string_view* arg0, Args*... args) {
    std::array<std::string_view*, sizeof...(Args) + 1> pStrings = {arg0, args...};

    if (pStrings.empty())
        return true;

    auto** p                 = &pStrings[0];
    auto** const strings_end = p + pStrings.size();

    for (auto previous = parseString.data(), current = previous, end = previous + parseString.size();
         current != end && previous != end; previous = current + 1) {
        current = std::find(previous, end, ',');

        if (previous != current) {
            **p = std::string_view{previous, static_cast<std::string::size_type>(current - previous)};
            if (++p == strings_end)
                return true;
        }
    }

    return false;
}

/**
    Splits a string into several substrings.
    \param parseString  the string to parse
    \param delim  regular expression to find the delimiter; default is ','
    \return a vector of the splitted parseString
*/
std::vector<std::string> splitStringToStringVector(const std::string& parseString, const std::string& delim = ",");

std::string replaceAll(const std::string& str, const std::unordered_map<std::string, std::string>& replacementMap);

/**
    Remove leading and trailing spaces and tabs.
    \param str  the string to trim
    \return the trimmed string
 */
std::string_view trim(std::string_view str);

/**
    Parse a string into a value of the given type.
    \param text     The string to parse
    \param value    The output value (unmodified on failure)
    \return true if the string was parsed and the value argument set
 */
template<typename TValue>
bool parseString(std::string_view text, TValue& value) {
    text = trim(text);

    auto last = text.data() + text.size();

    auto [ptr, ec] = std::from_chars(text.data(), last, value);

    return ec == std::errc{} && ptr == last;
}

inline void convertToLower(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), tolower);
}

inline std::string strToLower(std::string_view str) {
    std::string result{str};
    convertToLower(result);
    return result;
}

inline void convertToUpper(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), toupper);
}

inline std::string strToUpper(std::string_view str) {
    std::string result{str};
    convertToUpper(result);
    return result;
}

inline bool utf8IsStartByte(unsigned char c) {
    return (c & 0x80u) == 0u || (c & 0xC0u) == 0xC0u;
}

/**
    Returns the number of characters (=code points) in the supplied string
    \param  str an utf-8 string
    \return the number of code points
*/
size_t utf8Length(std::string_view str);

/**
    Returns a substring of the specified string with len characters (=code points). If len goes past the end of
    the string a truncated string is returned.
    \param  str an utf-8 string
    \param  pos the position to start with the substring specified in characters (=code points)
    \param  len the length of the substring to copy specified in characters (=code points)
    \return the utf8 substring
*/
std::string utf8Substr(std::string_view str, size_t pos, size_t len = std::string_view::npos);

/**
    This function splits a text into multiple lines such that each line is no longer than linewidth pixels. The function
   pGetTextWidth is used to determine how width a given text will be in pixels. \param  text            the text to
   split; any hard line breaks '\n' are also considered \param  linewidth       the maximum width of a line in pixel
    \param  pGetTextWidth   this function is used to determine the width in pixels of a given string. Its return value
   shall specify the width in pixels of its parameter. \return the returned vector contains the complete text, split
   into multiple lines.
*/
std::vector<std::string>
greedyWordWrap(std::string_view text, int linewidth, std::function<int(std::string_view)> pGetTextWidth);

std::string convertCP850ToUTF8(std::string_view text);

/// This function decodes a string to CP850 Code.
/**
    The parameter text is decoded to CP850 Code and returned
    \param text Text to decode
    \return The decoded text
*/
std::string decodeString(std::string_view text);

std::string to_hex(gsl::span<const uint8_t> data);
std::string to_hex(gsl::span<const uint32_t> data);
std::string to_hex(gsl::span<const uint64_t> data);

#endif // STRING_UTIL_H
