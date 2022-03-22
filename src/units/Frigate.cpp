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

#include <units/Frigate.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/SFXManager.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <SoundPlayer.h>

#include <structures/StarPort.h>

namespace {
constexpr AirUnitConstants frigate_constants {Frigate::item_id};
} // namespace

Frigate::Frigate(uint32_t objectID, const ObjectInitializer& initializer)
    : AirUnit(frigate_constants, objectID, initializer) {
    Frigate::init();

    ObjectBase::setHealth(getMaxHealth());

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

    graphicID     = ObjPic_Frigate;
    graphic       = pGFXManager->getObjPic(graphicID, getOwner()->getHouseID());
    shadowGraphic = pGFXManager->getObjPic(ObjPic_FrigateShadow, getOwner()->getHouseID());

    numImagesX = static_cast<int>(ANGLETYPE::NUM_ANGLES);
    numImagesY = 1;
}

Frigate::~Frigate() {
    auto* pStarPort = dynamic_cast<StarPort*>(target.getObjPointer());
    if (pStarPort) {
        pStarPort->informFrigateDestroyed();
    }
}

void Frigate::save(OutputStream& stream) const {
    AirUnit::save(stream);

    stream.writeBool(droppedOffCargo);
}

void Frigate::checkPos(const GameContext& context) {
    AirUnit::checkPos(context);

    if ((location == destination) && (distanceFrom(realX, realY, destination.x * TILESIZE + (TILESIZE / 2), destination.y * TILESIZE + (TILESIZE / 2)) < TILESIZE / 8)) {
        auto* pStarport = dynamic_cast<StarPort*>(target.getStructurePointer());

        if (pStarport != nullptr) {
            pStarport->startDeploying();
            setTarget(nullptr);
            setDestination(guardPoint);
            droppedOffCargo = true;
            soundPlayer->playSoundAt(Sound_Drop, location);
        }
    }
}

bool Frigate::update(const GameContext& context) {
    const FixPoint& maxSpeed = context.game.objectData.data[itemID][static_cast<int>(originalHouseID)].maxspeed;

    FixPoint dist       = -1;
    auto* const pTarget = target.getObjPointer();
    if (pTarget != nullptr && pTarget->isAUnit()) {
        dist = distanceFrom(realX, realY, pTarget->getRealX(), pTarget->getRealY());
    } else if ((pTarget != nullptr) || !droppedOffCargo) {
        dist = distanceFrom(realX, realY, destination.x * TILESIZE + TILESIZE / 2, destination.y * TILESIZE + TILESIZE / 2);
    }

    if (dist >= 0) {
        static const FixPoint minSpeed = FixPoint32(TILESIZE / 32);
        if (dist < TILESIZE / 2) {
            currentMaxSpeed = std::min(dist, minSpeed);
        } else if (dist >= 10 * TILESIZE) {
            currentMaxSpeed = maxSpeed;
        } else {
            const FixPoint m = (maxSpeed - minSpeed) / ((10 * TILESIZE) - (TILESIZE / 2));
            const FixPoint t = minSpeed - (TILESIZE / 2) * m;
            currentMaxSpeed  = dist * m + t;
        }
    } else {
        currentMaxSpeed = std::min(currentMaxSpeed + 0.2_fix, maxSpeed);
    }

    if (!AirUnit::update(context)) {
        return false;
    }

    // check if target is destroyed
    if ((!droppedOffCargo) && target.getStructurePointer() == nullptr) {
        setDestination(guardPoint);
        droppedOffCargo = true;
    }

    // check if this frigate has to be removed because it has just brought all units to the Starport
    if (active) {
        if (droppedOffCargo && ((getRealX() < -TILESIZE) || (getRealX() > (currentGameMap->getSizeX() + 1) * TILESIZE) || (getRealY() < -TILESIZE) || (getRealY() > (currentGameMap->getSizeY() + 1) * TILESIZE))) {

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
    if (active && droppedOffCargo && ((getRealX() < TILESIZE / 2) || (getRealX() > currentGameMap->getSizeX() * TILESIZE - TILESIZE / 2) || (getRealY() < TILESIZE / 2) || (getRealY() > currentGameMap->getSizeY() * TILESIZE - TILESIZE / 2))) {
        // already partially outside the map => do not turn
        return;
    }

    parent::turn(context);
}
