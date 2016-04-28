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
    typedef enum {
        NORMAL = 0,
        DEFENSIVE = 1,
        HARD = 2
    } enum_difficulty;

    void init();
    ~SmartBot();
    virtual void save(OutputStream& stream) const;

    virtual void update();

    virtual void onIncrementStructures(int itemID);
    virtual void onDecrementStructures(int itemID, const Coord& location);
    virtual void onDamage(const ObjectBase* pObject, int damage, Uint32 damagerID);

    static Player* create(House* associatedHouse, std::string playername, Uint8 difficulty) {
        return new SmartBot(associatedHouse, playername, difficulty);
    }

    static Player* load(InputStream& stream, House* associatedHouse) {
        return new SmartBot(stream, associatedHouse);
    }

private:
    SmartBot(House* associatedHouse, std::string playername, Uint8 difficulty);
    SmartBot(InputStream& stream, House* associatedHouse);

    void scrambleUnitsAndDefend(const ObjectBase* pIntruder);

    Coord findPlaceLocation(Uint32 itemID);

    int getNumAdjacentStructureTiles(Coord pos, int structureSizeX, int structureSizeY);

    void checkAllUnits();
    void build();
    void attack();

    bool isAllowedToArm() const;

    int getMaxHarvester() const;

    Uint8   difficulty;     ///< difficulty level
    Sint32  attackTimer;    ///< When to attack?
    Sint32  buildTimer;     ///< When to build the next structure/unit
    Sint32  AIStrategy;     ///< What assult strategy to use

    std::list<Coord> placeLocations;    ///< Where to place structures

    bool focusEconomy();
    bool focusMilitary();
    bool focusFactory();
    bool focusBase();
};

#endif //SmartBot_H
