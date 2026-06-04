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
 * Process-level determinism test.
 *
 * Spawns headless_hash_runner as a subprocess twice with identical inputs and
 * verifies that both produce the same SHA-384 hex digest.  Running in separate
 * processes guarantees that each run starts from a clean global state, making
 * this a true end-to-end determinism check.
 *
 * The path to the runner binary is supplied via the HASH_RUNNER environment
 * variable, set by CMake at test registration time.
 */

#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <string>

#ifdef _WIN32
#    define DUNE_POPEN  _popen
#    define DUNE_PCLOSE _pclose
#else
#    define DUNE_POPEN  popen
#    define DUNE_PCLOSE pclose
#endif

namespace {

/// Runs headless_hash_runner as a subprocess and returns the trimmed hex hash
/// printed to stdout, or an empty string on failure.
std::string runHashRunner(const std::string& runner_path, const char* map, unsigned ticks,
                          const std::string& seed_hex) {
    // Build a shell command that quotes the runner path so spaces are safe.
    // On Windows, _popen hands the string to cmd /c; the outer extra pair of
    // quotes is required by cmd.exe when the command itself is quoted.
    std::string cmd;
#ifdef _WIN32
    cmd = "\"\"" + runner_path + "\" " + map + " " + std::to_string(ticks) + " " + seed_hex +
          "\"";
#else
    cmd = "\"" + runner_path + "\" " + map + " " + std::to_string(ticks) + " " + seed_hex;
#endif

    FILE* fp = DUNE_POPEN(cmd.c_str(), "r"); // NOLINT(cert-env33-c)
    if (fp == nullptr)
        return {};

    char buf[256] = {};
    if (std::fgets(buf, sizeof(buf), fp) == nullptr)
        buf[0] = '\0';

    DUNE_PCLOSE(fp);

    // Strip trailing CR/LF
    std::string result{buf};
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
        result.pop_back();

    return result;
}

/// Generate a deterministic hex seed string.  For now, just return all-zeros
/// (64 bytes = 128 hex characters).  Can be extended later to accept different
/// seeds for parameterized testing.
std::string generateSeedHex() {
    return std::string(128, '0');
}

} // namespace

class DeterminismTest : public ::testing::Test {};

// Run headless_hash_runner twice with the same map, tick count, and seed, in two
// independent processes, and assert that the SHA-384 digests are equal.
// A mismatch means the simulation is non-deterministic.
TEST_F(DeterminismTest, TwoRunsProduceSameHash) {
    const char* runner_env = std::getenv("HASH_RUNNER");
    if (runner_env == nullptr || *runner_env == '\0')
        GTEST_SKIP() << "HASH_RUNNER environment variable not set";

    constexpr auto* kMap   = "SCENF001.INI";
    constexpr unsigned kTicks = 500;

    const std::string runner_path{runner_env};
    const auto seed_hex = generateSeedHex();
    const auto hash1    = runHashRunner(runner_path, kMap, kTicks, seed_hex);
    const auto hash2    = runHashRunner(runner_path, kMap, kTicks, seed_hex);

    ASSERT_FALSE(hash1.empty()) << "First run produced no output";
    ASSERT_FALSE(hash2.empty()) << "Second run produced no output";
    EXPECT_EQ(hash1, hash2)
        << "Simulation is non-deterministic after " << kTicks << " ticks on " << kMap;
}
