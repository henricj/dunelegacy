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

#include <SDL_endian.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <stdexcept>

MentatTextFile::MentatTextFile(SDL_RWops* rwop) {
	unsigned char* pFiledata;

	if(rwop == nullptr) {
	    throw std::invalid_argument("MentatTextFile:MentatTextFile(): rwop == nullptr!");
	}

	int mentatTextFilesize = SDL_RWseek(rwop,0,SEEK_END);
	if(mentatTextFilesize <= 0) {
        throw std::runtime_error("MentatTextFile:MentatTextFile(): Cannot determine size of this file!");
	}

	if(mentatTextFilesize < 20) {
        throw std::runtime_error("MentatTextFile:MentatTextFile(): No valid mentat textfile: File too small!");
	}

	if(SDL_RWseek(rwop,0,SEEK_SET) != 0) {
        throw std::runtime_error("MentatTextFile:MentatTextFile(): Seeking in this mentat textfile failed!");
	}

	if( (pFiledata = (unsigned char*) malloc(mentatTextFilesize)) == nullptr) {
        throw std::bad_alloc();
	}

	if(SDL_RWread(rwop, pFiledata, mentatTextFilesize, 1) != 1) {
	    free(pFiledata);
        throw std::runtime_error("MentatTextFile:MentatTextFile(): Reading this mentat textfile failed!");
	}

	if((pFiledata[0] != 'F') || (pFiledata[1] != 'O') || (pFiledata[2] != 'R') || (pFiledata[3] != 'M')) {
		free(pFiledata);
		throw std::runtime_error("MentatTextFile:MentatTextFile(): Invalid mentat textfile! Must start with 'FORM'");
	}

	Uint32 formSectionSize = SDL_SwapBE32(*((Uint32*) (pFiledata+4)));

	if((int) formSectionSize + 8 != mentatTextFilesize) {
		free(pFiledata);
		throw std::runtime_error("MentatTextFile:MentatTextFile(): Invalid mentat textfile!");
	}

	if((pFiledata[8] != 'M') || (pFiledata[9] != 'E') || (pFiledata[10] != 'N') || (pFiledata[11] != 'T') || (pFiledata[12] != 'N') || (pFiledata[13] != 'A') || (pFiledata[14] != 'M') || (pFiledata[15] != 'E')) {
		free(pFiledata);
		throw std::runtime_error("MentatTextFile:MentatTextFile(): Invalid mentat textfile!");
	}

	Uint32 mentnameSectionSize = SDL_SwapBE32(*((Uint32*) (pFiledata+16)));

	unsigned char* pMentNameSection = pFiledata + 20;

	unsigned char* pCurrentPos = pMentNameSection;
	unsigned char* pMentNameSectionEnd = pMentNameSection + mentnameSectionSize;
	while(pCurrentPos < pMentNameSectionEnd) {
		unsigned int entryLength = *pCurrentPos;

		Uint32 entryContentOffset = SDL_SwapBE32(*((Uint32*) (pCurrentPos+1)));

		unsigned int numMenuEntry = *((char*) pCurrentPos + 5) - '0';
		unsigned int menuLevel = *((char*) pCurrentPos + 6) - '0';
		std::string entryTitle((char*) pCurrentPos + 7);

		int techLevel = *((char*) pCurrentPos + entryLength - 1);

		if((int) entryContentOffset >= mentatTextFilesize) {
			free(pFiledata);
			fprintf(stderr, "0x%x\n", entryContentOffset);
			throw std::runtime_error("MentatTextFile:MentatTextFile(): Entry offset beyond file end!");
		}

		std::string compressedEntryContent((char*) pFiledata + entryContentOffset);

		std::string entryContent = convertCP850ToISO8859_1(decodeString(compressedEntryContent));

		size_t delimPos = entryContent.find_first_of('*');
		std::string filename = entryContent.substr(0,delimPos);
		std::string nameAndContent = (delimPos == std::string::npos) ? "" : entryContent.substr(delimPos + 1);

		size_t delimPos2 = nameAndContent.find(" \n");
		std::string name = nameAndContent.substr(0,delimPos2);
		std::string content = (delimPos2 == std::string::npos) ? "" : nameAndContent.substr(delimPos2 + 2);

		mentatEntries.push_back( MentatEntry( convertCP850ToISO8859_1(entryTitle), numMenuEntry, menuLevel, techLevel, filename, name, content) );

		pCurrentPos += entryLength;
	}

	free(pFiledata);
}

MentatTextFile::~MentatTextFile() {
}

