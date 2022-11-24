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

#include <FileClasses/POFile.h>

#include <misc/BufferedReader.h>
#include <misc/string_util.h>

#include <string>
#include <string_view>

namespace {
const std::unordered_map<std::string, std::string> unescape_replacement_map = {
    {"\\0", "\0"}, {"\\n", "\n"}, {"\\r", "\r"},  {"\\t", "\t"},  {"\\a", "\a"},
    {"\\b", "\b"}, {"\\?", "\?"}, {"\\\\", "\\"}, {"\\\"", "\""}, {"\\\'", "\'"}};

std::string unescapeString(const std::string& str) {
    return replaceAll(str, unescape_replacement_map);
}

std::string extractString(const std::string& str, const std::string& filename, int lineNum) {
    const size_t firstQuote = str.find_first_of('\"');
    const size_t lastQuote  = str.find_last_of('\"');

    if (firstQuote == std::string::npos || lastQuote == std::string::npos) {
        sdl2::log_info("%s:%d: Missing opening or closing quotes!", filename.c_str(), lineNum);
        return "";
    }

    return unescapeString(str.substr(firstQuote + 1, lastQuote - firstQuote - 1));
}

} // namespace

dune::string_unordered_map<std::string> loadPOFile(SDL_RWops* rwop, const std::string& filename) {
    using namespace std::literals;

    dune::string_unordered_map<std::string> mapping;

    if (rwop == nullptr) {
        sdl2::log_info("%s: Cannot find this file!", filename.c_str());
        return mapping;
    }

    std::string msgid;
    std::string msgstr;

    bool msgidMode  = false;
    bool msgstrMode = false;

    int lineNum    = 0;
    bool bFinished = false;

    const auto pending = std::make_unique<SimpleBufferedReader<128 * 1024>>(rwop);

    std::string completeLine;
    completeLine.reserve(128);

    while (!bFinished) {
        lineNum++;

        completeLine.clear();

        while (true) {
            const auto tmp = pending->getch();
            if (!tmp.has_value()) {
                bFinished = true;
                break;
            }

            if (tmp == '\n')
                break;
            if (tmp != '\r')
                completeLine += tmp.value();
        }

        const size_t lineStart = completeLine.find_first_not_of(" \t");
        if (lineStart == std::string::npos || completeLine[lineStart] == '#') {
            // blank line or comment line
            continue;
        }

        const auto line = std::string_view{completeLine}.substr(lineStart);

        static constexpr auto msgid_token  = "msgid"sv;
        static constexpr auto msgstr_token = "msgstr"sv;

        if (line.substr(0, msgid_token.size()) == msgid_token) {
            if (msgidMode) {
                sdl2::log_info("%s:%d: Opening a new msgid without finishing the previous one!", filename.c_str(),
                               lineNum);
            } else if (msgstrMode) {
                // we have finished the previous translation
                mapping[msgid] = std::move(msgstr);
                msgstr.clear();

                msgstrMode = false;
            }

            msgid = extractString(std::string{line.substr(msgid_token.size())}, filename, lineNum);

            msgidMode = true;
        } else {
            if (completeLine.substr(lineStart, msgstr_token.size()) == msgstr_token) {
                msgidMode = false;

                msgstr = extractString(std::string{line.substr(msgstr_token.size())}, filename, lineNum);

                msgstrMode = true;
            } else {
                if (msgidMode) {
                    msgid += extractString(completeLine, filename, lineNum);
                } else if (msgstrMode) {
                    msgstr += extractString(completeLine, filename, lineNum);
                }
            }
        }
    }

    if (msgstrMode) {
        // we have a last translation to finish
        mapping[msgid] = std::move(msgstr);
    }

    return mapping;
}
