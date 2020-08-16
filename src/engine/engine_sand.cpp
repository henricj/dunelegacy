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

#include <engine_sand.h>

#include <Game.h>
#include <GameInitSettings.h>
#include <data.h>

#include <misc/exceptions.h>

#include <algorithm>

/**
    This function returns the size of the specified item.
    \param ItemID   the id of the item (e.g. Structure_HeavyFactory)
    \return a Coord containing the size (e.g. (3,2) ). Returns (0,0) on error.
*/
Coord getStructureSize(ItemID_enum itemID) {

    // clang-format off
    switch(itemID) {
        case Structure_Barracks:            return Coord(2,2);
        case Structure_ConstructionYard:    return Coord(2,2);
        case Structure_GunTurret:           return Coord(1,1);
        case Structure_HeavyFactory:        return Coord(3,2);
        case Structure_HighTechFactory:     return Coord(3,2);
        case Structure_IX:                  return Coord(2,2);
        case Structure_LightFactory:        return Coord(2,2);
        case Structure_Palace:              return Coord(3,3);
        case Structure_Radar:               return Coord(2,2);
        case Structure_Refinery:            return Coord(3,2);
        case Structure_RepairYard:          return Coord(3,2);
        case Structure_RocketTurret:        return Coord(1,1);
        case Structure_Silo:                return Coord(2,2);
        case Structure_StarPort:            return Coord(3,3);
        case Structure_Slab1:               return Coord(1,1);
        case Structure_Slab4:               return Coord(2,2);
        case Structure_Wall:                return Coord(1,1);
        case Structure_WindTrap:            return Coord(2,2);
        case Structure_WOR:                 return Coord(2,2);
        default:                            return Coord(0,0);
    }
    // clang-format on
}

/**
    This function return the item id of an item specified by name. There may be multiple names for
    one item. The case of the name is ignored.
    \param name the name of the item (e.g. "rocket-turret" or "r-turret".
    \return the id of the item (e.g. Structure_RocketTurret)
*/
ItemID_enum  getItemIDByName(std::string_view name) {
    const std::string lowerName = strToLower(name);

    // clang-format off
    if(lowerName == "barracks")                                            return Structure_Barracks;
    if((lowerName == "const yard") || (lowerName == "construction yard"))  return Structure_ConstructionYard;
    if((lowerName == "r-turret") || (lowerName == "rocket-turret"))        return Structure_RocketTurret;
    if((lowerName == "turret") || (lowerName == "gun-turret"))             return Structure_GunTurret;
    if((lowerName == "heavy fctry") || (lowerName == "heavy factory"))     return Structure_HeavyFactory;
    if((lowerName == "hi-tech") || (lowerName == "hightech factory"))      return Structure_HighTechFactory;
    if((lowerName == "ix") || (lowerName == "house ix"))                   return Structure_IX;
    if((lowerName == "light fctry") || (lowerName == "light factory"))     return Structure_LightFactory;
    if(lowerName == "palace")                                              return Structure_Palace;
    if((lowerName == "outpost") || (lowerName == "radar"))                 return Structure_Radar;
    if(lowerName == "refinery")                                            return Structure_Refinery;
    if((lowerName == "repair") || (lowerName == "repair yard"))            return Structure_RepairYard;
    if((lowerName == "spice silo") || (lowerName == "silo"))               return Structure_Silo;
    if((lowerName == "concrete") || (lowerName == "slab1"))                return Structure_Slab1;
    if(lowerName == "slab4")                                               return Structure_Slab4;
    if((lowerName == "star port") || (lowerName == "starport"))            return Structure_StarPort;
    if(lowerName == "wall")                                                return Structure_Wall;
    if(lowerName == "windtrap")                                            return Structure_WindTrap;
    if(lowerName == "wor")                                                 return Structure_WOR;
    if((lowerName == "carryall") || (lowerName == "carry-all"))            return Unit_Carryall;
    if((lowerName == "devastator") || (lowerName == "devistator"))         return Unit_Devastator;
    if(lowerName == "deviator")                                            return Unit_Deviator;
    if(lowerName == "frigate")                                             return Unit_Frigate;
    if(lowerName == "harvester")                                           return Unit_Harvester;
    if(lowerName == "soldier")                                             return Unit_Soldier;
    if(lowerName == "launcher")                                            return Unit_Launcher;
    if(lowerName == "mcv")                                                 return Unit_MCV;
    if((lowerName == "thopters") || (lowerName == "'thopters")

       || (lowerName == "thopter") || (lowerName == "'thopter")

       || (lowerName == "ornithopter"))                                    return Unit_Ornithopter;
    if(lowerName == "quad")                                                return Unit_Quad;
    if(lowerName == "saboteur")                                            return Unit_Saboteur;
    if(lowerName == "sandworm")                                            return Unit_Sandworm;
    if(lowerName == "siege tank")                                          return Unit_SiegeTank;
    if((lowerName == "sonic tank") || (lowerName == "sonictank"))          return Unit_SonicTank;
    if(lowerName == "tank")                                                return Unit_Tank;
    if(lowerName == "trike")                                               return Unit_Trike;
    if((lowerName == "raider trike") || (lowerName == "raider"))           return Unit_RaiderTrike;
    if(lowerName == "trooper")                                             return Unit_Trooper;
    if(lowerName == "special")                                             return Unit_Special;
    if(lowerName == "infantry")                                            return Unit_Infantry;
    if(lowerName == "troopers")                                            return Unit_Troopers;
    // clang-format on

    return ItemID_Invalid;
}


