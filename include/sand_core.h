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

#ifndef SAND_CORE_H
#define SAND_CORE_H

#include <DataTypes.h>
#include <data.h>
#include <fixmath/FixPoint.h>

#include <string>
#include <string_view>

int getAnimByFilename(const std::string& filename);

Coord getStructureSize(ItemID_enum itemID);

ItemID_enum getItemIDByName(std::string_view name);
std::string_view getItemNameByID(ItemID_enum itemID);

HOUSETYPE getHouseByName(std::string_view name);
std::string getHouseNameByNumber(HOUSETYPE house);

ATTACKMODE getAttackModeByName(std::string_view name);
std::string getAttackModeNameByMode(ATTACKMODE attackMode);

DropLocation getDropLocationByName(std::string_view name);
std::string_view getDropLocationNameByID(DropLocation dropLocation);

AITeamBehavior getAITeamBehaviorByName(const std::string& name);
std::string getAITeamBehaviorNameByID(AITeamBehavior aiTeamBehavior);

AITeamType getAITeamTypeByName(const std::string& name);
std::string getAITeamTypeNameByID(AITeamType aiTeamType);

uint32_t getColorByTerrainType(TERRAINTYPE terrainType);

FixPoint getDeviateWeakness(HOUSETYPE house);

#endif // SAND_CORE_H
