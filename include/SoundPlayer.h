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

#ifndef SOUNDPLAYER_H
#define SOUNDPLAYER_H

#include <FileClasses/SFXManager.h>

#include <SDL2/SDL_mixer.h>

// forward declaration
class Coord;

/*!
    Class that handles sounds and music.
*/
class SoundPlayer
{
public:
    //! @name Constructor & Destructor
    //@{
    SoundPlayer();
    ~SoundPlayer();
    //@}

    /*!
        plays a certain sound at certain coordinates.
        the volume of sound depends on the difference between
        location of the sound and if location is explored.
        @param soundID id of the sound to be played
        @param location coordinates where the sound is to be played
    */
    void playSoundAt(Sound_enum soundID, const Coord& location);

    /*!
        Toggle the sound on and off
    */
    void toggleSound() { soundOn = !soundOn; }

    /*!
        turns sound playing on or off
        @param value when true the function turns sfx on
    */
    void setSound(bool value) { soundOn = value; }

    void playVoice(Voice_enum id, int houseID);

    void playSound(Mix_Chunk* sound);

    void playSound(Sound_enum id);

    /**
        Gets the current sfx volume.
        \return the current volume
    */
    inline int getSfxVolume() const { return sfxVolume; };

    /**
        Sets the volume of all channels
        \param  newVolume   the new volume [0;MIX_MAX_VOLUME]
    */
    void setSfxVolume(int newVolume) {
        if(newVolume >= 0 && newVolume <= MIX_MAX_VOLUME) {
            sfxVolume = newVolume;
        }
    }

private:
    enum class ChannelGroup {
        Voice,
        UI,
        Credits,
        Explosion,
        ExplosionStructure,
        Gun,
        Rocket,
        Scream,
        Sonic,
        Other
    };

    /*!
        the function plays a sound with a given volume
        @param soundID id of a sound to be played
        @param volume sound will be played with this volume
    */
    void playSound(Sound_enum soundID, int volume);

    //! whether sound should be played
    bool    soundOn;

    //! volume of sound effects
    int sfxVolume;

};

#endif //SOUNDPLAYER_H
