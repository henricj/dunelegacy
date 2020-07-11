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

#ifndef INIFILE_H
#define INIFILE_H

#include <misc/SDL2pp.h>

#include <array>
#include <string>
#include <list>
#include <algorithm>
#include <filesystem>
#include <charconv>

#define INVALID_LINE (-1)

//!  A class for reading and writing *.ini configuration files.
/*!
    This class can be used to read or write to a *.ini file. An ini-File has a very simple format.<br>
    Example:<br>
        <br>
        ; Comments start with ; or #<br>
        ; start of the first section with name ""<br>
        key1 = value1<br>
        key2 = value2<br>
        ; start of a section with name "Section1"<br>
        [Section1]<br>
        key3 = value3<br>
        key4 = value4<br>
    <br>
    The section names and key names are treated case insensitive.
*/
class INIFile
{
public:

    //\cond
    class INIFileLine;
    class Key;
    class KeyIterator;
    class Section;
    class SectionIterator;


    class INIFileLine
    {
    public:
        INIFileLine(std::string  completeLine, int lineNumber);

        [[nodiscard]] int getLineNumber() const noexcept { return line; };

        friend class INIFile;
        friend class INIFile::Section;

    protected:
        void shiftLineNumber(int shift) noexcept {
            auto pCurrentLine = this;
            while(pCurrentLine != nullptr) {
                pCurrentLine->line += shift;
                pCurrentLine = pCurrentLine->nextLine;
            }
        }

        std::string completeLine;
        int line;
        INIFileLine* nextLine;
        INIFileLine* prevLine;
    };


    class Key : public INIFileLine
    {
    public:
        Key(std::string completeLine, int lineNumber, int keystringbegin, int keystringlength, int valuestringbegin, int valuestringlength);
        Key(const std::string& keyname, const std::string& value, bool bEscapeIfNeeded = true, bool bWhitespace = true);

        [[nodiscard]] std::string      getKeyName() const;
        [[nodiscard]] std::string      getStringValue() const { return std::string{getStringView()}; }
        [[nodiscard]] std::string_view getStringView() const;

        [[nodiscard]] bool getBoolValue(bool defaultValue = false) const;

        template<typename T>
        [[nodiscard]] auto getValue(T defaultValue = T{}) const noexcept -> T {
            auto value = getStringView();
            if(value.empty()) return defaultValue;

            if(value.front() == '+') value.remove_prefix(1);

            auto ret = defaultValue;
            std::from_chars(value.data(), value.data() + value.size(), ret);
            return ret;
        }

#ifdef NEED_FLOAT_TO_CHARS
        [[nodiscard]] auto getValue(float defaultValue) const noexcept -> float {
            auto view = getStringView();
            if(view.empty()) return defaultValue;

            if(view.front() == '+') view.remove_prefix(1);

            const std::string str{view};

            char* end;
            auto  result = std::strtof(str.c_str(), &end);

            if(HUGE_VALF == result || end == str.c_str()) return defaultValue;

            return result;
        }
#endif

#ifdef NEED_DOUBLE_TO_CHARS
        [[nodiscard]] auto getValue(double defaultValue) const noexcept -> double {
            auto view = getStringView();
            if(view.empty()) return defaultValue;

            if(view.front() == '+') view.remove_prefix(1);

            const std::string str{view};

            char* end;
            auto  result = std::strtod(str.c_str(), &end);

            if(HUGE_VAL == result || end == str.c_str()) return defaultValue;

            return result;
        }
#endif

        [[nodiscard]] int    getIntValue(int defaultValue = 0) const noexcept { return getValue(defaultValue); }
        [[nodiscard]] float  getFloatValue(float defaultValue = 0.0f) const noexcept { return getValue(defaultValue); }
        [[nodiscard]] double getDoubleValue(double defaultValue = 0.0) const noexcept { return getValue(defaultValue); }

        void setStringValue(std::string_view newValue, bool bEscapeIfNeeded = true);
        void setBoolValue(bool newValue);

        template<typename T>
        void setValue(T newValue)
        {
            std::array<char, 128> buffer;
            const auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), newValue);

            setStringValue(std::string_view{&buffer[0], static_cast<size_t>(ptr - &buffer[0])});
        }

#ifdef NEED_FLOAT_TO_CHARS
        void setValue(float newValue) {
            const auto str = std::to_string(newValue);

            setStringValue(str);
        }
#endif

#ifdef NEED_DOUBLE_TO_CHARS
        void setValue(double newValue) {
            const auto str = std::to_string(newValue);

            setStringValue(str);
        }
