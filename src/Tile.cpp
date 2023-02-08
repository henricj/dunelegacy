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

#include <Tile.h>

#include "units/Soldier.h"
#include "units/Trike.h"

#include <globals.h>

#include <FileClasses/GFXManager.h>

#include <Explosion.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>
#include <sand.h>

#include <structures/StructureBase.h>
#include <units/AirUnit.h>
#include <units/InfantryBase.h>

namespace {
inline constexpr auto FOGTIME = MILLI2CYCLES(10 * 1000);
}

Tile::Tile() : sprite_{dune::globals::pGFXManager->getObjPic(ObjPic_Terrain)} { }

Tile::~Tile() = default;

void Tile::load(InputStream& stream) {
    type_ = static_cast<TERRAINTYPE>(stream.readUint32());

    stream.readBools(&explored_[0], &explored_[1], &explored_[2], &explored_[3], &explored_[4], &explored_[5],
                     &explored_[6]);

    std::array<bool, NUM_TEAMS> bLastAccess{};
    stream.readBools(&bLastAccess[0], &bLastAccess[1], &bLastAccess[2], &bLastAccess[3], &bLastAccess[4],
                     &bLastAccess[5], &bLastAccess[6]);

    for (int i = 0; i < NUM_TEAMS; i++) {
        if (bLastAccess[i]) {
            lastAccess_[i] = stream.readUint32();
        }
    }

    fogColor_ = stream.readUint32();

    owner_      = static_cast<HOUSETYPE>(stream.readSint32());
    sandRegion_ = stream.readUint32();

    spice_ = stream.readFixPoint();

    bool bHasDamage = false;

    bool bHasDeadUnits = false;

    bool bHasAirUnits = false;

    bool bHasInfantry = false;

    bool bHasUndergroundUnits = false;

    bool bHasNonInfantryGroundObjects = false;
    stream.readBools(&bHasDamage, &bHasDeadUnits, &bHasAirUnits, &bHasInfantry, &bHasUndergroundUnits,
                     &bHasNonInfantryGroundObjects);

    if (bHasDamage) {
        damage_.clear();
        const uint32_t numDamage = stream.readUint32();
        damage_.reserve(numDamage);
        for (uint32_t i = 0; i < numDamage; i++) {
            DAMAGETYPE newDamage;
            newDamage.damageType_ = static_cast<TerrainDamage_enum>(stream.readUint32());
            newDamage.tile_       = stream.readSint32();
            newDamage.realPos_.x  = stream.readSint32();
            newDamage.realPos_.y  = stream.readSint32();

            damage_.push_back(newDamage);
        }
    }

    if (bHasDeadUnits) {
        deadUnits_.clear();
        const uint32_t numDeadUnits = stream.readUint32();
        deadUnits_.reserve(numDeadUnits);
        for (uint32_t i = 0; i < numDeadUnits; i++) {
            DEADUNITTYPE newDeadUnit;
            newDeadUnit.type      = static_cast<deadUnitEnum>(stream.readUint8());
            newDeadUnit.house     = static_cast<HOUSETYPE>(stream.readUint8());
            newDeadUnit.onSand    = stream.readBool();
            newDeadUnit.realPos.x = static_cast<float>(stream.readSint32());
            newDeadUnit.realPos.y = static_cast<float>(stream.readSint32());
            newDeadUnit.timer     = stream.readSint16();

            deadUnits_.push_back(newDeadUnit);
        }
    }

    destroyedStructureTile_ = stream.readSint32();

    bool bTrackCounter[NUM_ANGLES]{};
    stream.readBools(&bTrackCounter[0], &bTrackCounter[1], &bTrackCounter[2], &bTrackCounter[3], &bTrackCounter[4],
                     &bTrackCounter[5], &bTrackCounter[6], &bTrackCounter[7]);

    for (int i = 0; i < NUM_ANGLES; i++) {
        if (bTrackCounter[i]) {
            tracksCreationTime_[i] = stream.readUint32();
        }
    }

    if (bHasAirUnits) {
        stream.readUint32Vector(assignedAirUnitList_);
    }

    if (bHasInfantry) {
        stream.readUint32Vector(assignedInfantryList_);
    }

    if (bHasUndergroundUnits) {
        stream.readUint32Vector(assignedUndergroundUnitList_);
    }

    if (bHasNonInfantryGroundObjects) {
        stream.readUint32Vector(assignedNonInfantryGroundObjectList_);
    }
}

