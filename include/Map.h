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

#include <Tile.h>
#include <AStarSearch.h>
#include <misc/InputStream.h>
#include <misc/OutputStream.h>
#include <misc/exceptions.h>
#include "misc/Random.h"

class Map final
{
public:
    /**
        Creates a map of size xSize x ySize. The map is initialized with all tiles of type Terrain_Sand.
    */
    Map(int xSize, int ySize);
    Map(const Map& o) = delete;
    Map(Map&& o) = delete;
    ~Map();

    Map& operator=(const Map &) = delete;
    Map& operator=(Map &&) = delete;

    void load(InputStream& stream);
    void save(OutputStream& stream) const;

    void createSandRegions();
    void damage(Uint32 damagerID, House* damagerOwner, const Coord& realPos, Uint32 bulletID, FixPoint damage, int damageRadius, bool air);
    static Coord getMapPos(ANGLETYPE angle, const Coord& source);
    void removeObjectFromMap(Uint32 objectID);
    void spiceRemoved(const Coord& coord);
    void selectObjects(const House* pHouse, int x1, int y1, int x2, int y2, int realX, int realY, bool objectARGMode);

    void viewMap(HOUSETYPE houseID, const Coord& location, const int maxViewRange);
    void viewMap(HOUSETYPE houseID, int x, int y, const int maxViewRange) {
        viewMap(houseID, Coord(x, y), maxViewRange);
    }

    bool findSpice(Coord& destination, const Coord& origin) const;
    bool okayToPlaceStructure(int x, int y, int buildingSizeX, int buildingSizeY, bool tilesRequired, const House* pHouse, bool bIgnoreUnits = false) const;
    bool isAStructureGap(int x, int y, int buildingSizeX, int buildingSizeY) const; // Allows AI to check to see if a gap exists between the current structure
    bool isWithinBuildRange(int x, int y, const House* pHouse) const;
    static ANGLETYPE getPosAngle(const Coord& source, const Coord& pos);
    Coord findClosestEdgePoint(const Coord& origin, const Coord& buildingSize) const;
    Coord findDeploySpot(UnitBase* pUnit, const Coord& origin, Random& randomGen, const Coord& gatherPoint = Coord::Invalid(), const Coord& buildingSize = Coord(0, 0)) const; //building size is num squares

    void createSpiceField(Coord location, int radius, bool centerIsThickSpice = false);

    Sint32 getSizeX() const noexcept {
        return sizeX;
    }

    Sint32 getSizeY() const noexcept {
        return sizeY;
    }

    int getKey(const Tile& tile) const noexcept { return tile_index(tile.getLocation().x, tile.getLocation().y); }

    int getKey(int xPos, int yPos) const {
        if (!tileExists(xPos, yPos))
            THROW(std::out_of_range, "Tile (%d, %d) does not exist!", xPos, yPos);

        return tile_index(xPos, yPos);
    }

    bool tileExists(int xPos, int yPos) const noexcept {
        return ((xPos >= 0) && (xPos < sizeX) && (yPos >= 0) && (yPos < sizeY));
    }

    bool tileExists(const Coord& pos) const noexcept {
        return tileExists(pos.x, pos.y);
    }

    Tile* tryGetTile(int xPos, int yPos) noexcept {
        if (!tileExists(xPos, yPos))
            return nullptr;

        return &tiles[tile_index(xPos, yPos)];
    }

    const Tile* tryGetTile(int xPos, int yPos) const noexcept {
        if (!tileExists(xPos, yPos))
            return nullptr;

        return &tiles[tile_index(xPos, yPos)];
    }

    const Tile* getTile(int xPos, int yPos) const {
        const auto tile = tryGetTile(xPos, yPos);

        if (!tile)
            THROW(std::out_of_range, "Tile (%d, %d) does not exist!", xPos, yPos);

        return tile;
    }

    Tile* getTile(int xPos, int yPos) {
        const auto tile = tryGetTile(xPos, yPos);

        if (!tile)
            THROW(std::out_of_range, "Tile (%d, %d) does not exist!", xPos, yPos);

        return tile;
    }

    const Tile* getTile(const Coord& location) const {
        return getTile(location.x, location.y);
    }

    Tile* getTile(const Coord& location) {
        return getTile(location.x, location.y);
    }

    template<class...Args>
    void add_bullet(Args&&...args)
    {
        bulletList.push_back(std::make_unique<Bullet>(std::forward<Args>(args)...));
    }

    template<typename F>
    void for_all(F&& f)
    {
        std::for_each(std::begin(tiles), std::end(tiles), f);
    }

