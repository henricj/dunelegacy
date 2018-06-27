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

#include <misc/exceptions.h>

#include <fstream>
#include <iostream>
#include <cctype>
#include <algorithm>
#include <stdio.h>


INIFile::INIFileLine::INIFileLine(const std::string& completeLine, int lineNumber)
 : completeLine(completeLine), line(lineNumber), nextLine(nullptr), prevLine(nullptr) {
}

INIFile::Key::Key(const std::string& completeLine, int lineNumber, int keystringbegin, int keystringlength, int valuestringbegin, int valuestringlength)
 :  INIFileLine(completeLine, lineNumber), keyStringBegin(keystringbegin), keyStringLength(keystringlength),
    valueStringBegin(valuestringbegin), valueStringLength(valuestringlength),
    nextKey(nullptr), prevKey(nullptr) {
}

INIFile::Key::Key(const std::string& keyname, const std::string& value, bool bEscapeIfNeeded, bool bWhitespace)
 :  INIFileLine(keyname + (bWhitespace ? " = " : "=") + (bEscapeIfNeeded ? escapeValue(value) : value), INVALID_LINE), keyStringBegin(0), keyStringLength(keyname.size()),
    valueStringBegin(keyname.size() + (bWhitespace ? 3 : 1) + ((bEscapeIfNeeded && escapingValueNeeded(value)) ? 1 : 0)), valueStringLength(value.size()),
    nextKey(nullptr), prevKey(nullptr) {
}

std::string INIFile::Key::getKeyName() const {
    return completeLine.substr(keyStringBegin,keyStringLength);
}

std::string INIFile::Key::getStringValue() const {
    return completeLine.substr(valueStringBegin,valueStringLength);
}

int INIFile::Key::getIntValue(int defaultValue) const {
    std::string value = getStringValue();
    if(value.size() == 0) {
        return defaultValue;
    }

    long ret;
    if(value.at(0) == '-') {
        ret = -(atol(value.c_str()+1));
    } else if (value.at(0) == '+') {
        ret = atol(value.c_str()+1);
    } else {
        ret = atol(value.c_str());
    }

    return ret;
}

bool INIFile::Key::getBoolValue(bool defaultValue) const {
    std::string value = getStringValue();
    if(value.size() == 0) {
        return defaultValue;
    }

    // convert string to lower case
    std::transform(value.begin(),value.end(), value.begin(), (int(*)(int)) tolower);

    if((value == "true") || (value == "enabled") || (value == "on") || (value == "1")) {
        return true;
    } else if((value == "false") || (value == "disabled") || (value == "off") || (value == "0")) {
        return false;
    } else {
        return defaultValue;
    }
}

float INIFile::Key::getFloatValue(float defaultValue) const {
    std::string value = getStringValue();
    if(value.size() == 0) {
        return defaultValue;
    }

    float ret = strtof(value.c_str(), nullptr);

    return ret;
}

double INIFile::Key::getDoubleValue(double defaultValue) const {
    std::string value = getStringValue();
    if(value.size() == 0) {
        return defaultValue;
    }

    double ret = strtod(value.c_str(), nullptr);

    return ret;
}

void INIFile::Key::setStringValue(const std::string& newValue, bool bEscapeIfNeeded) {
    if(completeLine[valueStringBegin-1] == '"') {
        completeLine.replace(valueStringBegin-1,valueStringLength+2, bEscapeIfNeeded ? escapeValue(newValue) : newValue);
    } else {
        completeLine.replace(valueStringBegin,valueStringLength, bEscapeIfNeeded ? escapeValue(newValue) : newValue);
    }
}

void INIFile::Key::setIntValue(int newValue) {
    setStringValue(std::to_string(newValue));
}

void INIFile::Key::setBoolValue(bool newValue) {
    if(newValue == true) {
        setStringValue("true");
    } else {
        setStringValue("false");
    }
}

void INIFile::Key::setDoubleValue(double newValue) {
    setStringValue(std::to_string(newValue));
}

bool INIFile::Key::escapingValueNeeded(const std::string& value) {
    if(value == "") {
        return true;
    } else {
        // test for non normal char
        for(unsigned int i = 0; i < value.size(); i++) {
            if(!isNormalChar(value[i])) {
                return true;
            }
        }
        return false;
    }
}

std::string INIFile::Key::escapeValue(const std::string& value) {
    if(escapingValueNeeded(value)) {
        return "\"" + value + "\"";
    } else {
        return value;
    }
}


