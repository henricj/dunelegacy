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

#include "units/Soldier.h"
#include "units/Trike.h"

#include <Tile.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>

#include <sand.h>
#include <Game.h>
#include <Map.h>
#include <House.h>
#include <SoundPlayer.h>
#include <ScreenBorder.h>
#include <Explosion.h>

#include <structures/StructureBase.h>
#include <units/InfantryBase.h>
#include <units/AirUnit.h>

#define FOGTIME MILLI2CYCLES(10 * 1000)

Tile::Tile() {
    type = Terrain_Sand;

    for (auto i = 0; i < NUM_TEAMS; i++) {
        explored[i] = false;
        lastAccess[i] = 0;
    }

    fogColor = COLOR_BLACK;

    owner = HOUSETYPE::HOUSE_INVALID;
    sandRegion = NONE_ID;

    spice = 0;

    sprite = pGFXManager->getObjPic(ObjPic_Terrain);

    for (auto& time : tracksCreationTime) {
        time = 0;
    }

    location.x = 0;
    location.y = 0;

    destroyedStructureTile = DestroyedStructure_None;
}


Tile::~Tile() = default;

void Tile::load(InputStream& stream) {
    type = static_cast<TERRAINTYPE>(stream.readUint32());

    stream.readBools(&explored[0], &explored[1], &explored[2], &explored[3], &explored[4], &explored[5], &explored[6]);

    bool bLastAccess[NUM_TEAMS];
    stream.readBools(&bLastAccess[0], &bLastAccess[1], &bLastAccess[2], &bLastAccess[3], &bLastAccess[4], &bLastAccess[5], &bLastAccess[6]);

    for (int i = 0; i < NUM_TEAMS; i++) {
        if (bLastAccess[i]) {
            lastAccess[i] = stream.readUint32();
        }
    }

    fogColor = stream.readUint32();

    owner = static_cast<HOUSETYPE>(stream.readSint32());
    sandRegion = stream.readUint32();

    spice = stream.readFixPoint();

    bool bHasDamage = 0;

    bool bHasDeadUnits = 0;

    bool bHasAirUnits = 0;

    bool bHasInfantry = 0;

    bool bHasUndergroundUnits = 0;

    bool bHasNonInfantryGroundObjects = 0;
    stream.readBools(&bHasDamage, &bHasDeadUnits, &bHasAirUnits, &bHasInfantry, &bHasUndergroundUnits, &bHasNonInfantryGroundObjects);

    if (bHasDamage) {
        damage.clear();
        Uint32 numDamage = stream.readUint32();
        damage.reserve(numDamage);
        for (Uint32 i = 0; i < numDamage; i++) {
            DAMAGETYPE newDamage;
            newDamage.damageType = static_cast<TerrainDamage_enum>(stream.readUint32());
            newDamage.tile = stream.readSint32();
            newDamage.realPos.x = stream.readSint32();
            newDamage.realPos.y = stream.readSint32();

            damage.push_back(newDamage);
        }
    }

    if (bHasDeadUnits) {
        deadUnits.clear();
        Uint32 numDeadUnits = stream.readUint32();
        deadUnits.reserve(numDeadUnits);
        for (Uint32 i = 0; i < numDeadUnits; i++) {
            DEADUNITTYPE newDeadUnit;
            newDeadUnit.type = stream.readUint8();
            newDeadUnit.house = static_cast<HOUSETYPE>(stream.readUint8());
            newDeadUnit.onSand = stream.readBool();
            newDeadUnit.realPos.x = stream.readSint32();
            newDeadUnit.realPos.y = stream.readSint32();
            newDeadUnit.timer = stream.readSint16();

            deadUnits.push_back(newDeadUnit);
        }
    }

    destroyedStructureTile = stream.readSint32();

    bool bTrackCounter[static_cast<int>(ANGLETYPE::NUM_ANGLES)];
    stream.readBools(&bTrackCounter[0], &bTrackCounter[1], &bTrackCounter[2], &bTrackCounter[3], &bTrackCounter[4], &bTrackCounter[5], &bTrackCounter[6], &bTrackCounter[7]);

    for (int i = 0; i < static_cast<int>(ANGLETYPE::NUM_ANGLES); i++) {
        if (bTrackCounter[i]) {
            tracksCreationTime[i] = stream.readUint32();
        }
    }

    if (bHasAirUnits) {
        stream.readUint32Vector(assignedAirUnitList);
    }

    if (bHasInfantry) {
        stream.readUint32Vector(assignedInfantryList);
    }

    if (bHasUndergroundUnits) {
        stream.readUint32Vector(assignedUndergroundUnitList);
    }

    if (bHasNonInfantryGroundObjects) {
        stream.readUint32Vector(assignedNonInfantryGroundObjectList);
    }
}

