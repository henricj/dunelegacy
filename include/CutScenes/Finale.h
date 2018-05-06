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

#ifndef FINALE_H
#define FINALE_H

#include <SDL2/SDL_mixer.h>
#include <misc/sound_util.h>

#include <CutScenes/CutScene.h>
#include <FileClasses/Wsafile.h>

/**
    This class is for showing the finale video after mission 9.
*/
class Finale : public CutScene
{
public:
    /**
        Constructor
        \param  house           the house for which the video is shown
    */
    explicit Finale(int house);

    /// destructor
    virtual ~Finale();

private:

    /// \cond
    enum FinaleText{
        FinaleText_Greetings_Emperor = 20,
        FinaleText_What_is_the_meaning = 21,
        FinaleText_You_are_formally_charged = 22,
        FinaleText_The_House_shall_determine = 23,
        FinaleText_Until_then_you_shall_no = 24,
        FinaleText_You_are_indeed_not_entirely = 25,
        FinaleText_You_have_lied_to_us = 26,
        FinaleText_What_lies_What_are = 27,
        FinaleText_Your_lies_of_loyalty = 28,
        FinaleText_A_crime_for_which_you = 29,
        FinaleText_with_your_life = 30,
        FinaleText_NO_NO_NOOO = 31,
        FinaleText_You_are_aware_Emperor = 32,
        FinaleText_What_games_What_are_you = 33,
        FinaleText_I_am_referring_to_your_game = 34,
        FinaleText_We_were_your_pawns_and_Dune = 35,
        FinaleText_We_have_decided_to_take = 36,
        FinaleText_You_are_to_be_our_pawn = 37
    };
    /// \endcond


    std::unique_ptr<Wsafile> pPalace1;              ///< video sequence showing the palace and the intruders
    std::unique_ptr<Wsafile> pPalace2;              ///< video sequence showing the palace after the imperator was degraded
    std::unique_ptr<Wsafile> pImperator;            ///< video sequence showing the imperator taking
    std::unique_ptr<Wsafile> pImperatorShocked;     ///< video sequence showing the imperator shocked

    sdl2::mix_chunk_ptr  lizard;     ///< SFX: the lizard barking
    sdl2::mix_chunk_ptr  glass;      ///< SFX: glass bursting
    sdl2::mix_chunk_ptr  click;      ///< SFX: loading the gun
    sdl2::mix_chunk_ptr  blaster;    ///< SFX: shooting the gun
    sdl2::mix_chunk_ptr  blowup;     ///< SFX: explosion
};

#endif // FINALE_H
