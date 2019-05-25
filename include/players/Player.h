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

#ifndef PLAYER_H
#define PLAYER_H

#include <data.h>
#include <DataTypes.h>
#include <misc/InputStream.h>
#include <misc/OutputStream.h>
#include <misc/RobustList.h>
#include <misc/string_util.h>

class GameInitSettings;
class Random;
class Map;
class House;
class ObjectBase;
class StructureBase;
class BuilderBase;
class StarPort;
class ConstructionYard;
class Palace;
class TurretBase;
class GroundUnit;
class UnitBase;
class Devastator;
class Harvester;
class InfantryBase;
class MCV;

class Player {
public:

    Player(House* associatedHouse, const std::string& playername);
    Player(InputStream& stream, House* associatedHouse);
    virtual ~Player() = 0;

    Player(const Player &) = delete;
    Player(Player &&) = delete;
    Player& operator=(const Player &) = delete;
    Player& operator=(Player &&) = delete;

    virtual void save(OutputStream& stream) const;

    virtual void update() = 0;

    /**
        Notifies that a structure or unit was built.
        \param  pObject  the object that was built
    */
    virtual void onObjectWasBuilt(const ObjectBase* pObject) { }

    /**
        Notifies that a structure of type itemID was destroyed at the specified location.
        \param  itemID      the structure that was destroyed
        \param  location    the location the building was located
    */
    virtual void onDecrementStructures(int itemID, const Coord& location) { }

    /**
        Notifies that a unit of type itemID was destroyed.
        \param  itemID      the unit that was destroyed
    */
    virtual void onDecrementUnits(int itemID) { }

    /**
        Notifies that an enemy unit of type itemID was destroyed.
        \param  itemID      the enemy unit that was destroyed
    */
    virtual void onIncrementUnitKills(int itemID) { }
    /**
        An object was hit by something or damaged somehow else.
        \param  pObject     the object that was damaged
        \param  damage      the damage taken
        \param  damagerID   the shooter of the bullet, rocket, etc. if known; NONE_ID otherwise
    */
    virtual void onDamage(const ObjectBase* pObject, int damage, Uint32 damagerID) { }

    const House* getHouse() const { return pHouse; }
    Uint8 getPlayerID() const { return playerID; }

    std::string getPlayername() const { return playername; }
    void setPlayername(const std::string& playername) { this->playername = playername; }

    std::string getPlayerclass() const { return playerclass; }
    void setPlayerclass(const std::string& playerclass) { this->playerclass = playerclass; }

protected:

    /**
        Logs a debug message
        \param  fmt the format string of the debug message
        \param ...
    */
    void logDebug(PRINTF_FORMAT_STRING const char* fmt, ...) const PRINTF_VARARG_FUNC(2);

    /**
        Logs a warning message
        \param  fmt the format string of the debug message
        \param ...
        */
    void logWarn(PRINTF_FORMAT_STRING const char* fmt, ...) const PRINTF_VARARG_FUNC(2);

    Random& getRandomGen() const;
    const GameInitSettings& getGameInitSettings() const;
    Uint32 getGameCycleCount() const;
    int getTechLevel() const;

    const Map& getMap() const;
    const ObjectBase* getObject(Uint32 objectID) const;

    const RobustList<const StructureBase*>& getStructureList() const;
    const RobustList<const UnitBase*>& getUnitList() const;

    const House* getHouse(int houseID) const;

    /**
        Start repairing the structure pObject or sending the unit pObject to the rapair yard.
        \param  pObject  the structure or unit to repair
    */
    void doRepair(const ObjectBase* pObject) const;

    /**
        Set the deploy position of pStructure. Units produced in this structure will move directly to
        this position x,y after being deployed.
        \param  pStructure  the structure to set the deploy coordinates for
        \param  x           the x coordinate (in tile coordinates)
        \param  y           the y coordinate (in tile coordinates)
    */
    void doSetDeployPosition(const StructureBase* pStructure, int x, int y) const;

    /**
        Start upgrading pBuilder.
        \param  pBuilder  the structure to upgrade
        \return true if upgrading was started, false if not possible or already upgrading
    */
    bool doUpgrade(const BuilderBase* pBuilder) const;

    /**
        Start production of the specified item in pBuilder.
        \param  pBuilder        the structure to build in
        \param  itemID          the item to produce
    */
    void doProduceItem(const BuilderBase* pBuilder, Uint32 itemID) const;

    /**
        Cancel production of the specified item in pBuilder.
        \param  pBuilder        the structure to build in
        \param  itemID          the item to cancel
    */
    void doCancelItem(const BuilderBase* pBuilder, Uint32 itemID) const;

