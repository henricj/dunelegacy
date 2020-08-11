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

#ifndef ENGINE_OBJECTBASE_H
#define ENGINE_OBJECTBASE_H

#include "data.h"
#include <ObjectPointer.h>
#include "EngineDefinitions.h"
#include "EngineDataTypes.h"
#include <fixmath/FixPoint.h>

#include <bitset>
#include <cassert>

// forward declarations
class Game;
class House;
class HumanPlayer;
class InputStream;
class OutputStream;
class ObjectInterface;
class ObjectManager;
class Map;
class Coord;


#define VIS_ALL -1

class ObjectInitializer final {
public:
    ObjectInitializer(Game& game, House* owner, bool byScenario) : game_{game}, owner_{owner}, byScenario_{byScenario} { }

    ObjectInitializer()                         = delete;
    ObjectInitializer(const ObjectInitializer&) = delete;
    ObjectInitializer(ObjectInitializer&&)      = delete;
    ObjectInitializer& operator=(const ObjectInitializer&) = delete;
    ObjectInitializer& operator=(ObjectInitializer&&) = delete;

    ~ObjectInitializer() = default;

    Game&  game() const noexcept { return game_; }
    House* owner() const noexcept { return owner_; }
    bool   byScenario() const noexcept { return byScenario_; }

private:
    Game& game_;
    House* const owner_;
    const bool   byScenario_;
};

class ObjectStreamInitializer final {
public:
    ObjectStreamInitializer(InputStream& inputStream) : stream_{inputStream} { }

    ObjectStreamInitializer()                               = delete;
    ObjectStreamInitializer(const ObjectStreamInitializer&) = delete;
    ObjectStreamInitializer(ObjectStreamInitializer&&)      = delete;
    ObjectStreamInitializer& operator=(const ObjectStreamInitializer&) = delete;
    ObjectStreamInitializer& operator=(ObjectStreamInitializer&&) = delete;

    ~ObjectStreamInitializer() = default;

    InputStream& stream() const noexcept { return stream_; }

private:
    InputStream& stream_;
};

class ObjectBaseConstants
{
public:
    bool canAttack() const noexcept { return canAttackStuff_; }
    bool isAFlyingUnit() const noexcept { return aFlyingUnit_; }
    bool isAGroundUnit() const noexcept { return aGroundUnit_; }
    bool isAStructure() const noexcept { return aStructure_; }
    bool isABuilder() const noexcept { return aBuilder_; }
    bool isInfantry() const noexcept { return infantry_; }
    bool isAUnit() const noexcept { return aUnit_; }

    constexpr explicit ObjectBaseConstants(ItemID_enum itemID) : itemID{itemID} { }

    // constant for all objects of the same type
    const ItemID_enum itemID; ///< The ItemID of this object.

protected:
    bool aStructure_{};      ///< Is this a structure?
    bool aBuilder_{};        ///< Is this a builder?

    bool aUnit_{};           ///< Is this a unit?
    bool aFlyingUnit_{};     ///< Is this a flying unit?
    bool aGroundUnit_{};     ///< Is this a ground unit?
    bool infantry_{};        ///< Is this an infantry unit?

    bool canAttackStuff_{};  ///< Can this unit/structure attack?
};

/*!
    Class from which all structure and unit classes are derived
*/
class ObjectBase
{
    ObjectBase(const ObjectBaseConstants& object_constants, uint32_t objectID);

protected:
    ObjectBase(const ObjectBaseConstants& object_constants, uint32_t objectID, const ObjectInitializer& initializer);
    ObjectBase(const ObjectBaseConstants& object_constants, uint32_t objectID, const ObjectStreamInitializer& initializer);

public:
    using parent = ObjectBase;

    virtual ~ObjectBase() = 0;

    ObjectBase(const ObjectBase &) = delete;
    ObjectBase(ObjectBase &&) = delete;
    ObjectBase& operator=(const ObjectBase &) = delete;
    ObjectBase& operator=(ObjectBase &&) = delete;

