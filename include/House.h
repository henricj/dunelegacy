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

#include "ObjectManager.h"
#include <AITeamInfo.h>
#include <Choam.h>
#include <DataTypes.h>
#include <data.h>
#include <misc/InputStream.h>
#include <misc/OutputStream.h>

#include <memory>

// forward declarations
class UnitBase;
class StructureBase;
class ObjectBase;
class HumanPlayer;

class House final {
    House(const GameContext& context);

public:
    House(const GameContext& context, HOUSETYPE newHouse, int newCredits, int maxUnits, uint8_t teamID = 0,
          int quota = 0);
    House(const GameContext& context, InputStream& stream);
    virtual ~House();

    House(const House&) = delete;
    House(House&&)      = delete;
    House& operator=(const House&) = delete;
    House& operator=(House&&) = delete;

    virtual void save(OutputStream& stream) const;

    void addPlayer(std::unique_ptr<Player> newPlayer);

    [[nodiscard]] HOUSETYPE getHouseID() const noexcept { return houseID; }
    [[nodiscard]] int getTeamID() const noexcept { return teamID; }

    [[nodiscard]] bool isAI() const noexcept { return ai; }
    [[nodiscard]] bool isAlive() const noexcept {
        return (teamID == 0)
            || !(((numStructures - numItem[Structure_Wall]) <= 0)
                 && (((numUnits - numItem[Unit_Carryall] - numItem[Unit_Harvester] - numItem[Unit_Frigate]
                       - numItem[Unit_Sandworm])
                      <= 0)));
    }

    [[nodiscard]] bool hasCarryalls() const noexcept { return (numItem[Unit_Carryall] > 0); }
    [[nodiscard]] bool hasBarracks() const noexcept { return (numItem[Structure_Barracks] > 0); }
    [[nodiscard]] bool hasIX() const noexcept { return (numItem[Structure_IX] > 0); }
    [[nodiscard]] bool hasLightFactory() const noexcept { return (numItem[Structure_LightFactory] > 0); }
    [[nodiscard]] bool hasHeavyFactory() const noexcept { return (numItem[Structure_HeavyFactory] > 0); }
    [[nodiscard]] bool hasRefinery() const noexcept { return (numItem[Structure_Refinery] > 0); }
    [[nodiscard]] bool hasRepairYard() const noexcept { return (numItem[Structure_RepairYard] > 0); }
    [[nodiscard]] bool hasStarPort() const noexcept { return (numItem[Structure_StarPort] > 0); }
    [[nodiscard]] bool hasWindTrap() const noexcept { return (numItem[Structure_WindTrap] > 0); }
    [[nodiscard]] bool hasSandworm() const noexcept { return (numItem[Unit_Sandworm] > 0); }
    [[nodiscard]] bool hasRadar() const noexcept { return (numItem[Structure_Radar] > 0); }

    [[nodiscard]] bool hasRadarOn() const noexcept { return (hasRadar() && hasPower()); }
    [[nodiscard]] bool hasPower() const noexcept { return (producedPower >= powerRequirement); }

    [[nodiscard]] int getNumStructures() const noexcept { return numStructures; }
    [[nodiscard]] int getNumUnits() const noexcept { return numUnits; }
    [[nodiscard]] int getNumItems(ItemID_enum itemID) const {
        return (isStructure(itemID) || isUnit(itemID)) ? numItem[itemID] : 0;
    }

    [[nodiscard]] int getCapacity() const noexcept { return capacity; }

    [[nodiscard]] int getProducedPower() const noexcept { return producedPower; }
    void setProducedPower(int newPower);
    [[nodiscard]] int getPowerRequirement() const noexcept { return powerRequirement; }

