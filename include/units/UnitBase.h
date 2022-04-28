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

#ifndef UNITBASE_H
#define UNITBASE_H

#include <Map.h>
#include <ObjectBase.h>

#include <House.h>

// forward declarations
class Tile;

class UnitBaseConstants : public ObjectBaseConstants {
public:
    constexpr explicit UnitBaseConstants(ItemID_enum itemID, int num_weapons = 0,
                                         BulletID_enum bullet_id = BulletID_enum::Bullet_Rocket)
        : ObjectBaseConstants{itemID}, numWeapons_(num_weapons), bulletType_(bullet_id) {
        aUnit_          = true;
        canAttackStuff_ = 0 != num_weapons;
    }

    [[nodiscard]] bool isTracked() const noexcept { return tracked_; }

    [[nodiscard]] bool isTurreted() const noexcept { return turreted_; }

    [[nodiscard]] int numWeapons() const noexcept { return numWeapons_; }

    [[nodiscard]] int bulletType() const noexcept { return bulletType_; }

protected:
    // constant for all units of the same type
    bool tracked_{};                 ///< Does this unit have tracks?
    bool turreted_{};                ///< Does this unit have a turret?
    int numWeapons_{};               ///< How many weapons do we have?
    int bulletType_{Bullet_DRocket}; ///< Type of bullet to shot with
};

class UnitBase : public ObjectBase {
protected:
    UnitBase(const UnitBaseConstants& constants, uint32_t objectID, const ObjectInitializer& initializer);
    UnitBase(const UnitBaseConstants& constants, uint32_t objectID, const ObjectStreamInitializer& initializer);

public:
    ~UnitBase() override = 0;

    UnitBase(const UnitBase&)            = delete;
    UnitBase(UnitBase&&)                 = delete;
    UnitBase& operator=(const UnitBase&) = delete;
    UnitBase& operator=(UnitBase&&)      = delete;

    void save(OutputStream& stream) const override;

    void blitToScreen() override;

    std::unique_ptr<ObjectInterface> getInterfaceContainer(const GameContext& context) override;

    virtual void checkPos(const GameContext& context) = 0;
    virtual void deploy(const GameContext& context, const Coord& newLocation);

    void destroy(const GameContext& context) override;
    void deviate(const GameContext& context, House* newOwner);

    void drawSelectionBox() override;
    void cleanup(const GameContext& context, HumanPlayer* humanPlayer) override;

    void drawOtherPlayerSelectionBox() override;

    /**
        This method is called when an unit is ordered by a right click
        \param  xPos    the x position on the map
        \param  yPos    the y position on the map
    */
    void handleActionClick(const GameContext& context, int xPos, int yPos) override;

    /**
        This method is called when an unit is ordered to attack
        \param  xPos    the x position on the map
        \param  yPos    the y position on the map
    */
    virtual void handleAttackClick(const GameContext& context, int xPos, int yPos);

    /**
        This method is called when an unit is ordered to move
        \param  xPos    the x position on the map
        \param  yPos    the y position on the map
    */
    virtual void handleMoveClick(const GameContext& context, int xPos, int yPos);

    /**
        This method is called when an unit is ordered to be in a new attack mode
        \param  newAttackMode   the new attack mode the unit is put in.
    */
    virtual void handleSetAttackModeClick(const GameContext& context, ATTACKMODE newAttackMode);

    /**
        This method is called when an unit is ordered to request a carryall drop
        \param  xPos    the x position on the map
        \param  yPos    the y position on the map
    */
    virtual void handleRequestCarryallDropClick(const GameContext& context, int xPos, int yPos);

    /**
        This method is called when an unit should move to (xPos,yPos)
        \param  xPos    the x position on the map
        \param  yPos    the y position on the map
        \param  bForced true, if the unit should ignore everything else
    */
    virtual void doMove2Pos(const GameContext& context, int xPos, int yPos, bool bForced);

    /**
        This method is called when an unit should move to coord
        \param  coord   the position on the map
        \param  bForced true, if the unit should ignore everything else
    */
    virtual void doMove2Pos(const GameContext& context, const Coord& coord, bool bForced);

