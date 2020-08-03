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

#include <units/Carryall.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/SFXManager.h>
#include <House.h>
#include <Map.h>
#include <Game.h>
#include <SoundPlayer.h>

#include <structures/RepairYard.h>
#include <structures/Refinery.h>
#include <structures/ConstructionYard.h>
#include <units/Harvester.h>

namespace {
constexpr AirUnitConstants carryall_constants{Carryall::item_id};
} // namespace

Carryall::Carryall(Uint32 objectID, const ObjectInitializer& initializer)
    : AirUnit(carryall_constants, objectID, initializer) {
    Carryall::init();

    ObjectBase::setHealth(getMaxHealth());

    owned = true;

    aDropOfferer = false;
    droppedOffCargo = false;
    respondable = false;
}

Carryall::Carryall(Uint32 objectID, const ObjectStreamInitializer& initializer)
    : AirUnit(carryall_constants, objectID, initializer) {
    Carryall::init();

    auto& stream = initializer.stream();

    pickedUpUnitList = stream.readUint32Vector();
    if(!pickedUpUnitList.empty()) {
        drawnFrame = 1;
    }

    stream.readBools(&owned, &aDropOfferer, &droppedOffCargo);
}

void Carryall::init()
{
    assert(itemID == Unit_Carryall);
    owner->incrementUnits(itemID);

    graphicID = ObjPic_Carryall;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    shadowGraphic = pGFXManager->getObjPic(ObjPic_CarryallShadow,getOwner()->getHouseID());

    numImagesX = static_cast<int>(ANGLETYPE::NUM_ANGLES);
    numImagesY = 2;
}

Carryall::~Carryall() = default;

void Carryall::save(OutputStream& stream) const
{
    AirUnit::save(stream);

    stream.writeUint32Vector(pickedUpUnitList);

    stream.writeBools(owned, aDropOfferer, droppedOffCargo);
}

bool Carryall::update(const GameContext& context) {
    const auto& maxSpeed = context.game.objectData.data[itemID][static_cast<int>(originalHouseID)].maxspeed;

    FixPoint dist = -1;
    auto *const pTarget = target.getObjPointer();
    if(pTarget != nullptr && pTarget->isAUnit()) {
        dist = distanceFrom(realX, realY, pTarget->getRealX(), pTarget->getRealY());
    } else if((pTarget != nullptr) || hasCargo()) {
        dist = distanceFrom(realX, realY, destination.x*TILESIZE + TILESIZE/2, destination.y*TILESIZE + TILESIZE/2);
    }

    if(dist >= 0) {
        static const FixPoint minSpeed = FixPoint32(TILESIZE/32);
        if(dist < TILESIZE/2) {
            currentMaxSpeed = std::min(dist, minSpeed);
        } else if(dist >= 10*TILESIZE) {
            currentMaxSpeed = maxSpeed;
        } else {
            FixPoint m = (maxSpeed-minSpeed) / ((10*TILESIZE)-(TILESIZE/2));
            FixPoint t = minSpeed-(TILESIZE/2)*m;
            currentMaxSpeed = dist*m+t;
        }
    } else {
        currentMaxSpeed = std::min(currentMaxSpeed + 0.2_fix, maxSpeed);
    }

    if(!AirUnit::update(context)) {
        return false;
    }

    // check if this carryall has to be removed because it has just brought something
    // to the map (e.g. new harvester)
    if (active) {
        const auto& map = context.map;
        if(aDropOfferer && droppedOffCargo && (!hasCargo())
            && ((getRealX() < -TILESIZE) || (getRealX() > (map.getSizeX()+1)*TILESIZE)
                || (getRealY() < -TILESIZE) || (getRealY() > (map.getSizeY()+1)*TILESIZE))) {
            setVisible(VIS_ALL, false);
            destroy(context);
            return false;
        }
    }
    return true;
}

void Carryall::deploy(const GameContext& context, const Coord& newLocation) {
    parent::deploy(context, newLocation);

    respondable = false;
}

void Carryall::checkPos(const GameContext& context) {
    parent::checkPos(context);

    if (!active) return;

    const auto& [game, map, objectManager] = context;

    if (hasCargo()) {
        if((location == destination) && (currentMaxSpeed <= 0.5_fix) ) {
            // drop up to 3 infantry units at once or one other unit
            auto droppedUnits = 0;
            do {
                const auto unitID = pickedUpUnitList.front();
                auto* const pUnit  = objectManager.getObject<UnitBase>(unitID);

                if(pUnit == nullptr) {
                    return;
                }

                if((!pUnit->isInfantry()) && (droppedUnits > 0)) {
                    // we already dropped infantry and this is no infantry
                    // => do not drop this here
                    break;
                }

                deployUnit(context, unitID);
                droppedUnits++;

                if(!pUnit->isInfantry()) {
                    // we dropped a non infantry unit
                    // => do not drop another unit
                    break;
                }
            } while(hasCargo() && (droppedUnits < 3));

            if(!pickedUpUnitList.empty()) {
                // find next place to drop
                for(auto i=8;i<18;i++) {
                    auto r = game.randomGen.rand(3,i/2);
                    const auto angle = 2 * FixPt_PI * game.randomGen.randFixPoint();

                    auto dropCoord = location + Coord( lround(r*FixPoint::sin(angle)), lround(-r*FixPoint::cos(angle)));
                    if(map.tileExists(dropCoord) && !map.getTile(dropCoord)->hasAGroundObject()) {
                        setDestination(dropCoord);
                        break;
                    }
                }
            } else {
                setTarget(nullptr);
                setDestination(guardPoint);
            }
        }
    } else if(!isBooked()) {
        if(destination.isValid()) {
            if(blockDistance(location, destination) <= 2) {
                destination.invalidate();
            }
        } else {
            if(blockDistance(location, guardPoint) > 17) {
                setDestination(guardPoint);
            }
        }
    }
}

