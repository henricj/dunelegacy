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

#include <structures/Wall.h>

#include <Map.h>
#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <House.h>

Wall::Wall(House* newOwner) : StructureBase(newOwner) {
    Wall::init();

    setHealth(getMaxHealth());

    bWallDestroyedUp = false;
    bWallDestroyedRight = false;
    bWallDestroyedDown = false;
    bWallDestroyedLeft = false;

    setWallTile(Wall_LeftRight);
}

Wall::Wall(InputStream& stream) : StructureBase(stream) {
    Wall::init();

    stream.readBools(&bWallDestroyedUp, &bWallDestroyedRight, &bWallDestroyedDown, &bWallDestroyedLeft);

    setWallTile(stream.readSint32());
}

void Wall::init() {
    itemID = Structure_Wall;
    owner->incrementStructures(itemID);

    structureSize.x = 1;
    structureSize.y = 1;

    graphicID = ObjPic_Wall;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    numImagesX = 25;
    numImagesY = 3;
}

Wall::~Wall() = default;

void Wall::save(OutputStream& stream) const {
    StructureBase::save(stream);

    stream.writeBools(bWallDestroyedUp, bWallDestroyedRight, bWallDestroyedDown, bWallDestroyedLeft);

    // save current wall tile
    stream.writeSint32(curAnimFrame);
}

void Wall::destroy() {
    // fix wall to the north
    if(currentGameMap->tileExists(location.x, location.y-1) == true) {
        ObjectBase* obj = currentGameMap->getTile(location.x, location.y-1)->getGroundObject();
        if((obj != nullptr) && (obj->getItemID() == Structure_Wall)) {
            Wall* pWall = static_cast<Wall*>(obj);
            pWall->bWallDestroyedDown = true;
            pWall->fixWall();
        }
    }

    // fix wall to the south
    if(currentGameMap->tileExists(location.x, location.y+1) == true) {
        ObjectBase* obj = currentGameMap->getTile(location.x, location.y+1)->getGroundObject();
        if((obj != nullptr) && (obj->getItemID() == Structure_Wall)) {
            Wall* pWall = static_cast<Wall*>(obj);
            pWall->bWallDestroyedUp = true;
            pWall->fixWall();
        }
    }

    // fix wall to the west
    if(currentGameMap->tileExists(location.x-1, location.y) == true) {
        ObjectBase* obj = currentGameMap->getTile(location.x-1, location.y)->getGroundObject();
        if((obj != nullptr) && (obj->getItemID() == Structure_Wall)) {
            Wall* pWall = static_cast<Wall*>(obj);
            pWall->bWallDestroyedRight = true;
            pWall->fixWall();
        }
    }

    // fix wall to the east
    if(currentGameMap->tileExists(location.x+1, location.y) == true) {
        ObjectBase* obj = currentGameMap->getTile(location.x+1, location.y)->getGroundObject();
        if((obj != nullptr) && (obj->getItemID() == Structure_Wall)) {
            Wall* pWall = static_cast<Wall*>(obj);
            pWall->bWallDestroyedLeft = true;
            pWall->fixWall();
        }
    }


    StructureBase::destroy();
}

/**
    Sets the place for this wall and fixes the wall and their surounding walls.
    \param  xPos    the x position of this wall
    \param  yPos    the y position of this wall
*/
void Wall::setLocation(int xPos, int yPos) {
    StructureBase::setLocation(xPos, yPos);

    // fix this wall
    fixWall();

    // fix wall to the north
    if(currentGameMap->tileExists(location.x, location.y-1) == true) {
        ObjectBase* obj = currentGameMap->getTile(location.x, location.y-1)->getGroundObject();
        if((obj != nullptr) && (obj->getItemID() == Structure_Wall)) {
            Wall* pWall = static_cast<Wall*>(obj);
            pWall->bWallDestroyedDown = false;
            pWall->fixWall();
        }
    }

    // fix wall to the south
    if(currentGameMap->tileExists(location.x, location.y+1) == true) {
        ObjectBase* obj = currentGameMap->getTile(location.x, location.y+1)->getGroundObject();
        if((obj != nullptr) && (obj->getItemID() == Structure_Wall)) {
            Wall* pWall = static_cast<Wall*>(obj);
            pWall->bWallDestroyedUp = false;
            pWall->fixWall();
        }
    }

    // fix wall to the west
    if(currentGameMap->tileExists(location.x-1, location.y) == true) {
        ObjectBase* obj = currentGameMap->getTile(location.x-1, location.y)->getGroundObject();
        if((obj != nullptr) && (obj->getItemID() == Structure_Wall)) {
            Wall* pWall = static_cast<Wall*>(obj);
            pWall->bWallDestroyedRight = false;
            pWall->fixWall();
        }
    }

    // fix wall to the east
    if(currentGameMap->tileExists(location.x+1, location.y) == true) {
        ObjectBase* obj = currentGameMap->getTile(location.x+1, location.y)->getGroundObject();
        if((obj != nullptr) && (obj->getItemID() == Structure_Wall)) {
            Wall* pWall = static_cast<Wall*>(obj);
            pWall->bWallDestroyedLeft = false;
            pWall->fixWall();
        }
    }
}