void Tile::save(OutputStream& stream, uint32_t gameCycleCount) const {
    stream.writeUint32(static_cast<Uint32>(type_));

    stream.writeBools(explored_[0], explored_[1], explored_[2], explored_[3], explored_[4], explored_[5], explored_[6]);

    stream.writeBools((lastAccess_[0] != 0), (lastAccess_[1] != 0), (lastAccess_[2] != 0), (lastAccess_[3] != 0),
                      (lastAccess_[4] != 0), (lastAccess_[5] != 0), (lastAccess_[6] != 0));
    for (const auto lastAccessFromTeam : lastAccess_) {
        if (lastAccessFromTeam != 0) {
            stream.writeUint32(lastAccessFromTeam);
        }
    }

    stream.writeUint32(fogColor_);

    stream.writeUint32(static_cast<uint32_t>(owner_));
    stream.writeUint32(sandRegion_);

    stream.writeFixPoint(spice_);

    stream.writeBools(!damage_.empty(), !deadUnits_.empty(), !assignedAirUnitList_.empty(),
                      !assignedInfantryList_.empty(), !assignedUndergroundUnitList_.empty(),
                      !assignedNonInfantryGroundObjectList_.empty());

    if (!damage_.empty()) {
        stream.writeUint32(damage_.size());
        for (const auto& damageItem : damage_) {
            stream.writeUint32(static_cast<uint32_t>(damageItem.damageType_));
            stream.writeSint32(damageItem.tile_);
            stream.writeSint32(damageItem.realPos_.x);
            stream.writeSint32(damageItem.realPos_.y);
        }
    }

    if (!deadUnits_.empty()) {
        stream.writeUint32(deadUnits_.size());
        for (const auto& deadUnit : deadUnits_) {
            stream.writeUint8(deadUnit.type);
            stream.writeUint8(static_cast<uint8_t>(deadUnit.house));
            stream.writeBool(deadUnit.onSand);
            stream.writeSint32(static_cast<int32_t>(deadUnit.realPos.x));
            stream.writeSint32(static_cast<int32_t>(deadUnit.realPos.y));
            stream.writeSint16(deadUnit.timer);
        }
    }

    stream.writeSint32(destroyedStructureTile_);

    // clean-up tracksCreationTime to save space in the save game
    std::array<uint32_t, NUM_ANGLES> tracksCreationTimeToSave{};
    for (auto i = 0U; i < tracksCreationTimeToSave.size(); ++i) {
        tracksCreationTimeToSave[i] =
            (tracksCreationTime_[i] + TRACKSTIME < gameCycleCount) ? 0 : tracksCreationTime_[i];
    }

    stream.writeBools((tracksCreationTimeToSave[0] != 0), (tracksCreationTimeToSave[1] != 0),
                      (tracksCreationTimeToSave[2] != 0), (tracksCreationTimeToSave[3] != 0),
                      (tracksCreationTimeToSave[4] != 0), (tracksCreationTimeToSave[5] != 0),
                      (tracksCreationTimeToSave[6] != 0), (tracksCreationTimeToSave[7] != 0));
    for (const auto i : tracksCreationTimeToSave) {
        if (i != 0) {
            stream.writeUint32(i);
        }
    }

    if (!assignedAirUnitList_.empty()) {
        stream.writeUint32Vector(assignedAirUnitList_);
    }

    if (!assignedInfantryList_.empty()) {
        stream.writeUint32Vector(assignedInfantryList_);
    }

    if (!assignedUndergroundUnitList_.empty()) {
        stream.writeUint32Vector(assignedUndergroundUnitList_);
    }

    if (!assignedNonInfantryGroundObjectList_.empty()) {
        stream.writeUint32Vector(assignedNonInfantryGroundObjectList_);
    }
}

void Tile::assignAirUnit(uint32_t newObjectID) {
    assignedAirUnitList_.push_back(newObjectID);
}

void Tile::assignDeadUnit(deadUnitEnum type, HOUSETYPE house, CoordF position) {
#if HAVE_PARENTHESIZED_INITIALIZATION_OF_AGGREGATES
    deadUnits_.emplace_back(position, static_cast<uint16_t>(2000), type, house, isSand() || isDunes());
#else
    deadUnits_.push_back({position, static_cast<uint16_t>(2000), type, house, isSand() || isDunes()});
#endif
}

void Tile::assignNonInfantryGroundObject(uint32_t newObjectID) {
    assignedNonInfantryGroundObjectList_.push_back(newObjectID);
}

int Tile::assignInfantry(ObjectManager& objectManager, uint32_t newObjectID, int8_t currentPosition) {
    auto newPosition = currentPosition;

    if (currentPosition < 0) {
        std::array<bool, NUM_INFANTRY_PER_TILE> used{};

        for (const auto objectID : assignedInfantryList_) {
            const auto* const pInfantry = dynamic_cast<InfantryBase*>(objectManager.getObject(objectID));
            if (pInfantry == nullptr) {
                continue;
            }

            const auto pos = pInfantry->getTilePosition();
            if ((pos >= 0) && (pos < NUM_INFANTRY_PER_TILE)) {
                used[pos] = true;
            }
        }

        for (newPosition = 0; newPosition < NUM_INFANTRY_PER_TILE; newPosition++) {
            if (!used[newPosition]) {
                break;
            }
        }

        newPosition =
            std::max(static_cast<int8_t>(0), std::min(newPosition, static_cast<int8_t>(NUM_INFANTRY_PER_TILE)));
    }

    assignedInfantryList_.push_back(newObjectID);
    return newPosition;
}

void Tile::assignUndergroundUnit(uint32_t newObjectID) {
    assignedUndergroundUnitList_.push_back(newObjectID);
}

void Tile::blitGround(Game* game) {
    if (hasANonInfantryGroundObject() && getNonInfantryGroundObject(game->getObjectManager())->isAStructure())
        return;

    const auto* const gfx          = dune::globals::pGFXManager.get();
    const auto* const screenborder = dune::globals::screenborder.get();
    auto* const renderer           = dune::globals::renderer.get();
    const auto zoom                = dune::globals::currentZoomlevel;

    const auto tileIndex       = static_cast<int>(getTerrainTile());
    const auto indexX          = tileIndex % NUM_TERRAIN_TILES_X;
    const auto indexY          = tileIndex / NUM_TERRAIN_TILES_X;
    const auto zoomed_tilesize = world2zoomedWorld(TILESIZE);
    SDL_Rect source{indexX * zoomed_tilesize, indexY * zoomed_tilesize, zoomed_tilesize, zoomed_tilesize};

    const SDL_FRect pos{screenborder->world2screenX(getLocation().x * TILESIZE),
                        screenborder->world2screenY(getLocation().y * TILESIZE), static_cast<float>(zoomed_tilesize),
                        static_cast<float>(zoomed_tilesize)};

    // draw terrain
    if (destroyedStructureTile_ == DestroyedStructure_None || destroyedStructureTile_ == DestroyedStructure_Wall) {
        Dune_RenderCopyF(renderer, sprite_[zoom], &source, &pos);
    }

    if (destroyedStructureTile_ != DestroyedStructure_None) {
        const auto* const pDestroyedStructureTex = gfx->getZoomedObjPic(ObjPic_DestroyedStructure, zoom);
        const SDL_Rect source2 = {destroyedStructureTile_ * zoomed_tilesize, 0, zoomed_tilesize, zoomed_tilesize};
        Dune_RenderCopyF(renderer, pDestroyedStructureTex, &source2, &pos);
    }

    if (isFoggedByTeam(game, dune::globals::pLocalHouse->getTeamID()))
        return;

    source.y = 0;

    const auto gameCycleCount = game->getGameCycleCount();

    source.y = 0;

    // tracks
    const auto* const pTracks = gfx->getZoomedObjPic(ObjPic_Terrain_Tracks, zoom);
    for (auto i = 0; i < NUM_ANGLES; i++) {
        const auto tracktime = static_cast<int>(gameCycleCount - tracksCreationTime_[i]);
        if ((tracksCreationTime_[i] != 0) && (tracktime < TRACKSTIME)) {
            source.x = ((10 - i) % 8) * zoomed_tilesize;
            SDL_SetTextureAlphaMod(pTracks->texture_,
                                   static_cast<Uint8>(std::min(255, 256 * (TRACKSTIME - tracktime) / TRACKSTIME)));
            Dune_RenderCopyF(renderer, pTracks, &source, &pos);
            SDL_SetTextureAlphaMod(pTracks->texture_, 255);
        }
    }

    // damage
    for (const auto& damageItem : damage_) {
        source.x = damageItem.tile_ * zoomed_tilesize;
        SDL_FRect dest{screenborder->world2screenX(damageItem.realPos_.x) - static_cast<float>(zoomed_tilesize) / 2.f,
                       screenborder->world2screenY(damageItem.realPos_.y) - static_cast<float>(zoomed_tilesize) / 2.f,
                       static_cast<float>(zoomed_tilesize), static_cast<float>(zoomed_tilesize)};

        if (damageItem.damageType_ == Tile::TerrainDamage_enum::Terrain_RockDamage) {
            auto* const texture = gfx->getZoomedObjPic(ObjPic_RockDamage, zoom);
            Dune_RenderCopyF(renderer, texture, &source, &dest);
        } else {
            auto* const texture = gfx->getZoomedObjPic(ObjPic_SandDamage, zoom);
            Dune_RenderCopyF(renderer, texture, &source, &pos);
        }
    }
}

