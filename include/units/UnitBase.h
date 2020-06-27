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

#include <ObjectBase.h>
#include <Map.h>

#include <House.h>

#include <deque>

// forward declarations
class Tile;

class UnitBase : public ObjectBase
{
public:
    explicit UnitBase(House* newOwner);
    explicit UnitBase(InputStream& stream);
    void init();
    ~UnitBase() override = 0;

    UnitBase(const UnitBase &) = delete;
    UnitBase(UnitBase &&) = delete;
    UnitBase& operator=(const UnitBase &) = delete;
    UnitBase& operator=(UnitBase &&) = delete;

    void save(OutputStream& stream) const override;

    void blitToScreen() override;

    ObjectInterface* getInterfaceContainer() override;

    virtual void checkPos() = 0;
    virtual void deploy(const Coord& newLocation);

    void destroy() override;
    void deviate(House* newOwner);

    void drawSelectionBox() override;
    void drawOtherPlayerSelectionBox() override;

    /**
        This method is called when an unit is ordered by a right click
        \param  xPos    the x position on the map
        \param  yPos    the y position on the map
    */
    void handleActionClick(int xPos, int yPos) override;

    /**
        This method is called when an unit is ordered to attack
        \param  xPos    the x position on the map
        \param  yPos    the y position on the map
    */
    virtual void handleAttackClick(int xPos, int yPos);

    /**
        This method is called when an unit is ordered to move
        \param  xPos    the x position on the map
        \param  yPos    the y position on the map
    */
    virtual void handleMoveClick(int xPos, int yPos);


    /**
        This method is called when an unit is ordered to be in a new attack mode
        \param  newAttackMode   the new attack mode the unit is put in.
    */
    virtual void handleSetAttackModeClick(ATTACKMODE newAttackMode);


    /**
        This method is called when an unit is ordered to request a carryall drop
        \param  xPos    the x position on the map
        \param  yPos    the y position on the map
    */
    virtual void handleRequestCarryallDropClick(int xPos, int yPos);

    /**
        This method is called when an unit should move to (xPos,yPos)
        \param  xPos    the x position on the map
        \param  yPos    the y position on the map
        \param  bForced true, if the unit should ignore everything else
    */
    virtual void doMove2Pos(int xPos, int yPos, bool bForced);

    /**
        This method is called when an unit should move to coord
        \param  coord   the position on the map
        \param  bForced true, if the unit should ignore everything else
    */
    virtual void doMove2Pos(const Coord& coord, bool bForced);

    /**
        This method is called when an unit should move to another unit/structure
        \param  TargetObjectID  the ID of the other unit/structure
    */
    virtual void doMove2Object(Uint32 TargetObjectID);

    /**
        This method is called when an unit should move to another unit/structure
        \param  pTargetObject   the other unit/structure
    */
    virtual void doMove2Object(const ObjectBase* pTargetObject);

    /**
        This method is called when an unit should attack a position
        \param  xPos    the x position on the map
        \param  yPos    the y position on the map
        \param  bForced true, if the unit should ignore everything else
    */
    virtual void doAttackPos(int xPos, int yPos, bool bForced);

    /**
        This method is called when an unit should attack to another unit/structure
        \param  pTargetObject   the target unit/structure
        \param  bForced true, if the unit should ignore everything else
    */
    virtual void doAttackObject(const ObjectBase* pTargetObject, bool bForced);

    /**
        This method is called when an unit should attack to another unit/structure
        \param  TargetObjectID  the ID of the other unit/structure
        \param  bForced true, if the unit should ignore everything else
    */
    virtual void doAttackObject(Uint32 TargetObjectID, bool bForced);

    /**
        This method is called when an unit should change it's current attack mode
        \param  newAttackMode   the new attack mode
    */
    void doSetAttackMode(ATTACKMODE newAttackMode);

    void handleDamage(int damage, Uint32 damagerID, House* damagerOwner) override;

    void doRepair() noexcept override { }

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
    void setLocation(int xPos, int yPos) override;

    using ObjectBase::setDestination;
    void setDestination(int newX, int newY) override
    {
        if((destination.x != newX) || (destination.y != newY)) {
            ObjectBase::setDestination(newX, newY);
            clearPath();
        }
    }

