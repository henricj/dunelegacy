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

#ifndef SFXMANAGER_H
#define SFXMANAGER_H

#include <SDL2/SDL_mixer.h>
#include <DataTypes.h>
#include <misc/sound_util.h>

#include <string>

#define NUM_MAPCHOICEPIECES 28
#define NUM_MAPCHOICEARROWS 9

// Voice
typedef enum {
    HarvesterDeployed,
    UnitDeployed,
    UnitLaunched,
    ConstructionComplete,
    VehicleRepaired,
    FrigateHasArrived,
    YourMissionIsComplete,
    YouHaveFailedYourMission,
    RadarActivated,
    RadarDeactivated,
    BloomLocated,
    WarningWormSign,
    BaseIsUnderAttack,
    SaboteurApproaching,
    MissileApproaching,
    YesSir,
    Reporting,
    Acknowledged,
    Affirmative,
    MovingOut,
    InfantryOut,
    SomethingUnderTheSand,
    HouseHarkonnen,
    HouseAtreides,
    HouseOrdos,
    NUM_VOICE
} Voice_enum;

// Sound
typedef enum {
    Sound_PlaceStructure,
    Sound_ButtonClick,
    Sound_InvalidAction,
    Sound_CreditsTick,
    Sound_Tick,
    Sound_RadarNoise,
    Sound_ExplosionGas,
    Sound_ExplosionTiny,
    Sound_ExplosionSmall,
    Sound_ExplosionMedium,
    Sound_ExplosionLarge,
    Sound_ExplosionStructure,
    Sound_WormAttack,
    Sound_Gun,
    Sound_Rocket,
    Sound_Bloom,
    Sound_Scream1,
    Sound_Scream2,
    Sound_Scream3,
    Sound_Scream4,
    Sound_Scream5,
    Sound_Trumpet,
    Sound_Drop,
    Sound_Squashed,
    Sound_MachineGun,
    Sound_Sonic,
    Sound_RocketSmall,
    NUM_SOUNDCHUNK
} Sound_enum;


class SFXManager {
public:
    SFXManager();
    ~SFXManager();

    Mix_Chunk*      getVoice(Voice_enum id, int house);
    Mix_Chunk*      getSound(Sound_enum id);

private:
    sdl2::mix_chunk_ptr loadMixFromADL(const std::string& adlFile, int index, int volume = MIX_MAX_VOLUME/2) const;

    void            loadEnglishVoice();
    Mix_Chunk*      getEnglishVoice(Voice_enum id, int house) const;

    void            loadNonEnglishVoice(const std::string& languagePrefix);
    Mix_Chunk*      getNonEnglishVoice(Voice_enum id, int house) const;

    std::vector<sdl2::mix_chunk_ptr> lngVoice;
    std::array<sdl2::mix_chunk_ptr, NUM_SOUNDCHUNK> soundChunk;
};

#endif // SFXMANAGER_H