void Tile::blitStructures(Game* game) const {
    if (!hasANonInfantryGroundObject())
        return;

    // if got a structure, draw the structure, and don't draw any terrain because wont be seen
    auto* pStructure = dune_cast<StructureBase>(getNonInfantryGroundObject(game->getObjectManager()));
    if (!pStructure)
        return;

    auto* map = game->getMap();

    const auto team_id = dune::globals::pLocalHouse->getTeamID();

    map->for_each(pStructure->getX(), pStructure->getY(), pStructure->getX() + pStructure->getStructureSizeX(),
                  pStructure->getY() + pStructure->getStructureSizeY(), [&](const auto& tile) {
                      if (dune::globals::screenborder->isTileInsideScreen(tile.location_)
                          && (tile.isExploredByTeam(game, team_id) || dune::globals::debug)) {
                          pStructure->setFogged(isFoggedByTeam(game, team_id));

                          if (&tile == this) {
                              // only this tile will draw it, so will be drawn only once
                              pStructure->blitToScreen();
                          }

                          return;
                      }
                  });
}

void Tile::blitUndergroundUnits(Game* game) const {
    if (!hasAnUndergroundUnit())
        return;

    const auto team_id = dune::globals::pLocalHouse->getTeamID();

    if (isFoggedByTeam(game, team_id))
        return;

    auto* current = getUndergroundUnit(game->getObjectManager());

    if (current->isVisible(team_id)) {
        if (location_ == current->getLocation()) {
            current->blitToScreen();
        }
    }
}

void Tile::blitDeadUnits(Game* game) {
    if (isFoggedByTeam(game, dune::globals::pLocalHouse->getTeamID()))
        return;

    const auto* const gfx          = dune::globals::pGFXManager.get();
    auto* const renderer           = dune::globals::renderer.get();
    const auto* const screenborder = dune::globals::screenborder.get();
    const auto zoom                = dune::globals::currentZoomlevel;

    const auto zoomed_tile = world2zoomedWorld(TILESIZE);

    for (const auto& deadUnit : deadUnits_) {
        SDL_Rect source{0, 0, zoomed_tile, zoomed_tile};
        const DuneTexture* pTexture = nullptr;
        switch (deadUnit.type) {
            case DeadUnit_Infantry: {
                pTexture = gfx->getZoomedObjPic(ObjPic_DeadInfantry, deadUnit.house, zoom);
                source.x = (deadUnit.timer < 1000 && deadUnit.onSand) ? zoomed_tile : 0;
            } break;

            case DeadUnit_Infantry_Squashed1: {
                pTexture = gfx->getZoomedObjPic(ObjPic_DeadInfantry, deadUnit.house, zoom);
                source.x = 4 * zoomed_tile;
            } break;

            case DeadUnit_Infantry_Squashed2: {
                pTexture = gfx->getZoomedObjPic(ObjPic_DeadInfantry, deadUnit.house, zoom);
                source.x = 5 * zoomed_tile;
            } break;

            case DeadUnit_Carryall: {
                pTexture = gfx->getZoomedObjPic(ObjPic_DeadAirUnit, deadUnit.house, zoom);
                if (deadUnit.onSand) {
                    source.x = (deadUnit.timer < 1000) ? 5 * zoomed_tile : 4 * zoomed_tile;
                } else {
                    source.x = 3 * zoomed_tile;
                }
            } break;

            case DeadUnit_Ornithopter: {
                pTexture = gfx->getZoomedObjPic(ObjPic_DeadAirUnit, deadUnit.house, zoom);
                if (deadUnit.onSand) {
                    source.x = (deadUnit.timer < 1000) ? 2 * zoomed_tile : zoomed_tile;
                } else {
                    source.x = 0;
                }
            } break;

            default: break;
        }

        if (pTexture != nullptr) {
            SDL_FRect dest{screenborder->world2screenX(deadUnit.realPos.x) - static_cast<float>(zoomed_tile) / 2.f,
                           screenborder->world2screenY(deadUnit.realPos.y) - static_cast<float>(zoomed_tile) / 2.f,
                           static_cast<float>(zoomed_tile), static_cast<float>(zoomed_tile)};

            Dune_RenderCopyF(renderer, pTexture, &source, &dest);
        }
    }
}