    virtual void save(OutputStream& stream) const;

    virtual void assignToMap(const GameContext& context, const Coord& pos) = 0;

    virtual void destroy(const GameContext& context);

    virtual void cleanup(const GameContext& context, HumanPlayer* humanPlayer) = 0;

    virtual Coord getClosestCenterPoint(const Coord& objectLocation) const;

    virtual bool canAttack(const ObjectBase* object) const;

    virtual Coord getCenterPoint() const;

    virtual void handleDamage(const GameContext& context, int damage, uint32_t damagerID, House* damagerOwner);

    virtual void doRepair(const GameContext& context) = 0;

    virtual void setDestination(int newX, int newY);
    virtual void setHealth(FixPoint newHealth);

    virtual void setLocation(const GameContext& context, int xPos, int yPos);

    virtual void setTarget(const ObjectBase* newTarget);
    void setVisible(int teamID, bool status);

    /**
        Updates this object.
        \return true if this object still exists, false if it was destroyed
    */
    virtual bool update(const GameContext& context) = 0;

    void unassignFromMap(const Coord& location) const;

    bool isOnScreen() const;
    bool isVisible(int teamID) const;
    bool isVisible() const;
    uint32_t getHealthColor() const;

    /**
        This method returns the closest coordinate of this object to objectLocation. If this is a building
        it returns the coordinate of the tile of the building that is closest to this object.
        \param objectLocation the location of the other object
        \return the coordinate that is closest in tile coordinates
    */
    virtual Coord getClosestPoint(const Coord& objectLocation) const;

    const StructureBase* findClosestTargetStructure() const;
    const UnitBase* findClosestTargetUnit() const;
    const ObjectBase* findClosestTarget() const;
    virtual const ObjectBase* findTarget() const;

    bool canAttack() const noexcept { return constants_.canAttack(); }
    bool isAFlyingUnit() const noexcept { return constants_.isAFlyingUnit(); }
    bool isAGroundUnit() const noexcept { return constants_.isAGroundUnit(); }
    bool isAStructure() const noexcept { return constants_.isAStructure(); }
    bool isABuilder() const noexcept { return constants_.isABuilder(); }
    bool isInfantry() const noexcept { return constants_.isInfantry(); }
    bool isAUnit() const noexcept { return constants_.isAUnit(); }

    void addHealth() { if (health < getMaxHealth()) setHealth(health + 1); }
    void setActive(bool status) noexcept { active = status; }
    void setForced(bool status) noexcept { forced = status; }
    void setRespondable(bool status) noexcept { respondable = status; }
    void setSelected(bool value) noexcept { selected = value; }
    void setSelectedByOtherPlayer(bool value) noexcept { selectedByOtherPlayer = value; }
    void setDestination(const Coord& location) { setDestination(location.x, location.y); }
    void setLocation(const GameContext& context, const Coord& location) { setLocation(context, location.x, location.y); }
    bool hasATarget() const noexcept { return (target); }
    bool hasObjectID(uint32_t id) const noexcept { return (objectID == id); }
    bool isActive() const noexcept { return active; }
    bool isRespondable() const noexcept { return respondable; }
    bool isByScenario() const noexcept { return byScenario; }
    bool isSelected() const noexcept { return selected; }
    bool isSelectedByOtherPlayer() const noexcept { return selectedByOtherPlayer; }
    bool isBadlyDamaged() const noexcept { return badlyDamaged; };
    bool wasForced() const noexcept { return forced; }
    ItemID_enum getItemID() const noexcept { return itemID; }
    int getX() const noexcept { return location.x; }
    int getY() const noexcept { return location.y; }

    FixPoint getHealth() const noexcept { return health; }
    int getMaxHealth() const;
    uint32_t getObjectID() const noexcept { return objectID; }


    int getViewRange() const;
    int getAreaGuardRange() const;
    int getWeaponRange() const;
    int getWeaponReloadTime() const;
    int getInfSpawnProp() const;

