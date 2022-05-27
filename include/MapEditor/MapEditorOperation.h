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

#ifndef MAPEDITOROPERATION_H
#define MAPEDITOROPERATION_H

#include <data.h>

#include <AITeamInfo.h>
#include <DataTypes.h>

#include <MapEditor/MapInfo.h>
#include <MapEditor/ReinforcementInfo.h>

#include <memory>
#include <utility>
#include <vector>

class MapEditor;

class MapEditorOperation {
public:
    MapEditorOperation() = default;

    virtual ~MapEditorOperation() = default;

    virtual std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) = 0;
};

class MapEditorNoOperation final : public MapEditorOperation {
public:
    MapEditorNoOperation() = default;

    ~MapEditorNoOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform([[maybe_unused]] MapEditor* pMapEditor) override {
        return std::make_unique<MapEditorNoOperation>();
    }
};

class MapEditorStartOperation final : public MapEditorOperation {
public:
    MapEditorStartOperation() { }

    ~MapEditorStartOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;
};

class MapEditorTerrainEditOperation final : public MapEditorOperation {
public:
    MapEditorTerrainEditOperation(int x, int y, TERRAINTYPE terrainType) : x(x), y(y), terrainType(terrainType) { }

    ~MapEditorTerrainEditOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    int x;
    int y;
    TERRAINTYPE terrainType;
};

class MapEditorTerrainAddSpiceBloomOperation final : public MapEditorOperation {
public:
    MapEditorTerrainAddSpiceBloomOperation(int x, int y) : x(x), y(y) { }

    ~MapEditorTerrainAddSpiceBloomOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    int x;
    int y;
};

class MapEditorTerrainRemoveSpiceBloomOperation final : public MapEditorOperation {
public:
    MapEditorTerrainRemoveSpiceBloomOperation(int x, int y) : x(x), y(y) { }

    ~MapEditorTerrainRemoveSpiceBloomOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    int x;
    int y;
};

class MapEditorTerrainAddSpecialBloomOperation final : public MapEditorOperation {
public:
    MapEditorTerrainAddSpecialBloomOperation(int x, int y) : x(x), y(y) { }

    ~MapEditorTerrainAddSpecialBloomOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    int x;
    int y;
};

class MapEditorTerrainRemoveSpecialBloomOperation final : public MapEditorOperation {
public:
    MapEditorTerrainRemoveSpecialBloomOperation(int x, int y) : x(x), y(y) { }

    ~MapEditorTerrainRemoveSpecialBloomOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    int x;
    int y;
};

class MapEditorTerrainAddSpiceFieldOperation final : public MapEditorOperation {
public:
    MapEditorTerrainAddSpiceFieldOperation(int x, int y) : x(x), y(y) { }

    ~MapEditorTerrainAddSpiceFieldOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    int x;
    int y;
};

class MapEditorTerrainRemoveSpiceFieldOperation final : public MapEditorOperation {
public:
    MapEditorTerrainRemoveSpiceFieldOperation(int x, int y) : x(x), y(y) { }

    ~MapEditorTerrainRemoveSpiceFieldOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    int x;
    int y;
};

class MapEditorSetTacticalPositionOperation final : public MapEditorOperation {
public:
    MapEditorSetTacticalPositionOperation(int x, int y) : x(x), y(y) { }

    ~MapEditorSetTacticalPositionOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    int x;
    int y;
};

class MapEditorStructurePlaceOperation final : public MapEditorOperation {
public:
    MapEditorStructurePlaceOperation(int preferredID, Coord position, HOUSETYPE house, ItemID_enum itemID, int health)
        : preferredID_(preferredID), position_(position), house_(house), itemID_(itemID), health_(health) { }

    MapEditorStructurePlaceOperation(Coord position, HOUSETYPE house, ItemID_enum itemID, int health)
        : preferredID_(INVALID), position_(position), house_(house), itemID_(itemID), health_(health) { }

    ~MapEditorStructurePlaceOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    int preferredID_;
    Coord position_;
    HOUSETYPE house_;
    ItemID_enum itemID_;
    int health_;
};

