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

#ifndef MAP_H
#define MAP_H

#include "ObjectBase.h"
#include "misc/Random.h"
#include <AStarSearch.h>
#include <Tile.h>
#include <misc/InputStream.h>
#include <misc/OutputStream.h>
#include <misc/exceptions.h>

#include <queue>

class Map final {
public:
    /**
        Creates a map of size xSize x ySize. The map is initialized with all tiles of type Terrain_Sand.
    */
    Map(Game& game, int xSize, int ySize);
    Map(const Map& o) = delete;
    Map(Map&& o)      = delete;
    ~Map();

    Map& operator=(const Map&) = delete;
    Map& operator=(Map&&)      = delete;

    void load(InputStream& stream);
    void save(OutputStream& stream, uint32_t gameCycleCount) const;

    void createSandRegions();
    void damage(const GameContext& context, uint32_t damagerID, House* damagerOwner, const Coord& realPos,
                uint32_t bulletID, FixPoint damage, int damageRadius, bool air);
    static Coord getMapPos(ANGLETYPE angle, const Coord& source);
    void removeObjectFromMap(uint32_t objectID);
    void spiceRemoved(const GameContext& context, const Coord& coord);
    void selectObjects(const House* pHouse, int x1, int y1, int x2, int y2, int realX, int realY, bool objectARGMode);

    void viewMap(HOUSETYPE houseID, const Coord& location, int maxViewRange);
    void viewMap(HOUSETYPE houseID, int x, int y, const int maxViewRange) {
        viewMap(houseID, Coord(x, y), maxViewRange);
    }

    bool findSpice(Coord& destination, const Coord& origin);
    bool okayToPlaceStructure(int x, int y, int buildingSizeX, int buildingSizeY, bool tilesRequired,
                              const House* pHouse, bool bIgnoreUnits = false) const;
    [[nodiscard]] bool
    isAStructureGap(const GameContext& context, int x, int y, int buildingSizeX,
                    int buildingSizeY) const; // Allows AI to check to see if a gap exists between the current structure
    bool isWithinBuildRange(int x, int y, const House* pHouse) const;
    static ANGLETYPE getPosAngle(const Coord& source, const Coord& pos);
    [[nodiscard]] Coord findClosestEdgePoint(const Coord& origin, const Coord& buildingSize) const;

    Coord findDeploySpot(UnitBase* pUnit, const Coord& origin, Random& randomGen,
                         const Coord& gatherPoint = Coord::Invalid(), const Coord& buildingSize = Coord(0, 0));
    // building size is num squares
    Coord findDeploySpot(UnitBase* pUnit, const Coord& origin, const Coord& gatherPoint = Coord::Invalid(),
                         const Coord& buildingSize = Coord(0, 0)) {
        return findDeploySpot(pUnit, origin, random_, gatherPoint, buildingSize);
    }

    void createSpiceField(const GameContext& context, Coord location, int radius, bool centerIsThickSpice = false);

    [[nodiscard]] int32_t getSizeX() const noexcept { return sizeX; }

    [[nodiscard]] int32_t getSizeY() const noexcept { return sizeY; }

    [[nodiscard]] int getKey(const Tile& tile) const noexcept {
        return tile_index(tile.getLocation().x, tile.getLocation().y);
    }

    [[nodiscard]] int getKey(int xPos, int yPos) const {
        if (!tileExists(xPos, yPos))
            THROW(std::out_of_range, "Tile (%d, %d) does not exist!", xPos, yPos);

        return tile_index(xPos, yPos);
    }

    [[nodiscard]] bool tileExists(int xPos, int yPos) const noexcept {
        return ((xPos >= 0) && (xPos < sizeX) && (yPos >= 0) && (yPos < sizeY));
    }

    [[nodiscard]] bool tileExists(const Coord& pos) const noexcept { return tileExists(pos.x, pos.y); }

    Tile* tryGetTile(int xPos, int yPos) noexcept {
        if (!tileExists(xPos, yPos))
            return nullptr;

        return &tiles[tile_index(xPos, yPos)];
    }

    [[nodiscard]] const Tile* tryGetTile(int xPos, int yPos) const noexcept {
        if (!tileExists(xPos, yPos))
            return nullptr;

        return &tiles[tile_index(xPos, yPos)];
    }

    [[nodiscard]] const Tile* getTile(int xPos, int yPos) const {
        const auto* const tile = tryGetTile(xPos, yPos);

        if (!tile)
            THROW(std::out_of_range, "Tile (%d, %d) does not exist!", xPos, yPos);

        return tile;
    }