void Tile::save(OutputStream& stream, Uint32 gameCycleCount) const {
    stream.writeUint32(type);

    stream.writeBools(explored[0], explored[1], explored[2], explored[3], explored[4], explored[5], explored[6]);

    stream.writeBools((lastAccess[0] != 0), (lastAccess[1] != 0), (lastAccess[2] != 0), (lastAccess[3] != 0), (lastAccess[4] != 0), (lastAccess[5] != 0), (lastAccess[6] != 0));
    for (auto lastAccessFromTeam : lastAccess) {
        if (lastAccessFromTeam != 0) {
            stream.writeUint32(lastAccessFromTeam);
        }
    }

    stream.writeUint32(fogColor);

    stream.writeUint32(static_cast<Uint32>(owner));
    stream.writeUint32(sandRegion);

    stream.writeFixPoint(spice);

    stream.writeBools(!damage.empty(), !deadUnits.empty(), !assignedAirUnitList.empty(),
        !assignedInfantryList.empty(), !assignedUndergroundUnitList.empty(), !assignedNonInfantryGroundObjectList.empty());

    if (!damage.empty()) {
        stream.writeUint32(damage.size());
        for (const auto& damageItem : damage) {
            stream.writeUint32(static_cast<Uint32>(damageItem.damageType));
            stream.writeSint32(damageItem.tile);
            stream.writeSint32(damageItem.realPos.x);
            stream.writeSint32(damageItem.realPos.y);
        }
    }

    if (!deadUnits.empty()) {
        stream.writeUint32(deadUnits.size());
        for (const auto& deadUnit : deadUnits) {
            stream.writeUint8(deadUnit.type);
            stream.writeUint8(static_cast<Uint8>(deadUnit.house));
            stream.writeBool(deadUnit.onSand);
            stream.writeSint32(deadUnit.realPos.x);
            stream.writeSint32(deadUnit.realPos.y);
            stream.writeSint16(deadUnit.timer);
        }
    }

    stream.writeSint32(destroyedStructureTile);

    // clean-up tracksCreationTime to save space in the save game
    std::array<Uint32, static_cast<int>(ANGLETYPE::NUM_ANGLES)> tracksCreationTimeToSave;
    for(int i = 0; i < tracksCreationTimeToSave.size(); i++) {
        tracksCreationTimeToSave[i] = (tracksCreationTime[i] + TRACKSTIME < gameCycleCount) ? 0 : tracksCreationTime[i];
    }

    stream.writeBools((tracksCreationTimeToSave[0] != 0), (tracksCreationTimeToSave[1] != 0), (tracksCreationTimeToSave[2] != 0), (tracksCreationTimeToSave[3] != 0),
        (tracksCreationTimeToSave[4] != 0), (tracksCreationTimeToSave[5] != 0), (tracksCreationTimeToSave[6] != 0), (tracksCreationTimeToSave[7] != 0));
    for (auto i : tracksCreationTimeToSave) {
        if (i != 0) {
            stream.writeUint32(i);
        }
    }

    if (!assignedAirUnitList.empty()) {
        stream.writeUint32Vector(assignedAirUnitList);
    }

    if (!assignedInfantryList.empty()) {
        stream.writeUint32Vector(assignedInfantryList);
    }

    if (!assignedUndergroundUnitList.empty()) {
        stream.writeUint32Vector(assignedUndergroundUnitList);
    }

    if (!assignedNonInfantryGroundObjectList.empty()) {
        stream.writeUint32Vector(assignedNonInfantryGroundObjectList);
    }
}

void Tile::assignAirUnit(Uint32 newObjectID) {
    assignedAirUnitList.push_back(newObjectID);
}

void Tile::assignNonInfantryGroundObject(Uint32 newObjectID) {
    assignedNonInfantryGroundObjectList.push_back(newObjectID);
}

