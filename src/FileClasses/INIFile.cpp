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

#include <FileClasses/INIFile.h>

#include <misc/BufferedReader.h>
#include <misc/SDL2pp.h>
#include <misc/exceptions.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <utility>

#include <cctype>

using namespace std::literals;

INIFile::INIFileLine::INIFileLine(std::string completeLine) : completeLine(std::move(completeLine)) { }

INIFile::INIFileLine::~INIFileLine() = default;

INIFile::Key::Key(std::string completeLine, substring key, substring value)
    : INIFileLine(std::move(completeLine)), key_(key), value_{value} { }

INIFile::Key::Key(initializer&& init) : Key(std::move(init.completeLine), init.key, init.value) { }

INIFile::Key::initializer
INIFile::Key::Key::createLine(std::string_view keyname, std::string value, bool bEscapeIfNeeded, bool bWhitespace) {
    const std::string escaped_value{bEscapeIfNeeded ? escapeValue(std::string{value}) : value};

    const auto escape_offset = !escaped_value.empty() && escaped_value[0] == '"' ? 1U : 0U;

    const auto equal = bWhitespace ? " = "sv : "="sv;

    const auto value_offset = keyname.size() + equal.size() + escape_offset;

    auto line = fmt::format("{}{}{}", keyname, equal, escaped_value);

    return {std::move(line), {0, keyname.size()}, {value_offset, value.size()}};
}

INIFile::Key::Key(std::string_view keyname, std::string_view value, bool bEscapeIfNeeded, bool bWhitespace)
    : Key{createLine(keyname, std::string{value}, bEscapeIfNeeded, bWhitespace)} { }

INIFile::Key::~Key() = default;

std::string_view INIFile::Key::getKeyName() const {
    return key_.apply(completeLine);
}

std::string_view INIFile::Key::getStringView() const {
    return value_.apply(completeLine);
}

namespace {
struct CaseInsensitiveEqualTo {
    bool operator()(const std::string_view& lhs, const std::string_view& rhs) const {
        if (&lhs == &rhs)
            return true;

        if (lhs.length() != rhs.length())
            return false;

        return std::ranges::equal(lhs, rhs, [](auto a, auto b) {
            // Force values to be "unsigned char" or std::tolower()'s behavior is undefined.
            return a == b || std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
        });
    }
};

struct CaseInsensitiveHash {
    size_t operator()(const std::string_view& v) const {
        auto sum = v.size();

        // Force "c" to be "unsigned char" or std::tolower()'s behavior is undefined.
        for (unsigned char c : v)
            sum = sum * 101 + std::tolower(c);

        return sum;
    }
};

using bool_lookup_type = std::unordered_map<std::string_view, bool, CaseInsensitiveHash, CaseInsensitiveEqualTo>;

const bool_lookup_type bool_lookup = {{"true", true},   {"enabled", true},   {"on", true},   {"1", true},
                                      {"false", false}, {"disabled", false}, {"off", false}, {"0", false}};
} // namespace

bool INIFile::Key::getBoolValue(bool defaultValue) const {
    if (value_.empty())
        return defaultValue;

    const auto it = bool_lookup.find(value_.apply(completeLine));

    if (it == bool_lookup.end())
        return defaultValue;

    return it->second;
}

void INIFile::Key::setStringValue(std::string_view newValue, bool bEscapeIfNeeded) {
    const auto need_escape = bEscapeIfNeeded ? escapingValueNeeded(newValue) : false;
    const auto is_escaped  = value_.offset() >= completeLine.size() ? false : '"' == completeLine[value_.offset() - 1u];

    auto valueStringBegin = value_.offset();

    if (need_escape == is_escaped) {
        completeLine.replace(valueStringBegin, value_.size(), newValue.data(), newValue.size());
    } else if (!need_escape && is_escaped) {
        --valueStringBegin;
        completeLine.replace(valueStringBegin, value_.size() + 2, newValue.data(), newValue.size());
    } else {
        // We need to add quotes...
        completeLine.replace(valueStringBegin, value_.size(), "\"\"");
        ++valueStringBegin;
        completeLine.insert(valueStringBegin, newValue.data(), newValue.size());
    }

    value_ = {valueStringBegin, newValue.size()};
}

void INIFile::Key::setBoolValue(bool newValue) {
    setStringValue(newValue ? "true" : "false");
}

