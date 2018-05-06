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

#ifndef MEANWHILE_H
#define MEANWHILE_H

#include <CutScenes/CutScene.h>
#include <FileClasses/Wsafile.h>

/**
    This class is for showing the meanwhile videos after mission 4 and 8.
*/
class Meanwhile : public CutScene
{
public:
    /**
        Constructor
        \param  house           the house for which the video is shown
        \param  firstMeanwhile  true = video after mission 4, false = video after mission 8
    */
    Meanwhile(int house, bool firstMeanwhile);

    /// destructor
    virtual ~Meanwhile();

private:
    static const int MeanwhileText_Base = 287;              ///< the index of the first text in Dune.lng
    static const int MeanwhileText_NumTextsPerHouse = 11;   ///< number of texts per house

    /// \cond
    enum MeanwhileText {
        /* Meanwhile after mission 4 */
        /* Atreides, Ordos and Harkonnen */
        MeanwhileText_At_the_Emperor_s_Palace = 0,
        MeanwhileText_You_of_all_people = 1,
        MeanwhileText_Yes_your_excellency_I = 2,
        MeanwhileText_You_let_the = 3,
        MeanwhileText_I_did_not_let = 4,
        MeanwhileText_I_will_not_allow = 5,

        /* Meanwhile after mission 8 */
        /* Atreides */
        MeanwhileText_At_the_Emperor_s_Palace_on_Dune = 6,
        MeanwhileText_Fools = 7,
        MeanwhileText_And_still_you_fail = 8,
        MeanwhileText_But_excell = 9,
        MeanwhileText_Enough_Together_we_must = 10,
        /* Ordos and Harkonnen (have the same order but different text) */
        MeanwhileText_The_Ordos_were_not_supposed = 7,
        MeanwhileText_Your_highness = 8,
        MeanwhileText_No_more_explanations = 9,
        MeanwhileText_Only_together_will_we = 10
    };
    /// \endcond

    std::unique_ptr<Wsafile> pMeanwhile;    ///< the video elements not showing the imperator. This video sequence is not shown continuesly but interrupted by the imperator
    std::unique_ptr<Wsafile> pImperator;    ///< the imperator talking
};

#endif // MEANWHILE_H
