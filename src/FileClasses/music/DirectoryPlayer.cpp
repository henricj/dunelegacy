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

#include <globals.h>

#include <misc/FileSystem.h>
#include <misc/fnkdat.h>
#include <mmath.h>

#include <filesystem>

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

    music = nullptr;

#if SDL_VERSIONNUM(SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL) >= SDL_VERSIONNUM(2, 0, 2)
    Mix_Init(MIX_INIT_MID | MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_OGG);
#else
    Mix_Init(MIX_INIT_FLUIDSYNTH | MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_OGG);
#endif
}

DirectoryPlayer::~DirectoryPlayer() {
    if (music != nullptr) {
        Mix_FreeMusic(music);
        music = nullptr;
    }

    Mix_Quit();
}

void DirectoryPlayer::changeMusic(MUSICTYPE musicType) {
    int musicNum                   = -1;
    std::filesystem::path filename = "";

    if (currentMusicType == musicType && Mix_PlayingMusic()) {
        return;
    }

    if (musicType >= 0 && musicType < MUSIC_NUM_MUSIC_TYPES && !musicFileList[musicType].empty()) {
        musicNum         = random().rand(0u, musicFileList[musicType].size() - 1u);
        filename         = musicFileList[musicType][musicNum];
        currentMusicType = musicType;
    } else {
        // MUSIC_RANDOM
        const int maxnum = musicFileList[MUSIC_ATTACK].size() + musicFileList[MUSIC_PEACE].size();

        if (maxnum > 0) {
            const unsigned int randnum = random().rand(0, maxnum - 1);

            if (randnum < musicFileList[MUSIC_ATTACK].size()) {
                musicNum         = randnum;
                filename         = musicFileList[MUSIC_ATTACK][musicNum];
                currentMusicType = MUSIC_ATTACK;
            } else {
                musicNum         = randnum - musicFileList[MUSIC_ATTACK].size();
                filename         = musicFileList[MUSIC_PEACE][musicNum];
                currentMusicType = MUSIC_PEACE;
            }
        }
    }

    if (musicOn && !filename.empty()) {

        Mix_HaltMusic();

        if (music != nullptr) {
            Mix_FreeMusic(music);
            music = nullptr;
        }

        music = Mix_LoadMUS(filename.string().c_str());
        if (music != nullptr) {
            sdl2::log_info("Now playing {}!", filename.string());
            Mix_PlayMusic(music, -1);
            Mix_VolumeMusic(musicVolume);
        } else {
            sdl2::log_info("Unable to play {}: {}!", filename.string(), Mix_GetError());
        }
    }
}

void DirectoryPlayer::toggleSound() {
    if (!musicOn) {
        musicOn = true;
        changeMusic(MUSIC_PEACE);
    } else {
        musicOn = false;
        if (music != nullptr) {
            Mix_HaltMusic();
            Mix_FreeMusic(music);
            music = nullptr;
        }
    }
}

bool DirectoryPlayer::isMusicPlaying() {
    return Mix_PlayingMusic();
}

void DirectoryPlayer::setMusic(bool value) {
    musicOn = value;

    if (musicOn) {
        changeMusic(MUSIC_RANDOM);
    } else if (music != nullptr) {
        Mix_HaltMusic();
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