INIFile::Section::Section(const std::string& completeLine, int lineNumber, int sectionstringbegin, int sectionstringlength, bool bWhitespace)
 :  INIFileLine(completeLine, lineNumber), sectionStringBegin(sectionstringbegin), sectionStringLength(sectionstringlength),
    nextSection(nullptr), prevSection(nullptr), keyRoot(nullptr), bWhitespace(bWhitespace) {
}

INIFile::Section::Section(const std::string& sectionname, bool bWhitespace)
 :  INIFileLine("[" + sectionname + "]", INVALID_LINE), sectionStringBegin(1), sectionStringLength(sectionname.size()),
    nextSection(nullptr), prevSection(nullptr), keyRoot(nullptr), bWhitespace(bWhitespace) {
}

/// Get the name for this section
/**
    This method returns the name of this section
    \return name of this section
*/
std::string INIFile::Section::getSectionName() const {
    return completeLine.substr(sectionStringBegin, sectionStringLength);
}


/// Get a key iterator pointing at the first key in this section
/**
    This method returns a key iterator pointing at the first key in this section.
    \return the iterator
*/
INIFile::KeyIterator INIFile::Section::begin() const {
    return KeyIterator(keyRoot);
}

/// Get a key iterator pointing past the last key in this section
/**
    This method returns a key iterator pointing past the last key in this section.
    \return the iterator
*/
INIFile::KeyIterator INIFile::Section::end() const {
    return KeyIterator();
}

/**
    This method checks whether the specified key exists in this section.
    \param  key             keyname
    \return true, if the key exists, false otherwise
*/
bool INIFile::Section::hasKey(const std::string& key) const {
    return (getKey(key) != nullptr);
}

INIFile::Key* INIFile::Section::getKey(const std::string& keyname) const {
    for(INIFile::Key& key : *this) {
        if(key.keyStringLength == (int) keyname.size()) {
                if(strncicmp(keyname.c_str(), key.completeLine.c_str()+key.keyStringBegin, keyname.size()) == 0) {
                    return &key;
                }
        }
    }

    return nullptr;
}


void INIFile::Section::setStringValue(const std::string& key, const std::string& newValue, bool bEscapeIfNeeded) {
    if(hasKey(key)) {
        getKey(key)->setStringValue(newValue, bEscapeIfNeeded);
    } else {
        // create new key
        if(isValidKeyName(key) == false) {
            std::cerr << "INIFile: Cannot create key with name " << key << "!" << std::endl;
            return;
        }

        Key* curKey = new Key(key, newValue, bEscapeIfNeeded, bWhitespace);
        Key* pKey = keyRoot;
        if(pKey == nullptr) {
            // Section has no key yet
            if(nextLine == nullptr) {
                // no line after this section declaration
                nextLine = curKey;
                curKey->prevLine = this;
                curKey->line = line+1;
            } else {
                // lines after this section declaration
                nextLine->prevLine = curKey;
                curKey->nextLine = nextLine;
                nextLine = curKey;
                curKey->prevLine = this;

                curKey->line = line+1;
                curKey->nextLine->shiftLineNumber(1);
            }
        } else {
            // Section already has some keys
            while(pKey->nextKey != nullptr) {
                pKey = pKey->nextKey;
            }

            if(pKey->nextLine == nullptr) {
                // no line after this key
                pKey->nextLine = curKey;
                curKey->prevLine = pKey;
                curKey->line = pKey->line+1;
            } else {
                // lines after this section declaration
                pKey->nextLine->prevLine = curKey;
                curKey->nextLine = pKey->nextLine;
                pKey->nextLine = curKey;
                curKey->prevLine = pKey;

                curKey->line = pKey->line+1;
                curKey->nextLine->shiftLineNumber(1);
            }
        }

        insertKey(curKey);
    }
}

void INIFile::Section::setIntValue(const std::string& key, int newValue) {
    setStringValue(key, std::to_string(newValue), false);
}

void INIFile::Section::setBoolValue(const std::string& key, bool newValue) {
    if(newValue == true) {
        setStringValue(key, "true", false);
    } else {
        setStringValue(key, "false", false);
    }
}

void INIFile::Section::setDoubleValue(const std::string& key, double newValue) {
    setStringValue(key, std::to_string(newValue), false);
}


void INIFile::Section::insertKey(Key* newKey) {
    if(keyRoot == nullptr) {
        // New root element
        keyRoot = newKey;
    } else {
        // insert into list
        Key* curKey = keyRoot;
        while(curKey->nextKey != nullptr) {
            curKey = curKey->nextKey;
        }

        curKey->nextKey = newKey;
        newKey->prevKey = curKey;
    }
}



// public methods