    Tile* getTile(int xPos, int yPos) {
        auto* const tile = tryGetTile(xPos, yPos);

        if (!tile)
            THROW(std::out_of_range, "Tile (%d, %d) does not exist!", xPos, yPos);

        return tile;
    }

    [[nodiscard]] const Tile* getTile(const Coord& location) const { return getTile(location.x, location.y); }

    Tile* getTile(const Coord& location) { return getTile(location.x, location.y); }

    template<class... Args>
    void add_bullet(Args&&... args) {
        dune::globals::bulletList.push_back(std::make_unique<Bullet>(std::forward<Args>(args)...));
    }

    template<typename F>
    void for_all(F&& f) {
        std::for_each(std::begin(tiles), std::end(tiles), f);
    }

    template<typename F>
    void for_all(F&& f) const {
        std::for_each(std::begin(tiles), std::end(tiles), f);
    }

    template<typename F>
    void for_all_xy(F&& f) {
        for (auto x = 0; x < sizeX; ++x) {
            for (auto y = 0; y < sizeY; ++y) {
                f(x, y, tiles[tile_index(x, y)]);
            }
        }
    }

    template<typename F>
    void for_all_xy(F&& f) const {
        for (auto x = 0; x < sizeX; ++x) {
            for (auto y = 0; y < sizeY; ++y) {
                f(x, y, tiles[tile_index(x, y)]);
            }
        }
    }

protected:
    template<typename F>
    void location_for_each(int x1, int y1, int x2, int y2, F&& f) const {
        static_assert(std::is_invocable_v<F, int, int>, "The function must be of the form void F(int, int)");

        if (x1 < 0)
            x1 = 0;
        if (x2 > sizeX)
            x2 = sizeX;

        if (y1 < 0)
            y1 = 0;
        if (y2 > sizeY)
            y2 = sizeY;

        assert(x1 >= 0 && x2 <= sizeX);
        assert(y1 >= 0 && y2 <= sizeY);

        for (auto x = x1; x < x2; ++x) {
            for (auto y = y1; y < y2; ++y) {
                f(x, y);
            }
        }
    }

    template<typename Predicate>
    bool location_find(int x1, int y1, int x2, int y2, Predicate&& predicate) const {
        static_assert(std::is_invocable_r_v<bool, Predicate, int, int>,
                      "The Predicate must be of the form bool Predicate(int, int)");

        if (x1 < 0)
            x1 = 0;
        if (x2 > sizeX)
            x2 = sizeX;

        if (y1 < 0)
            y1 = 0;
        if (y2 > sizeY)
            y2 = sizeY;

        assert(x1 >= 0 && x2 <= sizeX);
        assert(y1 >= 0 && y2 <= sizeY);

        for (auto x = x1; x < x2; ++x) {
            for (auto y = y1; y < y2; ++y) {
                if (predicate(x, y))
                    return true;
            }
        }

        return false;
    }

    template<typename F>
    void coord_for_each(int x1, int y1, int x2, int y2, F&& f) const {
        location_for_each(x1, y1, x2, y2, [&](int x, int y) { f(Coord(x, y)); });
    }
    template<typename Filter, typename F>
    void coord_for_each_filter(int x1, int y1, int x2, int y2, F&& f) const {
        location_for_each(x1, y1, x2, y2, [&](int x, int y) { f(Coord(x, y)); });
    }
    template<typename F>
    void index_for_each(int x1, int y1, int x2, int y2, F&& f) const {
        location_for_each(x1, y1, x2, y2, [&](int x, int y) { f(tile_index(x, y)); });
    }
    template<typename Predicate>
    bool index_find(int x1, int y1, int x2, int y2, Predicate&& predicate) const {
        return location_find(x1, y1, x2, y2, [&](int x, int y) { return predicate(tile_index(x, y)); });
    }
    template<typename Filter, typename F>
    void index_for_each_filter(int x1, int y1, int x2, int y2, Filter&& filter, F&& f) const {
        location_for_each(x1, y1, x2, y2, [&](int x, int y) {
            if (filter(x, y))
                f(tile_index(x, y));
        });
    }
    class BoxOffsets {
        std::vector<std::vector<std::pair<int, int>>> box_sets_;

    public:
        BoxOffsets(int size, Coord box = Coord(1, 1));
        std::vector<std::pair<int, int>>& search_set(size_t depth) { return box_sets_[depth - 1]; }