    [[nodiscard]] int getBuiltValue() const noexcept { return unitBuiltValue + structureBuiltValue; }
    [[nodiscard]] int getUnitBuiltValue() const noexcept { return unitBuiltValue; }
    [[nodiscard]] int getMilitaryValue() const noexcept { return militaryValue; }
    [[nodiscard]] int getKillValue() const noexcept { return killValue; }
    [[nodiscard]] int getLossValue() const noexcept { return lossValue; }
    [[nodiscard]] int getStructureBuiltValue() const noexcept { return structureBuiltValue; }
    [[nodiscard]] int getNumBuiltUnits() const noexcept { return numBuiltUnits; }
    [[nodiscard]] int getNumBuiltStructures() const noexcept { return numBuiltStructures; }
    [[nodiscard]] int getDestroyedValue() const noexcept { return destroyedValue; }
    [[nodiscard]] int getNumDestroyedUnits() const noexcept { return numDestroyedUnits; }
    [[nodiscard]] int getNumDestroyedStructures() const noexcept { return numDestroyedStructures; }
    [[nodiscard]] int getNumBuiltItems(ItemID_enum itemID) const noexcept { return numItemBuilt[itemID]; }
    [[nodiscard]] int getNumKilledItems(ItemID_enum itemID) const noexcept { return numItemKills[itemID]; }
    [[nodiscard]] int getNumLostItems(ItemID_enum itemID) const noexcept { return numItemLosses[itemID]; }
    [[nodiscard]] int32_t getNumItemDamageInflicted(ItemID_enum itemID) const noexcept {
        return numItemDamageInflicted[itemID];
    }
    [[nodiscard]] FixPoint getHarvestedSpice() const noexcept { return harvestedSpice; }
    [[nodiscard]] int getNumVisibleEnemyUnits() const noexcept { return numVisibleEnemyUnits; }
    [[nodiscard]] int getNumVisibleFriendlyUnits() const noexcept { return numVisibleFriendlyUnits; }

    [[nodiscard]] int getQuota() const noexcept { return quota; }
    [[nodiscard]] int getMaxUnits() const noexcept { return maxUnits; }

    void informContactWithEnemy() { bHadContactWithEnemy = true; }
    [[nodiscard]] bool hadContactWithEnemy() const { return bHadContactWithEnemy; }
    void informDirectContactWithEnemy() { bHadDirectContactWithEnemy = true; }
    [[nodiscard]] bool hadDirectContactWithEnemy() const { return bHadDirectContactWithEnemy; }

    void informVisibleEnemyUnit() { numVisibleEnemyUnits++; }

    void informVisibleFriendlyUnit() { numVisibleFriendlyUnits++; }

    /**
        This function checks if the limit for ground units is already reached. Infantry units are only counted as 1/3.
        \return true, if the limit is already reached, false if building further ground units is allowed
    */
    [[nodiscard]] bool isGroundUnitLimitReached() const {
        const int numGroundUnit = numUnits - numItem[Unit_Soldier] - numItem[Unit_Trooper] - numItem[Unit_Carryall]
                                - numItem[Unit_Ornithopter];
        return (numGroundUnit + (numItem[Unit_Soldier] + 2) / 3 + (numItem[Unit_Trooper] + 2) / 3 >= maxUnits);
    }

    /**
        This function checks if the limit for infantry units is already reached. Infantry units are only counted as 1/3.
        \return true, if the limit is already reached, false if building further infantry units is allowed
    */
    [[nodiscard]] bool isInfantryUnitLimitReached() const {
        const auto numGroundUnit = numUnits - numItem[Unit_Soldier] - numItem[Unit_Trooper] - numItem[Unit_Carryall]
                                 - numItem[Unit_Ornithopter];
        return (numGroundUnit + numItem[Unit_Soldier] / 3 + numItem[Unit_Trooper] / 3 >= maxUnits);
    }

    /**
        This function checks if the limit for air units is already reached.
        \return true, if the limit is already reached, false if building further air units is allowed
    */
    [[nodiscard]] bool isAirUnitLimitReached() const {
        return (numItem[Unit_Carryall] + numItem[Unit_Ornithopter] >= 11 * std::max(maxUnits, 25) / 25);
    }

    Choam& getChoam() { return choam; }
    [[nodiscard]] const Choam& getChoam() const { return choam; }

    [[nodiscard]] FixPoint getStartingCredits() const { return startingCredits; }
    [[nodiscard]] FixPoint getStoredCredits() const { return storedCredits; }
    [[nodiscard]] int getCredits() const { return lround(storedCredits + startingCredits); }
    void addCredits(FixPoint newCredits, bool wasRefined = false);
    void returnCredits(FixPoint newCredits);
    FixPoint takeCredits(FixPoint amount);

    void printStat() const;

    void updateBuildLists();

    void update();

    void incrementUnits(ItemID_enum itemID);
    void decrementUnits(ItemID_enum itemID);
    void incrementStructures(ItemID_enum itemID);
    void decrementStructures(ItemID_enum itemID, const Coord& location);