    /**
        Sets the currently in pBuilder produced item on hold or continues production.
        \param  pBuilder        the structure to stop/resume production in
        \param  bOnHold         true = hold production; false = resume production
    */
    void doSetOnHold(const BuilderBase* pBuilder, bool bOnHold) const;

    /**
        Limit the build speed of the specified builder.
        \param  pBuilder        the structure to limit the build speed of
        \param  buildSpeedLimit the new build speed limit if that builder; must be between 0.0 and 1.0
    */
    void doSetBuildSpeedLimit(const BuilderBase* pBuilder, FixPoint buildSpeedLimit) const;

    /**
        Start building a random item in pBuilder. If pBuilder is a Starport a random item is
        added to the order list
        \param  pBuilder  the structure to build in
    */
    void doBuildRandom(const BuilderBase* pBuilder) const;

    /**
        Send order and wait for delivery to pStarport.
        \param  pStarport  the Starport to send order for
    */
    void doPlaceOrder(const StarPort* pStarport) const;

    /**
        Places the currently in pConstYard produced structure on the map at x,y.
        \param  pConstYard  the construction yard that has produced the structure
        \param  x           the x coordinate (in tile coordinates)
        \param  y           the y coordinate (in tile coordinates)
        \return true if placement was successful, false otherwise
    */
    bool doPlaceStructure(const ConstructionYard* pConstYard, int x, int y) const;

    /**
        Activate the special palace weapon Fremen or Saboteur. For the Deathhand see doLaunchDeathhand.
        \param  pPalace the palace to activate the special weapon of
    */
    void doSpecialWeapon(const Palace* pPalace) const;

    /**
        Launch the deathhand missile an target position x,y.
        \param  pPalace the palace to activate the special weapon of
        \param  xpos    x coordinate (in tile coordinates)
        \param  ypos    y coordinate (in tile coordinates)
    */
    void doLaunchDeathhand(const Palace* pPalace, int x, int y) const;

    /**
        Attacks with turret pTurret the object pObject.
        \param  pTurret         the turret to attack with
        \param  pTargetObject   the object to attack
    */
    void doAttackObject(const TurretBase* pTurret, const ObjectBase* pTargetObject) const;


    /**
        Moves the unit pUnit to x,y.
        \param  pUnit   the unit to move
        \param  x    the x position on the map
        \param  y    the y position on the map
        \param  bForced true, if the unit should ignore everything else
    */
    void doMove2Pos(const UnitBase* pUnit, int x, int y, bool bForced) const;

    /**
        Moves the unit pUnit to x,y.
        \param  pUnit           the unit to move
        \param  pTargetObject   the object to move to
    */
    void doMove2Object(const UnitBase* pUnit, const ObjectBase* pTargetObject) const;

    /**
        Orders the unit pUnit to attack position x,y.
        \param  pUnit   the unit that shall attack
        \param  x       the x position on the map
        \param  y       the y position on the map
        \param  bForced true, if the unit should ignore everything else
    */
    void doAttackPos(const UnitBase* pUnit, int x, int y, bool bForced) const;

    /**
        Attacks with unit pUnit the object pObject.
        \param  pUnit           the unit to attack with
        \param  pTargetObject   the object to attack
        \param  bForced         true, if the unit should ignore everything else
    */
    void doAttackObject(const UnitBase* pUnit, const ObjectBase* pTargetObject, bool bForced) const;

    /**
        Change the attack mode of pUnit to attackMode.
        \param  pUnit           the unit to change attack mode of
        \param  attackMode      the new attack mode
    */
    void doSetAttackMode(const UnitBase* pUnit, ATTACKMODE attackMode) const;

    /**
        Start the devastation sequence of a devastator
        \param  pDevastator the devastator to devastate
    */
    void doStartDevastate(const Devastator* pDevastator) const;

    /**
        Orders pHarvester to return to a refinery
        \param  pHarvester the harvester to return
    */
    void doReturn(const Harvester* pHarvester) const;

    /**
        The infantry unit pInfantry shall capture pTargetStructure
        \param  pInfantry           the unit to capture with
        \param  pTargetStructure    the object to attack
    */
    void doCaptureStructure(const InfantryBase* pInfantry, const StructureBase* pTargetStructure) const;

    /**
       Deploy MCV pMCV. If deploying was successful this unit does not exist anymore.
       \param  pMCV the MCV to deploy
       \return true, if deploying was successful, false otherwise.
    */
    bool doDeploy(const MCV* pMCV) const;


    /**
        Request a carryall to take unit to location
        This isn't in the original game but will make it fun
    */

    bool doRequestCarryallDrop(const GroundUnit* pGroundUnit) const;


private:
    friend class House;

    House* pHouse;
    Uint8 playerID;
    std::string playername;
    std::string playerclass;
};

#endif // PLAYER_H
