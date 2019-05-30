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

#ifndef TILE_H
#define TILE_H

#include <DataTypes.h>
#include <mmath.h>
#include <data.h>

#include <misc/InputStream.h>
#include <misc/OutputStream.h>
#include <fixmath/FixPoint.h>

#include <list>
#include <vector>
#include <algorithm>

#define DAMAGE_PER_TILE 5


// forward declarations
class House;
class ObjectBase;
class UnitBase;
class AirUnit;
class InfantryBase;


enum deadUnitEnum {
    DeadUnit_Infantry = 1,
    DeadUnit_Infantry_Squashed1 = 2,
    DeadUnit_Infantry_Squashed2 = 3,
    DeadUnit_Carrall = 4,
    DeadUnit_Ornithopter = 5
};

typedef struct
{
    Uint32 damageType;
    int tile;
    Coord realPos;
} DAMAGETYPE;

typedef struct
{
    Coord   realPos;
    Sint16  timer;
    Uint8   type;
    Uint8   house;
    bool    onSand;
} DEADUNITTYPE;

enum destroyedStructureEnum {
    DestroyedStructure_None             = -1,
    DestroyedStructure_Wall             = 0,
    Destroyed1x1Structure               = 1,

    Destroyed3x3Structure_TopLeft       = 2,
    Destroyed3x3Structure_TopCenter     = 3,
    Destroyed3x3Structure_TopRight      = 4,
    Destroyed3x3Structure_CenterLeft    = 5,
    Destroyed3x3Structure_CenterCenter  = 6,
    Destroyed3x3Structure_CenterRight   = 7,
    Destroyed3x3Structure_BottomLeft    = 8,
    Destroyed3x3Structure_BottomCenter  = 9,
    Destroyed3x3Structure_BottomRight   = 10,

    Destroyed3x2Structure_TopLeft       = 5,
    Destroyed3x2Structure_TopCenter     = 6,
    Destroyed3x2Structure_TopRight      = 7,
    Destroyed3x2Structure_BottomLeft    = 8,
    Destroyed3x2Structure_BottomCenter  = 9,
    Destroyed3x2Structure_BottomRight   = 10,

    Destroyed2x2Structure_TopLeft       = 1,
    Destroyed2x2Structure_TopRight      = 11,
    Destroyed2x2Structure_BottomLeft    = 12,
    Destroyed2x2Structure_BottomRight   = 13
};


class Tile
{
public:

    typedef enum {
        Terrain_RockDamage,
        Terrain_SandDamage
    } TerrainDamage_enum;

    typedef enum {
        SandDamage1 = 0,
        SandDamage2 = 1,
        SandDamage3 = 2,
        SandDamage4 = 3
    } SANDDAMAGETYPE;

    typedef enum {
        RockDamage1 = 0,
        RockDamage2 = 1
    } ROCKDAMAGETYPE;