        [[nodiscard]] size_t max_depth() const noexcept { return box_sets_.size(); }
    };

public:
    template<typename F>
    void for_each(int x1, int y1, int x2, int y2, F&& f) const {
        index_for_each(x1, y1, x2, y2, [&](int index) { f(tiles[index]); });
    }

    template<typename F>
    void for_each(int x1, int y1, int x2, int y2, F&& f) {
        index_for_each(x1, y1, x2, y2, [&](int index) { f(tiles[index]); });
    }

    template<typename Predicate>
    bool find(int x1, int y1, int x2, int y2, Predicate&& predicate) const {
        static_assert(std::is_invocable_r_v<bool, Predicate, Tile&>,
                      "The Predicate must of the form bool Predicate(const Tile&)");
        return index_find(x1, y1, x2, y2, [&](int index) { return predicate(tiles[index]); });
    }

    template<typename Predicate>
    bool find(int x1, int y1, int x2, int y2, Predicate&& predicate) {
        static_assert(std::is_invocable_r_v<bool, Predicate, Tile&>,
                      "The Predicate must of the form bool Predicate(Tile&)");
        return index_find(x1, y1, x2, y2, [&](int index) { return predicate(tiles[index]); });
    }

    template<typename Filter, typename F>
    void for_each_filter(int x1, int y1, int x2, int y2, Filter&& filter, F&& f) const {
        index_for_each_filter(x1, y1, x2, y2, filter, [&](int index) { f(tiles[index]); });
    }

    template<typename Filter, typename F>
    void for_each_filter(int x1, int y1, int x2, int y2, Filter&& filter, F&& f) {
        index_for_each_filter(x1, y1, x2, y2, filter, [&](int index) { f(tiles[index]); });
    }

    enum class SearchResult { NotDone, DoneAtDepth, Done };

protected:
    template<typename Offsets, typename Predicate>
    SearchResult
    search_random_offsets(int x, int y, Offsets&& offsets, Random& generator, Predicate&& predicate) const {

        const auto size = offsets.size();
        auto found      = false;

        // We do an incremental Fisher-Yates shuffle.  This should be as
        // random as the generator, and guarantees that each tile will
        // be visited exactly once.
        for (auto i = 0u; i < size; ++i) {
            std::swap(offsets[i], offsets[generator.rand(i, size - 1)]);

            const auto ranX = x + offsets[i].first;
            const auto ranY = y + offsets[i].second;

            const auto tile = tryGetTile(ranX, ranY);

            if (!tile)
                continue;

            const auto result = predicate(*tile);

            if (SearchResult::Done == result)
                return SearchResult::Done;

            if (SearchResult::DoneAtDepth == result)
                found = true;
        }

        return found ? SearchResult::DoneAtDepth : SearchResult::NotDone;
    }

    template<typename Predicate>
    SearchResult
    search_box_edge(int x, int y, int depth, BoxOffsets* offsets, Random& generator, Predicate&& predicate) const {

        if (0 == depth) {
            const auto* const tile = tryGetTile(x, y);
            if (!tile)
                return SearchResult::NotDone;

            return predicate(*tile);
        }

        return search_random_offsets(x, y, offsets->search_set(depth), generator, predicate);
    }

    template<typename Predicate>
    bool search_all_by_box_edge(int x, int y, BoxOffsets* offsets, Random& generator, Predicate&& predicate) const {

        for (auto depth = 0u; depth <= offsets->max_depth(); ++depth) {
            const auto ret = search_box_edge(x, y, depth, offsets, generator, predicate);

            if (SearchResult::NotDone != ret)
                return true;
        }

        return false;
    }

    template<typename Predicate>
    bool search_all_by_box_edge(int x, int y, Random& generator, Predicate&& predicate) const {
        return search_all_by_box_edge(x, y, offsets_.get(), generator, predicate);
    }

    template<typename Predicate>
    bool search_all_by_box_edge_2x2(int x, int y, Random& generator, Predicate&& predicate) const {
        return search_all_by_box_edge(x, y, offsets_2x2_.get(), generator, predicate);
    }

    template<typename Predicate>
    bool search_all_by_box_edge_2x3(int x, int y, Random& generator, Predicate&& predicate) const {
        return search_all_by_box_edge(x, y, offsets_2x3_.get(), generator, predicate);
    }

    template<typename Predicate>
    bool search_all_by_box_edge_3x2(int x, int y, Random& generator, Predicate&& predicate) const {
        return search_all_by_box_edge(x, y, offsets_3x2_.get(), generator, predicate);
    }

