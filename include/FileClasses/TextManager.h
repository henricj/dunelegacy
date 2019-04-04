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

#ifndef TEXTMANAGER_H
#define TEXTMANAGER_H

#include "IndexedTextFile.h"
#include "MentatTextFile.h"

#include <string>
#include <vector>
#include <map>
#include <memory>

#include <algorithm>

#define MISSION_DESCRIPTION     0
#define MISSION_WIN             1
#define MISSION_LOSE            2
#define MISSION_ADVICE          3


class TextManager {
public:
    /**
        Default constructor
    */
    TextManager();

    /**
        Destructor
    */
    ~TextManager();

    /**
        Load data from Dune II data files
    */
    void loadData();

    /**
        This method returns a briefing text for the specified mission.
        \param  mission     the mission number (0 = house description, 1 - 9 missions)
        \param  texttype    one of MISSION_DESCRIPTION, MISSION_WIN, MISSION_LOSE, MISSION_ADVICE
        \param  house       the house to get the text from (only HOUSE_ATREIDES, HOUSE_ORDOS and HOUSE_HARKONNEN)
        \return the briefing text
    */
    std::string getBriefingText(unsigned int mission, unsigned int texttype, int house) const;

    /**
        This method returns all mentat entries for a specific house and up to the specified tech level.
        \param  house       the house to get the text from (only HOUSE_ATREIDES, HOUSE_ORDOS and HOUSE_HARKONNEN)
        \param  techLevel   the tech level (1 to 8)
        \return a vector of mentat texts
    */
    std::vector<MentatTextFile::MentatEntry> getAllMentatEntries(int house, unsigned int techLevel) const;

    /**
        This method returns a localized version of unlocalizedString
        \param  unlocalizedString   the string in English
        \return the localized version of unlocalizedString
    */
    const std::string& getLocalized(const std::string& unlocalizedString) const {
        const std::string& localizedStringRaw = getLocalizedRaw(unlocalizedString);

        if(!localizedStringRaw.empty() && localizedStringRaw[0] == '@') {
            // post-process
            return postProcessString(localizedStringRaw);
        } else {
            return localizedStringRaw;
        }
    }



private:
    /**
        This method returns a localized version of unlocalizedString. No post-processing of the string is performed
        \param  unlocalizedString   the string in english
        \return the localized version of unlocalizedString
    */
    const std::string& getLocalizedRaw(const std::string& unlocalizedString) const {
        auto iter = localizedString.find(unlocalizedString);
        if((iter != localizedString.end()) && !iter->second.empty()) {
            return iter->second;
        } else {
            return unlocalizedString;
        }
    }

    /**
        Post-process a string of the following form:
        "@FILENAME.EXT|INDEX"
        "@FILENAME.EXT|INDEX#comment"
        "@FILENAME.EXT|INDEX|orig1->replace1|orig2->replace2|orig3->replace3#comment"
        Filename and index must be specified in any case. Everything else is optional.
        \param  unprocessedString   the original string containing the @ at the start
        \return the processed string with data read from FILENAME.EXT
    */
    const std::string& postProcessString(const std::string& unprocessedString) const;

    /**
        Add a original Dune 2 text file
    */
    void addOrigDuneText(const std::string& filename, bool bDecode = false);


    std::array<std::unique_ptr<MentatTextFile>,3> mentatStrings;            ///< The MENTAT?.<EXTENSION> mentat menu texts

    std::map<std::string,std::unique_ptr<IndexedTextFile> > origDuneText;   ///< This map contains all the loaded original Dune II (indexed) text files

    mutable std::map<std::string, std::string> localizedString;             ///< The mapping between English text and localized text
};

#endif //TEXTMANAGER_H
