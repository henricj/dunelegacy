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

#ifndef QuantBot_H
#define QuantBot_H

#include <players/Player.h>
#include <units/MCV.h>

#include <DataTypes.h>

class QuantBot final : public Player {
public:
    enum class Difficulty {
        Easy   = 0,
        Medium = 1,
        Hard   = 2,
        Brutal = 3,
        Defend = 4
    };

    enum class GameMode {
        Custom   = 4,
        Campaign = 5
    };

    QuantBot(const GameContext& context, House* associatedHouse, const std::string& playername, const Random& random, Difficulty difficulty);
    QuantBot(const GameContext& context, InputStream& stream, House* associatedHouse);
    void init();
    ~QuantBot() override;
    void save(OutputStream& stream) const override;

    void update() override;

    void onObjectWasBuilt(const ObjectBase* pObject) override;
    void onDecrementStructures(ItemID_enum itemID, const Coord& location) override;
    void onDecrementUnits(ItemID_enum itemID) override;
    void onIncrementUnitKills(ItemID_enum itemID) override;
    void onDamage(const ObjectBase* pObject, int damage, uint32_t damagerID) override;

private:
    Difficulty difficulty; ///< difficulty level
    GameMode gameMode;     ///< game mode (custom or campaign)
    int32_t buildTimer;    ///< When to build the next structure/unit
    int32_t attackTimer;   ///< When to build the next structure/unit
    int32_t retreatTimer;  ///< When you last retreated>

    int initialItemCount[Num_ItemID];
    int initialMilitaryValue   = 0;
    int militaryValueLimit     = 0;
    int harvesterLimit         = 4;
    bool campaignAIAttackFlag  = false;
    Coord squadRallyLocation   = Coord::Invalid();
    Coord squadRetreatLocation = Coord::Invalid();

    void scrambleUnitsAndDefend(const ObjectBase* pIntruder, int numUnits = std::numeric_limits<int>::max());

    Coord findMcvPlaceLocation(const MCV* pMCV);
    Coord findPlaceLocation(ItemID_enum itemID);
    Coord findSquadCenter(HOUSETYPE houseID);
    Coord findBaseCentre(HOUSETYPE houseID);
    Coord findSquadRallyLocation();
    Coord findSquadRetreatLocation();

    std::list<Coord> placeLocations; ///< Where to place structures

    void checkAllUnits();
    void retreatAllUnits();
    void build(int militaryValue);
    void attack(int militaryValue);
};

#endif // QuantBot_H
