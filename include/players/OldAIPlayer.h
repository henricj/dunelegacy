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

#ifndef OLDAIPLAYER_H
#define OLDAIPLAYER_H

#include <players/Player.h>

#include <DataTypes.h>

class OldAIPlayer : public Player
{
public:
    typedef enum {
        EASY = 0,
        MEDIUM = 1,
        HARD = 2
    } enum_difficulty;

	void init();
	~OldAIPlayer();
	virtual void save(OutputStream& stream) const;

    virtual void update();

    virtual void onIncrementStructures(int itemID);
    virtual void onDecrementStructures(int itemID, const Coord& location);
    virtual void onDamage(const ObjectBase* pObject, int damage, Uint32 damagerID);

	void setAttackTimer(int newAttackTimer);
	bool isAttacking() const { return (attackTimer < 0); }
	inline int getAttackTimer() const { return attackTimer; }

	static Player* create(House* associatedHouse, std::string playername, Uint8 difficulty) {
        return new OldAIPlayer(associatedHouse, playername, difficulty);
	}

	static Player* load(InputStream& stream, House* associatedHouse) {
        return new OldAIPlayer(stream, associatedHouse);
	}

private:
	OldAIPlayer(House* associatedHouse, std::string playername, Uint8 difficulty);
	OldAIPlayer(InputStream& stream, House* associatedHouse);

    void scrambleUnitsAndDefend(Uint32 intruderID);

	Coord findPlaceLocation(Uint32 itemID);

	Uint8	difficulty;     ///< difficulty level
	Sint32  attackTimer;    ///< When to attack?
    Sint32  buildTimer;     ///< When to build the next structure/unit

	std::list<Coord> placeLocations;    ///< Where to place structures
};

#endif //OLDAIPLAYER_H
