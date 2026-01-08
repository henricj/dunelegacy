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

#include <MapEditor/MapEditor.h>

#include <MapEditor/MapMirror.h>

#include <globals.h>

void MapEditor::startOperation() {
    if (undoOperationStack_.empty() || !dynamic_cast<MapEditorStartOperation*>(undoOperationStack_.top().get())) {
        addUndoOperation(std::make_unique<MapEditorStartOperation>());
    }
}

void MapEditor::undoLastOperation() {
    if (!undoOperationStack_.empty()) {
        redoOperationStack_.push(std::make_unique<MapEditorStartOperation>());

        while ((!undoOperationStack_.empty())
               && !dynamic_cast<MapEditorStartOperation*>(undoOperationStack_.top().get())) {
            redoOperationStack_.push(undoOperationStack_.top()->perform(this));
            undoOperationStack_.pop();
        }

        if (!undoOperationStack_.empty()) {
            undoOperationStack_.pop();
        }
    }
}

void MapEditor::redoLastOperation() {
    if (!redoOperationStack_.empty()) {
        undoOperationStack_.push(std::make_unique<MapEditorStartOperation>());

        while ((!redoOperationStack_.empty())
               && !dynamic_cast<MapEditorStartOperation*>(redoOperationStack_.top().get())) {
            undoOperationStack_.push(redoOperationStack_.top()->perform(this));
            redoOperationStack_.pop();
        }

        if (!redoOperationStack_.empty()) {
            redoOperationStack_.pop();
        }
    }
}

void MapEditor::clearRedoOperations() {
    while (!redoOperationStack_.empty()) {
        redoOperationStack_.pop();
    }
}

void MapEditor::performMapEdit(int xpos, int ypos, bool bRepeated) {
    switch (currentEditorMode_.mode_) {
        case EditorMode::EditorMode_Terrain: {
            clearRedoOperations();

            if (!bRepeated) {
                startOperation();
            }

            if (getMapVersion() < 2) {
                // classic map
                if (!bRepeated && map_.isInsideMap(xpos, ypos)) {
                    const auto terrainType = currentEditorMode_.terrainType_;

                    switch (terrainType) {
                        case TERRAINTYPE::Terrain_SpiceBloom: {
                            MapEditorTerrainAddSpiceBloomOperation editOperation(xpos, ypos);
                            addUndoOperation(editOperation.perform(this));
                        } break;

                        case TERRAINTYPE::Terrain_SpecialBloom: {
                            MapEditorTerrainAddSpecialBloomOperation editOperation(xpos, ypos);
                            addUndoOperation(editOperation.perform(this));
                        } break;

                        case TERRAINTYPE::Terrain_Spice: {
                            MapEditorTerrainAddSpiceFieldOperation editOperation(xpos, ypos);
                            addUndoOperation(editOperation.perform(this));
                        } break;

                        default: {
                        } break;
                    }
                }

            } else {
                for (int i = 0; i < mapMirror_->getSize(); i++) {

                    const Coord position = mapMirror_->getCoord(Coord(xpos, ypos), i);

                    const int halfsize = currentEditorMode_.pen_size_ / 2;
                    for (int y = position.y - halfsize; y <= position.y + halfsize; y++) {
                        for (int x = position.x - halfsize; x <= position.x + halfsize; x++) {
                            if (map_.isInsideMap(x, y)) {
                                performTerrainChange(x, y, currentEditorMode_.terrainType_);
                            }
                        }
                    }
                }
            }

        } break;

        case EditorMode::EditorMode_Structure: {
            if (!bRepeated || currentEditorMode_.itemID_ == Structure_Slab1
                || currentEditorMode_.itemID_ == Structure_Wall) {

                const Coord structureSize = getStructureSize(currentEditorMode_.itemID_);

                if (!mapMirror_->mirroringPossible(Coord(xpos, ypos), structureSize)) {
                    return;
                }

                // check if all places are free
                for (int i = 0; i < mapMirror_->getSize(); i++) {
                    const Coord position = mapMirror_->getCoord(Coord(xpos, ypos), i, structureSize);

                    for (int x = position.x; x < position.x + structureSize.x; x++) {
                        for (int y = position.y; y < position.y + structureSize.y; y++) {
                            if (!map_.isInsideMap(x, y)
                                || isTileBlocked(x, y, true, (currentEditorMode_.itemID_ != Structure_Slab1))) {
                                return;
                            }
                        }
                    }
                }

                clearRedoOperations();

                if (!bRepeated) {
                    startOperation();
                }

                auto currentHouse         = currentEditorMode_.house_;
                const bool bHouseIsActive = players_[static_cast<int>(currentHouse)].bActive_;
                for (int i = 0; i < mapMirror_->getSize(); i++) {

                    auto nextHouse = HOUSETYPE::HOUSE_INVALID;
                    for (int k = static_cast<int>(currentHouse); k < static_cast<int>(currentHouse) + NUM_HOUSES; k++) {
                        if (players_[k % NUM_HOUSES].bActive_ == bHouseIsActive) {
                            nextHouse = static_cast<HOUSETYPE>(k % NUM_HOUSES);
                            break;
                        }
                    }

                    if (nextHouse != HOUSETYPE::HOUSE_INVALID) {
                        const Coord position = mapMirror_->getCoord(Coord(xpos, ypos), i, structureSize);

                        MapEditorStructurePlaceOperation placeOperation(
                            position, nextHouse, currentEditorMode_.itemID_, currentEditorMode_.health_);

                        addUndoOperation(placeOperation.perform(this));

                        currentHouse = static_cast<HOUSETYPE>((static_cast<int>(nextHouse) + 1) % NUM_HOUSES);
                    }
                }
            }
        } break;

        case EditorMode::EditorMode_Unit: {
            if (!bRepeated) {

                // first check if all places are free
                for (int i = 0; i < mapMirror_->getSize(); i++) {
                    const Coord position = mapMirror_->getCoord(Coord(xpos, ypos), i);

                    if (!map_.isInsideMap(position.x, position.y)
                        || isTileBlocked(position.x, position.y, false, true)) {
                        return;
                    }
                }

                clearRedoOperations();

                startOperation();

                auto currentHouse         = currentEditorMode_.house_;
                const bool bHouseIsActive = players_[static_cast<int>(currentHouse)].bActive_;
                for (int i = 0; i < mapMirror_->getSize(); i++) {

                    auto nextHouse = HOUSETYPE::HOUSE_INVALID;
                    for (int k = static_cast<int>(currentHouse); k < static_cast<int>(currentHouse) + NUM_HOUSES; k++) {
                        if (players_[k % NUM_HOUSES].bActive_ == bHouseIsActive) {
                            nextHouse = static_cast<HOUSETYPE>(k % NUM_HOUSES);
                            break;
                        }
                    }

                    if (nextHouse != HOUSETYPE::HOUSE_INVALID) {
                        const Coord position = mapMirror_->getCoord(Coord(xpos, ypos), i);

                        const auto angle_ = mapMirror_->getAngle(currentEditorMode_.angle_, i);

                        MapEditorUnitPlaceOperation placeOperation(position,
                                                                   nextHouse,
                                                                   currentEditorMode_.itemID_,
                                                                   currentEditorMode_.health_,
                                                                   angle_,
                                                                   currentEditorMode_.attackmode_);

                        addUndoOperation(placeOperation.perform(this));
                        currentHouse = static_cast<HOUSETYPE>((static_cast<int>(nextHouse) + 1) % NUM_HOUSES);
                    }
                }
            }
        } break;

        case EditorMode::EditorMode_TacticalPos: {
            if (!map_.isInsideMap(xpos, ypos)) {
                return;
            }

            clearRedoOperations();

            startOperation();

            MapEditorSetTacticalPositionOperation setOperation(xpos, ypos);

            addUndoOperation(setOperation.perform(this));

            setEditorMode(EditorMode());
        } break;

        default: {

        } break;
    }
}

