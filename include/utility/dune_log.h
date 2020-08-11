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

#ifndef DUNE_LOG_H
#define DUNE_LOG_H

#include <fmt/format.h>
#include <fmt/printf.h>

namespace Dune {

class Log {
public:
    using log_callback_type = void (*)(std::string_view message);

    template<typename... Args>
    void log(std::string_view format, Args&&... args) {
        if(callback_) callback_(fmt::sprintf(format, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void logf(std::string_view format, Args&&... args) {
        if(callback_) callback_(fmt::format(format, std::forward<Args>(args)...));
    }

    void set_callback(log_callback_type log_callback) { callback_ = log_callback; }

private:
    log_callback_type callback_{};
};

inline Log Logger;

} // namespace Dune

#endif // DUNE_LOG_H
