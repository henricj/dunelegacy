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
#include <data.h>

#include <fixmath/FixPoint.h>
#include <misc/InputStream.h>
#include <misc/OutputStream.h>

#include <array>
#include <vector>

inline constexpr auto DAMAGE_PER_TILE = 5;

// forward declarations
class House;
class ObjectBase;
class UnitBase;
class AirUnit;
class InfantryBase;

enum deadUnitEnum {
    DeadUnit_Infantry           = 1,
    DeadUnit_Infantry_Squashed1 = 2,
    DeadUnit_Infantry_Squashed2 = 3,
    DeadUnit_Carryall           = 4,
    DeadUnit_Ornithopter        = 5
};

struct DEADUNITTYPE {
    Coord realPos;
    int16_t timer;
    uint8_t type;
    HOUSETYPE house;
    bool onSand;
};

enum destroyedStructureEnum {
    DestroyedStructure_None = -1,
    DestroyedStructure_Wall = 0,
    Destroyed1x1Structure   = 1,

    Destroyed3x3Structure_TopLeft      = 2,
    Destroyed3x3Structure_TopCenter    = 3,
    Destroyed3x3Structure_TopRight     = 4,
    Destroyed3x3Structure_CenterLeft   = 5,
    Destroyed3x3Structure_CenterCenter = 6,
    Destroyed3x3Structure_CenterRight  = 7,
    Destroyed3x3Structure_BottomLeft   = 8,
    Destroyed3x3Structure_BottomCenter = 9,
    Destroyed3x3Structure_BottomRight  = 10,

    Destroyed3x2Structure_TopLeft      = 5,
    Destroyed3x2Structure_TopCenter    = 6,
    Destroyed3x2Structure_TopRight     = 7,
    Destroyed3x2Structure_BottomLeft   = 8,
    Destroyed3x2Structure_BottomCenter = 9,
    Destroyed3x2Structure_BottomRight  = 10,

    Destroyed2x2Structure_TopLeft     = 1,
    Destroyed2x2Structure_TopRight    = 11,
    Destroyed2x2Structure_BottomLeft  = 12,
    Destroyed2x2Structure_BottomRight = 13
};

class Tile final {
public:
    enum class TerrainDamage_enum { Terrain_RockDamage, Terrain_SandDamage };

    enum class SANDDAMAGETYPE { SandDamage1 = 0, SandDamage2 = 1, SandDamage3 = 2, SandDamage4 = 3 };

    enum class ROCKDAMAGETYPE { RockDamage1 = 0, RockDamage2 = 1 };

    enum class TERRAINTILETYPE {
        TerrainTile_SlabHalfDestroyed = 0x00,
        TerrainTile_SlabDestroyed     = 0x01,
        TerrainTile_Slab              = 0x02,

        TerrainTile_Sand = 0x03,

        TerrainTile_Rock          = 0x04,
        TerrainTile_RockIsland    = TerrainTile_Rock + 0x00,
        TerrainTile_RockUp        = TerrainTile_Rock + 0x01,
        TerrainTile_RockRight     = TerrainTile_Rock + 0x02,
        TerrainTile_RockUpRight   = TerrainTile_Rock + 0x03,
        TerrainTile_RockDown      = TerrainTile_Rock + 0x04,
        TerrainTile_RockUpDown    = TerrainTile_Rock + 0x05,
        TerrainTile_RockDownRight = TerrainTile_Rock + 0x06,
        TerrainTile_RockNotLeft   = TerrainTile_Rock + 0x07,
        TerrainTile_RockLeft      = TerrainTile_Rock + 0x08,
        TerrainTile_RockUpLeft    = TerrainTile_Rock + 0x09,
        TerrainTile_RockLeftRight = TerrainTile_Rock + 0x0A,
        TerrainTile_RockNotDown   = TerrainTile_Rock + 0x0B,
        TerrainTile_RockDownLeft  = TerrainTile_Rock + 0x0C,
        TerrainTile_RockNotRight  = TerrainTile_Rock + 0x0D,
        TerrainTile_RockNotUp     = TerrainTile_Rock + 0x0E,
        TerrainTile_RockFull      = TerrainTile_Rock + 0x0F,

