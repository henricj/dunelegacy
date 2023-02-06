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

#include <misc/dune_endian.h>
#include <misc/exceptions.h>
#include <misc/string_util.h>

#include <SDL2/SDL_endian.h>

#include <string>

MentatTextFile::MentatTextFile(SDL_RWops* rwop) {
    if (rwop == nullptr) {
        THROW(std::invalid_argument, "MentatTextFile:MentatTextFile(): rwop == nullptr!");
    }

    int64_t endOffset = SDL_RWsize(rwop);
    if (endOffset < 0) {
        THROW(std::runtime_error, "MentatTextFile:MentatTextFile(): Cannot determine size of this file!");
    }

    auto mentatTextFilesize = static_cast<size_t>(endOffset);

    if (mentatTextFilesize < 20) {
        THROW(std::runtime_error, "MentatTextFile:MentatTextFile(): No valid mentat textfile: File too small!");
    }

    std::vector<unsigned char> filedata(mentatTextFilesize);

    if (SDL_RWread(rwop, filedata.data(), mentatTextFilesize, 1) != 1) {
        THROW(std::runtime_error, "MentatTextFile:MentatTextFile(): Reading this mentat textfile failed!");
    }

    if (memcmp(&filedata[0], "FORM", 4) != 0) {
        THROW(std::runtime_error, "MentatTextFile:MentatTextFile(): Invalid mentat textfile! Must start with 'FORM'");
    }

    Uint32 formSectionSize = SDL_SwapBE32(reinterpret_cast<Uint32*>(filedata.data())[1]);

    if (formSectionSize + 8 != mentatTextFilesize) {
        THROW(std::runtime_error, "MentatTextFile:MentatTextFile(): Invalid mentat textfile!");
    }

    // MENTNAME
    if (memcmp(&filedata[8], "MENTNAME", 8) != 0) {
        THROW(std::runtime_error, "MentatTextFile:MentatTextFile(): Invalid mentat textfile!");
    }

    auto mentnameSectionSize = SDL_SwapBE32(reinterpret_cast<const Uint32*>(filedata.data())[4]);

    const auto* const pMentNameSection = filedata.data() + 20;

    const auto* pCurrentPos               = pMentNameSection;
    const auto* const pMentNameSectionEnd = pMentNameSection + mentnameSectionSize;
    while (pCurrentPos < pMentNameSectionEnd) {
        const auto entryLength = *pCurrentPos;

        auto entryContentOffset = dune::read_be_uint32(reinterpret_cast<const Uint32*>(pCurrentPos + 1)[0]);

        const auto* p = reinterpret_cast<const char*>(pCurrentPos);

        unsigned int numMenuEntry = p[5] - '0';
        unsigned int menuLevel    = p[6] - '0';

        const std::string_view entryTitle(p + 7);

        const uint16_t techLevel = p[entryLength - 1];

        if (entryContentOffset >= mentatTextFilesize) {
            THROW(std::runtime_error, "MentatTextFile:MentatTextFile(): Entry offset {:#X} beyond file end!",
                  entryContentOffset);
        }

        const std::string_view compressedEntryContent(reinterpret_cast<const char*>(filedata.data())
                                                      + entryContentOffset);

        const auto entryContent = convertCP850ToUTF8(decodeString(compressedEntryContent));

        const auto delimPos = entryContent.find_first_of('*');
        const auto valid    = delimPos != std::string::npos;
        std::string filename{entryContent.data(), valid ? delimPos : 0};
        const auto nameAndContent = valid ? std::string_view(entryContent.data() + delimPos + 1) : "";

        const auto delimPos2 = nameAndContent.find(" \n");
        const auto valid2    = delimPos2 != std::string_view::npos;
        std::string name{nameAndContent.data(), valid2 ? delimPos2 : 0};
        std::string content{valid2 ? nameAndContent.substr(delimPos2 + 2) : ""};

        mentatEntries.emplace_back(convertCP850ToUTF8(entryTitle), numMenuEntry, menuLevel, techLevel,
                                   std::move(filename), std::move(name), std::move(content));

        pCurrentPos += entryLength;
    }
}

MentatTextFile::~MentatTextFile() = default;
