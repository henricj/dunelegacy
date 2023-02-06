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
 *  along with Dune Legacy  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <stdexcept>

template<typename TException, typename... Args>
TException dune_exception(fmt::format_string<Args...> format, Args&&... args) {
    static_assert(std::is_base_of_v<std::exception, TException>);

    return TException{fmt::format(format, std::forward<decltype(args)>(args)...)};
}

#define THROW(TException, format, ...)                                                                                 \
    throw dune_exception<TException>("{}:{} ({}): " format, __FILE__, __LINE__, __func__ __VA_OPT__(, ) __VA_ARGS__)

class io_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class invalid_file_format : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class sdl_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

#endif // EXCEPTIONS_H