        TerrainTile_Dunes          = 0x14,
        TerrainTile_DunesIsland    = TerrainTile_Dunes + 0x00,
        TerrainTile_DunesUp        = TerrainTile_Dunes + 0x01,
        TerrainTile_DunesRight     = TerrainTile_Dunes + 0x02,
        TerrainTile_DunesUpRight   = TerrainTile_Dunes + 0x03,
        TerrainTile_DunesDown      = TerrainTile_Dunes + 0x04,
        TerrainTile_DunesUpDown    = TerrainTile_Dunes + 0x05,
        TerrainTile_DunesDownRight = TerrainTile_Dunes + 0x06,
        TerrainTile_DunesNotLeft   = TerrainTile_Dunes + 0x07,
        TerrainTile_DunesLeft      = TerrainTile_Dunes + 0x08,
        TerrainTile_DunesUpLeft    = TerrainTile_Dunes + 0x09,
        TerrainTile_DunesLeftRight = TerrainTile_Dunes + 0x0A,
        TerrainTile_DunesNotDown   = TerrainTile_Dunes + 0x0B,
        TerrainTile_DunesDownLeft  = TerrainTile_Dunes + 0x0C,
        TerrainTile_DunesNotRight  = TerrainTile_Dunes + 0x0D,
        TerrainTile_DunesNotUp     = TerrainTile_Dunes + 0x0E,
        TerrainTile_DunesFull      = TerrainTile_Dunes + 0x0F,

        TerrainTile_Mountain          = 0x24,
        TerrainTile_MountainIsland    = TerrainTile_Mountain + 0x00,
        TerrainTile_MountainUp        = TerrainTile_Mountain + 0x01,
        TerrainTile_MountainRight     = TerrainTile_Mountain + 0x02,
        TerrainTile_MountainUpRight   = TerrainTile_Mountain + 0x03,
        TerrainTile_MountainDown      = TerrainTile_Mountain + 0x04,
        TerrainTile_MountainUpDown    = TerrainTile_Mountain + 0x05,
        TerrainTile_MountainDownRight = TerrainTile_Mountain + 0x06,
        TerrainTile_MountainNotLeft   = TerrainTile_Mountain + 0x07,
        TerrainTile_MountainLeft      = TerrainTile_Mountain + 0x08,
        TerrainTile_MountainUpLeft    = TerrainTile_Mountain + 0x09,
        TerrainTile_MountainLeftRight = TerrainTile_Mountain + 0x0A,
        TerrainTile_MountainNotDown   = TerrainTile_Mountain + 0x0B,
        TerrainTile_MountainDownLeft  = TerrainTile_Mountain + 0x0C,
        TerrainTile_MountainNotRight  = TerrainTile_Mountain + 0x0D,
        TerrainTile_MountainNotUp     = TerrainTile_Mountain + 0x0E,
        TerrainTile_MountainFull      = TerrainTile_Mountain + 0x0F,

        TerrainTile_Spice          = 0x34,
        TerrainTile_SpiceIsland    = TerrainTile_Spice + 0x00,
        TerrainTile_SpiceUp        = TerrainTile_Spice + 0x01,
        TerrainTile_SpiceRight     = TerrainTile_Spice + 0x02,
        TerrainTile_SpiceUpRight   = TerrainTile_Spice + 0x03,
        TerrainTile_SpiceDown      = TerrainTile_Spice + 0x04,
        TerrainTile_SpiceUpDown    = TerrainTile_Spice + 0x05,
        TerrainTile_SpiceDownRight = TerrainTile_Spice + 0x06,
        TerrainTile_SpiceNotLeft   = TerrainTile_Spice + 0x07,
        TerrainTile_SpiceLeft      = TerrainTile_Spice + 0x08,
        TerrainTile_SpiceUpLeft    = TerrainTile_Spice + 0x09,
        TerrainTile_SpiceLeftRight = TerrainTile_Spice + 0x0A,
        TerrainTile_SpiceNotDown   = TerrainTile_Spice + 0x0B,
        TerrainTile_SpiceDownLeft  = TerrainTile_Spice + 0x0C,
        TerrainTile_SpiceNotRight  = TerrainTile_Spice + 0x0D,
        TerrainTile_SpiceNotUp     = TerrainTile_Spice + 0x0E,
        TerrainTile_SpiceFull      = TerrainTile_Spice + 0x0F,

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

