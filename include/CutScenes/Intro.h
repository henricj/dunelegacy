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

#ifndef INTRO_H
#define INTRO_H

#include <array>
#include <string>
#include <SDL2/SDL_mixer.h>

#include <misc/sound_util.h>
#include <misc/SDL2pp.h>

#include <CutScenes/CutScene.h>
#include <FileClasses/Wsafile.h>

/**
    This class is for showing the intro video.
*/
class Intro : public CutScene
{
public:

    /// Default constructor
    Intro();

    /// destructor
    virtual ~Intro();

private:

    /// \cond
    enum IntroText {
        IntroText_The_Battle_for_Arrakis = 2,
        IntroText_The_planet_Arrakis = 3,
        IntroText_Land_of_sand = 4,
        IntroText_Home_of_the_Spice_Melange = 5,
        IntroText_Spice_controls_the_Empire = 6,
        IntroText_Whoever_controls_Dune = 7,
        IntroText_The_Emperor_has_proposed = 8,
        IntroText_The_House_that_produces = 9,
        IntroText_There_are_no_set_territories = 10,
        IntroText_And_no_rules_of_engagement = 11,
        IntroText_Vast_armies_have_arrived = 12,
        IntroText_Now_three_houses_fight = 13,
        IntroText_The_noble_Atreides = 14,
        IntroText_The_insidious_Ordos = 15,
        IntroText_And_the_evil_Harkonnen = 16,
        IntroText_Only_one_House_will_prevail = 17,
        IntroText_Your_battle_for_Dune_begins = 18,
        IntroText_NOW = 19
    };
    /// \endcond

    /// \cond
    enum IntroVoice {
        Voice_The_building = 0,
        Voice_of_a_Dynasty,

        Voice_The_Planet_Arrakis,
        Voice_Known_As_Dune,

        Voice_Land_of_sand,
        Voice_Home,
        Voice_of_the_spice,
        Voice_Melange,

        Voice_The_spice,
        Voice_controls,
        Voice_the_Empire,
        Voice_Whoever,
        Voice_controls_dune,
        Voice_controls_the_spice,

        Voice_The_Emperor,
        Voice_has_proposed,
        Voice_to_each_of_the_houses,

        Voice_The_House,
        Voice_that_produces,
        Voice_the_most_spice,
        Voice_will_control_dune,

        Voice_There_are_no_set,
        Voice_territories,
        Voice_and_no,
        Voice_rules_of_engagment,

        Voice_Vast_armies,
        Voice_have_arrived,

        Voice_Now,
        Voice_three_Houses_fight,
        Voice_for_control,
        Voice_of_Dune,

        Voice_The_noble_Atreides,

        Voice_The_insidious,
        Voice_Ordos,

        Voice_And_the,
        Voice_evil_Harkonnen,

        Voice_Only_one_house,
        Voice_will_prevail,

        Voice_Your,
        Voice_battle_for_Dune,
        Voice_begins,

        Voice_Now_Now,

        Voice_NUM_ENTRIES
    };
    /// \endcond

    static const char* VoiceFileNames[Voice_NUM_ENTRIES];   ///< List of all the voice files

    sdl2::mix_chunk_ptr  voice[Voice_NUM_ENTRIES];          ///< All the loaded voices

    std::unique_ptr<Wsafile> pDuneText;         ///< 1. video sequence showing the dune text
    std::unique_ptr<Wsafile> pPlanet;           ///< 2. video sequence showing the planet
    std::unique_ptr<Wsafile> pSandstorm;        ///< 3. video sequence showing the sandstorm
    std::unique_ptr<Wsafile> pHarvesters;       ///< 4. video sequence showing two harvesters
    std::unique_ptr<Wsafile> pPalace;           ///< 5. video sequence showing the palace of the imperator
    std::unique_ptr<Wsafile> pImperator;        ///< 6. video sequence showing the imperator talking
    std::unique_ptr<Wsafile> pStarport;         ///< 7. video sequence showing the armies arriving at the starport
    std::unique_ptr<Wsafile> pOrdos;            ///< 8. video sequence showing two ordos launchers/deviators
    std::unique_ptr<Wsafile> pAtreides;         ///< 9. video sequence showing two atreides ornithopters
    std::unique_ptr<Wsafile> pHarkonnen;        ///< 10. video sequence showing two harkonnen troopers under attack
    std::unique_ptr<Wsafile> pDestroyedTank;    ///< 11. video sequence showing destroyed tanks

    sdl2::mix_chunk_ptr  wind;               ///< SFX: wind blowing
    sdl2::mix_chunk_ptr  carryallLanding;    ///< SFX: carryall loading a harvester
    sdl2::mix_chunk_ptr  harvester;          ///< SFX: harvester stopping
    sdl2::mix_chunk_ptr  gunshot;            ///< SFX: a gunshot
    sdl2::mix_chunk_ptr  glass;              ///< SFX: broken glass, destroyed by atreides ornithopters
    sdl2::mix_chunk_ptr  missle;             ///< SFX: missle launched
    sdl2::mix_chunk_ptr  blaster;            ///< SFX: trooper hit
    sdl2::mix_chunk_ptr  blowup1;            ///< SFX: explosion
    sdl2::mix_chunk_ptr  blowup2;            ///< SFX: explosion
};

#endif // INTRO_H