void MapEditor::performTerrainChange(int x, int y, TERRAINTYPE terrainType) {

    MapEditorTerrainEditOperation editOperation(x, y, terrainType);
    addUndoOperation(editOperation.perform(this));

    switch (terrainType) {
        case TERRAINTYPE::Terrain_Mountain: {
            if (map_.isInsideMap(x - 1, y) && (map_(x - 1, y) != TERRAINTYPE::Terrain_Mountain)
                && (map_(x - 1, y) != TERRAINTYPE::Terrain_Rock))
                performTerrainChange(x - 1, y, TERRAINTYPE::Terrain_Rock);
            if (map_.isInsideMap(x, y - 1) && (map_(x, y - 1) != TERRAINTYPE::Terrain_Mountain)
                && (map_(x, y - 1) != TERRAINTYPE::Terrain_Rock))
                performTerrainChange(x, y - 1, TERRAINTYPE::Terrain_Rock);
            if (map_.isInsideMap(x + 1, y) && (map_(x + 1, y) != TERRAINTYPE::Terrain_Mountain)
                && (map_(x + 1, y) != TERRAINTYPE::Terrain_Rock))
                performTerrainChange(x + 1, y, TERRAINTYPE::Terrain_Rock);
            if (map_.isInsideMap(x, y + 1) && (map_(x, y + 1) != TERRAINTYPE::Terrain_Mountain)
                && (map_(x, y + 1) != TERRAINTYPE::Terrain_Rock))
                performTerrainChange(x, y + 1, TERRAINTYPE::Terrain_Rock);
        } break;

        case TERRAINTYPE::Terrain_ThickSpice: {
            if (map_.isInsideMap(x - 1, y) && (map_(x - 1, y) != TERRAINTYPE::Terrain_ThickSpice)
                && (map_(x - 1, y) != TERRAINTYPE::Terrain_Spice))
                performTerrainChange(x - 1, y, TERRAINTYPE::Terrain_Spice);
            if (map_.isInsideMap(x, y - 1) && (map_(x, y - 1) != TERRAINTYPE::Terrain_ThickSpice)
                && (map_(x, y - 1) != TERRAINTYPE::Terrain_Spice))
                performTerrainChange(x, y - 1, TERRAINTYPE::Terrain_Spice);
            if (map_.isInsideMap(x + 1, y) && (map_(x + 1, y) != TERRAINTYPE::Terrain_ThickSpice)
                && (map_(x + 1, y) != TERRAINTYPE::Terrain_Spice))
                performTerrainChange(x + 1, y, TERRAINTYPE::Terrain_Spice);
            if (map_.isInsideMap(x, y + 1) && (map_(x, y + 1) != TERRAINTYPE::Terrain_ThickSpice)
                && (map_(x, y + 1) != TERRAINTYPE::Terrain_Spice))
                performTerrainChange(x, y + 1, TERRAINTYPE::Terrain_Spice);
        } break;

        case TERRAINTYPE::Terrain_Rock: {
            if (map_.isInsideMap(x - 1, y) && (map_(x - 1, y) == TERRAINTYPE::Terrain_ThickSpice))
                performTerrainChange(x - 1, y, TERRAINTYPE::Terrain_Spice);
            if (map_.isInsideMap(x, y - 1) && (map_(x, y - 1) == TERRAINTYPE::Terrain_ThickSpice))
                performTerrainChange(x, y - 1, TERRAINTYPE::Terrain_Spice);
            if (map_.isInsideMap(x + 1, y) && (map_(x + 1, y) == TERRAINTYPE::Terrain_ThickSpice))
                performTerrainChange(x + 1, y, TERRAINTYPE::Terrain_Spice);
            if (map_.isInsideMap(x, y + 1) && (map_(x, y + 1) == TERRAINTYPE::Terrain_ThickSpice))
                performTerrainChange(x, y + 1, TERRAINTYPE::Terrain_Spice);
        } break;

        case TERRAINTYPE::Terrain_Spice: {
            if (map_.isInsideMap(x - 1, y) && (map_(x - 1, y) == TERRAINTYPE::Terrain_Mountain))
                performTerrainChange(x - 1, y, TERRAINTYPE::Terrain_Rock);
            if (map_.isInsideMap(x, y - 1) && (map_(x, y - 1) == TERRAINTYPE::Terrain_Mountain))
                performTerrainChange(x, y - 1, TERRAINTYPE::Terrain_Rock);
            if (map_.isInsideMap(x + 1, y) && (map_(x + 1, y) == TERRAINTYPE::Terrain_Mountain))
                performTerrainChange(x + 1, y, TERRAINTYPE::Terrain_Rock);
            if (map_.isInsideMap(x, y + 1) && (map_(x, y + 1) == TERRAINTYPE::Terrain_Mountain))
                performTerrainChange(x, y + 1, TERRAINTYPE::Terrain_Rock);
        } break;

        case TERRAINTYPE::Terrain_Sand:
        case TERRAINTYPE::Terrain_Dunes:
        case TERRAINTYPE::Terrain_SpiceBloom:
        case TERRAINTYPE::Terrain_SpecialBloom: {
            if (map_.isInsideMap(x - 1, y) && (map_(x - 1, y) == TERRAINTYPE::Terrain_Mountain))
                performTerrainChange(x - 1, y, TERRAINTYPE::Terrain_Rock);
            if (map_.isInsideMap(x, y - 1) && (map_(x, y - 1) == TERRAINTYPE::Terrain_Mountain))
                performTerrainChange(x, y - 1, TERRAINTYPE::Terrain_Rock);
            if (map_.isInsideMap(x + 1, y) && (map_(x + 1, y) == TERRAINTYPE::Terrain_Mountain))
                performTerrainChange(x + 1, y, TERRAINTYPE::Terrain_Rock);
            if (map_.isInsideMap(x, y + 1) && (map_(x, y + 1) == TERRAINTYPE::Terrain_Mountain))
                performTerrainChange(x, y + 1, TERRAINTYPE::Terrain_Rock);

            if (map_.isInsideMap(x - 1, y) && (map_(x - 1, y) == TERRAINTYPE::Terrain_ThickSpice))
                performTerrainChange(x - 1, y, TERRAINTYPE::Terrain_Spice);
            if (map_.isInsideMap(x, y - 1) && (map_(x, y - 1) == TERRAINTYPE::Terrain_ThickSpice))
                performTerrainChange(x, y - 1, TERRAINTYPE::Terrain_Spice);
            if (map_.isInsideMap(x + 1, y) && (map_(x + 1, y) == TERRAINTYPE::Terrain_ThickSpice))
                performTerrainChange(x + 1, y, TERRAINTYPE::Terrain_Spice);
            if (map_.isInsideMap(x, y + 1) && (map_(x, y + 1) == TERRAINTYPE::Terrain_ThickSpice))
                performTerrainChange(x, y + 1, TERRAINTYPE::Terrain_Spice);
        } break;

        default: {
        } break;
    }
}