/**
    This function returns the name of an item id.
    \param itemID the id of the item (e.g. Unit_Sandworm)
    \return the name of the item (e.g. "Sandworm").
*/
std::string getItemNameByID(ItemID_enum itemID) {
    // clang-format off
    switch(itemID) {
        case Structure_Barracks:            return "Barracks";
        case Structure_ConstructionYard:    return "Const Yard";
        case Structure_GunTurret:           return "Turret";
        case Structure_HeavyFactory:        return "Heavy Fctry";
        case Structure_HighTechFactory:     return "Hi-Tech";
        case Structure_IX:                  return "IX";
        case Structure_LightFactory:        return "Light Fctry";
        case Structure_Palace:              return "Palace";
        case Structure_Radar:               return "Outpost";
        case Structure_Refinery:            return "Refinery";
        case Structure_RepairYard:          return "Repair";
        case Structure_RocketTurret:        return "R-Turret";
        case Structure_Silo:                return "Spice Silo";
        case Structure_Slab1:               return "Concrete";
        case Structure_Slab4:               return "Slab4";
        case Structure_StarPort:            return "Starport";
        case Structure_Wall:                return "Wall";
        case Structure_WindTrap:            return "Windtrap";
        case Structure_WOR:                 return "WOR";

        case Unit_Carryall:                 return "Carryall";
        case Unit_Devastator:               return "Devastator";
        case Unit_Deviator:                 return "Deviator";
        case Unit_Frigate:                  return "Frigate";
        case Unit_Harvester:                return "Harvester";
        case Unit_Launcher:                 return "Launcher";
        case Unit_MCV:                      return "MCV";
        case Unit_Ornithopter:              return "'Thopter";
        case Unit_Quad:                     return "Quad";
        case Unit_RaiderTrike:              return "Raider Trike";
        case Unit_SiegeTank:                return "Siege Tank";
        case Unit_SonicTank:                return "Sonic Tank";
        case Unit_Tank:                     return "Tank";
        case Unit_Trike:                    return "Trike";
        case Unit_Saboteur:                 return "Saboteur";
        case Unit_Sandworm:                 return "Sandworm";
        case Unit_Soldier:                  return "Soldier";
        case Unit_Trooper:                  return "Trooper";
        case Unit_Special:                  return "Special";
        case Unit_Infantry:                 return "Infantry";
        case Unit_Troopers:                 return "Troopers";

        default:
            THROW(std::invalid_argument, "getItemNameByID(): Invalid item ID!");
    }
    // clang-format on
}