/// Constructor for an empty INI-File.
/**
    This constructor creates an empty INI-File.
    \param  bWhitespace   Insert whitespace between key an value when creating a new entry
    \param  firstLineComment    A comment to put in the first line (no comment is added for an empty string)
*/
INIFile::INIFile(bool bWhitespace, const std::string& firstLineComment)
 : firstLine(nullptr), sectionRoot(nullptr), bWhitespace(bWhitespace)
{
    firstLine = nullptr;
    sectionRoot = nullptr;

    sectionRoot = new Section("", INVALID_LINE, 0, 0, bWhitespace);
    if(!firstLineComment.empty()) {
        firstLine = new INIFileLine("; " + firstLineComment, 0);
        INIFileLine* blankLine = new INIFileLine("",1);
        firstLine->nextLine = blankLine;
        blankLine->prevLine = firstLine;
    }
}


/// Constructor for reading the INI-File from a file.
/**
    This constructor reads the INI-File from the file specified by filename. The file opened in readonly-mode. After
    reading the file it is closed immediately. If the file does not exist, it is treated as empty.
    \param  filename        The file to be opened.
    \param  bWhitespace   Insert whitespace between key an value when creating a new entry
*/
INIFile::INIFile(const std::string& filename, bool bWhitespace)
 : firstLine(nullptr), sectionRoot(nullptr), bWhitespace(bWhitespace) {

    firstLine = nullptr;
    sectionRoot = nullptr;
    SDL_RWops * file;

    // open file
    if((file = SDL_RWFromFile(filename.c_str(),"r")) != nullptr) {
        readfile(file);
        SDL_RWclose(file);
    } else {
        sectionRoot = new Section("", INVALID_LINE, 0, 0, bWhitespace);
    }
}

/// Constructor for reading the INI-File from a SDL_RWops.
/**
    This constructor reads the INI-File from RWopsFile. The RWopsFile can be readonly.
    \param  RWopsFile   Pointer to RWopsFile (can be readonly)
*/
INIFile::INIFile(SDL_RWops * RWopsFile, bool bWhitespace)
 : firstLine(nullptr), sectionRoot(nullptr), bWhitespace(bWhitespace) {

    if(RWopsFile == nullptr) {
        THROW(std::invalid_argument, "RWopsFile == nullptr!");
    }

    readfile(RWopsFile);
}

/// Destructor.
/**
    This is the destructor. Changes to the INI-Files are not automaticly saved. Call INIFile::SaveChangesTo() for that purpose.
*/
INIFile::~INIFile() {
    INIFileLine* curLine = firstLine;
    while(curLine != nullptr) {
        INIFileLine* tmp = curLine;
        curLine = curLine->nextLine;
        delete tmp;
    }

    // now we have to delete the "" section
    delete sectionRoot;
}


/**
    This method checks whether the specified section exists.
    \param  section         sectionname
    \return true, if the section exists, false otherwise
*/
bool INIFile::hasSection(const std::string& section) const {
    return (getSectionInternal(section) != nullptr);
}


/**
    This method returns a reference to the section specified by sectionname
    \param  sectionname the name of the section
    \return the section if found, nullptr otherwise
*/
const INIFile::Section& INIFile::getSection(const std::string& sectionname) const {
    const Section* curSection = getSectionInternal(sectionname);

    if(curSection == nullptr) {
        throw std::out_of_range("There is no section '" + sectionname + "' in this INI file");
    } else {
        return *curSection;
    }
}


/**
    Removes the whole the specified section
    \param  sectionname the section to remove
    \return true on success

*/
bool INIFile::removeSection(const std::string& sectionname) {
    clearSection(sectionname, false);

    INIFile::Section* curSection = const_cast<Section*>(getSectionInternal(sectionname));
    if(curSection == nullptr) {
        return false;
    }

    if(curSection == sectionRoot) {
        // the "" section cannot be removed
        firstLine = sectionRoot->nextSection;
    } else {
        // remove line
        if(curSection->prevLine != nullptr) {
            curSection->prevLine->nextLine = curSection->nextLine;
        }

        if(curSection->nextLine != nullptr) {
            curSection->nextLine->prevLine = curSection->prevLine;
        }

        if(firstLine == curSection) {
            firstLine = curSection->nextLine;
        }

        // remove section from section list
        if(curSection->prevSection != nullptr) {
            curSection->prevSection->nextSection = curSection->nextSection;
        }

        if(curSection->nextSection != nullptr) {
            curSection->nextSection->prevSection = curSection->prevSection;
        }

        delete curSection;
    }

    return true;
}


