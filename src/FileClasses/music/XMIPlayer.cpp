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

#include <FileClasses/music/XMIPlayer.h>

#include <Audio/AudioEngine.h>
#include <globals.h>

#include "FileClasses/xmidi/MemoryDataSource.h"
#include "FileClasses/xmidi/SDLDataSource.h"
#include "FileClasses/xmidi/XMidiEventList.h"
#include <FileClasses/FileManager.h>
#include <FileClasses/xmidi/XMidiFile.h>

#include <gsl/gsl>
#include <misc/dune_sdl.h>

#include <filesystem>

namespace {

SDL_PropertiesID create_loop_options(int loops) {
    const auto options = SDL_CreateProperties();
    if (options != 0) {
        SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
    }
    return options;
}

} // namespace

XMIPlayer::XMIPlayer()
    : MusicPlayer(dune::globals::settings.audio.playMusic, dune::globals::settings.audio.musicVolume, "XMIPlayer") {
    auto* const audio_engine = dune::globals::pAudioEngine.get();
    if (audio_engine == nullptr) {
        return;
    }

    track_ = audio_engine->createTrack();
    if (!track_) {
        sdl2::log_warn("XMIPlayer: Unable to create music track: {}", SDL_GetError());
        return;
    }

    if (!audio_engine->tagTrack(track_.get(), AudioEngine::kMusicTag)) {
        sdl2::log_warn("XMIPlayer: Unable to tag music track: {}", SDL_GetError());
    }
    audio_engine->setTrackGain(track_.get(), volumeToGain(musicVolume));
}

XMIPlayer::~XMIPlayer() {
    setMusic(false);

    auto* const audio_engine = dune::globals::pAudioEngine.get();
    if (audio_engine != nullptr && track_) {
        audio_engine->untagTrack(track_.get(), AudioEngine::kMusicTag);
    }
}

