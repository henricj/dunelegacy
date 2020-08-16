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

#include <structures/StructureBase.h>

#include <House.h>
#include <Game.h>
#include <Map.h>
#include <Explosion.h>

#include <players/HumanPlayer.h>

#include <units/UnitBase.h>

namespace Dune::Engine {

StructureBase::StructureBase(const StructureBaseConstants& structure_constants, uint32_t objectID,
                             const ObjectInitializer& initializer)
    : ObjectBase(structure_constants, objectID, initializer) {
    StructureBase::init(initializer.game());

    repairing    = false;
    fogged       = false;
    degradeTimer = MILLI2CYCLES(15 * 1000);
}

StructureBase::StructureBase(const StructureBaseConstants& structure_constants, uint32_t objectID,
                             const ObjectStreamInitializer& initializer)
    : ObjectBase(structure_constants, objectID, initializer) {
    StructureBase::init(initializer.game());

    auto& stream = initializer.stream();

    repairing = stream.readBool();
    fogged    = stream.readBool();

    degradeTimer = stream.readSint32();

    size_t numSmoke = stream.readUint32();
    for(size_t i = 0; i < numSmoke; i++) {
        smoke.emplace_back(stream);
    }
}

void StructureBase::init(Game& game) {
    justPlacedTimer = 0;

    game.structureList.push_back(this);
}

StructureBase::~StructureBase() = default;

void StructureBase::cleanup(const GameContext& context, HumanPlayer* humanPlayer) {
    try {
        context.map.removeObjectFromMap(getObjectID()); // no map point will reference now
        context.game.structureList.remove(this);
        owner->decrementStructures(itemID, location);
    } catch(std::exception& e) { Dune::Logger.log("StructureBase::cleanup(): %s", e.what()); }

    parent::cleanup(context, humanPlayer);
}

void StructureBase::save(const Game& game, OutputStream& stream) const {
    ObjectBase::save(game, stream);

    stream.writeBool(repairing);
    stream.writeBool(fogged);

    stream.writeSint32(degradeTimer);

    stream.writeUint32(smoke.size());
    for(const auto& structureSmoke : smoke) {
        structureSmoke.save(stream);
    }
}

void StructureBase::assignToMap(const GameContext& context, const Coord& pos) {
    auto bFoundNonConcreteTile = false;

    auto& game = context.game;
    auto& map  = context.map;

    map.for_each(pos.x, pos.y, pos.x + getStructureSizeX(), pos.y + getStructureSizeY(), [&](Tile& t) {
        t.assignNonInfantryGroundObject(getObjectID());

        if(!t.isConcrete() && game.getGameInitSettings().getGameOptions().concreteRequired &&
           (game.gameState != GameState::Start)) {
            bFoundNonConcreteTile = true;

            if((itemID != Structure_Wall) && (itemID != Structure_ConstructionYard)) {
                setHealth(game,
                          getHealth() - FixPoint(getMaxHealth(game)) / (2 * getStructureSizeX() * getStructureSizeY()));
            }
        }
        t.setType(context, Terrain_Rock);
        t.setOwner(getOwner()->getHouseID());

        setVisible(VIS_ALL, true);
        setActive(true);
        setRespondable(true);
    });

    map.viewMap(getOwner()->getHouseID(), pos, getViewRange(context.game));

    if(!bFoundNonConcreteTile && !game.getGameInitSettings().getGameOptions().structuresDegradeOnConcrete) {
        degradeTimer = -1;
    }
}

/**
    Returns the center point of this structure
    \return the center point in world coordinates
*/
Coord StructureBase::getCenterPoint() const {
    return Coord(lround(realX + getStructureSizeX() * TILESIZE / 2),
                 lround(realY + getStructureSizeY() * TILESIZE / 2));
}

Coord StructureBase::getClosestCenterPoint(const Coord& objectLocation) const {
    return getClosestPoint(objectLocation) * TILESIZE + Coord(TILESIZE / 2, TILESIZE / 2);
}

void StructureBase::doSetDeployPosition(const GameContext& context, int xPos, int yPos) {
    setTarget(context.objectManager, nullptr);
    setDestination(context, xPos, yPos);
    setForced(true);
}

void StructureBase::doRepair(const GameContext& context) { repairing = true; }

void StructureBase::setDestination(const GameContext& context, int newX, int newY) {
    if(context.map.tileExists(newX, newY) || ((newX == INVALID_POS) && (newY == INVALID_POS))) {
        destination.x = newX;
        destination.y = newY;
    }
}

void StructureBase::setJustPlaced() { justPlacedTimer = 6; }

bool StructureBase::update(const GameContext& context) {
    if(((context.game.getGameCycleCount() + getObjectID()) % 512) == 0) {
        context.map.viewMap(owner->getHouseID(), location, getViewRange(context.game));
    }

    if(!fogged) { }

    // degrade
    if((degradeTimer >= 0) && context.game.getGameInitSettings().getGameOptions().concreteRequired &&
       (owner->getPowerRequirement() > owner->getProducedPower())) {
        degradeTimer--;
        if(degradeTimer <= 0) {
            degradeTimer = MILLI2CYCLES(15 * 1000);

            int damageMultiplier = 1;
            if(owner->getHouseID() == HOUSETYPE::HOUSE_HARKONNEN || owner->getHouseID() == HOUSETYPE::HOUSE_SARDAUKAR) {
                damageMultiplier = 3;
            } else if(owner->getHouseID() == HOUSETYPE::HOUSE_ORDOS) {
                damageMultiplier = 2;
            } else if(owner->getHouseID() == HOUSETYPE::HOUSE_MERCENARY) {
                damageMultiplier = 5;
            }

            if(getHealth() > getMaxHealth(context.game) / 2) {
                setHealth(context.game, getHealth() - FixPoint(damageMultiplier * getMaxHealth(context.game)) / 100);
            }
        }
    }

    updateStructureSpecificStuff(context);

    if(getHealth() <= 0) {
        destroy(context);
        return false;
    }

    if(repairing) {
        if(owner->getCredits() >= 5) {
            // Original dune 2 is doing the repair calculation with fix-point math (multiply everything with 256).
            // It is calculating what fraction 2 hitpoints of the maximum health would be.
            const auto     fraction = (2 * 256) / getMaxHealth(context.game);
            const FixPoint repairprice =
                FixPoint(fraction * context.game.objectData.data[itemID][static_cast<int>(originalHouseID)].price) /
                256;

            // Original dune is always repairing 5 hitpoints (for the costs of 2) but we are only repairing 1/30th of
            // that
            const auto repairHealth = 5_fix / 30_fix;
            owner->takeCredits(repairprice / 30);
            const auto newHealth = getHealth() + repairHealth;
            if(newHealth >= getMaxHealth(context.game)) {
                setHealth(context.game, getMaxHealth(context.game));
                repairing = false;
            } else {
                setHealth(context.game, newHealth);
            }
        } else {
            repairing = false;
        }
    } else if(owner->isAI() && (getHealth() < getMaxHealth(context.game) / 2)) {
        doRepair(context);
    }

    const auto game_cycle_count = context.game.getGameCycleCount();

    // check smoke
    smoke.erase(std::remove_if(std::begin(smoke), std::end(smoke),
                               [game_cycle_count](const StructureSmoke& s) {
                                   return game_cycle_count - s.startGameCycle >= MILLI2CYCLES(8 * 1000);
                               }),
                std::end(smoke));

    if(justPlacedTimer > 0) justPlacedTimer--;

    return true;
}

void StructureBase::destroy(const GameContext& context) {
    const int*       pDestroyedStructureTiles      = nullptr;
    int              DestroyedStructureTilesSizeY  = 0;
    static const int DestroyedStructureTilesWall[] = {DestroyedStructure_Wall};
    static const int DestroyedStructureTiles1x1[]  = {Destroyed1x1Structure};
    static const int DestroyedStructureTiles2x2[]  = {Destroyed2x2Structure_TopLeft, Destroyed2x2Structure_TopRight,
                                                     Destroyed2x2Structure_BottomLeft,
                                                     Destroyed2x2Structure_BottomRight};
    static const int DestroyedStructureTiles3x2[]  = {
        Destroyed3x2Structure_TopLeft,    Destroyed3x2Structure_TopCenter,    Destroyed3x2Structure_TopRight,
        Destroyed3x2Structure_BottomLeft, Destroyed3x2Structure_BottomCenter, Destroyed3x2Structure_BottomRight};
    static const int DestroyedStructureTiles3x3[] = {
        Destroyed3x3Structure_TopLeft,    Destroyed3x3Structure_TopCenter,    Destroyed3x3Structure_TopRight,
        Destroyed3x3Structure_CenterLeft, Destroyed3x3Structure_CenterCenter, Destroyed3x3Structure_CenterRight,
        Destroyed3x3Structure_BottomLeft, Destroyed3x3Structure_BottomCenter, Destroyed3x3Structure_BottomRight};

    if(itemID == Structure_Wall) {
        pDestroyedStructureTiles     = DestroyedStructureTilesWall;
        DestroyedStructureTilesSizeY = 1;
    } else {
        switch(getStructureSizeY()) {
            case 1: {
                pDestroyedStructureTiles     = DestroyedStructureTiles1x1;
                DestroyedStructureTilesSizeY = 1;
            } break;

            case 2: {
                if(getStructureSizeX() == 2) {
                    pDestroyedStructureTiles     = DestroyedStructureTiles2x2;
                    DestroyedStructureTilesSizeY = 2;
                } else if(getStructureSizeX() == 3) {
                    pDestroyedStructureTiles     = DestroyedStructureTiles3x2;
                    DestroyedStructureTilesSizeY = 3;
                } else {
                    THROW(std::runtime_error, "StructureBase::destroy(): Invalid structure size");
                }
            } break;

            case 3: {
                pDestroyedStructureTiles     = DestroyedStructureTiles3x3;
                DestroyedStructureTilesSizeY = 3;
            } break;

            default: {
                THROW(std::runtime_error, "StructureBase::destroy(): Invalid structure size");
            } break;
        }
    }

    const auto& [game, map, objectManager] = context;

    if(itemID != Structure_Wall) {
        for(int j = 0; j < getStructureSizeY(); j++) {
            for(int i = 0; i < getStructureSizeX(); i++) {
                auto* pTile = map.getTile(location.x + i, location.y + j);
                pTile->setDestroyedStructureTile(pDestroyedStructureTiles[DestroyedStructureTilesSizeY * j + i]);

                Coord position((location.x + i) * TILESIZE + TILESIZE / 2, (location.y + j) * TILESIZE + TILESIZE / 2);
                uint32_t explosionID = game.randomGen.getRandOf(Explosion_Large1, Explosion_Large2);
                game.addExplosion(explosionID, position, owner->getHouseID());

                if(game.randomGen.rand(1, 100) <= getInfSpawnProp(game)) {
                    auto* pNewUnit = owner->createUnit(Unit_Soldier);
                    pNewUnit->setHealth(game, pNewUnit->getMaxHealth(game) / 2);
                    pNewUnit->deploy(context, location + Coord(i, j));
                }
            }
        }
    }

    objectManager.removeObject(getObjectID());
}

Coord StructureBase::getClosestPoint(const Coord& objectLocation) const {
    Coord closestPoint;

    // find the closest tile of a structure from a location
    if(objectLocation.x <= location.x) {
        // if we are left of the structure
        // set destination, left most point
        closestPoint.x = location.x;
    } else if(objectLocation.x >= (location.x + getStructureSizeX() - 1)) {
        // vice versa
        closestPoint.x = location.x + getStructureSizeX() - 1;
    } else {
        // we are above or below at least one tile of the structure, closest path is straight
        closestPoint.x = objectLocation.x;
    }

    // same deal but with y
    if(objectLocation.y <= location.y) {
        closestPoint.y = location.y;
    } else if(objectLocation.y >= (location.y + getStructureSizeY() - 1)) {
        closestPoint.y = location.y + getStructureSizeY() - 1;
    } else {
        closestPoint.y = objectLocation.y;
    }

    return closestPoint;
}

} // namespace Dune::Engine

