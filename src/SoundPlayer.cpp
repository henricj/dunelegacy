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

#include <SoundPlayer.h>

#include <globals.h>

#include <ScreenBorder.h>
#include <Game.h>
#include <Map.h>
#include <House.h>


Mix_Chunk* curVoiceChunk = nullptr;
int voiceChannel = 0;
bool PlayingVoiceATM = false;

void VoiceChunkFinishedCallback(int channel) {
	if(channel == voiceChannel) {
		PlayingVoiceATM = false;
	}
}

SoundPlayer::SoundPlayer() {
	sfxVolume = MIX_MAX_VOLUME/2;

	Mix_Volume(-1, MIX_MAX_VOLUME);

	// init global variables
	curVoiceChunk = nullptr;
	PlayingVoiceATM = false;

	voiceChannel = Mix_ReserveChannels(1);	//Reserve a channel for voice over
	Mix_ChannelFinished(VoiceChunkFinishedCallback);

	soundOn = settings.audio.playSFX;
}

SoundPlayer::~SoundPlayer() {
}

void SoundPlayer::playSoundAt(Sound_enum soundID, const Coord& location)
{
	if(soundOn) {
        if( !currentGameMap->tileExists(location)
            || !currentGameMap->getTile(location)->isExplored(pLocalHouse->getHouseID()) ) {
            return;
        }

        Coord realCoord = location * TILESIZE + Coord(TILESIZE/2, TILESIZE/2);

        if(screenborder->isInsideScreen(realCoord, Coord(TILESIZE, TILESIZE)) ) {
            playSound(soundID, sfxVolume);
        } else if(screenborder->isInsideScreen(realCoord, Coord(TILESIZE*16, TILESIZE*16)) ) {
            playSound(soundID, (sfxVolume*3)/4);
        } else if(screenborder->isInsideScreen(realCoord, Coord(TILESIZE*24, TILESIZE*24)) ) {
            playSound(soundID, sfxVolume/2);
        } else {
            playSound(soundID, sfxVolume/4);
        }
	}
}

void SoundPlayer::playSound(Sound_enum soundID, int volume)
{
	if(soundOn) {
		Mix_Chunk* tmp;

		if((tmp = pSFXManager->getSound(soundID)) == nullptr) {
			return;
		}

		int channel = Mix_PlayChannel(-1,tmp, 0);
		if(channel != -1) {
			Mix_Volume(channel, (volume*sfxVolume)/MIX_MAX_VOLUME);
        }
	}
}

void SoundPlayer::playVoice(Voice_enum id, int houseID) {
	if(soundOn) {
		Mix_Chunk* tmp;

		if((tmp = pSFXManager->getVoice(id,houseID)) == nullptr) {
			fprintf(stderr,"There is no voice with id %d!\n",id);
			exit(EXIT_FAILURE);
		}

		int channel = Mix_PlayChannel(-1, tmp, 0);
		if(channel != -1) {
            Mix_Volume(channel, sfxVolume);
        }
	}
}

void SoundPlayer::playSound(Mix_Chunk* sound) {
	if(soundOn) {
		int channel = Mix_PlayChannel(-1, sound, 0);
		if(channel != -1) {
            Mix_Volume(channel, sfxVolume);
        }
	}
}

void SoundPlayer::playSound(Sound_enum id) {
	if(soundOn) {
		Mix_Chunk* tmp;

		if((tmp = pSFXManager->getSound(id)) == nullptr) {
			fprintf(stderr,"There is no sound with id %d!\n",id);
			exit(EXIT_FAILURE);
		}

		int channel = Mix_PlayChannel(-1, tmp, 0);
		if(channel != -1) {
            Mix_Volume(channel, sfxVolume);
        }
	}
}