/**
    Removes all keys from the specified section
    \param  sectionname             the section to clear
    \param  bBlankLineAtSectionEnd  add a blank line at the end of the now empty section
    \return true on success
*/
bool INIFile::clearSection(const std::string& sectionname, bool bBlankLineAtSectionEnd) {
    INIFile::Section* curSection = const_cast<Section*>(getSectionInternal(sectionname));
    if(curSection == nullptr) {
        return false;
    }

    INIFile::INIFileLine* pCurrentLine = (sectionRoot == curSection) ? firstLine : curSection->nextLine;

    while((pCurrentLine != nullptr) && (pCurrentLine != curSection->nextSection)) {
        INIFile::INIFileLine* tmp = pCurrentLine->nextLine;
        delete pCurrentLine;
        pCurrentLine = tmp;
    }

    if(sectionRoot == curSection) {
        // the "" section header is no line => the first line is the first section header
        firstLine = pCurrentLine;
    } else {
        curSection->nextLine = pCurrentLine;
        if(pCurrentLine != nullptr) {
            pCurrentLine->prevLine = curSection;
        }
    }


    curSection->keyRoot = nullptr;

    // now we add one blank line if not last section
    if(bBlankLineAtSectionEnd && (curSection->nextSection != nullptr)) {
        INIFileLine* blankLine = new INIFileLine("",INVALID_LINE);
        if(curSection->nextLine != nullptr) {
            curSection->nextLine->prevLine = blankLine;
            blankLine->nextLine = curSection->nextLine;
        }

        curSection->nextLine = blankLine;
        blankLine->prevLine = curSection;
    }

    return true;
}


/**
    This method checks whether the specified key exists in the specified section.
    \param  section         sectionname
    \param  key             keyname
    \return true, if the key exists, false otherwise
*/
bool INIFile::hasKey(const std::string& section, const std::string& key) const {
    const Section* curSection = getSectionInternal(section);
    if(curSection == nullptr) {
        return false;
    } else {
        return curSection->hasKey(key);
    }
}


/**
    This method returns a pointer to the key specified by sectionname and keyname
    \param  sectionname the section
    \param  keyname     the name of the key
    \return the key if found, nullptr otherwise
*/
const INIFile::Key* INIFile::getKey(const std::string& sectionname, const std::string& keyname) const {
    const INIFile::Section* curSection = getSectionInternal(sectionname);
    if(curSection == nullptr) {
        return nullptr;
    }

    return curSection->getKey(keyname);
}


/**
    Removes one key from this ini file
    \param  sectionname     the section containing the key
    \param  keyname         the name of the key
    \return true if removing was successful
*/
bool INIFile::removeKey(const std::string& sectionname, const std::string& keyname) {
    INIFile::Section* curSection = const_cast<Section*>(getSectionInternal(sectionname));
    if(curSection == nullptr) {
        return false;
    }

    INIFile::Key* key = curSection->getKey(keyname);
    if(key == nullptr) {
        return false;
    }

    // remove line
    if(key->prevLine != nullptr) {
        key->prevLine->nextLine = key->nextLine;
    }

    if(key->nextLine != nullptr) {
        key->nextLine->prevLine = key->prevLine;
    }

    if(firstLine == key) {
        firstLine = key->nextLine;
    }

    // remove key from section
    if(key->prevKey != nullptr) {
        key->prevKey->nextKey = key->nextKey;
    }

    if(key->nextKey != nullptr) {
        key->nextKey->prevKey = key->prevKey;
    }

    if(curSection->keyRoot == key) {
        curSection->keyRoot = key->nextKey;
    }

    delete key;

    return true;
}



/// Reads the string that is adressed by the section/key pair.
/**
    Returns the value that is adressed by the section/key pair as a string. If the key could not be found in
    this section defaultValue is returned. If no defaultValue is specified then "" is returned.
    \param  section         sectionname
    \param  key             keyname
    \param  defaultValue    default value for defaultValue is ""
    \return The read value or default
*/
std::string INIFile::getStringValue(const std::string& section, const std::string& key, const std::string& defaultValue) const {
    const Key* curKey = getKey(section,key);
    if(curKey == nullptr) {
        return defaultValue;
    } else {
        return curKey->getStringValue();
    }
}


/// Reads the int that is adressed by the section/key pair.
/**
    Returns the value that is adressed by the section/key pair as a int. If the key could not be found in
    this section defaultValue is returned. If no defaultValue is specified then 0 is returned. If the value
    could not be converted to an int 0 is returned.
    \param  section         sectionname
    \param  key             keyname
    \param  defaultValue    default value for defaultValue is 0
    \return The read number, defaultValue or 0
*/
int INIFile::getIntValue(const std::string& section, const std::string& key, int defaultValue) const {
    const Key* curKey = getKey(section,key);
    if(curKey == nullptr) {
        return defaultValue;
    } else {
        return curKey->getIntValue(defaultValue);
    }
}

