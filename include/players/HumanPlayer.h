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

#ifndef HUMANPLAYER_H
#define HUMANPLAYER_H

#include <players/Player.h>

#include <Definitions.h>

#include <SDL.h>

// forward declarations
class UnitBase;
class StructureBase;
class ObjectBase;

class HumanPlayer : public Player
{
public:
	void init();
	virtual ~HumanPlayer();
	virtual void save(OutputStream& stream) const;

    virtual void update();

	/**
        Returns one of the 9 saved units lists
        \param  groupListIndex   which list should be returned
        \return the n-th list.
	*/
	inline std::set<Uint32>& getGroupList(int groupListIndex) { return selectedLists[groupListIndex]; };

	/**
        Sets one of the 9 saved units lists
        \param  groupListIndex     which list should be set
        \param  newGroupList        the new list to set
	*/
	void setGroupList(int groupListIndex, const std::set<Uint32>& newGroupList);

	static Player* create(House* associatedHouse, std::string playername) {
        return new HumanPlayer(associatedHouse, playername);
	}

	static Player* load(InputStream& stream, House* associatedHouse) {
        return new HumanPlayer(stream, associatedHouse);
	}

public:
    Uint32 nextExpectedCommandsCycle;                       ///< The next cycle we expect commands for (using for network games)

    std::set<Uint32> selectedLists[NUMSELECTEDLISTS];       ///< Sets of all the different groups on key 1 to 9

private:
	HumanPlayer(House* associatedHouse, std::string playername);
	HumanPlayer(InputStream& stream, House* associatedHouse);
};



#endif // HUMANPLAYER_H
