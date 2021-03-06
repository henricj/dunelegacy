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

#include <Map.h>

#include <globals.h>

#include <Game.h>
#include <House.h>
#include <ScreenBorder.h>
#include <sand.h>

#include <units/UnitBase.h>
#include <units/InfantryBase.h>
#include <units/AirUnit.h>
#include <structures/StructureBase.h>

#include <climits>
#include <stack>
#include <set>

Map::Map(Game& game, int xSize, int ySize)
    : sizeX(xSize), sizeY(ySize), lastSinglySelectedObject(nullptr), pathfinder_(this), random_{game.randomFactory.create("Map")} {

    tiles.resize(sizeX * sizeY);

    if(game.getGameInitSettings().getGameOptions().startWithExploredMap) {
        this->for_all([](auto& tile) {
            for(auto h = 0; h < NUM_TEAMS; ++h)
                tile.setExplored(static_cast<HOUSETYPE>(h), true);
        });
    }

    init_tile_location();
    init_box_sets();
}


Map::~Map() = default;

void Map::load(InputStream& stream) {
    const auto x = stream.readSint32();
    const auto y = stream.readSint32();

    if (x != sizeX || y != sizeY)
        THROW(std::runtime_error, "Map load size mismatch (%d, %d) != (%d, %d)", x, y, sizeX, sizeY);

    assert(tiles.size() == sizeX * sizeY);

    for (auto& tile : tiles)
        tile.load(stream);

    auto state = stream.readUint8Vector();
    if(state.size() != decltype(random_)::state_bytes) THROW(std::runtime_error, "Random state size mismatch!");
    random_.setState(gsl::span<const Uint8, Random::state_bytes>{state});

    init_tile_location();
}

void Map::save(OutputStream& stream, Uint32 gameCycleCount) const {
    stream.writeSint32(sizeX);
    stream.writeSint32(sizeY);

    for (const auto & tile : tiles)
        tile.save(stream, gameCycleCount);

    stream.writeUint8Vector(random_.getState());
}

void Map::init_tile_location() {
    for (auto i = 0; i < sizeX; ++i) {
        for (auto j = 0; j < sizeY; ++j) {
            tiles[tile_index(i, j)].location = Coord(i, j);
        }
    }
}

void Map::createSandRegions() {
    std::stack<Tile*> tileQueue;
    std::vector<bool> visited(tiles.size());

    for (auto& tile : tiles)
        tile.setSandRegion(NONE_ID);

    Uint32 region = 0;
    for (auto& tile : tiles) {
        if (!tile.isRock() && !visited[tile_index(tile.location.x, tile.location.y)]) {
            tileQueue.push(&tile);

            while (!tileQueue.empty()) {
                auto *const pTile = tileQueue.top();
                tileQueue.pop();

                pTile->setSandRegion(region);

                index_for_each_angle(pTile->location.x, pTile->location.y,
                    [&](ANGLETYPE angle, int index) {
                        if (!visited[index])
                            return;

                        auto *const tile_angle = &tiles[index];

                        if (!tile_angle->isRock()) {
                            tileQueue.push(tile_angle);
                            visited[index] = true;
                        }
                    });
            }
            region++;
        }
    }
}

