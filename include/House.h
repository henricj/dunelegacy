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
class Player;
class HumanPlayer;

class House final {
    House(const GameContext& context);

public:
    House(const GameContext& context, HOUSETYPE newHouse, int newCredits, int maxUnits, uint8_t teamID = 0,
          int quota = 0);
    House(const GameContext& context, InputStream& stream);
    ~House();

    House(const House&)            = delete;
    House(House&&)                 = delete;
    House& operator=(const House&) = delete;
    House& operator=(House&&)      = delete;

    void save(OutputStream& stream) const;

    void addPlayer(std::unique_ptr<Player> newPlayer);

    [[nodiscard]] HOUSETYPE getHouseID() const noexcept { return houseID_; }
    [[nodiscard]] int getTeamID() const noexcept { return teamID_; }

    [[nodiscard]] bool isAI() const noexcept { return ai_; }
    [[nodiscard]] bool isAlive() const noexcept;

    [[nodiscard]] bool hasCarryalls() const noexcept { return (numItem_[Unit_Carryall] > 0); }
    [[nodiscard]] bool hasBarracks() const noexcept { return (numItem_[Structure_Barracks] > 0); }
    [[nodiscard]] bool hasIX() const noexcept { return (numItem_[Structure_IX] > 0); }
    [[nodiscard]] bool hasLightFactory() const noexcept { return (numItem_[Structure_LightFactory] > 0); }
    [[nodiscard]] bool hasHeavyFactory() const noexcept { return (numItem_[Structure_HeavyFactory] > 0); }
    [[nodiscard]] bool hasRefinery() const noexcept { return (numItem_[Structure_Refinery] > 0); }
    [[nodiscard]] bool hasRepairYard() const noexcept { return (numItem_[Structure_RepairYard] > 0); }
    [[nodiscard]] bool hasStarPort() const noexcept { return (numItem_[Structure_StarPort] > 0); }
    [[nodiscard]] bool hasWindTrap() const noexcept { return (numItem_[Structure_WindTrap] > 0); }
    [[nodiscard]] bool hasSandworm() const noexcept { return (numItem_[Unit_Sandworm] > 0); }
    [[nodiscard]] bool hasRadar() const noexcept { return (numItem_[Structure_Radar] > 0); }

    [[nodiscard]] bool hasRadarOn() const noexcept { return (hasRadar() && hasPower()); }
    [[nodiscard]] bool hasPower() const noexcept { return (producedPower_ >= powerRequirement_); }

    [[nodiscard]] int getNumStructures() const noexcept { return numStructures_; }
    [[nodiscard]] int getNumUnits() const noexcept { return numUnits_; }
    [[nodiscard]] int getNumItems(ItemID_enum itemID) const {
        return (isStructure(itemID) || isUnit(itemID)) ? numItem_[itemID] : 0;
    }

    [[nodiscard]] int getCapacity() const noexcept { return capacity_; }

    [[nodiscard]] int getProducedPower() const noexcept { return producedPower_; }
    void setProducedPower(int newPower);
    [[nodiscard]] int getPowerRequirement() const noexcept { return powerRequirement_; }

    [[nodiscard]] int getBuiltValue() const noexcept { return unitBuiltValue_ + structureBuiltValue_; }
    [[nodiscard]] int getUnitBuiltValue() const noexcept { return unitBuiltValue_; }
    [[nodiscard]] int getMilitaryValue() const noexcept { return militaryValue; }
    [[nodiscard]] int getKillValue() const noexcept { return killValue_; }
    [[nodiscard]] int getLossValue() const noexcept { return lossValue_; }
    [[nodiscard]] int getStructureBuiltValue() const noexcept { return structureBuiltValue_; }
    [[nodiscard]] int getNumBuiltUnits() const noexcept { return numBuiltUnits_; }
    [[nodiscard]] int getNumBuiltStructures() const noexcept { return numBuiltStructures_; }
    [[nodiscard]] int getDestroyedValue() const noexcept { return destroyedValue_; }
    [[nodiscard]] int getNumDestroyedUnits() const noexcept { return numDestroyedUnits_; }
    [[nodiscard]] int getNumDestroyedStructures() const noexcept { return numDestroyedStructures_; }
    [[nodiscard]] int getNumBuiltItems(ItemID_enum itemID) const noexcept { return numItemBuilt_[itemID]; }
    [[nodiscard]] int getNumKilledItems(ItemID_enum itemID) const noexcept { return numItemKills_[itemID]; }
    [[nodiscard]] int getNumLostItems(ItemID_enum itemID) const noexcept { return numItemLosses_[itemID]; }
    [[nodiscard]] int32_t getNumItemDamageInflicted(ItemID_enum itemID) const noexcept {
        return numItemDamageInflicted_[itemID];
    }
    [[nodiscard]] FixPoint getHarvestedSpice() const noexcept { return harvestedSpice_; }
    [[nodiscard]] int getNumVisibleEnemyUnits() const noexcept { return numVisibleEnemyUnits_; }
    [[nodiscard]] int getNumVisibleFriendlyUnits() const noexcept { return numVisibleFriendlyUnits_; }

    [[nodiscard]] int getQuota() const noexcept { return quota_; }
    [[nodiscard]] int getMaxUnits() const noexcept { return maxUnits_; }