bool INIFile::Key::escapingValueNeeded(const std::string_view value) {
    // test for non normal char
    if (value.empty())
        return true;

    return std::ranges::any_of(value, [](char c) { return !isNormalChar(c); });
}

std::string INIFile::Key::escapeValue(std::string value) {
    if (escapingValueNeeded(value))
        return std::format("\"{}\"", value);

    return value;
}

INIFile::Section::Section(std::string completeLine, substring section, bool bWhitespace)
    : INIFileLine(std::move(completeLine)), section_{section}, bWhitespace(bWhitespace) { }

INIFile::Section::Section(std::string_view sectionname, bool bWhitespace)
    : INIFileLine{std::format("[{}]", sectionname)}, section_{1, sectionname.size()}, bWhitespace(bWhitespace) { }

INIFile::Section::~Section() = default;

/// Get the name for this section
/**
    This method returns the name of this section
    \return name of this section
*/
std::string_view INIFile::Section::getSectionName() const {
    return section_.apply(completeLine);
}

// public methods

/// Constructor for an empty INI-File.
/**
    This constructor creates an empty INI-File.
    \param  bWhitespace   Insert whitespace between key an value when creating a new entry
    \param  firstLineComment    A comment to put in the first line (no comment is added for an empty string)
*/
INIFile::INIFile(bool bWhitespace, std::string_view firstLineComment) : bWhitespace(bWhitespace) {
    if (!firstLineComment.empty()) {
        lines_.emplace_back(fmt::format("; {}", firstLineComment));
        lines_.emplace_back(""s);
    }
}

/// Constructor for reading the INI-File from a file.
/**
    This constructor reads the INI-File from the file specified by filename. The file opened in readonly-mode. After
    reading the file it is closed immediately. If the file does not exist, it is treated as empty.
    \param  filename        The file to be opened.
    \param  bWhitespace   Insert whitespace between key an value when creating a new entry
*/
INIFile::INIFile(const std::filesystem::path& filename, bool bWhitespace) : bWhitespace(bWhitespace) {
    // open file
    const sdl2::RWops_ptr file{SDL_RWFromFile(filename.u8string().c_str(), "r")};

    if (file) {
        readfile(file.get());
    }
}

/// Constructor for reading the INI-File from a SDL_RWops.
/**
    This constructor reads the INI-File from RWopsFile. The RWopsFile can be readonly.
    \param  RWopsFile   Pointer to RWopsFile (can be readonly)
*/
INIFile::INIFile(SDL_RWops* RWopsFile, bool bWhitespace) : bWhitespace(bWhitespace) {
    if (RWopsFile == nullptr) {
        THROW(std::invalid_argument, "RWopsFile == nullptr!");
    }

    readfile(RWopsFile);
}

/// Destructor.
/**
    This is the destructor. Changes to the INI-Files are not automatically saved. Call INIFile::SaveChangesTo() for
   that purpose.
*/
INIFile::~INIFile() = default;

size_t INIFile::getLineNumber(std::string_view sectionname) const {
    const auto section = findSectionInternal(sectionname);

    if (section == lines_.end())
        return INVALID_LINE;

    return 1u + std::distance(lines_.begin(), section);
}

size_t INIFile::getLineNumber(std::string_view sectionname, std::string_view keyname) const {
    const auto key = getKeyInternal(sectionname, keyname);

    if (key == lines_.end())
        return INVALID_LINE;

    return 1u + std::distance(lines_.begin(), key);
}

/**
    This method checks whether the specified section exists.
    \param  section         sectionname
    \return true, if the section exists, false otherwise
*/
bool INIFile::hasSection(std::string_view section) const {
    return findSectionInternal(section) != lines_.end();
}

/**
    This method returns a reference to the section specified by sectionname
    \param  sectionname the name of the section
    \return the section if found, nullptr otherwise
*/
const INIFile::Section& INIFile::getSection(std::string_view sectionname) const {
    const auto curSection = findSectionInternal(sectionname);

    if (curSection == lines_.end()) {
        THROW(std::out_of_range, "There is no section '%s' in this INI file", sectionname);
    }

    assert(std::holds_alternative<Section>(*curSection));

    return std::get<Section>(*curSection);
}

/**
    Removes the whole the specified section
    \param  sectionname the section to remove
    \return true on success

*/
bool INIFile::removeSection(std::string_view sectionname) {
    auto section = getSectionInternal(sectionname);

    if (section.empty())
        return false;

    lines_.erase(section.begin(), section.end());

    return true;
}

