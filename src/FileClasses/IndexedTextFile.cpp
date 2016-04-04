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

#include <FileClasses/IndexedTextFile.h>

#include <misc/string_util.h>

#include <SDL_endian.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <stdexcept>

IndexedTextFile::IndexedTextFile(SDL_RWops* rwop, bool bDecode) {

	if(rwop == NULL) {
	    throw std::invalid_argument("IndexedTextFile:IndexedTextFile(): rwop == NULL!");
	}

	int indexedTextFilesize = SDL_RWseek(rwop,0,SEEK_END);
	if(indexedTextFilesize <= 0) {
        throw std::runtime_error("IndexedTextFile:IndexedTextFile(): Cannot determine size of this file!");
	}

	if(indexedTextFilesize < 2) {
        throw std::runtime_error("IndexedTextFile:IndexedTextFile(): No valid indexed textfile: File too small!");
	}

	if(SDL_RWseek(rwop,0,SEEK_SET) != 0) {
        throw std::runtime_error("IndexedTextFile:IndexedTextFile(): Seeking in this indexed textfile failed!");
	}

	unsigned char* pFiledata;
	if( (pFiledata = (unsigned char*) malloc(indexedTextFilesize)) == NULL) {
        throw std::bad_alloc();
	}

	if(SDL_RWread(rwop, pFiledata, indexedTextFilesize, 1) != 1) {
	    free(pFiledata);
        throw std::runtime_error("IndexedTextFile:IndexedTextFile(): Reading this indexed textfile failed!");
	}

	int numIndexedStrings = (SDL_SwapLE16(((Uint16*) pFiledata)[0]))/2 - 1;

	Uint16* pIndex = (Uint16*) pFiledata;
	for(int i=0; i <= numIndexedStrings; i++) {
		pIndex[i] = SDL_SwapLE16(pIndex[i]);
	}

    try {
        for(int i=0; i < numIndexedStrings; i++) {
            std::string text((const char*) (pFiledata+pIndex[i]));

            if(bDecode) {
                indexedStrings.push_back(convertCP850ToISO8859_1(decodeString(text)));
            } else {
                indexedStrings.push_back( convertCP850ToISO8859_1(text) );
            }
        }
    } catch(std::exception&) {
        delete [] pFiledata;
        throw;
    }

	free(pFiledata);
}

IndexedTextFile::~IndexedTextFile() {
}
