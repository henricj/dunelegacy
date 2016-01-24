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

#include <Menu/CampaignStatsMenu.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/music/MusicPlayer.h>
#include <misc/string_util.h>
#include <House.h>
#include <SoundPlayer.h>
#include <Game.h>
#include <structures/StructureBase.h>
#include <units/UnitBase.h>
#include <units/Harvester.h>

#include <climits>
#include <algorithm>

#define max3(a,b,c) (std::max((a),std::max((b),(c))))

#define PROGRESSBARTIME 4000.0f
#define WAITTIME 1000

CampaignStatsMenu::CampaignStatsMenu(int level) : MenuBase()
{
    currentStateStartTime = 0;
    currentState = State_HumanSpice;

    calculateScore(level);

	// set up window
	SDL_Surface *surf;
	surf = pGFXManager->getUIGraphic(UI_GameStatsBackground);

	setBackground(surf,false);
	resize(surf->w,surf->h);

	setWindowWidget(&windowWidget);

    scoreLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
    scoreLabel.setText(strprintf(_("@DUNE.ENG|21#Score: %d"), totalScore));
	windowWidget.addWidget(&scoreLabel, (getSize()/2) + Point(-175, -173), scoreLabel.getSize());

    timeLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
    timeLabel.setText(strprintf(_("@DUNE.ENG|22#Time: %d:%02d"), totalTime/3600, (totalTime%3600)/60));
	windowWidget.addWidget(&timeLabel, (getSize()/2) + Point(+180 - timeLabel.getSize().x, -173), timeLabel.getSize());

    yourRankLabel.setAlignment(Alignment_HCenter);
    yourRankLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
    yourRankLabel.setText(_("@DUNE.ENG|23#You have attained the rank"));
	windowWidget.addWidget(&yourRankLabel, (getSize()/2) + Point(-yourRankLabel.getSize().x/2, -127), yourRankLabel.getSize());

    rankLabel.setAlignment(Alignment_HCenter);
    rankLabel.setText(rank);
	windowWidget.addWidget(&rankLabel, (getSize()/2) + Point(-rankLabel.getSize().x/2, -105), rankLabel.getSize());

    spiceHarvestedByLabel.setTextColor(COLOR_WHITE, COLOR_BLACK, COLOR_THICKSPICE);
    spiceHarvestedByLabel.setAlignment(Alignment_HCenter);
    spiceHarvestedByLabel.setText(_("@DUNE.ENG|26#Spice harvested by"));
	windowWidget.addWidget(&spiceHarvestedByLabel, (getSize()/2) + Point(-spiceHarvestedByLabel.getSize().x/2, -40), spiceHarvestedByLabel.getSize());

    unitsDestroyedByLabel.setTextColor(COLOR_WHITE, COLOR_BLACK, COLOR_THICKSPICE);
    unitsDestroyedByLabel.setAlignment(Alignment_HCenter);
    unitsDestroyedByLabel.setText(_("@DUNE.ENG|24#Units destroyed by"));
	windowWidget.addWidget(&unitsDestroyedByLabel, (getSize()/2) + Point(-unitsDestroyedByLabel.getSize().x/2, 34), unitsDestroyedByLabel.getSize());

    buildingsDestroyedByLabel.setTextColor(COLOR_WHITE, COLOR_BLACK, COLOR_THICKSPICE);
    buildingsDestroyedByLabel.setAlignment(Alignment_HCenter);
    buildingsDestroyedByLabel.setText(_("@DUNE.ENG|25#Buildings destroyed by"));
	windowWidget.addWidget(&buildingsDestroyedByLabel, (getSize()/2) + Point(-buildingsDestroyedByLabel.getSize().x/2, 108), buildingsDestroyedByLabel.getSize());

    // spice statistics

    you1Label.setTextColor(COLOR_WHITE, COLOR_BLACK);
    you1Label.setAlignment(Alignment_Right);
    you1Label.setText(_("@DUNE.ENG|329#You:"));
	windowWidget.addWidget(&you1Label, (getSize()/2) + Point(-229 - you1Label.getSize().x, -23), you1Label.getSize());

	spiceYouShadowProgressBar.setColor(COLOR_BLACK);
	spiceYouShadowProgressBar.setProgress(0.0);
	windowWidget.addWidget(&spiceYouShadowProgressBar, (getSize()/2) + Point(-228 + 2, -15 + 2), Point(440,12));

	spiceYouProgressBar.setColor(houseColor[pLocalHouse->getHouseID()] + 1);
	spiceYouProgressBar.setProgress(0.0);
	windowWidget.addWidget(&spiceYouProgressBar, (getSize()/2) + Point(-228, -15), Point(440,12));

	spiceYouLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
	spiceYouLabel.setAlignment(Alignment_HCenter);
	spiceYouLabel.setVisible(false);
	windowWidget.addWidget(&spiceYouLabel, (getSize()/2) + Point(222, -21), Point(66,21));

    enemy1Label.setTextColor(COLOR_WHITE, COLOR_BLACK);
    enemy1Label.setAlignment(Alignment_Right);
    enemy1Label.setText(_("@DUNE.ENG|330#Enemy:"));
	windowWidget.addWidget(&enemy1Label, (getSize()/2) + Point(-229 - enemy1Label.getSize().x, -5), enemy1Label.getSize());

	spiceEnemyShadowProgressBar.setColor(COLOR_BLACK);
	spiceEnemyShadowProgressBar.setProgress(0.0);
	windowWidget.addWidget(&spiceEnemyShadowProgressBar, (getSize()/2) + Point(-228 + 2, 3 + 2), Point(440,12));

	spiceEnemyProgressBar.setColor(COLOR_SARDAUKAR + 1);
	spiceEnemyProgressBar.setProgress(0.0);
	windowWidget.addWidget(&spiceEnemyProgressBar, (getSize()/2) + Point(-228, 3), Point(440,12));

	spiceEnemyLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
	spiceEnemyLabel.setAlignment(Alignment_HCenter);
	spiceEnemyLabel.setVisible(false);
	windowWidget.addWidget(&spiceEnemyLabel, (getSize()/2) + Point(222, -3), Point(66,21));

    // unit kill statistics

    you2Label.setTextColor(COLOR_WHITE, COLOR_BLACK);
    you2Label.setAlignment(Alignment_Right);
    you2Label.setText(_("@DUNE.ENG|329#You:"));
	windowWidget.addWidget(&you2Label, (getSize()/2) + Point(-229 - you2Label.getSize().x, 51), you2Label.getSize());

	unitsYouShadowProgressBar.setColor(COLOR_BLACK);
	unitsYouShadowProgressBar.setProgress(0.0);
	windowWidget.addWidget(&unitsYouShadowProgressBar, (getSize()/2) + Point(-228 + 2, 59 + 2), Point(440,12));

	unitsYouProgressBar.setColor(houseColor[pLocalHouse->getHouseID()] + 1);
	unitsYouProgressBar.setProgress(0.0);
	windowWidget.addWidget(&unitsYouProgressBar, (getSize()/2) + Point(-228, 59), Point(440,12));

	unitsYouLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
	unitsYouLabel.setAlignment(Alignment_HCenter);
	unitsYouLabel.setVisible(false);
	windowWidget.addWidget(&unitsYouLabel, (getSize()/2) + Point(222, 53), Point(66,21));

    enemy2Label.setTextColor(COLOR_WHITE, COLOR_BLACK);
    enemy2Label.setAlignment(Alignment_Right);
    enemy2Label.setText(_("@DUNE.ENG|330#Enemy:"));
	windowWidget.addWidget(&enemy2Label, (getSize()/2) + Point(-229 - enemy2Label.getSize().x, 69), enemy2Label.getSize());

	unitsEnemyShadowProgressBar.setColor(COLOR_BLACK);
	unitsEnemyShadowProgressBar.setProgress(0.0);
	windowWidget.addWidget(&unitsEnemyShadowProgressBar, (getSize()/2) + Point(-228 + 2, 77 + 2), Point(440,12));

	unitsEnemyProgressBar.setColor(COLOR_SARDAUKAR + 1);
	unitsEnemyProgressBar.setProgress(0.0);
	windowWidget.addWidget(&unitsEnemyProgressBar, (getSize()/2) + Point(-228, 77), Point(440,12));

	unitsEnemyLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
	unitsEnemyLabel.setAlignment(Alignment_HCenter);
	unitsEnemyLabel.setVisible(false);
	windowWidget.addWidget(&unitsEnemyLabel, (getSize()/2) + Point(222, 71), Point(66,21));

    // buildings kill statistics

    you3Label.setTextColor(COLOR_WHITE, COLOR_BLACK);
    you3Label.setAlignment(Alignment_Right);
    you3Label.setText(_("@DUNE.ENG|329#You:"));
	windowWidget.addWidget(&you3Label, (getSize()/2) + Point(-229 - you3Label.getSize().x, 125), you3Label.getSize());

	buildingsYouShadowProgressBar.setColor(COLOR_BLACK);
	buildingsYouShadowProgressBar.setProgress(0.0);
	windowWidget.addWidget(&buildingsYouShadowProgressBar, (getSize()/2) + Point(-228 + 2, 133 + 2), Point(440,12));

	buildingsYouProgressBar.setColor(houseColor[pLocalHouse->getHouseID()] + 1);
	buildingsYouProgressBar.setProgress(0.0);
	windowWidget.addWidget(&buildingsYouProgressBar, (getSize()/2) + Point(-228, 133), Point(440,12));

	buildingsYouLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
	buildingsYouLabel.setAlignment(Alignment_HCenter);
	buildingsYouLabel.setVisible(false);
	windowWidget.addWidget(&buildingsYouLabel, (getSize()/2) + Point(222, 127), Point(66,21));

    enemy3Label.setTextColor(COLOR_WHITE, COLOR_BLACK);
    enemy3Label.setAlignment(Alignment_Right);
    enemy3Label.setText(_("@DUNE.ENG|330#Enemy:"));
	windowWidget.addWidget(&enemy3Label, (getSize()/2) + Point(-229 - enemy2Label.getSize().x, 143), enemy3Label.getSize());

	buildingsEnemyShadowProgressBar.setColor(COLOR_BLACK);
	buildingsEnemyShadowProgressBar.setProgress(0.0);
	windowWidget.addWidget(&buildingsEnemyShadowProgressBar, (getSize()/2) + Point(-228 + 2, 151 + 2), Point(440,12));

	buildingsEnemyProgressBar.setColor(COLOR_SARDAUKAR + 1);
	buildingsEnemyProgressBar.setProgress(0.0);
	windowWidget.addWidget(&buildingsEnemyProgressBar, (getSize()/2) + Point(-228, 151), Point(440,12));

	buildingsEnemyLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
	buildingsEnemyLabel.setAlignment(Alignment_HCenter);
	buildingsEnemyLabel.setVisible(false);
	windowWidget.addWidget(&buildingsEnemyLabel, (getSize()/2) + Point(222, 145), Point(66,21));

}