void Map::damage(const GameContext& context, Uint32 damagerID, House* damagerOwner, const Coord& realPos, Uint32 bulletID,
                 FixPoint damage, int damageRadius, bool air) {
    const auto location = Coord(realPos.x/TILESIZE, realPos.y/TILESIZE);

    std::vector<Dune::object_id_type> affectedAirUnits;
    std::vector<Dune::object_id_type> affectedGroundAndUndergroundUnits;

    for_each(location.x - 2, location.y - 2, location.x + 2, location.y + 2, [&](Tile& t) {
        std::copy(std::begin(t.getAirUnitList()), std::end(t.getAirUnitList()), std::back_inserter(affectedAirUnits));
        std::copy(std::begin(t.getInfantryList()), std::end(t.getInfantryList()),
                  std::back_inserter(affectedGroundAndUndergroundUnits));
        std::copy(std::begin(t.getUndergroundUnitList()), std::end(t.getUndergroundUnitList()),
                  std::back_inserter(affectedGroundAndUndergroundUnits));
        std::copy(std::begin(t.getNonInfantryGroundObjectList()), std::end(t.getNonInfantryGroundObjectList()),
                  std::back_inserter(affectedGroundAndUndergroundUnits));
    });

    if(bulletID == Bullet_Sandworm) {
        for(auto objectID : affectedGroundAndUndergroundUnits) {
            auto *pObject = context.objectManager.getObject(objectID);
            if(pObject
                && (pObject->getItemID() != Unit_Sandworm)
                && (pObject->isAGroundUnit() || pObject->isInfantry())
                && (pObject->getLocation() == location))
            {
                pObject->setVisible(VIS_ALL, false);
                pObject->handleDamage(context, lround(damage), damagerID, damagerOwner);
            }
        }
    } else {
        if(air) {
            // air damage
            if((bulletID == Bullet_DRocket) || (bulletID == Bullet_Rocket) || (bulletID == Bullet_TurretRocket)|| (bulletID == Bullet_SmallRocket)) {
                for(auto objectID : affectedAirUnits) {
                    auto *pAirUnit = dynamic_cast<AirUnit*>(context.objectManager.getObject(objectID));
                    if(pAirUnit == nullptr)
                        continue;

                    const auto centerPoint = pAirUnit->getCenterPoint();
                    const auto distance = lround(distanceFrom(centerPoint, realPos));

                    if(distance > damageRadius)
                        continue;

                    if(bulletID == Bullet_DRocket) {
                        if((pAirUnit->getItemID() != Unit_Carryall) && (pAirUnit->getItemID() != Unit_Sandworm) && (pAirUnit->getItemID() != Unit_Frigate)) {
                            // try to deviate
                            if(random_.randFixPoint() < getDeviateWeakness(static_cast<HOUSETYPE>(pAirUnit->getOriginalHouseID()))) {
                                pAirUnit->deviate(context, damagerOwner);
                            }
                        }
                    } else {
                        const auto scaledDamage = lround(damage) >> (distance/4 + 1);
                        pAirUnit->handleDamage(context, scaledDamage, damagerID, damagerOwner);
                    }
                }
            }
        } else {
            // non air damage
            for(auto objectID : affectedGroundAndUndergroundUnits) {
                auto *const pObject = context.objectManager.getObject(objectID);

                if(pObject && pObject->isAStructure()) {
                    auto *pStructure = static_cast<StructureBase*>(pObject);

                    const auto topLeftCorner = pStructure->getLocation()*TILESIZE;
                    const auto bottomRightCorner = topLeftCorner + pStructure->getStructureSize()*TILESIZE;

                    if(realPos.x >= topLeftCorner.x && realPos.y >= topLeftCorner.y && realPos.x < bottomRightCorner.x && realPos.y < bottomRightCorner.y) {
                        pStructure->handleDamage(context, lround(damage), damagerID, damagerOwner);

                        if( (bulletID == Bullet_LargeRocket || bulletID == Bullet_Rocket || bulletID == Bullet_TurretRocket || bulletID == Bullet_SmallRocket)
                            && (pStructure->getHealth() < pStructure->getMaxHealth()/2)) {
                            if(pStructure->getNumSmoke() < 5) {
                                pStructure->addSmoke(realPos, context.game.getGameCycleCount());
                            }
                        }
                    }
                } else if(pObject && pObject->isAUnit()) {
                    auto *const pUnit = static_cast<UnitBase*>(pObject);

                    const auto centerPoint = pUnit->getCenterPoint();
                    const auto distance = lround(distanceFrom(centerPoint, realPos));

                    if(distance <= damageRadius) {
                        if(bulletID == Bullet_DRocket) {
                            if((pUnit->getItemID() != Unit_Carryall) && (pUnit->getItemID() != Unit_Sandworm) && (pUnit->getItemID() != Unit_Frigate)) {
                                // try to deviate
                                if(random_.randFixPoint() < getDeviateWeakness(static_cast<HOUSETYPE>(pUnit->getOriginalHouseID()))) {
                                    pUnit->deviate(context, damagerOwner);
                                }
                            }
                        } else if(bulletID == Bullet_Sonic) {
                            pUnit->handleDamage(context, lround(damage), damagerID, damagerOwner);
                        } else {
                            const auto scaledDamage = lround(damage) >> (distance/16 + 1);
                            pUnit->handleDamage(context, scaledDamage, damagerID, damagerOwner);
                        }
                    }
                }
            }

            auto *const pTile = tryGetTile(location.x, location.y);

            if(pTile && ((bulletID == Bullet_Rocket) || (bulletID == Bullet_TurretRocket) ||
                         (bulletID == Bullet_SmallRocket) || (bulletID == Bullet_LargeRocket))) {
                if(auto* object = pTile->getGroundObject(context.objectManager); !object || !object->isAStructure()) {
                    const auto type = pTile->getType();

                    if(((type == TERRAINTYPE::Terrain_Rock) &&
                        (pTile->getTerrainTile() == Tile::TERRAINTILETYPE::TerrainTile_RockFull)) ||
                       (type == TERRAINTYPE::Terrain_Slab)) {
                        if(type == TERRAINTYPE::Terrain_Slab) {
                            pTile->setType(context, TERRAINTYPE::Terrain_Rock);
                            pTile->setDestroyedStructureTile(Destroyed1x1Structure);
                            pTile->setOwner(static_cast<HOUSETYPE>(NONE_ID));
                        }

                        const auto damage = (bulletID == BulletID_enum::Bullet_SmallRocket)
                                                ? Tile::ROCKDAMAGETYPE::RockDamage1
                                                : Tile::ROCKDAMAGETYPE::RockDamage2;

                        pTile->addDamage(Tile::TerrainDamage_enum::Terrain_RockDamage, static_cast<int>(damage),
                                         realPos);

                    } else if((type == TERRAINTYPE::Terrain_Sand) || (type == TERRAINTYPE::Terrain_Spice)) {
                        const auto damage_tile =
                            bulletID == BulletID_enum::Bullet_SmallRocket
                                ? random_.rand(static_cast<int>(Tile::SANDDAMAGETYPE::SandDamage1),
                                                              static_cast<int>(Tile::SANDDAMAGETYPE::SandDamage2))
                                : random_.rand(static_cast<int>(Tile::SANDDAMAGETYPE::SandDamage3),
                                                              static_cast<int>(Tile::SANDDAMAGETYPE::SandDamage4));

                        pTile->addDamage(Tile::TerrainDamage_enum::Terrain_SandDamage, damage_tile, realPos);
                    }
                }
            }
        }
    }

    if ((bulletID != Bullet_Sonic) && (bulletID != Bullet_Sandworm)) {
        auto *const tile = tryGetTile(location.x, location.y);

        if (tile && tile->isSpiceBloom()) {
            tile->triggerSpiceBloom(context, damagerOwner);
        }
    }
}