void Carryall::pre_deployUnits(const GameContext& context)
{
    soundPlayer->playSoundAt(Sound_Drop, location);

    currentMaxSpeed = 0;
    setSpeeds(context);
}

void Carryall::deployUnit(const GameContext& context, Uint32 unitID) {
    const auto iter = std::find(pickedUpUnitList.cbegin(), pickedUpUnitList.cend(), unitID);

    if (pickedUpUnitList.cend() == iter)
        return;

    pickedUpUnitList.erase(iter);

    auto *const pUnit = context.objectManager.getObject<UnitBase>(unitID);

    if(pUnit == nullptr)
        return;

    pre_deployUnits(context);

    auto* const tile = context.map.tryGetTile(location.x, location.y);

    if(tile) deployUnit(context, tile, pUnit);
    else
        sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Carryall deploy failed for location %d, %d", location.x, location.y);

    post_deployUnits();
}

void Carryall::deployUnit(const GameContext& context, Tile* tile, UnitBase* pUnit) {
    if(tile->hasANonInfantryGroundObject()) {
        auto* const object = tile->getNonInfantryGroundObject(context.objectManager);
        if(object && object->getOwner() == getOwner()) {
            if(auto* const repair_yard = dune_cast<RepairYard>(object)) {

                if(repair_yard->isFree()) {
                    pUnit->setTarget(object); // unit books repair yard again
                    pUnit->setGettingRepaired();

                    return;
                }
                // unit is still going to repair yard but was unbooked from repair yard at pickup => book now

                repair_yard->book();
            } else if(auto* const refinery = dune_cast<Refinery>(object)) {
                if(refinery->isFree()) {
                    if(auto* const harvester = dune_cast<Harvester>(pUnit)) {
                        harvester->setTarget(object);
                        harvester->setReturned(context);
                        goingToRepairYard = false;

                        return;
                    }
                }
            }
        }
    }

    pUnit->setAngle(drawnAngle);
    const auto deployPos = context.map.findDeploySpot(pUnit, location);
    pUnit->setForced(false); // Stop units being forced if they are deployed
    pUnit->deploy(context, deployPos);
    if(pUnit->getItemID() == Unit_Saboteur) {
        pUnit->doSetAttackMode(context, HUNT);
    } else if(pUnit->getItemID() != Unit_Harvester) {
        pUnit->doSetAttackMode(context, AREAGUARD);
    } else {
        pUnit->doSetAttackMode(context, HARVEST);
    }
}

void Carryall::post_deployUnits() {
    if (!pickedUpUnitList.empty()) return;

    if (!aDropOfferer) {
        setTarget(nullptr);
        setDestination(guardPoint);
    }
    droppedOffCargo = true;
    drawnFrame = 0;

    clearPath();
}

void Carryall::destroy(const GameContext& context)
{
    // destroy cargo
    for(const auto pickedUpUnitID : pickedUpUnitList) {
        if (auto* const pPickedUpUnit = context.objectManager.getObject<UnitBase>(pickedUpUnitID)) {
            pPickedUpUnit->destroy(context);
        }
    }
    pickedUpUnitList.clear();

    // place wreck
    if(isVisible()) {
        if(auto* const pTile = context.map.tryGetTile(location.x, location.y)) {
            pTile->assignDeadUnit(DeadUnit_Carrall, owner->getHouseID(), Coord(lround(realX), lround(realY)));
        }
    }

    parent::destroy(context);
}

void Carryall::releaseTarget() {
    setTarget(nullptr);

    if(!hasCargo()) {
        setDestination(guardPoint);
    }
}