        TerrainTile_SpiceBloom   = 0x54,
        TerrainTile_SpecialBloom = 0x55,

        TerrainTile_Invalid = ~0
    };

    struct DAMAGETYPE {
        TerrainDamage_enum damageType;
        int tile;
        Coord realPos;
    };

    /**
        Default constructor. Creates a tile of type Terrain_Sand.
    */
    Tile();
    ~Tile();

    Tile(const Tile&)            = default;
    Tile(Tile&&)                 = default;
    Tile& operator=(const Tile&) = default;
    Tile& operator=(Tile&&)      = default;

    void load(InputStream& stream);
    void save(OutputStream& stream, uint32_t gameCycleCount) const;

    void assignAirUnit(uint32_t newObjectID);
    void assignDeadUnit(uint8_t type, HOUSETYPE house, const Coord& position);

    void assignNonInfantryGroundObject(uint32_t newObjectID);
    int assignInfantry(ObjectManager& objectManager, uint32_t newObjectID, int8_t currentPosition = INVALID_POS);
    void assignUndergroundUnit(uint32_t newObjectID);

    /**
        This method draws the terrain of this tile
        \param game     the game
    */
    void blitGround(Game* game);

    /**
        This method draws the structures.
    */
    void blitStructures(Game* game) const;

    /**
        This method draws the underground units of this tile.
    */
    void blitUndergroundUnits(Game* game) const;

    /**
        This method draws the dead units of this tile.
    */
    void blitDeadUnits(Game* game);

    /**
        This method draws the infantry units of this tile.
    */
    void blitInfantry(Game* game);

    /**
        This method draws the ground units of this tile.
    */
    void blitNonInfantryGroundUnits(Game* game);

    /**
        This method draws the air units of this tile.
    */
    void blitAirUnits(Game* game);

    /**
        This method draws the infantry units of this tile.
    */
    void blitSelectionRects(Game* game) const;

    void update() {
        if (deadUnits.empty())
            return;

        update_impl();
    }

    void clearTerrain();

    void setTrack(ANGLETYPE direction, uint32_t gameCycleCounter);

    void selectAllPlayersUnits(Game* game, HOUSETYPE houseID, ObjectBase** lastCheckedObject,
                               ObjectBase** lastSelectedObject);
    void selectAllPlayersUnitsOfType(Game* game, HOUSETYPE houseID, ItemID_enum itemID, ObjectBase** lastCheckedObject,
                                     ObjectBase** lastSelectedObject);
    void unassignAirUnit(uint32_t objectID);
    void unassignNonInfantryGroundObject(uint32_t objectID);
    void unassignObject(uint32_t objectID);
    void unassignInfantry(uint32_t objectID, int currentPosition);
    void unassignUndergroundUnit(uint32_t objectID);
    void setType(const GameContext& context, TERRAINTYPE newType);
    void squash(const GameContext& context) const;
    int getInfantryTeam(const ObjectManager& objectManager) const;
    FixPoint harvestSpice(const GameContext& context);
    void setSpice(FixPoint newSpice);

    /**
        Returns the center point of this tile
        \return the center point in world coordinates
    */
    Coord getCenterPoint() const {
        return Coord(location.x * TILESIZE + (TILESIZE / 2), location.y * TILESIZE + (TILESIZE / 2));
    }

    /*!
        returns a pointer to an air unit on this tile (if there's one)
        @return AirUnit* pointer to air unit
    */
    AirUnit* getAirUnit(const ObjectManager& objectManager) const;

    /*!
        returns a pointer to a non infantry ground object on this tile (if there's one)
        @return ObjectBase*  pointer to non infantry ground object
    */
    ObjectBase* getNonInfantryGroundObject(const ObjectManager& objectManager) const;
    /*!
        returns a pointer to an underground object on this tile (if there's one)
        @return UnitBase*  pointer to underground object(sandworm?)
    */
    UnitBase* getUndergroundUnit(const ObjectManager& objectManager) const;

    /*!
        returns a pointer to an ground object on this tile (if there's one)
        @return ObjectBase*  pointer to ground object
    */
    ObjectBase* getGroundObject(const ObjectManager& objectManager) const;