/**
    This function returns the number of each house providing the house name as a string. The comparison is
    done case-insensitive.
    \param name the name of the house (e.g."Atreides")
    \return the number of the house (e.g. HOUSE_ATREIDES). HOUSE_INVALID is returned on error.
*/
HOUSETYPE getHouseByName(std::string_view name) {
    const std::string lowerName = strToLower(name);

    // clang-format off
    if(lowerName == "harkonnen")    return HOUSETYPE::HOUSE_HARKONNEN;
    if(lowerName == "atreides")     return HOUSETYPE::HOUSE_ATREIDES;
    if(lowerName == "ordos")        return HOUSETYPE::HOUSE_ORDOS;
    if(lowerName == "fremen")       return HOUSETYPE::HOUSE_FREMEN;
    if(lowerName == "sardaukar")    return HOUSETYPE::HOUSE_SARDAUKAR;
    if(lowerName == "mercenary")    return HOUSETYPE::HOUSE_MERCENARY;
    return HOUSETYPE::HOUSE_INVALID;

    // clang-format on
}

/**
    This function returns the name of house the house number.
    \param house the number of the house (e.g. HOUSE_ATREIDES)
    \return the name of the house (e.g. "Atreides").
*/
std::string getHouseNameByNumber(HOUSETYPE house) {
    if(const auto idx = static_cast<int>(house); idx >= 0 && house < HOUSETYPE::NUM_HOUSES) {
        static const char* const houseName[static_cast<int>(HOUSETYPE::NUM_HOUSES)]{
            "Harkonnen", "Atreides", "Ordos", "Fremen", "Sardaukar", "Mercenary"};
        return houseName[idx];
    }

    THROW(std::invalid_argument, "Invalid house number %d!", house);
}

ATTACKMODE getAttackModeByName(std::string_view name) {
    const std::string lowerName = strToLower(name);

    // clang-format off
    if(lowerName == "guard")                               return ATTACKMODE::GUARD;
    if(lowerName == "area guard")                          return ATTACKMODE::AREAGUARD;
    if(lowerName == "ambush")                              return ATTACKMODE::AMBUSH;
    if((lowerName == "hunt") || (lowerName == "attack"))   return ATTACKMODE::HUNT;
    if(lowerName == "harvest")                             return ATTACKMODE::HARVEST;
    if(lowerName == "sabotage")                            return ATTACKMODE::SABOTAGE;
    if(lowerName == "stop")                                return ATTACKMODE::STOP;
    if(lowerName == "capture")                             return ATTACKMODE::CAPTURE;
    if(lowerName == "retreat")                             return ATTACKMODE::RETREAT;
    return ATTACKMODE::ATTACKMODE_INVALID;

    // clang-format on
}


std::string getAttackModeNameByMode(ATTACKMODE attackMode) {
    // clang-format off
    switch(attackMode) {
        case ATTACKMODE::GUARD:     return "Guard";
        case ATTACKMODE::AREAGUARD: return "Area Guard";
        case ATTACKMODE::AMBUSH:    return "Ambush";
        case ATTACKMODE::HUNT:      return "Hunt";
        case ATTACKMODE::HARVEST:   return "Harvest";
        case ATTACKMODE::SABOTAGE:  return "Sabotage";
        case ATTACKMODE::STOP:      return "Stop";
        case ATTACKMODE::CAPTURE:   return "Capture";
        default:
            THROW(std::invalid_argument, "getAttackModeNameByMode(): Invalid attack mode!");
    }
    // clang-format on
}



DropLocation getDropLocationByName(std::string_view name) {
    const auto lowerName = strToLower(name);

    // clang-format off
    if(lowerName == "north")        { return DropLocation::Drop_North; }
    if(lowerName == "east")         { return DropLocation::Drop_East; }
    if(lowerName == "south")        { return DropLocation::Drop_South; }
    if(lowerName == "west")         { return DropLocation::Drop_West; }
    if(lowerName == "air")          { return DropLocation::Drop_Air; }
    if(lowerName == "visible")      { return DropLocation::Drop_Visible; }
    if(lowerName == "enemybase")    { return DropLocation::Drop_Enemybase; }
    if(lowerName == "homebase")     { return DropLocation::Drop_Homebase; }
    // clang-format on

    return DropLocation::Drop_Invalid;
}


std::string getDropLocationNameByID(DropLocation dropLocation) {
    // clang-format off
    switch(dropLocation) {
        case DropLocation::Drop_North:     return "North";
        case DropLocation::Drop_East:      return "East";
        case DropLocation::Drop_South:     return "South";
        case DropLocation::Drop_West:      return "West";
        case DropLocation::Drop_Air:       return "Air";
        case DropLocation::Drop_Visible:   return "Visible";
        case DropLocation::Drop_Enemybase: return "Enemybase";
        case DropLocation::Drop_Homebase:  return "Homebase";
        default:
            THROW(std::invalid_argument, "getDropLocationNameByID(): Invalid drop location!");
    }
    // clang-format on
}

