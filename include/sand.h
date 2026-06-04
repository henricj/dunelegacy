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

#ifndef SAND_H
#define SAND_H

#include "Menu/MenuBase.h"
#include "sand_core.h"
#include <misc/SDL2pp.h>

#include <filesystem>
#include <string>

// forward declarations
class GameInitSettings;

namespace dune::globals {
extern sdl2::window_ptr window;
extern sdl2::renderer_ptr renderer;
} // namespace dune::globals

void drawCursor();

std::string_view resolveItemName(ItemID_enum itemID);
const DuneTexture* resolveItemPicture(ItemID_enum itemID, HOUSETYPE house = HOUSETYPE::HOUSE_HARKONNEN);
std::string_view resolveDropLocationName(DropLocation dropLocation);

inline int missionNumberToLevelNumber(int missionNumber) {
    if (missionNumber != 22) {
        return ((missionNumber + 1) / 3) + 1;
    }
    return 9;
}

void startReplay(const std::filesystem::path& filename, MenuBase::event_handler_type handler);
void startSinglePlayerGame(const GameInitSettings& init, MenuBase::event_handler_type handler);
void startMultiPlayerGame(const GameInitSettings& init, MenuBase::event_handler_type handler);

#endif // SAND_H