    /**
        An object was hit by something or damaged somehow else.
        \param  pObject     the object that was damaged
        \param  damage      the damage taken
        \param  damagerID   the shooter of the bullet, rocket, etc. if known; NONE_ID otherwise
    */
    void noteDamageLocation(ObjectBase* pObject, int damage, uint32_t damagerID);

    void informWasBuilt(ObjectBase* pObject);
    void informHasKilled(ItemID_enum itemID);
    void informHasDamaged(ItemID_enum itemID, uint32_t damage);

    void lose(bool bSilent = false) const;
    void win();

    void freeHarvester(int xPos, int yPos);
    void freeHarvester(const Coord& coord) { freeHarvester(coord.x, coord.y); }
    StructureBase* placeStructure(uint32_t builderID, ItemID_enum itemID, int xPos, int yPos, bool byScenario = false,
                                  bool bForcePlacing = false);

    template<typename UnitType>
    UnitType* createUnit(bool byScenario = false) {
        static_assert(std::is_base_of<UnitBase, UnitType>::value, "UnitType not derived from UnitBase");

        return context.objectManager.createObjectFromType<UnitType>(ObjectInitializer {context.game, this, byScenario});
    }

    UnitBase* createUnit(ItemID_enum itemID, bool byScenario = false);
    StructureBase* createStructure(ItemID_enum itemID, bool byScenario = false);

    UnitBase* placeUnit(ItemID_enum itemID, int xPos, int yPos, bool byScenario = false);

    [[nodiscard]] Coord getCenterOfMainBase() const;

    [[nodiscard]] Coord getStrongestUnitPosition() const;

    [[nodiscard]] const std::vector<AITeamInfo>& getAITeams() const { return aiteams; }
    void addAITeam(AITeamBehavior aiTeamBehavior, AITeamType aiTeamType, int minUnits, int maxUnits) {
        aiteams.emplace_back(houseID, aiTeamBehavior, aiTeamType, minUnits, maxUnits);
    }

    [[nodiscard]] const std::list<std::unique_ptr<Player>>& getPlayerList() const { return players; }

protected:
    void decrementHarvesters();

    std::list<std::unique_ptr<Player>> players; ///< List of associated players that control this house

    bool ai; ///< Is this an ai player?

    HOUSETYPE houseID; ///< The house number
    uint8_t teamID;    ///< The team number

    int numStructures;       ///< How many structures does this player have?
    int numUnits;            ///< How many units does this player have?
    int numItem[Num_ItemID]; ///< This array contains the number of structures/units of a certain type this player has
    int numItemBuilt[Num_ItemID];  /// Number of items built by player
    int numItemKills[Num_ItemID];  /// Number of items killed by player
    int numItemLosses[Num_ItemID]; /// Number of items lost by player
    int32_t
        numItemDamageInflicted[Num_ItemID]; /// Amount of damage inflicted by a specific unit type owned by the player

    int capacity;         ///< Total spice capacity
    int producedPower;    ///< Power produced by this player
    int powerRequirement; ///< How much power does this player use?

    FixPoint storedCredits;   ///< current number of credits that are stored in refineries/silos
    FixPoint startingCredits; ///< number of starting credits this player still has
    int oldCredits;           ///< amount of credits in the last game cycle (used for playing the credits tick sound)

    int maxUnits; ///< maximum number of units this house is allowed to build
    int quota;    ///< number of credits to win

    Choam choam; ///< the things that are deliverable at the starport

    std::vector<AITeamInfo> aiteams; ///< the ai teams that were loaded from the map

    int powerUsageTimer; ///< every N ticks you have to pay for your power usage

    bool bHadContactWithEnemy; ///< did this house already have contact with an enemy (= tiles with enemy units were
                               ///< explored by this house or allied houses)
    bool bHadDirectContactWithEnemy; ///< did this house already have direct contact with an enemy (= tiles with enemy
                                     ///< units were explored by this house)

    int numVisibleEnemyUnits;    ///< the number of enemy units visible; will be reset to 0 each cycle
    int numVisibleFriendlyUnits; ///< the number of visible units from the same team; will be reset to 0 each cycle

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

    const GameContext context;

private:
    void init();

    void registerUnit(std::unique_ptr<UnitBase> unit);
};

#endif // HOUSE_H
