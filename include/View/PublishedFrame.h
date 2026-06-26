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

#ifndef VIEW_PUBLISHEDFRAME_H
#define VIEW_PUBLISHEDFRAME_H

#include <DataTypes.h>
#include <data.h>

#include <cstdint>
#include <vector>

/// Compact snapshot of a single object visible to a specific player at a tick boundary.
/// Uses stable IDs and value types only — no raw pointers, no UI types.
struct VisibleObjectRecord {
    uint32_t objectID{};
    ItemID_enum itemID{ItemID_Invalid};
    HOUSETYPE ownerHouse{HOUSETYPE::HOUSE_INVALID};
    Coord location{};
    uint8_t healthPercent{};                   ///< Health as 0–100 (clamped from simulation value)
    ANGLETYPE angle{ANGLETYPE::INVALID_ANGLE}; ///< Facing angle; INVALID_ANGLE for structures
    bool isSelected{};                         ///< Selected by the observing player this tick
    bool isAUnit{};                            ///< True when this record describes a unit
    bool isAStructure{};                       ///< True when this record describes a structure
};

/// Per-player snapshot: all objects visible to one player at a tick boundary.
struct PlayerView {
    HOUSETYPE playerHouse{HOUSETYPE::HOUSE_INVALID};
    uint32_t credits{};       ///< Current spice credits for this player
    uint32_t powerProduced{}; ///< Power produced by structures this tick
    uint32_t powerRequired{}; ///< Power required by structures this tick
    uint32_t spiceStored{};   ///< Current spice storage amount
    uint32_t spiceCapacity{}; ///< Maximum spice storage capacity
    std::vector<VisibleObjectRecord> visibleObjects;

    /// Clear visibleObjects while preserving allocated capacity.
    void reset() noexcept { visibleObjects.clear(); }

    /// Pre-allocate capacity for n visible object records.
    void reserveObjects(size_t n) { visibleObjects.reserve(n); }
};

/// Complete per-tick snapshot: frame metadata plus one PlayerView per active player.
///
/// Intended for once-per-tick reuse: call reset() before populating,
/// then set tick and append records via each PlayerView.
struct PublishedFrame {
    uint32_t tick{};
    std::vector<PlayerView> playerViews;

    /// Reset tick to zero and clear each player's visible-object list while
    /// preserving all vector capacities. Cheap enough for once-per-tick use.
    void reset() noexcept;

    /// Pre-allocate capacity for n player views.
    void reservePlayerViews(size_t n) { playerViews.reserve(n); }
};

#endif // VIEW_PUBLISHEDFRAME_H
