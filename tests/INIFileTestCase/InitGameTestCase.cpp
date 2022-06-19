#include "INIMap/INIMapLoader.h"

#include <gtest/gtest.h>

#include <array>

TEST(INIFileLoader, create_game) {
    static constexpr auto maps = std::to_array({
        // From data/maps/multiplayer
        "2P - 32x128 - Gatekeeper.ini",
        "2P - 32x32 - X-Factor.ini",
        "2P - 64x32 - Cliffs Of Rene.ini",
        "2P - 64x64 - Bottle Neck.ini",
        "2P - 64x64 - Broken Mountains.ini",
        "2P - 64x64 - David's Pass.ini",
        "2P - 64x64 - Face Off.ini",
        "2P - 64x64 - Great Divide.ini",
        "2P - 64x64 - Sanctuarys.ini",
        "4P - 128x128 - Deserted.ini",
        "4P - 128x128 - Equilibrium.ini",
        "4P - 128x128 - Four Cities.ini",
        "4P - 128x128 - Hungry Hippos.ini",
        "4P - 128x128 - Silicon Valley XL.ini",
        "4P - 128x128 - Snake Pass.ini",
        "4P - 128x128 - Spicestorm.ini",
        "4P - 128x128 - The Sardaukar Outpost.ini",
        "4P - 128x128 - Worm Investation.ini",
        "4P - 128x128 - Wormhole.ini",
        "4P - 128x64 - Gamma Sector.ini",
        "4P - 64x64 - Channels.ini",
        "4P - 64x64 - Clear Path.ini",
        "4P - 64x64 - Combed.ini",
        "4P - 64x64 - Four Chambers.ini",
        "4P - 64x64 - Four Courners.ini",
        "4P - 64x64 - Sietch Stefan.ini",
        "4P - 64x64 - Silicon Valley.ini",
        "4P - 64x64 - Stronghold.ini",
        "4P - 64x64 - Vast Armies Have Arrived.ini",
        "5P - 128x128 - Fortress.ini",
        "5P - 128x128 - Gridlocked.ini",
        "5P - 128x128 - Hellvetika.ini",
        "5P - 128x128 - Kragetam.ini",
        "5P - 128x128 - Meadow.ini",
        "5P - 128x64 - Watch Your Track.ini",
        "6P - 128x128 - Gargantuan Mountains.ini",
        "6P - 64x128 - Rocking Fields.ini",
        "6P - 64x64 - Fertile Basin.ini",

        // From data/maps/singleplayer
        "2P - 32x128 - Canyon.ini",
        "2P - 64x64 - Duality.ini",
        "2P - 64x64 - North vs. South.ini",
        "2P - 64x64 - Twin Fists.ini",
        "3P - 64x32 - Middle Man.ini",
        "4P - 64x64 - 3 vs 1.ini",
        "5P - 128x128 - All against Atreides.ini",
        "5P - 128x128 - Sardaukar Base Easy.ini",
        "5P - 128x128 - Sardaukar Base.ini",

        // From OPENSD2.PAK
        "SCENF001.INI",
        "SCENF002.INI",
        "SCENF003.INI",
        "SCENF004.INI",
        "SCENF005.INI",
        "SCENF006.INI",
        "SCENF007.INI",
        "SCENF008.INI",
        "SCENF009.INI",
        "SCENF010.INI",
        "SCENF011.INI",
        "SCENF012.INI",
        "SCENF013.INI",
        "SCENF014.INI",
        "SCENF015.INI",
        "SCENF016.INI",
        "SCENF017.INI",
        "SCENF018.INI",
        "SCENF019.INI",
        "SCENF020.INI",
        "SCENF021.INI",
        "SCENF022.INI",
        "SCENM001.INI",
        "SCENM002.INI",
        "SCENM003.INI",
        "SCENM004.INI",
        "SCENM005.INI",
        "SCENM006.INI",
        "SCENM007.INI",
        "SCENM008.INI",
        "SCENM009.INI",
        "SCENM010.INI",
        "SCENM011.INI",
        "SCENM012.INI",
        "SCENM013.INI",
        "SCENM014.INI",
        "SCENM015.INI",
        "SCENM016.INI",
        "SCENM017.INI",
        "SCENM018.INI",
        "SCENM019.INI",
        "SCENM020.INI",
        "SCENM021.INI",
        "SCENM022.INI",
        "SCENS001.INI",
        "SCENS002.INI",
        "SCENS003.INI",
        "SCENS004.INI",
        "SCENS005.INI",
        "SCENS006.INI",
        "SCENS007.INI",
        "SCENS008.INI",
        "SCENS009.INI",
        "SCENS010.INI",
        "SCENS011.INI",
        "SCENS012.INI",
        "SCENS013.INI",
        "SCENS014.INI",
        "SCENS015.INI",
        "SCENS016.INI",
        "SCENS017.INI",
        "SCENS018.INI",
        "SCENS019.INI",
        "SCENS020.INI",
        "SCENS021.INI",
        "SCENS022.INI",

    });

    dune::globals::pFileManager = std::make_unique<FileManager>();

    auto cleanup = gsl::finally([] { dune::globals::pFileManager.reset(); });

    auto* const file_manager = dune::globals::pFileManager.get();

    for (std::filesystem::path mapFilename : maps) {

        auto map_name = getBasename(mapFilename, true);

        std::string map_data;

        // Scope
        {
            auto rwop = file_manager->openFile(std::move(mapFilename));

            map_data.resize(SDL_RWsize(rwop.get()));

            ASSERT_EQ(1, SDL_RWread(rwop.get(), map_data.data(), map_data.size(), 1));
        }

        const GameInitSettings init(std::move(map_name), std::move(map_data), false,
                                    dune::globals::settings.gameOptions);

        dune::globals::currentGame = std::make_unique<Game>();

        dune::globals::currentGame->initGame(init);

        dune::globals::currentGame.reset();
    }
}