/**
    Check each tile which surrounds the building location to make sure there
    is no building. We want to ensure we don't block units in and have
    traffic lanes for our troops
**/

bool Map::isAStructureGap(const GameContext& context, int x, int y, int buildingSizeX, int buildingSizeY) const {

    // Spacing rules don't apply for rocket turrets
    if(buildingSizeX == 1){
        return true;
    }

    const auto xMin = x - 1;
    const auto xMax = x + buildingSizeX + 1;
    const auto yMin = y - 1;
    const auto yMax = y + buildingSizeY + 1;

    // I need some more conditions to make it ignore units
    const auto predicate = [&](const Tile * tile) {
        return !tile || (tile->hasAStructure(context.objectManager) && !tile->isConcrete());
    };

    //Corners are ok as units can get through

    // Vertical lines
    for (auto j = yMin; j < yMax; ++j) {
        if (predicate(tryGetTile(xMin, j)))
            return false;
        if (predicate(tryGetTile(xMax, j)))
            return false;
    }
    // Horizontal lines, but skipping the corners we already checked.
    for (auto i = xMin + 1; i < xMax - 1; ++i) {
        if (predicate(tryGetTile(i, yMin)))
            return false;
        if (predicate(tryGetTile(i, yMax)))
            return false;
    }

    return true;
}

bool Map::okayToPlaceStructure(int x, int y, int buildingSizeX, int buildingSizeY, bool tilesRequired, const House* pHouse, bool bIgnoreUnits) const {
    bool withinBuildRange = false;

    for(auto i = x; i < x + buildingSizeX; i++) {
        for(auto j = y; j < y + buildingSizeY; j++) {

            const auto *const pTile = tryGetTile(i,j);

            if (!pTile)
                return false;

            if(!pTile->isRock() || (tilesRequired && !pTile->isConcrete()) || (!bIgnoreUnits && pTile->isBlocked())) {
                return false;
            }

            if((pHouse == nullptr) || isWithinBuildRange(i, j, pHouse)) {
                withinBuildRange = true;
            }
        }
    }
    return withinBuildRange;
}


