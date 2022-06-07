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

#ifndef REVERSE_H
#define REVERSE_H
#pragma once

namespace dune {
namespace implementation {
template<typename T>
struct reverse_adapter {
    T& iterable;
};

template<typename T>
auto begin(reverse_adapter<T> w) {
    return std::rbegin(w.iterable);
}

template<typename T>
auto end(reverse_adapter<T> w) {
    return std::rend(w.iterable);
}
} // namespace implementation

template<typename T>
implementation::reverse_adapter<T> reverse(T&& iterable) {
    return {iterable};
}
} // namespace dune

#endif // REVERSE_H
