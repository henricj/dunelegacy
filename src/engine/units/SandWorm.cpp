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

#include <units/SandWorm.h>

#include <House.h>
#include <Game.h>
#include <Map.h>

#include <units/InfantryBase.h>

namespace {
using namespace Dune::Engine;

constexpr int MAX_SANDWORMSLEEPTIME = 50000;
constexpr int MIN_SANDWORMSLEEPTIME = 10000;

constexpr int SANDWORM_ATTACKFRAMETIME = 10;

class SandwormConstants : public GroundUnitConstants {
public:
    constexpr SandwormConstants() : GroundUnitConstants{Sandworm::item_id} { canAttackStuff_ = true; }
};

constexpr SandwormConstants sandworm_constants;
}

namespace Dune::Engine {

Sandworm::Sandworm(uint32_t objectID, const ObjectInitializer& initializer)
    : GroundUnit(sandworm_constants, objectID, initializer) {

    Sandworm::init();

    Sandworm::setHealth(initializer.game(), getMaxHealth(initializer.game()));

    kills                      = 0;
    attackFrameTimer           = 0;
    sleepTimer                 = 0;
    warningWormSignPlayedFlags = 0;
    respondable                = false;

    for(auto& lastLoc : lastLocs) {
        lastLoc.invalidate();
    }
    shimmerOffsetIndex = -1;
}

Sandworm::Sandworm(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : GroundUnit(sandworm_constants, objectID, initializer) {

    Sandworm::init();

    auto& stream = initializer.stream();

    kills                      = stream.readSint32();
    attackFrameTimer           = stream.readSint32();
    sleepTimer                 = stream.readSint32();
    warningWormSignPlayedFlags = stream.readUint8();
    shimmerOffsetIndex         = stream.readSint32();
    for(auto& lastLoc : lastLocs) {
        lastLoc.x = stream.readSint32();
        lastLoc.y = stream.readSint32();
    }
}

void Sandworm::init() {
    assert(itemID == Unit_Sandworm);
    owner->incrementUnits(itemID);

    drawnFrame = INVALID;
}

Sandworm::~Sandworm() = default;

void Sandworm::save(const Game& game, OutputStream& stream) const {
    GroundUnit::save(game, stream);

    stream.writeSint32(kills);
    stream.writeSint32(attackFrameTimer);
    stream.writeSint32(sleepTimer);
    stream.writeUint8(warningWormSignPlayedFlags);
    stream.writeSint32(shimmerOffsetIndex);
    for(auto lastLoc : lastLocs) {
        stream.writeSint32(lastLoc.x);
        stream.writeSint32(lastLoc.y);
    }
}

void Sandworm::assignToMap(const GameContext& context, const Coord& pos) {
    if(auto* tile = context.map.tryGetTile(pos.x, pos.y)) {
        tile->assignUndergroundUnit(getObjectID());
        // do not unhide map cause this would give Fremen players an advantage
        // currentGameMap->viewMap(owner->getHouseID(), location, getViewRange());
    }
}

bool Sandworm::attack(const GameContext& context) {
    if(primaryWeaponTimer == 0) {
        if(target) {
            drawnFrame         = 0;
            attackFrameTimer   = SANDWORM_ATTACKFRAMETIME;
            primaryWeaponTimer = getWeaponReloadTime(context.game);
            return true;
        }
    }
    return false;
}

void Sandworm::deploy(const GameContext& context, const Coord& newLocation) {
    parent::deploy(context, newLocation);

    respondable = false;
}

void Sandworm::checkPos(const GameContext& context) {
    if(justStoppedMoving) {
        realX = location.x * TILESIZE + TILESIZE / 2;
        realY = location.y * TILESIZE + TILESIZE / 2;

        const auto* const infantry = context.map.tryGetInfantry(context, location.x, location.y);
    }
}

void Sandworm::engageTarget(const GameContext& context) {
    if(isEating()) { return; }

    parent::engageTarget(context);

    if(target) {
        FixPoint maxDistance;

        if(forced) {
            maxDistance = FixPt_MAX;
        } else {
            switch(attackMode) {
                case GUARD:
                case AMBUSH: {
                    maxDistance = getViewRange(context.game);
                } break;

                case AREAGUARD:
                case HUNT: {
                    maxDistance = FixPt_MAX;
                } break;

                case STOP:
                case CAPTURE:
                default: {
                    maxDistance = 0;
                } break;
            }
        }

        if(targetDistance > maxDistance) {
            // give up
            setDestination(context, guardPoint);
            setTarget(context.objectManager, nullptr);
        }
    }
}

void Sandworm::setLocation(const GameContext& context, int xPos, int yPos) {
    if(context.map.tileExists(xPos, yPos) || ((xPos == INVALID_POS) && (yPos == INVALID_POS))) {
        parent::setLocation(context, xPos, yPos);
    }
}

/**
    Put sandworm to sleep for a while
*/
void Sandworm::sleep(const GameContext& context) {
    sleepTimer = context.game.randomGen.rand(MIN_SANDWORMSLEEPTIME, MAX_SANDWORMSLEEPTIME);
    setActive(false);
    setVisible(VIS_ALL, false);
    setForced(false);
    context.map.removeObjectFromMap(getObjectID()); // no map point will reference now
    setLocation(context, INVALID_POS, INVALID_POS);
    setHealth(context.game, getMaxHealth(context.game));
    kills                      = 0;
    warningWormSignPlayedFlags = 0;
    drawnFrame                 = INVALID;
    attackFrameTimer           = 0;
    shimmerOffsetIndex         = -1;
    for(auto& lastLoc : lastLocs) {
        lastLoc.invalidate();
    }
}

bool Sandworm::sleepOrDie(const GameContext& context) {

    // Make sand worms always drop spice, even if they don't die
    if(context.game.getGameInitSettings().getGameOptions().killedSandwormsDropSpice) {
        context.map.createSpiceField(context, location, 4);
    }

    if(context.game.getGameInitSettings().getGameOptions().sandwormsRespawn) {
        sleep(context);
        return true;
    }
    destroy(context);

    return false;
}

void Sandworm::setTarget(const ObjectManager& objectManager, const ObjectBase* newTarget) {
    parent::setTarget(objectManager, newTarget);
}

void Sandworm::handleDamage(const GameContext& context, int damage, uint32_t damagerID, House* damagerOwner) {
    if(damage > 0) { attackMode = HUNT; }
    parent::handleDamage(context, damage, damagerID, damagerOwner);
}

bool Sandworm::update(const GameContext& context) {
    auto& [game, map, objectManager] = context;

    if(getHealth() <= getMaxHealth(context.game) / 2) {
        if(!sleepOrDie(context)) { return false; }
    } else {
        if(!parent::update(context)) { return false; }

        if(isActive() && (moving || justStoppedMoving) && !game.isGameFinished()) {
            Coord realLocation = getLocation() * TILESIZE + Coord(TILESIZE / 2, TILESIZE / 2);
            if(lastLocs[1] != realLocation) {
                for(int i = (SANDWORM_SEGMENTS - 1); i > 0; i--) {
                    lastLocs[i] = lastLocs[i - 1];
                }
                lastLocs[1] = realLocation;
            }
            lastLocs[0].x      = lround(realX);
            lastLocs[0].y      = lround(realY);
            shimmerOffsetIndex = ((game.getGameCycleCount() + getObjectID()) % 48) / 6;
        }

        if(attackFrameTimer > 0) {
            attackFrameTimer--;

            // death frame has started
            if(attackFrameTimer == 0) {
                drawnFrame++;
                if(drawnFrame >= 9) {
                    drawnFrame = INVALID;
                    if(kills >= 3) {
                        if(!sleepOrDie(context)) { return false; }
                    }
                } else {
                    attackFrameTimer = SANDWORM_ATTACKFRAMETIME;
                    if(drawnFrame == 1) {
                        // the close mouth bit of graphic is currently shown => eat unit
                        if(target) {
                            auto* object = target.getObjPointer(objectManager);

                            if(object) {
                                bool wasAlive =
                                    object->isVisible(getOwner()->getTeamID()); // see if unit was alive before attack
                                Coord realPos = Coord(lround(realX), lround(realY));
                                map.damage(context, objectID, getOwner(), realPos, Bullet_Sandworm, 5000, NONE_ID,
                                           false);
                                // TODO: map.damage() might have invalidated "object"?  Do we need an object->isAlive()
                                // method?
                                if(wasAlive) {
                                    auto* const object2 = target.getObjPointer(objectManager);

                                    if(object2 && !object2->isVisible(getOwner()->getTeamID())) {
                                        kills++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if(sleepTimer > 0) {
            sleepTimer--;

            if(sleepTimer == 0) {
                // awaken the worm!

                for(int tries = 0; tries < 1000; tries++) {
                    int x = game.randomGen.rand(0, map.getSizeX() - 1);
                    int y = game.randomGen.rand(0, map.getSizeY() - 1);

                    if(canPass(context, x, y)) {
                        deploy(context, map.getTile(x, y)->getLocation());
                        break;
                    }
                }

                if(!isActive()) {
                    // no room for sandworm on map => take another nap
                    if(!sleepOrDie(context)) { return false; }
                }
            }
        }
    }

    return true;
}

bool Sandworm::canAttack(const GameContext& context, const ObjectBase* object) const {
    if(!object || !object->isAGroundUnit() || object->getItemID() == Unit_Sandworm) return false;

    auto& map = context.map;

    const auto  pos         = object->getLocation();
    auto* const object_tile = map.tryGetTile(pos.x, pos.y);

    if(!object_tile || !canPassTile(context, object_tile)) return false;

    auto* const this_tile = map.tryGetTile(location.x, location.y);
    if(!this_tile) return false;

    return object_tile->getSandRegion() == this_tile->getSandRegion();
}

bool Sandworm::canPassTile(const GameContext& context, const Tile* pTile) const {
    return !pTile->isRock() &&
           (!pTile->hasAnUndergroundUnit() || (pTile->getUndergroundUnit(context.objectManager) == this));
}

const ObjectBase* Sandworm::findTarget(const GameContext& context) const {
    if(isEating()) { return nullptr; }

    const ObjectBase* closestTarget = nullptr;

    if((attackMode == HUNT) || (attackMode == AREAGUARD)) {
        auto closestDistance = FixPt_MAX;

        for(auto* pUnit : context.game.unitList) {
            if(canAttack(context, pUnit) && (blockDistance(location, pUnit->getLocation()) < closestDistance)) {
                closestTarget   = pUnit;
                closestDistance = blockDistance(location, pUnit->getLocation());
            }
        }
    } else {
        closestTarget = parent::findTarget(context);
    }

    return closestTarget;
}

ANGLETYPE Sandworm::getCurrentAttackAngle() const {
    // we can always attack an target
    return targetAngle;
}

} // namespace Dune::Engine
