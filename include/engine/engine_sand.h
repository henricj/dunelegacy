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

#ifndef ENGINE_SAND_H
#define ENGINE_SAND_H

#include "data.h"

#include <fixmath/FixPoint.h>
#include <EngineDataTypes.h>

#include <string>

Coord           getStructureSize(ItemID_enum itemID);

ItemID_enum     getItemIDByName(std::string_view name);
std::string     getItemNameByID(ItemID_enum itemID);

HOUSETYPE       getHouseByName(std::string_view name);
std::string     getHouseNameByNumber(HOUSETYPE house);

ATTACKMODE      getAttackModeByName(std::string_view name);
std::string     getAttackModeNameByMode(ATTACKMODE attackMode);

DropLocation    getDropLocationByName(std::string_view name);
std::string     getDropLocationNameByID(DropLocation dropLocation);


AITeamBehavior  getAITeamBehaviorByName(const std::string& name);
std::string     getAITeamBehaviorNameByID(AITeamBehavior aiTeamBehavior);

AITeamType      getAITeamTypeByName(const std::string& name);
std::string     getAITeamTypeNameByID(AITeamType aiTeamType);


FixPoint        getDeviateWeakness(HOUSETYPE house);

inline int missionNumberToLevelNumber(int missionNumber) {
    if(missionNumber != 22) {
        return ((missionNumber+1)/3)+1;
    }         return 9;

   
}

#endif // ENGINE_SAND_H
