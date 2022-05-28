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

#include <globals.h>

#include <Explosion.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

#include <players/HumanPlayer.h>

#include "FileClasses/GFXManager.h"
#include <misc/draw_util.h>

#include <units/UnitBase.h>

#include <GUI/ObjectInterfaces/DefaultStructureInterface.h>

StructureBase::StructureBase(const StructureBaseConstants& structure_constants, uint32_t objectID,
                             const ObjectInitializer& initializer)
    : ObjectBase(structure_constants, objectID, initializer), degradeTimer(MILLI2CYCLES(15 * 1000)) {
    StructureBase::init();
}

StructureBase::StructureBase(const StructureBaseConstants& structure_constants, uint32_t objectID,
                             const ObjectStreamInitializer& initializer)
    : ObjectBase(structure_constants, objectID, initializer) {
    StructureBase::init();

    auto& stream = initializer.stream();

    repairing        = stream.readBool();
    fogged           = stream.readBool();
    lastVisibleFrame = stream.readUint32();

    degradeTimer = stream.readSint32();

    size_t numSmoke = stream.readUint32();
    for (size_t i = 0; i < numSmoke; i++) {
        smoke.emplace_back(stream);
    }
}

void StructureBase::init() {
    justPlacedTimer = 0;

    lastVisibleFrame = firstAnimFrame = lastAnimFrame = curAnimFrame = 2;
    animationCounter                                                 = 0;

    dune::globals::structureList.push_back(this);
}

StructureBase::~StructureBase() = default;

void StructureBase::cleanup(const GameContext& context, HumanPlayer* humanPlayer) {
    try {
        context.map.removeObjectFromMap(getObjectID()); // no map point will reference now
        dune::globals::structureList.remove(this);
        owner_->decrementStructures(itemID_, location_);
    } catch (std::exception& e) {
        sdl2::log_info("StructureBase::cleanup(): %s", e.what());
    }

    parent::cleanup(context, humanPlayer);
}

void StructureBase::save(OutputStream& stream) const {
    ObjectBase::save(stream);

    stream.writeBool(repairing);
    stream.writeBool(fogged);
    stream.writeUint32(lastVisibleFrame);

    stream.writeSint32(degradeTimer);

    stream.writeUint32(smoke.size());
    for (const auto& structureSmoke : smoke) {
        structureSmoke.save(stream);
    }
}

void StructureBase::assignToMap(const GameContext& context, const Coord& pos) {
    auto* map = &context.map;

    auto bFoundNonConcreteTile = false;

    const auto& game = context.game;

    map->for_each(pos.x, pos.y, pos.x + getStructureSizeX(), pos.y + getStructureSizeY(), [&](Tile& t) {
        t.assignNonInfantryGroundObject(getObjectID());

        if (!t.isConcrete() && game.getGameInitSettings().getGameOptions().concreteRequired
            && (game.gameState != GameState::Start)) {
            bFoundNonConcreteTile = true;

            if ((itemID_ != Structure_Wall) && (itemID_ != Structure_ConstructionYard)) {
                setHealth(getHealth() - FixPoint(getMaxHealth()) / (2 * getStructureSizeX() * getStructureSizeY()));
            }
        }
        t.setType(context, Terrain_Rock);
        t.setOwner(getOwner()->getHouseID());

        setVisible(VIS_ALL, true);
        setActive(true);
        setRespondable(true);
    });

    map->viewMap(getOwner()->getHouseID(), pos, getViewRange());

    if (!bFoundNonConcreteTile && !game.getGameInitSettings().getGameOptions().structuresDegradeOnConcrete) {
        degradeTimer = -1;
    }
}