void Tile::blitInfantry(Game* game) {
    const auto team_id = dune::globals::pLocalHouse->getTeamID();

    if (isFoggedByTeam(game, team_id))
        return;

    for (const auto objectID : assignedInfantryList_) {
        auto* pInfantry = game->getObjectManager().getObject<InfantryBase>(objectID);
        if (pInfantry == nullptr) {
            continue;
        }

        if (pInfantry->isVisible(team_id)) {
            if (location_ == pInfantry->getLocation()) {
                pInfantry->blitToScreen();
            }
        }
    }
}

void Tile::blitNonInfantryGroundUnits(Game* game) {
    const auto team_id = dune::globals::pLocalHouse->getTeamID();

    if (isFoggedByTeam(game, team_id))
        return;

    for (const auto objectID : assignedNonInfantryGroundObjectList_) {
        auto* pObject = game->getObjectManager().getObject(objectID);
        if (pObject == nullptr)
            continue;

        if (pObject->isAUnit() && pObject->isVisible(team_id)) {
            if (location_ == pObject->getLocation()) {
                pObject->blitToScreen();
            }
        }
    }
}

void Tile::blitAirUnits(Game* game) {
    const auto* const player_house = dune::globals::pLocalHouse;

    const auto team_id   = player_house->getTeamID();
    const auto is_fogged = isFoggedByTeam(game, team_id);

    for (const auto objectID : assignedAirUnitList_) {
        auto* pAirUnit = game->getObjectManager().getObject<AirUnit>(objectID);
        if (pAirUnit == nullptr) {
            continue;
        }

        if (!is_fogged || pAirUnit->getOwner() == player_house) {
            if (pAirUnit->isVisible(team_id)) {
                if (location_ == pAirUnit->getLocation()) {
                    pAirUnit->blitToScreen();
                }
            }
        }
    }
}

void Tile::blitSelectionRects(Game* game) const {
    const auto team_id = dune::globals::pLocalHouse->getTeamID();

    if (isFoggedByTeam(game, team_id))
        return;

    const auto& object_manager = game->getObjectManager();

    forEachUnit([&](uint32_t objectID) {
        auto* pObject = object_manager.getObject(objectID);
        if (pObject == nullptr)
            return;

        // possibly draw selection rectangle multiple times, e.g. for structures
        if (pObject->isVisible(team_id)) {
            if (pObject->isSelected())
                pObject->drawSelectionBox();

            if (pObject->isSelectedByOtherPlayer())
                pObject->drawOtherPlayerSelectionBox();
        }
    });
}

void Tile::addDamage(Tile::TerrainDamage_enum damageType, int tile, Coord realPos) {
    if (damage_.size() >= DAMAGE_PER_TILE)
        return;

#if HAVE_PARENTHESIZED_INITIALIZATION_OF_AGGREGATES
    damage_.emplace_back(damageType, tile, realPos);
#else
    damage_.push_back({damageType, tile, realPos});
#endif
}

void Tile::update_impl() {
    std::erase_if(deadUnits_, [](DEADUNITTYPE& dut) {
        if (0 == dut.timer)
            return true;
        --dut.timer;
        return false;
    });
}

void Tile::clearTerrain() {
    damage_.clear();
    deadUnits_.clear();
}

void Tile::setTrack(ANGLETYPE direction, uint32_t gameCycleCounter) {
    if (type_ == TERRAINTYPE::Terrain_Sand || type_ == TERRAINTYPE::Terrain_Dunes || type_ == TERRAINTYPE::Terrain_Spice
        || type_ == TERRAINTYPE::Terrain_ThickSpice) {
        tracksCreationTime_[static_cast<int>(direction)] = gameCycleCounter;
    }
}

void Tile::selectAllPlayersUnits(Game* game, HOUSETYPE houseID, ObjectBase** lastCheckedObject,
                                 ObjectBase** lastSelectedObject) {
    selectFilter(game, houseID, lastCheckedObject, lastSelectedObject,
                 [](ObjectBase* obj) { return obj->isAUnit() && obj->isRespondable(); });
}

void Tile::selectAllPlayersUnitsOfType(Game* game, HOUSETYPE houseID, ItemID_enum itemID,
                                       ObjectBase** lastCheckedObject, ObjectBase** lastSelectedObject) {
    selectFilter(game, houseID, lastCheckedObject, lastSelectedObject,
                 [=](ObjectBase* obj) { return obj->getItemID() == itemID; });
}

template<typename Container, typename Val>
static void erase_remove(Container& c, Val val) {
    c.erase(std::remove(std::begin(c), std::end(c), val), std::end(c));
}

void Tile::unassignAirUnit(uint32_t objectID) {
    erase_remove(assignedAirUnitList_, objectID);
}

void Tile::unassignNonInfantryGroundObject(uint32_t objectID) {
    erase_remove(assignedNonInfantryGroundObjectList_, objectID);
}

void Tile::unassignUndergroundUnit(uint32_t objectID) {
    erase_remove(assignedUndergroundUnitList_, objectID);
}

void Tile::unassignInfantry(uint32_t objectID, [[maybe_unused]] int currentPosition) {
    erase_remove(assignedInfantryList_, objectID);
}

void Tile::unassignObject(uint32_t objectID) {
    if (hasInfantry())
        unassignInfantry(objectID, -1);
    if (hasAnUndergroundUnit())
        unassignUndergroundUnit(objectID);
    if (hasANonInfantryGroundObject())
        unassignNonInfantryGroundObject(objectID);
    if (hasAnAirUnit())
        unassignAirUnit(objectID);
}

