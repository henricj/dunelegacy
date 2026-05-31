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

#include <Audio/AudioEngine.h>
#include <FileClasses/SFXManager.h>

#include <misc/dune_sdl_mixer.h>

#include <array>
#include <vector>

// forward declaration
class Coord;

/*!
    Class that handles sounds and music.
*/
class SoundPlayer {
public:
    //! @name Constructor & Destructor
    //@{
    SoundPlayer();
    ~SoundPlayer();
    //@}

    SoundPlayer(const SoundPlayer&)            = delete;
    SoundPlayer(SoundPlayer&&)                 = delete;
    SoundPlayer& operator=(const SoundPlayer&) = delete;
    SoundPlayer& operator=(SoundPlayer&&)      = delete;
    /*!
        plays a certain sound at certain coordinates.
        the volume of sound depends on the difference between
        location of the sound and if location is explored.
        @param soundID id of the sound to be played
        @param location coordinates where the sound is to be played
    */
    void playSoundAt(Sound_enum soundID, const Coord& location) const;

    /*!
        Toggle the sound on and off
    */
    void toggleSound() noexcept;

    /*!
        turns sound playing on or off
        @param value when true the function turns sfx on
    */
    void setSound(bool value) noexcept;

    void playVoice(Voice_enum id, HOUSETYPE houseID) const;

    void playSound(Mix_Chunk* sound) const;

    void playSound(Sound_enum id) const;

    /**
        Gets the current sfx volume.
        \return the current volume
    */
    [[nodiscard]] int getSfxVolume() const { return sfxVolume; }

    /**
        Sets the volume of all channels
        \param  newVolume   the new volume [0;MIX_MAX_VOLUME]
    */
    void setSfxVolume(int newVolume) {
        if (newVolume >= 0 && newVolume <= MIX_MAX_VOLUME) {
            sfxVolume = newVolume;
        }
    }

private:
    enum class ChannelGroup { Voice, UI, Credits, Explosion, ExplosionStructure, Gun, Rocket, Scream, Sonic, Other };
    static constexpr int kNumGroups = static_cast<int>(ChannelGroup::Other) + 1;

    /*!
        the function plays a sound with a given volume
        @param soundID id of a sound to be played
        @param volume sound will be played with this volume
    */
    void playSound(Sound_enum soundID, int volume) const;

    /*!
        Routes a chunk through the appropriate track pool at the given volume.
        @param chunk  the Mix_Chunk whose backend audio to play
        @param group  the channel group pool to pick a track from
        @param volume playback volume [0; MIX_MAX_VOLUME]
    */
    void playChunk(Mix_Chunk* chunk, ChannelGroup group, int volume) const;

    /*!
        Returns the first idle track in the pool for @p group, or nullptr if
        all tracks in the pool are currently playing (sound is dropped, matching
        the SDL2 Mix_GroupAvailable(-1) semantics).
    */
    [[nodiscard]] MIX_Track* acquireTrack(ChannelGroup group) const noexcept;

    //! whether sound should be played
    bool soundOn;

    //! volume of sound effects
    int sfxVolume;

    //! pre-allocated track pools, one per ChannelGroup
    std::array<std::vector<mix_track_ptr>, kNumGroups> trackPools_;
};

#endif // SOUNDPLAYER_H