    /**
        This method is called when an unit should move to another unit/structure
        \param  TargetObjectID  the ID of the other unit/structure
    */
    virtual void doMove2Object(const GameContext& context, uint32_t TargetObjectID);

    /**
        This method is called when an unit should move to another unit/structure
        \param  pTargetObject   the other unit/structure
    */
    virtual void doMove2Object(const GameContext& context, const ObjectBase* pTargetObject);

    /**
        This method is called when an unit should attack a position
        \param  xPos    the x position on the map
        \param  yPos    the y position on the map
        \param  bForced true, if the unit should ignore everything else
    */
    virtual void doAttackPos(const GameContext& context, int xPos, int yPos, bool bForced);

    /**
        This method is called when an unit should attack to another unit/structure
        \param  pTargetObject   the target unit/structure
        \param  bForced true, if the unit should ignore everything else
    */
    virtual void doAttackObject(const GameContext& context, const ObjectBase* pTargetObject, bool bForced);

    /**
        This method is called when an unit should attack to another unit/structure
        \param  TargetObjectID  the ID of the other unit/structure
        \param  bForced true, if the unit should ignore everything else
    */
    virtual void doAttackObject(const GameContext& context, uint32_t TargetObjectID, bool bForced);

    /**
        This method is called when an unit should change it's current attack mode
        \param  newAttackMode   the new attack mode
    */
    void doSetAttackMode(const GameContext& context, ATTACKMODE newAttackMode);

    void handleDamage(const GameContext& context, int damage, uint32_t damagerID, House* damagerOwner) override;

    void doRepair(const GameContext& context) noexcept override { }

    /**
        Is this object in a range we are guarding. If yes we shall react.
        \param  object  the object to check
    */
    bool isInGuardRange(const ObjectBase* object) const;

    /**
        Is this object in a range we want to attack. If no, we should stop following it.
        \param  object  the object to check
    */
    bool isInAttackRange(const ObjectBase* object) const;

    /**
        Is this object in a range we can attack.
        \param  object  the object to check
    */
    bool isInWeaponRange(const ObjectBase* object) const;

    void setAngle(ANGLETYPE newAngle);

    void setTarget(const ObjectBase* newTarget) override;

    void setGettingRepaired();

    void setGuardPoint(const Coord& newGuardPoint) { setGuardPoint(newGuardPoint.x, newGuardPoint.y); }

    void setGuardPoint(int newX, int newY);

    using ObjectBase::setLocation;
    void setLocation(const GameContext& context, int xPos, int yPos) override;

    using ObjectBase::setDestination;
    void setDestination(int newX, int newY) override {
        if ((destination.x != newX) || (destination.y != newY)) {
            ObjectBase::setDestination(newX, newY);
            clearPath();
        }
    }

    virtual void setPickedUp(const GameContext& context, UnitBase* newCarrier);

    /**
        Updates this unit.
        \return true if this unit still exists, false if it was destroyed
    */
    bool update(const GameContext& context) override;

    bool canPass(int xPos, int yPos) const {
        const auto* const pTile = dune::globals::currentGameMap->tryGetTile(xPos, yPos);

        return pTile ? canPassTile(pTile) : false;
    }

    virtual bool canPassTile(const Tile* pTile) const;

    virtual bool hasBumpyMovementOnRock() const { return false; }

    /**
        Returns how fast a unit can move over the specified terrain type.
        \param  terrainType the type to consider
        \return Returns a speed factor. Higher values mean slower.
    */
    virtual FixPoint getTerrainDifficulty(TERRAINTYPE terrainType) const { return 1; }

    virtual ANGLETYPE getCurrentAttackAngle() const;

    virtual FixPoint getMaxSpeed(const GameContext& context) const;

    void clearPath() {
        pathList.clear();
        nextSpotFound        = false;
        recalculatePathTimer = 0;
        nextSpotAngle        = ANGLETYPE::INVALID_ANGLE;
        noCloserPointCount   = 0;
    }

    bool isTracked() const { return unit_constants().isTracked(); }

    bool isTurreted() const noexcept { return unit_constants().isTurreted(); }

