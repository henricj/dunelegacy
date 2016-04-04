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

#ifndef MAP_H
#define MAP_H

#include <Tile.h>
#include <misc/InputStream.h>
#include <misc/OutputStream.h>

#include <stdio.h>

class Map
{
public:
    /**
        Creates a map of size xSize x ySize. The map is initialized with all tiles of type Terrain_Sand.
    */
	Map(int xSize, int ySize);
	~Map();

	void load(InputStream& stream);
	void save(OutputStream& stream) const;

	void createSandRegions();
	void damage(Uint32 damagerID, House* damagerOwner, const Coord& realPos, Uint32 bulletID, FixPoint damage, int damageRadius, bool air);
	Coord getMapPos(int angle, const Coord& source) const;
	void removeObjectFromMap(Uint32 objectID);
	void spiceRemoved(const Coord& coord);
	void selectObjects(int houseID, int x1, int y1, int x2, int y2, int realX, int realY, bool objectARGMode);

	void viewMap(int playerTeam, const Coord& location, int maxViewRange);
	void viewMap(int playerTeam, int x, int y, int maxViewRange) {
        viewMap(playerTeam, Coord(x,y), maxViewRange);
    }

	bool findSpice(Coord& destination, const Coord& origin) const;
	bool okayToPlaceStructure(int x, int y, int buildingSizeX, int buildingSizeY, bool tilesRequired, const House* pHouse, bool bIgnoreUnits = false) const;
	bool isAStructureGap(int x, int y, int buildingSizeX, int buildingSizeY) const; // Allows AI to check to see if a gap exists between the current structure
	bool isWithinBuildRange(int x, int y, const House* pHouse) const;
	int getPosAngle(const Coord& source, const Coord& pos) const;
	Coord findClosestEdgePoint(const Coord& origin, const Coord& buildingSize) const;
	Coord findDeploySpot(UnitBase* pUnit, const Coord origin, const Coord gatherPoint = Coord::Invalid(), const Coord buildingSize = Coord(0,0)) const;//building size is num squares

	void createSpiceField(Coord location, int radius, bool centerIsThickSpice = false);

    inline Sint32 getSizeX() const {
        return sizeX;
    }

    inline Sint32 getSizeY() const {
        return sizeY;
    }

	inline bool tileExists(int xPos, int yPos) const {
		return ((xPos >= 0) && (xPos < sizeX) && (yPos >= 0) && (yPos < sizeY));
	}

	inline bool tileExists(const Coord& pos) const {
		return tileExists(pos.x, pos.y);
	}

	inline const Tile* getTile(int xPos, int yPos) const {
		if(tileExists(xPos,yPos)) {
			return &tiles[xPos + yPos*sizeX];
		} else {
			fprintf(stderr,"getTile(): tile (%d, %d) does not exist\n",xPos,yPos);
			fflush(stderr);
			return nullptr;
		}
	}

	inline const Tile* getTile(const Coord& location) const {
		return getTile(location.x, location.y);
	}

	inline Tile* getTile(int xPos, int yPos) {
		if(tileExists(xPos,yPos)) {
			return &tiles[xPos + yPos*sizeX];
		} else {
			fprintf(stderr,"getTile(): tile (%d, %d) does not exist\n",xPos,yPos);
			fflush(stderr);
			return nullptr;
		}
	}

	inline Tile* getTile(const Coord& location) {
		return getTile(location.x, location.y);
	}

private:
	Sint32	sizeX;                          ///< number of tiles this map is wide (read only)
	Sint32  sizeY;                          ///< number of tiles this map is high (read only)
	Tile*   tiles;                          ///< the 2d-array containing all the tiles of the map
	ObjectBase* lastSinglySelectedObject;   ///< The last selected object. If selected again all units of the same type are selected
};


#endif // MAP_H