void StructureBase::blitToScreen() {
    const auto* const screenborder = dune::globals::screenborder.get();
    auto* const renderer           = dune::globals::renderer.get();
    const auto zoom                = dune::globals::currentZoomlevel;

    const auto index  = fogged ? lastVisibleFrame : curAnimFrame;
    const auto indexX = index % numImagesX_;
    const auto indexY = index / numImagesX_;

    const auto* const texture = graphic_[zoom];

    const auto dest   = calcSpriteDrawingRect(texture, screenborder->world2screenX(realX_),
                                              screenborder->world2screenY(realY_.toFloat()), numImagesX_, numImagesY_);
    const auto source = calcSpriteSourceRect(texture, indexX, numImagesX_, indexY, numImagesY_);

    Dune_RenderCopyF(renderer, texture, &source, &dest);

    if (!fogged) {
        const auto* const pSmokeTex =
            dune::globals::pGFXManager->getZoomedObjPic(ObjPic_Smoke, getOwner()->getHouseID(), zoom);
        auto smokeSource = calcSpriteSourceRect(pSmokeTex, 0, 3);
        for (const auto& structureSmoke : smoke) {
            const auto smokeDest = calcSpriteDrawingRect(
                pSmokeTex, screenborder->world2screenX(structureSmoke.realPos.x),
                screenborder->world2screenY(structureSmoke.realPos.y), 3, 1, HAlign::Center, VAlign::Bottom);
            const auto cycleDiff = dune::globals::currentGame->getGameCycleCount() - structureSmoke.startGameCycle;

            auto smokeFrame = static_cast<int>((cycleDiff / 25) % 4);
            if (smokeFrame == 3) {
                smokeFrame = 1;
            }

            smokeSource.x = smokeFrame * smokeSource.w;
            Dune_RenderCopyF(renderer, pSmokeTex, &smokeSource, &smokeDest);
        }
    }
}

std::unique_ptr<ObjectInterface> StructureBase::getInterfaceContainer(const GameContext& context) {
    if ((dune::globals::pLocalHouse == owner_) || (dune::globals::debug)) {
        return Widget::create<DefaultStructureInterface>(context, objectID_);
    }
    return Widget::create<DefaultObjectInterface>(context, objectID_);
}

void StructureBase::drawSelectionBox() {
    const auto* const screenborder = dune::globals::screenborder.get();
    auto* const renderer           = dune::globals::renderer.get();
    const auto zoom                = dune::globals::currentZoomlevel;

    auto* const texture = graphic_[zoom];

    SDL_FRect dest{};
    dest.x = screenborder->world2screenX(realX_);
    dest.y = screenborder->world2screenY(realY_);
    dest.w = getWidth(texture) / static_cast<float>(numImagesX_);
    dest.h = getHeight(texture) / static_cast<float>(numImagesY_);

    // now draw the selection box thing, with parts at all corners of structure
    DuneDrawSelectionBox(renderer, dest);

    // health bar
    const SDL_FRect healthRect{
        dest.x, dest.y - static_cast<float>(zoom) - 2,
        ((getHealth() / getMaxHealth()) * (world2zoomedWorld(TILESIZE) * getStructureSizeX() - 1)).toFloat(),
        static_cast<float>(zoom + 1)};
    renderFillRectF(renderer, &healthRect, getHealthColor());
}

void StructureBase::drawOtherPlayerSelectionBox() {
    const auto* const screenborder = dune::globals::screenborder.get();
    auto* const renderer           = dune::globals::renderer.get();
    const auto zoom                = dune::globals::currentZoomlevel;

    SDL_FRect dest{};
    dest.x = screenborder->world2screenX(realX_) + static_cast<float>(zoom + 1);
    dest.y = screenborder->world2screenY(realY_) + static_cast<float>(zoom + 1);
    dest.w = getWidth(graphic_[zoom]) / static_cast<float>(numImagesX_) - static_cast<float>(2 * (zoom + 1));
    dest.h = getHeight(graphic_[zoom]) / static_cast<float>(numImagesY_) - static_cast<float>(2 * (zoom + 1));

    // now draw the selection box thing, with parts at all corners of structure
    DuneDrawSelectionBox(renderer, dest, COLOR_LIGHTBLUE);
}

