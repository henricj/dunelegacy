#include "HeadlessTestFixture.h"

#include <globals.h>

#include <gtest/gtest.h>

// Verify that SetUp initializes pFileManager and TearDown cleans it up without
// crashing.  This is intentionally minimal — it exercises fixture lifecycle
// only and does not load any map or game data.
TEST_F(HeadlessTestFixture, FileManagerInitialized) {
    EXPECT_NE(dune::globals::pFileManager, nullptr);
}

TEST_F(HeadlessTestFixture, NoRendererOrAudio) {
    EXPECT_EQ(dune::globals::window, nullptr);
    EXPECT_EQ(dune::globals::renderer, nullptr);
    EXPECT_EQ(dune::globals::pAudioEngine, nullptr);
    EXPECT_EQ(dune::globals::soundPlayer, nullptr);
    EXPECT_EQ(dune::globals::musicPlayer, nullptr);
}
