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

#ifndef OBJECTBASE_H
#define OBJECTBASE_H

#include <ObjectPointer.h>
#include <Definitions.h>
#include <DataTypes.h>
#include <fixmath/FixPoint.h>

#include <globals.h>

#include <bitset>

#include <SDL2/SDL.h>

// forward declarations
class House;
class InputStream;
class OutputStream;
class ObjectInterface;
class Coord;
template<class WidgetData> class Container;

#define VIS_ALL -1

class ObjectInitializer final {
public:
    ObjectInitializer(House* owner, bool byScenario) : Owner{owner}, ByScenario{byScenario} { }

    ObjectInitializer()                         = delete;
    ObjectInitializer(const ObjectInitializer&) = delete;
    ObjectInitializer(ObjectInitializer&&)      = delete;
    ObjectInitializer& operator=(const ObjectInitializer&) = delete;
    ObjectInitializer& operator=(ObjectInitializer&&) = delete;

    House* const Owner;
    const bool   ByScenario;
};

class ObjectStreamInitializer final {
public:
    ObjectStreamInitializer(InputStream& inputStream) : Stream{inputStream} { }

    ObjectStreamInitializer()                               = delete;
    ObjectStreamInitializer(const ObjectStreamInitializer&) = delete;
    ObjectStreamInitializer(ObjectStreamInitializer&&)      = delete;
    ObjectStreamInitializer& operator=(const ObjectStreamInitializer&) = delete;
    ObjectStreamInitializer& operator=(ObjectStreamInitializer&&) = delete;

    InputStream& Stream;
};

/*!
    Class from which all structure and unit classes are derived
*/
class ObjectBase
{
    ObjectBase(ItemID_enum itemID, Uint32 objectID);

protected:
    ObjectBase(ItemID_enum itemID, Uint32 objectID, const ObjectInitializer& initializer);
    ObjectBase(ItemID_enum itemID, Uint32 objectID, const ObjectStreamInitializer& initializer);

public:
    virtual ~ObjectBase() = 0;

    ObjectBase(const ObjectBase &) = delete;
    ObjectBase(ObjectBase &&) = delete;
    ObjectBase& operator=(const ObjectBase &) = delete;
    ObjectBase& operator=(ObjectBase &&) = delete;

    virtual void save(OutputStream& stream) const;

    virtual ObjectInterface* getInterfaceContainer();

    virtual void assignToMap(const Coord& pos) = 0;
    virtual void blitToScreen() = 0;

    virtual void drawSelectionBox() { ; };
    virtual void drawOtherPlayerSelectionBox() { ; };

    virtual void destroy() = 0;

    virtual void cleanup(Game* game, HumanPlayer* humanPlayer, Map* map) = 0;

    virtual Coord getClosestCenterPoint(const Coord& objectLocation) const;

    virtual bool canAttack(const ObjectBase* object) const;

    virtual Coord getCenterPoint() const;

    virtual void handleDamage(int damage, Uint32 damagerID, House* damagerOwner);

    /**
        This method is called when an object is ordered by a right click
        \param  xPos    the x position on the map
        \param  yPos    the y position on the map
    */
    virtual void handleActionClick(int xPos, int yPos) = 0;

    virtual void doRepair() = 0;

    virtual void handleInterfaceEvent(SDL_Event* event);

    virtual void playSelectSound() = 0;
    virtual void playConfirmSound() = 0;

    virtual void setDestination(int newX, int newY);
    virtual void setHealth(FixPoint newHealth);

    virtual void setLocation(int xPos, int yPos);

    virtual void setTarget(const ObjectBase* newTarget);
    void setVisible(int teamID, bool status);

    /**
        Updates this object.
        \return true if this object still exists, false if it was destroyed
    */
    virtual bool update() = 0;

    void unassignFromMap(const Coord& location) const;

    bool isOnScreen() const;
    bool isVisible(int teamID) const;
    bool isVisible() const;
    Uint32 getHealthColor() const;

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

    void addHealth() { if (health < getMaxHealth()) setHealth(health + 1); }
    void setActive(bool status) noexcept { active = status; }
    void setForced(bool status) noexcept { forced = status; }
    void setRespondable(bool status) noexcept { respondable = status; }
    void setSelected(bool value) noexcept { selected = value; }
    void setSelectedByOtherPlayer(bool value) noexcept { selectedByOtherPlayer = value; }
    void setDestination(const Coord& location) { setDestination(location.x, location.y); }
    void setLocation(const Coord& location) { setLocation(location.x, location.y); }
    bool canAttack() const noexcept { return canAttackStuff; }
    bool hasATarget() const noexcept { return (target); }
    bool hasObjectID(Uint32 id) const noexcept { return (objectID == id); }
    bool isActive() const noexcept { return active; }
    bool isAFlyingUnit() const noexcept { return aFlyingUnit; }
    bool isAGroundUnit() const noexcept { return aGroundUnit; }
    bool isAStructure() const noexcept { return aStructure; }
    bool isABuilder() const noexcept { return aBuilder; }
    bool isInfantry() const noexcept { return infantry; }
    bool isAUnit() const noexcept { return aUnit; }
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
    Uint32 getObjectID() const noexcept { return objectID; }


    int getViewRange() const;
    int getAreaGuardRange() const;
    int getWeaponRange() const;
    int getWeaponReloadTime() const;
    int getInfSpawnProp() const;

    FixPoint getRealX() const noexcept { return realX; }
    FixPoint getRealY() const noexcept { return realY; }
    const Coord& getLocation() const noexcept { return location; }
    const Coord& getDestination() const noexcept { return destination; }
    ObjectBase* getTarget() noexcept { return target.getObjPointer(); }
    const ObjectBase* getTarget() const noexcept { return target.getObjPointer(); }

    HOUSETYPE getOriginalHouseID() const noexcept { return originalHouseID; }
    virtual void setOriginalHouseID(HOUSETYPE i) { originalHouseID = i; }
    House* getOwner() const noexcept { return owner; }

    void setOwner(House* no) noexcept { owner = no; }

    static std::unique_ptr<ObjectBase> createObject(ItemID_enum itemID, Uint32 objectID,
                                                    const ObjectInitializer& initializer);
    static std::unique_ptr<ObjectBase> loadObject(ItemID_enum itemID, Uint32 objectID,
                                                  const ObjectStreamInitializer& initializer);

protected:
    bool targetInWeaponRange() const;

    // constant for all objects of the same type
    const ItemID_enum itemID = ItemID_Invalid; ///< The ItemID of this object.
    int      radius = TILESIZE/2;    ///< The radius of this object

    bool     aStructure{};           ///< Is this a structure?
    bool     aBuilder{};             ///< Is this a builder?

    bool     aUnit{};                ///< Is this a unit?
    bool     aFlyingUnit{};          ///< Is this a flying unit?
    bool     aGroundUnit{};          ///< Is this a ground unit?
    bool     infantry{};             ///< Is this an infantry unit?

    bool     canAttackStuff{};       ///< Can this unit/structure attack?

    // object state/properties
    const Uint32   objectID;               ///< The unique object ID of this object
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

    zoomable_texture graphic{};      ///< The graphic for this object
    int         graphicID = -1;      ///< The id of the graphic (needed if we want to reload the graphic, e.g. when a unit is deviated)
    int         numImagesX = 0;      ///< The number of images in x direction
    int         numImagesY = 0;      ///< The number of images in y direction

private:
    FixPoint health;                 ///< The health of this object
    void init();
};



#endif // OBJECTBASE_H