void Tile::setType(const GameContext& context, TERRAINTYPE newType) {
    const auto& [game, map, objectManager] = context;

    type_                   = newType;
    destroyedStructureTile_ = DestroyedStructure_None;

    terrainTile_ = TERRAINTILETYPE::TerrainTile_Invalid;
    map.for_each_neighbor(location_.x, location_.y,
                          [](Tile& t) { t.terrainTile_ = TERRAINTILETYPE::TerrainTile_Invalid; });

    if (type_ == TERRAINTYPE::Terrain_Spice) {
        spice_ = game.randomGen.rand(RANDOMSPICEMIN, RANDOMSPICEMAX);
    } else if (type_ == TERRAINTYPE::Terrain_ThickSpice) {
        spice_ = game.randomGen.rand(RANDOMTHICKSPICEMIN, RANDOMTHICKSPICEMAX);
    } else if (type_ == TERRAINTYPE::Terrain_Dunes) {
    } else {
        spice_ = 0;

        if (isRock()) {
            std::vector<ObjectBase*> pending_destroy;

            sandRegion_ = NONE_ID;
            if (hasAnUndergroundUnit()) {
                const auto units = std::move(assignedUndergroundUnitList_);
                assignedUndergroundUnitList_.clear();

                for (const auto object_id : units) {
                    auto* const current = game.getObjectManager().getObject(object_id);

                    unassignUndergroundUnit(current->getObjectID());
                    current->destroy(context);
                }
            }

            if (type_ == TERRAINTYPE::Terrain_Mountain) {
                if (hasANonInfantryGroundObject()) {
                    auto units = std::move(assignedNonInfantryGroundObjectList_);
                    assignedNonInfantryGroundObjectList_.clear();

                    for (const auto object_id : units) {
                        auto* const object = game.getObjectManager().getObject(object_id);

                        if (object)
                            pending_destroy.push_back(object);
                        else
                            assignedNonInfantryGroundObjectList_.push_back(object_id);
                    }

                    // Try to keep the largest buffer.
                    if (assignedNonInfantryGroundObjectList_.empty()
                        && units.capacity() > assignedNonInfantryGroundObjectList_.capacity()) {
                        units.clear();
                        assignedNonInfantryGroundObjectList_ = std::move(units);
                    }
                }
            }

            std::ranges::for_each(pending_destroy, [&](ObjectBase* obj) { obj->destroy(context); });
        }
    }

    map.for_each(location_.x, location_.y, location_.x + 4, location_.y + 4, [](Tile& t) { t.clearTerrain(); });
}

void Tile::squash(const GameContext& context) const {
    if (!hasInfantry())
        return;

    for (const auto object_id : assignedInfantryList_) {
        auto* current = context.objectManager.getObject<InfantryBase>(object_id);

        if (current == nullptr)
            continue;

        current->squash(context);
    }
}

int Tile::getInfantryTeam(const ObjectManager& objectManager) const {
    int team = INVALID;
    if (hasInfantry()) {
        if (const auto* infantry = getInfantry(objectManager)) {
            if (const auto* owner = infantry->getOwner())
                team = owner->getTeamID();
        }
    }
    return team;
}

FixPoint Tile::harvestSpice(const GameContext& context) {
    const auto oldSpice = spice_;

    if ((spice_ - HARVESTSPEED) >= 0) {
        spice_ -= HARVESTSPEED;
    } else {
        spice_ = 0;
    }

    if (oldSpice >= RANDOMTHICKSPICEMIN && spice_ < RANDOMTHICKSPICEMIN) {
        setType(context, TERRAINTYPE::Terrain_Spice);
    }

    if (oldSpice > 0 && spice_ == 0) {
        setType(context, TERRAINTYPE::Terrain_Sand);
    }

    return (oldSpice - spice_);
}

void Tile::setSpice(FixPoint newSpice) {
    if (newSpice <= 0) {
        type_ = TERRAINTYPE::Terrain_Sand;
    } else if (newSpice >= RANDOMTHICKSPICEMIN) {
        type_ = TERRAINTYPE::Terrain_ThickSpice;
    } else {
        type_ = TERRAINTYPE::Terrain_Spice;
    }
    spice_ = newSpice;
}

AirUnit* Tile::getAirUnit(const ObjectManager& objectManager) const {
    return assignedAirUnitList_.empty() ? nullptr
                                        : dune_cast<AirUnit>(objectManager.getObject(assignedAirUnitList_.front()));
}

ObjectBase* Tile::getGroundObject(const ObjectManager& objectManager) const {
    if (hasANonInfantryGroundObject())
        return getNonInfantryGroundObject(objectManager);
    if (hasInfantry())
        return getInfantry(objectManager);

    return nullptr;
}

std::pair<bool, dune::object_id_type> Tile::getGroundObjectID() const {
    if (hasANonInfantryGroundObject())
        return {true, assignedNonInfantryGroundObjectList_.front()};
    if (hasInfantry())
        return {true, assignedInfantryList_.front()};

    return {false, 0};
}

InfantryBase* Tile::getInfantry(const ObjectManager& objectManager) const {
    return assignedInfantryList_.empty() ? nullptr
                                         : objectManager.getObject<InfantryBase>(assignedInfantryList_.front());
}

ObjectBase* Tile::getNonInfantryGroundObject(const ObjectManager& objectManager) const {
    return assignedNonInfantryGroundObjectList_.empty()
             ? nullptr
             : objectManager.getObject(assignedNonInfantryGroundObjectList_.front());
}

UnitBase* Tile::getUndergroundUnit(const ObjectManager& objectManager) const {
    return assignedUndergroundUnitList_.empty()
             ? nullptr
             : objectManager.getObject<UnitBase>(assignedUndergroundUnitList_.front());
}

/*ObjectBase* Tile::getInfantry(int i)
{
    int count;
    InfantryBase* infantry;
    assignedInfantry.reset();
    while (assignedInfantry.currentNotNull())
    {
        ((InfantryBase*)assignedInfantry.getCurrent())->squash();
        assignedInfantry.nextLink();
    }
    return assignedInfantry.removeElement();
}*/

ObjectBase* Tile::getObject(const ObjectManager& objectManager) const {
    if (hasAnAirUnit())
        return getAirUnit(objectManager);
    if (hasANonInfantryGroundObject())
        return getNonInfantryGroundObject(objectManager);
    if (hasInfantry())
        return getInfantry(objectManager);
    if (hasAnUndergroundUnit())
        return getUndergroundUnit(objectManager);

    return nullptr;
}