bool Map::isWithinBuildRange(int x, int y, const House* pHouse) const {
    for (auto i = x - BUILDRANGE; i <= x + BUILDRANGE; i++) {
        for (auto j = y - BUILDRANGE; j <= y + BUILDRANGE; j++) {
            const auto *const tile = tryGetTile(i, j);

            if (tile && tile->getOwner() == pHouse->getHouseID())
                return true;
        }
    }

    return false;
}

/**
    This method figures out the direction of tile pos relative to tile source.
    \param  source  the starting point
    \param  pos     the destination
    \return one of RIGHT, RIGHTUP, UP, LEFTUP, LEFT, LEFTDOWN, DOWN, RIGHTDOWN or INVALID
*/
ANGLETYPE Map::getPosAngle(const Coord& source, const Coord& pos) {
    if(pos.x > source.x) {
        if(pos.y > source.y) {
            return ANGLETYPE::RIGHTDOWN;
        } if(pos.y < source.y) {

            return ANGLETYPE::RIGHTUP;

        } else {

            return ANGLETYPE::RIGHT;

        }
    } else if(pos.x < source.x) {
        if(pos.y > source.y) {
            return ANGLETYPE::LEFTDOWN;
        } if(pos.y < source.y) {

            return ANGLETYPE::LEFTUP;

        } else {

            return ANGLETYPE::LEFT;

        }
    } else {
        if(pos.y > source.y) {
            return ANGLETYPE::DOWN;
        } if(pos.y < source.y) {

            return ANGLETYPE::UP;

        } else {

            THROW(std::runtime_error, "Map::getPosAngle(): Impossible angle!");

        }
    }
}

/**
    This method calculates the coordinate of one of the neighbor tiles of source
    \param  angle   one of RIGHT, RIGHTUP, UP, LEFTUP, LEFT, LEFTDOWN, DOWN, RIGHTDOWN
    \param  source  the tile to calculate neighbor tiles from
*/
Coord Map::getMapPos(ANGLETYPE angle, const Coord& source) {
    // clang-format off
    switch (angle)
    {
        case ANGLETYPE::RIGHT:      return Coord(source.x + 1 , source.y    );  break;
        case ANGLETYPE::RIGHTUP:    return Coord(source.x + 1 , source.y - 1);  break;
        case ANGLETYPE::UP:         return Coord(source.x     , source.y - 1);  break;
        case ANGLETYPE::LEFTUP:     return Coord(source.x - 1 , source.y - 1);  break;
        case ANGLETYPE::LEFT:       return Coord(source.x - 1 , source.y    );  break;
        case ANGLETYPE::LEFTDOWN:   return Coord(source.x - 1 , source.y + 1);  break;
        case ANGLETYPE::DOWN:       return Coord(source.x     , source.y + 1);  break;
        case ANGLETYPE::RIGHTDOWN:  return Coord(source.x + 1 , source.y + 1);  break;
        default:                    return Coord(source.x     , source.y    );  break;
    }
    // clang-format on
}

//building size is num squares
Coord Map::findDeploySpot(UnitBase* pUnit, const Coord& origin, Random& random, const Coord& gatherPoint, const Coord& buildingSize) {
    if (pUnit->isAFlyingUnit()) {
        return origin + Coord(random.rand(0, buildingSize.x - 1), random.rand(0, buildingSize.y - 1));
    }

    auto closestDistance = FixPt_MAX;
    Coord       closestPoint;

    const auto found = search_all_by_box_edge(origin.x, origin.y, buildingSize, random,
        [&](const Tile& t) {
            if (!pUnit->canPassTile(&t))
                return SearchResult::NotDone;

            if (pUnit->isTracked()) {
                if (t.hasInfantry()) {
                    // we do not deploy on enemy infantry
                    return SearchResult::NotDone;
                }
            }

            if (gatherPoint.isInvalid()) {
                closestPoint = t.location;
                return SearchResult::Done;
            }

            if (blockDistance(t.location, gatherPoint) < closestDistance) {
                closestDistance = blockDistance(t.location, gatherPoint);
                closestPoint = t.location;
                return SearchResult::DoneAtDepth;
            }

            return SearchResult::NotDone;
        });

    if (found)
        return closestPoint;

    sdl2::log_info("Warning: Cannot find deploy position because the map is full!");

    return Coord::Invalid();
}

