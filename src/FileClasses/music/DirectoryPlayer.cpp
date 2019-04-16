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

DirectoryPlayer::DirectoryPlayer() : MusicPlayer(settings.audio.playMusic, settings.audio.musicVolume) {
    // determine path to config file
    char tmp[FILENAME_MAX];
    fnkdat(nullptr, tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
    std::string configfilepath(tmp);

    static const char* const musicDirectoryNames[MUSIC_NUM_MUSIC_TYPES] = { "/music/attack/",
                                                                        "/music/peace/",
                                                                        "/music/intro/",
                                                                        "/music/menu/",
                                                                        "/music/briefingH/",
                                                                        "/music/briefingA/",
                                                                        "/music/briefingO/",
                                                                        "/music/winH/",
                                                                        "/music/winA/",
                                                                        "/music/winO/",
                                                                        "/music/loseH/",
                                                                        "/music/loseA/",
                                                                        "/music/loseO/",
                                                                        "/music/gamestats/",
                                                                        "/music/mapchoice/",
                                                                        "/music/meanwhile/",
                                                                        "/music/finaleH/",
                                                                        "/music/finaleA/",
                                                                        "/music/finaleO/"
                                                                    };

    for(int i=0;i<MUSIC_NUM_MUSIC_TYPES;i++) {
        char tmp2[FILENAME_MAX];
        const char* dirName =  musicDirectoryNames[i] + 1; // skip '/' at the beginning
        fnkdat(dirName, tmp2, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
        musicFileList[i] = getMusicFileNames(configfilepath + musicDirectoryNames[i]);
    }

    music = nullptr;

#if SDL_VERSIONNUM(SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL) >= SDL_VERSIONNUM(2,0,2)
    Mix_Init(MIX_INIT_MID | MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_OGG);
#else
    Mix_Init(MIX_INIT_FLUIDSYNTH | MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_OGG);
#endif
}

DirectoryPlayer::~DirectoryPlayer() {
    if(music != nullptr) {
        Mix_FreeMusic(music);
        music = nullptr;
    }

    Mix_Quit();
}

void DirectoryPlayer::changeMusic(MUSICTYPE musicType)
{
    int musicNum = -1;
    std::string filename = "";

    if(currentMusicType == musicType && Mix_PlayingMusic()) {
        return;
    }

    if(musicType >= 0 && musicType < MUSIC_NUM_MUSIC_TYPES && !musicFileList[musicType].empty()) {
        musicNum = getRandomInt(0, musicFileList[musicType].size()-1);
        filename = musicFileList[musicType][musicNum];
        currentMusicType = musicType;
    } else {
       // MUSIC_RANDOM
        int maxnum = musicFileList[MUSIC_ATTACK].size() + musicFileList[MUSIC_PEACE].size();

        if(maxnum > 0) {
            unsigned int randnum = getRandomInt(0, maxnum-1);

            if(randnum < musicFileList[MUSIC_ATTACK].size()) {
                musicNum = randnum;
                filename = musicFileList[MUSIC_ATTACK][musicNum];
                currentMusicType = MUSIC_ATTACK;
            } else {
                musicNum = randnum - musicFileList[MUSIC_ATTACK].size();
                filename = musicFileList[MUSIC_PEACE][musicNum];
                currentMusicType = MUSIC_PEACE;
            }
        }
    }

    if((musicOn == true) && (filename != "")) {

        Mix_HaltMusic();

        if(music != nullptr) {
            Mix_FreeMusic(music);
            music = nullptr;
        }

        music = Mix_LoadMUS(filename.c_str());
        if(music != nullptr) {
            SDL_Log("Now playing %s!",filename.c_str());
            Mix_PlayMusic(music, -1);
            Mix_VolumeMusic(musicVolume);
        } else {
            SDL_Log("Unable to play %s: %s!",filename.c_str(), Mix_GetError());
        }
    }
}

void DirectoryPlayer::toggleSound() {
    if(musicOn == false) {
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

    if(musicOn) {
        changeMusic(MUSIC_RANDOM);
    } else if(music != nullptr) {
        Mix_HaltMusic();
    }
}

std::vector<std::string> DirectoryPlayer::getMusicFileNames(const std::string& dir) {
    std::vector<std::string> files;

    for(const std::string& filename : getFileNamesList(dir,"mp3",true)) {
        files.push_back(dir + filename);
    }

    for(const std::string& filename : getFileNamesList(dir,"ogg",true)) {
        files.push_back(dir + filename);
    }

    for(const std::string& filename : getFileNamesList(dir,"wav",true)) {
        files.push_back(dir + filename);
    }

    for(const std::string& filename : getFileNamesList(dir,"flac",true)) {
        files.push_back(dir + filename);
    }

    for(const std::string& filename : getFileNamesList(dir,"mid",true)) {
        files.push_back(dir + filename);
    }

    return files;
}
