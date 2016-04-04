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

#include <FileClasses/SaveWAV.h>

int SaveWAV(SDL_RWops* rwop, Mix_Chunk* chunk, int Frequency, Uint16 Format, int channels) {
	if(rwop == nullptr) {
		return -1;
	}

	if((Format != AUDIO_S16LSB) && (Format != AUDIO_S16MSB) && (Format != AUDIO_U16LSB) && (Format != AUDIO_U16MSB)) {
		return -1;
	}

	struct T_RIFFHeader {
		char RIFFSignature[4];
		Uint32 length;
		char RIFFType[4];
	} RIFFHeader;

	struct T_FormatChunk {
		char	chunkID[4];
		Sint32	chunkSize;
		Sint16	wFormatTag;
		Uint16	wChannels;
		Uint32	dwSamplesPerSec;
		Uint32	dwAvgBytesPerSec;
		Uint16	wBlockAlign;
		Uint16	wBitsPerSample;
	} FormatChunk;

	struct T_DataChunk {
		char	chunkID[4];
		Sint32	chunkSize;
	} DataChunk;

	RIFFHeader.RIFFSignature[0] = 'R';
	RIFFHeader.RIFFSignature[1] = 'I';
	RIFFHeader.RIFFSignature[2] = 'F';
	RIFFHeader.RIFFSignature[3] = 'F';
	RIFFHeader.length = SDL_SwapLE32((Uint32) (4 + 24 + 8 + chunk->alen));
	RIFFHeader.RIFFType[0] = 'W';
        RIFFHeader.RIFFType[1] = 'A';
        RIFFHeader.RIFFType[2] = 'V';
        RIFFHeader.RIFFType[3] = 'E';

	FormatChunk.chunkID[0] = 'f';
	FormatChunk.chunkID[1] = 'm';
	FormatChunk.chunkID[2] = 't';
	FormatChunk.chunkID[3] = ' ';
	FormatChunk.chunkSize = SDL_SwapLE32((Uint32) 16);
	FormatChunk.wFormatTag = SDL_SwapLE16((Uint16) 1);
	FormatChunk.wChannels = SDL_SwapLE16( (Uint16) channels);
	FormatChunk.dwSamplesPerSec = SDL_SwapLE32( (Uint32) Frequency);
	FormatChunk.dwAvgBytesPerSec = SDL_SwapLE32((Uint32) (sizeof(Sint16) * Frequency * channels));
	FormatChunk.wBlockAlign = SDL_SwapLE16( (Uint16) (sizeof(Sint16)*channels) );
	FormatChunk.wBitsPerSample = SDL_SwapLE16( (Uint16) 16);

	DataChunk.chunkID[0] = 'd';
	DataChunk.chunkID[1] = 'a';
	DataChunk.chunkID[2] = 't';
	DataChunk.chunkID[3] = 'a';
	DataChunk.chunkSize = SDL_SwapLE32((Uint32) chunk->alen);

	// write out
	SDL_RWwrite(rwop,RIFFHeader.RIFFSignature, 1, 4);
	SDL_RWwrite(rwop,&RIFFHeader.length, sizeof(Uint32),1);
	SDL_RWwrite(rwop,RIFFHeader.RIFFType, 1, 4);

	SDL_RWwrite(rwop,FormatChunk.chunkID, 1, 4);
	SDL_RWwrite(rwop,&FormatChunk.chunkSize, sizeof(Uint32),1);
	SDL_RWwrite(rwop,&FormatChunk.wFormatTag, sizeof(Uint16),1);
	SDL_RWwrite(rwop,&FormatChunk.wChannels, sizeof(Uint16),1);
	SDL_RWwrite(rwop,&FormatChunk.dwSamplesPerSec, sizeof(Uint32),1);
	SDL_RWwrite(rwop,&FormatChunk.dwAvgBytesPerSec, sizeof(Uint32),1);
	SDL_RWwrite(rwop,&FormatChunk.wBlockAlign, sizeof(Uint16),1);
	SDL_RWwrite(rwop,&FormatChunk.wBitsPerSample, sizeof(Uint16),1);

	SDL_RWwrite(rwop,DataChunk.chunkID, 1, 4);
	SDL_RWwrite(rwop,&DataChunk.chunkSize, sizeof(Uint32),1);

	switch(Format) {
		case AUDIO_U16LSB:
		{
			for(unsigned int i=0; i < chunk->alen / 2 ; i++) {
				int Sample = SDL_SwapLE16(((Uint16*) chunk->abuf)[i]);
				Sample -= 32768;
				Sint16 Sint16Sample = SDL_SwapLE16( (Sint16) (Sample));
				if(SDL_RWwrite(rwop,&Sint16Sample, sizeof(Sint16),1) != 1) {
					return -1;
				}
			}
		} break;

		case AUDIO_U16MSB:
		{
			for(unsigned int i=0; i < chunk->alen / 2 ; i++) {
				int Sample = SDL_Swap16(((Uint16*) chunk->abuf)[i]);
				Sample -= 32768;
				Sint16 Sint16Sample = SDL_Swap16( (Sint16) (Sample));
				if(SDL_RWwrite(rwop,&Sint16Sample, sizeof(Sint16),1) != 1) {
					return -1;
				}
			}
		} break;

		case AUDIO_S16LSB:
		{
			if(SDL_RWwrite(rwop,chunk->abuf, chunk->alen,1) != 1) {
					return -1;
			}
		} break;

		case AUDIO_S16MSB:
		{
			for(unsigned int i=0; i < chunk->alen / 2 ; i++) {
				Sint16 Sint16Sample = ((Sint16*) chunk->abuf)[i];
				Sint16Sample = SDL_Swap16(Sint16Sample);
				if(SDL_RWwrite(rwop,&Sint16Sample, sizeof(Sint16),1) != 1) {
					return -1;
				}
			}
		} break;

		default:
		{
			return -1;
		} break;
	}

	return 0;
}

int SaveWAV(SDL_RWops* rwop,Mix_Chunk* chunk) {
	// Get audio device specifications
	int Frequency, channels;
	Uint16 Format;
	if(Mix_QuerySpec(&Frequency, &Format, &channels) == 0) {
		return -1;
	}

	if((Format != AUDIO_S16LSB) && (Format != AUDIO_S16MSB) && (Format != AUDIO_U16LSB) && (Format != AUDIO_U16MSB)) {
		return -1;
	}

	return SaveWAV(rwop, chunk, Frequency, Format, channels);
}