/// Reads the boolean that is adressed by the section/key pair.
/**
    Returns the value that is adressed by the section/key pair as a boolean. If the key could not be found in
    this section defaultValue is returned. If no defaultValue is specified then false is returned. If the value
    is one of "true", "enabled", "on" or "1" then true is returned; if it is one of "false", "disabled", "off" or
    "0" than false is returned; otherwise defaultValue is returned.
    \param  section         sectionname
    \param  key             keyname
    \param  defaultValue    default value for defaultValue is 0
    \return true for "true", "enabled", "on" and "1"<br>false for "false", "disabled", "off" and "0"
*/
bool INIFile::getBoolValue(const std::string& section, const std::string& key, bool defaultValue) const {
    const Key* curKey = getKey(section,key);
    if(curKey == nullptr) {
        return defaultValue;
    } else {
        return curKey->getBoolValue(defaultValue);
    }
}

/// Reads the float that is adressed by the section/key pair.
/**
    Returns the value that is adressed by the section/key pair as a double. If the key could not be found in
    this section defaultValue is returned. If no defaultValue is specified then 0.0f is returned. If the value
    could not be converted to an float 0.0f is returned.
    \param  section         sectionname
    \param  key             keyname
    \param  defaultValue    default value for defaultValue is 0.0f
    \return The read number, defaultValue or 0.0f
*/
float INIFile::getFloatValue(const std::string& section, const std::string& key, float defaultValue) const {
    const Key* curKey = getKey(section,key);
    if(curKey == nullptr) {
        return defaultValue;
    } else {
        return curKey->getFloatValue(defaultValue);
    }
}


/// Reads the double that is adressed by the section/key pair.
/**
    Returns the value that is adressed by the section/key pair as a double. If the key could not be found in
    this section defaultValue is returned. If no defaultValue is specified then 0.0 is returned. If the value
    could not be converted to an double 0.0 is returned.
    \param  section         sectionname
    \param  key             keyname
    \param  defaultValue    default value for defaultValue is 0.0
    \return The read number, defaultValue or 0.0
*/
double INIFile::getDoubleValue(const std::string& section, const std::string& key, double defaultValue) const {
    const Key* curKey = getKey(section,key);
    if(curKey == nullptr) {
        return defaultValue;
    } else {
        return curKey->getDoubleValue(defaultValue);
    }
}

/// Sets the string that is adressed by the section/key pair.
/**
    Sets the string that is adressed by the section/key pair to value. If the section and/or the key does not exist it will
    be created. A valid sectionname/keyname is not allowed to contain '[',']',';' or '#' and can not start or end with
    whitespaces (' ' or '\\t').
    \param  section         sectionname
    \param  key                 keyname
    \param  value               value that should be set
    \param  bEscapeIfNeeded   escape the string if it contains any special characters
*/
void INIFile::setStringValue(const std::string& section, const std::string& key, const std::string& value, bool bEscapeIfNeeded) {
    Section* curSection = getSectionOrCreate(section);

    if(curSection == nullptr) {
        std::cerr << "INIFile: Cannot create section with name " << section << "!" << std::endl;
        return;
    }

    curSection->setStringValue(key, value, bEscapeIfNeeded);
}

/// Sets the int that is adressed by the section/key pair.
/**
    Sets the int that is adressed by the section/key pair to value. If the section and/or the key does not exist it will
    be created. A valid sectionname/keyname is not allowed to contain '[',']',';' or '#' and can not start or end with
    whitespaces (' ' or '\\t').
    \param  section         sectionname
    \param  key             keyname
    \param  value           value that should be set
*/
void INIFile::setIntValue(const std::string& section, const std::string& key, int value) {
    setStringValue(section, key, std::to_string(value), false);
}

/// Sets the boolean that is adressed by the section/key pair.
/**
    Sets the boolean that is adressed by the section/key pair to value. If the section and/or the key does not exist it will
    be created. A valid sectionname/keyname is not allowed to contain '[',']',';' or '#' and can not start or end with
    whitespaces (' ' or '\\t').
    \param  section         sectionname
    \param  key             keyname
    \param  value           value that should be set
*/
void INIFile::setBoolValue(const std::string& section, const std::string& key, bool value) {
    if(value == true) {
        setStringValue(section, key, "true", false);
    } else {
        setStringValue(section, key, "false", false);
    }
}

/// Sets the double that is adressed by the section/key pair.
/**
    Sets the double that is adressed by the section/key pair to value. If the section and/or the key does not exist it will
    be created. A valid sectionname/keyname is not allowed to contain '[',']',';' or '#' and can not start or end with
    whitespaces (' ' or '\\t').
    \param  section         sectionname
    \param  key             keyname
    \param  value           value that should be set
*/
void INIFile::setDoubleValue(const std::string& section, const std::string& key, double value) {
    setStringValue(section, key, std::to_string(value), false);
}


