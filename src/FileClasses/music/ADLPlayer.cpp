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

#include <FileClasses/music/ADLPlayer.h>

#include <Audio/AudioEngine.h>
#include <globals.h>

#include "FileClasses/adl/sound_adlib.h"

#include <FileClasses/FileManager.h>

namespace {

SDL_PropertiesID create_loop_options(int loops) {
    const auto options = SDL_CreateProperties();
    if (options != 0) {
        SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
    }
    return options;
}

} // namespace

ADLPlayer::ADLPlayer()
    : MusicPlayer(dune::globals::settings.audio.playMusic, dune::globals::settings.audio.musicVolume, "ADLPlayer") {
    auto* const audio_engine = dune::globals::pAudioEngine.get();
    if (audio_engine == nullptr) {
        return;
    }

    SDL_AudioSpec spec{};
    if (!audio_engine->getFormat(spec)) {
        spec.freq = 44100;
    }
    playbackFrequency_ = spec.freq;

    track_ = audio_engine->createTrack();
    if (!track_) {
        sdl2::log_warn("ADLPlayer: Unable to create music track: {}", SDL_GetError());
        return;
    }

    if (!audio_engine->tagTrack(track_.get(), AudioEngine::kMusicTag)) {
        sdl2::log_warn("ADLPlayer: Unable to tag music track: {}", SDL_GetError());
    }
    audio_engine->setTrackGain(track_.get(), volumeToGain(musicVolume));
}

ADLPlayer::~ADLPlayer() {
    setMusic(false);

    auto* const audio_engine = dune::globals::pAudioEngine.get();
    if (audio_engine != nullptr && track_) {
        audio_engine->untagTrack(track_.get(), AudioEngine::kMusicTag);
    }
}