    int numWeapons() const noexcept { return unit_constants().numWeapons(); }

    int bulletType() const noexcept { return unit_constants().bulletType(); }

    bool isMoving() const noexcept { return moving; }

    bool wasDeviated() const noexcept { return (owner->getHouseID() != originalHouseID); }

    ANGLETYPE getAngle() const noexcept { return drawnAngle; }

    ATTACKMODE getAttackMode() const noexcept { return attackMode; }

    const Coord& getGuardPoint() const noexcept { return guardPoint; }

    virtual void playAttackSound();

protected:
    const UnitBaseConstants& unit_constants() const noexcept {
        return *static_cast<const UnitBaseConstants*>(&constants_);
    }

    void updateVisibleUnits(const GameContext& context);

    virtual bool attack(const GameContext& context);

    virtual void releaseTarget();
    virtual void engageTarget(const GameContext& context);
    virtual void move(const GameContext& context);

    virtual void
    bumpyMovementOnRock(FixPoint fromDistanceX, FixPoint fromDistanceY, FixPoint toDistanceX, FixPoint toDistanceY);

    virtual void navigate(const GameContext& context);

    /**
        When the unit is currently idling this method is called about every 5 seconds.
    */
    virtual void idleAction(const GameContext& context);

    virtual void setSpeeds(const GameContext& context);

    virtual void targeting(const GameContext& context);

    virtual void turn(const GameContext& context);
    void turnLeft(const GameContext& context);
    void turnRight(const GameContext& context);

    void quitDeviation(const GameContext& context);

    bool SearchPathWithAStar();

    void drawSmoke(float x, float y) const;

    // unit state/properties
    Coord guardPoint; ///< The guard point where to return to after the micro-AI hunted some nearby enemy unit
    Coord attackPos;  ///< The position to attack
    bool goingToRepairYard = false; ///< Are we currently going to a repair yard?
    bool pickedUp          = false; ///< Were we picked up by a carryall?
    bool bFollow           = false; ///< Do we currently follow some other unit (specified by target)?

    bool moving            = false; ///< Are we currently moving?
    bool turning           = false; ///< Are we currently turning?
    bool justStoppedMoving = false; ///< Do we have just stopped moving?
    FixPoint xSpeed        = 0;     ///< Speed in x direction
    FixPoint ySpeed        = 0;     ///< Speed in y direction
    FixPoint bumpyOffsetX  = 0;     ///< The bumpy offset in x direction which is already included in realX
    FixPoint bumpyOffsetY  = 0;     ///< The bumpy offset in y direction which is already included in realY

    FixPoint targetDistance = 0; ///< Distance to the destination
    ANGLETYPE targetAngle;       ///< Angle to the destination

    // path finding
    uint8_t noCloserPointCount = 0;     ///< How often have we tried to dinf a path?
    bool nextSpotFound         = false; ///< Is the next spot to move to already found?
    ANGLETYPE nextSpotAngle;            ///< The angle to get to the next spot
    int32_t recalculatePathTimer = 0;   ///< This timer is for recalculating the best path after x ticks
    Coord nextSpot;                     ///< The next spot to move to
    std::vector<Coord> pathList;        ///< The path to the destination found so far

    int32_t findTargetTimer      = 0;       ///< When to look for the next target?
    int32_t primaryWeaponTimer   = 0;       ///< When can the primary weapon shot again?
    int32_t secondaryWeaponTimer = INVALID; ///< When can the secondary weapon shot again?

    // deviation
    int32_t deviationTimer = INVALID; ///< When to revert back to the original owner?

    // drawing information
    int drawnFrame; ///< Which row in the picture should be drawn

private:
    void init();

    void navigate_fallback(const GameContext& context);
};

template<>
inline UnitBase* dune_cast(ObjectBase* base) {
    if (base && base->isAUnit())
        return static_cast<UnitBase*>(base); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

    return nullptr;
}

template<>
inline const UnitBase* dune_cast(const ObjectBase* base) { // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    if (base && base->isAUnit())
        return static_cast<const UnitBase*>(base);

    return nullptr;
}

#endif // UNITBASE_H