void StructureBase::drawGatheringPointLine() {
    if (!isABuilder() || getItemID() == Structure_ConstructionYard || !destination_.isValid()
        || getOwner() != dune::globals::pLocalHouse)
        return;

    const auto* const screenborder = dune::globals::screenborder.get();
    auto* const renderer           = dune::globals::renderer.get();

    const auto indicatorPosition = destination_ * TILESIZE + Coord(TILESIZE / 2, TILESIZE / 2);
    const auto structurePosition = getCenterPoint();

    renderDrawLineF(renderer, screenborder->world2screenX(structurePosition.x),
                    screenborder->world2screenY(structurePosition.y), screenborder->world2screenX(indicatorPosition.x),
                    screenborder->world2screenY(indicatorPosition.y), COLOR_HALF_TRANSPARENT);

    const auto* const pUIIndicator = dune::globals::pGFXManager->getUIGraphic(UI_Indicator);
    const auto source              = calcSpriteSourceRect(pUIIndicator, 0, 3);
    const auto drawLocation =
        calcSpriteDrawingRect(pUIIndicator, screenborder->world2screenX(indicatorPosition.x),
                              screenborder->world2screenY(indicatorPosition.y), 3, 1, HAlign::Center, VAlign::Center);

    // Render twice
    Dune_RenderCopyF(renderer, pUIIndicator, &source, &drawLocation);
    Dune_RenderCopyF(renderer, pUIIndicator, &source, &drawLocation);
}

/**
    Returns the center point of this structure
    \return the center point in world coordinates
*/
Coord StructureBase::getCenterPoint() const {
    return {lround(realX_ + getStructureSizeX() * TILESIZE / 2), lround(realY_ + getStructureSizeY() * TILESIZE / 2)};
}

Coord StructureBase::getClosestCenterPoint(const Coord& objectLocation) const {
    return getClosestPoint(objectLocation) * TILESIZE + Coord(TILESIZE / 2, TILESIZE / 2);
}

void StructureBase::handleActionClick(const GameContext& context, int xPos, int yPos) {
    const auto* const local_player = dune::globals::pLocalPlayer;

    auto& command_manager = context.game.getCommandManager();

    if ((xPos < location_.x) || (xPos >= (location_.x + getStructureSizeX())) || (yPos < location_.y)
        || (yPos >= (location_.y + getStructureSizeY()))) {
        command_manager.addCommand(Command(local_player->getPlayerID(), CMDTYPE::CMD_STRUCTURE_SETDEPLOYPOSITION,
                                           objectID_, static_cast<uint32_t>(xPos), static_cast<uint32_t>(yPos)));
    } else {
        command_manager.addCommand(Command(local_player->getPlayerID(), CMDTYPE::CMD_STRUCTURE_SETDEPLOYPOSITION,
                                           objectID_, static_cast<uint32_t>(NONE_ID), static_cast<uint32_t>(NONE_ID)));
    }
}

void StructureBase::handleRepairClick() {
    dune::globals::currentGame->getCommandManager().addCommand(
        Command(dune::globals::pLocalPlayer->getPlayerID(), CMDTYPE::CMD_STRUCTURE_REPAIR, objectID_));
}

void StructureBase::doSetDeployPosition(int xPos, int yPos) {
    setTarget(nullptr);
    setDestination(xPos, yPos);
    setForced(true);
}

void StructureBase::doRepair([[maybe_unused]] const GameContext& context) {
    repairing = true;
}

void StructureBase::setDestination(int newX, int newY) {
    if (dune::globals::currentGameMap->tileExists(newX, newY) || ((newX == INVALID_POS) && (newY == INVALID_POS))) {
        destination_.x = newX;
        destination_.y = newY;
    }
}

void StructureBase::setJustPlaced() {
    justPlacedTimer  = 6;
    curAnimFrame     = 0;
    animationCounter = -STRUCTURE_ANIMATIONTIMER; // make first build animation double as long
}

