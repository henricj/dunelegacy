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

#ifndef HARVESTER_H
#define HARVESTER_H

#include <units/TrackedUnit.h>

class Harvester : public TrackedUnit
{
public:

	Harvester(House* newOwner);
	Harvester(InputStream& stream);
	void init();
	virtual ~Harvester();

	virtual void save(OutputStream& stream) const;

	void blitToScreen();

	void checkPos();
	virtual void deploy(const Coord& newLocation);
	void destroy();
	virtual void drawSelectionBox();
	void handleDamage(int damage, Uint32 damagerID, House* damagerOwner);

	void handleReturnClick();

    /**
        Order this harvester to return to a refinery.
    */
	void doReturn();

	void move();
	void setAmountOfSpice(FixPoint newSpice);
	void setReturned();

	void setDestination(int newX, int newY);
    inline void setDestination(const Coord& location) { setDestination(location.x, location.y); }

	void setTarget(const ObjectBase* newTarget);

	bool canAttack(const ObjectBase* object) const;

	FixPoint extractSpice(FixPoint extractionSpeed);

	inline FixPoint getAmountOfSpice() const { return spice; }
	inline bool isReturning() const { return returningToRefinery; }
	bool isHarvesting() const;

private:

    virtual void setSpeeds();

    // harvester state
	bool	 harvestingMode;         ///< currently harvesting
    bool     returningToRefinery;    ///< currently on the way back to the refinery
	FixPoint spice;                  ///< loaded spice
	Uint32   spiceCheckCounter;      ///< Check for available spice on map to harvest
};

#endif // HARVESTER_H
