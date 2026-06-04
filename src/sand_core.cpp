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

#include <sand_core.h>

#include <Game.h>
#include <globals.h>

#include <misc/exceptions.h>

#include <utility>

/**
    This function returns the anim id based on the passed filename.
    \param  filename    the filename (e.g. STARPORT.WSA)
    \return the id of the animation (e.g. Anim_StarPort)
*/
int getAnimByFilename(const std::string& filename) {
    const auto lowerFilename = strToLower(filename);

    // clang-format off
    if(lowerFilename == "fartr.wsa")       return Anim_AtreidesPlanet;
    if(lowerFilename == "fhark.wsa")       return Anim_HarkonnenPlanet;
    if(lowerFilename == "fordos.wsa")      return Anim_OrdosPlanet;
    if(lowerFilename == "win1.wsa")        return Anim_Win1;
    if(lowerFilename == "win2.wsa")        return Anim_Win2;
    if(lowerFilename == "lostbild.wsa")    return Anim_Lose1;
    if(lowerFilename == "lostvehc.wsa")    return Anim_Lose2;
    if(lowerFilename == "barrac.wsa")      return Anim_Barracks;
    if(lowerFilename == "carryall.wsa")    return Anim_Carryall;
    if(lowerFilename == "construc.wsa")    return Anim_ConstructionYard;
    if(lowerFilename == "fremen.wsa")      return Anim_Fremen;
    if(lowerFilename == "gold-bb.wsa")     return Anim_DeathHand;
    if(lowerFilename == "harktank.wsa")    return Anim_Devastator;
    if(lowerFilename == "harvest.wsa")     return Anim_Harvester;
    if(lowerFilename == "headqrts.wsa")    return Anim_Radar;
    if(lowerFilename == "hitcftry.wsa")    return Anim_HighTechFactory;
    if(lowerFilename == "htank.wsa")       return Anim_SiegeTank;
    if(lowerFilename == "hvyftry.wsa")     return Anim_HeavyFactory;
    if(lowerFilename == "hyinfy.wsa")      return Anim_Trooper;
    if(lowerFilename == "infantry.wsa")    return Anim_Infantry;
    if(lowerFilename == "ix.wsa")          return Anim_IX;
    if(lowerFilename == "liteftry.wsa")    return Anim_LightFactory;
    if(lowerFilename == "ltank.wsa")       return Anim_Tank;
    if(lowerFilename == "mcv.wsa")         return Anim_MCV;
    if(lowerFilename == "ordrtank.wsa")    return Anim_Deviator;
    if(lowerFilename == "orni.wsa")        return Anim_Ornithopter;
    if(lowerFilename == "otrike.wsa")      return Anim_Raider;
    if(lowerFilename == "palace.wsa")      return Anim_Palace;
    if(lowerFilename == "quad.wsa")        return Anim_Quad;
    if(lowerFilename == "refinery.wsa")    return Anim_Refinery;
    if(lowerFilename == "repair.wsa")      return Anim_RepairYard;
    if(lowerFilename == "rtank.wsa")       return Anim_Launcher;
    if(lowerFilename == "rturret.wsa")     return Anim_RocketTurret;
    if(lowerFilename == "saboture.wsa")    return Anim_Saboteur;
    if(lowerFilename == "slab.wsa")        return Anim_Slab1;
    if(lowerFilename == "stank.wsa")       return Anim_SonicTank;
    if(lowerFilename == "starport.wsa")    return Anim_StarPort;
    if(lowerFilename == "storage.wsa")     return Anim_Silo;
    if(lowerFilename == "trike.wsa")       return Anim_Trike;
    if(lowerFilename == "turret.wsa")      return Anim_GunTurret;
    if(lowerFilename == "wall.wsa")        return Anim_Wall;
    if(lowerFilename == "windtrap.wsa")    return Anim_WindTrap;
    if(lowerFilename == "wor.wsa")         return Anim_WOR;
    if(lowerFilename == "worm.wsa")        return Anim_Sandworm;
    if(lowerFilename == "sardukar.wsa")    return Anim_Sardaukar;
    if(lowerFilename == "frigate.wsa")     return Anim_Frigate;
    if(lowerFilename == "4slab.wsa")       return Anim_Slab4;
    // clang-format on
    return 0;
}