/**
    Removes all keys from the specified section
    \param  sectionname             the section to clear
    \param  bBlankLineAtSectionEnd  add a blank line at the end of the now empty section
    \return true on success
*/
bool INIFile::clearSection(std::string_view sectionname, bool bBlankLineAtSectionEnd) {
    auto section = getSectionInternal(sectionname);

    if (section.empty())
        return false;

    const auto section_index = std::distance(lines_.begin(), section.begin());

    const auto was_last = section.end() == lines_.end();

    auto skip_section_header = section | std::views::drop(sectionname.empty() ? 0 : 1);

    if (!skip_section_header.empty())
        lines_.erase(skip_section_header.begin(), skip_section_header.end());

    if (!sectionname.empty()) {
        assert(section_index < lines_.size());

        // Get the new iterator that points to our Section.
        auto section_header = lines_.begin();

        std::advance(section_header, section_index);

        // now we add one blank line if not last section
        if (bBlankLineAtSectionEnd && was_last) {
            auto pos = section_header + 1;
            if (pos != lines_.end())
                lines_.emplace(pos, ""sv);
        }
    }

    return true;
}

[[nodiscard]] INIFile::lines_type::iterator
INIFile::getKeyInternal(std::ranges::subrange<lines_type::iterator> section, std::string_view key) {
    auto it = std::ranges::find_if(section, [key](auto& v) {
        if (!std::holds_alternative<Key>(v))
            return false;

        const auto& key_line = std::get<Key>(v);

        return lower_compare(key_line.getKeyName(), key);
    });

    return it == section.end() ? this->lines_.end() : it;
}

[[nodiscard]] INIFile::lines_type::const_iterator
INIFile::getKeyInternal(std::ranges::subrange<lines_type::const_iterator> section, std::string_view key) const {
    auto it = std::ranges::find_if(section, [key](auto& v) {
        if (!std::holds_alternative<Key>(v))
            return false;

        const auto& key_line = std::get<Key>(v);

        return lower_compare(key_line.getKeyName(), key);
    });

    return it == section.end() ? this->lines_.end() : it;
}

INIFile::lines_type::iterator INIFile::getKeyInternal(std::string_view section, std::string_view key) {
    return getKeyInternal(getSectionInternal(section), key);
}

INIFile::lines_type::const_iterator INIFile::getKeyInternal(std::string_view section, std::string_view key) const {
    return getKeyInternal(getSectionInternal(section), key);
}

/**
    This method checks whether the specified key exists in the specified section.
    \param  section         sectionname
    \param  key             keyname
    \return true, if the key exists, false otherwise
*/
bool INIFile::hasKey(std::string_view section, std::string_view key) const {
    const auto it = getKeyInternal(section, key);

    return it != lines_.end();
}

/**
    This method returns a pointer to the key specified by sectionname and keyname
    \param  section     the section
    \param  key         the name of the key
    \return the key if found, nullptr otherwise
*/
const INIFile::Key* INIFile::getKey(std::string_view section, std::string_view key) const {
    const auto it = getKeyInternal(section, key);

    if (it == lines_.end() || !std::holds_alternative<Key>(*it))
        return nullptr;

    return std::addressof(std::get<Key>(*it));
}

/**
    Removes one key from this ini file
    \param  sectionname     the section containing the key
    \param  keyname         the name of the key
    \return true if removing was successful
*/
bool INIFile::removeKey(std::string_view section, std::string_view key) {
    const auto it = getKeyInternal(section, key);

    if (it == lines_.end())
        return false;

    lines_.erase(it);

    return true;
}

/// Reads the string that is addressed by the section/key pair.
/**
    Returns the value that is addressed by the section/key pair as a string. If the key could not be found in
    this section defaultValue is returned. If no defaultValue is specified then "" is returned.
    \param  section         sectionname
    \param  key             keyname
    \param  defaultValue    default value for defaultValue is ""
    \return The read value or default
*/
std::string
INIFile::getStringValue(std::string_view section, std::string_view key, std::string_view defaultValue) const {
    const auto* const curKey = getKey(section, key);

    return std::string{curKey ? curKey->getStringView() : defaultValue};
}

