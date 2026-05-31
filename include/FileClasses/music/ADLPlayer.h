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

#ifndef ADLPLAYER_H
#define ADLPLAYER_H

#include <Audio/AudioEngine.h>
#include <FileClasses/music/MusicPlayer.h>

class ADLPlayer final : public MusicPlayer {
    using parent = MusicPlayer;

public:
    ADLPlayer();
    ~ADLPlayer() override;

    ADLPlayer(const ADLPlayer&)            = delete;
    ADLPlayer(ADLPlayer&&)                 = delete;
    ADLPlayer& operator=(const ADLPlayer&) = delete;
    ADLPlayer& operator=(ADLPlayer&&)      = delete;

    /*!
        change type of current music
        @param musicType type of music to be played
    */
    void changeMusic(MUSICTYPE musicType) override;

    /*!
        Toggle the music on and off
    */
    void toggleSound() override;

    /**
        Returns whether music is currently being played
        \return true = currently playing, false = not playing
    */
    bool isMusicPlaying() override;

    /*!
        turns music playing on or off
        @param value when true the function turns music on
    */
    void setMusic(bool value) override;

    /**
        Sets the volume of the music channel
        \param  newVolume   the new volume [0;MIX_MAX_VOLUME]
    */
    void setMusicVolume(int newVolume) override;

private:
    int playbackFrequency_{};
    mix_track_ptr track_;
    mix_audio_ptr audio_;
};

#endif // ADLPLAYER_H