CampaignStatsMenu::~CampaignStatsMenu() {
}

int CampaignStatsMenu::showMenu()
{
    musicPlayer->changeMusic(MUSIC_GAMESTAT);

    currentStateStartTime = SDL_GetTicks();
    currentState = State_HumanSpice;

    return MenuBase::showMenu();
}

bool CampaignStatsMenu::doInput(SDL_Event &event)
{
    if(event.type == SDL_MOUSEBUTTONUP) {
        if(currentState == State_Finished) {
            quit();
        } else {
            while(currentState != State_Finished) {
                doState(INT_MAX);
            }
        }
    }

    return MenuBase::doInput(event);
}

void CampaignStatsMenu::drawSpecificStuff()
{
    doState(SDL_GetTicks() - currentStateStartTime);
}
void CampaignStatsMenu::doState(int elapsedTime)
{
    switch(currentState) {
        case State_HumanSpice:
        {
            float MaxSpiceHarvested = max3(spiceHarvestedByHuman, spiceHarvestedByAI, 3000.0f);
            float SpiceComplete = std::min(elapsedTime / PROGRESSBARTIME, 1.0f);

            float Human_PercentSpiceComplete;
            if(SpiceComplete < spiceHarvestedByHuman / MaxSpiceHarvested) {
                Human_PercentSpiceComplete = SpiceComplete * 100.0f;
                spiceYouLabel.setText( stringify( (int) (SpiceComplete*MaxSpiceHarvested)));
                soundPlayer->playSound(CreditsTick);
            } else {
                Human_PercentSpiceComplete = spiceHarvestedByHuman * 100.0f / MaxSpiceHarvested;
                spiceYouLabel.setText(stringify( (int) spiceHarvestedByHuman));
                soundPlayer->playSound(Tick);
                currentState = State_Between_HumanSpice_and_AISpice;
                currentStateStartTime = SDL_GetTicks();
            }

            spiceYouLabel.setVisible(true);
            spiceYouProgressBar.setProgress( Human_PercentSpiceComplete );
            spiceYouShadowProgressBar.setProgress( Human_PercentSpiceComplete );
        } break;

        case State_Between_HumanSpice_and_AISpice:
        {
            if(elapsedTime > WAITTIME) {
                currentState = State_AISpice;
                currentStateStartTime = SDL_GetTicks();
            }
        }
        break;

        case State_AISpice:
        {
            float MaxSpiceHarvested = max3(spiceHarvestedByHuman, spiceHarvestedByAI, 3000.0f);
            float SpiceComplete = std::min(elapsedTime / PROGRESSBARTIME, 1.0f);

            float AI_PercentSpiceComplete;
            if(SpiceComplete < spiceHarvestedByAI / MaxSpiceHarvested) {
                AI_PercentSpiceComplete = SpiceComplete * 100.0f;
                spiceEnemyLabel.setText( stringify( (int) (SpiceComplete*MaxSpiceHarvested)));
                soundPlayer->playSound(CreditsTick);
            } else {
                AI_PercentSpiceComplete = spiceHarvestedByAI * 100.0f / MaxSpiceHarvested;
                spiceEnemyLabel.setText(stringify( (int) spiceHarvestedByAI));
                soundPlayer->playSound(Tick);
                currentState = State_Between_AISpice_and_HumanUnits;
                currentStateStartTime = SDL_GetTicks();
            }

            spiceEnemyLabel.setVisible(true);
            spiceEnemyProgressBar.setProgress( AI_PercentSpiceComplete );
            spiceEnemyShadowProgressBar.setProgress( AI_PercentSpiceComplete );
        } break;

        case State_Between_AISpice_and_HumanUnits:
            if(elapsedTime > WAITTIME) {
                currentState = State_HumanUnits;
                currentStateStartTime = SDL_GetTicks();
            }
        break;

        case State_HumanUnits:
        {
            float MaxUnitsDestroyed = max3(unitsDestroyedByHuman, unitsDestroyedByAI, 200);
            float UnitsComplete = std::min(elapsedTime / PROGRESSBARTIME, 1.0f);

            float Human_PercentUnitsComplete;
            if(UnitsComplete < unitsDestroyedByHuman / MaxUnitsDestroyed) {
                Human_PercentUnitsComplete = UnitsComplete * 100.0f;
                unitsYouLabel.setText( stringify( (int) (UnitsComplete*MaxUnitsDestroyed)));
                soundPlayer->playSound(CreditsTick);
            } else {
                Human_PercentUnitsComplete = unitsDestroyedByHuman * 100.0f / MaxUnitsDestroyed;
                unitsYouLabel.setText( stringify(unitsDestroyedByHuman));
                soundPlayer->playSound(Tick);
                currentState = State_Between_HumanUnits_and_AIUnits;
                currentStateStartTime = SDL_GetTicks();
            }

            unitsYouLabel.setVisible(true);
            unitsYouProgressBar.setProgress( Human_PercentUnitsComplete );
            unitsYouShadowProgressBar.setProgress( Human_PercentUnitsComplete );
        } break;

        case State_Between_HumanUnits_and_AIUnits:
            if(elapsedTime > WAITTIME) {
                currentState = State_AIUnits;
                currentStateStartTime = SDL_GetTicks();
            }
        break;

        case State_AIUnits:
        {
            float MaxUnitsDestroyed = max3(unitsDestroyedByHuman, unitsDestroyedByAI, 200);
            float UnitsComplete = std::min(elapsedTime / PROGRESSBARTIME, 1.0f);

            float AI_PercentUnitsComplete;
            if(UnitsComplete < unitsDestroyedByAI / MaxUnitsDestroyed) {
                AI_PercentUnitsComplete = UnitsComplete * 100.0f;
                unitsEnemyLabel.setText( stringify( (int) (UnitsComplete*MaxUnitsDestroyed)));
                soundPlayer->playSound(CreditsTick);
            } else {
                AI_PercentUnitsComplete = unitsDestroyedByAI * 100.0f / MaxUnitsDestroyed;
                unitsEnemyLabel.setText( stringify(unitsDestroyedByAI));
                soundPlayer->playSound(Tick);
                currentState = State_Between_AIUnits_and_HumanBuildings;
                currentStateStartTime = SDL_GetTicks();
            }

            unitsEnemyLabel.setVisible(true);
            unitsEnemyProgressBar.setProgress( AI_PercentUnitsComplete );
            unitsEnemyShadowProgressBar.setProgress( AI_PercentUnitsComplete );
        } break;

        case State_Between_AIUnits_and_HumanBuildings:
            if(elapsedTime > WAITTIME) {
                currentState = State_HumanBuildings;
                currentStateStartTime = SDL_GetTicks();
            }
        break;

        case State_HumanBuildings:
        {
            float MaxBuildingsDestroyed = max3(structuresDestroyedByHuman, structuresDestroyedByAI, 200);
            float BuildingsComplete = std::min(elapsedTime / PROGRESSBARTIME, 1.0f);

            float Human_PercentBuildingsComplete;
            if(BuildingsComplete < structuresDestroyedByHuman / MaxBuildingsDestroyed) {
                Human_PercentBuildingsComplete = BuildingsComplete * 100.0f;
                buildingsYouLabel.setText( stringify( (int) (BuildingsComplete*MaxBuildingsDestroyed)));
                soundPlayer->playSound(CreditsTick);
            } else {
                Human_PercentBuildingsComplete = structuresDestroyedByHuman * 100.0f / MaxBuildingsDestroyed;
                buildingsYouLabel.setText( stringify(structuresDestroyedByHuman));
                soundPlayer->playSound(Tick);
                currentState = State_Between_HumanBuildings_and_AIBuildings;
                currentStateStartTime = SDL_GetTicks();
            }

            buildingsYouLabel.setVisible(true);
            buildingsYouProgressBar.setProgress( Human_PercentBuildingsComplete );
            buildingsYouShadowProgressBar.setProgress( Human_PercentBuildingsComplete );
        } break;

        case State_Between_HumanBuildings_and_AIBuildings:
            if(elapsedTime > WAITTIME) {
                currentState = State_AIBuildings;
                currentStateStartTime = SDL_GetTicks();
            }
        break;

        case State_AIBuildings:
        {
            float MaxBuildingsDestroyed = max3(structuresDestroyedByHuman, structuresDestroyedByAI, 200);
            float BuildingsComplete = std::min(elapsedTime / PROGRESSBARTIME, 1.0f);

            float AI_PercentBuildingsComplete;
            if(BuildingsComplete < structuresDestroyedByAI / MaxBuildingsDestroyed) {
                AI_PercentBuildingsComplete = BuildingsComplete * 100.0f;
                buildingsEnemyLabel.setText( stringify( (int) (BuildingsComplete*MaxBuildingsDestroyed)));
                soundPlayer->playSound(CreditsTick);
            } else {
                AI_PercentBuildingsComplete = structuresDestroyedByAI * 100.0f / MaxBuildingsDestroyed;
                buildingsEnemyLabel.setText( stringify(structuresDestroyedByAI));
                soundPlayer->playSound(Tick);
                currentState = State_Finished;
                currentStateStartTime = SDL_GetTicks();
            }

            buildingsEnemyLabel.setVisible(true);
            buildingsEnemyProgressBar.setProgress( AI_PercentBuildingsComplete );
            buildingsEnemyShadowProgressBar.setProgress( AI_PercentBuildingsComplete );
        } break;

        case State_Finished:
        default:
        {
            // nothing
        } break;
    }
}

