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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SOUND_ADLIB_H
#define SOUND_ADLIB_H

#include <misc/SDL2pp.h>

#include <SDL2/SDL_mixer.h>

#include <memory>
#include <vector>

class CFileProvider;
class CPlayer;
class Copl;

/**
 * AdLib implementation of the sound output device.
 *
 * It uses a special sound file format special to
 * Dune II, Kyrandia 1 and 2. While Dune II and
 * Kyrandia 1 are using exact the same format, the
 * one of Kyrandia 2 slightly differs.
 *
 * See AdlibDriver for more information.
 * @see AdlibDriver
 */
class SoundAdlibPC final {
public:
    SoundAdlibPC(SDL_RWops* rwop, int freq = 0);
    SoundAdlibPC(const SoundAdlibPC& soundAdlibPC)            = delete;
    SoundAdlibPC& operator=(const SoundAdlibPC& soundAdlibPC) = delete;
    ~SoundAdlibPC();

    static void callback(void* udata, uint8_t* stream, int len);

    void playTrack(int track);

    [[nodiscard]] bool isPlaying() const { return playing_; }

    sdl2::mix_chunk_ptr getSubsong(int Num);

    void setVolume(int newVolume) noexcept { volume_ = newVolume; }

    [[nodiscard]] int getVolume() const noexcept { return volume_; }

private:
    void read(int16_t* data, int samples);

    static std::unique_ptr<CFileProvider> create_provider(SDL_RWops* rwop);

    std::unique_ptr<Copl> create_opl();

    [[nodiscard]] auto getsampsize() const noexcept {
        return m_channels * (m_format == AUDIO_U8 || m_format == AUDIO_S8 ? 1 : 2);
    }

    std::unique_ptr<Copl> opl_;
    std::unique_ptr<CPlayer> driver_;

    bool playing_{};
    int volume_{MIX_MAX_VOLUME / 2};

    int m_channels;
    int m_freq;
    uint16_t m_format;

    float offset_{};
};

#endif
