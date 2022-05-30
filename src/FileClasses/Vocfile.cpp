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

#include "misc/BufferedReader.h"
#include "misc/string_error.h"
#include <misc/SDL2pp.h>

#include <SDL2/SDL_mixer.h>

#include <soxr.h>

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {
inline constexpr auto VOC_CODE_TERM      = 0;
inline constexpr auto VOC_CODE_DATA      = 1;
inline constexpr auto VOC_CODE_CONT      = 2;
inline constexpr auto VOC_CODE_SILENCE   = 3;
inline constexpr auto VOC_CODE_MARKER    = 4;
inline constexpr auto VOC_CODE_TEXT      = 5;
inline constexpr auto VOC_CODE_LOOPBEGIN = 6;
inline constexpr auto VOC_CODE_LOOPEND   = 7;
inline constexpr auto VOC_CODE_EXTENDED  = 8;
inline constexpr auto VOC_CODE_DATA_16   = 9;

inline constexpr auto NUM_SAMPLES_OF_SILENCE = 160;

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
uint32_t getSampleRateFromVOCRate(uint8_t vocSR) {
    if (vocSR == 0xa5 || vocSR == 0xa6) {
        return 11025;
    }
    if (vocSR == 0xd2 || vocSR == 0xd3) {

        return 22050;
    }
    const auto sr = 1000000L / (256L - vocSR);

    // inexact sampling rates occur e.g. in the kitchen in Monkey Island,

    // very easy to reach right from the start of the game.

    // warning("inexact sample rate used: %i (0x%x)", sr, vocSR);

    return sr;
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
sdl2::sdl_ptr<uint8_t[]> LoadVOC_RW(SDL_RWops* rwop, uint32_t& decsize, uint32_t& rate) {
    using namespace std::literals;

    BufferedReader<> buffer{rwop};

    static constexpr auto creative_voice_file = "Creative Voice File"sv;

    std::array<uint8_t, creative_voice_file.size() + 1> description;

    uint16_t offset  = 0;
    uint16_t version = 0;
    uint16_t id      = 0;

    if (!buffer.read_one(description.data(), description.size())) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid header!");
    }

    if (0 != memcmp(description.data(), creative_voice_file.data(), creative_voice_file.size())) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid header!");
    }

    if (description[19] != 0x1A) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid header!");
    }

    if (!buffer.read_type(offset)) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid header!");
    }
    offset = SDL_SwapLE16(offset);

    if (!buffer.read_type(version)) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid header!");
    }
    version = SDL_SwapLE16(version);

    if (!buffer.read_type(id)) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid header!");
    }
    id = SDL_SwapLE16(id);

    if (offset != sizeof description + sizeof offset + sizeof version + sizeof id) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid datablock offset in header!");
    }

    // 0x100 is an invalid VOC version used by German version of DOTT (Disk) and
    // French version of Simon the Sorcerer 2 (CD)
    if (!(version == 0x010A || version == 0x0114 || version == 0x0100)) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid version (0x%X) in header!", version);
    }

    if (id != ~version + 0x1234) {
        THROW(std::runtime_error, "LoadVOC_RW(): Invalid id in header!");
    }

    sdl2::sdl_ptr<uint8_t[]> ret_sound;

    decsize = 0;

    uint8_t code = 0;
    rate         = 0;

    while (buffer.read_type(code)) {
        if (code == VOC_CODE_TERM) {
            return ret_sound;
        }

        uint8_t tmp[3];
        if (!buffer.read_one(tmp, sizeof tmp)) {
            THROW(std::runtime_error, "LoadVOC_RW(): Invalid block length!");
        }
        size_t len = tmp[0];
        len |= static_cast<uint32_t>(tmp[1]) << 8U;
        len |= static_cast<uint32_t>(tmp[2]) << 16U;

        switch (code) {
            case VOC_CODE_DATA: {
                uint8_t time_constant = 0;
                if (!buffer.read_type(time_constant)) {
                    THROW(std::runtime_error, "LoadVOC_RW(): Cannot read time constant!");
                }

                uint8_t packing = 0;
                if (!buffer.read_type(packing)) {
                    THROW(std::runtime_error, "LoadVOC_RW(): Cannot read packing!");
                }
                len -= 2;
                const auto tmp_rate = getSampleRateFromVOCRate(time_constant);
                if (rate != 0 && rate != tmp_rate) {
                    sdl2::log_info(
                        "This voc-file contains data blocks with different sampling rates: old rate: %d, new rate: %d",
                        rate, tmp_rate);
                }
                rate = tmp_rate;

                // sdl2::log_info("VOC Data Block: Rate: %d, Packing: %d, Length: %d", rate, packing, len);

                if (packing == 0) {
                    auto* tmp_ret_sound = static_cast<uint8_t*>(SDL_realloc(ret_sound.get(), decsize + len));
                    if (tmp_ret_sound == nullptr) {
                        THROW(std::runtime_error, "LoadVOC_RW(): %s", dune::string_error(errno));
                    }
                    ret_sound.release();
                    ret_sound.reset(tmp_ret_sound);

                    if (!buffer.read_one(ret_sound.get() + decsize, len)) {
                        THROW(std::runtime_error, "LoadVOC_RW(): Cannot read data!");
                    }

                    decsize += len;
                } else {
                    THROW(std::runtime_error, "LoadVOC_RW(): VOC file packing %d unsupported!", packing);
                }
            } break;

            case VOC_CODE_SILENCE: {
                uint16_t SilenceLength = 0;
                if (!buffer.read_type(SilenceLength)) {
                    THROW(std::runtime_error, "LoadVOC_RW(): Cannot read silence length!");
                }
                SilenceLength = SDL_SwapLE16(SilenceLength);

                uint8_t time_constant = 0;
                if (!buffer.read_type(time_constant)) {
                    THROW(std::runtime_error, "LoadVOC_RW(): Cannot read time constant!");
                }

                const auto SilenceRate = getSampleRateFromVOCRate(time_constant);

                uint32_t length = 0;
                if (rate != 0) {
                    length = static_cast<uint32_t>(static_cast<double>(SilenceRate) / static_cast<double>(rate)
                                                   * SilenceLength)
                           + 1;
                } else {
                    sdl2::log_info("LoadVOC_RW(): The silence in this voc-file is right at the beginning. Therefore it "
                                   "is not possible to adjust the silence sample rate to the sample rate of the other "
                                   "sound data in this file!");
                    length = SilenceLength;
                }

                auto* tmp_ret_sound = static_cast<uint8_t*>(SDL_realloc(ret_sound.get(), decsize + length));
                if (tmp_ret_sound == nullptr) {
                    THROW(std::runtime_error, "LoadVOC_RW(): %s", dune::string_error(errno));
                }
                ret_sound.release();
                ret_sound.reset(tmp_ret_sound);

                memset(ret_sound.get() + decsize, 0x80, length);

                decsize += length;

            } break;

            case VOC_CODE_CONT:
            case VOC_CODE_MARKER:
            case VOC_CODE_TEXT:
            case VOC_CODE_LOOPBEGIN:
            case VOC_CODE_LOOPEND:
            case VOC_CODE_EXTENDED:
            case VOC_CODE_DATA_16:
            default: THROW(std::runtime_error, "LoadVOC_RW(): Unsupported code in VOC file : %d", code);
        }
    }
    return ret_sound;
}

