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

#include <units/Harvester.h>

#include <structures/RepairYard.h>
#include <structures/Refinery.h>

#include <House.h>
#include <Map.h>
#include <Game.h>

namespace Dune::Engine {

TrackedUnit::TrackedUnit(const TrackedUnitConstants& constants, uint32_t objectID, const ObjectInitializer& initializer)
    : GroundUnit(constants, objectID, initializer) { }

TrackedUnit::TrackedUnit(const TrackedUnitConstants& constants, uint32_t objectID,
                         const ObjectStreamInitializer& initializer)
    : GroundUnit(constants, objectID, initializer) { }

TrackedUnit::~TrackedUnit() = default;

void TrackedUnit::save(const Game& game, OutputStream& stream) const { GroundUnit::save(game, stream); }

void TrackedUnit::checkPos(const GameContext& context) {
    GroundUnit::checkPos(context);

    if(active && justStoppedMoving) {
        const auto* const tile = context.map.tryGetTile(location.x, location.y);

        if(tile) tile->squash(context);
    }
}

bool TrackedUnit::canPassTile(const GameContext& context, const Tile* pTile) const {
    if(!pTile || pTile->isMountain()) { return false; }

    const auto [ok, ground_object_id] = pTile->getGroundObjectID();

    if(!ok) return true;

    if(ground_object_id == target.getObjectID()) {
        auto* const pObject = context.objectManager.getObject(ground_object_id);

        if((pObject != nullptr) && targetFriendly && pObject->isAStructure() &&
           (pObject->getOwner()->getTeamID() == owner->getTeamID()) && pObject->isVisible(getOwner()->getTeamID())) {
            // are we entering a repair yard?
            if(goingToRepairYard && (pObject->getItemID() == Structure_RepairYard)) {
                return static_cast<const RepairYard*>(pObject)->isFree();
            }

            if(const auto* const pHarvester = dune_cast<Harvester>(this)) {
                return (pHarvester->isReturning() && (pObject->getItemID() == Structure_Refinery) &&
                        static_cast<const Refinery*>(pObject)->isFree());
            }

            return false;
        }
    }

    if(!pTile->hasANonInfantryGroundObject()) {
        // The tile does not have a non-infantry ground object, therefore the ground object ID must
        // be for an infantry unit.  We have complicated this function since profiling puts it in
        // the hotpath...
        auto* const pObject = context.objectManager.getObject(ground_object_id);
        if(pObject && pObject->getOwner()->getTeamID() != getOwner()->getTeamID()) {
            // possibly squashing this unit
            return true;
        }
        return false;
    }

    return false;
}

constexpr std::array<FixPoint, Terrain_SpecialBloom + 1> TrackedUnit::terrain_difficulty = {
    1_fix,      // Terrain_Slab
    1.5625_fix, // Terrain_Sand
    1.375_fix,  // Terrain_Rock
    1.375_fix,  // Terrain_Dunes
    1.0_fix,    // Terrain_Mountain
    1.375_fix,  // Terrain_Spice
    1.375_fix,  // Terrain_ThickSpice
    1.5625_fix, // Terrain_SpiceBloom
    1.5625_fix  // Terrain_SpecialBloom
};

} // namespace Dune::Engine
