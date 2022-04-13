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

#ifndef MENTATTEXTFILE_H
#define MENTATTEXTFILE_H

#include <misc/SDL2pp.h>
#include <misc/exceptions.h>

#include <string>
#include <utility>
#include <vector>

/// A class for loading a mentat textfile (e.g. MENTATA.ENG).
/**
    This class can read mentat textfiles and return their content in decoded ANSI Code.
*/
class MentatTextFile {
public:
    class MentatEntry {
    public:
        MentatEntry(std::string title, unsigned int numMenuEntry, unsigned int menuLevel, unsigned int techLevel,
                    std::string filename, std::string name, std::string content)
            : title(std::move(title)), numMenuEntry(numMenuEntry), menuLevel(menuLevel), techLevel(techLevel),
              filename(std::move(filename)), name(std::move(name)), content(std::move(content)) { }

        std::string title;
        unsigned int numMenuEntry;
        unsigned int menuLevel;
        unsigned int techLevel;
        std::string filename;
        std::string name;
        std::string content;
    };

    explicit MentatTextFile(SDL_RWops* rwop);
    ~MentatTextFile();

    MentatTextFile(const MentatTextFile&)            = delete;
    MentatTextFile(MentatTextFile&&)                 = delete;
    MentatTextFile& operator=(const MentatTextFile&) = delete;
    MentatTextFile& operator=(MentatTextFile&&)      = delete;

    /// This method returns the nth entry in this text file.
    /**
        This method returns the nth entry in this text file.
        \param  n       the number of the entry to return
        \return the nth entry in the file.
    */
    [[nodiscard]] const MentatEntry& getMentatEntry(unsigned int n) const {
        if (n >= mentatEntries.size()) {
            THROW(std::invalid_argument, "MentatTextFile:getMentatEntry(): Invalid index!");
        }

        return mentatEntries[n];
    }

    /// This method returns the number of entries in this file
    /**
    \return the number of strings in this file
    */
    [[nodiscard]] unsigned int getNumEntries() const { return mentatEntries.size(); }

private:
    std::vector<MentatEntry> mentatEntries;
};

#endif // MENTATTEXTFILE_H
