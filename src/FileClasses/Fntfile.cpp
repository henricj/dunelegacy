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

#include <FileClasses/Fntfile.h>
#include <FileClasses/Palette.h>

#include <SDL_endian.h>
#include <stdlib.h>
#include <string.h>

extern Palette palette;

/// Constructor
/**
	The constructor reads from the RWop all data and saves them internal. The SDL_RWops can be readonly but must support
	seeking. Immediately after the Fntfile-Object is constructed RWops can be closed. All data is saved in the class.
	\param	RWop	SDL_RWops to the fnt-File. (can be readonly)
*/
Fntfile::Fntfile(SDL_RWops* RWop)
{
	unsigned char* pFiledata;
	Uint32 FntFilesize;
	if(RWop == NULL) {
		fprintf(stderr, "Fntfile: RWop == NULL!\n");
		exit(EXIT_FAILURE);
	}

	FntFilesize = SDL_RWseek(RWop,0,SEEK_END);
	if(FntFilesize <= 0) {
		fprintf(stderr,"Fntfile: Cannot determine size of this *.fnt-File!\n");
		exit(EXIT_FAILURE);
	}

	if(SDL_RWseek(RWop,0,SEEK_SET) != 0) {
		fprintf(stderr,"Fntfile: Seeking in this *.fnt-File failed!\n");
		exit(EXIT_FAILURE);
	}

	if( (pFiledata = (unsigned char*) malloc(FntFilesize)) == NULL) {
		fprintf(stderr,"Fntfile: Allocating memory failed!\n");
		exit(EXIT_FAILURE);
	}

	if(SDL_RWread(RWop, pFiledata, FntFilesize, 1) != 1) {
		fprintf(stderr,"Fntfile: Reading this *.fnt-File failed!\n");
		exit(EXIT_FAILURE);
	}

	readfile(pFiledata,(Uint16) FntFilesize);

	free(pFiledata);
}

/// Destructor
/**
	Frees all memory.
*/
Fntfile::~Fntfile()
{
	if(character != NULL) {
		for(Uint32 i = 0; i < numCharacters; i++) {
			if(character[i].data != NULL) {
				free(character[i].data);
			}
		}
		free(character);
	}
}

/// Returns one character in this fnt-File
/**
	This method returns a SDL_Surface containing the nth character in this fnt-File.
	The returned SDL_Surface should be freed with SDL_FreeSurface() if no longer needed.
	\param	indexOfFile	specifies which character to return (zero based)
	\return	nth picture in this fnt-File
*/
SDL_Surface *Fntfile::getCharacter(Uint32 indexOfFile)
{
	SDL_Surface *pic = NULL;

	if(indexOfFile >= numCharacters) {
		return NULL;
	}
	// create new picture surface
	if((pic = SDL_CreateRGBSurface(0,character[indexOfFile].width,characterHeight,8,0,0,0,0))== NULL) {
		return NULL;
	}

    palette.applyToSurface(pic);
	SDL_LockSurface(pic);

	//Now we can copy pixel by pixel
	for(int y = 0; y < characterHeight; y++) {
		for(int x = 0; x < character[indexOfFile].width; x++) {
			if((character[indexOfFile].data[y*character[indexOfFile].width+x] == 1)
				|| ((character[indexOfFile].data[y*character[indexOfFile].width+x] > 3)
				&& (character[indexOfFile].data[y*character[indexOfFile].width+x] < 0x0E))) {
				*(((unsigned char*) pic->pixels) + y*pic->pitch + x) = 255;
			} else {
				*(((unsigned char*) pic->pixels) + y*pic->pitch + x) = 0;
			}
			//*(((char*) pic->pixels) + y*pic->pitch + x) = Character[indexOfFile].data[y*Character[indexOfFile].width+x];

		}
	}

	SDL_UnlockSurface(pic);
/*	DEBUG:
	for(int y = 0; y < CharacterHeight; y++) {
		for(int x = 0; x < Character[indexOfFile].width; x++) {
			if(Character[indexOfFile].data[y*Character[indexOfFile].width+x] == 0) {
				printf(" ");
			} else {
				printf("%X",Character[indexOfFile].data[y*Character[indexOfFile].width+x]);
			}
		}
		printf("\n");
	}
	printf("\n");
*/
	return pic;
}



void Fntfile::drawTextOnSurface(SDL_Surface* pSurface, std::string text, Uint32 baseColor) {
	SDL_LockSurface(pSurface);

	int curXPos = 0;
	const unsigned char* pText = (unsigned char*) text.c_str();
	while(*pText != '\0') {
		int index = 0;
		if(*pText < numCharacters) {
			index = *pText;
		}

		//Now we can copy pixel by pixel
		for(int y = 0; y < characterHeight; y++) {
			for(int x = 0; x < character[index].width; x++) {
				/*
				// old way of gettting color out of the font
				if((Character[index].data[y*Character[index].width+x] == 1)
				|| ((Character[index].data[y*Character[index].width+x] > 3)
				&& (Character[index].data[y*Character[index].width+x] < 0x0E))) {
					*(((char*) pSurface->pixels) + y*pSurface->pitch + x + curXPos) = color;
				}
				*/

				// new way
				char color = character[index].data[y*character[index].width+x];
				if(color > 7) {
					color = 15 - color;
				}

				if(color != 0) {
					*(((char*) pSurface->pixels) + y*pSurface->pitch + x + curXPos) = baseColor + (color-1);
				}

			}
		}

		curXPos += character[index].width;
		pText++;
	}


	SDL_UnlockSurface(pSurface);
}