    typedef enum {
        TerrainTile_SlabHalfDestroyed   = 0x00,
        TerrainTile_SlabDestroyed       = 0x01,
        TerrainTile_Slab                = 0x02,

        TerrainTile_Sand                = 0x03,

        TerrainTile_Rock                = 0x04,
        TerrainTile_RockIsland          = TerrainTile_Rock + 0x00,
        TerrainTile_RockUp              = TerrainTile_Rock + 0x01,
        TerrainTile_RockRight           = TerrainTile_Rock + 0x02,
        TerrainTile_RockUpRight         = TerrainTile_Rock + 0x03,
        TerrainTile_RockDown            = TerrainTile_Rock + 0x04,
        TerrainTile_RockUpDown          = TerrainTile_Rock + 0x05,
        TerrainTile_RockDownRight       = TerrainTile_Rock + 0x06,
        TerrainTile_RockNotLeft         = TerrainTile_Rock + 0x07,
        TerrainTile_RockLeft            = TerrainTile_Rock + 0x08,
        TerrainTile_RockUpLeft          = TerrainTile_Rock + 0x09,
        TerrainTile_RockLeftRight       = TerrainTile_Rock + 0x0A,
        TerrainTile_RockNotDown         = TerrainTile_Rock + 0x0B,
        TerrainTile_RockDownLeft        = TerrainTile_Rock + 0x0C,
        TerrainTile_RockNotRight        = TerrainTile_Rock + 0x0D,
        TerrainTile_RockNotUp           = TerrainTile_Rock + 0x0E,
        TerrainTile_RockFull            = TerrainTile_Rock + 0x0F,

        TerrainTile_Dunes               = 0x14,
        TerrainTile_DunesIsland         = TerrainTile_Dunes + 0x00,
        TerrainTile_DunesUp             = TerrainTile_Dunes + 0x01,
        TerrainTile_DunesRight          = TerrainTile_Dunes + 0x02,
        TerrainTile_DunesUpRight        = TerrainTile_Dunes + 0x03,
        TerrainTile_DunesDown           = TerrainTile_Dunes + 0x04,
        TerrainTile_DunesUpDown         = TerrainTile_Dunes + 0x05,
        TerrainTile_DunesDownRight      = TerrainTile_Dunes + 0x06,
        TerrainTile_DunesNotLeft        = TerrainTile_Dunes + 0x07,
        TerrainTile_DunesLeft           = TerrainTile_Dunes + 0x08,
        TerrainTile_DunesUpLeft         = TerrainTile_Dunes + 0x09,
        TerrainTile_DunesLeftRight      = TerrainTile_Dunes + 0x0A,
        TerrainTile_DunesNotDown        = TerrainTile_Dunes + 0x0B,
        TerrainTile_DunesDownLeft       = TerrainTile_Dunes + 0x0C,
        TerrainTile_DunesNotRight       = TerrainTile_Dunes + 0x0D,
        TerrainTile_DunesNotUp          = TerrainTile_Dunes + 0x0E,
        TerrainTile_DunesFull           = TerrainTile_Dunes + 0x0F,

        TerrainTile_Mountain            = 0x24,
        TerrainTile_MountainIsland      = TerrainTile_Mountain + 0x00,
        TerrainTile_MountainUp          = TerrainTile_Mountain + 0x01,
        TerrainTile_MountainRight       = TerrainTile_Mountain + 0x02,
        TerrainTile_MountainUpRight     = TerrainTile_Mountain + 0x03,
        TerrainTile_MountainDown        = TerrainTile_Mountain + 0x04,
        TerrainTile_MountainUpDown      = TerrainTile_Mountain + 0x05,
        TerrainTile_MountainDownRight   = TerrainTile_Mountain + 0x06,
        TerrainTile_MountainNotLeft     = TerrainTile_Mountain + 0x07,
        TerrainTile_MountainLeft        = TerrainTile_Mountain + 0x08,
        TerrainTile_MountainUpLeft      = TerrainTile_Mountain + 0x09,
        TerrainTile_MountainLeftRight   = TerrainTile_Mountain + 0x0A,
        TerrainTile_MountainNotDown     = TerrainTile_Mountain + 0x0B,
        TerrainTile_MountainDownLeft    = TerrainTile_Mountain + 0x0C,
        TerrainTile_MountainNotRight    = TerrainTile_Mountain + 0x0D,
        TerrainTile_MountainNotUp       = TerrainTile_Mountain + 0x0E,
        TerrainTile_MountainFull        = TerrainTile_Mountain + 0x0F,

        TerrainTile_Spice               = 0x34,
        TerrainTile_SpiceIsland         = TerrainTile_Spice + 0x00,
        TerrainTile_SpiceUp             = TerrainTile_Spice + 0x01,
        TerrainTile_SpiceRight          = TerrainTile_Spice + 0x02,
        TerrainTile_SpiceUpRight        = TerrainTile_Spice + 0x03,
        TerrainTile_SpiceDown           = TerrainTile_Spice + 0x04,
        TerrainTile_SpiceUpDown         = TerrainTile_Spice + 0x05,
        TerrainTile_SpiceDownRight      = TerrainTile_Spice + 0x06,
        TerrainTile_SpiceNotLeft        = TerrainTile_Spice + 0x07,
        TerrainTile_SpiceLeft           = TerrainTile_Spice + 0x08,
        TerrainTile_SpiceUpLeft         = TerrainTile_Spice + 0x09,
        TerrainTile_SpiceLeftRight      = TerrainTile_Spice + 0x0A,
        TerrainTile_SpiceNotDown        = TerrainTile_Spice + 0x0B,
        TerrainTile_SpiceDownLeft       = TerrainTile_Spice + 0x0C,
        TerrainTile_SpiceNotRight       = TerrainTile_Spice + 0x0D,
        TerrainTile_SpiceNotUp          = TerrainTile_Spice + 0x0E,
        TerrainTile_SpiceFull           = TerrainTile_Spice + 0x0F,

        TerrainTile_ThickSpice          = 0x44,
        TerrainTile_ThickSpiceIsland    = TerrainTile_ThickSpice + 0x00,
        TerrainTile_ThickSpiceUp        = TerrainTile_ThickSpice + 0x01,
        TerrainTile_ThickSpiceRight     = TerrainTile_ThickSpice + 0x02,
        TerrainTile_ThickSpiceUpRight   = TerrainTile_ThickSpice + 0x03,
        TerrainTile_ThickSpiceDown      = TerrainTile_ThickSpice + 0x04,
        TerrainTile_ThickSpiceUpDown    = TerrainTile_ThickSpice + 0x05,
        TerrainTile_ThickSpiceDownRight = TerrainTile_ThickSpice + 0x06,
        TerrainTile_ThickSpiceNotLeft   = TerrainTile_ThickSpice + 0x07,
        TerrainTile_ThickSpiceLeft      = TerrainTile_ThickSpice + 0x08,
        TerrainTile_ThickSpiceUpLeft    = TerrainTile_ThickSpice + 0x09,
        TerrainTile_ThickSpiceLeftRight = TerrainTile_ThickSpice + 0x0A,
        TerrainTile_ThickSpiceNotDown   = TerrainTile_ThickSpice + 0x0B,
        TerrainTile_ThickSpiceDownLeft  = TerrainTile_ThickSpice + 0x0C,
        TerrainTile_ThickSpiceNotRight  = TerrainTile_ThickSpice + 0x0D,
        TerrainTile_ThickSpiceNotUp     = TerrainTile_ThickSpice + 0x0E,
        TerrainTile_ThickSpiceFull      = TerrainTile_ThickSpice + 0x0F,

        TerrainTile_SpiceBloom          = 0x54,
        TerrainTile_SpecialBloom        = 0x55
    } TERRAINTILETYPE;


