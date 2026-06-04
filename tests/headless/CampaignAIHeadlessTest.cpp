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

#include "HeadlessTestFixture.h"

#include <Definitions.h>
#include <Game.h>
#include <GameInitSettings.h>
#include <House.h>
#include <globals.h>

#include <gtest/gtest.h>

namespace {

// Minimal 8x8 all-sand map with Atreides and Harkonnen house sections.
// No [STRUCTURES] or [UNITS] sections so the test has no dependency on
// graphics resources; only house and AI player lifecycle are exercised.
constexpr auto kAIBehaviorMapIni = R"ini(
[BASIC]
Version=2
WinFlags=3
LoseFlags=1
TechLevel=5

[MAP]
SizeX=8
SizeY=8
000=--------
001=--------
002=--------
003=--------
004=--------
005=--------
006=--------
007=--------

[atreides]
Credits=2000
MaxUnits=50

[harkonnen]
Credits=2000
MaxUnits=50
)ini";

/// Build a GameInitSettings for the inline AI-behavior map with an explicit
/// Atreides (human) and Harkonnen (CampaignAI) house configuration.
GameInitSettings makeAITestInit() {
    GameInitSettings init{
        std::filesystem::path{"ai_behavior_test"},
        std::string{kAIBehaviorMapIni},
        false,
        dune::globals::settings.gameOptions,
    };

    GameInitSettings::HouseInfo atreidesInfo{HOUSETYPE::HOUSE_ATREIDES, 1};
    atreidesInfo.addPlayerInfo(GameInitSettings::PlayerInfo{"Player", HUMANPLAYERCLASS});
    init.addHouseInfo(atreidesInfo);

    GameInitSettings::HouseInfo harkonnenInfo{HOUSETYPE::HOUSE_HARKONNEN, 2};
    harkonnenInfo.addPlayerInfo(GameInitSettings::PlayerInfo{"HarkAI", "CampaignAIPlayer"});
    init.addHouseInfo(harkonnenInfo);

    return init;
}

} // namespace

class CampaignAIHeadlessTest : public HeadlessTestFixture {};

// Verify that both houses are created when house info lists are provided
// explicitly along with matching INI sections.
TEST_F(CampaignAIHeadlessTest, BothHousesInitializeSuccessfully) {
    const auto init = makeAITestInit();

    dune::globals::currentGame = std::make_unique<Game>();
    dune::globals::currentGame->initGame(init);

    EXPECT_NE(dune::globals::currentGame->getHouse(HOUSETYPE::HOUSE_ATREIDES), nullptr);
    EXPECT_NE(dune::globals::currentGame->getHouse(HOUSETYPE::HOUSE_HARKONNEN), nullptr);
}

// Verify that GameCore::stepSimulation advances the game cycle count by exactly
// the number of ticks requested when a CampaignAI player is present.
// Exercises the core boundary directly without any UI or renderer setup.
TEST_F(CampaignAIHeadlessTest, CoreCycleCountAdvancesAfterStep) {
    const auto init = makeAITestInit();

    dune::globals::currentGame = std::make_unique<Game>();
    dune::globals::currentGame->initGame(init);

    constexpr uint32_t steps = 100;
    dune::globals::currentGame->getCore().stepSimulation(steps);

    EXPECT_EQ(dune::globals::currentGame->getGameCycleCount(), steps);
}

// Verify that the CampaignAI player stays dormant when no direct enemy
// contact has occurred.  CampaignAIPlayer::update() returns early unless
// hadDirectContactWithEnemy() is true; with no units on the map and no
// fog-clearing contact, the flag must remain false after 100 ticks.
TEST_F(CampaignAIHeadlessTest, CampaignAIRemainsInactiveWithoutContact) {
    const auto init = makeAITestInit();

    dune::globals::currentGame = std::make_unique<Game>();
    dune::globals::currentGame->initGame(init);

    constexpr uint32_t steps = 100;
    dune::globals::currentGame->getCore().stepSimulation(steps);

    const auto* const hark = dune::globals::currentGame->getHouse(HOUSETYPE::HOUSE_HARKONNEN);
    ASSERT_NE(hark, nullptr);
    EXPECT_FALSE(hark->hadDirectContactWithEnemy());
}