bool StructureBase::update(const GameContext& context) {
    if (((context.game.getGameCycleCount() + getObjectID()) % 512) == 0) {
        context.map.viewMap(owner_->getHouseID(), location_, getViewRange());
    }

    if (!fogged) {
        lastVisibleFrame = curAnimFrame;
    }

    // degrade
    if ((degradeTimer >= 0) && context.game.getGameInitSettings().getGameOptions().concreteRequired
        && (owner_->getPowerRequirement() > owner_->getProducedPower())) {
        degradeTimer--;
        if (degradeTimer <= 0) {
            degradeTimer = MILLI2CYCLES(15 * 1000);

            int damageMultiplier = 1;
            if (owner_->getHouseID() == HOUSETYPE::HOUSE_HARKONNEN
                || owner_->getHouseID() == HOUSETYPE::HOUSE_SARDAUKAR) {
                damageMultiplier = 3;
            } else if (owner_->getHouseID() == HOUSETYPE::HOUSE_ORDOS) {
                damageMultiplier = 2;
            } else if (owner_->getHouseID() == HOUSETYPE::HOUSE_MERCENARY) {
                damageMultiplier = 5;
            }

            if (getHealth() > getMaxHealth() / 2) {
                setHealth(getHealth() - FixPoint(damageMultiplier * getMaxHealth()) / 100);
            }
        }
    }

    updateStructureSpecificStuff(context);

    if (getHealth() <= 0) {
        destroy(context);
        return false;
    }

    if (repairing) {
        if (owner_->getCredits() >= 5) {
            // Original dune 2 is doing the repair calculation with fix-point math (multiply everything with 256).
            // It is calculating what fraction 2 hitpoints of the maximum health would be.
            const auto fraction     = (2 * 256) / getMaxHealth();
            const auto object_price = context.game.objectData.data[itemID_][static_cast<int>(originalHouseID_)].price;

            const auto repairprice = FixPoint(fraction * object_price) / 256;

            // Original dune is always repairing 5 hitpoints (for the costs of 2) but we are only repairing 1/30th of
            // that
            const auto repairHealth = 5_fix / 30_fix;
            owner_->takeCredits(repairprice / 30);
            const auto newHealth = getHealth() + repairHealth;
            if (newHealth >= getMaxHealth()) {
                setHealth(getMaxHealth());
                repairing = false;
            } else {
                setHealth(newHealth);
            }
        } else {
            repairing = false;
        }
    } else if (owner_->isAI() && (getHealth() < getMaxHealth() / 2)) {
        doRepair(context);
    }

    const auto game_cycle_count = context.game.getGameCycleCount();

    // check smoke
    std::erase_if(smoke, [game_cycle_count](const StructureSmoke& s) {
        return game_cycle_count - s.startGameCycle >= MILLI2CYCLES(8 * 1000);
    });

    // update animations
    animationCounter++;
    if (animationCounter > STRUCTURE_ANIMATIONTIMER) {
        animationCounter = 0;
        curAnimFrame++;
        if ((curAnimFrame < firstAnimFrame) || (curAnimFrame > lastAnimFrame)) {
            curAnimFrame = firstAnimFrame;
        }

        justPlacedTimer--;
        if ((justPlacedTimer > 0) && (justPlacedTimer % 2 == 0)) {
            curAnimFrame = 0;
        }
    }

    return true;
}