void XMIPlayer::changeMusic(MUSICTYPE musicType) {
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
        DUNE0.XMI/4
        DUNE1.XMI/2 and DUNE10.XMI/2
    */

    switch (musicType) {
        case MUSIC_ATTACK: {

            switch (random().rand(0, 5)) {
                case 0:
                    filename = "DUNE10.XMI";
                    musicNum = 7;
                    break;
                case 1:
                    filename = "DUNE11.XMI";
                    musicNum = 7;
                    break;
                case 2:
                    filename = "DUNE12.XMI";
                    musicNum = 7;
                    break;
                case 3:
                    filename = "DUNE13.XMI";
                    musicNum = 7;
                    break;
                case 4:
                    filename = "DUNE14.XMI";
                    musicNum = 7;
                    break;
                case 5:
                    filename = "DUNE15.XMI";
                    musicNum = 7;
                    break;
            }

        } break;

        case MUSIC_PEACE: {

            switch (random().rand(0, 8)) {
                case 0:
                    filename = "DUNE1.XMI";
                    musicNum = 6;
                    break;
                case 1:
                    filename = "DUNE2.XMI";
                    musicNum = 6;
                    break;
                case 2:
                    filename = "DUNE3.XMI";
                    musicNum = 6;
                    break;
                case 3:
                    filename = "DUNE4.XMI";
                    musicNum = 6;
                    break;
                case 4:
                    filename = "DUNE5.XMI";
                    musicNum = 6;
                    break;
                case 5:
                    filename = "DUNE6.XMI";
                    musicNum = 6;
                    break;
                case 6:
                    filename = "DUNE9.XMI";
                    musicNum = 4;
                    break;
                case 7:
                    filename = "DUNE9.XMI";
                    musicNum = 5;
                    break;
                case 8:
                    filename = "DUNE18.XMI";
                    musicNum = 6;
                    break;
            }

        } break;

        case MUSIC_INTRO: {
            filename = "DUNE0.XMI";
            musicNum = 2;
        } break;

        case MUSIC_MENU: {
            filename = "DUNE7.XMI";
            musicNum = 6;
        } break;

        case MUSIC_BRIEFING_H: {
            filename = "DUNE7.XMI";
            musicNum = 2;
        } break;

        case MUSIC_BRIEFING_A: {
            filename = "DUNE7.XMI";
            musicNum = 3;
        } break;

        case MUSIC_BRIEFING_O: {
            filename = "DUNE7.XMI";
            musicNum = 4;
        } break;

        case MUSIC_WIN_H: {
            filename = "DUNE8.XMI";
            musicNum = 3;
        } break;

        case MUSIC_WIN_A: {
            filename = "DUNE8.XMI";
            musicNum = 2;
        } break;

        case MUSIC_WIN_O: {
            filename = "DUNE17.XMI";
            musicNum = 4;
        } break;

        case MUSIC_LOSE_H: {
            filename = "DUNE1.XMI";
            musicNum = 4;
        } break;

        case MUSIC_LOSE_A: {
            filename = "DUNE1.XMI";
            musicNum = 5;
        } break;

        case MUSIC_LOSE_O: {
            filename = "DUNE1.XMI";
            musicNum = 3;
        } break;

        case MUSIC_GAMESTAT: {
            filename = "DUNE20.XMI";
            musicNum = 2;
        } break;

        case MUSIC_MAPCHOICE: {
            filename = "DUNE16.XMI";
            musicNum = 7;
        } break;

        case MUSIC_MEANWHILE: {
            filename = "DUNE16.XMI";
            musicNum = 8;
        } break;

        case MUSIC_FINALE_H: {
            filename = "DUNE19.XMI";
            musicNum = 4;
        } break;

        case MUSIC_FINALE_A: {
            filename = "DUNE19.XMI";
            musicNum = 2;
        } break;

        case MUSIC_FINALE_O: {
            filename = "DUNE19.XMI";
            musicNum = 3;
        } break;

        case MUSIC_RANDOM:
        default: {

            switch (random().rand(0, 14)) {
                // attack
                case 0:
                    filename = "DUNE10.XMI";
                    musicNum = 7;
                    break;
                case 1:
                    filename = "DUNE11.XMI";
                    musicNum = 7;
                    break;
                case 2:
                    filename = "DUNE12.XMI";
                    musicNum = 7;
                    break;
                case 3:
                    filename = "DUNE13.XMI";
                    musicNum = 7;
                    break;
                case 4:
                    filename = "DUNE14.XMI";
                    musicNum = 7;
                    break;
                case 5:
                    filename = "DUNE15.XMI";
                    musicNum = 7;
                    break;

                // peace
                case 6:
                    filename = "DUNE1.XMI";
                    musicNum = 6;
                    break;
                case 7:
                    filename = "DUNE2.XMI";
                    musicNum = 6;
                    break;
                case 8:
                    filename = "DUNE3.XMI";
                    musicNum = 6;
                    break;
                case 9:
                    filename = "DUNE4.XMI";
                    musicNum = 6;
                    break;
                case 10:
                    filename = "DUNE5.XMI";
                    musicNum = 6;
                    break;
                case 11:
                    filename = "DUNE6.XMI";
                    musicNum = 6;
                    break;
                case 12:
                    filename = "DUNE9.XMI";
                    musicNum = 4;
                    break;
                case 13:
                    filename = "DUNE9.XMI";
                    musicNum = 5;
                    break;
                case 14:
                    filename = "DUNE18.XMI";
                    musicNum = 6;
                    break;
            }

        } break;
    }

    currentMusicType = musicType;

    if (musicOn && !filename.empty()) {
        midiBuffer_.clear();

        { // Scope
            auto input_path = std::filesystem::path(reinterpret_cast<const char8_t*>(filename.c_str()));
            auto inputrwop  = dune::globals::pFileManager->openFile(input_path);
            ISDLDataSource input(inputrwop.get(), 0);

            XMidiFile myXMIDI(&input, XMIDIFILE_CONVERT_NOCONVERSION);

            input.close();
            inputrwop.reset();

            const auto event_list = myXMIDI.GetEventList(musicNum);

            if (nullptr == event_list) {
                sdl2::log_info("XMIPlayer: Playing music failed: {}", SDL_GetError());
                return;
            }

            OMemoryDataSource output;

            event_list->write(&output);

            midiBuffer_ = output.takeBuffer();
        }

        audio_engine->stopMusic();
        audio_.reset();

        sdl2::IOStream_ptr midi_rwops{SDL_IOFromConstMem(midiBuffer_.data(), midiBuffer_.size())};
        audio_ = audio_engine->loadAudioIO(midi_rwops.release(), true, true);
        if (audio_ && audio_engine->setTrackAudio(track_.get(), audio_.get())) {
            const auto play_options = create_loop_options(1);
            const auto ok           = audio_engine->playTrack(track_.get(), play_options);
            if (play_options != 0) {
                SDL_DestroyProperties(play_options);
            }
            if (!ok) {
                sdl2::log_info("XMIPlayer: Playing music failed: {}", SDL_GetError());
            } else {
                sdl2::log_info("Now playing {}!", filename);
            }
        } else {
            sdl2::log_info("Unable to play {}: {}!", filename, SDL_GetError());
        }
    }
}

void XMIPlayer::toggleSound() {
    if (!musicOn) {
        musicOn = true;
        changeMusic(MUSIC_PEACE);
    } else {
        setMusic(false);
    }
}

bool XMIPlayer::isMusicPlaying() {
    auto* const audio_engine = dune::globals::pAudioEngine.get();
    return audio_engine != nullptr && track_ && audio_engine->isTrackPlaying(track_.get());
}

void XMIPlayer::setMusic(bool value) {
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

void XMIPlayer::setMusicVolume(int newVolume) {
    MusicPlayer::setMusicVolume(newVolume);

    auto* const audio_engine = dune::globals::pAudioEngine.get();
    if (audio_engine != nullptr && track_) {
        audio_engine->setTrackGain(track_.get(), volumeToGain(musicVolume));
    }
}
