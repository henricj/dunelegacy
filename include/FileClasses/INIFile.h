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

#include <string>
#include <list>
#include <algorithm>

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
        INIFileLine(const std::string& completeLine, int lineNumber);

        inline int getLineNumber() const { return line; };

        friend class INIFile;
        friend class INIFile::Section;

    protected:
        inline void shiftLineNumber(int shift) {
            INIFileLine* pCurrentLine = this;
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
        Key(const std::string& completeLine, int lineNumber, int keystringbegin, int keystringlength, int valuestringbegin, int valuestringlength);
        Key(const std::string& keyname, const std::string& value, bool bEscapeIfNeeded = true, bool bWhitespace = true);

        std::string getKeyName() const;
        std::string getStringValue() const;
        int getIntValue(int defaultValue = 0) const;
        bool getBoolValue(bool defaultValue = false) const;
        float getFloatValue(float defaultValue = 0.0f) const;
        double getDoubleValue(double defaultValue = 0.0) const;

        void setStringValue(const std::string& newValue, bool bEscapeIfNeeded = true);
        void setIntValue(int newValue);
        void setBoolValue(bool newValue);
        void setDoubleValue(double newValue);

        friend class INIFile;
        friend class INIFile::KeyIterator;
        friend class INIFile::Section;
        friend class INIFile::SectionIterator;

    protected:
        static bool escapingValueNeeded(const std::string& value);
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
        KeyIterator() : key(nullptr) {
        }

        explicit KeyIterator(Key* pKey) : key(pKey) {
        }

        Key& operator*() const {
            return *key;
        }

        Key* operator->() const {
            return key;
        }

        bool operator==(const KeyIterator& other) const {
            return (key == other.key);
        }

        bool operator!=(const KeyIterator& other) const {
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
        Section(const std::string& completeLine, int lineNumber, int sectionstringbegin, int sectionstringlength, bool bWhitespace = true);
        Section(const std::string& sectionname, bool bWhitespace = true);

        std::string getSectionName() const;
        KeyIterator begin() const;
        KeyIterator end() const;

        bool hasKey(const std::string& key) const;
        Key* getKey(const std::string& keyname) const;

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
        SectionIterator() : section(nullptr) {
        }

        explicit SectionIterator(Section* pSection) : section(pSection) {
        }

        Section& operator*() const {
            return *section;
        }

        Section* operator->() const {
            return section;
        }

        bool operator==(const SectionIterator& other) const {
            return (section == other.section);
        }

        bool operator!=(const SectionIterator& other) const {
            return !(operator==(other));
        }

        void operator++() {
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
    INIFile(const std::string& filename, bool bWhitespace = true);
    INIFile(SDL_RWops * RWopsFile, bool bWhitespace = true);
    INIFile(const INIFile& o) = delete;
    ~INIFile();

    bool hasSection(const std::string& section) const;
    const Section& getSection(const std::string& sectionname) const;
    bool removeSection(const std::string& sectionname);
    bool clearSection(const std::string& sectionname, bool bBlankLineAtSectionEnd = true);
    bool hasKey(const std::string& section, const std::string& key) const;
    const Key* getKey(const std::string& sectionname, const std::string& keyname) const;
    bool removeKey(const std::string& section, const std::string& key);

    std::string getStringValue(const std::string& section, const std::string& key, const std::string& defaultValue = "") const;
    int getIntValue(const std::string& section, const std::string& key, int defaultValue = 0) const;
    bool getBoolValue(const std::string& section, const std::string& key, bool defaultValue = false) const;
    float getFloatValue(const std::string& section, const std::string& key, float defaultValue = 0.0f) const;
    double getDoubleValue(const std::string& section, const std::string& key, double defaultValue = 0.0) const;

    void setStringValue(const std::string& section, const std::string& key, const std::string& value, bool bEscapeIfNeeded = true);
    void setIntValue(const std::string& section, const std::string& key, int value);
    void setBoolValue(const std::string& section, const std::string& key, bool value);
    void setDoubleValue(const std::string& section, const std::string& key, double value);

    SectionIterator begin() const;
    SectionIterator end() const;

    KeyIterator begin(const std::string& section) const;
    KeyIterator end(const std::string& section) const;

    bool saveChangesTo(const std::string& filename, bool bDOSLineEnding = false) const;
    bool saveChangesTo(SDL_RWops * file, bool bDOSLineEnding = false) const;

private:
    INIFileLine* firstLine;
    Section* sectionRoot;
    bool bWhitespace;

    void flush() const;
    void readfile(SDL_RWops * file);

    void insertSection(Section* newSection);

    const Section* getSectionInternal(const std::string& sectionname) const;
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

