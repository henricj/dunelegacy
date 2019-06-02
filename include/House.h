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

#ifndef HOUSE_H
#define HOUSE_H

#include <misc/InputStream.h>
#include <misc/OutputStream.h>
#include <Definitions.h>
#include <DataTypes.h>
#include <AITeamInfo.h>
#include <data.h>
#include <Choam.h>

#include <players/Player.h>

#include <memory>

// forward declarations
class UnitBase;
class StructureBase;
class ObjectBase;
class HumanPlayer;

class House
{
public:
    House(int newHouse, int newCredits, int maxUnits, Uint8 teamID = 0, int quota = 0);
    explicit House(InputStream& stream);
    void init();
    virtual ~House();
    virtual void save(OutputStream& stream) const;

    void addPlayer(std::unique_ptr<Player> newPlayer);

    inline int getHouseID() const { return houseID; }
    inline int getTeamID() const { return teamID; }

    inline bool isAI() const { return ai; }
    inline bool isAlive() const { return (teamID == 0) || !(((numStructures - numItem[Structure_Wall]) <= 0) && (((numUnits - numItem[Unit_Carryall] - numItem[Unit_Harvester] - numItem[Unit_Frigate] - numItem[Unit_Sandworm]) <= 0))); }

    inline bool hasCarryalls() const { return (numItem[Unit_Carryall] > 0); }
    inline bool hasBarracks() const { return (numItem[Structure_Barracks] > 0); }
    inline bool hasIX() const { return (numItem[Structure_IX] > 0); }
    inline bool hasLightFactory() const { return (numItem[Structure_LightFactory] > 0); }
    inline bool hasHeavyFactory() const { return (numItem[Structure_HeavyFactory] > 0); }
    inline bool hasRefinery() const { return (numItem[Structure_Refinery] > 0); }
    inline bool hasRepairYard() const { return (numItem[Structure_RepairYard] > 0); }
    inline bool hasStarPort() const { return (numItem[Structure_StarPort] > 0); }
    inline bool hasWindTrap() const { return (numItem[Structure_WindTrap] > 0); }
    inline bool hasSandworm() const { return (numItem[Unit_Sandworm] > 0); }
    inline bool hasRadar() const { return (numItem[Structure_Radar] > 0); }

    inline bool hasRadarOn() const { return (hasRadar() && hasPower()); }
    inline bool hasPower() const { return (producedPower >= powerRequirement); }

    inline int getNumStructures() const { return numStructures; };
    inline int getNumUnits() const { return numUnits; };
    inline int getNumItems(int itemID) const { return (isStructure(itemID) || isUnit(itemID)) ? numItem[itemID] : 0; };

    inline int getCapacity() const { return capacity; }

    inline int getProducedPower() const { return producedPower; }
    void setProducedPower(int newPower);
    inline int getPowerRequirement() const { return powerRequirement; }

    inline int getBuiltValue() const { return unitBuiltValue + structureBuiltValue; }
    inline int getUnitBuiltValue() const { return unitBuiltValue; }
    inline int getMilitaryValue() const { return militaryValue; }
    inline int getKillValue() const { return killValue; }
    inline int getLossValue() const { return lossValue; }
    inline int getStructureBuiltValue() const { return structureBuiltValue; }
    inline int getNumBuiltUnits() const { return numBuiltUnits; }
    inline int getNumBuiltStructures() const { return numBuiltStructures; }
    inline int getDestroyedValue() const { return destroyedValue; }
    inline int getNumDestroyedUnits() const { return numDestroyedUnits; }
    inline int getNumDestroyedStructures() const { return numDestroyedStructures; }
    inline int getNumBuiltItems(int itemID) const { return numItemBuilt[itemID]; }
    inline int getNumKilledItems(int itemID) const { return numItemKills[itemID]; }
    inline int getNumLostItems(int itemID) const { return numItemLosses[itemID]; }
    inline Sint32 getNumItemDamageInflicted(int itemID) const { return numItemDamageInflicted[itemID]; }
    inline FixPoint getHarvestedSpice() const { return harvestedSpice; }
    inline int getNumVisibleEnemyUnits() const { return numVisibleEnemyUnits; }
    inline int getNumVisibleFriendlyUnits() const { return numVisibleFriendlyUnits; }

    inline int getQuota() const { return quota; };
    inline int getMaxUnits() const { return maxUnits; };

    inline void informContactWithEnemy() { bHadContactWithEnemy = true; };
    inline bool hadContactWithEnemy() const { return bHadContactWithEnemy; };
    inline void informDirectContactWithEnemy() { bHadDirectContactWithEnemy = true; };
    inline bool hadDirectContactWithEnemy() const { return bHadDirectContactWithEnemy; };

    inline void informVisibleEnemyUnit() {
        numVisibleEnemyUnits++;
    }

    inline void informVisibleFriendlyUnit() {
        numVisibleFriendlyUnits++;
    }

    /**
        This function checks if the limit for ground units is already reached. Infantry units are only counted as 1/3.
        \return true, if the limit is already reached, false if building further ground units is allowed
    */
    inline bool isGroundUnitLimitReached() const {
        int numGroundUnit = numUnits - numItem[Unit_Soldier] - numItem[Unit_Trooper] - numItem[Unit_Carryall] - numItem[Unit_Ornithopter];
        return (numGroundUnit + (numItem[Unit_Soldier]+2)/3 + (numItem[Unit_Trooper]+2)/3  >= maxUnits);
    };

