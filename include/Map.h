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
#include <misc/InputStream.h>
#include <misc/OutputStream.h>
#include <misc/exceptions.h>
#include <misc/Random.h>

#include <cstdio>

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
    static Coord getMapPos(int angle, const Coord& source);
    void removeObjectFromMap(Uint32 objectID);
    void spiceRemoved(const Coord& coord);
    void selectObjects(const House* pHouse, int x1, int y1, int x2, int y2, int realX, int realY, bool objectARGMode);

    void viewMap(int houseID, const Coord& location, const int maxViewRange);
    void viewMap(int houseID, int x, int y, const int maxViewRange) {
        viewMap(houseID, Coord(x, y), maxViewRange);
    }

    bool findSpice(Coord& destination, const Coord& origin) const;
    bool okayToPlaceStructure(int x, int y, int buildingSizeX, int buildingSizeY, bool tilesRequired, const House* pHouse, bool bIgnoreUnits = false) const;
    bool isAStructureGap(int x, int y, int buildingSizeX, int buildingSizeY) const; // Allows AI to check to see if a gap exists between the current structure
    bool isWithinBuildRange(int x, int y, const House* pHouse) const;
    static int getPosAngle(const Coord& source, const Coord& pos);
    Coord findClosestEdgePoint(const Coord& origin, const Coord& buildingSize) const;
    Coord findDeploySpot(UnitBase* pUnit, const Coord& origin, Random& randomGen, const Coord& gatherPoint = Coord::Invalid(), const Coord& buildingSize = Coord(0, 0)) const; //building size is num squares

    void createSpiceField(Coord location, int radius, bool centerIsThickSpice = false);

    Sint32 getSizeX() const noexcept {
        return sizeX;
    }

    Sint32 getSizeY() const noexcept {
        return sizeY;
    }

    bool tileExists(int xPos, int yPos) const noexcept {
        return ((xPos >= 0) && (xPos < sizeX) && (yPos >= 0) && (yPos < sizeY));
    }

    bool tileExists(const Coord& pos) const noexcept {
        return tileExists(pos.x, pos.y);
    }

    const Tile* getTile(int xPos, int yPos) const {
        const auto tile = getTile_internal(xPos, yPos);

        if (!tile)
            THROW(std::out_of_range, "Tile (%d, %d) does not exist!", xPos, yPos);

        return tile;
    }

    Tile* getTile(int xPos, int yPos) {
        const auto tile = getTile_internal(xPos, yPos);

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

        auto max_depth() const noexcept { return box_sets_.size(); }
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

    template<typename F, typename Predicate>
    bool search(int x1, int y1, int x2, int y2, Predicate&& predicate) const {
        index_for_each(x1, y1, x2, y2, [&](int index) { predicate(tiles[index]); });
    }

    template<typename Filter, typename Predicate>
    bool search_filter(int x1, int y1, int x2, int y2, Filter&& filter, Predicate&& predicate) const {
        index_for_each_filter(x1, y1, x2, y2, filter, [&](int index) { predicate(tiles[index]); });
    }

    template<typename Offsets, typename Generator, typename Predicate>
    bool search_random_offsets(int x, int y, Offsets&& offsets, Generator&& generator, Predicate&& predicate) const {

        const auto size = offsets.size();

        // We do an incremental Fisher-Yates shuffle.  This should be as
        // random as the generator, and guarantees that each tile will
        // be visited exactly once.
        for (auto i = 0u; i < size; ++i) {
            std::swap(offsets[i], offsets[generator.rand(i, size - 1)]);

            const auto ranX = x + offsets[i].first;
            const auto ranY = y + offsets[i].second;

            const auto tile = getTile_internal(ranX, ranY);

            if (tile && predicate(*tile))
                return true;
        }

        return false;
    }

protected:
    template<typename Generator, typename Predicate>
    bool search_box_edge(int x, int y, int depth, BoxOffsets* offsets, Generator&& generator, Predicate&& predicate) const {

        if (0 == depth) {
            const auto tile = getTile_internal(x, y);
            return tile && predicate(*tile);
        }

        return search_random_offsets(x, y, offsets->search_set(depth), generator, predicate);
    }

    template<typename Generator, typename Predicate>
    bool search_all_by_box_edge(int x, int y, BoxOffsets* offsets, Generator generator, Predicate&& predicate) const {
        for (auto depth = 0u; depth <= offsets->max_depth(); ++depth) {
            if (search_box_edge(x, y, depth, offsets, generator, predicate))
                return true;
        }

        return false;
    }

public:
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

    template<typename Generator, typename Predicate>
    bool search_all_by_box_edge(int x, int y, Coord buildingSize, Generator&& generator, Predicate&& predicate) const {

        if (buildingSize == Coord{ 2, 2 })
            return search_all_by_box_edge_2x2(x, y, currentGame->randomGen, predicate);
        if (buildingSize == Coord{ 2, 3 })
            return search_all_by_box_edge_2x3(x, y, currentGame->randomGen, predicate);
        if (buildingSize == Coord{ 3, 2 })
            return search_all_by_box_edge_3x2(x, y, currentGame->randomGen, predicate);
        if (buildingSize == Coord{ 3, 3 })
            return search_all_by_box_edge_3x3(x, y, currentGame->randomGen, predicate);

        return search_all_by_box_edge(x, y, currentGame->randomGen, predicate);
    }

protected:
    template<typename F>
    void index_for_each_angle(int x, int y, F&& f)
    {
        if (tileExists(x - 1, y - 1)) f(LEFTUP   , tile_index(x - 1, y - 1));
        if (tileExists(x - 1, y    )) f(LEFT     , tile_index(x - 1, y    ));
        if (tileExists(x - 1, y + 1)) f(LEFTDOWN , tile_index(x - 1, y + 1));
        if (tileExists(x    , y - 1)) f(UP       , tile_index(x    , y - 1));
        if (tileExists(x    , y + 1)) f(DOWN     , tile_index(x    , y + 1));
        if (tileExists(x + 1, y - 1)) f(RIGHTUP  , tile_index(x + 1, y - 1));
        if (tileExists(x + 1, y    )) f(RIGHT    , tile_index(x + 1, y    ));
        if (tileExists(x + 1, y + 1)) f(RIGHTDOWN, tile_index(x + 1, y + 1));
    }
public:
    template<typename F>
    void for_each_angle(int x, int y, F&& f)
    {
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

    template<typename F>
    int get_neighbor_mask(int x, int y, F&& predicate)
    {
        const auto e_up = tileExists(x, y - 1);
        const auto e_right = tileExists(x + 1, y);
        const auto e_down = tileExists(x, y + 1);
        const auto e_left = tileExists(x - 1, y);

        if (!e_up && !e_right && !e_down && !e_left)
            return 0;

        auto mask = 0;

        if (!e_up || predicate(tiles[tile_index(x, y - 1)]))
            mask |= 1 << 0;
        if (!e_right || predicate(tiles[tile_index(x + 1, y)]))
            mask |= 1 << 1;
        if (!e_down || predicate(tiles[tile_index(x, y + 1)]))
            mask |= 1 << 2;
        if (!e_left || predicate(tiles[tile_index(x - 1, y)]))
            mask |= 1 << 3;

        if (0x0f == mask)
            return 0;

        const auto e_mask = static_cast<int>(e_up) | (static_cast<int>(e_right) << 1) | (static_cast<int>(e_down) << 2) | (static_cast<int>(e_left) << 3);

        // up = !e_up || predicate(up)
        // Substitute, and simplify...
        //   !e_up || !up
        // = !e_up || !(!e_up || predicate(up))
        // = !e_up || (e_up && !predicate(up))
        // = (!e_up || e_up) && (!e_up || !predicate(up))
        // = true && (!e_up || !predicate(up))
        // = !e_up || !predicate(up)

        //if (!e_up || !up)
        //    mask |= 1 << 0;
        //if (!e_right && !right)
        //    mask |= 1 << 1;
        //if (!e_down || !down)
        //    mask |= 1 << 2;
        //if (!e_left || !left)
        //    mask |= 1 << 3;

        return 0x0f & (~e_mask | ~mask);
    }

    template<typename F>
    int get_neighbor_mask_expensive_predicate(int x, int y, F&& predicate)
    {
        const auto e_up    = tileExists(x    , y - 1);
        const auto e_right = tileExists(x + 1, y    );
        const auto e_down  = tileExists(x    , y + 1);
        const auto e_left  = tileExists(x - 1, y    );

        if (!e_up && !e_right && !e_down && !e_left)
            return 0;

        const auto up    = !e_up    || predicate(tiles[tile_index(x    , y - 1)]);
        if (up && !e_right && !e_down && !e_left)
            return 0;

        const auto right = !e_right || predicate(tiles[tile_index(x + 1, y    )]);
        if (up && right && !e_down && !e_left)
            return 0;

        const auto down  = !e_down  || predicate(tiles[tile_index(x    , y + 1)]);
        if (up && right && down && !e_left)
            return 0;

        const auto left  = !e_left  || predicate(tiles[tile_index(x - 1, y    )]);
        if (up && right && down && left)
            return 0;

        auto mask = 0;

        // up = !e_up || predicate(up)
        // Substitute, and simplify...
        //   !e_up || !up
        // = !e_up || !(!e_up || predicate(up))
        // = !e_up || (e_up && !predicate(up))
        // = (!e_up || e_up) && (!e_up || !predicate(up))
        // = true && (!e_up || !predicate(up))
        // = !e_up || !predicate(up)
        if (!e_up || !up)
            mask |= 1 << 0;
        if (!e_right && !right)
            mask |= 1 << 1;
        if (!e_down || !down)
            mask |= 1 << 2;
        if (!e_left || !left)
            mask |= 1 << 3;

        return mask;
    }
private:
    Sint32  sizeX;                          ///< number of tiles this map is wide (read only)
    Sint32  sizeY;                          ///< number of tiles this map is high (read only)
    std::vector<Tile> tiles;                ///< the 2d-array containing all the tiles of the map
    ObjectBase* lastSinglySelectedObject;   ///< The last selected object. If selected again all units of the same type are selected

    void init_tile_location();

    int tile_index(int xPos, int yPos) const noexcept
    {
        return xPos * sizeY + yPos;
    }

    Tile* getTile_internal(int xPos, int yPos) noexcept {
        if (!tileExists(xPos, yPos))
            return nullptr;

        return &tiles[tile_index(xPos, yPos)];
    }

    const Tile* getTile_internal(int xPos, int yPos) const noexcept {
        if (!tileExists(xPos, yPos))
            return nullptr;

        return &tiles[tile_index(xPos, yPos)];
    }

    std::unique_ptr<BoxOffsets> offsets_;
    std::unique_ptr<BoxOffsets> offsets_2x2_;
    std::unique_ptr<BoxOffsets> offsets_3x2_;
    std::unique_ptr<BoxOffsets> offsets_2x3_;
    std::unique_ptr<BoxOffsets> offsets_3x3_;

    void init_box_sets();
};


#endif // MAP_H
