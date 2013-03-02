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

#ifndef WALL_H
#define WALL_H

#include <structures/StructureBase.h>

class Wall : public StructureBase
{
public:
    typedef enum {
        Wall_Standalone     = 1,
        Wall_LeftRight      = 2,
        Wall_UpRight        = 3,
        Wall_UpDown         = 4,
        Wall_DownRight      = 5,
        Wall_UpDownRight    = 6,
        Wall_UpLeft         = 7,
        Wall_UpLeftRight    = 8,
        Wall_DownLeft       = 9,
        Wall_UpDownLeft     = 10,
        Wall_DownLeftRight  = 11,
        Wall_Full           = 12
    } WALLTYPE;

	Wall(House* newOwner);
	Wall(InputStream& stream);
	void init();
	virtual ~Wall();

    virtual void save(OutputStream& stream) const;

	void destroy();

    /**
        Can this structure be captured by infantry units?
        \return true, if this structure can be captured, false otherwise
	*/
	virtual bool canBeCaptured() const { return false; };

    inline void setLocation(const Coord& location) { setLocation(location.x, location.y); }
	virtual void setLocation(int xPos, int yPos);

private:
	inline void setWallTile(int newTile) {
	    curAnimFrame = firstAnimFrame = lastAnimFrame = newTile;
	}

    void fixWall();

    bool bWallDestroyedUp;
    bool bWallDestroyedRight;
    bool bWallDestroyedDown;
    bool bWallDestroyedLeft;

};

#endif //WALL_H