void CampaignStatsMenu::calculateScore(int level)
{
    unitsDestroyedByHuman = 0;
	unitsDestroyedByAI = 0;

    structuresDestroyedByHuman = 0;
	structuresDestroyedByAI = 0;

	spiceHarvestedByHuman = 0.0f;
	spiceHarvestedByAI = 0.0f;

	totalTime = currentGame->getGameTime()/1000;

    totalScore = level*45;

    float totalHumanCredits = 0.0f;
	for(int i=0; i < NUM_HOUSES; i++) {
        House* pHouse = currentGame->getHouse(i);
        if(pHouse != NULL) {
            if(pHouse->isAI() == true) {
                unitsDestroyedByAI += pHouse->getNumDestroyedUnits();
                structuresDestroyedByAI += pHouse->getNumDestroyedStructures();
                spiceHarvestedByAI += pHouse->getHarvestedSpice().toFloat();

                totalScore -= pHouse->getDestroyedValue();
            } else {
                unitsDestroyedByHuman += pHouse->getNumDestroyedUnits();
                structuresDestroyedByHuman += pHouse->getNumDestroyedStructures();
                spiceHarvestedByHuman += pHouse->getHarvestedSpice().toFloat();

                totalHumanCredits += pHouse->getCredits();

                totalScore += pHouse->getDestroyedValue();
            }
        }
	}

	totalScore += ((int) totalHumanCredits) / 100;

    for(RobustList<StructureBase*>::const_iterator iter = structureList.begin(); iter != structureList.end(); ++iter) {
        StructureBase* pStructure = *iter;
        if(pStructure->getOwner()->isAI() == false) {
            totalScore += currentGame->objectData.data[pStructure->getItemID()][pStructure->getOriginalHouseID()].price / 100;
        }
    }

    totalScore -= ((totalTime/60) + 1);

    for(RobustList<UnitBase*>::const_iterator iter = unitList.begin(); iter != unitList.end(); ++iter) {
        UnitBase* pUnit = *iter;
        if(pUnit->getItemID() == Unit_Harvester) {
            Harvester* pHarvester = (Harvester*) pUnit;
            if(pHarvester->getOwner()->isAI() == true) {
                spiceHarvestedByAI += pHarvester->getAmountOfSpice().toFloat();
            } else {
                spiceHarvestedByHuman += pHarvester->getAmountOfSpice().toFloat();
            }
        }
    }

    if(currentGame->areCheatsEnabled() == true) {
        rank = "Cheater";
    } else {

        if(totalScore >= 1400)       rank = _("@DUNE.ENG|282#Emperor");
        else if(totalScore >= 1000)  rank = _("@DUNE.ENG|281#Ruler of Arrakis");
        else if(totalScore >= 700)   rank = _("@DUNE.ENG|280#Chief Warlord");
        else if(totalScore >= 500)   rank = _("@DUNE.ENG|279#Warlord");
        else if(totalScore >= 400)   rank = _("@DUNE.ENG|278#Base Commander");
        else if(totalScore >= 300)   rank = _("@DUNE.ENG|277#Outpost Commander");
        else if(totalScore >= 200)   rank = _("@DUNE.ENG|276#Squad Leader");
        else if(totalScore >= 150)   rank = _("@DUNE.ENG|275#Dune Trooper");
        else if(totalScore >= 100)   rank = _("@DUNE.ENG|274#Sand Warrior");
        else if(totalScore >= 50)    rank = _("@DUNE.ENG|273#Desert Mongoose");
        else if(totalScore >= 25)    rank = _("@DUNE.ENG|272#Sand Snake");
        else                         rank = _("@DUNE.ENG|271#Sand Flea");
    }
}
