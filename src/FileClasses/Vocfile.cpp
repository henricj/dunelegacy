/* This code is based on code from the ScummVM project
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 *
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



#include <FileClasses/Vocfile.h>

#include <string>
#include <SDL2/SDL_mixer.h>
#include <stdlib.h>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <cerrno>

#define VOC_CODE_TERM       0
#define VOC_CODE_DATA       1
#define VOC_CODE_CONT       2
#define VOC_CODE_SILENCE    3
#define VOC_CODE_MARKER     4
#define VOC_CODE_TEXT       5
#define VOC_CODE_LOOPBEGIN  6
#define VOC_CODE_LOOPEND    7
#define VOC_CODE_EXTENDED   8
#define VOC_CODE_DATA_16    9


#define NUM_SAMPLES_OF_SILENCE  160

/**
 * Take a sample rate parameter as it occurs in a VOC sound header, and
 * return the corresponding sample frequency.
 *
 * This method has special cases for the standard rates of 11025 and 22050 kHz,
 * which due to limitations of the format, cannot be encoded exactly in a VOC
 * file. As a consequence, many game files have sound data sampled with those
 * rates, but the VOC marks them incorrectly as 11111 or 22222 kHz. This code
 * works around that and "unrounds" the sampling rates.
 */
static Uint32 getSampleRateFromVOCRate(Uint8 vocSR) {
    if (vocSR == 0xa5 || vocSR == 0xa6) {
        return 11025;
    } else if (vocSR == 0xd2 || vocSR == 0xd3) {
        return 22050;
    } else {
        int sr = 1000000L / (256L - vocSR);
        // inexact sampling rates occur e.g. in the kitchen in Monkey Island,
        // very easy to reach right from the start of the game.
        //warning("inexact sample rate used: %i (0x%x)", sr, vocSR);
        return sr;
    }
}

