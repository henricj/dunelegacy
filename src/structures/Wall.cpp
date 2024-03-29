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

namespace {
constexpr StructureBaseConstants wall_constants{Wall::item_id, Coord{1, 1}};
}

Wall::Wall(uint32_t objectID, const ObjectInitializer& initializer)
    : StructureBase(wall_constants, objectID, initializer) {
    Wall::init();

    Wall::setHealth(getMaxHealth());

    setWallTile(Wall_LeftRight);
}

Wall::Wall(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : StructureBase(wall_constants, objectID, initializer) {
    Wall::init();

    auto& stream = initializer.stream();

    stream.readBools(&bWallDestroyedUp, &bWallDestroyedRight, &bWallDestroyedDown, &bWallDestroyedLeft);

    setWallTile(stream.readSint32());
}

void Wall::init() {
    assert(itemID_ == Structure_Wall);
    owner_->incrementStructures(itemID_);

    graphicID_  = ObjPic_Wall;
    graphic_    = dune::globals::pGFXManager->getObjPic(graphicID_, getOwner()->getHouseID());
    numImagesX_ = 25;
    numImagesY_ = 3;
}

Wall::~Wall() = default;

void Wall::save(OutputStream& stream) const {
    StructureBase::save(stream);

    stream.writeBools(bWallDestroyedUp, bWallDestroyedRight, bWallDestroyedDown, bWallDestroyedLeft);

    // save current wall tile
    stream.writeSint32(curAnimFrame);
}

void Wall::destroy(const GameContext& context) {
    auto& [game, map, objectManager] = context;

    // fix wall to the north
    if (const auto* tile = map.tryGetTile(location_.x, location_.y - 1)) {
        if (auto* pWall = dune_cast<Wall>(tile->getGroundObject(objectManager))) {
            pWall->bWallDestroyedDown = true;
            pWall->fixWall(context);
        }
    }

    // fix wall to the south
    if (const auto* tile = map.tryGetTile(location_.x, location_.y + 1)) {
        if (auto* pWall = dune_cast<Wall>(tile->getGroundObject(objectManager))) {
            pWall->bWallDestroyedUp = true;
            pWall->fixWall(context);
        }
    }

    // fix wall to the west
    if (const auto* tile = map.tryGetTile(location_.x - 1, location_.y)) {
        if (auto* pWall = dune_cast<Wall>(tile->getGroundObject(objectManager))) {
            pWall->bWallDestroyedRight = true;
            pWall->fixWall(context);
        }
    }

    // fix wall to the east
    if (const auto* tile = map.tryGetTile(location_.x + 1, location_.y)) {
        if (auto* pWall = dune_cast<Wall>(tile->getGroundObject(objectManager))) {
            pWall->bWallDestroyedLeft = true;
            pWall->fixWall(context);
        }
    }

    parent::destroy(context);
}

/**
    Sets the place for this wall and fixes the wall and their surrounding walls.
    \param  xPos    the x position of this wall
    \param  yPos    the y position of this wall
*/
void Wall::setLocation(const GameContext& context, int xPos, int yPos) {
    parent::setLocation(context, xPos, yPos);

    // fix this wall
    fixWall(context);

    const auto& map = context.map;

    Wall* pWall = nullptr;

    // fix wall to the north
    pWall = map.getGroundObject<Wall>(context, location_.x, location_.y - 1);
    if (pWall) {
        pWall->bWallDestroyedDown = false;
        pWall->fixWall(context);
    }

    // fix wall to the south
    pWall = map.getGroundObject<Wall>(context, location_.x, location_.y + 1);
    if (pWall) {
        pWall->bWallDestroyedUp = false;
        pWall->fixWall(context);
    }

    // fix wall to the west
    pWall = map.getGroundObject<Wall>(context, location_.x - 1, location_.y);
    if (pWall) {
        pWall->bWallDestroyedRight = false;
        pWall->fixWall(context);
    }

    // fix wall to the east
    pWall = map.getGroundObject<Wall>(context, location_.x + 1, location_.y);
    if (pWall) {
        pWall->bWallDestroyedLeft = false;
        pWall->fixWall(context);
    }
}

/**
    Fixes this wall. The choosen wall tile is based on the 4 surounding tiles and previously destroyed walls.
*/
void Wall::fixWall(const GameContext& context) {

    const int i = location_.x;
    const int j = location_.y;

    int maketile = Wall_LeftRight;

    const auto& map = context.map;

    // clang-format off
    // Walls
    const bool up    = map.hasAGroundObject<Wall>(context, i    , j - 1) || bWallDestroyedUp;
    const bool right = map.hasAGroundObject<Wall>(context, i + 1, j    ) || bWallDestroyedRight;
    const bool down  = map.hasAGroundObject<Wall>(context, i    , j + 1) || bWallDestroyedDown;
    const bool left  = map.hasAGroundObject<Wall>(context, i - 1, j    ) || bWallDestroyedLeft;
    // clang-format on

    // calculate destroyed tile index
    int destroyedTileIndex = 0;
    if (left)
        destroyedTileIndex = (destroyedTileIndex << 1) | static_cast<int>(bWallDestroyedLeft);
    if (down)
        destroyedTileIndex = (destroyedTileIndex << 1) | static_cast<int>(bWallDestroyedDown);
    if (right)
        destroyedTileIndex = (destroyedTileIndex << 1) | static_cast<int>(bWallDestroyedRight);
    if (up)
        destroyedTileIndex = (destroyedTileIndex << 1) | static_cast<int>(bWallDestroyedUp);

    // Now perform the test
    if ((left) && (right) && (up) && (down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_Full : 59 + destroyedTileIndex; // solid wall
    } else if ((!left) && (right) && (up) && (down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpDownRight : 31 + destroyedTileIndex; // missing left edge
    } else if ((left) && (!right) && (up) && (down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpDownLeft : 45 + destroyedTileIndex; // missing right edge
    } else if ((left) && (right) && (!up) && (down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_DownLeftRight : 38 + destroyedTileIndex; // missing top edge
    } else if ((left) && (right) && (up) && (!down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpLeftRight : 52 + destroyedTileIndex; // missing bottom edge
    } else if ((!left) && (right) && (!up) && (down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_DownRight : 22 + destroyedTileIndex; // missing top left edge
    } else if ((left) && (!right) && (up) && (!down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpLeft : 28 + destroyedTileIndex; // missing bottom right edge
    } else if ((left) && (!right) && (!up) && (down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_DownLeft : 25 + destroyedTileIndex; // missing top right edge
    } else if ((!left) && (right) && (up) && (!down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpRight : 19 + destroyedTileIndex; // missing bottom left edge
    } else if ((left) && (!right) && (!up) && (!down)) {
        maketile =
            (destroyedTileIndex == 0) ? Wall_LeftRight : 14 + destroyedTileIndex; // missing above, right and below
    } else if ((!left) && (right) && (!up) && (!down)) {
        maketile =
            (destroyedTileIndex == 0) ? Wall_LeftRight : 13 + destroyedTileIndex; // missing above, left and below
    } else if ((!left) && (!right) && (up) && (!down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpDown : 16 + destroyedTileIndex; // only up
    } else if ((!left) && (!right) && (!up) && (down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpDown : 17 + destroyedTileIndex; // only down
    } else if ((left) && (right) && (!up) && (!down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_LeftRight : 13 + destroyedTileIndex; // missing above and below
    } else if ((!left) && (!right) && (up) && (down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_UpDown : 16 + destroyedTileIndex; // missing left and right
    } else if ((!left) && (!right) && (!up) && (!down)) {
        maketile = (destroyedTileIndex == 0) ? Wall_Standalone : 12 + destroyedTileIndex; // missing left and right
    }

    setWallTile(maketile);
}