uint8_t Float2Uint8(float x) {
    int val = lround(x * 127.0f + 128.0f);
    if (val < 0) {
        val = 0;
    } else if (val > 255) {
        val = 255;
    }

    return static_cast<uint8_t>(val);
}

int8_t Float2Sint8(float x) {
    int val = lround(x * 127.0f);
    if (val < -128) {
        val = -128;
    } else if (val > 127) {
        val = 127;
    }

    return static_cast<int8_t>(val);
}

uint16_t Float2Uint16(float x) {
    int val = lround(x * 32767.0f + 32768.0f);
    if (val < 0) {
        val = 0;
    } else if (val > 65535) {
        val = 65535;
    }

    return static_cast<uint16_t>(val);
}

int16_t Float2Sint16(float x) {
    int val = lround(x * 32767.0f);
    if (val < -32768) {
        val = -32768;
    } else if (val > 32767) {
        val = 32767;
    }

    return static_cast<int16_t>(val);
}

} // namespace

sdl2::mix_chunk_ptr LoadVOC_RW(SDL_RWops* rwop) {

    if (rwop == nullptr) {
        THROW(std::invalid_argument, "LoadVOC_RW(): rwop == nullptr!");
    }

    // Read voc file
    uint32_t RawData_Frequency            = 0;
    uint32_t RawData_Samples              = 0;
    sdl2::sdl_ptr<uint8_t[]> RawDataUint8 = LoadVOC_RW(rwop, RawData_Samples, RawData_Frequency);
    if (RawDataUint8 == nullptr) {
        THROW(std::runtime_error, "LoadVOC_RW(): Cannot read raw data!");
    }

    // shift level so that the last sample is 128
    int minValue = 255;
    int maxValue = 0;
    for (uint32_t i = 0; i < RawData_Samples; i++) {
        if (RawDataUint8[i] < minValue) {
            minValue = RawDataUint8[i];
        }
        if (RawDataUint8[i] > maxValue) {
            maxValue = RawDataUint8[i];
        }
    }

    auto levelShift = 128 - static_cast<int>(RawDataUint8[RawData_Samples - 1]);
    if (minValue + levelShift < 0) {
        levelShift = -minValue;
    } else if (maxValue + levelShift > 255) {
        levelShift = 255 - maxValue;
    }

    for (uint32_t i = 0; i < RawData_Samples; i++) {
        RawDataUint8[i] = static_cast<uint8_t>(RawDataUint8[i] + levelShift);
    }

    // Convert to floats
    std::vector<float> RawDataFloat(RawData_Samples + 2 * NUM_SAMPLES_OF_SILENCE);

    for (uint32_t i = 0; i < NUM_SAMPLES_OF_SILENCE; i++) {
        RawDataFloat[i] = 0.0;
    }

    for (uint32_t i = NUM_SAMPLES_OF_SILENCE; i < RawData_Samples + NUM_SAMPLES_OF_SILENCE; i++) {
        RawDataFloat[i] = static_cast<float>(RawDataUint8[i - NUM_SAMPLES_OF_SILENCE]) / 128.0f - 1.0f;
    }

    for (uint32_t i = RawData_Samples + NUM_SAMPLES_OF_SILENCE; i < RawData_Samples + 2 * NUM_SAMPLES_OF_SILENCE; i++) {
        RawDataFloat[i] = 0.0f;
    }

    RawDataUint8.reset();

    RawData_Samples += 2 * NUM_SAMPLES_OF_SILENCE;

    // To prevent strange invalid read in src_linear
    RawData_Samples--;

    // Get audio device specifications
    int TargetFrequency = 0;

    int channels          = 0;
    uint16_t TargetFormat = 0;
    if (Mix_QuerySpec(&TargetFrequency, &TargetFormat, &channels) == 0) {
        THROW(std::runtime_error, "LoadVOC_RW(): Mix_QuerySpec failed!");
    }

    // Convert to audio device frequency
    const auto ConversionRatio   = TargetFrequency / static_cast<double>(RawData_Frequency);
    auto TargetDataFloat_Samples = static_cast<size_t>(std::ceil(RawData_Samples * ConversionRatio));
    std::vector<float> TargetDataFloat(TargetDataFloat_Samples);

    size_t odone;

    const auto* const serror =
        soxr_oneshot(RawData_Frequency, TargetFrequency, 1, RawDataFloat.data(), RawData_Samples, nullptr,
                     TargetDataFloat.data(), TargetDataFloat.size(), &odone, nullptr, nullptr, nullptr);

    if (serror) {
        sdl2::log_error("Unable to resample from %g to %g: %s", RawData_Frequency, TargetFrequency, serror);

        for (auto x = 0U; x < TargetDataFloat_Samples; ++x) {
            const auto pos = x / ConversionRatio;
            const auto i   = static_cast<int>(pos); // lrint(floor(pos));
            TargetDataFloat[x] =
                RawDataFloat[i] * static_cast<float>(i + 1 - pos) + RawDataFloat[i + 1] * static_cast<float>(pos - i);
        }
    } else {
        if (odone != TargetDataFloat.size())
            TargetDataFloat.resize(odone);
        TargetDataFloat_Samples = TargetDataFloat.size();
    }

    auto TargetData_Samples = TargetDataFloat_Samples;

    RawDataFloat.clear();

    // Normalize if necessary
    auto distance = 0.0f;
    for (const auto s : TargetDataFloat) {
        auto abs_s = std::abs(s);
        if (abs_s > distance) {
            distance = abs_s;
        }
    }

    if (distance > 1.0f) {
        const auto scale = 1.0f / distance;

        // Normalize
        for (auto& s : TargetDataFloat)
            s *= scale;
    }

    // Convert floats back to integers but leave out 3/4 of silence
    const auto ThreeQuaterSilenceLength = static_cast<int>(NUM_SAMPLES_OF_SILENCE * ConversionRatio * (3.0f / 4.0f));
    TargetData_Samples -= 2 * ThreeQuaterSilenceLength;

    auto myChunk = sdl2::mix_chunk_ptr{static_cast<Mix_Chunk*>(SDL_calloc(sizeof(Mix_Chunk), 1))};
    if (myChunk == nullptr) {
        throw std::bad_alloc();
    }

    memset(myChunk.get(), 0, sizeof(Mix_Chunk));

    myChunk->allocated = 1;
    myChunk->volume    = 128;

    size_t SizeOfTargetSample = 0;
    switch (TargetFormat) {
        case AUDIO_U8: SizeOfTargetSample = sizeof(uint8_t) * channels; break;
        case AUDIO_S8: SizeOfTargetSample = sizeof(int8_t) * channels; break;
        case AUDIO_U16LSB: SizeOfTargetSample = sizeof(uint16_t) * channels; break;
        case AUDIO_S16LSB: SizeOfTargetSample = sizeof(int16_t) * channels; break;
        case AUDIO_U16MSB: SizeOfTargetSample = sizeof(uint16_t) * channels; break;
        case AUDIO_S16MSB: SizeOfTargetSample = sizeof(int16_t) * channels; break;
        default: {
            THROW(std::runtime_error, "LoadVOC_RW(): Invalid target sample format!");
        }
    }

    if ((myChunk->abuf = static_cast<uint8_t*>(SDL_malloc(TargetData_Samples * SizeOfTargetSample))) == nullptr) {
        throw std::bad_alloc();
    }
    myChunk->alen = TargetData_Samples * SizeOfTargetSample;

    switch (TargetFormat) {
        case AUDIO_U8: {
            auto* TargetData = myChunk->abuf;
            for (uint32_t i = 0; i < TargetData_Samples * channels; i += channels) {
                const auto v = Float2Uint8(TargetDataFloat[i / channels + ThreeQuaterSilenceLength]);
                for (auto j = 0; j < channels; j++) {
                    TargetData[i + j] = v;
                }
            }
        } break;

        case AUDIO_S8: {
            auto* TargetData = reinterpret_cast<int8_t*>(myChunk->abuf);
            for (uint32_t i = 0; i < TargetData_Samples * channels; i += channels) {
                const auto v = Float2Sint8(TargetDataFloat[i / channels + ThreeQuaterSilenceLength]);
                for (auto j = 0; j < channels; j++) {
                    TargetData[i + j] = v;
                }
            }
        } break;

        case AUDIO_U16LSB: {
            auto* TargetData = reinterpret_cast<uint16_t*>(myChunk->abuf);
            for (uint32_t i = 0; i < TargetData_Samples * channels; i += channels) {
                const auto v = SDL_SwapLE16(Float2Uint16(TargetDataFloat[i / channels + ThreeQuaterSilenceLength]));
                for (auto j = 0; j < channels; j++) {
                    TargetData[i + j] = v;
                }
            }
        } break;

        case AUDIO_S16LSB: {
            auto* TargetData = reinterpret_cast<int16_t*>(myChunk->abuf);
            for (uint32_t i = 0; i < TargetData_Samples * channels; i += channels) {
                const auto v = SDL_SwapLE16(Float2Sint16(TargetDataFloat[i / channels + ThreeQuaterSilenceLength]));
                for (int j = 0; j < channels; j++) {
                    TargetData[i + j] = v;
                }
            }
        } break;

        case AUDIO_U16MSB: {
            auto* TargetData = reinterpret_cast<uint16_t*>(myChunk->abuf);
            for (uint32_t i = 0; i < TargetData_Samples * channels; i += channels) {
                const auto v = SDL_SwapBE16(Float2Uint16(TargetDataFloat[i / channels + ThreeQuaterSilenceLength]));
                for (int j = 0; j < channels; j++) {
                    TargetData[i + j] = v;
                }
            }
        } break;

        case AUDIO_S16MSB: {
            auto* TargetData = reinterpret_cast<int16_t*>(myChunk->abuf);
            for (uint32_t i = 0; i < TargetData_Samples * channels; i += channels) {
                const auto v = SDL_SwapBE16(Float2Sint16(TargetDataFloat[i / channels + ThreeQuaterSilenceLength]));
                for (int j = 0; j < channels; j++) {
                    TargetData[i + j] = v;
                }
            }
        } break;

        default: {
            THROW(std::runtime_error, "LoadVOC_RW(): Invalid target sample format!");
        }
    }

    return myChunk;
}