    template<typename F>
    void for_all(F&& f) const
    {
        std::for_each(std::begin(tiles), std::end(tiles), f);
    }
protected:
    template<typename F>
    void location_for_each(int x1, int y1, int x2, int y2, F&& f) const {
        if (x1 < 0)
            x1 = 0;
        if (x2 < x1)
            x2 = x1;
        else if (x2 >= sizeX)
            x2 = sizeX;
        if (x1 > x2)
            x1 = x2;

        if (y1 < 0)
            y1 = 0;
        if (y2 < y1)
            y2 = y1;
        else if (y2 >= sizeY)
            y2 = sizeY;
        if (y1 > y2)
            y1 = y2;

        for (auto x = x1; x < x2; ++x) {
            for (auto y = y1; y < y2; ++y) {
                f(x, y);
            }
        }
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
        location_for_each(x1, y1, x2, y2, [&](int x, int y) {f(tile_index(x, y)); });
    }
    template<typename Filter, typename F>
    void index_for_each_filter(int x1, int y1, int x2, int y2, Filter&& filter, F&& f) const {
        location_for_each(x1, y1, x2, y2, [&](int x, int y) { if (filter(x, y)) f(tile_index(x, y)); });
    }
    class BoxOffsets
    {
        std::vector<std::vector<std::pair<int, int>>> box_sets_;
    public:
        BoxOffsets(int size, Coord box = Coord(1, 1));
        std::vector<std::pair<int, int>>& search_set(int depth) {
            return box_sets_[depth - 1];
        }

        size_t max_depth() const noexcept { return box_sets_.size(); }
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
    template<typename Offsets, typename Generator, typename Predicate>
    SearchResult search_random_offsets(int x, int y, Offsets&& offsets, Generator&& generator, Predicate&& predicate) const {

        const auto size = offsets.size();
        auto found = false;

        // We do an incremental Fisher-Yates shuffle.  This should be as
        // random as the generator, and guarantees that each tile will
        // be visited exactly once.
        for (auto i = 0u; i < size; ++i) {
            std::swap(offsets[i], offsets[generator.rand(i, size - 1)]);

            const auto ranX = x + offsets[i].first;
            const auto ranY = y + offsets[i].second;

            const auto tile = tryGetTile(ranX, ranY);

            if (!tile)
                continue;;

            const auto result = predicate(*tile);

            if (SearchResult::Done == result)
                return SearchResult::Done;

            if (SearchResult::DoneAtDepth == result)
                found = true;
        }

        return found ? SearchResult::DoneAtDepth : SearchResult::NotDone;
    }

    template<typename Generator, typename Predicate>
    SearchResult search_box_edge(int x, int y, int depth, BoxOffsets* offsets, Generator&& generator, Predicate&& predicate) const {

        if (0 == depth) {
            const auto tile = tryGetTile(x, y);
            if (!tile)
                return SearchResult::NotDone;

            return predicate(*tile);
        }

        return search_random_offsets(x, y, offsets->search_set(depth), generator, predicate);
    }

    template<typename Generator, typename Predicate>
    bool search_all_by_box_edge(int x, int y, BoxOffsets* offsets, Generator generator, Predicate&& predicate) const {

        for (auto depth = 0u; depth <= offsets->max_depth(); ++depth) {
            const auto ret = search_box_edge(x, y, depth, offsets, generator, predicate);

            if (SearchResult::NotDone != ret)
                return true;
        }

        return false;
    }

    template<typename Generator, typename Predicate>
    bool search_all_by_box_edge(int x, int y, Generator&& generator, Predicate&& predicate) const {
        return search_all_by_box_edge(x, y, offsets_.get(), generator, predicate);
    }

    template<typename Generator, typename Predicate>
    bool search_all_by_box_edge_2x2(int x, int y, Generator&& generator, Predicate&& predicate) const {
        return search_all_by_box_edge(x, y, offsets_2x2_.get(), generator, predicate);
    }

    template<typename Generator, typename Predicate>
    bool search_all_by_box_edge_2x3(int x, int y, Generator&& generator, Predicate&& predicate) const {
        return search_all_by_box_edge(x, y, offsets_2x3_.get(), generator, predicate);
    }

    template<typename Generator, typename Predicate>
    bool search_all_by_box_edge_3x2(int x, int y, Generator&& generator, Predicate&& predicate) const {
        return search_all_by_box_edge(x, y, offsets_3x2_.get(), generator, predicate);
    }