void ADLPlayer::changeMusic(MUSICTYPE musicType) {
    int musicNum = -1;
    std::string filename;

    auto* const audio_engine = dune::globals::pAudioEngine.get();
    if (audio_engine == nullptr || !track_) {
        currentMusicType = musicType;
        return;
    }

    if (currentMusicType == musicType && audio_engine->isTrackPlaying(track_.get())) {
        return;
    }

    /* currently unused:
        DUNE0.ADL/4
        DUNE1.ADL/2 and DUNE10.ADL/2
    */

    switch (musicType) {
        case MUSIC_ATTACK: {

            switch (random().rand(0, 5)) {
                case 0:
                    filename = "DUNE10.ADL";
                    musicNum = 7;
                    break;
                case 1:
                    filename = "DUNE11.ADL";
                    musicNum = 7;
                    break;
                case 2:
                    filename = "DUNE12.ADL";
                    musicNum = 7;
                    break;
                case 3:
                    filename = "DUNE13.ADL";
                    musicNum = 7;
                    break;
                case 4:
                    filename = "DUNE14.ADL";
                    musicNum = 7;
                    break;
                case 5:
                    filename = "DUNE15.ADL";
                    musicNum = 7;
                    break;
            }

        } break;

        case MUSIC_PEACE: {

            switch (random().rand(0, 8)) {
                case 0:
                    filename = "DUNE1.ADL";
                    musicNum = 6;
                    break;
                case 1:
                    filename = "DUNE2.ADL";
                    musicNum = 6;
                    break;
                case 2:
                    filename = "DUNE3.ADL";
                    musicNum = 6;
                    break;
                case 3:
                    filename = "DUNE4.ADL";
                    musicNum = 6;
                    break;
                case 4:
                    filename = "DUNE5.ADL";
                    musicNum = 6;
                    break;
                case 5:
                    filename = "DUNE6.ADL";
                    musicNum = 6;
                    break;
                case 6:
                    filename = "DUNE9.ADL";
                    musicNum = 4;
                    break;
                case 7:
                    filename = "DUNE9.ADL";
                    musicNum = 5;
                    break;
                case 8:
                    filename = "DUNE18.ADL";
                    musicNum = 6;
                    break;
            }

        } break;

        case MUSIC_INTRO: {
            filename = "DUNE0.ADL";
            musicNum = 2;
        } break;

        case MUSIC_MENU: {
            filename = "DUNE7.ADL";
            musicNum = 6;
        } break;

        case MUSIC_BRIEFING_H: {
            filename = "DUNE7.ADL";
            musicNum = 2;
        } break;

        case MUSIC_BRIEFING_A: {
            filename = "DUNE7.ADL";
            musicNum = 3;
        } break;

        case MUSIC_BRIEFING_O: {
            filename = "DUNE7.ADL";
            musicNum = 4;
        } break;

        case MUSIC_WIN_H: {
            filename = "DUNE8.ADL";
            musicNum = 3;
        } break;

        case MUSIC_WIN_A: {
            filename = "DUNE8.ADL";
            musicNum = 2;
        } break;

        case MUSIC_WIN_O: {
            filename = "DUNE17.ADL";
            musicNum = 4;
        } break;

        case MUSIC_LOSE_H: {
            filename = "DUNE1.ADL";
            musicNum = 4;
        } break;

        case MUSIC_LOSE_A: {
            filename = "DUNE1.ADL";
            musicNum = 5;
        } break;

        case MUSIC_LOSE_O: {
            filename = "DUNE1.ADL";
            musicNum = 3;
        } break;

        case MUSIC_GAMESTAT: {
            filename = "DUNE20.ADL";
            musicNum = 2;
        } break;

        case MUSIC_MAPCHOICE: {
            filename = "DUNE16.ADL";
            musicNum = 7;
        } break;

        case MUSIC_MEANWHILE: {
            filename = "DUNE16.ADL";
            musicNum = 8;
        } break;

        case MUSIC_FINALE_H: {
            filename = "DUNE19.ADL";
            musicNum = 4;
        } break;

        case MUSIC_FINALE_A: {
            filename = "DUNE19.ADL";
            musicNum = 2;
        } break;

        case MUSIC_FINALE_O: {
            filename = "DUNE19.ADL";
            musicNum = 3;
        } break;

        case MUSIC_RANDOM:
        default: {

            switch (random().rand(0, 14)) {
                // attack
                case 0:
                    filename = "DUNE10.ADL";
                    musicNum = 7;
                    break;
                case 1:
                    filename = "DUNE11.ADL";
                    musicNum = 7;
                    break;
                case 2:
                    filename = "DUNE12.ADL";
                    musicNum = 7;
                    break;
                case 3:
                    filename = "DUNE13.ADL";
                    musicNum = 7;
                    break;
                case 4:
                    filename = "DUNE14.ADL";
                    musicNum = 7;
                    break;
                case 5:
                    filename = "DUNE15.ADL";
                    musicNum = 7;
                    break;

                // peace
                case 6:
                    filename = "DUNE1.ADL";
                    musicNum = 6;
                    break;
                case 7:
                    filename = "DUNE2.ADL";
                    musicNum = 6;
                    break;
                case 8:
                    filename = "DUNE3.ADL";
                    musicNum = 6;
                    break;
                case 9:
                    filename = "DUNE4.ADL";
                    musicNum = 6;
                    break;
                case 10:
                    filename = "DUNE5.ADL";
                    musicNum = 6;
                    break;
                case 11:
                    filename = "DUNE6.ADL";
                    musicNum = 6;
                    break;
                case 12:
                    filename = "DUNE9.ADL";
                    musicNum = 4;
                    break;
                case 13:
                    filename = "DUNE9.ADL";
                    musicNum = 5;
                    break;
                case 14:
                    filename = "DUNE18.ADL";
                    musicNum = 6;
                    break;
            }

        } break;
    }

    currentMusicType = musicType;

    if (musicOn && !filename.empty()) {
        auto rwop = dune::globals::pFileManager->openFile(filename);
        if (!rwop) {
            sdl2::log_info("Unable to open {}!", filename);
            return;
        }

        SoundAdlibPC decoder{rwop.get(), playbackFrequency_};
        decoder.setVolume(musicVolume);

        auto pcm_chunk = decoder.getSubsong(musicNum);
        if (pcm_chunk == nullptr || pcm_chunk->abuf == nullptr || pcm_chunk->alen == 0) {
            sdl2::log_info("Unable to decode {}!", filename);
            return;
        }

        audio_engine->stopMusic();
        audio_.reset();

        SDL_AudioSpec input_spec{};
        input_spec.format   = SDL_AUDIO_S16LE;
        input_spec.channels = 2;
        input_spec.freq     = playbackFrequency_ > 0 ? playbackFrequency_ : 44100;

        audio_ = audio_engine->loadRawAudio(pcm_chunk->abuf, pcm_chunk->alen, input_spec);
        if (!audio_ || !audio_engine->setTrackAudio(track_.get(), audio_.get())) {
            sdl2::log_info("Unable to queue {}: {}!", filename, SDL_GetError());
            return;
        }

        const auto play_options = create_loop_options(1);
        if (!audio_engine->playTrack(track_.get(), play_options)) {
            sdl2::log_info("Unable to play {}: {}!", filename, SDL_GetError());
        } else {
            sdl2::log_info("Now playing {}!", filename);
        }
        if (play_options != 0) {
            SDL_DestroyProperties(play_options);
        }
    }
}

void ADLPlayer::toggleSound() {
    if (!musicOn) {
        musicOn          = true;
        currentMusicType = MUSIC_RANDOM;
        changeMusic(MUSIC_PEACE);
    } else {
        setMusic(false);
    }
}

bool ADLPlayer::isMusicPlaying() {
    auto* const audio_engine = dune::globals::pAudioEngine.get();
    return audio_engine != nullptr && track_ && audio_engine->isTrackPlaying(track_.get());
}

void ADLPlayer::setMusic(bool value) {
    musicOn = value;

    if (musicOn) {
        changeMusic(MUSIC_RANDOM);
    } else {
        auto* const audio_engine = dune::globals::pAudioEngine.get();
        if (audio_engine != nullptr) {
            audio_engine->stopMusic();
        }
    }
}

void ADLPlayer::setMusicVolume(int newVolume) {
    parent::setMusicVolume(newVolume);
    auto* const audio_engine = dune::globals::pAudioEngine.get();
    if (audio_engine != nullptr && track_) {
        audio_engine->setTrackGain(track_.get(), volumeToGain(musicVolume));
    }
}