ObjectBase* Tile::getObjectAt(const ObjectManager& objectManager, int x, int y) const {
    ObjectBase* pObject = nullptr;
    if (hasAnAirUnit()) {
        pObject = getAirUnit(objectManager);
    } else if (hasANonInfantryGroundObject()) {
        pObject = getNonInfantryGroundObject(objectManager);
    } else if (hasInfantry()) {
        auto closestDistance = FixPt_MAX;
        const Coord atPos(x, y);

        for (const auto objectID : assignedInfantryList_) {
            auto* const pInfantry = objectManager.getObject<InfantryBase>(objectID);
            if (pInfantry == nullptr) {
                continue;
            }

            const auto centerPoint = pInfantry->getCenterPoint();
            const auto distance    = distanceFrom(atPos, centerPoint);
            if (distance < closestDistance) {
                closestDistance = distance;
                pObject         = pInfantry;
            }
        }
    } else if (hasAnUndergroundUnit()) {
        pObject = getUndergroundUnit(objectManager);
    }

    return pObject;
}

ObjectBase* Tile::getObjectWithID(const ObjectManager& objectManager, uint32_t objectID) const {
    const auto predicate = [=](uint32_t n) { return n == objectID; };

    if (std::ranges::any_of(assignedInfantryList_, predicate)
        || std::ranges::any_of(assignedNonInfantryGroundObjectList_, predicate)
        || std::ranges::any_of(assignedUndergroundUnitList_, predicate)
        || std::ranges::any_of(assignedAirUnitList_, predicate)) {
        return objectManager.getObject(objectID);
    }

    return nullptr;
}

void Tile::triggerSpiceBloom(const GameContext& context, House* pTrigger) {
    if (!isSpiceBloom())
        return;

    // a spice bloom
    dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_Bloom, getLocation());
    dune::globals::screenborder->shakeScreen(18);
    const auto* const house = dune::globals::pLocalHouse;
    if (pTrigger == house) {
        dune::globals::soundPlayer->playVoice(Voice_enum::BloomLocated, house->getHouseID());
    }

    setType(context, TERRAINTYPE::Terrain_Spice); // Set this tile to spice first
    context.map.createSpiceField(context, location_, 5);

    const auto realLocation = location_ * TILESIZE + Coord(TILESIZE / 2, TILESIZE / 2);

    if (damage_.size() < DAMAGE_PER_TILE) {
        DAMAGETYPE newDamage;
        newDamage.tile_       = static_cast<int>(SANDDAMAGETYPE::SandDamage1);
        newDamage.damageType_ = TerrainDamage_enum::Terrain_SandDamage;
        newDamage.realPos_    = realLocation;

        damage_.push_back(newDamage);
    }

    context.game.addExplosion(Explosion_SpiceBloom, realLocation, pTrigger->getHouseID());
}

void Tile::triggerSpecialBloom(const GameContext& context, House* pTrigger) {
    if (!isSpecialBloom())
        return;

    setType(context, TERRAINTYPE::Terrain_Sand);

    const auto& [game, map, objectManager] = context;

    switch (game.randomGen.rand(0, 3)) {
        case 0: {
            // the player gets an randomly chosen amount of credits between 150 and 400
            pTrigger->addCredits(game.randomGen.rand(150, 400), false);
        } break;

        case 1: {
            // The house gets a Trike for free. It spawns beside the special bloom.
            auto* pNewUnit = pTrigger->createUnit<Trike>();
            if (pNewUnit != nullptr) {
                const auto spot = map.findDeploySpot(pNewUnit, location_);
                pNewUnit->deploy(context, spot);
            }
        } break;

        case 2: {
            // One of the AI players on the map (one that has at least one unit) gets a Trike for free. It spawns beside
            // the special bloom.
            int numCandidates = 0;
            game.for_each_house([&](const auto& house) {
                if (house.getTeamID() != pTrigger->getTeamID() && house.getNumUnits() > 0)
                    numCandidates++;
            });

            if (numCandidates == 0)
                break;

            int candidate = game.randomGen.rand(0, numCandidates - 1);

            auto* pEnemyHouse = game.house_find_if([&](auto& house) {
                if (house.getTeamID() != pTrigger->getTeamID() && house.getNumUnits() > 0) {
                    if (candidate-- == 0)
                        return true;
                    candidate--;
                }
                return false;
            });

            if (!pEnemyHouse)
                break;

            auto* const pNewUnit = pEnemyHouse->createUnit<Trike>();
            if (pNewUnit != nullptr) {
                const auto spot = map.findDeploySpot(pNewUnit, location_);
                pNewUnit->deploy(context, spot);
            }
        } break;

        case 3:
        default: {
            // One of the AI players on the map (one that has at least one unit) gets an Infantry unit (3 Soldiers) for
            // free. The spawn beside the special bloom.
            auto numCandidates = 0;
            game.for_each_house([&](auto& house) {
                if (house.getTeamID() != pTrigger->getTeamID() && house.getNumUnits() > 0) {
                    numCandidates++;
                }
            });

            if (numCandidates == 0) {
                break;
            }

            auto candidate = game.randomGen.rand(0, numCandidates - 1);

            House* pEnemyHouse = nullptr;
            for (auto i = 0; i < NUM_HOUSES; i++) {
                auto* const pHouse = game.getHouse(static_cast<HOUSETYPE>(i));
                if (pHouse != nullptr && pHouse->getTeamID() != pTrigger->getTeamID() && pHouse->getNumUnits() > 0) {
                    if (candidate == 0) {
                        pEnemyHouse = pHouse;
                        break;
                    }
                    candidate--;
                }
            }

            if (pEnemyHouse) {
                for (int i = 0; i < 3; i++) {
                    auto* const pNewUnit = pEnemyHouse->createUnit<Soldier>();
                    if (pNewUnit != nullptr) {
                        const auto spot = map.findDeploySpot(pNewUnit, location_);
                        pNewUnit->deploy(context, spot);
                    }
                }
            }
        } break;
    }
}