/// Reads the int that is addressed by the section/key pair.
/**
    Returns the value that is addressed by the section/key pair as a int. If the key could not be found in
    this section defaultValue is returned. If no defaultValue is specified then 0 is returned. If the value
    could not be converted to an int 0 is returned.
    \param  section         sectionname
    \param  key             keyname
    \param  defaultValue    default value for defaultValue is 0
    \return The read number, defaultValue or 0
*/
int INIFile::getIntValue(std::string_view section, std::string_view key, int defaultValue) const {
    const auto* const curKey = getKey(section, key);

    return curKey ? curKey->getIntValue(defaultValue) : defaultValue;
}

/// Reads the boolean that is addressed by the section/key pair.
/**
    Returns the value that is addressed by the section/key pair as a boolean. If the key could not be found in
    this section defaultValue is returned. If no defaultValue is specified then false is returned. If the value
    is one of "true", "enabled", "on" or "1" then true is returned; if it is one of "false", "disabled", "off" or
    "0" than false is returned; otherwise defaultValue is returned.
    \param  section         sectionname
    \param  key             keyname
    \param  defaultValue    default value for defaultValue is 0
    \return true for "true", "enabled", "on" and "1"<br>false for "false", "disabled", "off" and "0"
*/
bool INIFile::getBoolValue(std::string_view section, std::string_view key, bool defaultValue) const {
    const auto* const curKey = getKey(section, key);

    return curKey ? curKey->getBoolValue(defaultValue) : defaultValue;
}

/// Reads the float that is addressed by the section/key pair.
/**
    Returns the value that is addressed by the section/key pair as a double. If the key could not be found in
    this section defaultValue is returned. If no defaultValue is specified then 0.0f is returned. If the value
    could not be converted to an float 0.0f is returned.
    \param  section         sectionname
    \param  key             keyname
    \param  defaultValue    default value for defaultValue is 0.0f
    \return The read number, defaultValue or 0.0f
*/
float INIFile::getFloatValue(std::string_view section, std::string_view key, float defaultValue) const {
    const auto* const curKey = getKey(section, key);

    return curKey ? curKey->getFloatValue(defaultValue) : defaultValue;
}

/// Reads the double that is addressed by the section/key pair.
/**
    Returns the value that is addressed by the section/key pair as a double. If the key could not be found in
    this section defaultValue is returned. If no defaultValue is specified then 0.0 is returned. If the value
    could not be converted to an double 0.0 is returned.
    \param  section         sectionname
    \param  key             keyname
    \param  defaultValue    default value for defaultValue is 0.0
    \return The read number, defaultValue or 0.0
*/
double INIFile::getDoubleValue(std::string_view section, std::string_view key, double defaultValue) const {
    const auto* const curKey = getKey(section, key);

    return curKey ? curKey->getDoubleValue(defaultValue) : defaultValue;
}

/// Sets the string that is addressed by the section/key pair.
/**
    Sets the string that is addressed by the section/key pair to value. If the section and/or the key does not exist
   it will be created. A valid sectionname/keyname is not allowed to contain '[',']',';' or '#' and can not start or
   end with whitespaces (' ' or '\\t').
   \param  sectionname         sectionname
   \param  keyname                 keyname
   \param value value that should be set
   \param  bEscapeIfNeeded   escape the string if it contains any special characters
*/
void INIFile::setStringValue(std::string_view sectionname, std::string_view keyname, std::string_view value,
                             bool bEscapeIfNeeded) {
    auto section = getSectionOrCreate(sectionname);

    auto last_key   = section.end();
    auto last_blank = section.end();

    if (section.empty()) {
        if (!sectionname.empty()) {
            std::cerr << "INIFile: Cannot create section with name " << sectionname << "!" << std::endl;
            return;
        }
    } else {
        const auto begin = sectionname.empty() ? section.begin() : std::next(section.begin());

        for (auto it = begin; it != section.end(); ++it) {
            auto& line = *it;

            if (std::holds_alternative<Section>(line))
                break;

            if (std::holds_alternative<Key>(line)) {
                last_key = it;

                auto& key = std::get<Key>(line);

                if (lower_compare(keyname, key.getKeyName())) {
                    key.setStringValue(value, bEscapeIfNeeded);
                    return;
                }
            } else if (std::holds_alternative<INIFileLine>(line)) {
                if (std::get<INIFileLine>(line).completeLine.empty())
                    last_blank = it;
            }
        }
    }

    // If we don't have a blank line and this isn't the last section, then add a blank line.
    if (last_blank == section.end() && section.end() != lines_.end())
        last_blank = lines_.emplace(section.end(), ""s);

    // Insert the new key after the last key in the section.  If there are no keys, add it before the last
    // blank line.
    const auto pos = last_key == section.end() ? last_blank : std::next(last_key);

    lines_.emplace(pos, std::in_place_type<Key>, keyname, value, bEscapeIfNeeded);
}

