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

/**
 * Standalone helper for the process-level determinism test.
 *
 * Usage: headless_hash_runner <map_filename> <ticks> [seed_hex] [--max-units=N] [--credits=N]
 *
 * If seed_hex is omitted, a fixed all-zeros seed is used (for reproducibility within CI).
 * If provided, seed_hex should be a hex string (zero-padded to 128 chars for 64 bytes).
 *
 * Optional stress-test parameters:
 *   --max-units=N   Override MaxUnits in all house sections to N (e.g., 200 for chaos testing)
 *   --credits=N     Override Credits in all house sections to N (e.g., 50000 for chaos testing)
 *
 * Initialises a headless Game instance (no SDL window/renderer/audio),
 * steps the simulation for <ticks> cycles, calls computeStateHash(), and
 * prints the SHA-384 hex digest followed by a newline to stdout.
 * Exits 0 on success, 1 on any error.
 */

#include <FileClasses/FileManager.h>
#include <Game.h>
#include <GameInitSettings.h>
#include <globals.h>
#include <misc/Random.h>
#include <misc/string_util.h>

#include <charconv>
#include <chrono>
#include <iomanip>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: headless_hash_runner <map_filename> <ticks> [seed_hex] [--max-units=N] "
                     "[--credits=N]\n";
        return 1;
    }

    const char* map_filename = argv[1];

    uint32_t ticks = 0;
    {
        const std::string_view ticks_str{argv[2]};
        const auto [ptr, ec] =
            std::from_chars(ticks_str.data(), ticks_str.data() + ticks_str.size(), ticks);
        if (ec != std::errc{}) {
            std::cerr << "Invalid ticks: " << argv[2] << "\n";
            return 1;
        }
    }

    std::vector<uint8_t> seed(RandomFactory::seed_size, 0);
    int max_units = -1;
    int credits   = -1;

    // Parse remaining arguments
    for (int i = 3; i < argc; ++i) {
        const std::string_view arg{argv[i]};

        if (arg.starts_with("--max-units=")) {
            const auto val_str = arg.substr(12);
            const auto [ptr, ec] =
                std::from_chars(val_str.data(), val_str.data() + val_str.size(), max_units);
            if (ec != std::errc{} || max_units < 0) {
                std::cerr << "Invalid --max-units value: " << argv[i] << "\n";
                return 1;
            }
        } else if (arg.starts_with("--credits=")) {
            const auto val_str = arg.substr(10);
            const auto [ptr, ec] =
                std::from_chars(val_str.data(), val_str.data() + val_str.size(), credits);
            if (ec != std::errc{} || credits < 0) {
                std::cerr << "Invalid --credits value: " << argv[i] << "\n";
                return 1;
            }
        } else {
            // Try parsing as hex seed (backward compat: seed as 3rd positional arg)
            if (i == 3 && arg.size() == 128 && arg.find_first_not_of("0123456789abcdefABCDEF") == std::string_view::npos) {
                for (size_t j = 0; j < seed.size(); ++j) {
                    const auto byte_str = arg.substr(j * 2, 2);
                    unsigned byte_val   = 0;
                    const auto [ptr, ec] =
                        std::from_chars(byte_str.data(), byte_str.data() + 2, byte_val, 16);
                    if (ec != std::errc{} || byte_val > 255) {
                        std::cerr << "Invalid hex byte at position " << (j * 2) << "\n";
                        return 1;
                    }
                    seed[j] = static_cast<uint8_t>(byte_val);
                }
            } else {
                std::cerr << "Unknown argument: " << argv[i] << "\n";
                return 1;
            }
        }
    }

    dune::globals::pFileManager = std::make_unique<FileManager>();

    std::string map_data;
    {
        auto rwop = dune::globals::pFileManager->openFile(std::filesystem::path{map_filename});
        if (!rwop) {
            std::cerr << "Cannot open map: " << map_filename << "\n";
            dune::globals::pFileManager.reset();
            return 1;
        }
        map_data.resize(SDL_GetIOSize(rwop.get()));
        if (SDL_ReadIO(rwop.get(), map_data.data(), map_data.size()) != map_data.size()) {
            std::cerr << "Failed to read map: " << map_filename << "\n";
            dune::globals::pFileManager.reset();
            return 1;
        }
    }

    auto map_name = getBasename(std::filesystem::path{map_filename}, true);

    // Apply stress-test overrides to map data before creating GameInitSettings
    if (max_units >= 0 || credits >= 0) {
        // Simple string replacement in INI data
        if (max_units >= 0) {
            std::string old_mu = "MaxUnits=";
            size_t pos         = 0;
            while ((pos = map_data.find(old_mu, pos)) != std::string::npos) {
                size_t eol = map_data.find('\n', pos);
                if (eol == std::string::npos)
                    eol = map_data.size();
                map_data.replace(pos, eol - pos, "MaxUnits=" + std::to_string(max_units));
                pos += 10; // Move past "MaxUnits="
            }
        }
        if (credits >= 0) {
            std::string old_cred = "Credits=";
            size_t pos           = 0;
            while ((pos = map_data.find(old_cred, pos)) != std::string::npos) {
                size_t eol = map_data.find('\n', pos);
                if (eol == std::string::npos)
                    eol = map_data.size();
                map_data.replace(pos, eol - pos, "Credits=" + std::to_string(credits));
                pos += 8; // Move past "Credits="
            }
        }
    }

    GameInitSettings init{
        std::move(map_name),
        std::move(map_data),
        false,
        dune::globals::settings.gameOptions,
    };

    init.setRandomSeed(seed);

    auto t_init_start = std::chrono::high_resolution_clock::now();
    dune::globals::currentGame = std::make_unique<Game>();
    dune::globals::currentGame->initGame(init);
    auto t_init_end = std::chrono::high_resolution_clock::now();

    auto t_step_start = std::chrono::high_resolution_clock::now();
    dune::globals::currentGame->getCore().stepSimulation(ticks);
    auto t_step_end = std::chrono::high_resolution_clock::now();

    auto t_hash_start = std::chrono::high_resolution_clock::now();
    const auto hash = dune::globals::currentGame->getCore().computeStateHash();
    auto t_hash_end = std::chrono::high_resolution_clock::now();

    auto ms_init = std::chrono::duration_cast<std::chrono::milliseconds>(t_init_end - t_init_start).count();
    auto ms_step = std::chrono::duration_cast<std::chrono::milliseconds>(t_step_end - t_step_start).count();
    auto ms_hash = std::chrono::duration_cast<std::chrono::milliseconds>(t_hash_end - t_hash_start).count();
    std::cerr << "[TIMING] initGame=" << ms_init << "ms, stepSimulation=" << ms_step << "ms, computeStateHash=" << ms_hash << "ms\n";

    for (const uint8_t b : hash)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(b);
    std::cout << '\n';

    dune::globals::currentGame.reset();
    dune::globals::pFileManager.reset();

    return 0;
}
