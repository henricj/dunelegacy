/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * LGPL License
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * $URL$
 * $Id$
 *
 */

#include <FileClasses/adl/sound_adlib.h>

#include <misc/exceptions.h>

#include <SDL2/SDL_endian.h>
#include <SDL2/SDL_mixer.h>

#include <adl.h>
#include <opl.h>
#include <player.h>
#include <surroundopl.h>
#include <wemuopl.h>

#include <adplug.h>
#include <binfile.h>
#include <binio.h>
#include <binstr.h>
#include <fprovide.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <optional>

typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint8_t byte;

CAdPlugDatabase* CAdPlug::database = nullptr;

namespace {
class CProvider_Cache final : public CFileProvider {
public:
    explicit CProvider_Cache(SDL_RWops* rwop) : buffer_{SDL_LoadFile_RW(rwop, &size_, 0)} { }

    [[nodiscard]] binistream* open(std::string filename) const override;
    void close(binistream* f) const override;

private:
    size_t size_{};
    sdl2::sdl_ptr<void> buffer_;
};

binistream* CProvider_Cache::open(std::string filename) const {
    auto f = std::make_unique<binisstream>(buffer_.get(), static_cast<unsigned long>(size_));

    if (!f)
        return nullptr;
    if (f->error())
        return nullptr;

    // Open all files as little endian with IEEE floats by default
    f->setFlag(binio::BigEndian, false);
    f->setFlag(binio::FloatIEEE);

    return f.release();
}

void CProvider_Cache::close(binistream* f) const {
    delete f;
}

} // namespace

std::unique_ptr<Copl> SoundAdlibPC::create_opl() {
    auto a = std::make_unique<CWemuopl>(m_freq, true, false);
    auto b = std::make_unique<CWemuopl>(m_freq, true, false);

    COPLprops pa{a.get(), true, false};
    COPLprops pb{b.get(), true, false};

    auto opl = std::make_unique<CSurroundopl>(&pa, &pb, true);

    a.release();
    b.release();

    return opl;
}

SoundAdlibPC::SoundAdlibPC(SDL_RWops* rwop, int freq) {

    if (freq > 0) {
        m_freq     = freq;
        m_format   = AUDIO_S16LSB;
        m_channels = 2;
    } else
        Mix_QuerySpec(&m_freq, &m_format, &m_channels);

    opl_ = create_opl();

    opl_->init();

    driver_.reset(CadlPlayer::factory(opl_.get()));
    assert(driver_);

    { // Scope
        const auto file = create_provider(rwop);

        driver_->load("file.adl", *file);
    }
}

SoundAdlibPC::~SoundAdlibPC() = default;

std::unique_ptr<CFileProvider> SoundAdlibPC::create_provider(SDL_RWops* rwop) {
    return std::make_unique<CProvider_Cache>(rwop);
}

void SoundAdlibPC::playTrack(int track) {
    driver_->rewind(-1); // Reinitialize the opl.
    driver_->rewind(track);

    offset_  = static_cast<float>(m_freq) / driver_->getrefresh() - 4;
    playing_ = true;
}

namespace {

template<typename TOutput, typename TInput>
TOutput clamp_cast(TInput input) {
    static constexpr auto min_val = static_cast<TInput>(std::numeric_limits<TOutput>::min());
    static constexpr auto max_val = static_cast<TInput>(std::numeric_limits<TOutput>::max());

    return static_cast<TOutput>(std::clamp(input, min_val, max_val));
}

} // namespace

void SoundAdlibPC::read(int16_t* data, int samples) {
    const auto refresh_rate     = driver_->getrefresh();
    const auto inv_refresh_rate = 1.f / refresh_rate;

    auto* const data0   = data;
    const auto samples0 = samples;

    while (samples > 0) {
        const auto ticks_remaining = static_cast<float>(m_freq) - refresh_rate * offset_;

        if (ticks_remaining < refresh_rate) {
            offset_ -= static_cast<float>(m_freq) * inv_refresh_rate;

            if (playing_)
                playing_ = driver_->update();

            continue;
        }

        const auto samples_remaining = static_cast<int>(std::floor(ticks_remaining * inv_refresh_rate));

        const auto n = std::min(samples_remaining, samples);

        opl_->update(data, n);

        offset_ += static_cast<float>(n);
        data += static_cast<ptrdiff_t>(n) * m_channels;
        samples -= n;
    }

    const auto volume = getVolume();
    if (volume != MIX_MAX_VOLUME) {
        for (int i = 0; i < m_channels * samples0; i++) {
            data0[i] = clamp_cast<int16>(data0[i] * volume / MIX_MAX_VOLUME);
        }
    }
}

void SoundAdlibPC::callback(void* udata, uint8_t* stream, int len) {
    auto* self = static_cast<SoundAdlibPC*>(udata);
    auto* buf  = reinterpret_cast<int16*>(stream);

    const auto samples = len / self->getsampsize();

    self->read(buf, samples);
}

sdl2::mix_chunk_ptr SoundAdlibPC::getSubsong(int Num) {

    playTrack(Num);

    uint8_t* buf = nullptr;
    int bufSize  = 0;

    for (;;) {
        static constexpr auto size = 16384;

        bufSize += size;
        if ((buf = static_cast<uint8_t*>(SDL_realloc(buf, bufSize))) == nullptr) {
            THROW(std::runtime_error, "Cannot allocate memory!");
        }

        memset(buf + bufSize - size, 0, size);

        callback(this, buf + bufSize - size, size);

        if (!isPlaying())
            break;

        auto bSilent = true;
        for (auto* p = buf + bufSize - size; p < buf + bufSize; p++) {
            if (*p != 0) {
                bSilent = false;
                break;
            }
        }
        if (bSilent)
            break;

        if (bufSize > 1024 * 1024 * 16) {
            sdl2::log_info("SoundAdlibPC::getSubsong(): Decoding aborted after 16MB have been decoded.");
            break;
        }
    }

    for (auto i = bufSize - 1; i > 0; --i) {
        if (0 == buf[i])
            continue;

        static constexpr auto step = 16;

        // Round up, leaving a block of zeros at the end.
        // Note that "i" is one less than the size of the block that contains it.
        const auto rounded_size = (i + 2 * step) & ~(step - 1);

        if (rounded_size < bufSize)
            bufSize = rounded_size;

        break;
    }

    sdl2::mix_chunk_ptr myChunk{static_cast<Mix_Chunk*>(SDL_calloc(sizeof(Mix_Chunk), 1))};
    if (myChunk == nullptr)
        return nullptr;

    myChunk->volume    = 128;
    myChunk->allocated = 1;
    myChunk->abuf      = buf;
    myChunk->alen      = bufSize;

    return myChunk;
}