/// Sets the int that is addressed by the section/key pair.
/**
    Sets the int that is addressed by the section/key pair to value. If the section and/or the key does not exist it
   will be created. A valid sectionname/keyname is not allowed to contain '[',']',';' or '#' and can not start or
   end with whitespaces (' ' or '\\t').
   \param  section  sectionname
   \param  key      keyname
   \param  value    value that should be set
*/
void INIFile::setIntValue(std::string_view section, std::string_view key, int value) {
    setStringValue(section, key, std::to_string(value), false);
}

/// Sets the boolean that is addressed by the section/key pair.
/**
    Sets the boolean that is addressed by the section/key pair to value. If the section and/or the key does not
   exist it will be created. A valid sectionname/keyname is not allowed to contain '[',']',';' or '#' and can not
   start or end with whitespaces (' ' or '\\t').
   \param  section  sectionname
   \param  key      keyname
   \param  value    value that should be set
*/
void INIFile::setBoolValue(std::string_view section, std::string_view key, bool value) {
    setStringValue(section, key, value ? "true" : "false", false);
}

/// Sets the double that is addressed by the section/key pair.
/**
    Sets the double that is addressed by the section/key pair to value. If the section and/or the key does not exist
   it will be created. A valid sectionname/keyname is not allowed to contain '[',']',';' or '#' and can not start or
   end with whitespaces (' ' or '\\t').
   \param  section  sectionname
   \param  key      keyname
   \param  value    value that should be set
*/
void INIFile::setDoubleValue(std::string_view section, std::string_view key, double value) {
    setStringValue(section, key, std::to_string(value), false);
}

/// Saves the changes made in the INI-File to a file.
/**
    Saves the changes made in the INI-File to a file specified by filename.
    If something goes wrong false is returned otherwise true.
    \param  filename            Filename of the file. This file is opened for writing.
    \param  bDOSLineEnding     Use dos line ending
    \return true on success otherwise false.
*/
bool INIFile::saveChangesTo(const std::filesystem::path& filename, bool bDOSLineEnding) const {
    const sdl2::RWops_ptr file{SDL_RWFromFile(filename.u8string().c_str(), "wb")};

    if (!file) {
        return false;
    }

    return saveChangesTo(file.get(), bDOSLineEnding);
}

/// Saves the changes made in the INI-File to a RWop.
/**
    Saves the changes made in the INI-File to a RWop specified by file.
    If something goes wrong false is returned otherwise true.
    \param  file    SDL_RWops that is used for writing. (Cannot be readonly)
    \return true on success otherwise false.
*/
bool INIFile::saveChangesTo(SDL_RWops* file, bool bDOSLineEnding) const {
    const auto write_eol = [file, line_ending = bDOSLineEnding ? "\r\n"sv : "\n"sv] {
        return 1 == SDL_RWwrite(file, line_ending.data(), line_ending.size(), 1);
    };

    auto first = true;

    for (auto& line_variant : lines_) {
        const auto ok = std::visit(
            [file, write_eol, &first](auto& value) {
                const auto& line = value.completeLine;

                if (first)
                    first = false;
                else {
                    if (!write_eol())
                        return false;
                }

                if (!line.empty() && 1 != SDL_RWwrite(file, line.data(), line.size(), 1)) {
                    std::cout << SDL_GetError() << std::endl;
                    return false;
                }

                return true;
            },
            line_variant);

        if (!ok)
            return false;
    }

    if (bDOSLineEnding) {
        // when dos line ending we also put it at the end of the file
        if (!write_eol())
            return false;
    }

    return true;
}

void INIFile::flush() const {
    for (auto& line_variant : lines_) {
        std::visit(
            [&](const auto& value) {
                const auto& line = value.completeLine;

                std::cout << line << std::endl;
            },
            line_variant);
    }
}