    template<typename ObjectType>
    ObjectType* getGroundObject(const ObjectManager& objectManager) const {
        return dune_cast<ObjectType>(getGroundObject(objectManager));
    }

    std::pair<bool, Dune::object_id_type> getGroundObjectID() const {
        if (hasANonInfantryGroundObject())
            return {true, assignedNonInfantryGroundObjectList.front()};
        if (hasInfantry())
            return {true, assignedInfantryList.front()};

        return {false, 0};
    }

    /*!
        returns a pointer to infantry object on this tile (if there's one)
        @return InfantryBase*  pointer to infantry object
    */
    InfantryBase* getInfantry(const ObjectManager& objectManager) const;
    ObjectBase* getObject(const ObjectManager& objectManager) const;
    ObjectBase* getObjectAt(const ObjectManager& objectManager, int x, int y) const;
    ObjectBase* getObjectWithID(const ObjectManager& objectManager, uint32_t objectID) const;

    const std::vector<uint32_t>& getAirUnitList() const { return assignedAirUnitList; }

    const std::vector<uint32_t>& getInfantryList() const { return assignedInfantryList; }

    const std::vector<uint32_t>& getUndergroundUnitList() const { return assignedUndergroundUnitList; }

    const std::vector<uint32_t>& getNonInfantryGroundObjectList() const { return assignedNonInfantryGroundObjectList; }

    /**
        This method is called when the spice bloom on this till shall be triggered. If this tile has no spice bloom
       nothing happens. \param  pTrigger    the house that triggered the bloom
    */
    void triggerSpiceBloom(const GameContext& context, House* pTrigger);

    /**
        This method is called when the spice bloom on this tile shall be triggered. If this tile has no spice bloom
       nothing happens. \param  pTrigger    the house that triggered the bloom
    */
    void triggerSpecialBloom(const GameContext& context, House* pTrigger);

    /**
        Sets this tile as explored for this house.
        \param  houseID the house this tile should be explored for
        \param  cycle   the cycle this happens (normally the current game cycle)
    */
    void setExplored(HOUSETYPE houseID, uint32_t cycle) {
        lastAccess[static_cast<int>(houseID)] = cycle;
        explored[static_cast<int>(houseID)]   = true;
    }

    void setOwner(HOUSETYPE newOwner) noexcept { owner = newOwner; }
    void setSandRegion(uint32_t newSandRegion) noexcept { sandRegion = newSandRegion; }
    void setDestroyedStructureTile(int newDestroyedStructureTile) noexcept {
        destroyedStructureTile = newDestroyedStructureTile;
    }

    bool hasAGroundObject() const noexcept { return (hasInfantry() || hasANonInfantryGroundObject()); }
    bool hasAnAirUnit() const noexcept { return !assignedAirUnitList.empty(); }
    bool hasAnUndergroundUnit() const noexcept { return !assignedUndergroundUnitList.empty(); }
    bool hasANonInfantryGroundObject() const noexcept { return !assignedNonInfantryGroundObjectList.empty(); }
    bool hasAStructure(const ObjectManager& objectManager) const;
    bool hasInfantry() const noexcept { return !assignedInfantryList.empty(); }
    bool hasAnObject() const noexcept { return (hasAGroundObject() || hasAnAirUnit() || hasAnUndergroundUnit()); }

    bool hasSpice() const noexcept { return (spice > 0); }
    bool infantryNotFull() const noexcept { return (assignedInfantryList.size() < NUM_INFANTRY_PER_TILE); }
    bool isConcrete() const noexcept { return (type == TERRAINTYPE::Terrain_Slab); }
    bool isExploredByHouse(HOUSETYPE houseID) const { return explored[static_cast<int>(houseID)]; }
    bool isExploredByTeam(const Game* game, int teamID) const;

    bool isFoggedByHouse(bool fogOfWarEnabled, uint32_t gameCycleCount, HOUSETYPE houseID) const noexcept;
    bool isFoggedByTeam(const Game* game, int teamID) const;
    bool isMountain() const noexcept { return (type == TERRAINTYPE::Terrain_Mountain); }
    bool isRock() const noexcept {
        return ((type == TERRAINTYPE::Terrain_Rock) || (type == TERRAINTYPE::Terrain_Slab)
                || (type == TERRAINTYPE::Terrain_Mountain));
    }

