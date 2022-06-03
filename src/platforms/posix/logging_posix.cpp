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

#include "misc/SDL2pp.h"
#include <SDL2/SDL.h>

#include <gsl/gsl>

#include <filesystem>
#include <optional>
#include <unordered_map>

#include <fcntl.h>
#include <pwd.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

namespace dune {

void log_computer_info() { }

void log_process_info() {
    rusage usage{};

    if (0 != getrusage(RUSAGE_SELF, &usage))
        return;
}

void log_sdk_info() { }

void log_capture_output(std::filesystem::path logfilePath) {
    auto* const pLogfilePath = logfilePath.c_str();

    int d = open(pLogfilePath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (d < 0) {
        THROW(io_error, "Opening logfile '%s' failed!", pLogfilePath);
    }

    auto cleanup_d = gsl::finally([d] { close(d); });

    // Hint: fileno(stdout) != STDOUT_FILENO on Win32
    if (dup2(d, fileno(stdout)) < 0) {
        THROW(io_error, "Redirecting stdout failed!");
    }

    // Hint: fileno(stderr) != STDERR_FILENO on Win32
    if (dup2(d, fileno(stderr)) < 0) {
        THROW(io_error, "Redirecting stderr failed!");
    }
}

} // namespace dune
