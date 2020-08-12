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

#include "engine/data.h"

#include <DataTypes.h>

#include <filesystem>
#include <string>

// forward declarations
namespace Dune::Engine {
class GameInitSettings;
} // namespace Dune::Engine
extern SDL_Window* window;
extern SDL_Renderer* renderer;

void drawCursor();

std::string     resolveItemName(ItemID_enum itemID);

int             getAnimByFilename(const std::string& filename);

const DuneTexture*    resolveItemPicture(ItemID_enum itemID, HOUSETYPE house = HOUSETYPE::HOUSE_HARKONNEN);

std::string     resolveDropLocationName(DropLocation dropLocation);

Uint32 getColorByTerrainType(TERRAINTYPE terrainType);

void startReplay(const std::filesystem::path& filename);
void startSinglePlayerGame(const Dune::Engine::GameInitSettings& init);
void startMultiPlayerGame(const Dune::Engine::GameInitSettings& init);


#endif //SAND_H