void Carryall::engageTarget(const GameContext& context) {
    if(!target) {
        assert(!hasCargo());
        return;
    }

    auto* object = target.getObjPointer();
    if(object == nullptr) {
        // the target does not exist anymore
        releaseTarget();
        return;
    }

    if(!object->isActive()) {
        // the target changed its state to inactive
        releaseTarget();
        return;
    }

    if(auto* groundUnit = dune_cast<GroundUnit>(object); groundUnit && !groundUnit->isAwaitingPickup()) {
        // the target changed its state to not awaiting pickup anymore
        releaseTarget();
        return;
    }

    if(object->getOwner()->getTeamID() != owner->getTeamID()) {
        // the target changed its owner (e.g. was deviated)
        releaseTarget();
        return;
    }

    Coord targetLocation;
    if(object->getItemID() == Structure_Refinery) {
        targetLocation = object->getLocation() + Coord(2,0);
    } else {
        targetLocation = object->getClosestPoint(location);
    }

    const Coord realLocation = Coord(lround(realX), lround(realY));
    const Coord realDestination = targetLocation * TILESIZE + Coord(TILESIZE/2,TILESIZE/2);

    targetDistance = distanceFrom(realLocation, realDestination);

    if (targetDistance <= TILESIZE/32) {
        if(hasCargo()) {
            if(object->isAStructure()) {
                while(!pickedUpUnitList.empty()) {
                    deployUnit(context, pickedUpUnitList.back());
                }

                setTarget(nullptr);
                setDestination(guardPoint);
            }
        } else {
            pickupTarget(context);
        }
    } else {
        setDestination(targetLocation);
    }
}

void Carryall::giveCargo(const GameContext& context, UnitBase* newUnit) {
    if(newUnit == nullptr) {
        return;
    }

    pickedUpUnitList.push_back(newUnit->getObjectID());

    newUnit->setPickedUp(context, this);

    drawnFrame = 1;

    droppedOffCargo = false;
}

void Carryall::pickupTarget(const GameContext& context) {
    currentMaxSpeed = 0;
    setSpeeds(context);

    auto* pTarget = target.getObjPointer();

    if(!pTarget) return;

    if(auto* pGroundUnitTarget = dune_cast<GroundUnit>(pTarget)) {

        if(pTarget->getHealth() <= 0) {
            // unit died just in the moment we tried to pick it up => carryall also crushes
            setHealth(0);
            return;
        }

        if (  pGroundUnitTarget->hasATarget()
            || ( pGroundUnitTarget->getDestination() != pGroundUnitTarget->getLocation())
            || pGroundUnitTarget->isBadlyDamaged()) {

            if(pGroundUnitTarget->isBadlyDamaged() || (!pGroundUnitTarget->hasATarget() && pGroundUnitTarget->getItemID() != Unit_Harvester))   {
                pGroundUnitTarget->doRepair(context);
            }

            auto *newTarget = pGroundUnitTarget->hasATarget() ? pGroundUnitTarget->getTarget() : nullptr;

            pickedUpUnitList.push_back(target.getObjectID());
            pGroundUnitTarget->setPickedUp(context, this);

            drawnFrame = 1;

            if(newTarget && (newTarget->getItemID() == Structure_Refinery)) {
                pGroundUnitTarget->setGuardPoint(pGroundUnitTarget->getLocation());
                setTarget(newTarget);
                setDestination(target.getObjPointer()->getLocation() + Coord(2,0));
            } else if(newTarget && (newTarget->getItemID() == Structure_RepairYard)) {
                pGroundUnitTarget->setGuardPoint(pGroundUnitTarget->getLocation());
                setTarget(newTarget);
                setDestination(target.getObjPointer()->getClosestPoint(location));
            } else if (pGroundUnitTarget->getDestination().isValid()) {
                setDestination(pGroundUnitTarget->getDestination());
            }

            clearPath();

        } else {
            pGroundUnitTarget->setAwaitingPickup(false);
            if(pGroundUnitTarget->getAttackMode() == CARRYALLREQUESTED) {
                pGroundUnitTarget->doSetAttackMode(context, STOP);
            }
            releaseTarget();
        }
    } else {
        // get unit from structure
        if(auto* refinery = dune_cast<Refinery>(pTarget)) {
            // get harvester
            refinery->deployHarvester(context, this);
        } else if(auto* repairYard = dune_cast<RepairYard>(pTarget)) {
            // get repaired unit
            repairYard->deployRepairUnit(context, this);
        }
    }
}

void Carryall::setTarget(const ObjectBase* newTarget) {
    auto* pTarget = target.getObjPointer();

    if(pTarget) {
        if(targetFriendly) {
            auto* groundUnit = dune_cast<GroundUnit>(pTarget);
            if(groundUnit && groundUnit->getCarrier() == this) groundUnit->bookCarrier(nullptr);
        }

        if(auto* refinery = dune_cast<Refinery>(pTarget)) refinery->unBook();
    }

    parent::setTarget(newTarget);

    pTarget = target.getObjPointer();
    if(!pTarget) return;

    if(auto* refinery = dune_cast<Refinery>(pTarget)) refinery->book();

    if(targetFriendly) {
        if(auto* groundUnit = dune_cast<GroundUnit>(pTarget)) groundUnit->setAwaitingPickup(true);
    }
}

void Carryall::targeting(const GameContext& context) {
    if(target) {
        engageTarget(context);
    }
}

void Carryall::turn(const GameContext& context) {
    auto& map = context.map;

    if (active && aDropOfferer && droppedOffCargo && (!hasCargo())
        && ((getRealX() < TILESIZE/2) || (getRealX() > map.getSizeX()*TILESIZE - TILESIZE/2)
            || (getRealY() < TILESIZE/2) || (getRealY() > map.getSizeY()*TILESIZE - TILESIZE/2))) {
        // already partially outside the map => do not turn
        return;
    }

    parent::turn(context);
}
