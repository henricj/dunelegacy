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

#include "engine_sand.h"

#include <units/MCV.h>

#include <House.h>
#include <Explosion.h>
#include <Game.h>
#include <Map.h>

#include <players/HumanPlayer.h>

namespace {
using namespace Dune::Engine;

constexpr GroundUnitConstants mcv_constants{MCV::item_id, false};
}

namespace Dune::Engine {

MCV::MCV(uint32_t objectID, const ObjectInitializer& initializer) : GroundUnit(mcv_constants, objectID, initializer) {
    MCV::init();

    MCV::setHealth(initializer.game(), getMaxHealth(initializer.game()));
    attackMode = GUARD;
}

MCV::MCV(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : GroundUnit(mcv_constants, objectID, initializer) {
    MCV::init();
}

void MCV::init() {
    assert(itemID == Unit_MCV);
    owner->incrementUnits(itemID);
}

MCV::~MCV() = default;

bool MCV::doDeploy(const GameContext& context) {
    // check if there is enough room for construction yard
    if(!canDeploy(context)) { return false; }

    // save needed values
    auto* const pOwner      = getOwner();
    const auto  newLocation = getLocation();

    // first place construction yard and then destroy MCV, otherwise a player with only MCV left will lose

    // place construction yard (force placing to place on still existing MCV)
    if(pOwner->placeStructure(NONE_ID, Structure_ConstructionYard, newLocation.x, newLocation.y, false, true) !=
       nullptr) {
        // we hide the MVC so we don't get a soldier on destroy
        setVisible(VIS_ALL, false);

        // destroy MCV but with base class method since we want no explosion
        parent::destroy(context);

        return true;
    }

    return false;
}

bool MCV::canAttack(const GameContext& context, const ObjectBase* object) const {
    return ((object != nullptr) && object->isInfantry() && (object->getOwner()->getTeamID() != owner->getTeamID()) &&
            object->isVisible(getOwner()->getTeamID()));
}

void MCV::destroy(const GameContext& context) {
    if(context.map.tileExists(location) && isVisible()) {
        Coord realPos(lround(realX), lround(realY));
        context.game.addExplosion(Explosion_SmallUnit, realPos, owner->getHouseID());
    }

    GroundUnit::destroy(context);
}

bool MCV::canDeploy(const GameContext& context, int x, int y) const {
    const auto structure_size = getStructureSize(Structure_ConstructionYard);

    for(int i = 0; i < structure_size.x; i++) {
        for(int j = 0; j < structure_size.y; j++) {
            const Tile* pTile = context.map.tryGetTile(x + i, y + j);
            if(!pTile) return false;

            if(i == 0 && j == 0 || !pTile->isBlocked()) {
                // tile is not blocked or we're checking the tile with the MCV on
                if(!pTile->isRock()) { return false; }
            } else {
                return false;
            }
        }
    }

    return true;
}

} // namespace Dune::Engine