#endif

        void setIntValue(int newValue) { setValue(newValue); }
        void setDoubleValue(double newValue) { setValue(newValue); }


        friend class INIFile;
        friend class INIFile::KeyIterator;
        friend class INIFile::Section;
        friend class INIFile::SectionIterator;

    protected:
        static bool        escapingValueNeeded(std::string_view value);
        static std::string escapeValue(const std::string& value);

        int keyStringBegin;
        int keyStringLength;
        int valueStringBegin;
        int valueStringLength;
        Key* nextKey;
        Key* prevKey;
    };


    class KeyIterator
    {
    public:
        KeyIterator() noexcept : key(nullptr) {
        }

        explicit KeyIterator(Key* pKey) noexcept : key(pKey) {
        }

        Key& operator*() const noexcept {
            return *key;
        }

        Key* operator->() const noexcept {
            return key;
        }

        bool operator==(const KeyIterator& other) const noexcept {
            return (key == other.key);
        }

        bool operator!=(const KeyIterator& other) const noexcept {
            return !(operator==(other));
        }

        void operator++() {
            if(key != nullptr) {
                key = key->nextKey;
            }
        }

    private:
        Key* key;
    };


    class Section : public INIFileLine
    {
    public:
        Section(std::string completeLine, int lineNumber, int sectionstringbegin, int sectionstringlength, bool bWhitespace = true);
        Section(const std::string& sectionname, bool bWhitespace = true);

        [[nodiscard]] std::string getSectionName() const;
        [[nodiscard]] KeyIterator begin() const;
        [[nodiscard]] KeyIterator end() const;

        [[nodiscard]] bool hasKey(const std::string& key) const;
        [[nodiscard]] Key* getKey(const std::string& keyname) const;

        void setStringValue(const std::string& key, const std::string& newValue, bool bEscapeIfNeeded = true);
        void setIntValue(const std::string& key, int newValue);
        void setBoolValue(const std::string& key, bool newValue);
        void setDoubleValue(const std::string& key, double newValue);

        friend class INIFile;
        friend class INIFile::SectionIterator;

    protected:
        void insertKey(Key* newKey);

        int sectionStringBegin;
        int sectionStringLength;
        Section* nextSection;
        Section* prevSection;
        Key* keyRoot;
        bool bWhitespace;
    };


    class SectionIterator
    {
    public:
        SectionIterator() noexcept : section(nullptr) {
        }

        explicit SectionIterator(Section* pSection) noexcept : section(pSection) {
        }

        Section& operator*() const noexcept {
            return *section;
        }

        Section* operator->() const noexcept {
            return section;
        }

        bool operator==(const SectionIterator& other) const noexcept {
            return (section == other.section);
        }

        bool operator!=(const SectionIterator& other) const noexcept {
            return !(operator==(other));
        }

        void operator++() noexcept {
            if(section != nullptr) {
                section = section->nextSection;
            }
        }

    private:
        Section* section;
    };
    //\endcond



public:

    INIFile(bool bWhitespace, const std::string& firstLineComment);
    INIFile(const std::filesystem::path& filename, bool bWhitespace = true);
    INIFile(SDL_RWops * RWopsFile, bool bWhitespace = true);
    INIFile(const INIFile& o) = delete;
    ~INIFile();

    [[nodiscard]] bool hasSection(const std::string& section) const;
    [[nodiscard]] const Section& getSection(const std::string& sectionname) const;
    bool removeSection(const std::string& sectionname);
    bool clearSection(const std::string& sectionname, bool bBlankLineAtSectionEnd = true);
    [[nodiscard]] bool hasKey(const std::string& section, const std::string& key) const;
    [[nodiscard]] const Key* getKey(const std::string& sectionname, const std::string& keyname) const;
    bool removeKey(const std::string& section, const std::string& key);

    [[nodiscard]] std::string getStringValue(const std::string& section, const std::string& key, const std::string& defaultValue = "") const;
    [[nodiscard]] int getIntValue(const std::string& section, const std::string& key, int defaultValue = 0) const;
    [[nodiscard]] bool getBoolValue(const std::string& section, const std::string& key, bool defaultValue = false) const;
    [[nodiscard]] float getFloatValue(const std::string& section, const std::string& key, float defaultValue = 0.0f) const;
    [[nodiscard]] double getDoubleValue(const std::string& section, const std::string& key, double defaultValue = 0.0) const;

    void setStringValue(const std::string& section, const std::string& key, const std::string& value, bool bEscapeIfNeeded = true);
    void setIntValue(const std::string& section, const std::string& key, int value);
    void setBoolValue(const std::string& section, const std::string& key, bool value);
    void setDoubleValue(const std::string& section, const std::string& key, double value);

    [[nodiscard]] SectionIterator begin() const;
    [[nodiscard]] SectionIterator end() const;

    [[nodiscard]] KeyIterator begin(const std::string& section) const;
    [[nodiscard]] KeyIterator end(const std::string& section) const;

    [[nodiscard]] bool saveChangesTo(const std::filesystem::path& filename, bool bDOSLineEnding = false) const;
    bool saveChangesTo(SDL_RWops * file, bool bDOSLineEnding = false) const;

private:
    INIFileLine* firstLine;
    Section* sectionRoot;
    bool bWhitespace;

    void flush() const;
    void readfile(SDL_RWops * file);

    void insertSection(Section* newSection);

    [[nodiscard]] const Section* getSectionInternal(const std::string& sectionname) const;
    Section* getSectionOrCreate(const std::string& sectionname);

    static bool isValidSectionName(const std::string& sectionname);
    static bool isValidKeyName(const std::string& keyname);

    static int getNextChar(const unsigned char* line, int startpos);
    static int skipName(const unsigned char* line,int startpos);
    static int skipValue(const unsigned char* line,int startpos);
    static int skipKey(const unsigned char* line,int startpos);
    static int getNextQuote(const unsigned char* line,int startpos);

    static bool isWhitespace(unsigned char s);
    static bool isNormalChar(unsigned char s);

    static int strncicmp(const char *s1, const char *s2, size_t n);
};

#endif // INIFILE_H