    void informContactWithEnemy() { bHadContactWithEnemy_ = true; }
    [[nodiscard]] bool hadContactWithEnemy() const { return bHadContactWithEnemy_; }
    void informDirectContactWithEnemy() { bHadDirectContactWithEnemy_ = true; }
    [[nodiscard]] bool hadDirectContactWithEnemy() const { return bHadDirectContactWithEnemy_; }

    void informVisibleEnemyUnit() { numVisibleEnemyUnits_++; }

    void informVisibleFriendlyUnit() { numVisibleFriendlyUnits_++; }

    /**
        This function checks if the limit for ground units is already reached. Infantry units are only counted as 1/3.
        \return true, if the limit is already reached, false if building further ground units is allowed
    */
    [[nodiscard]] bool isGroundUnitLimitReached() const;

    /**
        This function checks if the limit for infantry units is already reached. Infantry units are only counted as 1/3.
        \return true, if the limit is already reached, false if building further infantry units is allowed
    */
    [[nodiscard]] bool isInfantryUnitLimitReached() const;

    /**
        This function checks if the limit for air units is already reached.
        \return true, if the limit is already reached, false if building further air units is allowed
    */
    [[nodiscard]] bool isAirUnitLimitReached() const;

    Choam& getChoam() { return choam_; }
    [[nodiscard]] const Choam& getChoam() const { return choam_; }

    [[nodiscard]] FixPoint getStartingCredits() const { return startingCredits_; }
    [[nodiscard]] FixPoint getStoredCredits() const { return storedCredits_; }
    [[nodiscard]] int getCredits() const { return lround(storedCredits_ + startingCredits_); }
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
        static_assert(std::is_base_of_v<UnitBase, UnitType>, "UnitType not derived from UnitBase");

        return context_.objectManager.createObjectFromType<UnitType>(
            ObjectInitializer{context_.game, this, byScenario});
    }

    UnitBase* createUnit(ItemID_enum itemID, bool byScenario = false);
    StructureBase* createStructure(ItemID_enum itemID, bool byScenario = false);

    UnitBase* placeUnit(ItemID_enum itemID, int xPos, int yPos, bool byScenario = false);

    [[nodiscard]] Coord getCenterOfMainBase() const;

    [[nodiscard]] Coord getStrongestUnitPosition() const;

    [[nodiscard]] const std::vector<AITeamInfo>& getAITeams() const { return aiteams_; }
    void addAITeam(AITeamBehavior aiTeamBehavior, AITeamType aiTeamType, int minUnits, int maxUnits) {
        aiteams_.emplace_back(houseID_, aiTeamBehavior, aiTeamType, minUnits, maxUnits);
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Player>>& getPlayerList() const { return players_; }

protected:
    void decrementHarvesters();

    std::vector<std::unique_ptr<Player>> players_; ///< List of associated players that control this house

    bool ai_ = true; ///< Is this an ai player?

    HOUSETYPE houseID_{}; ///< The house number
    uint8_t teamID_{};    ///< The team number

    int numStructures_{}; ///< How many structures does this player have?
    int numUnits_{};      ///< How many units does this player have?
    std::array<int, Num_ItemID>
        numItem_{}; ///< This array contains the number of structures/units of a certain type this player has
    std::array<int, Num_ItemID> numItemBuilt_{};  /// Number of items built by player
    std::array<int, Num_ItemID> numItemKills_{};  /// Number of items killed by player
    std::array<int, Num_ItemID> numItemLosses_{}; /// Number of items lost by player
    std::array<int32_t, Num_ItemID>
        numItemDamageInflicted_{}; /// Amount of damage inflicted by a specific unit type owned by the player

    int capacity_{};         ///< Total spice capacity
    int producedPower_{};    ///< Power produced by this player
    int powerRequirement_{}; ///< How much power does this player use?

    FixPoint storedCredits_{};   ///< current number of credits that are stored in refineries/silos
    FixPoint startingCredits_{}; ///< number of starting credits this player still has
    int oldCredits_{};           ///< amount of credits in the last game cycle (used for playing the credits tick sound)

    int maxUnits_{}; ///< maximum number of units this house is allowed to build
    int quota_{};    ///< number of credits to win

    Choam choam_; ///< the things that are deliverable at the starport

    std::vector<AITeamInfo> aiteams_; ///< the ai teams that were loaded from the map

    int powerUsageTimer_{}; ///< every N ticks you have to pay for your power usage

    bool bHadContactWithEnemy_{}; ///< did this house already have contact with an enemy (= tiles with enemy units were
                                  ///< explored by this house or allied houses)
    bool bHadDirectContactWithEnemy_{}; ///< did this house already have direct contact with an enemy (= tiles with
                                        ///< enemy units were explored by this house)

    int numVisibleEnemyUnits_{};    ///< the number of enemy units visible; will be reset to 0 each cycle
    int numVisibleFriendlyUnits_{}; ///< the number of visible units from the same team; will be reset to 0 each cycle

    // statistic
    int unitBuiltValue_{};
    int structureBuiltValue_{};
    int militaryValue{};
    int killValue_{};
    int lossValue_{};
    int numBuiltUnits_{};
    int numBuiltStructures_{};
    int destroyedValue_{};
    int numDestroyedUnits_{};
    int numDestroyedStructures_{};
    FixPoint harvestedSpice_{};

    const GameContext context_;
};

#endif // HOUSE_H
