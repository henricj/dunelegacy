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

#include <FileClasses/MentatTextFile.h>

#include <misc/string_util.h>
#include <misc/exceptions.h>

#include <SDL2/SDL_endian.h>
#include <stdio.h>
#include <string>
#include <algorithm>

MentatTextFile::MentatTextFile(SDL_RWops* rwop) {
    if(rwop == nullptr) {
        THROW(std::invalid_argument, "MentatTextFile:MentatTextFile(): rwop == nullptr!");
    }

    Sint64 endOffset = SDL_RWsize(rwop);
    if(endOffset < 0) {
        THROW(std::runtime_error, "MentatTextFile:MentatTextFile(): Cannot determine size of this file!");
    }

    size_t mentatTextFilesize = static_cast<size_t>(endOffset);

    if(mentatTextFilesize < 20) {
        THROW(std::runtime_error, "MentatTextFile:MentatTextFile(): No valid mentat textfile: File too small!");
    }

    std::vector<unsigned char> filedata(mentatTextFilesize);

    if(SDL_RWread(rwop, filedata.data(), mentatTextFilesize, 1) != 1) {
        THROW(std::runtime_error, "MentatTextFile:MentatTextFile(): Reading this mentat textfile failed!");
    }

    if(memcmp(&filedata[0], "FORM", 4) != 0) {
        THROW(std::runtime_error, "MentatTextFile:MentatTextFile(): Invalid mentat textfile! Must start with 'FORM'");
    }

    Uint32 formSectionSize = SDL_SwapBE32(*((Uint32*) (filedata.data()+4)));

    if(formSectionSize + 8 != mentatTextFilesize) {
        THROW(std::runtime_error, "MentatTextFile:MentatTextFile(): Invalid mentat textfile!");
    }

    // MENTNAME
    if(memcmp(&filedata[8], "MENTNAME", 8) != 0) {
        THROW(std::runtime_error, "MentatTextFile:MentatTextFile(): Invalid mentat textfile!");
    }

    Uint32 mentnameSectionSize = SDL_SwapBE32(*((Uint32*) (filedata.data()+16)));

    unsigned char* pMentNameSection = filedata.data() + 20;

    unsigned char* pCurrentPos = pMentNameSection;
    unsigned char* pMentNameSectionEnd = pMentNameSection + mentnameSectionSize;
    while(pCurrentPos < pMentNameSectionEnd) {
        unsigned int entryLength = *pCurrentPos;

        Uint32 entryContentOffset = SDL_SwapBE32(*((Uint32*) (pCurrentPos+1)));

        unsigned int numMenuEntry = *((char*) pCurrentPos + 5) - '0';
        unsigned int menuLevel = *((char*) pCurrentPos + 6) - '0';
        std::string entryTitle((char*) pCurrentPos + 7);

        int techLevel = *((char*) pCurrentPos + entryLength - 1);

        if(entryContentOffset >= mentatTextFilesize) {
            THROW(std::runtime_error, "MentatTextFile:MentatTextFile(): Entry offset 0x%X beyond file end!", entryContentOffset);
        }

        std::string compressedEntryContent((char*) filedata.data() + entryContentOffset);

        std::string entryContent = convertCP850ToUTF8(decodeString(compressedEntryContent));

        size_t delimPos = entryContent.find_first_of('*');
        std::string filename = entryContent.substr(0,delimPos);
        std::string nameAndContent = (delimPos == std::string::npos) ? "" : entryContent.substr(delimPos + 1);

        size_t delimPos2 = nameAndContent.find(" \n");
        std::string name = nameAndContent.substr(0,delimPos2);
        std::string content = (delimPos2 == std::string::npos) ? "" : nameAndContent.substr(delimPos2 + 2);

        mentatEntries.emplace_back( convertCP850ToUTF8(entryTitle), numMenuEntry, menuLevel, techLevel, filename, name, content );

        pCurrentPos += entryLength;
    }
}

MentatTextFile::~MentatTextFile() = default;