    template<typename Predicate>
    bool search_all_by_box_edge_3x3(int x, int y, Random& generator, Predicate&& predicate) const {
        return search_all_by_box_edge(x, y, offsets_3x3_.get(), generator, predicate);
    }

public:
    template<typename Predicate>
    bool
    search_all_by_box_edge(int x, int y, const Coord& buildingSize, Random& generator, Predicate&& predicate) const {

        if (buildingSize == Coord{2, 2})
            return search_all_by_box_edge_2x2(x, y, generator, predicate);
        if (buildingSize == Coord{2, 3})
            return search_all_by_box_edge_2x3(x, y, generator, predicate);
        if (buildingSize == Coord{3, 2})
            return search_all_by_box_edge_3x2(x, y, generator, predicate);
        if (buildingSize == Coord{3, 3})
            return search_all_by_box_edge_3x3(x, y, generator, predicate);

        return search_all_by_box_edge(x, y, generator, predicate);
    }

protected:
    template<typename F>
    void index_for_each_angle(int x, int y, F&& f) {
        if (x >= 1) {
            if (tileExists(x - 1, y - 1))
                f(ANGLETYPE::LEFTUP, tile_index(x - 1, y - 1));
            if (tileExists(x - 1, y))
                f(ANGLETYPE::LEFT, tile_index(x - 1, y));
            if (tileExists(x - 1, y + 1))
                f(ANGLETYPE::LEFTDOWN, tile_index(x - 1, y + 1));
        }
        if (tileExists(x, y - 1))
            f(ANGLETYPE::UP, tile_index(x, y - 1));
        if (tileExists(x, y + 1))
            f(ANGLETYPE::DOWN, tile_index(x, y + 1));
        if (x + 1 < sizeX) {
            if (tileExists(x + 1, y - 1))
                f(ANGLETYPE::RIGHTUP, tile_index(x + 1, y - 1));
            if (tileExists(x + 1, y))
                f(ANGLETYPE::RIGHT, tile_index(x + 1, y));
            if (tileExists(x + 1, y + 1))
                f(ANGLETYPE::RIGHTDOWN, tile_index(x + 1, y + 1));
        }
    }

public:
    template<typename F>
    void for_each_angle(int x, int y, F&& f) {
        if (tileExists(x, y))
            index_for_each_angle(x, y, [&](ANGLETYPE angle, int index) { f(angle, tiles[index]); });
    }

    template<typename F>
    void for_each_neighbor(int x, int y, F&& f) {
        if (tileExists(x - 1, y))
            f(tiles[tile_index(x - 1, y)]);
        if (tileExists(x, y - 1))
            f(tiles[tile_index(x, y - 1)]);
        if (tileExists(x, y + 1))
            f(tiles[tile_index(x, y + 1)]);
        if (tileExists(x + 1, y))
            f(tiles[tile_index(x + 1, y)]);
    }

    template<bool EmptyTile = true, bool PredicateValue = true, int AllEmptyMask = EmptyTile ? 0x0f : 0,
             int AllTrueMask = 0x0f, typename F>
    int get_neighbor_mask(int x, int y, F&& predicate) {
        const auto e_up    = tileExists(x, y - 1);
        const auto e_right = tileExists(x + 1, y);
        const auto e_down  = tileExists(x, y + 1);
        const auto e_left  = tileExists(x - 1, y);

        if (!e_up && !e_right && !e_down && !e_left)
            return AllEmptyMask;

        auto mask = 0;

        if (EmptyTile) {
            if (!e_up || PredicateValue == predicate(tiles[tile_index(x, y - 1)]))
                mask |= 1 << 0;
            if (!e_right || PredicateValue == predicate(tiles[tile_index(x + 1, y)]))
                mask |= 1 << 1;
            if (!e_down || PredicateValue == predicate(tiles[tile_index(x, y + 1)]))
                mask |= 1 << 2;
            if (!e_left || PredicateValue == predicate(tiles[tile_index(x - 1, y)]))
                mask |= 1 << 3;
        } else {
            if (e_up && PredicateValue == predicate(tiles[tile_index(x, y - 1)]))
                mask |= 1 << 0;
            if (e_right && PredicateValue == predicate(tiles[tile_index(x + 1, y)]))
                mask |= 1 << 1;
            if (e_down && PredicateValue == predicate(tiles[tile_index(x, y + 1)]))
                mask |= 1 << 2;
            if (e_left && PredicateValue == predicate(tiles[tile_index(x - 1, y)]))
                mask |= 1 << 3;
        }

        if constexpr (AllTrueMask != 0x0f) {
            if (0x0f == mask)
                return AllTrueMask;
        }

        return mask;
    }

