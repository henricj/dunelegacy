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
#include <Game.h>
#include <House.h>
#include <SoundPlayer.h>
#include <fmt/printf.h>
#include <structures/StructureBase.h>
#include <units/Harvester.h>
#include <units/UnitBase.h>

#include <algorithm>
#include <cmath>

namespace {
constexpr auto max3(float a, float b, float c) {
    return std::max({a, b, c});
}

inline constexpr auto PROGRESSBARTIME = 4000.0f;
inline constexpr auto WAITTIME        = dune::as_dune_clock_duration(1000);
} // namespace

CampaignStatsMenu::CampaignStatsMenu(int level) {
    // We are a top level window
    const auto size = getRendererSize();

    CampaignStatsMenu::resize(size.w, size.h);

    calculateScore(level);

    const Uint32 colorYou = SDL2RGB(
        dune::globals::palette
            [dune::globals::houseToPaletteIndex[static_cast<int>(dune::globals::pLocalHouse->getHouseID())] + 1]);
    const Uint32 colorEnemy = SDL2RGB(dune::globals::palette[PALCOLOR_SARDAUKAR + 1]);

    // set up window
    const auto* pBackground = dune::globals::pGFXManager->getUIGraphic(UI_GameStatsBackground);
    setBackground(pBackground);

    CampaignStatsMenu::setWindowWidget(&windowWidget);

    scoreLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
    scoreLabel.setText(fmt::sprintf(_("@DUNE.ENG|21#Score: %d"), totalScore));
    windowWidget.addWidget(&scoreLabel, getSize() / 2 + Point(-175, -172), scoreLabel.getSize());

    timeLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
    timeLabel.setText(fmt::sprintf(_("@DUNE.ENG|22#Time: %d:%02d"), totalTime / 3600, totalTime % 3600 / 60));
    windowWidget.addWidget(&timeLabel, getSize() / 2 + Point(+180 - timeLabel.getSize().x, -172), timeLabel.getSize());

    yourRankLabel.setAlignment(Alignment_HCenter);
    yourRankLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
    yourRankLabel.setText(_("@DUNE.ENG|23#You have attained the rank"));
    windowWidget.addWidget(&yourRankLabel, getSize() / 2 + Point(-yourRankLabel.getSize().x / 2, -126),
                           yourRankLabel.getSize());

    rankLabel.setAlignment(Alignment_HCenter);
    rankLabel.setText(rank);
    windowWidget.addWidget(&rankLabel, getSize() / 2 + Point(-rankLabel.getSize().x / 2, -104), rankLabel.getSize());

    spiceHarvestedByLabel.setTextColor(COLOR_WHITE, COLOR_BLACK, COLOR_THICKSPICE);
    spiceHarvestedByLabel.setAlignment(Alignment_HCenter);
    spiceHarvestedByLabel.setText(_("@DUNE.ENG|26#Spice harvested by"));
    windowWidget.addWidget(&spiceHarvestedByLabel, getSize() / 2 + Point(-spiceHarvestedByLabel.getSize().x / 2, -40),
                           spiceHarvestedByLabel.getSize());

    unitsDestroyedByLabel.setTextColor(COLOR_WHITE, COLOR_BLACK, COLOR_THICKSPICE);
    unitsDestroyedByLabel.setAlignment(Alignment_HCenter);
    unitsDestroyedByLabel.setText(_("@DUNE.ENG|24#Units destroyed by"));
    windowWidget.addWidget(&unitsDestroyedByLabel, getSize() / 2 + Point(-unitsDestroyedByLabel.getSize().x / 2, 34),
                           unitsDestroyedByLabel.getSize());

    buildingsDestroyedByLabel.setTextColor(COLOR_WHITE, COLOR_BLACK, COLOR_THICKSPICE);
    buildingsDestroyedByLabel.setAlignment(Alignment_HCenter);
    buildingsDestroyedByLabel.setText(_("@DUNE.ENG|25#Buildings destroyed by"));
    windowWidget.addWidget(&buildingsDestroyedByLabel,
                           getSize() / 2 + Point(-buildingsDestroyedByLabel.getSize().x / 2, 108),
                           buildingsDestroyedByLabel.getSize());

    // spice statistics

    you1Label.setTextColor(COLOR_WHITE, COLOR_BLACK);
    you1Label.setAlignment(Alignment_Right);
    you1Label.setText(_("@DUNE.ENG|329#You:"));
    windowWidget.addWidget(&you1Label, getSize() / 2 + Point(-229 - you1Label.getSize().x, -21), you1Label.getSize());

    spiceYouShadowProgressBar.setColor(COLOR_BLACK);
    spiceYouShadowProgressBar.setProgress(0.0);
    windowWidget.addWidget(&spiceYouShadowProgressBar, getSize() / 2 + Point(-228 + 2, -15 + 2), Point(440, 12));

    spiceYouProgressBar.setColor(colorYou);
    spiceYouProgressBar.setProgress(0.0);
    windowWidget.addWidget(&spiceYouProgressBar, getSize() / 2 + Point(-228, -15), Point(440, 12));

    spiceYouLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
    spiceYouLabel.setAlignment(Alignment_HCenter);
    spiceYouLabel.setVisible(false);
    windowWidget.addWidget(&spiceYouLabel, getSize() / 2 + Point(222, -20), Point(66, 21));

    enemy1Label.setTextColor(COLOR_WHITE, COLOR_BLACK);
    enemy1Label.setAlignment(Alignment_Right);
    enemy1Label.setText(_("@DUNE.ENG|330#Enemy:"));
    windowWidget.addWidget(&enemy1Label, getSize() / 2 + Point(-229 - enemy1Label.getSize().x, -3),
                           enemy1Label.getSize());

    spiceEnemyShadowProgressBar.setColor(COLOR_BLACK);
    spiceEnemyShadowProgressBar.setProgress(0.0);
    windowWidget.addWidget(&spiceEnemyShadowProgressBar, getSize() / 2 + Point(-228 + 2, 3 + 2), Point(440, 12));

    spiceEnemyProgressBar.setColor(colorEnemy);
    spiceEnemyProgressBar.setProgress(0.0);
    windowWidget.addWidget(&spiceEnemyProgressBar, getSize() / 2 + Point(-228, 3), Point(440, 12));

    spiceEnemyLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
    spiceEnemyLabel.setAlignment(Alignment_HCenter);
    spiceEnemyLabel.setVisible(false);
    windowWidget.addWidget(&spiceEnemyLabel, getSize() / 2 + Point(222, -2), Point(66, 21));

    // unit kill statistics

    you2Label.setTextColor(COLOR_WHITE, COLOR_BLACK);
    you2Label.setAlignment(Alignment_Right);
    you2Label.setText(_("@DUNE.ENG|329#You:"));
    windowWidget.addWidget(&you2Label, getSize() / 2 + Point(-229 - you2Label.getSize().x, 53), you2Label.getSize());

    unitsYouShadowProgressBar.setColor(COLOR_BLACK);
    unitsYouShadowProgressBar.setProgress(0.0);
    windowWidget.addWidget(&unitsYouShadowProgressBar, getSize() / 2 + Point(-228 + 2, 59 + 2), Point(440, 12));

    unitsYouProgressBar.setColor(colorYou);
    unitsYouProgressBar.setProgress(0.0);
    windowWidget.addWidget(&unitsYouProgressBar, getSize() / 2 + Point(-228, 59), Point(440, 12));

    unitsYouLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
    unitsYouLabel.setAlignment(Alignment_HCenter);
    unitsYouLabel.setVisible(false);
    windowWidget.addWidget(&unitsYouLabel, getSize() / 2 + Point(222, 54), Point(66, 21));

    enemy2Label.setTextColor(COLOR_WHITE, COLOR_BLACK);
    enemy2Label.setAlignment(Alignment_Right);
    enemy2Label.setText(_("@DUNE.ENG|330#Enemy:"));
    windowWidget.addWidget(&enemy2Label, getSize() / 2 + Point(-229 - enemy2Label.getSize().x, 71),
                           enemy2Label.getSize());

    unitsEnemyShadowProgressBar.setColor(COLOR_BLACK);
    unitsEnemyShadowProgressBar.setProgress(0.0);
    windowWidget.addWidget(&unitsEnemyShadowProgressBar, getSize() / 2 + Point(-228 + 2, 77 + 2), Point(440, 12));

    unitsEnemyProgressBar.setColor(colorEnemy);
    unitsEnemyProgressBar.setProgress(0.0);
    windowWidget.addWidget(&unitsEnemyProgressBar, getSize() / 2 + Point(-228, 77), Point(440, 12));

    unitsEnemyLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
    unitsEnemyLabel.setAlignment(Alignment_HCenter);
    unitsEnemyLabel.setVisible(false);
    windowWidget.addWidget(&unitsEnemyLabel, getSize() / 2 + Point(222, 72), Point(66, 21));

    // buildings kill statistics

    you3Label.setTextColor(COLOR_WHITE, COLOR_BLACK);
    you3Label.setAlignment(Alignment_Right);
    you3Label.setText(_("@DUNE.ENG|329#You:"));
    windowWidget.addWidget(&you3Label, getSize() / 2 + Point(-229 - you3Label.getSize().x, 127), you3Label.getSize());

    buildingsYouShadowProgressBar.setColor(COLOR_BLACK);
    buildingsYouShadowProgressBar.setProgress(0.0);
    windowWidget.addWidget(&buildingsYouShadowProgressBar, getSize() / 2 + Point(-228 + 2, 133 + 2), Point(440, 12));

    buildingsYouProgressBar.setColor(colorYou);
    buildingsYouProgressBar.setProgress(0.0);
    windowWidget.addWidget(&buildingsYouProgressBar, getSize() / 2 + Point(-228, 133), Point(440, 12));

    buildingsYouLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
    buildingsYouLabel.setAlignment(Alignment_HCenter);
    buildingsYouLabel.setVisible(false);
    windowWidget.addWidget(&buildingsYouLabel, getSize() / 2 + Point(222, 128), Point(66, 21));

    enemy3Label.setTextColor(COLOR_WHITE, COLOR_BLACK);
    enemy3Label.setAlignment(Alignment_Right);
    enemy3Label.setText(_("@DUNE.ENG|330#Enemy:"));
    windowWidget.addWidget(&enemy3Label, getSize() / 2 + Point(-229 - enemy2Label.getSize().x, 145),
                           enemy3Label.getSize());

    buildingsEnemyShadowProgressBar.setColor(COLOR_BLACK);
    buildingsEnemyShadowProgressBar.setProgress(0.0);
    windowWidget.addWidget(&buildingsEnemyShadowProgressBar, getSize() / 2 + Point(-228 + 2, 151 + 2), Point(440, 12));

    buildingsEnemyProgressBar.setColor(colorEnemy);
    buildingsEnemyProgressBar.setProgress(0.0);
    windowWidget.addWidget(&buildingsEnemyProgressBar, getSize() / 2 + Point(-228, 151), Point(440, 12));

    buildingsEnemyLabel.setTextColor(COLOR_WHITE, COLOR_BLACK);
    buildingsEnemyLabel.setAlignment(Alignment_HCenter);
    buildingsEnemyLabel.setVisible(false);
    windowWidget.addWidget(&buildingsEnemyLabel, getSize() / 2 + Point(222, 146), Point(66, 21));
}