/// Get a section iterator pointing at the first section
/**
    This method returns a section iterator pointing at the first section (which is the anonymous "" section)
    \return the iterator
*/
INIFile::SectionIterator INIFile::begin() const {
    return SectionIterator(sectionRoot);
}

/// Get a section iterator pointing past the last section
/**
    This method returns a section iterator pointing past the last section.
    \return the iterator
*/
INIFile::SectionIterator INIFile::end() const {
    return SectionIterator();
}

/// Get a key iterator pointing at the first key in the specified section
/**
    This method returns a key iterator pointing at the first key in the specified section.
    \param  section the section to iterate over
    \return the iterator
*/
INIFile::KeyIterator INIFile::begin(const std::string& section) const {
    const Section* curSection = getSectionInternal(section);
    if(curSection == nullptr) {
        return KeyIterator(nullptr);
    } else {
        return KeyIterator(curSection->keyRoot);
    }
}

/// Get a key iterator pointing past the end of the specified section
/**
    This method returns a key iterator pointing past the end of the specified section.
    \param  section the section to iterate over
    \return the iterator
*/
INIFile::KeyIterator INIFile::end(const std::string& section) const {
    return KeyIterator();
}

/// Saves the changes made in the INI-File to a file.
/**
    Saves the changes made in the INI-File to a file specified by filename.
    If something goes wrong false is returned otherwise true.
    \param  filename            Filename of the file. This file is opened for writing.
    \param  bDOSLineEnding     Use dos line ending
    \return true on success otherwise false.
*/
bool INIFile::saveChangesTo(const std::string& filename, bool bDOSLineEnding) const {
    SDL_RWops * file;
    if((file = SDL_RWFromFile(filename.c_str(),"wb")) == nullptr) {
        return false;
    }

    bool ret = saveChangesTo(file, bDOSLineEnding);
    SDL_RWclose(file);
    return ret;
}

/// Saves the changes made in the INI-File to a RWop.
/**
    Saves the changes made in the INI-File to a RWop specified by file.
    If something goes wrong false is returned otherwise true.
    \param  file    SDL_RWops that is used for writing. (Cannot be readonly)
    \return true on success otherwise false.
*/
bool INIFile::saveChangesTo(SDL_RWops * file, bool bDOSLineEnding) const {
    INIFileLine* curLine = firstLine;

    bool error = false;
    while(curLine != nullptr) {
        unsigned int written = SDL_RWwrite(file, curLine->completeLine.c_str(), 1, curLine->completeLine.size());
        if(written != curLine->completeLine.size()) {
            std::cout << SDL_GetError() << std::endl;
            error = true;
        }

        if(bDOSLineEnding) {
            // when dos line ending we also put it at the end of the file
            if((written = SDL_RWwrite(file,"\r\n",2,1)) != 1) {
                error = true;
            }
        } else if(curLine->nextLine != nullptr) {
            // when no dos line ending we skip the ending at the last line
            if((written = SDL_RWwrite(file,"\n",1,1)) != 1) {
                error = true;
            }
        }
        curLine = curLine->nextLine;
    }

    return !error;
}

// private methods

void INIFile::flush() const {
    INIFileLine* curLine = firstLine;

    while(curLine != nullptr) {
        std::cout << curLine->completeLine << std::endl;
        curLine = curLine->nextLine;
    }
}