/**
    This method decodes a voc-file and returns a pointer the decoded data. The size of the
    decoded data is returned through the parameter size and the sampling rate of this voc-file
    is returned through the parameter rate.
    The kind of voc-files that this function can decode is very restricted. Only voc-files
    with 8-bit unsigned sound samples, with no loops, no silence blocks, no extended blocks
    and no markers are supported.
    \param  rwop    An SDL_RWop that contains the voc-file
    \param  decsize The size of the decoded data in bytes
    \param  rate    The sampling rate of the voc-file
    \return A pointer to a memory block that contains the data.
*/
static sdl2::sdl_ptr<Uint8[]> LoadVOC_RW(SDL_RWops* rwop, Uint32 &decsize, Uint32 &rate) {
    Uint8 description[20];
    Uint16 offset;
    Uint16 version;
    Uint16 id;

    if(SDL_RWread(rwop,(char*) description,20,1) != 1) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid header!");
    }

    if (memcmp(description, "Creative Voice File", 19) != 0) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid header!");
    }

    if (description[19] != 0x1A) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid header!");
    }

    if(SDL_RWread(rwop,&offset,sizeof(offset),1) != 1) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid header!");
    }
    offset = SDL_SwapLE16(offset);

    if(SDL_RWread(rwop,&version,sizeof(version),1) != 1) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid header!");
    }
    version = SDL_SwapLE16(version);

    if(SDL_RWread(rwop,&id,sizeof(id),1) != 1) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid header!");
    }
    id = SDL_SwapLE16(id);

    if(offset != sizeof(description) + sizeof(offset) + sizeof(version) + sizeof(id)) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid datablock offset in header!");
    }

    // 0x100 is an invalid VOC version used by German version of DOTT (Disk) and
    // French version of Simon the Sorcerer 2 (CD)
    if(!(version == 0x010A || version == 0x0114 || version == 0x0100)) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid version (0x%X) in header!", version);
    }

    if(id != (~version + 0x1234)) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid id in header!");
    }

    sdl2::sdl_ptr<Uint8[]> ret_sound = nullptr;
    decsize = 0;

    Uint8 code;
    rate = 0;
    while (SDL_RWread(rwop,&code,sizeof(Uint8),1) == 1) {
        if(code == VOC_CODE_TERM) {
            return ret_sound;
        }

        Uint8 tmp[3];
        if(SDL_RWread(rwop,tmp,1,3) != 3) {
            THROW(std::runtime_error, "LoadVOC_RW(): Invalid block length!");
        }
        size_t len = tmp[0];
        len |= tmp[1] << 8;
        len |= tmp[2] << 16;

        switch (code) {
            case VOC_CODE_DATA: {
                Uint8 time_constant;
                if(SDL_RWread(rwop,&time_constant,sizeof(Uint8),1) != 1) {
                    THROW(std::runtime_error, "LoadVOC_RW(): Cannot read time constant!");
                }

                Uint8 packing;
                if(SDL_RWread(rwop,&packing,sizeof(Uint8),1) != 1) {
                    THROW(std::runtime_error, "LoadVOC_RW(): Cannot read packing!");
                }
                len -= 2;
                Uint32 tmp_rate = getSampleRateFromVOCRate(time_constant);
                if((rate != 0) && (rate != tmp_rate)) {
                    SDL_Log("This voc-file contains data blocks with different sampling rates: old rate: %d, new rate: %d",rate,tmp_rate);
                }
                rate = tmp_rate;

                //SDL_Log("VOC Data Block: Rate: %d, Packing: %d, Length: %d", rate, packing, len);

                if (packing == 0) {
                    Uint8* tmp_ret_sound = (Uint8 *) SDL_realloc(ret_sound.get(), decsize + len);
                    if(tmp_ret_sound == nullptr) {
                        THROW(std::runtime_error, "LoadVOC_RW(): %s", strerror(errno));
                    } else {
                        ret_sound.release();
                        ret_sound = sdl2::sdl_ptr<Uint8[]> { tmp_ret_sound };
                    }

                    if(SDL_RWread(rwop,ret_sound.get() + decsize,1,len) != len) {
                        THROW(std::runtime_error, "LoadVOC_RW(): Cannot read data!");
                    }

                    decsize += len;
                } else {
                    THROW(std::runtime_error, "LoadVOC_RW(): VOC file packing %d unsupported!", packing);
                }
            } break;

            case VOC_CODE_SILENCE: {
                Uint16 SilenceLength;
                if(SDL_RWread(rwop,&SilenceLength,sizeof(Uint16),1) != 1) {
                    THROW(std::runtime_error, "LoadVOC_RW(): Cannot read silence length!");
                }
                SilenceLength = SDL_SwapLE16(SilenceLength);

                Uint8 time_constant;
                if(SDL_RWread(rwop,&time_constant,sizeof(Uint8),1) != 1) {
                    THROW(std::runtime_error, "LoadVOC_RW(): Cannot read time constant!");
                }

                Uint32 SilenceRate = getSampleRateFromVOCRate(time_constant);


                Uint32 length = 0;
                if(rate != 0) {
                    length = (Uint32) ((((double) SilenceRate)/((double) rate)) * SilenceLength) + 1;
                } else {
                    SDL_Log("LoadVOC_RW(): The silence in this voc-file is right at the beginning. Therefore it is not possible to adjust the silence sample rate to the sample rate of the other sound data in this file!");
                    length = SilenceLength;
                }

                Uint8* tmp_ret_sound = (Uint8 *) SDL_realloc(ret_sound.get(), decsize + length);
                if(tmp_ret_sound == nullptr) {
                    THROW(std::runtime_error, "LoadVOC_RW(): %s", strerror(errno));
                } else {
                    ret_sound.release();
                    ret_sound = sdl2::sdl_ptr<Uint8[]> { tmp_ret_sound };
                }

                memset(ret_sound.get() + decsize,0x80,length);

                decsize += length;

            } break;

            case VOC_CODE_CONT:
            case VOC_CODE_MARKER:
            case VOC_CODE_TEXT:
            case VOC_CODE_LOOPBEGIN:
            case VOC_CODE_LOOPEND:
            case VOC_CODE_EXTENDED:
            case VOC_CODE_DATA_16:
            default:
                THROW(std::runtime_error, "LoadVOC_RW(): Unsupported code in VOC file : %d", code);
        }
    }
    return ret_sound;
}

inline Uint8 Float2Uint8(float x) {
    int val = lround(x*127.0f + 128.0f);
    if(val < 0) {
        val = 0;
    } else if(val > 255) {
        val = 255;
    }

    return (Uint8) val;
}

inline Sint8 Float2Sint8(float x) {
    int val = lround(x*127.0f);
    if(val < -128) {
        val = -128;
    } else if(val > 127) {
        val = 127;
    }

    return (Sint8) val;
}

inline Uint16 Float2Uint16(float x) {
    int val = lround(x*32767.0f + 32768.0f);
    if(val < 0) {
        val = 0;
    } else if(val > 65535) {
        val = 65535;
    }

    return (Uint16) val;
}

