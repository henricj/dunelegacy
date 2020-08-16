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

#include "engine_mmath.h"

#include <units/Frigate.h>

#include <House.h>
#include <Map.h>
#include <Game.h>

#include <structures/StarPort.h>

namespace {
using namespace Dune::Engine;

constexpr AirUnitConstants frigate_constants{Frigate::item_id};
} // namespace

namespace Dune::Engine {

Frigate::Frigate(uint32_t objectID, const ObjectInitializer& initializer)
    : AirUnit(frigate_constants, objectID, initializer) {
    Frigate::init();

    Frigate::setHealth(initializer.game(), getMaxHealth(initializer.game()));

    attackMode = GUARD;

    respondable     = false;
    droppedOffCargo = false;
}

Frigate::Frigate(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : AirUnit(frigate_constants, objectID, initializer) {
    Frigate::init();

    auto& stream = initializer.stream();

    droppedOffCargo = stream.readBool();
}

void Frigate::init() {
    assert(itemID == Unit_Frigate);
    owner->incrementUnits(itemID);
}

Frigate::~Frigate() {
    auto* pStarPort = dynamic_cast<StarPort*>(target.getObjPointer());
    if(pStarPort) { pStarPort->informFrigateDestroyed(); }
}

void Frigate::save(const Game& game, OutputStream& stream) const {
    parent::save(game, stream);

    stream.writeBool(droppedOffCargo);
}

void Frigate::checkPos(const GameContext& context) {
    parent::checkPos(context);

    if((location == destination) && (distanceFrom(realX, realY, destination.x * TILESIZE + (TILESIZE / 2),
                                                  destination.y * TILESIZE + (TILESIZE / 2)) < TILESIZE / 8)) {
        auto* const pStarport = dune_cast<StarPort>(target.getStructurePointer(context.objectManager));

        if(pStarport != nullptr) {
            pStarport->startDeploying();
            setTarget(context.objectManager, nullptr);
            setDestination(context, guardPoint);
            droppedOffCargo = true;
        }
    }
}

bool Frigate::update(const GameContext& context) {
    const FixPoint& maxSpeed = context.game.getObjectData(itemID, originalHouseID).maxspeed;

    FixPoint    dist    = -1;
    auto* const pTarget = target.getObjPointer(context.objectManager);
    if(pTarget != nullptr && pTarget->isAUnit()) {
        dist = distanceFrom(realX, realY, pTarget->getRealX(), pTarget->getRealY());
    } else if((pTarget != nullptr) || !droppedOffCargo) {
        dist = distanceFrom(realX, realY, destination.x * TILESIZE + TILESIZE / 2,
                            destination.y * TILESIZE + TILESIZE / 2);
    }

    if(dist >= 0) {
        static const FixPoint minSpeed = FixPoint32(TILESIZE / 32);
        if(dist < TILESIZE / 2) {
            currentMaxSpeed = std::min(dist, minSpeed);
        } else if(dist >= 10 * TILESIZE) {
            currentMaxSpeed = maxSpeed;
        } else {
            FixPoint m      = (maxSpeed - minSpeed) / ((10 * TILESIZE) - (TILESIZE / 2));
            FixPoint t      = minSpeed - (TILESIZE / 2) * m;
            currentMaxSpeed = dist * m + t;
        }
    } else {
        currentMaxSpeed = std::min(currentMaxSpeed + 0.2_fix, maxSpeed);
    }

    if(!AirUnit::update(context)) { return false; }

    // check if target is destroyed
    if((!droppedOffCargo) && target.getStructurePointer(context.objectManager) == nullptr) {
        setDestination(context, guardPoint);
        droppedOffCargo = true;
    }

    // check if this frigate has to be removed because it has just brought all units to the Starport
    if(active) {
        if(droppedOffCargo && ((getRealX() < -TILESIZE) || (getRealX() > (context.map.getSizeX() + 1) * TILESIZE) ||
                               (getRealY() < -TILESIZE) || (getRealY() > (context.map.getSizeY() + 1) * TILESIZE))) {

            setVisible(VIS_ALL, false);
            destroy(context);
            return false;
        }
    }
    return true;
}

void Frigate::deploy(const GameContext& context, const Coord& newLocation) {
    parent::deploy(context, newLocation);

    respondable = false;
}

void Frigate::turn(const GameContext& context) {
    if(active && droppedOffCargo &&
       ((getRealX() < TILESIZE / 2) || (getRealX() > context.map.getSizeX() * TILESIZE - TILESIZE / 2) ||
        (getRealY() < TILESIZE / 2) || (getRealY() > context.map.getSizeY() * TILESIZE - TILESIZE / 2))) {
        // already partially outside the map => do not turn
        return;
    }

    parent::turn(context);
}

} // namespace Dune::Engine