CampaignStatsMenu::~CampaignStatsMenu() = default;

int CampaignStatsMenu::showMenuImpl() {
    dune::globals::musicPlayer->changeMusic(MUSIC_GAMESTAT);

    currentStateStartTime = dune::dune_clock::now();
    currentState          = CampaignStatsState::State_HumanSpice;

    return parent::showMenuImpl();
}

void CampaignStatsMenu::doInputImpl(const SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONUP) {
        if (currentState == CampaignStatsState::State_Finished) {
            quit();
        } else {
            while (currentState != CampaignStatsState::State_Finished) {
                doState(dune::as_dune_clock_duration(PROGRESSBARTIME));
            }
        }
    }

    parent::doInputImpl(event);
}

void CampaignStatsMenu::drawSpecificStuff() {
    doState(dune::dune_clock::now() - currentStateStartTime);
}

void CampaignStatsMenu::doState(dune::dune_clock::duration elapsedTime) {
    using dune::globals::soundPlayer;

    switch (currentState) {
        case CampaignStatsState::State_HumanSpice: {
            const float MaxSpiceHarvested = max3(spiceHarvestedByHuman, spiceHarvestedByAI, 3000.0f);
            const float SpiceComplete     = std::min(dune::as_milliseconds<float>(elapsedTime) / PROGRESSBARTIME, 1.0f);

            float Human_PercentSpiceComplete = NAN;
            if (SpiceComplete < spiceHarvestedByHuman / MaxSpiceHarvested) {
                Human_PercentSpiceComplete = SpiceComplete * 100.0f;
                spiceYouLabel.setText(std::to_string(static_cast<int>(SpiceComplete * MaxSpiceHarvested)));
                soundPlayer->playSound(Sound_enum::Sound_CreditsTick);
            } else {
                Human_PercentSpiceComplete = spiceHarvestedByHuman * 100.0f / MaxSpiceHarvested;
                spiceYouLabel.setText(std::to_string(static_cast<int>(spiceHarvestedByHuman)));
                soundPlayer->playSound(Sound_enum::Sound_Tick);
                currentState          = CampaignStatsState::State_Between_HumanSpice_and_AISpice;
                currentStateStartTime = dune::dune_clock::now();
            }

            spiceYouLabel.setVisible(true);
            spiceYouProgressBar.setProgress(Human_PercentSpiceComplete);
            spiceYouShadowProgressBar.setProgress(Human_PercentSpiceComplete);
        } break;

        case CampaignStatsState::State_Between_HumanSpice_and_AISpice: {
            if (elapsedTime > WAITTIME) {
                currentState          = CampaignStatsState::State_AISpice;
                currentStateStartTime = dune::dune_clock::now();
            }
        } break;

        case CampaignStatsState::State_AISpice: {
            const float MaxSpiceHarvested = max3(spiceHarvestedByHuman, spiceHarvestedByAI, 3000.0f);
            const float SpiceComplete     = std::min(dune::as_milliseconds<float>(elapsedTime) / PROGRESSBARTIME, 1.0f);

            float AI_PercentSpiceComplete = NAN;
            if (SpiceComplete < spiceHarvestedByAI / MaxSpiceHarvested) {
                AI_PercentSpiceComplete = SpiceComplete * 100.0f;
                spiceEnemyLabel.setText(std::to_string(static_cast<int>(SpiceComplete * MaxSpiceHarvested)));
                soundPlayer->playSound(Sound_enum::Sound_CreditsTick);
            } else {
                AI_PercentSpiceComplete = spiceHarvestedByAI * 100.0f / MaxSpiceHarvested;
                spiceEnemyLabel.setText(std::to_string(static_cast<int>(spiceHarvestedByAI)));
                soundPlayer->playSound(Sound_enum::Sound_Tick);
                currentState          = CampaignStatsState::State_Between_AISpice_and_HumanUnits;
                currentStateStartTime = dune::dune_clock::now();
            }

            spiceEnemyLabel.setVisible(true);
            spiceEnemyProgressBar.setProgress(AI_PercentSpiceComplete);
            spiceEnemyShadowProgressBar.setProgress(AI_PercentSpiceComplete);
        } break;

        case CampaignStatsState::State_Between_AISpice_and_HumanUnits:
            if (elapsedTime > WAITTIME) {
                currentState          = CampaignStatsState::State_HumanUnits;
                currentStateStartTime = dune::dune_clock::now();
            }
            break;

        case CampaignStatsState::State_HumanUnits: {
            const float MaxUnitsDestroyed = max3(unitsDestroyedByHuman, unitsDestroyedByAI, 200);
            const float UnitsComplete     = std::min(dune::as_milliseconds<float>(elapsedTime) / PROGRESSBARTIME, 1.0f);

            float Human_PercentUnitsComplete = NAN;
            if (UnitsComplete < unitsDestroyedByHuman / MaxUnitsDestroyed) {
                Human_PercentUnitsComplete = UnitsComplete * 100.0f;
                unitsYouLabel.setText(std::to_string(static_cast<int>(UnitsComplete * MaxUnitsDestroyed)));
                soundPlayer->playSound(Sound_enum::Sound_CreditsTick);
            } else {
                Human_PercentUnitsComplete = unitsDestroyedByHuman * 100.0f / MaxUnitsDestroyed;
                unitsYouLabel.setText(std::to_string(unitsDestroyedByHuman));
                soundPlayer->playSound(Sound_enum::Sound_Tick);
                currentState          = CampaignStatsState::State_Between_HumanUnits_and_AIUnits;
                currentStateStartTime = dune::dune_clock::now();
            }

            unitsYouLabel.setVisible(true);
            unitsYouProgressBar.setProgress(Human_PercentUnitsComplete);
            unitsYouShadowProgressBar.setProgress(Human_PercentUnitsComplete);
        } break;

        case CampaignStatsState::State_Between_HumanUnits_and_AIUnits:
            if (elapsedTime > WAITTIME) {
                currentState          = CampaignStatsState::State_AIUnits;
                currentStateStartTime = dune::dune_clock::now();
            }
            break;

        case CampaignStatsState::State_AIUnits: {
            const float MaxUnitsDestroyed = max3(unitsDestroyedByHuman, unitsDestroyedByAI, 200);
            const float UnitsComplete     = std::min(dune::as_milliseconds<float>(elapsedTime) / PROGRESSBARTIME, 1.0f);

            float AI_PercentUnitsComplete = NAN;
            if (UnitsComplete < unitsDestroyedByAI / MaxUnitsDestroyed) {
                AI_PercentUnitsComplete = UnitsComplete * 100.0f;
                unitsEnemyLabel.setText(std::to_string(static_cast<int>(UnitsComplete * MaxUnitsDestroyed)));
                soundPlayer->playSound(Sound_enum::Sound_CreditsTick);
            } else {
                AI_PercentUnitsComplete = unitsDestroyedByAI * 100.0f / MaxUnitsDestroyed;
                unitsEnemyLabel.setText(std::to_string(unitsDestroyedByAI));
                soundPlayer->playSound(Sound_enum::Sound_Tick);
                currentState          = CampaignStatsState::State_Between_AIUnits_and_HumanBuildings;
                currentStateStartTime = dune::dune_clock::now();
            }

            unitsEnemyLabel.setVisible(true);
            unitsEnemyProgressBar.setProgress(AI_PercentUnitsComplete);
            unitsEnemyShadowProgressBar.setProgress(AI_PercentUnitsComplete);
        } break;

        case CampaignStatsState::State_Between_AIUnits_and_HumanBuildings:
            if (elapsedTime > WAITTIME) {
                currentState          = CampaignStatsState::State_HumanBuildings;
                currentStateStartTime = dune::dune_clock::now();
            }
            break;

        case CampaignStatsState::State_HumanBuildings: {
            const float MaxBuildingsDestroyed = max3(structuresDestroyedByHuman, structuresDestroyedByAI, 200);
            const float BuildingsComplete = std::min(dune::as_milliseconds<float>(elapsedTime) / PROGRESSBARTIME, 1.0f);

            float Human_PercentBuildingsComplete = NAN;
            if (BuildingsComplete < structuresDestroyedByHuman / MaxBuildingsDestroyed) {
                Human_PercentBuildingsComplete = BuildingsComplete * 100.0f;
                buildingsYouLabel.setText(std::to_string(static_cast<int>(BuildingsComplete * MaxBuildingsDestroyed)));
                soundPlayer->playSound(Sound_enum::Sound_CreditsTick);
            } else {
                Human_PercentBuildingsComplete = structuresDestroyedByHuman * 100.0f / MaxBuildingsDestroyed;
                buildingsYouLabel.setText(std::to_string(structuresDestroyedByHuman));
                soundPlayer->playSound(Sound_enum::Sound_Tick);
                currentState          = CampaignStatsState::State_Between_HumanBuildings_and_AIBuildings;
                currentStateStartTime = dune::dune_clock::now();
            }

            buildingsYouLabel.setVisible(true);
            buildingsYouProgressBar.setProgress(Human_PercentBuildingsComplete);
            buildingsYouShadowProgressBar.setProgress(Human_PercentBuildingsComplete);
        } break;

        case CampaignStatsState::State_Between_HumanBuildings_and_AIBuildings:
            if (elapsedTime > WAITTIME) {
                currentState          = CampaignStatsState::State_AIBuildings;
                currentStateStartTime = dune::dune_clock::now();
            }
            break;

        case CampaignStatsState::State_AIBuildings: {
            const float MaxBuildingsDestroyed = max3(structuresDestroyedByHuman, structuresDestroyedByAI, 200);
            const float BuildingsComplete = std::min(dune::as_milliseconds<float>(elapsedTime) / PROGRESSBARTIME, 1.0f);

            float AI_PercentBuildingsComplete = NAN;
            if (BuildingsComplete < structuresDestroyedByAI / MaxBuildingsDestroyed) {
                AI_PercentBuildingsComplete = BuildingsComplete * 100.0f;
                buildingsEnemyLabel.setText(
                    std::to_string(static_cast<int>(BuildingsComplete * MaxBuildingsDestroyed)));
                soundPlayer->playSound(Sound_enum::Sound_CreditsTick);
            } else {
                AI_PercentBuildingsComplete = structuresDestroyedByAI * 100.0f / MaxBuildingsDestroyed;
                buildingsEnemyLabel.setText(std::to_string(structuresDestroyedByAI));
                soundPlayer->playSound(Sound_enum::Sound_Tick);
                currentState          = CampaignStatsState::State_Finished;
                currentStateStartTime = dune::dune_clock::now();
            }

            buildingsEnemyLabel.setVisible(true);
            buildingsEnemyProgressBar.setProgress(AI_PercentBuildingsComplete);
            buildingsEnemyShadowProgressBar.setProgress(AI_PercentBuildingsComplete);
        } break;

        case CampaignStatsState::State_Finished:
        default: {
            // nothing
        } break;
    }
}