    /**
        Default constructor. Creates a tile of type Terrain_Sand.
    */
    Tile();
    ~Tile();

    void load(InputStream& stream);
    void save(OutputStream& stream) const;

    void assignAirUnit(Uint32 newObjectID);
    void assignDeadUnit(Uint8 type, Uint8 house, const Coord& position) {
        DEADUNITTYPE newDeadUnit;
        newDeadUnit.type = type;
        newDeadUnit.house = house;
        newDeadUnit.onSand = isSand() || isDunes();
        newDeadUnit.realPos = position;
        newDeadUnit.timer = 2000;

        deadUnits.push_back(newDeadUnit);
    }

    void assignNonInfantryGroundObject(Uint32 newObjectID);
    int assignInfantry(Uint32 newObjectID, Sint8 currentPosition = INVALID_POS);
    void assignUndergroundUnit(Uint32 newObjectID);

    /**
        This method draws the terrain of this tile
        \param xPos the x position of the left top corner of this tile on the screen
        \param yPos the y position of the left top corner of this tile on the screen
    */
    void blitGround(int xPos, int yPos);

    /**
        This method draws the structures.
        \param xPos the x position of the left top corner of this tile on the screen
        \param yPos the y position of the left top corner of this tile on the screen
    */
    void blitStructures(int xPos, int yPos) const;

    /**
        This method draws the underground units of this tile.
        \param xPos the x position of the left top corner of this tile on the screen
        \param yPos the y position of the left top corner of this tile on the screen
    */
    void blitUndergroundUnits(int xPos, int yPos) const;

    /**
        This method draws the dead units of this tile.
        \param xPos the x position of the left top corner of this tile on the screen
        \param yPos the y position of the left top corner of this tile on the screen
    */
    void blitDeadUnits(int xPos, int yPos);

    /**
        This method draws the infantry units of this tile.
        \param xPos the x position of the left top corner of this tile on the screen
        \param yPos the y position of the left top corner of this tile on the screen
    */
    void blitInfantry(int xPos, int yPos);

    /**
        This method draws the ground units of this tile.
        \param xPos the x position of the left top corner of this tile on the screen
        \param yPos the y position of the left top corner of this tile on the screen
    */
    void blitNonInfantryGroundUnits(int xPos, int yPos);

    /**
        This method draws the air units of this tile.
        \param xPos the x position of the left top corner of this tile on the screen
        \param yPos the y position of the left top corner of this tile on the screen
    */
    void blitAirUnits(int xPos, int yPos);