    bool find_path(UnitBase* pUnit, Coord start, Coord destination, std::vector<Coord>& path) {
        if (!tileExists(start.x, start.y))
            return false;
        if (!tileExists(destination.x, destination.y))
            return false;

        pathfinder_.Search(this, pUnit, start, destination);

        return pathfinder_.getFoundPath(this, path);
    }

    template<typename F>
    void consume_removed_objects(F&& f) {
        while (!removedObjects.empty()) {
            const auto objectID = removedObjects.front();
            removedObjects.pop();
            f(objectID);
        }
    }

    void removeSelection(uint32_t objectID) { removedObjects.push(objectID); }

    bool trySetTileType(const GameContext& context, int x, int y, TERRAINTYPE type) {
        auto* const tile = tryGetTile(x, y);
        if (!tile)
            return false;

        tile->setType(context, type);

        return true;
    }

    [[nodiscard]] bool hasAStructure(const GameContext& context, int x, int y) const {
        const auto* const tile = tryGetTile(x, y);
        if (!tile)
            return false;

        return tile->hasAStructure(context.objectManager);
    }

    [[nodiscard]] ObjectBase* getGroundObject(const GameContext& context, int x, int y) const {
        const auto* const tile = tryGetTile(x, y);
        if (!tile)
            return nullptr;

        return tile->getGroundObject(context.objectManager);
    }

    template<typename ObjectType>
    ObjectType* getGroundObject(const GameContext& context, int x, int y) const {
        static_assert(!std::is_abstract_v<ObjectType>, "ObjectType is abstract");
        static_assert(std::is_base_of_v<ObjectBase, ObjectType>, "ObjectType not derived from ObjectBase");

        auto* const object = getGroundObject(context, x, y);
        if (!object)
            return nullptr;

        if (object->getItemID() != ObjectType::item_id)
            return nullptr;

        return static_cast<ObjectType*>(object);
    }

    template<typename ObjectType>
    [[nodiscard]] bool hasAGroundObject(const GameContext& context, int x, int y) const {
        static_assert(!std::is_abstract_v<ObjectType>, "ObjectType is abstract");
        static_assert(std::is_base_of_v<ObjectBase, ObjectType>, "ObjectType not derived from ObjectBase");

        const auto* const tile = tryGetTile(x, y);
        if (!tile)
            return false;

        auto* const object = tile->getGroundObject(context.objectManager);
        if (!object)
            return false;

        return object->getItemID() == ObjectType::item_id;
    }

    [[nodiscard]] ObjectBase* tryGetObject(const GameContext& context, int x, int y) const {
        const auto* const tile = tryGetTile(x, y);
        if (!tile)
            return nullptr;

        return tile->getObject(context.objectManager);
    }

    [[nodiscard]] House* tryGetOwner(const GameContext& context, int x, int y) const {
        const auto* const object = tryGetObject(context, x, y);
        if (!object)
            return nullptr;

        return object->getOwner();
    }

    [[nodiscard]] InfantryBase* tryGetInfantry(const GameContext& context, int x, int y) const {
        const auto* const tile = tryGetTile(x, y);
        if (!tile)
            return nullptr;

        if (!tile->hasInfantry())
            return nullptr;

        return tile->getInfantry(context.objectManager);
    }

private:
    const int32_t sizeX;                  ///< number of tiles this map is wide (read only)
    const int32_t sizeY;                  ///< number of tiles this map is high (read only)
    std::vector<Tile> tiles;              ///< the 2d-array containing all the tiles of the map
    ObjectBase* lastSinglySelectedObject; ///< The last selected object. If selected again all units of the same type
                                          ///< are selected

    std::queue<uint32_t> removedObjects;

    void init_tile_location();

    [[nodiscard]] int tile_index(int xPos, int yPos) const noexcept { return xPos * sizeY + yPos; }

    AStarSearch pathfinder_;

    Random random_;

    std::unique_ptr<BoxOffsets> offsets_;
    std::unique_ptr<BoxOffsets> offsets_2x2_;
    std::unique_ptr<BoxOffsets> offsets_3x2_;
    std::unique_ptr<BoxOffsets> offsets_2x3_;
    std::unique_ptr<BoxOffsets> offsets_3x3_;

    void init_box_sets();
};

#endif // MAP_H