    bool isSand() const noexcept { return (type == TERRAINTYPE::Terrain_Sand); }
    bool isDunes() const noexcept { return (type == TERRAINTYPE::Terrain_Dunes); }
    bool isSpiceBloom() const noexcept { return (type == TERRAINTYPE::Terrain_SpiceBloom); }
    bool isSpecialBloom() const noexcept { return (type == TERRAINTYPE::Terrain_SpecialBloom); }
    bool isSpice() const noexcept {
        return ((type == TERRAINTYPE::Terrain_Spice) || (type == TERRAINTYPE::Terrain_ThickSpice));
    }
    bool isThickSpice() const noexcept { return (type == TERRAINTYPE::Terrain_ThickSpice); }

    uint32_t getSandRegion() const noexcept { return sandRegion; }
    HOUSETYPE getOwner() const noexcept { return owner; }
    TERRAINTYPE getType() const noexcept { return type; }
    FixPoint getSpice() const noexcept { return spice; }

    FixPoint getSpiceRemaining() const noexcept { return spice; }

    const Coord& getLocation() const noexcept { return location; }

    uint32_t getRadarColor(const Game* game, House* pHouse, bool radar);
    TERRAINTILETYPE getTerrainTile() const {
        if (terrainTile == TERRAINTILETYPE::TerrainTile_Invalid)
            terrainTile = getTerrainTileImpl();

        return terrainTile;
    }
    int getHideTile(const Game* game, int teamID) const;
    int getFogTile(const Game* game, int teamID) const;
    int getDestroyedStructureTile() const noexcept { return destroyedStructureTile; }

    bool isBlocked() const noexcept { return (isMountain() || hasAGroundObject()); }

    void addDamage(Tile::TerrainDamage_enum damageType, int tile, Coord realPos) {
        if (damage.size() >= DAMAGE_PER_TILE)
            return;

        DAMAGETYPE newDamage;
        newDamage.tile       = tile;
        newDamage.damageType = damageType;
        newDamage.realPos    = realPos;

        damage.push_back(newDamage);
    }

    Coord location; ///< location of this tile in map coordinates

private:
    TERRAINTYPE type; ///< the type of the tile (Terrain_Sand, Terrain_Rock, ...)

    uint32_t fogColor{COLOR_BLACK}; ///< remember last color (radar)

    HOUSETYPE owner{HOUSETYPE::HOUSE_INVALID}; ///< house ID of the owner of this tile
    uint32_t sandRegion{NONE_ID};              ///< used by sandworms to check if can get to a unit

    FixPoint spice{0}; ///< how much spice on this particular tile is left

    zoomable_texture sprite{}; ///< the graphic to draw

    int32_t destroyedStructureTile{DestroyedStructure_None}; ///< the tile drawn for a destroyed structure
    mutable TERRAINTILETYPE terrainTile{TERRAINTILETYPE::TerrainTile_Invalid};
    std::array<uint32_t, static_cast<int>(ANGLETYPE::NUM_ANGLES)>
        tracksCreationTime{};            ///< Contains the game cycle the tracks on sand appeared
    std::vector<DAMAGETYPE> damage;      ///< damage positions
    std::vector<DEADUNITTYPE> deadUnits; ///< dead units

    std::vector<uint32_t> assignedAirUnitList;                 ///< all the air units on this tile
    std::vector<uint32_t> assignedInfantryList;                ///< all infantry units on this tile
    std::vector<uint32_t> assignedUndergroundUnitList;         ///< all underground units on this tile
    std::vector<uint32_t> assignedNonInfantryGroundObjectList; ///< all structures/vehicles on this tile

    std::array<uint32_t, NUM_TEAMS> lastAccess{}; ///< contains for every team when this tile was seen last by this house
    std::array<bool, NUM_TEAMS> explored{};       ///< contains for every team if this tile is explored

    void update_impl();

    template<typename Pred>
    void selectFilter(Game* game, HOUSETYPE houseID, ObjectBase** lastCheckedObject, ObjectBase** lastSelectedObject,
                      Pred&& predicate);

    template<typename Visitor>
    void forEachUnit(Visitor&& visitor) const;

    TERRAINTILETYPE getTerrainTileImpl() const;
};

#endif // TILE_H
