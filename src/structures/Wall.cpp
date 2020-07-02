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

Wall::Wall(ItemID_enum itemID, Uint32 objectID, const ObjectInitializer& initializer) : StructureBase(itemID, objectID, initializer) {
    Wall::init();

    setHealth(getMaxHealth());

    bWallDestroyedUp = false;
    bWallDestroyedRight = false;
    bWallDestroyedDown = false;
    bWallDestroyedLeft = false;

    setWallTile(Wall_LeftRight);
}

Wall::Wall(ItemID_enum itemID, Uint32 objectID, const ObjectStreamInitializer& initializer) : StructureBase(itemID, objectID, initializer) {
    Wall::init();

    auto& stream = initializer.Stream;

    stream.readBools(&bWallDestroyedUp, &bWallDestroyedRight, &bWallDestroyedDown, &bWallDestroyedLeft);

    setWallTile(stream.readSint32());
}

void Wall::init() {
    assert(itemID == Structure_Wall);
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
    auto& objectManager = currentGame->getObjectManager();

    // fix wall to the north
    if(currentGameMap->tileExists(location.x, location.y-1)) {
        ObjectBase* obj = currentGameMap->getTile(location.x, location.y - 1)->getGroundObject(objectManager);
        if((obj != nullptr) && (obj->getItemID() == Structure_Wall)) {
            Wall* pWall = static_cast<Wall*>(obj);
            pWall->bWallDestroyedDown = true;
            pWall->fixWall();
        }
    }

    // fix wall to the south
    if(currentGameMap->tileExists(location.x, location.y+1)) {
        ObjectBase* obj = currentGameMap->getTile(location.x, location.y + 1)->getGroundObject(objectManager);
        if((obj != nullptr) && (obj->getItemID() == Structure_Wall)) {
            Wall* pWall = static_cast<Wall*>(obj);
            pWall->bWallDestroyedUp = true;
            pWall->fixWall();
        }
    }

    // fix wall to the west
    if(currentGameMap->tileExists(location.x-1, location.y)) {
        ObjectBase* obj = currentGameMap->getTile(location.x - 1, location.y)->getGroundObject(objectManager);
        if((obj != nullptr) && (obj->getItemID() == Structure_Wall)) {
            Wall* pWall = static_cast<Wall*>(obj);
            pWall->bWallDestroyedRight = true;
            pWall->fixWall();
        }
    }

    // fix wall to the east
    if(currentGameMap->tileExists(location.x+1, location.y)) {
        ObjectBase* obj = currentGameMap->getTile(location.x + 1, location.y)->getGroundObject(objectManager);
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

    const auto* const map = currentGameMap;

    Wall* pWall = nullptr;

    // fix wall to the north
    pWall = map->getGroundObject<Wall>(location.x, location.y - 1);
    if(pWall) {
        pWall->bWallDestroyedDown = false;
        pWall->fixWall();
    }

    // fix wall to the south
    pWall = map->getGroundObject<Wall>(location.x, location.y + 1);
    if(pWall) {
        pWall->bWallDestroyedUp = false;
        pWall->fixWall();
    }

    // fix wall to the west
    pWall = map->getGroundObject<Wall>(location.x - 1, location.y);
    if(pWall) {
        pWall->bWallDestroyedRight = false;
        pWall->fixWall();
    }

    // fix wall to the east
    pWall = map->getGroundObject<Wall>(location.x + 1, location.y);
    if(pWall) {
        pWall->bWallDestroyedLeft = false;
        pWall->fixWall();
    }
}

/**
    Fixes this wall. The choosen wall tile is based on the 4 surounding tiles and previously destroyed walls.
*/
void Wall::fixWall() {

    int i = location.x;
    int j = location.y;

    int maketile = Wall_LeftRight;

    const auto* const map = currentGameMap;

    // Walls
    bool up    = map->hasAGroundObject<Wall>(i    , j - 1) || bWallDestroyedUp;
    bool right = map->hasAGroundObject<Wall>(i + 1, j    ) || bWallDestroyedRight;
    bool down  = map->hasAGroundObject<Wall>(i    , j + 1) || bWallDestroyedDown;
    bool left  = map->hasAGroundObject<Wall>(i - 1, j    ) || bWallDestroyedLeft;

    // calculate destroyed tile index
    int destroyedTileIndex = 0;
    if(left)    destroyedTileIndex = (destroyedTileIndex << 1) | ((int) bWallDestroyedLeft);
    if(down)    destroyedTileIndex = (destroyedTileIndex << 1) | ((int) bWallDestroyedDown);
    if(right)   destroyedTileIndex = (destroyedTileIndex << 1) | ((int) bWallDestroyedRight);
    if(up)      destroyedTileIndex = (destroyedTileIndex << 1) | ((int) bWallDestroyedUp);

    // Now perform the test
    if ((left) && (right) && (up) && (down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_Full : 59 + destroyedTileIndex; //solid wall
    } else if ((!left) && (right) && (up) && (down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpDownRight : 31 + destroyedTileIndex; //missing left edge
    } else if ((left) && (!right)&& (up) && (down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpDownLeft : 45 + destroyedTileIndex; //missing right edge
    } else if ((left) && (right) && (!up) && (down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_DownLeftRight : 38 + destroyedTileIndex; //missing top edge
    } else if ((left) && (right) && (up) && (!down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpLeftRight : 52 + destroyedTileIndex; //missing bottom edge
    } else if ((!left) && (right) && (!up) && (down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_DownRight : 22 + destroyedTileIndex; //missing top left edge
    } else if ((left) && (!right) && (up) && (!down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpLeft : 28 + destroyedTileIndex; //missing bottom right edge
    } else if ((left) && (!right) && (!up) && (down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_DownLeft : 25 + destroyedTileIndex; //missing top right edge
    } else if ((!left) && (right) && (up) && (!down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpRight : 19 + destroyedTileIndex; //missing bottom left edge
    } else if ((left) && (!right) && (!up) && (!down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_LeftRight : 14 + destroyedTileIndex; //missing above, right and below
    } else if ((!left) && (right) && (!up) && (!down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_LeftRight : 13 + destroyedTileIndex; //missing above, left and below
    } else if ((!left) && (!right) && (up) && (!down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpDown : 16 + destroyedTileIndex; //only up
    } else if ((!left) && (!right) && (!up) && (down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpDown : 17 + destroyedTileIndex; //only down
    } else if ((left) && (right) && (!up) && (!down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_LeftRight : 13 + destroyedTileIndex; //missing above and below
    } else if ((!left) && (!right) && (up) && (down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpDown : 16 + destroyedTileIndex; //missing left and right
    } else if ((!left) && (!right) && (!up) && (!down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_Standalone : 12 + destroyedTileIndex; //missing left and right
    }

    setWallTile(maketile);
}