void INIFile::readfile(SDL_RWops* file) {
    lines_.clear();

    std::string completeLine;
    completeLine.reserve(256);

    int lineNum                 = 0;
    INIFileLine* curLine        = nullptr;
    INIFileLine* newINIFileLine = nullptr;
    Section* newSection         = nullptr;
    Key* newKey                 = nullptr;

    SimpleBufferedReader buffer{file};

    auto readfinished = false;

    while (!readfinished) {
        lineNum++;

        completeLine.clear();

        while (true) {
            const auto tmp = buffer.getch();
            if (!tmp.has_value()) {
                readfinished = true;
                break;
            }
            if (tmp == '\n')
                break;
            if (tmp != '\r')
                completeLine += tmp.value();
        }

        const auto* line  = reinterpret_cast<const unsigned char*>(completeLine.c_str());
        bool bSyntaxError = false;

        int ret = getNextChar(line, 0);

        if (ret == -1) {
            // empty line or comment
            lines_.emplace_back(completeLine);
        } else {

            if (line[ret] == '[') {
                // section line
                const int sectionstart = ret + 1;
                const int sectionend   = skipName(line, ret + 1);

                if (line[sectionend] != ']' || getNextChar(line, sectionend + 1) != -1) {
                    bSyntaxError = true;
                } else {
                    // valid section line
                    lines_.emplace_back(std::in_place_type<Section>, completeLine,
                                        substring(sectionstart, sectionend - sectionstart), bWhitespace);
                }
            } else {

                // might be key/value line
                const int keystart = ret;
                const int keyend   = skipKey(line, keystart);

                if (keystart == keyend) {
                    bSyntaxError = true;
                } else {
                    ret = getNextChar(line, keyend);
                    if (ret == -1 || line[ret] != '=') {
                        bSyntaxError = true;
                    } else {
                        const int valuestart = getNextChar(line, ret + 1);
                        if (valuestart == -1) {
                            bSyntaxError = true;
                        } else {
                            if (line[valuestart] == '"') {
                                // now get the next '"'

                                const int valueend = getNextQuote(line, valuestart + 1);

                                if (valueend == -1 || getNextChar(line, valueend + 1) != -1) {
                                    bSyntaxError = true;
                                } else {
                                    // valid key/value line
                                    lines_.emplace_back(std::in_place_type<Key>, completeLine,
                                                        substring(keystart, keyend - keystart),
                                                        substring(valuestart + 1, valueend - valuestart - 1));
                                }

                            } else {
                                const int valueend = skipValue(line, valuestart);

                                if (getNextChar(line, valueend) != -1) {
                                    bSyntaxError = true;
                                } else {
                                    // valid key/value line
                                    lines_.emplace_back(std::in_place_type<Key>, completeLine,
                                                        substring(keystart, keyend - keystart),
                                                        substring(valuestart, valueend - valuestart));
                                }
                            }
                        }
                    }
                }
            }
        }

        if (bSyntaxError) {
            if (completeLine.size() < 100) {
                // there are some buggy ini-files which have a lot of waste at the end of the file
                // and it makes no sense to print all this stuff out. just skip it
                std::cerr << "INIFile: Syntax-Error in line " << lineNum << ":" << completeLine << " !" << std::endl;
            }
            // save this line as a comment
            lines_.emplace_back(completeLine);
        }
    }
}

INIFile::lines_type::iterator INIFile::findSectionInternal(std::string_view sectionname) {
    if (sectionname.empty())
        return lines_.begin();

    return std::ranges::find_if(lines_, [sectionname](const auto& v) {
        if (!std::holds_alternative<Section>(v))
            return false;

        const auto& section = std::get<Section>(v);

        const auto name = section.getSectionName();

        return lower_compare(name, sectionname);
    });
}

INIFile::lines_type::const_iterator INIFile::findSectionInternal(std::string_view sectionname) const {
    if (sectionname.empty())
        return lines_.begin();

    return std::ranges::find_if(lines_, [sectionname](const auto& v) {
        if (!std::holds_alternative<Section>(v))
            return false;

        const auto& section = std::get<Section>(v);

        const auto name = section.getSectionName();

        return lower_compare(name, sectionname);
    });
}

std::ranges::subrange<INIFile::lines_type::const_iterator>
INIFile::getSectionInternal(std::string_view sectionname) const {
    auto begin = findSectionInternal(sectionname);

    auto end = lines_.end();

    if (begin != lines_.end()) {
        end = std::find_if(sectionname.empty() ? begin : std::next(begin), lines_.end(),
                           [](const auto& v) { return std::holds_alternative<Section>(v); });
    }

    return {begin, end};
}