    virtual void setPickedUp(UnitBase* newCarrier);

    /**
        Updates this unit.
        \return true if this unit still exists, false if it was destroyed
    */
    bool update() override;

    bool canPass(int xPos, int yPos) const {
        const auto pTile = currentGameMap->tryGetTile(xPos, yPos);

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

    virtual FixPoint getMaxSpeed() const;

    void clearPath() {
        pathList.clear();
        nextSpotFound = false;
        recalculatePathTimer = 0;
        nextSpotAngle = ANGLETYPE::INVALID_ANGLE;;
        noCloserPointCount = 0;
    }

    bool isTracked() const { return tracked; }

     bool isTurreted() const noexcept { return turreted; }

     bool isMoving() const noexcept { return moving; }

     bool wasDeviated() const noexcept { return (owner->getHouseID() != originalHouseID); }

     ANGLETYPE getAngle() const noexcept { return drawnAngle; }

     ATTACKMODE getAttackMode() const noexcept { return attackMode; }

     const Coord& getGuardPoint() const noexcept { return guardPoint; }

    virtual void playAttackSound();

protected:

    void updateVisibleUnits();

    virtual bool attack();

    virtual void releaseTarget();
    virtual void engageTarget();
    virtual void move();

    virtual void bumpyMovementOnRock(FixPoint fromDistanceX, FixPoint fromDistanceY, FixPoint toDistanceX, FixPoint toDistanceY);

    virtual void navigate();

    /**
        When the unit is currently idling this method is called about every 5 seconds.
    */
    virtual void idleAction();

    virtual void setSpeeds();

    virtual void targeting();

    virtual void turn();
    void turnLeft();
    void turnRight();

    void quitDeviation();

    bool SearchPathWithAStar();

    void drawSmoke(int x, int y) const;

    // constant for all units of the same type
    bool     tracked;                ///< Does this unit have tracks?
    bool     turreted;               ///< Does this unit have a turret?
    int      numWeapons;             ///< How many weapons do we have?
    int      bulletType;             ///< Type of bullet to shot with

    // unit state/properties
    Coord    guardPoint;             ///< The guard point where to return to after the micro-AI hunted some nearby enemy unit
    Coord    attackPos;              ///< The position to attack
    bool     goingToRepairYard;      ///< Are we currently going to a repair yard?
    bool     pickedUp;               ///< Were we picked up by a carryall?
    bool     bFollow;                ///< Do we currently follow some other unit (specified by target)?


    bool     moving;                 ///< Are we currently moving?
    bool     turning;                ///< Are we currently turning?
    bool     justStoppedMoving;      ///< Do we have just stopped moving?
    FixPoint xSpeed;                 ///< Speed in x direction
    FixPoint ySpeed;                 ///< Speed in y direction
    FixPoint bumpyOffsetX;           ///< The bumpy offset in x direction which is already included in realX
    FixPoint bumpyOffsetY;           ///< The bumpy offset in y direction which is already included in realY

    FixPoint targetDistance;         ///< Distance to the destination
    ANGLETYPE targetAngle;           ///< Angle to the destination

    // path finding
    Uint8    noCloserPointCount;     ///< How often have we tried to dinf a path?
    bool     nextSpotFound;          ///< Is the next spot to move to already found?
    ANGLETYPE nextSpotAngle;         ///< The angle to get to the next spot
    Sint32   recalculatePathTimer;   ///< This timer is for recalculating the best path after x ticks
    Coord    nextSpot;               ///< The next spot to move to
    std::vector<Coord> pathList;     ///< The path to the destination found so far

    Sint32  findTargetTimer;         ///< When to look for the next target?
    Sint32  primaryWeaponTimer;      ///< When can the primary weapon shot again?
    Sint32  secondaryWeaponTimer;    ///< When can the secondary weapon shot again?

    // deviation
    Sint32          deviationTimer;  ///< When to revert back to the original owner?

    // drawing information
    int drawnFrame;                  ///< Which row in the picture should be drawn
};

#endif //UNITBASE_H
