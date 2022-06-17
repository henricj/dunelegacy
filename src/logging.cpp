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

#include "logging.h"

#include "config.h"
#include "dune_version.h"

#include "misc/SDL2pp.h"
#include "misc/fnkdat.h"

#include <SDL2/SDL.h>

#include <enet/enet.h>

#include <fmt/core.h>

#include "lodepng.h"

#include <fcntl.h>
#include <filesystem>
#include <unordered_map>

namespace {

void log_git_info() {
#if defined(DUNE_GIT_DESCRIBE)
#    if defined(DUNE_GIT_REPO_BRANCH)
    sdl2::log_info("   git " DUNE_GIT_REPO_BRANCH "/" DUNE_GIT_DESCRIBE " " DUNE_GIT_TIME);
#    else
    sdl2::log_info("   git " DUNE_GIT_DESCRIBE " " DUNE_GIT_TIME);
#    endif
    sdl2::log_info("   git " DUNE_GIT_REPO_URL);
#endif
}

void log_build_info() {
    dune::log_compiler_info();

    sdl2::log_info("   %d bit build, C++ standard %d", 8 * sizeof(void*), __cplusplus);

#if defined(DEBUG)
    sdl2::log_info("   *** DEBUG build " __DATE__ " " __TIME__);
#endif

    log_git_info();

#if !defined(HAS_ASAN)
#    if defined(__has_feature)
#        if __has_feature(address_sanitizer)
#            define HAS_ASAN 1
#        endif
#    endif
#endif

#if !defined(HAS_ASAN)
#    if defined(__SANITIZE_ADDRESS__)
#        define HAS_ASAN 1
#    endif
#endif

#if defined(HAS_ASAN)
    sdl2::log_info("   *** Address Sanitizer (ASan) enabled");
#endif

#if defined(__has_feature)
#    if __has_feature(undefined_behavior_sanitizer)
    sdl2::log_info("   *** Undefined Behavior Sanitizer (UBSan) enabled");
#    endif
#endif

    dune::log_sdk_info();

#if defined(FMT_VERSION)
    sdl2::log_info("fmt %d.%d.%d", FMT_VERSION / 10000, FMT_VERSION / 100 % 100, FMT_VERSION % 100);
#endif

    sdl2::log_info("lodepng %s", LODEPNG_VERSION_STRING);

#if defined(ENET_VERSION_MAJOR) && defined(ENET_VERSION_MINOR) && defined(ENET_VERSION_PATCH)
    sdl2::log_info("enet %d.%d.%d", ENET_VERSION_MAJOR, ENET_VERSION_MINOR, ENET_VERSION_PATCH);
#endif
}

std::filesystem::path getLogFilepath() {
    // determine path to config file
    auto [ok, tmp] = fnkdat(LOGFILENAME, FNKDAT_USER | FNKDAT_CREAT);

    if (!ok) {
        THROW(std::runtime_error, "fnkdat() failed!");
    }

    return tmp;
}

void logOutputFunction([[maybe_unused]] void* userdata, [[maybe_unused]] int category, SDL_LogPriority priority,
                       const char* message) {

    static constexpr std::string_view priorityStrings[] = {"<UNK> ",   "VERBOSE ", "DEBUG   ", "INFO    ",
                                                           "WARN    ", "ERROR   ", "CRITICAL"};

    static constexpr auto priorityStringsSize = static_cast<int>(std::size(priorityStrings));

    const auto n = static_cast<int>(priority);

    const auto priority_name = n >= 0 && n < priorityStringsSize ? priorityStrings[n] : priorityStrings[0];

    const auto output = fmt::format("{}:   {}\n", priority_name, message);
#ifdef _WIN32
    OutputDebugStringA(output.c_str());
#endif
    fwrite(output.data(), output.size(), 1, stderr);
    fflush(stderr);
}

} // namespace

namespace dune {

void logging_initialize() {
    SDL_LogSetOutputFunction(logOutputFunction, nullptr);
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);
}

void logging_configure(bool capture_output) {
    if (capture_output) {
        // get utf8-encoded log file path
        const auto logfilePath = getLogFilepath();

        log_capture_output(logfilePath);
    }

    sdl2::log_info("Starting Dune Legacy %s on %s", VERSION, SDL_GetPlatform());

    log_computer_info();

    log_process_info();

    log_build_info();
}

void logging_complete() {

    log_process_info();

    std::fflush(stderr);
    std::fflush(stdout);
}

} // namespace dune
