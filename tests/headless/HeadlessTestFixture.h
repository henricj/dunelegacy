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

#pragma once

#include <gtest/gtest.h>

/// GTest fixture that initializes the minimal set of globals needed for gameplay
/// map/game setup tests (FileManager, currentGame) without creating any SDL
/// window, renderer, audio, or UI objects.
///
/// Usage:
///   class MyTest : public HeadlessTestFixture { ... };
///   TEST_F(MyTest, SomeCase) { ... }
class HeadlessTestFixture : public ::testing::Test {
protected:
    /// Creates dune::globals::pFileManager so PAK and map files are accessible.
    void SetUp() override;

    /// Destroys dune::globals::currentGame (if any) and dune::globals::pFileManager.
    void TearDown() override;
};