void INIFile::readfile(SDL_RWops * file) {
    sectionRoot = new Section("", INVALID_LINE, 0, 0, bWhitespace);

    Section* curSection = sectionRoot;

    std::string completeLine;
    int lineNum = 0;
    INIFileLine* curLine = nullptr;
    INIFileLine* newINIFileLine;
    Section* newSection;
    Key* newKey;

    bool readfinished = false;

    while(!readfinished) {
        lineNum++;

        completeLine = "";
        unsigned char tmp;

        while(1) {
            size_t readbytes = SDL_RWread(file,&tmp,1,1);
            if(readbytes == 0) {
                readfinished = true;
                break;
            } else if(tmp == '\n') {
                break;
            } else if(tmp != '\r') {
                completeLine += tmp;
            }
        }

        const unsigned char* line = (const unsigned char*) completeLine.c_str();
        bool bSyntaxError = false;

        int ret = getNextChar(line,0);

        if(ret == -1) {
            // empty line or comment
            newINIFileLine = new INIFileLine(completeLine,lineNum);

            if(curLine == nullptr) {
                firstLine = newINIFileLine;
                curLine = newINIFileLine;
            } else {
                curLine->nextLine = newINIFileLine;
                newINIFileLine->prevLine = curLine;
                curLine = newINIFileLine;
            }
        } else {

            if(line[ret] == '[') {
                // section line
                int sectionstart = ret+1;
                int sectionend = skipName(line,ret+1);

                if((line[sectionend] != ']') || (getNextChar(line,sectionend+1) != -1)) {
                    bSyntaxError = true;
                } else {
                    // valid section line
                    newSection = new Section(completeLine,lineNum,sectionstart,sectionend-sectionstart,bWhitespace);

                    if(curLine == nullptr) {
                        firstLine = newSection;
                        curLine = newSection;
                    } else {
                        curLine->nextLine = newSection;
                        newSection->prevLine = curLine;
                        curLine = newSection;
                    }

                    insertSection(newSection);
                    curSection = newSection;
                }
            } else {

                // might be key/value line
                int keystart = ret;
                int keyend = skipKey(line,keystart);

                if(keystart == keyend) {
                    bSyntaxError = true;
                } else {
                    ret = getNextChar(line,keyend);
                    if((ret == -1) ||(line[ret] != '=')) {
                        bSyntaxError = true;
                    } else {
                        int valuestart = getNextChar(line,ret+1);
                        if(valuestart == -1) {
                            bSyntaxError = true;
                        } else {
                            if(line[valuestart] == '"') {
                                // now get the next '"'

                                int valueend = getNextQuote(line,valuestart+1);

                                if((valueend == -1) || (getNextChar(line,valueend+1) != -1)) {
                                    bSyntaxError = true;
                                } else {
                                    // valid key/value line
                                    newKey = new Key(completeLine,lineNum,keystart,keyend-keystart,valuestart+1,valueend-valuestart-1);

                                    if(firstLine == nullptr) {
                                        firstLine = newKey;
                                        curLine = newKey;
                                    } else {
                                        curLine->nextLine = newKey;
                                        newKey->prevLine = curLine;
                                        curLine = newKey;
                                    }

                                    curSection->insertKey(newKey);
                                }

                            } else {
                                int valueend = skipValue(line,valuestart);

                                if(getNextChar(line,valueend) != -1) {
                                    bSyntaxError = true;
                                } else {
                                    // valid key/value line
                                    newKey = new Key(completeLine,lineNum,keystart,keyend-keystart,valuestart,valueend-valuestart);

                                    if(firstLine == nullptr) {
                                        firstLine = newKey;
                                        curLine = newKey;
                                    } else {
                                        curLine->nextLine = newKey;
                                        newKey->prevLine = curLine;
                                        curLine = newKey;
                                    }

                                    curSection->insertKey(newKey);
                                }
                            }
                        }
                    }
                }

            }
        }

        if(bSyntaxError == true) {
            if(completeLine.size() < 100) {
                // there are some buggy ini-files which have a lot of waste at the end of the file
                // and it makes no sense to print all this stuff out. just skip it
                std::cerr << "INIFile: Syntax-Error in line " << lineNum << ":" << completeLine << " !" << std::endl;
            }
            // save this line as a comment
            newINIFileLine = new INIFileLine(completeLine,lineNum);

            if(curLine == nullptr) {
                firstLine = newINIFileLine;
                curLine = newINIFileLine;
            } else {
                curLine->nextLine = newINIFileLine;
                newINIFileLine->prevLine = curLine;
                curLine = newINIFileLine;
            }
        }



    }
}

void INIFile::insertSection(Section* newSection) {
    if(sectionRoot == nullptr) {
        // New root element
        sectionRoot = newSection;
    } else {
        // insert into list
        Section* curSection = sectionRoot;
        while(curSection->nextSection != nullptr) {
            curSection = curSection->nextSection;
        }

        curSection->nextSection = newSection;
        newSection->prevSection = curSection;
    }
}


const INIFile::Section* INIFile::getSectionInternal(const std::string& sectionname) const {
    Section* curSection = sectionRoot;
    int sectionnameSize = sectionname.size();

    while(curSection != nullptr) {
        if(curSection->sectionStringLength == sectionnameSize) {
                if(strncicmp(sectionname.c_str(), curSection->completeLine.c_str()+curSection->sectionStringBegin, sectionnameSize) == 0) {
                    return curSection;
                }
        }

        curSection = curSection->nextSection;
    }

    return nullptr;
}


