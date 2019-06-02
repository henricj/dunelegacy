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

std::string     resolveItemName(int itemID);

int             getAnimByFilename(const std::string& filename);

Coord           getStructureSize(int itemID);

Uint32          getItemIDByName(const std::string& name);
std::string     getItemNameByID(Uint32 itemID);
SDL_Texture*    resolveItemPicture(int itemID, HOUSETYPE house = HOUSE_HARKONNEN);

HOUSETYPE       getHouseByName(const std::string& name);
std::string     getHouseNameByNumber(HOUSETYPE house);

ATTACKMODE      getAttackModeByName(const std::string& name);
std::string     getAttackModeNameByMode(ATTACKMODE attackMode);

DropLocation    getDropLocationByName(const std::string& name);
std::string     getDropLocationNameByID(DropLocation dropLocation);
std::string     resolveDropLocationName(DropLocation dropLocation);


AITeamBehavior    getAITeamBehaviorByName(const std::string& name);
std::string     getAITeamBehaviorNameByID(AITeamBehavior aiTeamBehavior);

AITeamType        getAITeamTypeByName(const std::string& name);
std::string     getAITeamTypeNameByID(AITeamType aiTeamType);


Uint32          getColorByTerrainType(int terrainType);

FixPoint        getDeviateWeakness(HOUSETYPE house);

inline int missionNumberToLevelNumber(int missionNumber) {
    if(missionNumber != 22) {
        return ((missionNumber+1)/3)+1;
    } else {
        return 9;
    }
}

void startReplay(const std::string& filename);
void startSinglePlayerGame(const GameInitSettings& init);
void startMultiPlayerGame(const GameInitSettings& init);


#endif //SAND_H