bool Tile::hasAStructure(const ObjectManager& objectManager) const {
    if (!hasANonInfantryGroundObject()) {
        return false;
    }

    const auto* const pObject = objectManager.getObject(assignedNonInfantryGroundObjectList_.front());
    return ((pObject != nullptr) && pObject->isAStructure());
}

bool Tile::isExploredByTeam(const Game* game, int teamID) const {
    for (auto h = 0; h < NUM_HOUSES; h++) {
        const auto* pHouse = game->getHouse(static_cast<HOUSETYPE>(h));
        if ((pHouse != nullptr) && (pHouse->getTeamID() == teamID)) {
            if (isExploredByHouse(static_cast<HOUSETYPE>(h))) {
                return true;
            }
        }
    }
    return false;
}

bool Tile::isFoggedByHouse(bool fogOfWarEnabled, uint32_t gameCycleCount, HOUSETYPE houseID) const noexcept {
    if (dune::globals::debug)
        return false;

    if (fogOfWarEnabled) {
        return false;
    }

    return (gameCycleCount - lastAccess_[static_cast<int>(houseID)]) >= FOGTIME;
}

bool Tile::isFoggedByTeam(const Game* game, int teamID) const {
    if (dune::globals::debug)
        return false;

    if (!game->getGameInitSettings().getGameOptions().fogOfWar) {
        return false;
    }

    for (auto h = 0; h < NUM_HOUSES; h++) {
        const auto* pHouse = game->getHouse(static_cast<HOUSETYPE>(h));
        if ((pHouse != nullptr) && (pHouse->getTeamID() == teamID)) {
            if ((game->getGameCycleCount() - lastAccess_[h]) < FOGTIME) {
                return false;
            }
        }
    }

    return true;
}

uint32_t Tile::getRadarColor(const Game* game, House* pHouse, bool radar) {
    if (!dune::globals::debug && !isExploredByTeam(game, pHouse->getTeamID())) {
        return COLOR_BLACK;
    }

    if (radar && isFoggedByTeam(game, pHouse->getTeamID())) {
        return fogColor_;
    }

    if (const auto* const pObject = getObject(game->getObjectManager())) {
        uint32_t color = 0;

        if (pObject->getItemID() == Unit_Sandworm) {
            color = COLOR_WHITE;
        } else {
            const auto& palette = dune::globals::palette;

            // clang-format off
            switch (pObject->getOwner()->getHouseID()) {
            case HOUSETYPE::HOUSE_HARKONNEN:   color = SDL2RGB(palette[PALCOLOR_HARKONNEN]);  break;
            case HOUSETYPE::HOUSE_ATREIDES:    color = SDL2RGB(palette[PALCOLOR_ATREIDES]);   break;
            case HOUSETYPE::HOUSE_ORDOS:       color = SDL2RGB(palette[PALCOLOR_ORDOS]);      break;
            case HOUSETYPE::HOUSE_FREMEN:      color = SDL2RGB(palette[PALCOLOR_FREMEN]);     break;
            case HOUSETYPE::HOUSE_SARDAUKAR:   color = SDL2RGB(palette[PALCOLOR_SARDAUKAR]);  break;
            case HOUSETYPE::HOUSE_MERCENARY:   color = SDL2RGB(palette[PALCOLOR_MERCENARY]);  break;
            default:                           color = COLOR_BLACK;                           break;
            }
            // clang-format on
        }

        if (pObject->isAUnit()) {
            fogColor_ = getColorByTerrainType(getType());
        } else {
            fogColor_ = color;
        }

        // units and structures of the enemy are not visible if no radar
        if (!radar && !dune::globals::debug && (pObject->getOwner()->getTeamID() != pHouse->getTeamID())) {
            return COLOR_BLACK;
        }

        return color;
    }

    if (!radar && !dune::globals::debug) {
        return COLOR_BLACK;
    }

    fogColor_ = getColorByTerrainType(getType());

    return fogColor_;
}

Tile::TERRAINTILETYPE Tile::getTerrainTileImpl() const {
    const auto terrainType = type_;
    auto* const map        = dune::globals::currentGameMap;

    const auto x = location_.x;
    const auto y = location_.y;

    if (terrainType == TERRAINTYPE::Terrain_ThickSpice) {
        // check if we are surrounded by spice/thick spice
        const auto mask = map->get_neighbor_mask(x, y, [](const Tile& t) { return t.isSpice(); });

        if (0x0f != mask) {
            // to avoid graphical glitches (there is no tile for thick spice next to a non-spice tile) we draw this tile
            // as normal spice
            return static_cast<TERRAINTILETYPE>(static_cast<int>(TERRAINTILETYPE::TerrainTile_Spice) + mask);
        }
    }

    switch (terrainType) {
        case TERRAINTYPE::Terrain_Slab: {
            return TERRAINTILETYPE::TerrainTile_Slab;
        }

        case TERRAINTYPE::Terrain_Sand: {
            return TERRAINTILETYPE::TerrainTile_Sand;
        }

        case TERRAINTYPE::Terrain_Rock: {
            // determine which surrounding tiles are rock
            const auto mask = map->get_neighbor_mask(x, y, [](const Tile& t) { return t.isRock(); });

            return static_cast<TERRAINTILETYPE>(static_cast<int>(TERRAINTILETYPE::TerrainTile_Rock) + mask);
        }

        case TERRAINTYPE::Terrain_Dunes: {
            // determine which surrounding tiles are dunes
            const auto mask =
                map->get_neighbor_mask(x, y, [](const Tile& t) { return t.getType() == TERRAINTYPE::Terrain_Dunes; });

            return static_cast<TERRAINTILETYPE>(static_cast<int>(TERRAINTILETYPE::TerrainTile_Dunes) + mask);
        }

        case TERRAINTYPE::Terrain_Mountain: {
            // determine which surrounding tiles are mountains
            const auto mask = map->get_neighbor_mask(x, y, [](const Tile& t) { return t.isMountain(); });

            return static_cast<TERRAINTILETYPE>(static_cast<int>(TERRAINTILETYPE::TerrainTile_Mountain) + mask);
        }

        case TERRAINTYPE::Terrain_Spice: {
            // determine which surrounding tiles are spice
            const auto mask = map->get_neighbor_mask(x, y, [](const Tile& t) { return t.isSpice(); });

            return static_cast<TERRAINTILETYPE>(static_cast<int>(TERRAINTILETYPE::TerrainTile_Spice) + mask);
        }

        case TERRAINTYPE::Terrain_ThickSpice: {
            // determine which surrounding tiles are thick spice
            const auto mask = map->get_neighbor_mask(
                x, y, [](const Tile& t) { return t.getType() == TERRAINTYPE::Terrain_ThickSpice; });

            return static_cast<TERRAINTILETYPE>(static_cast<int>(TERRAINTILETYPE::TerrainTile_ThickSpice) + mask);
        }

        case TERRAINTYPE::Terrain_SpiceBloom: {
            return TERRAINTILETYPE::TerrainTile_SpiceBloom;
        }

        case TERRAINTYPE::Terrain_SpecialBloom: {
            return TERRAINTILETYPE::TerrainTile_SpecialBloom;
        }

        default: THROW(std::runtime_error, "Tile::getTerrainTile(): Invalid terrain type");
    }
}

