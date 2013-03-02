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

#include <SDL_mixer.h>
#include <DataTypes.h>

#include <string>

#define NUM_MAPCHOICEPIECES	28
#define NUM_MAPCHOICEARROWS	9

// Voice
typedef enum {
	HarvesterDeployed,
	ConstructionComplete,
	VehicleRepaired,
	FrigateHasArrived,
	YourMissionIsComplete,
	YouHaveFailedYourMission,
	RadarActivated,
	RadarDeactivated,
	BloomLocated,
	WarningWormSign,
	BaseIsUnderAttack,  ///< unused
    SaboteurApproaching,
    MissileApproaching,
	NUM_VOICE
} Voice_enum;

// Sound
typedef enum {
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
	PlaceStructure,
	ButtonClick,
	InvalidAction,
	CreditsTick,
	Tick,
	RadarNoise,
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

	Mix_Chunk*		getVoice(Voice_enum id, int house);
	Mix_Chunk*		getSound(Sound_enum id);

private:
    Mix_Chunk*      loadMixFromADL(std::string adlFile, int index);

	void			loadEnglishVoice();
	Mix_Chunk* 		getEnglishVoice(Voice_enum id, int house);

	void			loadNonEnglishVoice(std::string languagePrefix);
	Mix_Chunk* 		getNonEnglishVoice(Voice_enum id, int house);

	Mix_Chunk**		lngVoice;
	int				numLngVoice;
	Mix_Chunk*		soundChunk[NUM_SOUNDCHUNK];
};

#endif // SFXMANAGER_H