std::ranges::subrange<INIFile::lines_type::iterator> INIFile::getSectionInternal(std::string_view sectionname) {
    auto begin = findSectionInternal(sectionname);

    auto end = lines_.end();

    if (begin != lines_.end()) {
        end = std::find_if(sectionname.empty() ? begin : std::next(begin), lines_.end(),
                           [](const auto& v) { return std::holds_alternative<Section>(v); });
    }

    return {begin, end};
}

std::ranges::subrange<INIFile::lines_type::iterator> INIFile::getSectionOrCreate(std::string_view sectionname) {
    auto section = findSectionInternal(sectionname);

    if (section != lines_.end()) {
        auto end = std::find_if(sectionname.empty() ? section : std::next(section), lines_.end(),
                                [](const auto& v) { return std::holds_alternative<Section>(v); });

        return {section, end};
    }

    if (sectionname.empty())
        return {lines_.end(), lines_.end()};

    // create new section

    if (!isValidSectionName(sectionname)) {
        std::cerr << "INIFile: Cannot create section with name " << sectionname << "!" << std::endl;
        return {};
    }

    lines_.emplace_back(std::in_place_type<Section>, sectionname, bWhitespace);

    return {lines_.begin() + lines_.size() - 1u, lines_.end()};
}

bool INIFile::isValidSectionName(std::string_view sectionname) {
    for (const char i : sectionname) {
        if (!isNormalChar(i) && !isWhitespace(i)) {
            return false;
        }
    }

    return !(isWhitespace(sectionname[0]) || isWhitespace(sectionname[sectionname.size() - 1]));
}

bool INIFile::isValidKeyName(std::string_view keyname) {
    for (const auto i : keyname) {
        if (!isNormalChar(i) && !isWhitespace(i)) {
            return false;
        }
    }

    return !(isWhitespace(keyname[0]) || isWhitespace(keyname[keyname.size() - 1]));
}

int INIFile::getNextChar(const unsigned char* line, int startpos) {
    while (line[startpos] != '\0') {
        if (line[startpos] == ';' || line[startpos] == '#') {
            // comment
            return -1;
        }
        if (!isWhitespace(line[startpos])) {

            return startpos;
        }
        startpos++;
    }
    return -1;
}

int INIFile::skipName(const unsigned char* line, int startpos) {
    while (line[startpos] != '\0') {
        if (isNormalChar(line[startpos]) || line[startpos] == ' ' || line[startpos] == '\t') {
            startpos++;
        } else {
            return startpos;
        }
    }
    return startpos;
}

int INIFile::skipValue(const unsigned char* line, int startpos) {
    int i = startpos;
    while (line[i] != '\0') {
        if (isNormalChar(line[i]) || isWhitespace(line[i])) {
            i++;
        } else if (line[i] == ';' || line[i] == '#') {
            // begin of a comment
            break;
        } else {
            // some invalid character
            return i;
        }
    }

    // now we go backwards
    while (i >= startpos) {
        if (isNormalChar(line[i])) {
            return i + 1;
        }
        i--;
    }
    return startpos + 1;
}

int INIFile::skipKey(const unsigned char* line, int startpos) {
    int i = startpos;
    while (line[i] != '\0') {
        if (isNormalChar(line[i]) || isWhitespace(line[i])) {
            i++;
        } else if (line[i] == ';' || line[i] == '#' || line[i] == '=') {
            // begin of a comment or '='
            break;
        } else {
            // some invalid character
            return i;
        }
    }

    // now we go backwards
    while (i >= startpos) {
        if (isNormalChar(line[i])) {
            return i + 1;
        }
        i--;
    }
    return startpos + 1;
}

int INIFile::getNextQuote(const unsigned char* line, int startpos) {
    while (line[startpos] != '\0') {
        if (line[startpos] != '"') {
            startpos++;
        } else {
            return startpos;
        }
    }
    return -1;
}

bool INIFile::isWhitespace(unsigned char s) {
    return s == ' ' || s == '\t' || s == '\n' || s == '\r';
}

bool INIFile::isNormalChar(unsigned char s) {
    return !isWhitespace(s) && s >= 33 && s != '"' && s != ';' && s != '#' && s != '[' && s != ']' && s != '=';
}

bool INIFile::lower_compare(std::string_view s1, std::string_view s2) {
    if (s1.size() != s2.size())
        return false;

    return std::ranges::equal(s1, s2, [](auto a, auto b) { return std::tolower(a) == std::tolower(b); });
}