/**
    Fixes this wall. The choosen wall tile is based on the 4 surounding tiles and previously destroyed walls.
*/
void Wall::fixWall() {

    int i = location.x;
    int j = location.y;

    int maketile = Wall_LeftRight;

    // Walls
    bool up = (currentGameMap->tileExists(i, j-1) && (currentGameMap->getTile(i,j-1)->hasAGroundObject()
                && (currentGameMap->getTile(i,j-1)->getGroundObject()->getItemID() == Structure_Wall))) || bWallDestroyedUp;

    bool right = (currentGameMap->tileExists(i+1, j) && (currentGameMap->getTile(i+1,j)->hasAGroundObject()
                && (currentGameMap->getTile(i+1,j)->getGroundObject()->getItemID() == Structure_Wall))) || bWallDestroyedRight;

    bool down = (currentGameMap->tileExists(i, j+1) && (currentGameMap->getTile(i,j+1)->hasAGroundObject()
                && (currentGameMap->getTile(i,j+1)->getGroundObject()->getItemID() == Structure_Wall))) || bWallDestroyedDown;

    bool left = (currentGameMap->tileExists(i-1, j) && (currentGameMap->getTile(i-1,j)->hasAGroundObject()
                && (currentGameMap->getTile(i-1,j)->getGroundObject()->getItemID() == Structure_Wall))) || bWallDestroyedLeft;

    // calculate destroyed tile index
    int destroyedTileIndex = 0;
    if(left == true)    destroyedTileIndex = (destroyedTileIndex << 1) | ((int) bWallDestroyedLeft);
    if(down == true)    destroyedTileIndex = (destroyedTileIndex << 1) | ((int) bWallDestroyedDown);
    if(right == true)   destroyedTileIndex = (destroyedTileIndex << 1) | ((int) bWallDestroyedRight);
    if(up == true)      destroyedTileIndex = (destroyedTileIndex << 1) | ((int) bWallDestroyedUp);

    // Now perform the test
    if ((left == true) && (right == true) && (up == true) && (down == true)) {
        maketile = (destroyedTileIndex == 0) ? Wall_Full : 59 + destroyedTileIndex; //solid wall
    } else if ((left == false) && (right == true) && (up == true) && (down == true)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpDownRight : 31 + destroyedTileIndex; //missing left edge
    } else if ((left == true) && (right == false)&& (up == true) && (down == true)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpDownLeft : 45 + destroyedTileIndex; //missing right edge
    } else if ((left == true) && (right == true) && (up == false) && (down == true)) {
        maketile = (destroyedTileIndex == 0) ? Wall_DownLeftRight : 38 + destroyedTileIndex; //missing top edge
    } else if ((left == true) && (right == true) && (up == true) && (down == false)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpLeftRight : 52 + destroyedTileIndex; //missing bottom edge
    } else if ((left == false) && (right == true) && (up == false) && (down == true)) {
        maketile = (destroyedTileIndex == 0) ? Wall_DownRight : 22 + destroyedTileIndex; //missing top left edge
    } else if ((left == true) && (right == false) && (up == true) && (down == false)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpLeft : 28 + destroyedTileIndex; //missing bottom right edge
    } else if ((left == true) && (right == false) && (up == false) && (down == true)) {
        maketile = (destroyedTileIndex == 0) ? Wall_DownLeft : 25 + destroyedTileIndex; //missing top right edge
    } else if ((left == false) && (right == true) && (up == true) && (down == false)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpRight : 19 + destroyedTileIndex; //missing bottom left edge
    } else if ((left == true) && (right == false) && (up == false) && (down == false)) {
        maketile = (destroyedTileIndex == 0) ? Wall_LeftRight : 14 + destroyedTileIndex; //missing above, right and below
    } else if ((left == false) && (right == true) && (up == false) && (down == false)) {
        maketile = (destroyedTileIndex == 0) ? Wall_LeftRight : 13 + destroyedTileIndex; //missing above, left and below
    } else if ((left == false) && (right == false) && (up == true) && (down == false)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpDown : 16 + destroyedTileIndex; //only up
    } else if ((left == false) && (right == false) && (up == false) && (down == true)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpDown : 17 + destroyedTileIndex; //only down
    } else if ((left == true) && (right == true) && (up == false) && (down == false)) {
        maketile = (destroyedTileIndex == 0) ? Wall_LeftRight : 13 + destroyedTileIndex; //missing above and below
    } else if ((left == false) && (right == false) && (up == true) && (down == true)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpDown : 16 + destroyedTileIndex; //missing left and right
    } else if ((left == false) && (right == false) && (up == false) && (down == false)) {
        maketile = (destroyedTileIndex == 0) ? Wall_Standalone : 12 + destroyedTileIndex; //missing left and right
    }

    setWallTile(maketile);
}