/**
    This method finds the tile which is at a map border and is at minimal distance to the structure
    specified by origin and buildingsSize. This method is especcially useful for Carryalls and Frigates
    that have to enter the map to deliver units.
    \param origin           the position of the structure in map coordinates
    \param buildingSize    the number of tiles occupied by the building (e.g. 3x2 for refinery)
*/
Coord Map::findClosestEdgePoint(const Coord& origin, const Coord& buildingSize) const {
    auto closestDistance = INT_MAX;
    Coord closestPoint;

    if(origin.x < (sizeX - (origin.x + buildingSize.x))) {
        closestPoint.x = 0;
        closestDistance = origin.x;
    } else {
        closestPoint.x = sizeX - 1;
        closestDistance = sizeX - (origin.x + buildingSize.x);
    }
    closestPoint.y = origin.y;

    if(origin.y < closestDistance) {
        closestPoint.x = origin.x;
        closestPoint.y = 0;
        closestDistance = origin.y;
    }

    if((sizeY - (origin.y + buildingSize.y)) < closestDistance) {
        closestPoint.x = origin.x;
        closestPoint.y = sizeY - 1;
        // closestDistance = origin.y;  //< Not needed anymore
    }

    return closestPoint;
}


void Map::removeObjectFromMap(Uint32 objectID) {
    // TODO: Should we try the object manager first?
    // At worst, if we find the object, we can use the location
    // plus the size of the building to avoid going through the
    // whole map.
    for (auto& tile : tiles)
        tile.unassignObject(objectID);

    removedObjects.push(objectID);
}

void Map::selectObjects(const House* pHouse, int x1, int y1, int x2, int y2, int realX, int realY, bool objectARGMode) {

    ObjectBase *lastCheckedObject = nullptr;
    ObjectBase *lastSelectedObject = nullptr;

    //if selection rectangle is checking only one tile and has shift selected we want to add/ remove that unit from the selected group of units
    if(!objectARGMode) {
        currentGame->unselectAll(currentGame->getSelectedList());
        currentGame->getSelectedList().clear();
        currentGame->selectionChanged();
    }

    if((x1 == x2) && (y1 == y2)) {
        auto *const tile_center = tryGetTile(x1, y1);

        if (!tile_center)
            return;

        if(tile_center->isExploredByTeam(currentGame.get(), pHouse->getTeamID()) || debug) {
            lastCheckedObject = tile_center->getObjectAt(currentGame->getObjectManager(), realX, realY);
        } else {
            lastCheckedObject = nullptr;
        }

        if((lastCheckedObject != nullptr) && (lastCheckedObject->getOwner() == pHouse)) {
            if((lastCheckedObject == lastSinglySelectedObject) && ( !lastCheckedObject->isAStructure())) {
                for(auto i = screenborder->getTopLeftTile().x; i <= screenborder->getBottomRightTile().x; i++) {
                    for(auto j = screenborder->getTopLeftTile().y; j <= screenborder->getBottomRightTile().y; j++) {
                        auto *const tile = tryGetTile(i, j);

                        if (tile && tile->hasAnObject()) {
                            tile->selectAllPlayersUnitsOfType(currentGame.get(), pHouse->getHouseID(), lastSinglySelectedObject->getItemID(), &lastCheckedObject, &lastSelectedObject);
                        }
                    }
                }
                lastSinglySelectedObject = nullptr;

            } else if(!lastCheckedObject->isSelected()) {

                lastCheckedObject->setSelected(true);
                currentGame->getSelectedList().insert(lastCheckedObject->getObjectID());
                currentGame->selectionChanged();
                lastSelectedObject = lastCheckedObject;
                lastSinglySelectedObject = lastSelectedObject;

            } else if(objectARGMode) {
                //holding down shift, unselect this unit
                lastCheckedObject->setSelected(false);
                currentGame->getSelectedList().erase(lastCheckedObject->getObjectID());
                currentGame->selectionChanged();
            }

        } else {
            lastSinglySelectedObject = nullptr;
        }

    } else {
        lastSinglySelectedObject = nullptr;
        for(auto i = std::min(x1, x2); i <= std::max(x1, x2); i++) {
            for(auto j = std::min(y1, y2); j <= std::max(y1, y2); j++) {
                auto *const tile = tryGetTile(i, j);

                if (tile && tile->hasAnObject() && tile->isExploredByTeam(currentGame.get(), pHouse->getTeamID()) && !tile->isFoggedByTeam(currentGame.get(), pHouse->getTeamID())) {
                    tile->selectAllPlayersUnits(currentGame.get(), pHouse->getHouseID(), &lastCheckedObject, &lastSelectedObject);
                }
            }
        }
    }

    //select an enemy unit if none of your units found
    if(currentGame->getSelectedList().empty() && (lastCheckedObject != nullptr) && !lastCheckedObject->isSelected()) {
        lastCheckedObject->setSelected(true);
        lastSelectedObject = lastCheckedObject;
        currentGame->getSelectedList().insert(lastCheckedObject->getObjectID());
        currentGame->selectionChanged();
    } else if (lastSelectedObject != nullptr) {
        lastSelectedObject->playSelectSound();  //we only want one unit responding
    }
}