    /**
        This method draws the infantry units of this tile.
        \param xPos the x position of the left top corner of this tile on the screen
        \param yPos the y position of the left top corner of this tile on the screen
    */
    void blitSelectionRects(int xPos, int yPos) const;


    void update() {
        if (deadUnits.empty())
            return;

        update_impl();
    }

    void clearTerrain();

    void setTrack(Uint8 direction);

    void selectAllPlayersUnits(int houseID, ObjectBase** lastCheckedObject, ObjectBase** lastSelectedObject);
    void selectAllPlayersUnitsOfType(int houseID, int itemID, ObjectBase** lastCheckedObject, ObjectBase** lastSelectedObject);
    void unassignAirUnit(Uint32 objectID);
    void unassignNonInfantryGroundObject(Uint32 objectID);
    void unassignObject(Uint32 objectID);
    void unassignInfantry(Uint32 objectID, int currentPosition);
    void unassignUndergroundUnit(Uint32 objectID);
    void setType(int newType);
    void squash() const;
    int getInfantryTeam() const;
    FixPoint harvestSpice();
    void setSpice(FixPoint newSpice);

    /**
        Returns the center point of this tile
        \return the center point in world coordinates
    */
    Coord getCenterPoint() const {
        return Coord(location.x*TILESIZE + (TILESIZE / 2), location.y*TILESIZE + (TILESIZE / 2));
    }

    /*!
        returns a pointer to an air unit on this tile (if there's one)
        @return AirUnit* pointer to air unit
    */
    AirUnit* getAirUnit() const;

    /*!
        returns a pointer to a non infantry ground object on this tile (if there's one)
        @return ObjectBase*  pointer to non infantry ground object
    */
    ObjectBase* getNonInfantryGroundObject() const;
    /*!
        returns a pointer to an underground object on this tile (if there's one)
        @return UnitBase*  pointer to underground object(sandworm?)
    */
    UnitBase* getUndergroundUnit() const;

    /*!
        returns a pointer to an ground object on this tile (if there's one)
        @return ObjectBase*  pointer to ground object
    */
    ObjectBase* getGroundObject() const;

    /*!
        returns a pointer to infantry object on this tile (if there's one)
        @return InfantryBase*  pointer to infantry object
    */
    InfantryBase* getInfantry() const;
    ObjectBase* getObject() const;
    ObjectBase* getObjectAt(int x, int y) const;
    ObjectBase* getObjectWithID(Uint32 objectID) const;


    const std::list<Uint32>& getAirUnitList() const {
        return assignedAirUnitList;
    }

    const std::list<Uint32>& getInfantryList() const {
        return assignedInfantryList;
    }

    const std::list<Uint32>& getUndergroundUnitList() const {
        return assignedUndergroundUnitList;
    }

    const std::list<Uint32>& getNonInfantryGroundObjectList() const {
        return assignedNonInfantryGroundObjectList;
    }

    /**
        This method is called when the spice bloom on this till shall be triggered. If this tile has no spice bloom nothing happens.
        \param  pTrigger    the house that triggered the bloom
    */
    void triggerSpiceBloom(House* pTrigger);

    /**
        This method is called when the spice bloom on this tile shall be triggered. If this tile has no spice bloom nothing happens.
        \param  pTrigger    the house that triggered the bloom
    */
    void triggerSpecialBloom(House* pTrigger);

    /**
        Sets this tile as explored for this house.
        \param  houseID the house this tile should be explored for
        \param  cycle   the cycle this happens (normally the current game cycle)
    */
    void setExplored(int houseID, Uint32 cycle) {
        lastAccess[houseID] = cycle;
        explored[houseID] = true;
    }

    void setOwner(int newOwner) noexcept { owner = newOwner; }
    void setSandRegion(Uint32 newSandRegion) noexcept { sandRegion = newSandRegion; }
    void setDestroyedStructureTile(int newDestroyedStructureTile) noexcept { destroyedStructureTile = newDestroyedStructureTile; };

