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

#ifndef SmartBot_H
#define SmartBot_H

#include <players/Player.h>

#include <DataTypes.h>

class SmartBot : public Player
{
public:
    enum class Difficulty {
        Normal = 0,
        Defense = 1,
        Hard = 2
    };

    SmartBot(House* associatedHouse, const std::string& playername, Difficulty difficulty);
    SmartBot(InputStream& stream, House* associatedHouse);
    void init();
    ~SmartBot();
    void save(OutputStream& stream) const override;

    void update() override;

    void onObjectWasBuilt(const ObjectBase* pObject) override;
    void onDecrementStructures(int itemID, const Coord& location) override;
    void onDamage(const ObjectBase* pObject, int damage, Uint32 damagerID) override;

private:
    void scrambleUnitsAndDefend(const ObjectBase* pIntruder);

    Coord findPlaceLocation(Uint32 itemID);

    int getNumAdjacentStructureTiles(Coord pos, int structureSizeX, int structureSizeY);

    void checkAllUnits();
    void build();
    void attack();

    bool isAllowedToArm() const;

    int getMaxHarvester() const;

    Difficulty difficulty;  ///< difficulty level
    Sint32  attackTimer;    ///< When to attack?
    Sint32  buildTimer;     ///< When to build the next structure/unit
    int harvesterLimit = 4; ///< maximum number of harvesters

    std::list<Coord> placeLocations;    ///< Where to place structures

    bool focusEconomy();
    bool focusMilitary();
    bool focusFactory();
    bool focusBase();
};

#endif //SmartBot_H