bool Map::findSpice(Coord& destination, const Coord& origin) {

    return search_all_by_box_edge(origin.x, origin.y, random_,
        [&](const Tile& t) {
            if (t.hasAGroundObject() || !t.hasSpice())
                return SearchResult::NotDone;

            destination = t.location;
            return SearchResult::Done;
        });
}

/**
    This method fixes surounding thick spice tiles after spice gone to make things look smooth.
    \param coord    the coordinate where spice was removed from
*/
void Map::spiceRemoved(const GameContext& context, const Coord& coord) {

    auto *const pCenterTile = tryGetTile(coord.x , coord.y);

    if(!pCenterTile || pCenterTile->getType() != Terrain_Sand) return;

    //thickspice tiles can't handle non-(thick)spice tiles next to them, if this happens after changes, make it non thick
    //only check tile right, up, left and down of this one
    for_each_neighbor(coord.x, coord.y, [&](Tile& t) {
        if (t.isThickSpice())
            t.setType(context, Terrain_Spice);
    });
}

void Map::viewMap(HOUSETYPE houseID, const Coord& location, const int maxViewRange) {

//makes map viewable in an area like as shown below
//
//                    *
//                  *****
//                  *****
//                 ***T***
//                  *****
//                  *****
//                    *

    const auto cycle_count = currentGame->getGameCycleCount();

    for_each_filter(location.x - maxViewRange, location.y - maxViewRange,
        location.x + maxViewRange + 1, location.y + maxViewRange + 1,
        [&](int x, int y) {
            const auto distance = maxViewRange <= 1
                ? maximumDistance(location, { x, y })
                : blockDistanceApprox(location, { x, y });
            return distance <= maxViewRange;
        },
        [&](Tile& t) {
            t.setExplored(houseID, cycle_count);
        });
}

/**
    Creates a spice field of the given radius at the given location.
    \param  location            the location in tile coordinates
    \param  radius              the radius in tiles (0 = only one tile is filled)
    \param  centerIsThickSpice  if set the center is filled with thick spice
*/
void Map::createSpiceField(const GameContext& context, Coord location, int radius, bool centerIsThickSpice) {

    for_each_filter(location.x - radius, location.y - radius, location.x + radius, location.y + radius,
        [&](int x, int y) { return distanceFrom(location, { x, y }) <= radius; },
        [&](Tile& t) {
            if (t.isSand()) {
                const auto terrain = centerIsThickSpice && (t.location == location)
                    ? Terrain_ThickSpice : Terrain_Spice;

                t.setType(context, terrain);
            }
        });
}

Map::BoxOffsets::BoxOffsets(int size, Coord box)
{
    if (size > static_cast<int>(box_sets_.size()))
        box_sets_.resize(size);

    for (auto depth = 1; depth <= size; ++depth) {

        auto& set = box_sets_[depth - 1];

        if (!set.empty())
            return;

        const auto border_length = 2 * (2 * depth + box.x - 1 + 2 * depth + box.y - 1);
        set.reserve(border_length);

        for (auto i = -depth; i < depth + box.y; ++i)
        {
            set.emplace_back(-depth, i); // Left
            set.emplace_back(depth + box.x - 1, i); // Right
        }

        // We skip the corners since they were already handled above.
        for (auto i = -depth + 1; i < depth + box.x - 1; ++i)
        {
            set.emplace_back(i, -depth); // Top
            set.emplace_back(i, depth + box.y - 1); // Bottom
        }

        assert(border_length == set.size());
    }
}

void Map::init_box_sets()
{
    auto size = std::max(sizeX, sizeY);

    offsets_ = std::make_unique<BoxOffsets>(size);
    offsets_2x2_ = std::make_unique<BoxOffsets>(size, Coord(2, 2));
    offsets_2x3_ = std::make_unique<BoxOffsets>(size, Coord(2, 3));
    offsets_3x2_ = std::make_unique<BoxOffsets>(size, Coord(3, 2));
    offsets_3x3_ = std::make_unique<BoxOffsets>(size, Coord(3, 3));
}
