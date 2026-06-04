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

#include <FileClasses/FileManager.h>
#include <Game.h>
#include <GameInitSettings.h>
#include <globals.h>
#include <misc/string_util.h>

#include <gtest/gtest.h>

#include <string>

namespace {

/// Load map bytes and construct a GameInitSettings for a custom map.
/// The map name (without extension) is derived from the filename.
GameInitSettings makeCustomMapInit(std::string_view mapFilename) {
    auto* const file_manager = dune::globals::pFileManager.get();

    std::string map_data;

    {
        auto rwop = file_manager->openFile(std::filesystem::path{mapFilename});
        map_data.resize(SDL_GetIOSize(rwop.get()));
        EXPECT_EQ(map_data.size(), SDL_ReadIO(rwop.get(), map_data.data(), map_data.size()));
    }

    auto map_name = getBasename(std::filesystem::path{mapFilename}, true);

    return GameInitSettings{std::move(map_name), std::move(map_data), false,
                            dune::globals::settings.gameOptions};
}

} // namespace

class HeadlessSimulationTest : public HeadlessTestFixture {};

// Verify that GameCore::stepSimulation advances the game cycle count by exactly
// the number of ticks requested, using a campaign map from OPENSD2.PAK.
// This exercises the core boundary directly without any UI or renderer setup.
TEST_F(HeadlessSimulationTest, CoreStepSimulationAdvancesCycleCount) {
    const auto init = makeCustomMapInit("SCENF001.INI");

    dune::globals::currentGame = std::make_unique<Game>();
    dune::globals::currentGame->initGame(init);

    constexpr uint32_t steps = 10;
    dune::globals::currentGame->getCore().stepSimulation(steps);

    EXPECT_EQ(dune::globals::currentGame->getGameCycleCount(), steps);
}

// Verify that GameCore::stepSimulation runs for a moderate number of ticks
// without crashing on a campaign map from OPENSD2.PAK.
TEST_F(HeadlessSimulationTest, CoreStepSimulationDoesNotCrash) {
    const auto init = makeCustomMapInit("SCENF001.INI");

    dune::globals::currentGame = std::make_unique<Game>();
    dune::globals::currentGame->initGame(init);

    constexpr uint32_t steps = 100;
    dune::globals::currentGame->getCore().stepSimulation(steps);

    EXPECT_EQ(dune::globals::currentGame->getGameCycleCount(), steps);
}

// Compatibility check: Game::stepSimulation() delegates to GameCore::stepSimulation()
// and produces the same cycle-count result.  This guards the bridge against
// accidental divergence from the core path.
TEST_F(HeadlessSimulationTest, GameBridgeStepSimulationDelegates) {
    const auto init = makeCustomMapInit("SCENF001.INI");

    dune::globals::currentGame = std::make_unique<Game>();
    dune::globals::currentGame->initGame(init);

    constexpr uint32_t steps = 10;
    dune::globals::currentGame->stepSimulation(steps);

    EXPECT_EQ(dune::globals::currentGame->getGameCycleCount(), steps);
}
