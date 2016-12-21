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

#ifndef DATA_H
#define DATA_H

// data definitions
typedef enum {
    Bullet_DRocket = 0,
    Bullet_LargeRocket = 1,
    Bullet_Rocket = 2,
    Bullet_TurretRocket = 3,
    Bullet_ShellSmall = 4,
    Bullet_ShellMedium = 5,
    Bullet_ShellLarge = 6,
    Bullet_ShellTurret = 7,
    Bullet_SmallRocket = 8,
    Bullet_Sonic = 9,
    Bullet_Sandworm = 10
} BulletID_enum;

typedef enum {
    Explosion_Small = 0,
    Explosion_Medium1 = 1,
    Explosion_Medium2 = 2,
    Explosion_Large1 = 3,
    Explosion_Large2 = 4,
    Explosion_Gas = 5,
    Explosion_ShellSmall = 6,
    Explosion_ShellMedium = 7,
    Explosion_ShellLarge = 8,
    Explosion_SmallUnit = 9,
    Explosion_Flames = 10,
    Explosion_SpiceBloom = 11
} ExplosionID_enum;

typedef enum {
    ItemID_Invalid = 0,
    ItemID_FirstID = 1,

    Structure_FirstID = 1,
    Structure_Barracks = 1,
    Structure_ConstructionYard = 2,
    Structure_GunTurret = 3,
    Structure_HeavyFactory = 4,
    Structure_HighTechFactory = 5,
    Structure_IX = 6,
    Structure_LightFactory = 7,
    Structure_Palace = 8,
    Structure_Radar = 9,
    Structure_Refinery = 10,
    Structure_RepairYard = 11,
    Structure_RocketTurret = 12,
    Structure_Silo = 13,
    Structure_Slab1 = 14,
    Structure_Slab4 = 15,
    Structure_StarPort = 16,
    Structure_Wall = 17,
    Structure_WindTrap = 18,
    Structure_WOR = 19,
    Structure_LastID = 19,

    Unit_FirstID = 20,
    Unit_Carryall = 20,
    Unit_Devastator = 21,
    Unit_Deviator = 22,
    Unit_Frigate = 23,
    Unit_Harvester = 24,
    Unit_Soldier = 25,
    Unit_Launcher = 26,
    Unit_MCV = 27,
    Unit_Ornithopter = 28,
    Unit_Quad = 29,
    Unit_Saboteur = 30,
    Unit_Sandworm = 31,
    Unit_SiegeTank = 32,
    Unit_SonicTank = 33,
    Unit_Tank = 34,
    Unit_Trike = 35,
    Unit_RaiderTrike = 36,
    Unit_Trooper = 37,
    Unit_Special = 38,
    Unit_Infantry = 39,
    Unit_Troopers = 40,
    Unit_LastID = 40,
    ItemID_LastID = 40,

    Num_ItemID
} ItemID_enum;

typedef enum {
    Terrain_Slab,
    Terrain_Sand,
    Terrain_Rock,
    Terrain_Dunes,
    Terrain_Mountain,
    Terrain_Spice,
    Terrain_ThickSpice,
    Terrain_SpiceBloom,
    Terrain_SpecialBloom
} TERRAINTYPE;

typedef enum {
    Terrain_HiddenIsland    = 0x0,
    Terrain_HiddenUp        = 0x1,
    Terrain_HiddenRight     = 0x2,
    Terrain_HiddenUpRight   = 0x3,
    Terrain_HiddenDown      = 0x4,
    Terrain_HiddenUpDown    = 0x5,
    Terrain_HiddenDownRight = 0x6,
    Terrain_HiddenNotLeft   = 0x7,
    Terrain_HiddenLeft      = 0x8,
    Terrain_HiddenUpLeft    = 0x9,
    Terrain_HiddenLeftRight = 0xA,
    Terrain_HiddenNotDown   = 0xB,
    Terrain_HiddenDownLeft  = 0xC,
    Terrain_HiddenNotRight  = 0xD,
    Terrain_HiddenNotUp     = 0xE,
    Terrain_HiddenFull      = 0xF
} HIDDENTYPE;

/**
    This function determines if the specified itemID is an unit or not.
    \param itemID   the ID of the item (e.g. Unit_Harvester)
    \return true if it is an unit, false otherwise
*/
inline bool isUnit(int itemID) { return (itemID >= Unit_FirstID && itemID <= Unit_LastID); }

/**
    This function determines if the specified itemID is a structure or not.
    \param itemID   the ID of the item (e.g. Structure_ConstructionYard)
    \return true if it is a structure, false otherwise
*/
inline bool isStructure(int itemID) { return (itemID >= Structure_FirstID && itemID <= Structure_LastID); }


/**
    This function determines if the specified itemID is a flying unit or not.
    \param itemID   the ID of the item (e.g. Unit_Carryall)
    \return true if it is a flying unit, false otherwise
*/
inline bool isFlyingUnit(int itemID) { return (itemID == Unit_Carryall) || (itemID == Unit_Ornithopter) || (itemID == Unit_Frigate); }

/**
    This function determines if the specified itemID is an infantry unit or not.
    \param itemID   the ID of the item (e.g. Unit_Carryall)
    \return true if it is an infantry unit, false otherwise
*/
inline bool isInfantryUnit(int itemID) { return (itemID == Unit_Soldier) || (itemID == Unit_Trooper) || (itemID == Unit_Infantry) || (itemID == Unit_Troopers) || (itemID == Unit_Saboteur); }


#endif // DATA_H