/// Returns the number of pixels a text needs
/**
		This methods returns the number of pixels this text would need if printed.
		\param	text	The text to be checked for it's length in pixel
		\return Number of pixels needed
*/
int	Fntfile::getTextWidth(std::string text) const {
	int width = 0;
	const unsigned char* pText = (unsigned char*) text.c_str();
	while(*pText != '\0') {
		if(*pText >= numCharacters) {
			width += character[0].width;
		} else {
			width += character[*pText].width;
		}
		pText++;
	}

	return width;
}

void Fntfile::readfile(unsigned char* pFiledata, Uint16 filesize) {
	// a valid fnt-file must be at least 20 bytes big
	if(filesize < 20) {
		fprintf(stderr,"Fntfile::readfile(): A valid fnt-file must be at least 20 bytes long!\n");
		exit(EXIT_FAILURE);
	}

	Uint16 size = SDL_SwapLE16(((Uint16*) pFiledata)[0]);
	Uint16 magic1 = SDL_SwapLE16(((Uint16*) pFiledata)[1]);
	Uint16 magic2 = SDL_SwapLE16(((Uint16*) pFiledata)[2]);
	Uint16 magic3 = SDL_SwapLE16(((Uint16*) pFiledata)[7]);

	if((size != filesize) || (magic1 != 0x0500)  || (magic2 != 0x000E) || (magic3 != 0x1012)) {
		fprintf(stderr,"Fntfile::readfile(): Invalid filesize or magic number!\n");
		exit(EXIT_FAILURE);
	}

	if(		(SDL_SwapLE16(((Uint16*) pFiledata)[4]) >= filesize)
		||	(SDL_SwapLE16(((Uint16*) pFiledata)[5]) >= filesize)
		||	(SDL_SwapLE16(((Uint16*) pFiledata)[6]) >= filesize)) {
		fprintf(stderr,"Fntfile::readfile(): Invalid data positions!\n");
		exit(EXIT_FAILURE);
	}

	Uint16* chars = (Uint16*) (pFiledata + SDL_SwapLE16(((Uint16*) pFiledata)[3]));
	Uint8* widths = pFiledata + SDL_SwapLE16(((Uint16*) pFiledata)[4]);
	//unsigned char* graphics = pFiledata + SDL_SwapLE16(((Uint16*) pFiledata)[5]);
	Uint16* heights = (Uint16*) (pFiledata + SDL_SwapLE16(((Uint16*) pFiledata)[6]));

	// pFiledata[16] might be the first character index and pFiledata[17] the last index,
	// but I don't know for sure.
	numCharacters =  pFiledata[17] + 1;
	characterHeight = pFiledata[18];
	maxCharacterWidth = pFiledata[19];

	// Check if every array is in the file
	if(	( (unsigned char*) (chars+numCharacters) > (pFiledata+filesize))
		|| ( (unsigned char*) (widths+numCharacters) > (pFiledata+filesize))
		|| ( (unsigned char*) (heights+numCharacters) > (pFiledata+filesize))) {
		fprintf(stderr,"Fntfile::readfile(): Invalid fnt-File. File too small and cannot contain all characters!\n");
		exit(EXIT_FAILURE);
	}

	if((character = (FntfileCharacter*) calloc(sizeof(FntfileCharacter) * numCharacters,1)) == NULL) {
		fprintf(stderr,"Fntfile::readfile(): Cannot allocate memory!\n");
		exit(EXIT_FAILURE);
	}

	for(Uint32 i = 0; i < numCharacters; i++) {
		character[i].width = widths[i];

		if((character[i].data = (unsigned char*) calloc(character[i].width*characterHeight,1)) == NULL) {
			fprintf(stderr,"Fntfile::readfile(): Cannot allocate memory!\n");
			exit(EXIT_FAILURE);
		}

		unsigned char* chardata = pFiledata+SDL_SwapLE16(chars[i]);
		int charHeight = (SDL_SwapLE16(heights[i])) >> 8;
		int yPos = (SDL_SwapLE16(heights[i])) & 0x00FF;

		if(chardata + charHeight*((character[i].width+1)/2) > (pFiledata+filesize)) {
			fprintf(stderr,"Fntfile::readfile(): Invalid character graphic. The graphic goes beyond end of file!\n");
			exit(EXIT_FAILURE);
		}

		if(yPos+charHeight > characterHeight) {
			fprintf(stderr,"Fntfile::readfile(): Invalid character graphic. The character height is bigger than the font height!\n");
			exit(EXIT_FAILURE);
		}

		int j=0;
		// now we can copy the data
		for(int y = yPos; y < yPos+charHeight; y++) {
			for(int x = 0; x < character[i].width; x+=2) {
				character[i].data[y*character[i].width + x] = chardata[j] & 0x0F;
				if(x+1 < character[i].width) {
					character[i].data[y*character[i].width + x + 1] = chardata[j] >> 4;
				}
				j++;
			}
		}
	}
}