void CampaignStatsMenu::calculateScore(int level) {
    unitsDestroyedByHuman = 0;
    unitsDestroyedByAI    = 0;

    structuresDestroyedByHuman = 0;
    structuresDestroyedByAI    = 0;

    auto spice_harvested_by_human = 0.0_fix;
    auto spice_harvested_by_ai    = 0.0_fix;

    const auto* const game = dune::globals::currentGame.get();

    totalTime = game->getGameTime() / 1000;

    totalScore = level * 45;

    auto totalHumanCredits = 0.0_fix;

    game->for_each_house([&](auto& house) {
        if (house.isAI() == true) {
            unitsDestroyedByAI += house.getNumDestroyedUnits();
            structuresDestroyedByAI += house.getNumDestroyedStructures();
            spice_harvested_by_ai += house.getHarvestedSpice();

            totalScore -= house.getDestroyedValue();
        } else {
            unitsDestroyedByHuman += house.getNumDestroyedUnits();
            structuresDestroyedByHuman += house.getNumDestroyedStructures();
            spice_harvested_by_human += house.getHarvestedSpice();

            totalHumanCredits += house.getStoredCredits() + house.getStartingCredits();

            totalScore += house.getDestroyedValue();
        }
    });

    totalScore += (totalHumanCredits / 100).lround();

    for (const auto* pStructure : dune::globals::structureList) {
        if (!pStructure->getOwner()->isAI()) {
            const auto item_id  = pStructure->getItemID();
            const auto house_id = static_cast<int>(pStructure->getOriginalHouseID());

            totalScore += game->objectData.data[item_id][house_id].price / 100;
        }
    }

    totalScore -= totalTime / 60 + 1;

    for (const auto* pUnit : dune::globals::unitList) {
        if (const auto* pHarvester = dune_cast<const Harvester>(pUnit)) {
            const auto spice = pHarvester->getAmountOfSpice();

            if (pHarvester->getOwner()->isAI())
                spice_harvested_by_ai += spice;
            else
                spice_harvested_by_human += spice;
        }
    }

    spiceHarvestedByAI    = spice_harvested_by_ai.toFloat();
    spiceHarvestedByHuman = spice_harvested_by_human.toFloat();

    if (game->areCheatsEnabled()) {
        rank = "Cheater";
    } else {

        if (totalScore >= 1400)
            rank = _("@DUNE.ENG|282#Emperor");
        else if (totalScore >= 1000)
            rank = _("@DUNE.ENG|281#Ruler of Arrakis");
        else if (totalScore >= 700)
            rank = _("@DUNE.ENG|280#Chief Warlord");
        else if (totalScore >= 500)
            rank = _("@DUNE.ENG|279#Warlord");
        else if (totalScore >= 400)
            rank = _("@DUNE.ENG|278#Base Commander");
        else if (totalScore >= 300)
            rank = _("@DUNE.ENG|277#Outpost Commander");
        else if (totalScore >= 200)
            rank = _("@DUNE.ENG|276#Squad Leader");
        else if (totalScore >= 150)
            rank = _("@DUNE.ENG|275#Dune Trooper");
        else if (totalScore >= 100)
            rank = _("@DUNE.ENG|274#Sand Warrior");
        else if (totalScore >= 50)
            rank = _("@DUNE.ENG|273#Desert Mongoose");
        else if (totalScore >= 25)
            rank = _("@DUNE.ENG|272#Sand Snake");
        else
            rank = _("@DUNE.ENG|271#Sand Flea");
    }
}