AITeamBehavior getAITeamBehaviorByName(const std::string& name) {
    const auto lowerName = strToLower(name);

    // clang-format off
    if(lowerName == "normal")   { return AITeamBehavior::AITeamBehavior_Normal; }
    if(lowerName == "guard")    { return AITeamBehavior::AITeamBehavior_Guard; }
    if(lowerName == "kamikaze") { return AITeamBehavior::AITeamBehavior_Kamikaze; }
    if(lowerName == "staging")  { return AITeamBehavior::AITeamBehavior_Staging; }
    if(lowerName == "flee")     { return AITeamBehavior::AITeamBehavior_Flee; }
    // clang-format on

    return AITeamBehavior::AITeamBehavior_Invalid;
}


std::string getAITeamBehaviorNameByID(AITeamBehavior aiTeamBehavior) {
    // clang-format off
    switch(aiTeamBehavior) {
        case AITeamBehavior::AITeamBehavior_Normal:     return "Normal";
        case AITeamBehavior::AITeamBehavior_Guard:      return "Guard";
        case AITeamBehavior::AITeamBehavior_Kamikaze:   return "Kamikaze";
        case AITeamBehavior::AITeamBehavior_Staging:    return "Staging";
        case AITeamBehavior::AITeamBehavior_Flee:       return "Flee";
        default: THROW(std::invalid_argument, "getAITeamBehaviorNameByID(): Invalid team behavior!");
    }
    // clang-format on
}


AITeamType getAITeamTypeByName(const std::string& name) {
    const auto lowerName = strToLower(name);

    // clang-format off
    if(lowerName == "foot")                             { return AITeamType::AITeamType_Foot; }
    if(lowerName == "wheel" || lowerName == "wheeled")  { return AITeamType::AITeamType_Wheeled; }
    if(lowerName == "track" || lowerName == "tracked")  { return AITeamType::AITeamType_Tracked; }
    if(lowerName == "winged")                           { return AITeamType::AITeamType_Winged; }
    if(lowerName == "slither")                          { return AITeamType::AITeamType_Slither; }
    if(lowerName == "harvester")                        { return AITeamType::AITeamType_Harvester; }
    // clang-format on

    return AITeamType::AITeamType_Invalid;
}


std::string getAITeamTypeNameByID(AITeamType aiTeamType) {
    // clang-format off
    switch(aiTeamType) {
        case AITeamType::AITeamType_Foot:       return "Foot";
        case AITeamType::AITeamType_Wheeled:    return "Wheeled";
        case AITeamType::AITeamType_Tracked:    return "Tracked";
        case AITeamType::AITeamType_Winged:     return "Winged";
        case AITeamType::AITeamType_Slither:    return "Slither";
        case AITeamType::AITeamType_Harvester:  return "Harvester";
        default:
            THROW(std::invalid_argument, "getAITeamTypeNameByID(): Invalid team type!");
    }
    // clang-format on
}


/**
    This function returns the house-dependent weakness of a unit to get deviated
    \param  house   the house of the unit (choose the real owner);
*/
FixPoint getDeviateWeakness(const GameContext& context, HOUSETYPE house) {

    // Deviators are crap enough. If this is a custom game remove the weakness nerf
    // so that Ordos is playable for Humans
    if(context.game.gameType == GameType::CustomGame || context.game.gameType == GameType::CustomMultiplayer) {
        return 1.00_fix;
    }

    // clang-format off
    switch(house) {
        case HOUSETYPE::HOUSE_HARKONNEN:    return 0.78_fix;
        case HOUSETYPE::HOUSE_ATREIDES:     return 0.30_fix;
        case HOUSETYPE::HOUSE_ORDOS:        return 0.50_fix;
        case HOUSETYPE::HOUSE_FREMEN:       return 0.08_fix;
        case HOUSETYPE::HOUSE_SARDAUKAR:    return 0.04_fix;
        case HOUSETYPE::HOUSE_MERCENARY:    return 0.50_fix;
        default:                            return 0.00_fix;
    }
    // clang-format on
}


