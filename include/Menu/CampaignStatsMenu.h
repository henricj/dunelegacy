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

#ifndef CAMPAIGNSTATSMENU_H
#define CAMPAIGNSTATSMENU_H

#include "MenuBase.h"

#include <GUI/StaticContainer.h>
#include <GUI/Label.h>
#include <GUI/ProgressBar.h>

#include <string>

class CampaignStatsMenu : public MenuBase {
public:
    CampaignStatsMenu(int level);
    virtual ~CampaignStatsMenu();

    virtual int showMenu();

    virtual bool doInput(SDL_Event &event);

    virtual void drawSpecificStuff();

private:
    void doState(int elapsedTime);

    void calculateScore(int level);

    typedef enum {
        State_HumanSpice,
        State_Between_HumanSpice_and_AISpice,
        State_AISpice,
        State_Between_AISpice_and_HumanUnits,
        State_HumanUnits,
        State_Between_HumanUnits_and_AIUnits,
        State_AIUnits,
        State_Between_AIUnits_and_HumanBuildings,
        State_HumanBuildings,
        State_Between_HumanBuildings_and_AIBuildings,
        State_AIBuildings,
        State_Finished
    } CampaignStatsState;


    StaticContainer windowWidget;
    Label scoreLabel;
    Label timeLabel;

    Label yourRankLabel;
    Label rankLabel;

    // spice statistics
    Label       spiceHarvestedByLabel;
    Label       you1Label;
    ProgressBar spiceYouShadowProgressBar;
    ProgressBar spiceYouProgressBar;
    Label       spiceYouLabel;
    Label       enemy1Label;
    ProgressBar spiceEnemyShadowProgressBar;
    ProgressBar spiceEnemyProgressBar;
    Label       spiceEnemyLabel;

    // units statistics
    Label       unitsDestroyedByLabel;
    Label       you2Label;
    ProgressBar unitsYouShadowProgressBar;
    ProgressBar unitsYouProgressBar;
    Label       unitsYouLabel;
    Label       enemy2Label;
    ProgressBar unitsEnemyShadowProgressBar;
    ProgressBar unitsEnemyProgressBar;
    Label       unitsEnemyLabel;

    // buildings statistics
    Label       buildingsDestroyedByLabel;
    Label       you3Label;
    ProgressBar buildingsYouShadowProgressBar;
    ProgressBar buildingsYouProgressBar;
    Label       buildingsYouLabel;
    Label       enemy3Label;
    ProgressBar buildingsEnemyShadowProgressBar;
    ProgressBar buildingsEnemyProgressBar;
    Label       buildingsEnemyLabel;

    int currentStateStartTime;
    CampaignStatsState currentState;

    int unitsDestroyedByHuman;
    int unitsDestroyedByAI;

    int structuresDestroyedByHuman;
    int structuresDestroyedByAI;

    float spiceHarvestedByHuman;
    float spiceHarvestedByAI;

    int totalTime;
    int totalScore;

    std::string rank;
};

#endif //CAMPAIGNSTATSMENU_H