int Tile::assignInfantry(ObjectManager& objectManager, Uint32 newObjectID, Sint8 currentPosition) {
    auto newPosition = currentPosition;

    if (currentPosition < 0) {
        std::array<bool, NUM_INFANTRY_PER_TILE> used{};

        for (auto objectID : assignedInfantryList) {
            auto *const pInfantry = dynamic_cast<InfantryBase*>(objectManager.getObject(objectID));
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

        newPosition = std::max(static_cast<Sint8>(0), std::min(newPosition, static_cast<Sint8>(NUM_INFANTRY_PER_TILE)));
    }

    assignedInfantryList.push_back(newObjectID);
    return newPosition;
}


void Tile::assignUndergroundUnit(Uint32 newObjectID) {
    assignedUndergroundUnitList.push_back(newObjectID);
}

void Tile::blitGround(Game* game, int xPos, int yPos) {
    if (hasANonInfantryGroundObject() && getNonInfantryGroundObject(game->getObjectManager())->isAStructure())
        return;

    const auto tileIndex = static_cast<int>(getTerrainTile());
    const auto indexX = tileIndex % NUM_TERRAIN_TILES_X;
    const auto indexY = tileIndex / NUM_TERRAIN_TILES_X;
    const auto zoomed_tilesize = world2zoomedWorld(TILESIZE);
    SDL_Rect source = { indexX*zoomed_tilesize, indexY*zoomed_tilesize, zoomed_tilesize, zoomed_tilesize };
    SDL_Rect drawLocation = { xPos, yPos, zoomed_tilesize, zoomed_tilesize };

    // draw terrain
    if (destroyedStructureTile == DestroyedStructure_None || destroyedStructureTile == DestroyedStructure_Wall) {
        Dune_RenderCopy(renderer, sprite[currentZoomlevel], &source, &drawLocation);
    }

    if(destroyedStructureTile != DestroyedStructure_None) {
        const auto* const pDestroyedStructureTex = pGFXManager->getZoomedObjPic(ObjPic_DestroyedStructure, currentZoomlevel);
        SDL_Rect source2 = { destroyedStructureTile*zoomed_tilesize, 0, zoomed_tilesize, zoomed_tilesize };
        Dune_RenderCopy(renderer, pDestroyedStructureTex, &source2, &drawLocation);
    }

    if (isFoggedByTeam(game, pLocalHouse->getTeamID()))
        return;

    source.y = 0;

    const auto gameCycleCount = game->getGameCycleCount();

    source.y = 0;

    // tracks
    const auto* const pTracks = pGFXManager->getZoomedObjPic(ObjPic_Terrain_Tracks, currentZoomlevel);
    for (auto i = 0; i < static_cast<int>(ANGLETYPE::NUM_ANGLES); i++) {
        const auto tracktime = static_cast<int>(gameCycleCount - tracksCreationTime[i]);
        if ((tracksCreationTime[i] != 0) && (tracktime < TRACKSTIME)) {
            source.x = ((10 - i) % 8)*zoomed_tilesize;
            SDL_SetTextureAlphaMod(pTracks->texture_, std::min(255, 256 * (TRACKSTIME - tracktime) / TRACKSTIME));
            Dune_RenderCopy(renderer, pTracks, &source, &drawLocation);
            SDL_SetTextureAlphaMod(pTracks->texture_, 255);
        }
    }

    // damage
    for (const auto& damageItem : damage) {
        source.x = damageItem.tile*zoomed_tilesize;
        SDL_Rect dest = { screenborder->world2screenX(damageItem.realPos.x) - zoomed_tilesize / 2,
            screenborder->world2screenY(damageItem.realPos.y) - zoomed_tilesize / 2,
            zoomed_tilesize,
            zoomed_tilesize };

        if (damageItem.damageType == Tile::TerrainDamage_enum::Terrain_RockDamage) {
            Dune_RenderCopy(renderer, pGFXManager->getZoomedObjPic(ObjPic_RockDamage, currentZoomlevel), &source, &dest);
        }
        else {
            Dune_RenderCopy(renderer, pGFXManager->getZoomedObjPic(ObjPic_SandDamage, currentZoomlevel), &source, &drawLocation);
        }
    }
}

void Tile::blitStructures(Game* game, int xPos, int yPos) const {
    if(!hasANonInfantryGroundObject()) return;

    // if got a structure, draw the structure, and don't draw any terrain because wont be seen
    auto* pStructure = dune_cast<StructureBase>(getNonInfantryGroundObject(game->getObjectManager()));
    if(!pStructure) return;

    auto* map = game->getMap();

    map->for_each(pStructure->getX(), pStructure->getY(), pStructure->getX() + pStructure->getStructureSizeX(),
                  pStructure->getY() + pStructure->getStructureSizeY(), [&](const auto& tile) {
                      if(screenborder->isTileInsideScreen(tile.location) &&
                         (tile.isExploredByTeam(game, pLocalHouse->getTeamID()) || debug)) {
                          pStructure->setFogged(isFoggedByTeam(game, pLocalHouse->getTeamID()));

                          if(&tile == this) {
                              // only this tile will draw it, so will be drawn only once
                              pStructure->blitToScreen();
                          }

                          return;
                      }
                  });
}

void Tile::blitUndergroundUnits(Game* game, int xPos, int yPos) const {
    if (!hasAnUndergroundUnit() || isFoggedByTeam(game, pLocalHouse->getTeamID()))
        return;

    auto *current = getUndergroundUnit(game->getObjectManager());

    if (current->isVisible(pLocalHouse->getTeamID())) {
        if (location == current->getLocation()) {
            current->blitToScreen();
        }
    }
}

void Tile::blitDeadUnits(Game* game, int xPos, int yPos) {
    if (isFoggedByTeam(game, pLocalHouse->getTeamID()))
        return;

    const auto zoomed_tile = world2zoomedWorld(TILESIZE);

    for (const auto& deadUnit : deadUnits) {
        SDL_Rect source = { 0, 0, zoomed_tile, zoomed_tile };
        const DuneTexture* pTexture = nullptr;
        switch (deadUnit.type) {
            case DeadUnit_Infantry: {
                pTexture = pGFXManager->getZoomedObjPic(ObjPic_DeadInfantry, deadUnit.house, currentZoomlevel);
                source.x = (deadUnit.timer < 1000 && deadUnit.onSand) ? zoomed_tile : 0;
            } break;

            case DeadUnit_Infantry_Squashed1: {
                pTexture = pGFXManager->getZoomedObjPic(ObjPic_DeadInfantry, deadUnit.house, currentZoomlevel);
                source.x = 4 * zoomed_tile;
            } break;

            case DeadUnit_Infantry_Squashed2: {
                pTexture = pGFXManager->getZoomedObjPic(ObjPic_DeadInfantry, deadUnit.house, currentZoomlevel);
                source.x = 5 * zoomed_tile;
            } break;

            case DeadUnit_Carrall: {
                pTexture = pGFXManager->getZoomedObjPic(ObjPic_DeadAirUnit, deadUnit.house, currentZoomlevel);
                if (deadUnit.onSand) {
                    source.x = (deadUnit.timer < 1000) ? 5 * zoomed_tile : 4 * zoomed_tile;
                }
                else {
                    source.x = 3 * zoomed_tile;
                }
            } break;

            case DeadUnit_Ornithopter: {
                pTexture = pGFXManager->getZoomedObjPic(ObjPic_DeadAirUnit, deadUnit.house, currentZoomlevel);
                if (deadUnit.onSand) {
                    source.x = (deadUnit.timer < 1000) ? 2 * zoomed_tile : zoomed_tile;
                }
                else {
                    source.x = 0;
                }
            } break;

            default:
                break;
        }

        if (pTexture != nullptr) {
            SDL_Rect dest = { screenborder->world2screenX(deadUnit.realPos.x) - zoomed_tile / 2,
                screenborder->world2screenY(deadUnit.realPos.y) - zoomed_tile / 2,
                zoomed_tile,
                zoomed_tile };
            Dune_RenderCopy(renderer, pTexture, &source, &dest);
        }
    }
}

void Tile::blitInfantry(Game* game, int xPos, int yPos) {
    if (isFoggedByTeam(game, pLocalHouse->getTeamID()))
        return;

    for (auto objectID : assignedInfantryList) {
        auto *pInfantry = game->getObjectManager().getObject<InfantryBase>(objectID);
        if (pInfantry == nullptr) {
            continue;
        }

        if (pInfantry->isVisible(pLocalHouse->getTeamID())) {
            if (location == pInfantry->getLocation()) {
                pInfantry->blitToScreen();
            }
        }
    }
}

void Tile::blitNonInfantryGroundUnits(Game* game, int xPos, int yPos) {
    if (isFoggedByTeam(game, pLocalHouse->getTeamID()))
        return;

    for (auto objectID : assignedNonInfantryGroundObjectList) {
        auto *pObject = game->getObjectManager().getObject(objectID);

        if (pObject && pObject->isAUnit() && pObject->isVisible(pLocalHouse->getTeamID())) {
            if (location == pObject->getLocation()) {
                pObject->blitToScreen();
            }
        }
    }
}


void Tile::blitAirUnits(Game* game, int xPos, int yPos) {
    auto *const player_house = pLocalHouse;
    const auto is_fogged = isFoggedByTeam(game, player_house->getTeamID());

    for (auto objectID : assignedAirUnitList) {
        auto *pAirUnit = game->getObjectManager().getObject<AirUnit>(objectID);
        if (pAirUnit == nullptr) {
            continue;
        }

        if (!is_fogged || pAirUnit->getOwner() == player_house) {
            if (pAirUnit->isVisible(pLocalHouse->getTeamID())) {
                if (location == pAirUnit->getLocation()) {
                    pAirUnit->blitToScreen();
                }
            }
        }
    }
}

void Tile::blitSelectionRects(Game* game, int xPos, int yPos) const {
    if (isFoggedByTeam(game, pLocalHouse->getTeamID()))
        return;

    forEachUnit([&](Uint32 objectID) {
        auto *pObject = game->getObjectManager().getObject(objectID);
        if (pObject == nullptr) {
            return;
        }
        // possibly draw selection rectangle multiple times, e.g. for structures
        if (pObject->isVisible(pLocalHouse->getTeamID())) {
            if (pObject->isSelected()) {
                pObject->drawSelectionBox();
            }

            if (pObject->isSelectedByOtherPlayer()) {
                pObject->drawOtherPlayerSelectionBox();
            }
        }
    });
}

void Tile::update_impl()
{
    deadUnits.erase(
        std::remove_if(std::begin(deadUnits), std::end(deadUnits),
            [](DEADUNITTYPE& dut)
            {
                if (0 == dut.timer)
                    return true;
                --dut.timer;
                return false;
            }),
        std::end(deadUnits));
}


void Tile::clearTerrain() {
    damage.clear();
    deadUnits.clear();
}

void Tile::setTrack(ANGLETYPE direction, Uint32 gameCycleCounter) {
    if (type == Terrain_Sand || type == Terrain_Dunes || type == Terrain_Spice || type == Terrain_ThickSpice) {
        tracksCreationTime[static_cast<int>(direction)] = gameCycleCounter;
    }
}

void Tile::selectAllPlayersUnits(Game* game, HOUSETYPE houseID, ObjectBase** lastCheckedObject, ObjectBase** lastSelectedObject) {
    selectFilter(game, houseID, lastCheckedObject, lastSelectedObject,
        [](ObjectBase* obj) { return  obj->isAUnit() && obj->isRespondable(); });
}


void Tile::selectAllPlayersUnitsOfType(Game* game, HOUSETYPE houseID, ItemID_enum itemID, ObjectBase** lastCheckedObject, ObjectBase** lastSelectedObject) {
    selectFilter(game, houseID, lastCheckedObject, lastSelectedObject,
        [=](ObjectBase* obj) { return  obj->getItemID() == itemID; });
}

template<typename Container, typename Val>
static void erase_remove(Container& c, Val val) {
    c.erase(std::remove(std::begin(c), std::end(c), val), std::end(c));
}

void Tile::unassignAirUnit(Uint32 objectID) {
    erase_remove(assignedAirUnitList, objectID);
}

void Tile::unassignNonInfantryGroundObject(Uint32 objectID) {
    erase_remove(assignedNonInfantryGroundObjectList, objectID);
}

void Tile::unassignUndergroundUnit(Uint32 objectID) {
    erase_remove(assignedUndergroundUnitList, objectID);
}

void Tile::unassignInfantry(Uint32 objectID, int currentPosition) {
    erase_remove(assignedInfantryList, objectID);
}

void Tile::unassignObject(Uint32 objectID) {
    if (hasInfantry()) unassignInfantry(objectID, -1);
    if (hasAnUndergroundUnit()) unassignUndergroundUnit(objectID);
    if (hasANonInfantryGroundObject()) unassignNonInfantryGroundObject(objectID);
    if (hasAnAirUnit()) unassignAirUnit(objectID);
}


void Tile::setType(const GameContext& context, TERRAINTYPE newType) {
    const auto& [game, map, objectManager] = context;

    type = newType;
    destroyedStructureTile = DestroyedStructure_None;

    terrainTile = TERRAINTILETYPE::TerrainTile_Invalid;
    map.for_each_neighbor(location.x, location.y,
                          [](Tile& t) { t.terrainTile = TERRAINTILETYPE::TerrainTile_Invalid; });

    if (type == Terrain_Spice) {
        spice = game.randomGen.rand(RANDOMSPICEMIN, RANDOMSPICEMAX);
    }
    else if (type == Terrain_ThickSpice) {
        spice = game.randomGen.rand(RANDOMTHICKSPICEMIN, RANDOMTHICKSPICEMAX);
    }
    else if (type == Terrain_Dunes) {
    }
    else {
        spice = 0;

        if (isRock()) {
            std::vector<ObjectBase*> pending_destroy;

            sandRegion = NONE_ID;
            if (hasAnUndergroundUnit()) {
                auto units = std::move(assignedUndergroundUnitList);
                assignedUndergroundUnitList.clear();

                for (const auto object_id : units)
                {
                    auto *const current = game.getObjectManager().getObject(object_id);

                    unassignUndergroundUnit(current->getObjectID());
                    current->destroy(context);
                }
            }

            if (type == Terrain_Mountain) {
                if (hasANonInfantryGroundObject()) {
                    auto units = std::move(assignedNonInfantryGroundObjectList);
                    assignedNonInfantryGroundObjectList.clear();

                    for (const auto object_id : units)
                    {
                        auto *const object = game.getObjectManager().getObject(object_id);

                        if (object)
                            pending_destroy.push_back(object);
                        else
                            assignedNonInfantryGroundObjectList.push_back(object_id);
                    }

                    // Try to keep the largest buffer.
                    if (assignedNonInfantryGroundObjectList.empty() && units.capacity() > assignedNonInfantryGroundObjectList.capacity())
                    {
                        units.clear();
                        assignedNonInfantryGroundObjectList = std::move(units);
                    }
                }
            }

            std::for_each(std::begin(pending_destroy), std::end(pending_destroy),
                          [&](ObjectBase* obj) { obj->destroy(context); });
        }
    }

    map.for_each(location.x, location.y, location.x + 4, location.y + 4, [](Tile &t) { t.clearTerrain(); });
}


void Tile::squash(const GameContext& context) const {
    if (!hasInfantry()) return;

    for (const auto object_id : assignedInfantryList) {
        auto *current = context.objectManager.getObject<InfantryBase>(object_id);

        if(current == nullptr)
            continue;

        current->squash(context);
    }
}


int Tile::getInfantryTeam(const ObjectManager& objectManager) const {
    int team = INVALID;
    if(hasInfantry()) {
        if(auto* infantry = getInfantry(objectManager)) {
            if(auto* owner = infantry->getOwner()) team = owner->getTeamID();
        }
    }
    return team;
}


FixPoint Tile::harvestSpice(const GameContext& context) {
    const auto oldSpice = spice;

    if ((spice - HARVESTSPEED) >= 0) {
        spice -= HARVESTSPEED;
    }
    else {
        spice = 0;
    }

    if (oldSpice >= RANDOMTHICKSPICEMIN && spice < RANDOMTHICKSPICEMIN) {
        setType(context, Terrain_Spice);
    }

    if (oldSpice > 0 && spice == 0) {
        setType(context, Terrain_Sand);
    }

    return (oldSpice - spice);
}


void Tile::setSpice(FixPoint newSpice) {
    if (newSpice <= 0) {
        type = Terrain_Sand;
    }
    else if (newSpice >= RANDOMTHICKSPICEMIN) {
        type = Terrain_ThickSpice;
    }
    else {
        type = Terrain_Spice;
    }
    spice = newSpice;
}


AirUnit* Tile::getAirUnit(const ObjectManager& objectManager) const {
    return assignedAirUnitList.empty() ? nullptr
                                       : dune_cast<AirUnit>(objectManager.getObject(assignedAirUnitList.front()));
}

ObjectBase* Tile::getGroundObject(const ObjectManager& objectManager) const {
    if(hasANonInfantryGroundObject()) return getNonInfantryGroundObject(objectManager);
    if(hasInfantry()) return getInfantry(objectManager);

    return nullptr;
}

InfantryBase* Tile::getInfantry(const ObjectManager& objectManager) const {
    return assignedInfantryList.empty()
               ? nullptr
               : objectManager.getObject<InfantryBase>(assignedInfantryList.front());
}

ObjectBase* Tile::getNonInfantryGroundObject(const ObjectManager& objectManager) const {
    return assignedNonInfantryGroundObjectList.empty()
               ? nullptr
               : objectManager.getObject(assignedNonInfantryGroundObjectList.front());
}

UnitBase* Tile::getUndergroundUnit(const ObjectManager& objectManager) const {
    return assignedUndergroundUnitList.empty()
               ? nullptr
               : objectManager.getObject<UnitBase>(assignedUndergroundUnitList.front());
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
    if(hasAnAirUnit()) return getAirUnit(objectManager);
    if(hasANonInfantryGroundObject()) return getNonInfantryGroundObject(objectManager);
    if(hasInfantry()) return getInfantry(objectManager);
    if(hasAnUndergroundUnit()) return getUndergroundUnit(objectManager);

    return nullptr;
}


ObjectBase* Tile::getObjectAt(const ObjectManager& objectManager, int x, int y) const {
    ObjectBase* pObject = nullptr;
    if (hasAnAirUnit()) {
        pObject = getAirUnit(objectManager);
    }
    else if (hasANonInfantryGroundObject()) {
        pObject = getNonInfantryGroundObject(objectManager);
    }
    else if (hasInfantry()) {
        auto closestDistance = FixPt_MAX;
        const Coord atPos(x, y);

        for (const auto objectID : assignedInfantryList) {
            auto *const pInfantry = objectManager.getObject<InfantryBase>(objectID);
            if (pInfantry == nullptr) {
                continue;
            }

            const auto centerPoint = pInfantry->getCenterPoint();
            const auto distance = distanceFrom(atPos, centerPoint);
            if (distance < closestDistance) {
                closestDistance = distance;
                pObject = pInfantry;
            }
        }
    }
    else if (hasAnUndergroundUnit()) {
        pObject = getUndergroundUnit(objectManager);
    }

    return pObject;
}


ObjectBase* Tile::getObjectWithID(const ObjectManager& objectManager, Uint32 objectID) const {
    const auto predicate = [=](Uint32 n) { return n == objectID; };

    if(std::any_of(assignedInfantryList.begin(), assignedInfantryList.end(), predicate)
        || std::any_of(assignedNonInfantryGroundObjectList.begin(), assignedNonInfantryGroundObjectList.end(), predicate)
        ||  std::any_of(assignedUndergroundUnitList.begin(), assignedUndergroundUnitList.end(), predicate)
        ||  std::any_of(assignedAirUnitList.begin(), assignedAirUnitList.end(), predicate))
    {
        return objectManager.getObject(objectID);
    }

    return nullptr;
}

void Tile::triggerSpiceBloom(const GameContext& context, House* pTrigger) {
    if (!isSpiceBloom()) return;

    //a spice bloom
    soundPlayer->playSoundAt(Sound_Bloom, getLocation());
    screenborder->shakeScreen(18);
    if (pTrigger == pLocalHouse) {
        soundPlayer->playVoice(BloomLocated, pLocalHouse->getHouseID());
    }

    setType(context, Terrain_Spice); // Set this tile to spice first
    context.map.createSpiceField(context, location, 5);

    const auto realLocation = location * TILESIZE + Coord(TILESIZE / 2, TILESIZE / 2);

    if (damage.size() < DAMAGE_PER_TILE) {
        DAMAGETYPE newDamage;
        newDamage.tile = static_cast<int>(SANDDAMAGETYPE::SandDamage1);
        newDamage.damageType = TerrainDamage_enum::Terrain_SandDamage;
        newDamage.realPos = realLocation;

        damage.push_back(newDamage);
    }

    context.game.addExplosion(Explosion_SpiceBloom, realLocation, pTrigger->getHouseID());
}

void Tile::triggerSpecialBloom(const GameContext& context, House* pTrigger) {
    if (!isSpecialBloom()) return;

    setType(context, Terrain_Sand);

    const auto& [game, map, objectManager] = context;

    switch (game.randomGen.rand(0, 3)) {
        case 0: {
            // the player gets an randomly chosen amount of credits between 150 and 400
            pTrigger->addCredits(game.randomGen.rand(150, 400), false);
        } break;

        case 1: {
            // The house gets a Trike for free. It spawns beside the special bloom.
            auto *pNewUnit = pTrigger->createUnit<Trike>();
            if (pNewUnit != nullptr) {
                const auto spot = map.findDeploySpot(pNewUnit, location);
                pNewUnit->deploy(context, spot);
            }
        } break;

        case 2: {
            // One of the AI players on the map (one that has at least one unit) gets a Trike for free. It spawns beside
            // the special bloom.
            int numCandidates = 0;
            game.for_each_house([&](const auto& house) {
                if(house.getTeamID() != pTrigger->getTeamID() && house.getNumUnits() > 0) numCandidates++;
            });

            if(numCandidates == 0) break;

            int candidate = game.randomGen.rand(0, numCandidates - 1);

            auto* pEnemyHouse = game.house_find_if([&](auto& house) {
                if(house.getTeamID() != pTrigger->getTeamID() && house.getNumUnits() > 0) {
                    if(candidate-- == 0) return true;
                    candidate--;
                }
                return false;
            });

            if(!pEnemyHouse) break;

            auto* const pNewUnit = pEnemyHouse->createUnit<Trike>();
            if(pNewUnit != nullptr)
            {
                const auto spot = map.findDeploySpot(pNewUnit, location);
                pNewUnit->deploy(context, spot);
            }
        } break;

        case 3:
        default: {
            // One of the AI players on the map (one that has at least one unit) gets an Infantry unit (3 Soldiers) for free. The spawn beside the special bloom.
            auto numCandidates = 0;
            game.for_each_house([&](auto& house) {
                if(house.getTeamID() != pTrigger->getTeamID() && house.getNumUnits() > 0) { numCandidates++; }
            });

            if (numCandidates == 0) {
                break;
            }

            auto candidate = game.randomGen.rand(0, numCandidates - 1);

            House* pEnemyHouse = nullptr;
            for(auto i = 0; i < static_cast<int>(HOUSETYPE::NUM_HOUSES); i++) {
                auto *const pHouse = game.getHouse(static_cast<HOUSETYPE>(i));
                if (pHouse != nullptr && pHouse->getTeamID() != pTrigger->getTeamID() && pHouse->getNumUnits() > 0) {
                    if (candidate == 0) {
                        pEnemyHouse = pHouse;
                        break;
                    }
                    candidate--;
                }
            }

            if(pEnemyHouse) {
                for (int i = 0; i < 3; i++) {
                    auto *const pNewUnit = pEnemyHouse->createUnit<Soldier>();
                    if (pNewUnit != nullptr) {
                        const auto spot = map.findDeploySpot(pNewUnit, location);
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

    auto *const pObject = objectManager.getObject(assignedNonInfantryGroundObjectList.front());
    return ((pObject != nullptr) && pObject->isAStructure());
}

bool Tile::isExploredByTeam(const Game* game, int teamID) const {
    for(auto h = 0; h < static_cast<int>(HOUSETYPE::NUM_HOUSES); h++) {
        const auto* pHouse = game->getHouse(static_cast<HOUSETYPE>(h));
        if ((pHouse != nullptr) && (pHouse->getTeamID() == teamID)) {
            if(isExploredByHouse(static_cast<HOUSETYPE>(h))) {
                return true;
            }
        }
    }
    return false;
}

bool Tile::isFoggedByHouse(bool fogOfWarEnabled, Uint32 gameCycleCount, HOUSETYPE houseID) const noexcept {
    if (debug)
        return false;

    if (fogOfWarEnabled) {
        return false;
    }

    return (gameCycleCount - lastAccess[static_cast<int>(houseID)]) >= FOGTIME;
}

bool Tile::isFoggedByTeam(const Game* game, int teamID) const {
    if (debug)
        return false;

    if (!game->getGameInitSettings().getGameOptions().fogOfWar) {
        return false;
    }

    for (auto h = 0; h < static_cast<int>(HOUSETYPE::NUM_HOUSES); h++) {
        const auto* pHouse = game->getHouse(static_cast<HOUSETYPE>(h));
        if ((pHouse != nullptr) && (pHouse->getTeamID() == teamID)) {
            if((game->getGameCycleCount() - lastAccess[h]) < FOGTIME) {
                return false;
            }
        }
    }

    return true;
}

Uint32 Tile::getRadarColor(const Game* game, House* pHouse, bool radar) {
    if (!debug && !isExploredByTeam(game, pHouse->getTeamID())) {
        return COLOR_BLACK;
    }

    if (radar && isFoggedByTeam(game, pHouse->getTeamID())) {
        return fogColor;
    }

    auto *const pObject = getObject(game->getObjectManager());
    if (pObject != nullptr) {
        Uint32 color = 0;

        if (pObject->getItemID() == Unit_Sandworm) {
            color = COLOR_WHITE;
        }
        else {
            // clang-format off
            switch (pObject->getOwner()->getHouseID()) {
            case HOUSETYPE::HOUSE_HARKONNEN:   color = SDL2RGB(palette[PALCOLOR_HARKONNEN]);  break;
            case HOUSETYPE::HOUSE_ATREIDES:    color = SDL2RGB(palette[PALCOLOR_ATREIDES]);   break;
            case HOUSETYPE::HOUSE_ORDOS:       color = SDL2RGB(palette[PALCOLOR_ORDOS]);      break;
            case HOUSETYPE::HOUSE_FREMEN:      color = SDL2RGB(palette[PALCOLOR_FREMEN]);     break;
            case HOUSETYPE::HOUSE_SARDAUKAR:   color = SDL2RGB(palette[PALCOLOR_SARDAUKAR]);  break;
            case HOUSETYPE::HOUSE_MERCENARY:   color = SDL2RGB(palette[PALCOLOR_MERCENARY]);  break;
            default:                color = COLOR_BLACK;                           break;
            }
            // clang-format on
        }

        if (pObject->isAUnit()) {
            fogColor = getColorByTerrainType(getType());
        }
        else {
            fogColor = color;
        }

        // units and structures of the enemy are not visible if no radar
        if (!radar && !debug && (pObject->getOwner()->getTeamID() != pHouse->getTeamID())) {
            return COLOR_BLACK;
        }

        return color;
    }

    if (!radar && !debug) {
        return COLOR_BLACK;
    }

    fogColor = getColorByTerrainType(getType());

    return fogColor;
}


Tile::TERRAINTILETYPE Tile::getTerrainTileImpl() const {
    const auto terrainType = type;
    auto *const map = currentGameMap;

    const auto x = location.x;
    const auto y = location.y;

    if (terrainType == Terrain_ThickSpice) {
        // check if we are surrounded by spice/thick spice
        const auto mask = map->get_neighbor_mask(x, y, [](const Tile& t) { return t.isSpice(); });

        if (0x0f != mask) {
            // to avoid graphical glitches (there is no tile for thick spice next to a non-spice tile) we draw this tile as normal spice
            return static_cast<TERRAINTILETYPE>(static_cast<int>(TERRAINTILETYPE::TerrainTile_Spice) + mask);
        }
    }

    switch (terrainType) {
    case TERRAINTYPE::Terrain_Slab: {
        return TERRAINTILETYPE::TerrainTile_Slab;
    } break;

    case TERRAINTYPE::Terrain_Sand: {
        return TERRAINTILETYPE::TerrainTile_Sand;
    } break;

    case Terrain_Rock: {
        // determine which surrounding tiles are rock
        const auto mask = map->get_neighbor_mask(x, y, [](const Tile& t) { return t.isRock(); });

        return static_cast<TERRAINTILETYPE>(static_cast<int>(TERRAINTILETYPE::TerrainTile_Rock) + mask);
      }

    case TERRAINTYPE::Terrain_Dunes: {
        // determine which surrounding tiles are dunes
        const auto mask = map->get_neighbor_mask(x, y, [](const Tile& t) { return t.getType() == TERRAINTYPE::Terrain_Dunes; });

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
        const auto mask = map->get_neighbor_mask(x, y, [](const Tile& t) { return t.getType() == Terrain_ThickSpice; });

        return static_cast<TERRAINTILETYPE>(static_cast<int>(TERRAINTILETYPE::TerrainTile_ThickSpice) + mask);
    }

    case TERRAINTYPE::Terrain_SpiceBloom: {
        return TERRAINTILETYPE::TerrainTile_SpiceBloom;
    } break;

    case TERRAINTYPE::Terrain_SpecialBloom: {
        return TERRAINTILETYPE::TerrainTile_SpecialBloom;
    } break;

    default:
        THROW(std::runtime_error, "Tile::getTerrainTile(): Invalid terrain type");
    }
}

int Tile::getHideTile(const Game* game, int teamID) const {

    const auto x = location.x;
    const auto y = location.y;

    auto *const map = currentGameMap;

    // are all surrounding tiles explored?

    if (((!map->tileExists(x, y - 1)) || (map->getTile(x, y - 1)->isExploredByTeam(game, teamID)))
        && ((!map->tileExists(x + 1, y)) || (map->getTile(x + 1, y)->isExploredByTeam(game, teamID)))
        && ((!map->tileExists(x, y + 1)) || (map->getTile(x, y + 1)->isExploredByTeam(game, teamID)))
        && ((!map->tileExists(x - 1, y)) || (map->getTile(x - 1, y)->isExploredByTeam(game, teamID)))) {
        return 0;
    }

    // determine what tiles are unexplored
    bool up = (!map->tileExists(x, y - 1)) || (!map->getTile(x, y - 1)->isExploredByTeam(game, teamID));
    bool right = (!map->tileExists(x + 1, y)) || (!map->getTile(x + 1, y)->isExploredByTeam(game, teamID));
    bool down = (!map->tileExists(x, y + 1)) || (!map->getTile(x, y + 1)->isExploredByTeam(game, teamID));
    bool left = (!map->tileExists(x - 1, y)) || (!map->getTile(x - 1, y)->isExploredByTeam(game, teamID));

    return (((int)up) | (right << 1) | (down << 2) | (left << 3));
}

int Tile::getFogTile(const Game* game, int teamID) const {
    const auto x = location.x;
    const auto y = location.y;

    auto *const map = currentGameMap;

    // are all surrounding tiles fogged?
    if (((!map->tileExists(x, y - 1)) || (!map->getTile(x, y - 1)->isFoggedByTeam(game, teamID)))
        && ((!map->tileExists(x + 1, y)) || (!map->getTile(x + 1, y)->isFoggedByTeam(game, teamID)))
        && ((!map->tileExists(x, y + 1)) || (!map->getTile(x, y + 1)->isFoggedByTeam(game, teamID)))
        && ((!map->tileExists(x - 1, y)) || (!map->getTile(x - 1, y)->isFoggedByTeam(game, teamID)))) {
        return 0;
    }

    // determine what tiles are fogged
    bool up = (!map->tileExists(x, y - 1)) || (map->getTile(x, y - 1)->isFoggedByTeam(game, teamID));
    bool right = (!map->tileExists(x + 1, y)) || (map->getTile(x + 1, y)->isFoggedByTeam(game, teamID));
    bool down = (!map->tileExists(x, y + 1)) || (map->getTile(x, y + 1)->isFoggedByTeam(game, teamID));
    bool left = (!map->tileExists(x - 1, y)) || (map->getTile(x - 1, y)->isFoggedByTeam(game, teamID));

    return (((int)up) | (right << 1) | (down << 2) | (left << 3));
}

template<typename Pred>
void Tile::selectFilter(Game* game, HOUSETYPE houseID, ObjectBase** lastCheckedObject, ObjectBase** lastSelectedObject, Pred&& predicate)
{
    auto changed = false;
    ObjectBase* obj = nullptr;
    ObjectBase* last_selected = nullptr;

    const auto selectUnit = [&](Uint32 objectID) {
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

    std::for_each(  assignedInfantryList.begin(),
                    assignedInfantryList.end(),
                    selectUnit);

    std::for_each(  assignedNonInfantryGroundObjectList.begin(),
                    assignedNonInfantryGroundObjectList.end(),
                    selectUnit);

    std::for_each(  assignedUndergroundUnitList.begin(),
                    assignedUndergroundUnitList.end(),
                    selectUnit);

    std::for_each(  assignedAirUnitList.begin(),
                    assignedAirUnitList.end(),
                    selectUnit);

    if (changed)
        game->selectionChanged();

    if (obj)
        *lastCheckedObject = obj;

    if (last_selected)
        *lastSelectedObject = last_selected;
}

template<typename Visitor>
void Tile::forEachUnit(Visitor&& visitor) const
{
    for (auto i : assignedInfantryList)
        visitor(i);

    for (auto i : assignedNonInfantryGroundObjectList)
        visitor(i);

    for (auto i : assignedUndergroundUnitList)
        visitor(i);

    for (auto i : assignedAirUnitList)
        visitor(i);
}
