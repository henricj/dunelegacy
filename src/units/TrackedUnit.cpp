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

#include <units/TrackedUnit.h>

#include <globals.h>

#include <House.h>
#include <Map.h>


TrackedUnit::TrackedUnit(House* newOwner) : GroundUnit(newOwner)
{
    TrackedUnit::init();
}

TrackedUnit::TrackedUnit(InputStream& stream) : GroundUnit(stream)
{
    TrackedUnit::init();
}

void TrackedUnit::init()
{
    tracked = true;
}

TrackedUnit::~TrackedUnit()
{
}

void TrackedUnit::save(OutputStream& stream) const
{
	GroundUnit::save(stream);
}

void TrackedUnit::checkPos()
{
	GroundUnit::checkPos();

	if(active && justStoppedMoving)
		currentGameMap->getTile(location.x, location.y)->squash();
}

bool TrackedUnit::canPass(int xPos, int yPos) const
{
	bool passable = false;
	if(currentGameMap->tileExists(xPos, yPos)) {
		Tile* pTile = currentGameMap->getTile(xPos, yPos);

		if(!pTile->isMountain()) {
			if (!pTile->hasAGroundObject())
				passable = true;
			else if (!pTile->hasANonInfantryGroundObject() && (pTile->getInfantryTeam() != getOwner()->getTeam()))
				passable = true;
		}
	}

	return passable;
}