    bool hasAGroundObject() const noexcept { return (hasInfantry() || hasANonInfantryGroundObject()); }
    bool hasAnAirUnit() const noexcept { return !assignedAirUnitList.empty(); }
    bool hasAnUndergroundUnit() const noexcept { return !assignedUndergroundUnitList.empty(); }
    bool hasANonInfantryGroundObject() const noexcept { return !assignedNonInfantryGroundObjectList.empty(); }
    bool hasAStructure() const;
    bool hasInfantry() const noexcept { return !assignedInfantryList.empty(); }
    bool hasAnObject() const noexcept { return (hasAGroundObject() || hasAnAirUnit() || hasAnUndergroundUnit()); }

    bool hasSpice() const noexcept { return (spice > 0); }
    bool infantryNotFull() const noexcept { return (assignedInfantryList.size() < NUM_INFANTRY_PER_TILE); }
    bool isConcrete() const noexcept { return (type == Terrain_Slab); }
    bool isExploredByHouse(int houseID) const { return explored[houseID]; }
    bool isExploredByTeam(int teamID) const;

    bool isFoggedByHouse(int houseID) const noexcept;
    bool isFoggedByTeam(int teamID) const noexcept;
    bool isMountain() const noexcept { return (type == Terrain_Mountain); }
    bool isRock() const noexcept { return ((type == Terrain_Rock) || (type == Terrain_Slab) || (type == Terrain_Mountain)); }

    bool isSand() const noexcept { return (type == Terrain_Sand); }
    bool isDunes() const noexcept { return (type == Terrain_Dunes); }
    bool isSpiceBloom() const noexcept { return (type == Terrain_SpiceBloom); }
    bool isSpecialBloom() const noexcept { return (type == Terrain_SpecialBloom); }
    bool isSpice() const noexcept { return ((type == Terrain_Spice) || (type == Terrain_ThickSpice)); }
    bool isThickSpice() const noexcept { return (type == Terrain_ThickSpice); }

    Uint32 getSandRegion() const noexcept { return sandRegion; }
    int getOwner() const noexcept { return owner; }
    int getType() const noexcept { return type; }
    FixPoint getSpice() const noexcept { return spice; }

    FixPoint getSpiceRemaining() const noexcept { return spice; }

    const Coord& getLocation() const noexcept { return location; }

    Uint32 getRadarColor(House* pHouse, bool radar);
    int getTerrainTile() const;
    int getHideTile(int teamID) const;
    int getFogTile(int teamID) const;
    int getDestroyedStructureTile() const noexcept { return  destroyedStructureTile; };

    bool isBlocked() const noexcept {
        return (isMountain() || hasAGroundObject());
    }


    void addDamage(Uint32 damageType, int tile, Coord realPos) {
        if (damage.size() >= DAMAGE_PER_TILE) return;

        DAMAGETYPE newDamage;
        newDamage.tile = tile;
        newDamage.damageType = damageType;
        newDamage.realPos = realPos;

        damage.push_back(newDamage);
    }

    Coord   location;   ///< location of this tile in map coordinates

private:

    Uint32      type;           ///< the type of the tile (Terrain_Sand, Terrain_Rock, ...)

    Uint32      fogColor;       ///< remember last color (radar)

    Sint32      owner;          ///< house ID of the owner of this tile
    Uint32      sandRegion;     ///< used by sandworms to check if can get to a unit

    FixPoint    spice;          ///< how much spice on this particular tile is left

    zoomable_texture sprite{};  ///< the graphic to draw

    Sint32                          destroyedStructureTile;         ///< the tile drawn for a destroyed structure
    Uint32                          tracksCreationTime[NUM_ANGLES]; ///< Contains the game cycle the tracks on sand appeared
    std::vector<DAMAGETYPE>         damage;                         ///< damage positions
    std::vector<DEADUNITTYPE>       deadUnits;                      ///< dead units

    std::list<Uint32>   assignedAirUnitList;                      ///< all the air units on this tile
    std::list<Uint32>   assignedInfantryList;                     ///< all infantry units on this tile
    std::list<Uint32>   assignedUndergroundUnitList;              ///< all underground units on this tile
    std::list<Uint32>   assignedNonInfantryGroundObjectList;      ///< all structures/vehicles on this tile

    Uint32      lastAccess[NUM_TEAMS];    ///< contains for every team when this tile was seen last by this house
    bool        explored[NUM_TEAMS];      ///< contains for every team if this tile is explored

    void update_impl();

    template<typename Pred>
    void selectFilter(int houseID, ObjectBase** lastCheckedObject, ObjectBase** lastSelectedObject, Pred&& predicate);
};



#endif //TILE_H