/**
    This function returns the size of the specified item.
    \param ItemID   the id of the item (e.g. Structure_HeavyFactory)
    \return a Coord containg the size (e.g. (3,2) ). Returns (0,0) on error.
*/
Coord getStructureSize(ItemID_enum itemID) {

    // clang-format off
    switch(itemID) {
        case Structure_Barracks:            return {2,2};
        case Structure_ConstructionYard:    return {2,2};
        case Structure_GunTurret:           return {1,1};
        case Structure_HeavyFactory:        return {3,2};
        case Structure_HighTechFactory:     return {3,2};
        case Structure_IX:                  return {2,2};
        case Structure_LightFactory:        return {2,2};
        case Structure_Palace:              return {3,3};
        case Structure_Radar:               return {2,2};
        case Structure_Refinery:            return {3,2};
        case Structure_RepairYard:          return {3,2};
        case Structure_RocketTurret:        return {1,1};
        case Structure_Silo:                return {2,2};
        case Structure_StarPort:            return {3,3};
        case Structure_Slab1:               return {1,1};
        case Structure_Slab4:               return {2,2};
        case Structure_Wall:                return {1,1};
        case Structure_WindTrap:            return {2,2};
        case Structure_WOR:                 return {2,2};
        default:                            return {0,0};
    }
    // clang-format on
}

/**
    This function return the item id of an item specified by name. There may be multiple names for
    one item. The case of the name is ignored.
    \param name the name of the item (e.g. "rocket-turret" or "r-turret".
    \return the id of the item (e.g. Structure_RocketTurret)
*/
ItemID_enum getItemIDByName(std::string_view name) {
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
std::string_view getItemNameByID(ItemID_enum itemID) {
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
    static constexpr auto houseName =
        std::to_array({"Harkonnen", "Atreides", "Ordos", "Fremen", "Sardaukar", "Mercenary"});

    if (const auto idx = static_cast<decltype(houseName)::size_type>(house); idx < houseName.size())
        return houseName[idx];

    THROW(std::invalid_argument, "Invalid house number {}!", static_cast<int>(house));
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

uint32_t getColorByTerrainType(TERRAINTYPE terrainType) {
    auto color = COLOR_BLACK;
    // clang-format off
    switch (terrainType) {
        case TERRAINTYPE::Terrain_Dunes:         color = COLOR_DESERTSAND;  break;
        case TERRAINTYPE::Terrain_Mountain:      color = COLOR_MOUNTAIN;    break;
        case TERRAINTYPE::Terrain_Rock:          color = COLOR_ROCK;        break;
        case TERRAINTYPE::Terrain_Sand:          color = COLOR_DESERTSAND;  break;
        case TERRAINTYPE::Terrain_Spice:         color = COLOR_SPICE;       break;
        case TERRAINTYPE::Terrain_ThickSpice:    color = COLOR_THICKSPICE;  break;
        case TERRAINTYPE::Terrain_SpiceBloom:    color = COLOR_BLOOM;       break;
        case TERRAINTYPE::Terrain_SpecialBloom:  color = COLOR_BLOOM;       break;
        case TERRAINTYPE::Terrain_Slab:          color = COLOR_ROCK;        break;
        default:                                 color = COLOR_ROCK;        break;
    }
    // clang-format on

    return color;
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

std::string_view getDropLocationNameByID(DropLocation dropLocation) {
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
FixPoint getDeviateWeakness(HOUSETYPE house) {

    const auto* const game = dune::globals::currentGame.get();

    // Deviators are crap enough. If this is a custom game remove the weakness nerf
    // so that Ordos is playable for Humans
    if (game->gameType == GameType::CustomGame || game->gameType == GameType::CustomMultiplayer) {
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
