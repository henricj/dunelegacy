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

#include <globals.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/xmidi/xmidi.h>

#include <misc/fnkdat.h>
#include <mmath.h>

#include <iostream>

XMIPlayer::XMIPlayer() : MusicPlayer(settings.audio.playMusic, settings.audio.musicVolume) {
    music = nullptr;

#if SDL_VERSIONNUM(SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL) >= SDL_VERSIONNUM(2,0,2)
    if((Mix_Init(MIX_INIT_MID) & MIX_INIT_MID) == 0) {
        SDL_Log("XMIPlayer: Failed to init required midi support: %s", SDL_GetError());
    }
#else
    if((Mix_Init(MIX_INIT_FLUIDSYNTH) & MIX_INIT_FLUIDSYNTH) == 0) {
        SDL_Log("XMIPlayer: Failed to init required midi support: %s", SDL_GetError());
    }
#endif
}

XMIPlayer::~XMIPlayer() {
    if(music != nullptr) {
        Mix_FreeMusic(music);
        music = nullptr;
    }

    remove(getTmpFileName().c_str());

    Mix_Quit();
}

void XMIPlayer::changeMusic(MUSICTYPE musicType)
{
    int musicNum = -1;
    std::string filename = "";

    if(currentMusicType == musicType && Mix_PlayingMusic()) {
        return;
    }

    /* currently unused:
        DUNE0.XMI/4
        DUNE1.XMI/2 and DUNE10.XMI/2
    */


    switch(musicType) {
        case MUSIC_ATTACK: {

            switch(getRandomInt(0, 5)) {
                case 0:     filename = "DUNE10.XMI";    musicNum = 7;   break;
                case 1:     filename = "DUNE11.XMI";    musicNum = 7;   break;
                case 2:     filename = "DUNE12.XMI";    musicNum = 7;   break;
                case 3:     filename = "DUNE13.XMI";    musicNum = 7;   break;
                case 4:     filename = "DUNE14.XMI";    musicNum = 7;   break;
                case 5:     filename = "DUNE15.XMI";    musicNum = 7;   break;
            }

        } break;

        case MUSIC_PEACE: {

            switch(getRandomInt(0, 8)) {
                case 0:     filename = "DUNE1.XMI";     musicNum = 6;   break;
                case 1:     filename = "DUNE2.XMI";     musicNum = 6;   break;
                case 2:     filename = "DUNE3.XMI";     musicNum = 6;   break;
                case 3:     filename = "DUNE4.XMI";     musicNum = 6;   break;
                case 4:     filename = "DUNE5.XMI";     musicNum = 6;   break;
                case 5:     filename = "DUNE6.XMI";     musicNum = 6;   break;
                case 6:     filename = "DUNE9.XMI";     musicNum = 4;   break;
                case 7:     filename = "DUNE9.XMI";     musicNum = 5;   break;
                case 8:     filename = "DUNE18.XMI";    musicNum = 6;   break;
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

            switch(getRandomInt(0, 14)) {
                // attack
                case 0:     filename = "DUNE10.XMI";    musicNum = 7;   break;
                case 1:     filename = "DUNE11.XMI";    musicNum = 7;   break;
                case 2:     filename = "DUNE12.XMI";    musicNum = 7;   break;
                case 3:     filename = "DUNE13.XMI";    musicNum = 7;   break;
                case 4:     filename = "DUNE14.XMI";    musicNum = 7;   break;
                case 5:     filename = "DUNE15.XMI";    musicNum = 7;   break;

                // peace
                case 6:     filename = "DUNE1.XMI";     musicNum = 6;   break;
                case 7:     filename = "DUNE2.XMI";     musicNum = 6;   break;
                case 8:     filename = "DUNE3.XMI";     musicNum = 6;   break;
                case 9:     filename = "DUNE4.XMI";     musicNum = 6;   break;
                case 10:    filename = "DUNE5.XMI";     musicNum = 6;   break;
                case 11:    filename = "DUNE6.XMI";     musicNum = 6;   break;
                case 12:    filename = "DUNE9.XMI";     musicNum = 4;   break;
                case 13:    filename = "DUNE9.XMI";     musicNum = 5;   break;
                case 14:    filename = "DUNE18.XMI";    musicNum = 6;   break;
            }

        } break;
    }

    currentMusicType = musicType;

    if((musicOn == true) && (filename != "")) {
        sdl2::RWops_ptr inputrwop = pFileManager->openFile(filename);
        SDLDataSource input(inputrwop.release(),1);

        std::string tmpFilename = getTmpFileName();

        SDL_RWops* outputrwop = SDL_RWFromFile(tmpFilename.c_str(),"wb");
        if(outputrwop == nullptr) {
            std::cerr << "Cannot open file " << tmpFilename << "!" << std::endl;
            return;
        }
        SDLDataSource output(outputrwop,1);

        XMIDI myXMIDI(&input, XMIDI_CONVERT_NOCONVERSION);
        myXMIDI.retrieve(musicNum, &output );

        input.close();
        output.close();

        Mix_HaltMusic();
        if(music != nullptr) {
            Mix_FreeMusic(music);
            music = nullptr;
        }

        music = Mix_LoadMUS(tmpFilename.c_str());
        if(music != nullptr) {
            if(Mix_PlayMusic(music, -1) == -1) {
                SDL_Log("XMIPlayer: Playing music failed: %s", SDL_GetError());
            } else {
                Mix_VolumeMusic(musicVolume);
                SDL_Log("Now playing %s!", tmpFilename.c_str());
            }
        } else {
            SDL_Log("Unable to play %s: %s!", filename.c_str(), Mix_GetError());
        }

    }
}

void XMIPlayer::toggleSound() {
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

bool XMIPlayer::isMusicPlaying() {
    return Mix_PlayingMusic();
}

void XMIPlayer::setMusic(bool value) {
    musicOn = value;

    if(musicOn) {
        changeMusic(MUSIC_RANDOM);
    } else if(music != nullptr) {
        Mix_HaltMusic();
    }
}

std::string XMIPlayer::getTmpFileName() {
    // determine path to config file
    char tmp[FILENAME_MAX];
    fnkdat("tmp.mid", tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);
    return std::string(tmp);
}