class MapEditorRemoveStructureOperation final : public MapEditorOperation {
public:
    explicit MapEditorRemoveStructureOperation(int id) : id(id) { }

    ~MapEditorRemoveStructureOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    int id;
};

class MapEditorUnitPlaceOperation final : public MapEditorOperation {
public:
    MapEditorUnitPlaceOperation(int preferredID, Coord position, HOUSETYPE house, ItemID_enum itemID, int health,
                                ANGLETYPE angle, ATTACKMODE attackmode)
        : preferredID_(preferredID), position_(position), house_(house), angle_(angle), itemID_(itemID),
          health_(health), attack_mode_(attackmode) { }

    MapEditorUnitPlaceOperation(Coord position, HOUSETYPE house, ItemID_enum itemID, int health, ANGLETYPE angle,
                                ATTACKMODE attackmode)
        : preferredID_(INVALID), position_(position), house_(house), angle_(angle), itemID_(itemID), health_(health),
          attack_mode_(attackmode) { }

    ~MapEditorUnitPlaceOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    int preferredID_;
    Coord position_;
    HOUSETYPE house_;
    ANGLETYPE angle_;
    ItemID_enum itemID_;
    int health_;
    ATTACKMODE attack_mode_;
};

class MapEditorRemoveUnitOperation final : public MapEditorOperation {
public:
    explicit MapEditorRemoveUnitOperation(int id) : id_(id) { }

    ~MapEditorRemoveUnitOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    int id_;
};

class MapEditorEditStructureOperation final : public MapEditorOperation {
public:
    MapEditorEditStructureOperation(int id, int health) : id_(id), health_(health) { }

    ~MapEditorEditStructureOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    int id_;
    int health_;
};

class MapEditorEditUnitOperation final : public MapEditorOperation {
public:
    MapEditorEditUnitOperation(int id, int health, ANGLETYPE angle, ATTACKMODE attackmode)
        : id_(id), health_(health), angle_(angle), attack_mode_(attackmode) { }

    ~MapEditorEditUnitOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    int id_;
    int health_;
    ANGLETYPE angle_;
    ATTACKMODE attack_mode_;
};

class MapEditorChangePlayer final : public MapEditorOperation {
public:
    MapEditorChangePlayer(int playerNum, bool bActive, bool bAnyHouse, int credits, std::string brain, int quota = 0,
                          int maxunit = 0)
        : playerNum_(playerNum), bActive_(bActive), bAnyHouse_(bAnyHouse), credits_(credits), brain_(std::move(brain)),
          quota_(quota), max_unit_(maxunit) { }

    ~MapEditorChangePlayer() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    int playerNum_;
    bool bActive_;
    bool bAnyHouse_;
    int credits_;
    std::string brain_;
    int quota_;
    int max_unit_;
};

class MapEditorChangeChoam final : public MapEditorOperation {
public:
    MapEditorChangeChoam(ItemID_enum itemID, int amount) : itemID_(itemID), amount_(amount) { }

    ~MapEditorChangeChoam() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    ItemID_enum itemID_;
    int amount_;
};

class MapEditorChangeReinforcements final : public MapEditorOperation {
public:
    explicit MapEditorChangeReinforcements(std::vector<ReinforcementInfo>& reinforcements)
        : reinforcements_(reinforcements) { }

    ~MapEditorChangeReinforcements() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    std::vector<ReinforcementInfo> reinforcements_;
};

class MapEditorChangeTeams final : public MapEditorOperation {
public:
    explicit MapEditorChangeTeams(std::vector<AITeamInfo>& aiteams) : ai_teams_(aiteams) { }

    ~MapEditorChangeTeams() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    std::vector<AITeamInfo> ai_teams_;
};

class MapEditorChangeMapInfo final : public MapEditorOperation {
public:
    explicit MapEditorChangeMapInfo(MapInfo& mapInfo) : mapInfo_(mapInfo) { }

    ~MapEditorChangeMapInfo() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor* pMapEditor) override;

    MapInfo mapInfo_;
};

#endif // MAPEDITOROPERATION_H