    template<typename Generator, typename Predicate>
    bool search_all_by_box_edge_3x3(int x, int y, Generator&& generator, Predicate&& predicate) const {
        return search_all_by_box_edge(x, y, offsets_3x3_.get(), generator, predicate);
    }

public:
    template<typename Generator, typename Predicate>
    bool search_all_by_box_edge(int x, int y, const Coord& buildingSize, Generator&& generator, Predicate&& predicate) const {

        if (buildingSize == Coord{ 2, 2 })
            return search_all_by_box_edge_2x2(x, y, generator, predicate);
        if (buildingSize == Coord{ 2, 3 })
            return search_all_by_box_edge_2x3(x, y, generator, predicate);
        if (buildingSize == Coord{ 3, 2 })
            return search_all_by_box_edge_3x2(x, y, generator, predicate);
        if (buildingSize == Coord{ 3, 3 })
            return search_all_by_box_edge_3x3(x, y, generator, predicate);

        return search_all_by_box_edge(x, y, generator, predicate);
    }

protected:
    template<typename F>
    void index_for_each_angle(int x, int y, F&& f)
    {
        if (x >= 1) {
            if (tileExists(x - 1, y - 1)) f(ANGLETYPE::LEFTUP, tile_index(x - 1, y - 1));
            if (tileExists(x - 1, y)) f(ANGLETYPE::LEFT, tile_index(x - 1, y));
            if (tileExists(x - 1, y + 1)) f(ANGLETYPE::LEFTDOWN, tile_index(x - 1, y + 1));
        }
        if (tileExists(x    , y - 1)) f(ANGLETYPE::UP       , tile_index(x    , y - 1));
        if (tileExists(x    , y + 1)) f(ANGLETYPE::DOWN     , tile_index(x    , y + 1));
        if (x + 1 < sizeX) {
            if (tileExists(x + 1, y - 1)) f(ANGLETYPE::RIGHTUP, tile_index(x + 1, y - 1));
            if (tileExists(x + 1, y)) f(ANGLETYPE::RIGHT, tile_index(x + 1, y));
            if (tileExists(x + 1, y + 1)) f(ANGLETYPE::RIGHTDOWN, tile_index(x + 1, y + 1));
        }
    }
public:
    template<typename F>
    void for_each_angle(int x, int y, F&& f)
    {
        if (tileExists(x, y))
            index_for_each_angle(x, y, [&](ANGLETYPE angle, int index) { f(angle, tiles[index]); });
    }

    template<typename F>
    void for_each_neighbor(int x, int y, F&& f)
    {
        if (tileExists(x - 1, y)) f(tiles[tile_index(x - 1, y)]);
        if (tileExists(x, y - 1)) f(tiles[tile_index(x, y - 1)]);
        if (tileExists(x, y + 1)) f(tiles[tile_index(x, y + 1)]);
        if (tileExists(x + 1, y)) f(tiles[tile_index(x + 1, y)]);
    }

    template<bool EmptyTile = true, bool PredicateValue = true, int AllEmptyMask = EmptyTile ? 0x0f : 0, int AllTrueMask = 0x0f, typename F>
    int get_neighbor_mask(int x, int y, F&& predicate)
    {
        const auto e_up = tileExists(x, y - 1);
        const auto e_right = tileExists(x + 1, y);
        const auto e_down = tileExists(x, y + 1);
        const auto e_left = tileExists(x - 1, y);

        if (!e_up && !e_right && !e_down && !e_left)
            return AllEmptyMask;

        auto mask = 0;

        if (EmptyTile)
        {
            if (!e_up || PredicateValue == predicate(tiles[tile_index(x, y - 1)]))
                mask |= 1 << 0;
            if (!e_right || PredicateValue == predicate(tiles[tile_index(x + 1, y)]))
                mask |= 1 << 1;
            if (!e_down || PredicateValue == predicate(tiles[tile_index(x, y + 1)]))
                mask |= 1 << 2;
            if (!e_left || PredicateValue == predicate(tiles[tile_index(x - 1, y)]))
                mask |= 1 << 3;
        }
        else
        {
            if (e_up && PredicateValue == predicate(tiles[tile_index(x, y - 1)]))
                mask |= 1 << 0;
            if (e_right && PredicateValue == predicate(tiles[tile_index(x + 1, y)]))
                mask |= 1 << 1;
            if (e_down && PredicateValue == predicate(tiles[tile_index(x, y + 1)]))
                mask |= 1 << 2;
            if (e_left && PredicateValue == predicate(tiles[tile_index(x - 1, y)]))
                mask |= 1 << 3;
        }

        if (AllTrueMask != 0x0f && 0x0f == mask)
            return AllTrueMask;

        return mask;
    }

    bool find_path(UnitBase* pUnit, Coord start, Coord destination, std::vector<Coord>& path)
    {
        pathfinder_.Search(this, pUnit, start, destination);

        return pathfinder_.getFoundPath(this, path);
    }
private:
    const Sint32  sizeX;                    ///< number of tiles this map is wide (read only)
    const Sint32  sizeY;                    ///< number of tiles this map is high (read only)
    std::vector<Tile> tiles;                ///< the 2d-array containing all the tiles of the map
    ObjectBase* lastSinglySelectedObject;   ///< The last selected object. If selected again all units of the same type are selected

    void init_tile_location();

    int tile_index(int xPos, int yPos) const noexcept
    {
        return xPos * sizeY + yPos;
    }

    AStarSearch pathfinder_;

    std::unique_ptr<BoxOffsets> offsets_;
    std::unique_ptr<BoxOffsets> offsets_2x2_;
    std::unique_ptr<BoxOffsets> offsets_3x2_;
    std::unique_ptr<BoxOffsets> offsets_2x3_;
    std::unique_ptr<BoxOffsets> offsets_3x3_;

    void init_box_sets();
};


#endif // MAP_H