inline Sint16 Float2Sint16(float x) {
    int val = lround(x*32767.0f);
    if(val < -32768) {
        val = -32768;
    } else if(val > 32767) {
        val = 32767;
    }

    return (Sint16) val;
}

sdl2::mix_chunk_ptr LoadVOC_RW(SDL_RWops* rwop) {

    if(rwop == nullptr) {
        THROW(std::invalid_argument, "LoadVOC_RW(): rwop == nullptr!");
    }

    // Read voc file
    Uint32 RawData_Frequency;
    Uint32 RawData_Samples;
    sdl2::sdl_ptr<Uint8[]> RawDataUint8 = LoadVOC_RW(rwop, RawData_Samples, RawData_Frequency);
    if(RawDataUint8 == nullptr) {
        THROW(std::runtime_error, "LoadVOC_RW(): Cannot read raw data!");
    }

    // shift level so that the last sample is 128
    int minValue = 255;
    int maxValue = 0;
    for(Uint32 i=0; i < RawData_Samples; i++) {
        if(RawDataUint8[i] < minValue) {
            minValue = RawDataUint8[i];
        }
        if(RawDataUint8[i] > maxValue) {
            maxValue = RawDataUint8[i];
        }
    }

    int levelShift = 128 - (int) RawDataUint8[RawData_Samples-1];
    if(minValue + levelShift < 0) {
        levelShift = -minValue;
    } else if(maxValue + levelShift > 255) {
        levelShift = (255-maxValue);
    }

    for(Uint32 i=0; i < RawData_Samples; i++) {
        RawDataUint8[i] = (Uint8) (RawDataUint8[i] + levelShift);
    }

    // Convert to floats
    std::vector<float> RawDataFloat(RawData_Samples+2*NUM_SAMPLES_OF_SILENCE);

    for(Uint32 i=0; i < NUM_SAMPLES_OF_SILENCE; i++) {
        RawDataFloat[i] = 0.0;
    }

    for(Uint32 i=NUM_SAMPLES_OF_SILENCE; i < RawData_Samples+NUM_SAMPLES_OF_SILENCE; i++) {
        RawDataFloat[i] = (((float) RawDataUint8[i-NUM_SAMPLES_OF_SILENCE])/128.0f) - 1.0f;
    }

    for(Uint32 i=RawData_Samples+NUM_SAMPLES_OF_SILENCE; i < RawData_Samples + 2*NUM_SAMPLES_OF_SILENCE; i++) {
        RawDataFloat[i] = 0.0;
    }

    RawDataUint8.reset();

    RawData_Samples += 2*NUM_SAMPLES_OF_SILENCE;

    // To prevent strange invalid read in src_linear
    RawData_Samples--;

    // Get audio device specifications
    int TargetFrequency, channels;
    Uint16 TargetFormat;
    if(Mix_QuerySpec(&TargetFrequency, &TargetFormat, &channels) == 0) {
        THROW(std::runtime_error, "LoadVOC_RW(): Mix_QuerySpec failed!");
    }

    // Convert to audio device frequency
    float ConversionRatio = ((float) TargetFrequency) / ((float) RawData_Frequency);
    Uint32 TargetDataFloat_Samples = (Uint32) ((float) RawData_Samples * ConversionRatio);
    std::vector<float> TargetDataFloat(TargetDataFloat_Samples);

    for(Uint32 x=0;x<TargetDataFloat_Samples;x++) {
        float pos = x/ConversionRatio;
        int i = (int) pos; //lrint(floor(pos));
        TargetDataFloat[x] = RawDataFloat[i] * ((i+1)-pos) + RawDataFloat[i+1] * (pos-i);
    }

    Uint32 TargetData_Samples = TargetDataFloat_Samples;

    RawDataFloat.clear();


    // Equalize if neccessary
    float distance = 0.0f;
    for(Uint32 i=0; i < TargetData_Samples; i++) {
        if(std::abs(TargetDataFloat[i]) > distance) {
            distance = std::abs(TargetDataFloat[i]);
        }
    }

    if(distance > 1.0f) {
        //Equalize
        for(Uint32 i=0; i < TargetData_Samples; i++) {
            TargetDataFloat[i] = TargetDataFloat[i] / distance;
        }
    }


    // Convert floats back to integers but leave out 3/4 of silence
    int ThreeQuaterSilenceLength = (int) ((NUM_SAMPLES_OF_SILENCE * ConversionRatio)*(3.0f/4.0f));
    TargetData_Samples -= 2*ThreeQuaterSilenceLength;

    auto myChunk = sdl2::mix_chunk_ptr{ (Mix_Chunk*) SDL_calloc(sizeof(Mix_Chunk),1) };
    if(myChunk == nullptr) {
        throw std::bad_alloc();
    }

    myChunk->allocated = 1;
    myChunk->volume = 128;

    int SizeOfTargetSample;
    switch(TargetFormat) {
        case AUDIO_U8:      SizeOfTargetSample = sizeof(Uint8) * channels;      break;
        case AUDIO_S8:      SizeOfTargetSample = sizeof(Sint8) * channels;      break;
        case AUDIO_U16LSB:  SizeOfTargetSample = sizeof(Uint16) * channels;     break;
        case AUDIO_S16LSB:  SizeOfTargetSample = sizeof(Sint16) * channels;     break;
        case AUDIO_U16MSB:  SizeOfTargetSample = sizeof(Uint16) * channels;     break;
        case AUDIO_S16MSB:  SizeOfTargetSample = sizeof(Sint16) * channels;     break;
        default: {
            THROW(std::runtime_error, "LoadVOC_RW(): Invalid target sample format!");
        } break;
    }

    if((myChunk->abuf = (Uint8*) SDL_malloc(TargetData_Samples * SizeOfTargetSample)) == nullptr) {
        throw std::bad_alloc();
    }
    myChunk->alen = TargetData_Samples * SizeOfTargetSample;

    switch(TargetFormat) {
        case AUDIO_U8: {
            Uint8* TargetData = reinterpret_cast<Uint8*>(myChunk->abuf);
            for(Uint32 i=0; i < TargetData_Samples*channels; i+=channels) {
                TargetData[i] = Float2Uint8(TargetDataFloat[(i/channels)+ThreeQuaterSilenceLength]);
                for(int j = 1; j < channels; j++) {
                    TargetData[i+j] = TargetData[i];
                }
            }
        } break;

        case AUDIO_S8: {
            Sint8* TargetData = reinterpret_cast<Sint8*>(myChunk->abuf);
            for(Uint32 i=0; i < TargetData_Samples*channels; i+=channels) {
                TargetData[i] = Float2Sint8(TargetDataFloat[(i/channels)+ThreeQuaterSilenceLength]);
                for(int j = 1; j < channels; j++) {
                    TargetData[i+j] = TargetData[i];
                }
            }
        } break;

        case AUDIO_U16LSB: {
            Uint16* TargetData = reinterpret_cast<Uint16*>(myChunk->abuf);
            for(Uint32 i=0; i < TargetData_Samples*channels; i+=channels) {
                TargetData[i] = SDL_SwapLE16(Float2Uint16(TargetDataFloat[(i/channels)+ThreeQuaterSilenceLength]));
                for(int j = 1; j < channels; j++) {
                    TargetData[i+j] = TargetData[i];
                }

            }
        } break;

        case AUDIO_S16LSB: {
            Sint16* TargetData = reinterpret_cast<Sint16*>(myChunk->abuf);
            for(Uint32 i=0; i < TargetData_Samples*channels; i+=channels) {
                TargetData[i] = SDL_SwapLE16(Float2Sint16(TargetDataFloat[(i/channels)+ThreeQuaterSilenceLength]));
                for(int j = 1; j < channels; j++) {
                    TargetData[i+j] = TargetData[i];
                }
            }
        } break;

        case AUDIO_U16MSB: {
            Uint16* TargetData = reinterpret_cast<Uint16*>(myChunk->abuf);
            for(Uint32 i=0; i < TargetData_Samples*channels; i+=channels) {
                TargetData[i] = SDL_SwapBE16(Float2Uint16(TargetDataFloat[(i/channels)+ThreeQuaterSilenceLength]));
                for(int j = 1; j < channels; j++) {
                    TargetData[i+j] = TargetData[i];
                }

            }
        } break;

        case AUDIO_S16MSB: {
            Sint16* TargetData = reinterpret_cast<Sint16*>(myChunk->abuf);
            for(Uint32 i=0; i < TargetData_Samples*channels; i+=channels) {
                TargetData[i] = SDL_SwapBE16(Float2Sint16(TargetDataFloat[(i/channels)+ThreeQuaterSilenceLength]));
                for(int j = 1; j < channels; j++) {
                    TargetData[i+j] = TargetData[i];
                }
            }
        } break;

        default: {
            THROW(std::runtime_error, "LoadVOC_RW(): Invalid target sample format!");
        } break;
    }

    return myChunk;
}
