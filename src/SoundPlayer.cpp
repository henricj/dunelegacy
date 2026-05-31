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

#include <Audio/AudioEngine.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>

#include <misc/exceptions.h>

SoundPlayer::SoundPlayer() : soundOn(false), sfxVolume(dune::globals::settings.audio.sfxVolume) {

    if (!dune::globals::pSFXManager)
        return;

    auto* const engine = dune::globals::pAudioEngine.get();
    if (!engine || !engine->isOpen())
        return;

    // Pool sizes mirror the original SDL2 Mix_GroupChannels ranges (total = 24 tracks):
    // clang-format off
    static constexpr int kGroupPoolSizes[kNumGroups] = {
        2, // Voice
        2, // UI
        2, // Credits
        3, // Explosion
        2, // ExplosionStructure
        3, // Gun
        3, // Rocket
        2, // Scream
        3, // Sonic
        2, // Other
    };
    // clang-format on
    static_assert(std::size(kGroupPoolSizes) == kNumGroups);

    for (int g = 0; g < kNumGroups; ++g) {
        auto& pool = trackPools_[g];
        pool.reserve(kGroupPoolSizes[g]);
        for (int i = 0; i < kGroupPoolSizes[g]; ++i) {
            if (auto track = engine->createTrack())
                pool.push_back(std::move(track));
        }
    }

    soundOn = dune::globals::settings.audio.playSFX;
}

SoundPlayer::~SoundPlayer() = default;

void SoundPlayer::playVoice(Voice_enum id, HOUSETYPE houseID) const {
    if (!soundOn || !dune::globals::pSFXManager)
        return;

    const auto voice_index = static_cast<int>(id);
    if (voice_index < 0 || voice_index >= static_cast<int>(Voice_enum::NUM_VOICE))
        THROW(std::invalid_argument, "The voice ID {} is invalid!", voice_index);

    Mix_Chunk* const tmp = dune::globals::pSFXManager->getVoice(id, houseID);
    if (!tmp)
        THROW(std::invalid_argument, "There is no voice with ID {}!", voice_index);

    playChunk(tmp, ChannelGroup::Voice, sfxVolume);
}

void SoundPlayer::playSoundAt(Sound_enum soundID, const Coord& location) const {
    if (!soundOn)
        return;

    auto* const currentGameMap = dune::globals::currentGameMap;

    if (!currentGameMap->tileExists(location)
        || !currentGameMap->getTile(location)->isExploredByTeam(dune::globals::currentGame.get(),
                                                                dune::globals::pLocalHouse->getTeamID())) {
        return;
    }

    const auto realCoord = location * TILESIZE + Coord(TILESIZE / 2, TILESIZE / 2);

    const auto* const screenborder = dune::globals::screenborder.get();

    if (screenborder->isInsideScreen(realCoord, Coord(TILESIZE, TILESIZE))) {
        playSound(soundID, sfxVolume);
    } else if (screenborder->isInsideScreen(realCoord, Coord(TILESIZE * 16, TILESIZE * 16))) {
        playSound(soundID, (sfxVolume * 3) / 4);
    } else if (screenborder->isInsideScreen(realCoord, Coord(TILESIZE * 24, TILESIZE * 24))) {
        playSound(soundID, sfxVolume / 2);
    } else {
        playSound(soundID, sfxVolume / 4);
    }
}

void SoundPlayer::toggleSound() noexcept {
    soundOn = !soundOn && dune::globals::pSFXManager;
}

void SoundPlayer::setSound(bool value) noexcept {
    soundOn = value && dune::globals::pSFXManager;
}

void SoundPlayer::playSound(Mix_Chunk* sound) const {
    if (!soundOn)
        return;

    playChunk(sound, ChannelGroup::Other, sfxVolume);
}

void SoundPlayer::playSound(Sound_enum id) const {
    playSound(id, sfxVolume);
}

void SoundPlayer::playSound(Sound_enum soundID, int volume) const {
    if (!soundOn || !dune::globals::pSFXManager)
        return;

    const auto sound_index = static_cast<int>(soundID);
    if (sound_index < 0 || sound_index >= static_cast<int>(Sound_enum::NUM_SOUNDCHUNK))
        THROW(std::invalid_argument, "The sound ID {} is invalid!", sound_index);

    static constexpr ChannelGroup soundID2ChannelGroup[] = {
        ChannelGroup::UI,                 // Sound_PlaceStructure
        ChannelGroup::UI,                 // Sound_ButtonClick
        ChannelGroup::UI,                 // Sound_InvalidAction
        ChannelGroup::Credits,            // Sound_CreditsTick
        ChannelGroup::Credits,            // Sound_CreditsTickDown
        ChannelGroup::Credits,            // Sound_Tick
        ChannelGroup::UI,                 // Sound_RadarNoise
        ChannelGroup::Explosion,          // Sound_ExplosionGas
        ChannelGroup::Explosion,          // Sound_ExplosionTiny
        ChannelGroup::Explosion,          // Sound_ExplosionSmall
        ChannelGroup::Explosion,          // Sound_ExplosionMedium
        ChannelGroup::Explosion,          // Sound_ExplosionLarge
        ChannelGroup::ExplosionStructure, // Sound_ExplosionStructure
        ChannelGroup::Other,              // Sound_WormAttack
        ChannelGroup::Gun,                // Sound_Gun
        ChannelGroup::Rocket,             // Sound_Rocket
        ChannelGroup::Explosion,          // Sound_Bloom
        ChannelGroup::Scream,             // Sound_Scream1
        ChannelGroup::Scream,             // Sound_Scream2
        ChannelGroup::Scream,             // Sound_Scream3
        ChannelGroup::Scream,             // Sound_Scream4
        ChannelGroup::Scream,             // Sound_Scream5
        ChannelGroup::Scream,             // Sound_Trumpet
        ChannelGroup::Other,              // Sound_Drop
        ChannelGroup::Scream,             // Sound_Squashed
        ChannelGroup::Gun,                // Sound_MachineGun
        ChannelGroup::Sonic,              // Sound_Sonic
        ChannelGroup::Rocket,             // Sound_RocketSmall
    };

    static_assert(static_cast<size_t>(Sound_enum::NUM_SOUNDCHUNK) == std::size(soundID2ChannelGroup));

    Mix_Chunk* const sound = dune::globals::pSFXManager->getSound(soundID);
    if (!sound)
        THROW(std::invalid_argument, "There is no sound with ID {}!", sound_index);

    playChunk(sound, soundID2ChannelGroup[sound_index], volume);
}

void SoundPlayer::playChunk(Mix_Chunk* chunk, ChannelGroup group, int volume) const {
    auto* const engine = dune::globals::pAudioEngine.get();
    if (!engine || !chunk)
        return;

    auto* const audio = Mix_GetChunkAudio(chunk);
    if (!audio)
        return;

    auto* const track = acquireTrack(group);
    if (!track)
        return;

    const float gain = static_cast<float>(volume) / MIX_MAX_VOLUME;
    engine->setTrackGain(track, gain);
    engine->setTrackAudio(track, audio);
    engine->playTrack(track);
}

MIX_Track* SoundPlayer::acquireTrack(ChannelGroup group) const noexcept {
    auto* const engine = dune::globals::pAudioEngine.get();
    if (!engine)
        return nullptr;

    const auto& pool = trackPools_[static_cast<int>(group)];
    for (const auto& track : pool) {
        if (!engine->isTrackPlaying(track.get()))
            return track.get();
    }

    // All tracks in this group are busy — drop the sound, matching
    // SDL2 Mix_GroupAvailable returning -1 when no channel is free.
    return nullptr;
}
