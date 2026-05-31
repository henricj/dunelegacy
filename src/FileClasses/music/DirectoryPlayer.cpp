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

#include <FileClasses/music/DirectoryPlayer.h>

#include <Audio/AudioEngine.h>
#include <globals.h>

#include <misc/FileSystem.h>
#include <misc/fnkdat.h>
#include <mmath.h>

#include <gsl/gsl>

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

DirectoryPlayer::DirectoryPlayer()
    : MusicPlayer(dune::globals::settings.audio.playMusic, dune::globals::settings.audio.musicVolume,
                  "DirectoryPlayer") {
    // determine path to config file
    auto [ok, configfilepath] = fnkdat(FNKDAT_USER | FNKDAT_CREAT);

    static constexpr auto musicDirectoryNames =
        // clang-format off
        std::to_array({
        "/music/attack/",    "/music/peace/",     "/music/intro/",   "/music/menu/",      "/music/briefingH/",
        "/music/briefingA/", "/music/briefingO/", "/music/winH/",    "/music/winA/",      "/music/winO/",
        "/music/loseH/",     "/music/loseA/",     "/music/loseO/",   "/music/gamestats/", "/music/mapchoice/",
        "/music/meanwhile/", "/music/finaleH/",   "/music/finaleA/", "/music/finaleO/"});
    // clang-format on

    static_assert(std::tuple_size_v<decltype(musicDirectoryNames)> == MUSIC_NUM_MUSIC_TYPES);
    static_assert(std::tuple_size_v<decltype(musicFileList)> == MUSIC_NUM_MUSIC_TYPES);

    for (int i = 0; i < MUSIC_NUM_MUSIC_TYPES; i++) {
        const char* dirName = musicDirectoryNames[i] + 1; // skip '/' at the beginning
        fnkdat(dirName, FNKDAT_USER | FNKDAT_CREAT);
        musicFileList[i] = getMusicFileNames(configfilepath / musicDirectoryNames[i]);
    }

    auto* const audio_engine = dune::globals::pAudioEngine.get();
    if (audio_engine == nullptr) {
        return;
    }

    track_ = audio_engine->createTrack();
    if (!track_) {
        sdl2::log_warn("DirectoryPlayer: Unable to create music track: {}", SDL_GetError());
        return;
    }

    if (!audio_engine->tagTrack(track_.get(), AudioEngine::kMusicTag)) {
        sdl2::log_warn("DirectoryPlayer: Unable to tag music track: {}", SDL_GetError());
    }
    audio_engine->setTrackGain(track_.get(), volumeToGain(musicVolume));
}

DirectoryPlayer::~DirectoryPlayer() {
    setMusic(false);

    auto* const audio_engine = dune::globals::pAudioEngine.get();
    if (audio_engine != nullptr && track_) {
        audio_engine->untagTrack(track_.get(), AudioEngine::kMusicTag);
    }
}

void DirectoryPlayer::changeMusic(MUSICTYPE musicType) {
    int musicNum                   = -1;
    std::filesystem::path filename = "";

    auto* const audio_engine = dune::globals::pAudioEngine.get();
    if (audio_engine == nullptr || !track_) {
        currentMusicType = musicType;
        return;
    }

    if (currentMusicType == musicType && audio_engine->isTrackPlaying(track_.get())) {
        return;
    }

    if (musicType >= MUSIC_FIRST_MUSIC_TYPE && musicType < MUSIC_NUM_MUSIC_TYPES && !musicFileList[musicType].empty()) {
        musicNum         = random().rand(0, gsl::narrow<int32_t>(musicFileList[musicType].size() - 1u));
        filename         = musicFileList[musicType][musicNum];
        currentMusicType = musicType;
    } else {
        // MUSIC_RANDOM
        const auto attack_size = musicFileList[MUSIC_ATTACK].size();
        const auto maxnum      = attack_size + musicFileList[MUSIC_PEACE].size();

        if (maxnum > 0U) {
            const auto randnum = random().rand(0U, gsl::narrow<uint32_t>(maxnum - 1U));

            if (randnum < attack_size) {
                musicNum         = gsl::narrow<int>(randnum);
                filename         = musicFileList[MUSIC_ATTACK][musicNum];
                currentMusicType = MUSIC_ATTACK;
            } else {
                musicNum         = gsl::narrow<int>(randnum - attack_size);
                filename         = musicFileList[MUSIC_PEACE][musicNum];
                currentMusicType = MUSIC_PEACE;
            }
        }
    }

    if (musicOn && !filename.empty()) {
        audio_engine->stopMusic();
        audio_.reset();

        auto file = dune::globals::pFileManager->openFile(filename);
        if (!file) {
            sdl2::log_info("Unable to open {}!", filename.string());
            return;
        }
        audio_ = audio_engine->loadAudioIO(file.release(), true, true);
        if (audio_ && audio_engine->setTrackAudio(track_.get(), audio_.get())) {
            sdl2::log_info("Now playing {}!", filename.string());
            const auto play_options = create_loop_options(-1);
            const auto ok           = audio_engine->playTrack(track_.get(), play_options);
            if (play_options != 0) {
                SDL_DestroyProperties(play_options);
            }
            if (!ok) {
                sdl2::log_info("Unable to play {}: {}!", filename.string(), SDL_GetError());
            }
        } else {
            sdl2::log_info("Unable to play {}: {}!", filename.string(), SDL_GetError());
        }
    }
}

void DirectoryPlayer::toggleSound() {
    if (!musicOn) {
        musicOn = true;
        changeMusic(MUSIC_PEACE);
    } else {
        setMusic(false);
    }
}

bool DirectoryPlayer::isMusicPlaying() {
    auto* const audio_engine = dune::globals::pAudioEngine.get();
    return audio_engine != nullptr && track_ && audio_engine->isTrackPlaying(track_.get());
}

void DirectoryPlayer::setMusic(bool value) {
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

void DirectoryPlayer::setMusicVolume(int newVolume) {
    MusicPlayer::setMusicVolume(newVolume);

    auto* const audio_engine = dune::globals::pAudioEngine.get();
    if (audio_engine != nullptr && track_) {
        audio_engine->setTrackGain(track_.get(), volumeToGain(musicVolume));
    }
}

std::vector<std::filesystem::path> DirectoryPlayer::getMusicFileNames(const std::filesystem::path& dir) {
    std::vector<std::filesystem::path> files;

    for (const auto& filename : getFileNamesList(dir, "mp3", true)) {
        files.push_back((dir / filename).lexically_normal());
    }

    for (const auto& filename : getFileNamesList(dir, "ogg", true)) {
        files.push_back((dir / filename).lexically_normal());
    }

    for (const auto& filename : getFileNamesList(dir, "wav", true)) {
        files.push_back((dir / filename).lexically_normal());
    }

    for (const auto& filename : getFileNamesList(dir, "flac", true)) {
        files.push_back((dir / filename).lexically_normal());
    }

    for (const auto& filename : getFileNamesList(dir, "mid", true)) {
        files.push_back((dir / filename).lexically_normal());
    }

    return files;
}