    /**
        This function checks if the limit for infantry units is already reached. Infantry units are only counted as 1/3.
        \return true, if the limit is already reached, false if building further infantry units is allowed
    */
    inline bool isInfantryUnitLimitReached() const {
        int numGroundUnit = numUnits - numItem[Unit_Soldier] - numItem[Unit_Trooper] - numItem[Unit_Carryall] - numItem[Unit_Ornithopter];
        return (numGroundUnit + numItem[Unit_Soldier]/3 + numItem[Unit_Trooper]/3  >= maxUnits);
    };

    /**
        This function checks if the limit for air units is already reached.
        \return true, if the limit is already reached, false if building further air units is allowed
    */
    inline bool isAirUnitLimitReached() const {
        return (numItem[Unit_Carryall] + numItem[Unit_Ornithopter] >= 11*std::max(maxUnits,25)/25);
    }

    inline Choam& getChoam() { return choam; };
    inline const Choam& getChoam() const { return choam; };


    inline FixPoint getStartingCredits() const { return startingCredits; }
    inline FixPoint getStoredCredits() const { return storedCredits; }
    inline int getCredits() const { return lround(storedCredits+startingCredits); }
    void addCredits(FixPoint newCredits, bool wasRefined = false);
    void returnCredits(FixPoint newCredits);
    FixPoint takeCredits(FixPoint amount);

    void printStat() const;

    void updateBuildLists();

    void update();

    void incrementUnits(int itemID);
    void decrementUnits(int itemID);
    void incrementStructures(int itemID);
    void decrementStructures(int itemID, const Coord& location);

    /**
        An object was hit by something or damaged somehow else.
        \param  pObject     the object that was damaged
        \param  damage      the damage taken
        \param  damagerID   the shooter of the bullet, rocket, etc. if known; NONE_ID otherwise
    */
    void noteDamageLocation(ObjectBase* pObject, int damage, Uint32 damagerID);

    void informWasBuilt(ObjectBase* pObject);
    void informHasKilled(Uint32 itemID);
    void informHasDamaged(Uint32 itemID, Uint32 damage);

    void lose(bool bSilent = false);
    void win();

    void freeHarvester(int xPos, int yPos);
    void freeHarvester(const Coord& coord) { freeHarvester(coord.x, coord.y); };
    StructureBase* placeStructure(Uint32 builderID, int itemID, int xPos, int yPos, bool byScenario = false, bool bForcePlacing = false);
    UnitBase* createUnit(int itemID, bool byScenario = false);
    UnitBase* placeUnit(int itemID, int xPos, int yPos, bool byScenario = false);

    Coord getCenterOfMainBase() const;

    Coord getStrongestUnitPosition() const;

    const std::vector<AITeamInfo> getAITeams() const { return aiteams; };
    void addAITeam(AITeamBehavior aiTeamBehavior, AITeamType aiTeamType, int minUnits, int maxUnits) {
        aiteams.emplace_back(houseID, aiTeamBehavior, aiTeamType, minUnits, maxUnits);
    }

    const std::list<std::unique_ptr<Player> >& getPlayerList() const { return players; };

protected:
    void decrementHarvesters();

    std::list<std::unique_ptr<Player> > players;        ///< List of associated players that control this house

    bool    ai;             ///< Is this an ai player?

    Uint8   houseID;        ///< The house number
    Uint8   teamID;         ///< The team number

    int numStructures;          ///< How many structures does this player have?
    int numUnits;               ///< How many units does this player have?
    int numItem[Num_ItemID];    ///< This array contains the number of structures/units of a certain type this player has
    int numItemBuilt[Num_ItemID];  /// Number of items built by player
    int numItemKills[Num_ItemID]; /// Number of items killed by player
    int numItemLosses [Num_ItemID]; /// Number of items lost by player
    Sint32 numItemDamageInflicted[Num_ItemID]; /// Amount of damage inflicted by a specific unit type owned by the player

    int capacity;             ///< Total spice capacity
    int producedPower;        ///< Power prodoced by this player
    int powerRequirement;     ///< How much power does this player use?

    FixPoint storedCredits;   ///< current number of credits that are stored in refineries/silos
    FixPoint startingCredits; ///< number of starting credits this player still has
    int oldCredits;           ///< amount of credits in the last game cycle (used for playing the credits tick sound)

    int maxUnits;             ///< maximum number of units this house is allowed to build
    int quota;                ///< number of credits to win

    Choam   choam;            ///< the things that are deliverable at the starport

    std::vector<AITeamInfo> aiteams;    ///< the ai teams that were loaded from the map

    int powerUsageTimer;      ///< every N ticks you have to pay for your power usage

    bool bHadContactWithEnemy;      ///< did this house already have contact with an enemy (= tiles with enemy units were explored by this house or allied houses)
    bool bHadDirectContactWithEnemy;///< did this house already have direct contact with an enemy (= tiles with enemy units were explored by this house)

    int numVisibleEnemyUnits;   ///< the number of enemy units visible; will be reset to 0 each cycle
    int numVisibleFriendlyUnits;///< the number of visible units from the same team; will be reset to 0 each cycle

    // statistic
    int unitBuiltValue;
    int structureBuiltValue;
    int militaryValue;
    int killValue;
    int lossValue;
    int numBuiltUnits;
    int numBuiltStructures;
    int destroyedValue;
    int numDestroyedUnits;
    int numDestroyedStructures;
    FixPoint harvestedSpice;
};

#endif // HOUSE_H