void StructureBase::destroy(const GameContext& context) {
    const int* pDestroyedStructureTiles                = nullptr;
    int DestroyedStructureTilesSizeY                   = 0;
    static constexpr int DestroyedStructureTilesWall[] = {DestroyedStructure_Wall};
    static constexpr int DestroyedStructureTiles1x1[]  = {Destroyed1x1Structure};
    static constexpr int DestroyedStructureTiles2x2[]  = {Destroyed2x2Structure_TopLeft, Destroyed2x2Structure_TopRight,
                                                          Destroyed2x2Structure_BottomLeft,
                                                          Destroyed2x2Structure_BottomRight};
    static constexpr int DestroyedStructureTiles3x2[]  = {
         Destroyed3x2Structure_TopLeft,    Destroyed3x2Structure_TopCenter,    Destroyed3x2Structure_TopRight,
         Destroyed3x2Structure_BottomLeft, Destroyed3x2Structure_BottomCenter, Destroyed3x2Structure_BottomRight};
    static constexpr int DestroyedStructureTiles3x3[] = {
        Destroyed3x3Structure_TopLeft,    Destroyed3x3Structure_TopCenter,    Destroyed3x3Structure_TopRight,
        Destroyed3x3Structure_CenterLeft, Destroyed3x3Structure_CenterCenter, Destroyed3x3Structure_CenterRight,
        Destroyed3x3Structure_BottomLeft, Destroyed3x3Structure_BottomCenter, Destroyed3x3Structure_BottomRight};

    if (itemID_ == Structure_Wall) {
        pDestroyedStructureTiles     = DestroyedStructureTilesWall;
        DestroyedStructureTilesSizeY = 1;
    } else {
        switch (getStructureSizeY()) {
            case 1: {
                pDestroyedStructureTiles     = DestroyedStructureTiles1x1;
                DestroyedStructureTilesSizeY = 1;
            } break;

            case 2: {
                if (getStructureSizeX() == 2) {
                    pDestroyedStructureTiles     = DestroyedStructureTiles2x2;
                    DestroyedStructureTilesSizeY = 2;
                } else if (getStructureSizeX() == 3) {
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
            }
        }
    }

    auto& [game, map, objectManager] = context;

    if (itemID_ != Structure_Wall) {
        for (int j = 0; j < getStructureSizeY(); j++) {
            for (int i = 0; i < getStructureSizeX(); i++) {
                auto* const pTile = map.getTile(location_.x + i, location_.y + j);
                pTile->setDestroyedStructureTile(pDestroyedStructureTiles[DestroyedStructureTilesSizeY * j + i]);

                Coord position((location_.x + i) * TILESIZE + TILESIZE / 2,
                               (location_.y + j) * TILESIZE + TILESIZE / 2);
                uint32_t explosionID = game.randomGen.getRandOf(Explosion_Large1, Explosion_Large2);
                game.addExplosion(explosionID, position, owner_->getHouseID());

                if (game.randomGen.rand(1, 100) <= getInfSpawnProp()) {
                    auto* pNewUnit = owner_->createUnit(Unit_Soldier);
                    pNewUnit->setHealth(pNewUnit->getMaxHealth() / 2);
                    pNewUnit->deploy(context, location_ + Coord(i, j));
                }
            }
        }
    }

    objectManager.removeObject(getObjectID());

    if (isVisible(dune::globals::pLocalHouse->getTeamID()))
        dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_ExplosionStructure, location_);
}

Coord StructureBase::getClosestPoint(const Coord& objectLocation) const {
    Coord closestPoint;

    // find the closest tile of a structure from a location
    if (objectLocation.x <= location_.x) {
        // if we are left of the structure
        // set destination, left most point
        closestPoint.x = location_.x;
    } else if (objectLocation.x >= (location_.x + getStructureSizeX() - 1)) {
        // vica versa
        closestPoint.x = location_.x + getStructureSizeX() - 1;
    } else {
        // we are above or below at least one tile of the structure, closest path is straight
        closestPoint.x = objectLocation.x;
    }

    // same deal but with y
    if (objectLocation.y <= location_.y) {
        closestPoint.y = location_.y;
    } else if (objectLocation.y >= (location_.y + getStructureSizeY() - 1)) {
        closestPoint.y = location_.y + getStructureSizeY() - 1;
    } else {
        closestPoint.y = objectLocation.y;
    }

    return closestPoint;
}