INIFile::Section* INIFile::getSectionOrCreate(const std::string& sectionname) {
    Section* curSection = const_cast<Section*>(getSectionInternal(sectionname));

    if(curSection == nullptr) {
        // create new section

        if(isValidSectionName(sectionname) == false) {
            std::cerr << "INIFile: Cannot create section with name " << sectionname << "!" << std::endl;
            return nullptr;
        }

        curSection = new Section(sectionname, bWhitespace);

        if(firstLine == nullptr) {
            firstLine = curSection;
        } else {
            INIFileLine* curLine = firstLine;
            while(curLine->nextLine != nullptr) {
                curLine = curLine->nextLine;
            }


            if(curLine->completeLine != "") {
                // previous line is not a blank line => add one blank line
                INIFileLine* blankLine = new INIFileLine("",INVALID_LINE);
                curLine->nextLine = blankLine;
                blankLine->prevLine = curLine;

                blankLine->nextLine = curSection;
                curSection->prevLine = blankLine;
            } else {
                // previous line is an empty line => directly add new section
                curLine->nextLine = curSection;
                curSection->prevLine = curLine;
            }

            curSection->line = curLine->line + 1;
        }

        insertSection(curSection);
    }
    return curSection;
}


bool INIFile::isValidSectionName(const std::string& sectionname) {
    for(unsigned int i = 0; i < sectionname.size(); i++) {
        if( (!isNormalChar(sectionname[i])) && (!isWhitespace(sectionname[i])) ) {
            return false;
        }
    }

    if(isWhitespace(sectionname[0]) || isWhitespace(sectionname[sectionname.size()-1])) {
        return false;
    } else {
        return true;
    }
}

bool INIFile::isValidKeyName(const std::string& keyname) {
    for(unsigned int i = 0; i < keyname.size(); i++) {
        if( (!isNormalChar(keyname[i])) && (!isWhitespace(keyname[i])) ) {
            return false;
        }
    }

    if(isWhitespace(keyname[0]) || isWhitespace(keyname[keyname.size()-1])) {
        return false;
    }
    return true;
}

int INIFile::getNextChar(const unsigned char* line, int startpos) {
    while(line[startpos] != '\0') {
        if((line[startpos] == ';') || (line[startpos] == '#')) {
            // comment
            return -1;
        } else if(!isWhitespace(line[startpos])) {
            return startpos;
        }
        startpos++;
    }
    return -1;
}

int INIFile::skipName(const unsigned char* line,int startpos) {
    while(line[startpos] != '\0') {
        if(isNormalChar(line[startpos]) || (line[startpos] == ' ') || (line[startpos] == '\t')) {
            startpos++;
        } else {
            return startpos;
        }
    }
    return startpos;
}

int INIFile::skipValue(const unsigned char* line,int startpos) {
    int i = startpos;
    while(line[i] != '\0') {
        if(isNormalChar(line[i]) || isWhitespace(line[i])) {
            i++;
        } else if((line[i] == ';') || (line[i] == '#')) {
            // begin of a comment
            break;
        } else {
            // some invalid character
            return i;
        }
    }

    // now we go backwards
    while(i >= startpos) {
        if(isNormalChar(line[i])) {
            return i+1;
        }
        i--;
    }
    return startpos+1;
}

int INIFile::skipKey(const unsigned char* line,int startpos) {
    int i = startpos;
    while(line[i] != '\0') {
        if(isNormalChar(line[i]) || isWhitespace(line[i])) {
            i++;
        } else if((line[i] == ';') || (line[i] == '#') || (line[i] == '=')) {
            // begin of a comment or '='
            break;
        } else {
            // some invalid character
            return i;
        }
    }

    // now we go backwards
    while(i >= startpos) {
        if(isNormalChar(line[i])) {
            return i+1;
        }
        i--;
    }
    return startpos+1;
}

int INIFile::getNextQuote(const unsigned char* line,int startpos) {
    while(line[startpos] != '\0') {
        if(line[startpos] != '"') {
            startpos++;
        } else {
            return startpos;
        }
    }
    return -1;
}

bool INIFile::isWhitespace(unsigned char s) {
    if((s == ' ') || (s == '\t') || (s == '\n') || (s == '\r')) {
        return true;
    } else {
        return false;
    }
}

bool INIFile::isNormalChar(unsigned char s) {
    if((!isWhitespace(s)) && (s >= 33) && (s != '"') && (s != ';') && (s != '#') && (s != '[') && (s != ']') && (s != '=')) {
        return true;
    } else {
        return false;
    }
}

int INIFile::strncicmp(const char *s1, const char *s2, size_t n) {
    const char* p1 = s1;
    const char* p2 = s2;
    while((p1 < s1 + n) && (*p1 != 0) && (toupper(*p1) == toupper(*p2))) {
        ++p1;
        ++p2;
    }
    if(s1 + n == p1) {
        return 0;
    } else {
        return (toupper(*p1) - toupper(*p2));
    }
}
