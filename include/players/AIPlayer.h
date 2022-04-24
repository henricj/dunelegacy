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

#ifndef AIPLAYER_H
#define AIPLAYER_H

#include <players/Player.h>

#include <DataTypes.h>

#include <deque>

class AIPlayer final : public Player {
public:
    enum class Difficulty { Easy = 0, Medium = 1, Hard = 2 };

    AIPlayer(const GameContext& context, House* associatedHouse, const std::string& playername, const Random& random,
             Difficulty difficulty);
    AIPlayer(const GameContext& context, InputStream& stream, House* associatedHouse);
    ~AIPlayer() override;
    void save(OutputStream& stream) const override;

    void update() override;

    void onObjectWasBuilt(const ObjectBase* pObject) override;
    void onDecrementStructures(ItemID_enum itemID, const Coord& location) override;
    void onDamage(const ObjectBase* pObject, int damage, uint32_t damagerID) override;

private:
    void scrambleUnitsAndDefend(const ObjectBase* pIntruder);

    Coord findPlaceLocation(ItemID_enum itemID);

    int getNumAdjacentStructureTiles(Coord pos, int structureSizeX, int structureSizeY);

    void checkAllUnits();
    void build();
    void attack();

    void handle_sandworm(const UnitBase* sandworm);

    [[nodiscard]] bool isAllowedToArm() const;

    [[nodiscard]] int getMaxHarvester() const;

    Difficulty difficulty; ///< difficulty level
    int32_t attackTimer;   ///< When to attack?
    int32_t buildTimer;    ///< When to build the next structure/unit

    std::deque<Coord> placeLocations; ///< Where to place structures
};

#endif // AIPLAYER_H