int Tile::getHideTile(const Game* game, int teamID) const {

    const auto x = location_.x;
    const auto y = location_.y;

    auto* const map = dune::globals::currentGameMap;

    // are all surrounding tiles explored?

    if (((!map->tileExists(x, y - 1)) || (map->getTile(x, y - 1)->isExploredByTeam(game, teamID)))
        && ((!map->tileExists(x + 1, y)) || (map->getTile(x + 1, y)->isExploredByTeam(game, teamID)))
        && ((!map->tileExists(x, y + 1)) || (map->getTile(x, y + 1)->isExploredByTeam(game, teamID)))
        && ((!map->tileExists(x - 1, y)) || (map->getTile(x - 1, y)->isExploredByTeam(game, teamID)))) {
        return 0;
    }

    // determine what tiles are unexplored
    const bool up    = (!map->tileExists(x, y - 1)) || (!map->getTile(x, y - 1)->isExploredByTeam(game, teamID));
    const bool right = (!map->tileExists(x + 1, y)) || (!map->getTile(x + 1, y)->isExploredByTeam(game, teamID));
    const bool down  = (!map->tileExists(x, y + 1)) || (!map->getTile(x, y + 1)->isExploredByTeam(game, teamID));
    const bool left  = (!map->tileExists(x - 1, y)) || (!map->getTile(x - 1, y)->isExploredByTeam(game, teamID));

    return (static_cast<int>(up) | (right << 1) | (down << 2) | (left << 3));
}

int Tile::getFogTile(const Game* game, int teamID) const {
    const auto x = location_.x;
    const auto y = location_.y;

    auto* const map = dune::globals::currentGameMap;

    // are all surrounding tiles fogged?
    if (((!map->tileExists(x, y - 1)) || (!map->getTile(x, y - 1)->isFoggedByTeam(game, teamID)))
        && ((!map->tileExists(x + 1, y)) || (!map->getTile(x + 1, y)->isFoggedByTeam(game, teamID)))
        && ((!map->tileExists(x, y + 1)) || (!map->getTile(x, y + 1)->isFoggedByTeam(game, teamID)))
        && ((!map->tileExists(x - 1, y)) || (!map->getTile(x - 1, y)->isFoggedByTeam(game, teamID)))) {
        return 0;
    }

    // determine what tiles are fogged
    const bool up    = (!map->tileExists(x, y - 1)) || (map->getTile(x, y - 1)->isFoggedByTeam(game, teamID));
    const bool right = (!map->tileExists(x + 1, y)) || (map->getTile(x + 1, y)->isFoggedByTeam(game, teamID));
    const bool down  = (!map->tileExists(x, y + 1)) || (map->getTile(x, y + 1)->isFoggedByTeam(game, teamID));
    const bool left  = (!map->tileExists(x - 1, y)) || (map->getTile(x - 1, y)->isFoggedByTeam(game, teamID));

    return (static_cast<int>(up) | (right << 1) | (down << 2) | (left << 3));
}

template<typename Pred>
void Tile::selectFilter(Game* game, HOUSETYPE houseID, ObjectBase** lastCheckedObject, ObjectBase** lastSelectedObject,
                        Pred&& predicate) {
    auto changed              = false;
    ObjectBase* obj           = nullptr;
    ObjectBase* last_selected = nullptr;

    const auto selectUnit = [&](uint32_t objectID) {
        obj = game->getObjectManager().getObject(objectID);

        if (obj->isSelected() || houseID != obj->getOwner()->getHouseID())
            return;

        if (!predicate(obj))
            return;

        obj->setSelected(true);

        if (game->getSelectedList().insert(obj->getObjectID()).second)
            changed = true;

        last_selected = obj;
    };

    std::for_each(assignedInfantryList_.begin(), assignedInfantryList_.end(), selectUnit);

    std::for_each(assignedNonInfantryGroundObjectList_.begin(), assignedNonInfantryGroundObjectList_.end(), selectUnit);

    std::for_each(assignedUndergroundUnitList_.begin(), assignedUndergroundUnitList_.end(), selectUnit);

    std::for_each(assignedAirUnitList_.begin(), assignedAirUnitList_.end(), selectUnit);

    if (changed)
        game->selectionChanged();

    if (obj)
        *lastCheckedObject = obj;

    if (last_selected)
        *lastSelectedObject = last_selected;
}

template<typename Visitor>
void Tile::forEachUnit(Visitor&& visitor) const {
    for (auto i : assignedInfantryList_)
        visitor(i);

    for (auto i : assignedNonInfantryGroundObjectList_)
        visitor(i);

    for (auto i : assignedUndergroundUnitList_)
        visitor(i);

    for (auto i : assignedAirUnitList_)
        visitor(i);
}
