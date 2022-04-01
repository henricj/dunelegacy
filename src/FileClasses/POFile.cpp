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

#include <cstdio>

static std::string unescapeString(const std::string& str) {
    std::map<std::string, std::string> replacementMap;
    replacementMap["\\0"]  = "\0";
    replacementMap["\\n"]  = "\n";
    replacementMap["\\r"]  = "\r";
    replacementMap["\\t"]  = "\t";
    replacementMap["\\a"]  = "\a";
    replacementMap["\\b"]  = "\b";
    replacementMap["\\?"]  = "\?";
    replacementMap["\\\\"] = "\\";
    replacementMap["\\\""] = "\"";
    replacementMap["\\\'"] = "\'";

    return replaceAll(str, replacementMap);
}

static std::string extractString(const std::string& str, const std::string& filename, int lineNum) {
    const size_t firstQuote = str.find_first_of('\"');
    const size_t lastQuote  = str.find_last_of('\"');

    if (firstQuote == std::string::npos || lastQuote == std::string::npos) {
        sdl2::log_info("%s:%d: Missing opening or closing quotes!", filename.c_str(), lineNum);
        return "";
    }

    return unescapeString(str.substr(firstQuote + 1, lastQuote - firstQuote - 1));
}

std::map<std::string, std::string> loadPOFile(SDL_RWops* rwop, const std::string& filename) {

    std::map<std::string, std::string> mapping;

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

    SimpleBufferedReader<> pending{rwop};

    while (!bFinished) {
        lineNum++;

        std::string completeLine;
        unsigned char tmp = 0;

        while (true) {
            const auto tmp = pending.getch();
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

        if (completeLine.substr(lineStart, 5) == "msgid") {
            if (msgidMode) {
                sdl2::log_info("%s:%d: Opening a new msgid without finishing the previous one!", filename.c_str(),
                               lineNum);
            } else if (msgstrMode) {
                // we have finished the previous translation
                mapping[msgid] = msgstr;
                msgstr         = "";

                msgstrMode = false;
            }

            msgid = extractString(completeLine.substr(lineStart + 5), filename, lineNum);

            msgidMode = true;
        } else if (completeLine.substr(lineStart, 6) == "msgstr") {
            msgidMode = false;

            msgstr = extractString(completeLine.substr(lineStart + 6), filename, lineNum);

            msgstrMode = true;
        } else {
            if (msgidMode) {
                msgid += extractString(completeLine, filename, lineNum);
            } else if (msgstrMode) {
                msgstr += extractString(completeLine, filename, lineNum);
            }
        }
    }

    if (msgstrMode) {
        // we have a last translation to finish
        mapping[msgid] = msgstr;
    }

    return mapping;
}
