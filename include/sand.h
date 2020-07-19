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

#include <misc/SDL2pp.h>
#include <fixmath/FixPoint.h>
#include <DataTypes.h>

#include <string>

// forward declarations
class GameInitSettings;
extern SDL_Window* window;
extern SDL_Renderer* renderer;

void drawCursor();

std::string     resolveItemName(ItemID_enum itemID);

int             getAnimByFilename(const std::string& filename);

Coord           getStructureSize(ItemID_enum itemID);

ItemID_enum     getItemIDByName(std::string_view name);
std::string     getItemNameByID(ItemID_enum itemID);
const DuneTexture*    resolveItemPicture(ItemID_enum itemID, HOUSETYPE house = HOUSETYPE::HOUSE_HARKONNEN);

HOUSETYPE       getHouseByName(std::string_view name);
std::string     getHouseNameByNumber(HOUSETYPE house);

ATTACKMODE      getAttackModeByName(std::string_view name);
std::string     getAttackModeNameByMode(ATTACKMODE attackMode);

DropLocation    getDropLocationByName(std::string_view name);
std::string     getDropLocationNameByID(DropLocation dropLocation);
std::string     resolveDropLocationName(DropLocation dropLocation);


AITeamBehavior  getAITeamBehaviorByName(const std::string& name);
std::string     getAITeamBehaviorNameByID(AITeamBehavior aiTeamBehavior);

AITeamType      getAITeamTypeByName(const std::string& name);
std::string     getAITeamTypeNameByID(AITeamType aiTeamType);


Uint32 getColorByTerrainType(TERRAINTYPE terrainType);

FixPoint        getDeviateWeakness(HOUSETYPE house);

inline int missionNumberToLevelNumber(int missionNumber) {
    if(missionNumber != 22) {
        return ((missionNumber+1)/3)+1;
    }         return 9;

   
}

void startReplay(const std::filesystem::path& filename);
void startSinglePlayerGame(const GameInitSettings& init);
void startMultiPlayerGame(const GameInitSettings& init);


#endif //SAND_H
