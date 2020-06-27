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

class QuantBot : public Player
{
public:
    enum class Difficulty {
        Easy = 0,
        Medium = 1,
        Hard = 2,
        Brutal = 3,
        Defend = 4
    };

    enum class GameMode {
        Custom = 4,
        Campaign = 5
    };

    QuantBot(House* associatedHouse, const std::string& playername, Difficulty difficulty);
    QuantBot(InputStream& stream, House* associatedHouse);
    void init();
    ~QuantBot() override;
    void save(OutputStream& stream) const override;

    void update() override;

    void onObjectWasBuilt(const ObjectBase* pObject) override;
    void onDecrementStructures(int itemID, const Coord& location) override;
    void onDecrementUnits(int itemID) override;
    void onIncrementUnitKills(int itemID) override;
    void onDamage(const ObjectBase* pObject, int damage, Uint32 damagerID) override;

private:

    Difficulty difficulty;  ///< difficulty level
    GameMode  gameMode;     ///< game mode (custom or campaign)
    Sint32  buildTimer;     ///< When to build the next structure/unit
    Sint32  attackTimer;    ///< When to build the next structure/unit
    Sint32  retreatTimer;   ///< When you last retreated>

    int initialItemCount[Num_ItemID];
    int initialMilitaryValue = 0;
    int militaryValueLimit = 0;
    int harvesterLimit = 4;
    bool campaignAIAttackFlag = false;
    Coord squadRallyLocation = Coord::Invalid();
    Coord squadRetreatLocation = Coord::Invalid();

    void scrambleUnitsAndDefend(const ObjectBase* pIntruder, int numUnits = std::numeric_limits<int>::max());


    static Coord findMcvPlaceLocation(const MCV* pMCV);
    static Coord findPlaceLocation(Uint32 itemID);
    static Coord findSquadCenter(HOUSETYPE houseID);
    static Coord findBaseCentre(HOUSETYPE houseID);
    static Coord findSquadRallyLocation();
    static Coord findSquadRetreatLocation();

    std::list<Coord> placeLocations;    ///< Where to place structures

    static void checkAllUnits();
    void retreatAllUnits();
    void build(int militaryValue);
    void attack(int militaryValue);

};

#endif //QuantBot_H
