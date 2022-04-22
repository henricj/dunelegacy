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

#ifndef DUNE_EVENTS_H
#define DUNE_EVENTS_H

#include <SDL2/SDL.h>

namespace dune {
bool Dune_WaitEvent(SDL_Event* event, uint32_t timeout);

class DuneEventWatcher final {
public:
    DuneEventWatcher();
    ~DuneEventWatcher();
};
} // namespace dune

#endif // DUNE_EVENTS_H
