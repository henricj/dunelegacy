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
    typedef enum {
        EASY = 0,
        MEDIUM = 2,
        HARD = 3,
        BRUTAL = 4,
        SKIRMISH = 5,
        CAMPAIGN = 6
    } gameType;


	void init();
	~QuantBot();
	virtual void save(OutputStream& stream) const;

    virtual void update();

    virtual void onIncrementStructures(int itemID);
    virtual void onDecrementStructures(int itemID, const Coord& location);
    virtual void onDamage(const ObjectBase* pObject, int damage, Uint32 damagerID);

	static Player* create(House* associatedHouse, std::string playername, Uint32 difficulty, Uint32 gameMode) {
        return new QuantBot(associatedHouse, playername, difficulty, gameMode);
	}

	static Player* load(InputStream& stream, House* associatedHouse) {
        return new QuantBot(stream, associatedHouse);
	}

private:
	QuantBot(House* associatedHouse, std::string playername, Uint32 difficulty, Uint32 gameMode);
	QuantBot(InputStream& stream, House* associatedHouse);


	Uint32	difficulty;     ///< difficulty level
	Uint32	gameMode;     ///< game mode (skirmish or campaign)
    Sint32  buildTimer;     ///< When to build the next structure/unit
    Sint32  attackTimer;     ///< When to build the next structure/unit

    int initialItemCount[ItemID_LastID];
    int itemCount[ItemID_LastID];
    bool initialCountComplete;
    Coord squadCentreLocation;
    Coord squadRallyLocation;

    void scrambleUnitsAndDefend(const ObjectBase* pIntruder);


    Coord findMcvPlaceLocation(const MCV* pMCV);
	Coord findPlaceLocation(Uint32 itemID);
	Coord findSquadCentre(int houseID);
	Coord findBaseCentre(int houseID);
	Coord findSquadRallyLocation();

	std::list<Coord> placeLocations;    ///< Where to place structures

	void checkAllUnits();
	void retreatAllUnits();
	void build();
	void attack();

};

#endif //QuantBot_H