    FixPoint getRealX() const noexcept { return realX; }
    FixPoint getRealY() const noexcept { return realY; }
    const Coord& getLocation() const noexcept { return location; }
    const Coord& getDestination() const noexcept { return destination; }
    ObjectBase* getTarget(const ObjectManager& objectManager) noexcept { return target.getObjPointer(objectManager); }
    const ObjectBase* getTarget(const ObjectManager& objectManager) const noexcept { return target.getObjPointer(objectManager); }

    HOUSETYPE getOriginalHouseID() const noexcept { return originalHouseID; }
    virtual void setOriginalHouseID(HOUSETYPE i) { originalHouseID = i; }

    House* getOwner() const noexcept {
        assert(owner);
        return owner;
    }

    void setOwner(House* no) noexcept {
        assert(no);
        owner = no;
    }

    static std::unique_ptr<ObjectBase> createObject(ItemID_enum itemID, uint32_t objectID,
                                                    const ObjectInitializer& initializer);
    static std::unique_ptr<ObjectBase> loadObject(ItemID_enum itemID, uint32_t objectID,
                                                  const ObjectStreamInitializer& initializer);

protected:
    bool targetInWeaponRange() const;

    // constant for all objects of the same type
    const ObjectBaseConstants& constants_;

    const ItemID_enum itemID = ItemID_Invalid; ///< The ItemID of this object.

    // object state/properties
    const uint32_t   objectID;               ///< The unique object ID of this object
    HOUSETYPE      originalHouseID;  ///< for takeover/deviation, we still want to keep track of what the original house was
    House    *owner;                 ///< The owner of this object

    Coord    location;               ///< The current position of this object in tile coordinates
    Coord    oldLocation;            ///< The previous position of this object in tile coordinates (used when moving from one tile to the next tile)
    Coord    destination;            ///< The destination tile
    FixPoint realX;                  ///< The x-coordinate of this object in world coordinates
    FixPoint realY;                  ///< The y-coordinate of this object in world coordinates

    FixPoint angle;                  ///< The current angle of this unit/structure (8 = 360°)
    ANGLETYPE drawnAngle;           ///< The angle this unit/structure is drawn with. (e.g. 0 to 7)

    bool     active;                 ///< Is this unit/structure active?
    bool     respondable;            ///< Is this unit/structure respondable to commands?
    bool     byScenario;             ///< Did this unit/structure either already exist at the start of the map or is a reinforcement?
    bool     selected;               ///< Is this object currently selected?
    bool     selectedByOtherPlayer;  ///< This is only used in multiplayer games where two players control one house

    bool          forced;            ///< Is this unit/structure forced to do what it currently does or did the micro-AI decide to do that?
    bool          targetFriendly;    ///< Is the current target a friendly unit/structure to follow/move to instead to attack?
    ObjectPointer target;            ///< The target to attack or move to
    ATTACKMODE    attackMode;        ///< The attack mode of this unit/structure

    std::bitset<NUM_TEAMS> visible;  ///< To which teams is this unit visible?

    // drawing information
    bool     badlyDamaged;           ///< Is the health below 50%?

private:
    FixPoint health;                 ///< The health of this object
    void init();
};

template<typename ObjectType>
ObjectType* dune_cast(ObjectBase* base) {
    static_assert(std::is_base_of<ObjectBase, ObjectType>::value, "UnitType not derived from UnitBase");
    static_assert(!std::is_abstract<ObjectType>::value, "ObjectType is abstract");

    if(base && ObjectType::item_id == base->getItemID()) return static_cast<ObjectType*>(base);

    return nullptr;
}

template<typename ObjectType>
const ObjectType* dune_cast(const ObjectBase* base) {
    static_assert(std::is_base_of<ObjectBase, ObjectType>::value, "UnitType not derived from UnitBase");
    static_assert(!std::is_abstract<ObjectType>::value, "ObjectType is abstract");

    if(base && ObjectType::item_id == base->getItemID()) return static_cast<const ObjectType*>(base);

    return nullptr;
}

#endif // ENGINE_OBJECTBASE_H
