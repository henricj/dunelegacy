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

#include <DataTypes.h>
#include <AITeamInfo.h>

#include <MapEditor/ReinforcementInfo.h>
#include <MapEditor/MapInfo.h>

#include <utility>
#include <vector>
#include <memory>

class MapEditor;

class MapEditorOperation {
public:

    MapEditorOperation() = default;

    virtual ~MapEditorOperation() = default;

    virtual std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) = 0;
};

class MapEditorNoOperation : public MapEditorOperation {
public:

    MapEditorNoOperation() = default;

    ~MapEditorNoOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override
    {
        return std::make_unique<MapEditorNoOperation>();
    };
};

class MapEditorStartOperation : public MapEditorOperation {
public:

    MapEditorStartOperation()  {
    }

    ~MapEditorStartOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;
};

class MapEditorTerrainEditOperation : public MapEditorOperation {
public:

    MapEditorTerrainEditOperation(int x, int y, TERRAINTYPE terrainType)
     :  x(x), y(y), terrainType(terrainType) {
    }

    ~MapEditorTerrainEditOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    int x;
    int y;
    TERRAINTYPE terrainType;
};

class MapEditorTerrainAddSpiceBloomOperation : public MapEditorOperation {
public:

    MapEditorTerrainAddSpiceBloomOperation(int x, int y)
     :  x(x), y(y) {
    }

    ~MapEditorTerrainAddSpiceBloomOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    int x;
    int y;
};

class MapEditorTerrainRemoveSpiceBloomOperation : public MapEditorOperation {
public:

    MapEditorTerrainRemoveSpiceBloomOperation(int x, int y)
     :  x(x), y(y) {
    }

    ~MapEditorTerrainRemoveSpiceBloomOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    int x;
    int y;
};

class MapEditorTerrainAddSpecialBloomOperation : public MapEditorOperation {
public:

    MapEditorTerrainAddSpecialBloomOperation(int x, int y)
     :  x(x), y(y) {
    }

    ~MapEditorTerrainAddSpecialBloomOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    int x;
    int y;
};

class MapEditorTerrainRemoveSpecialBloomOperation : public MapEditorOperation {
public:

    MapEditorTerrainRemoveSpecialBloomOperation(int x, int y)
     :  x(x), y(y) {
    }

    ~MapEditorTerrainRemoveSpecialBloomOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    int x;
    int y;
};

class MapEditorTerrainAddSpiceFieldOperation : public MapEditorOperation {
public:

    MapEditorTerrainAddSpiceFieldOperation(int x, int y)
     :  x(x), y(y) {
    }

    ~MapEditorTerrainAddSpiceFieldOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    int x;
    int y;
};

class MapEditorTerrainRemoveSpiceFieldOperation : public MapEditorOperation {
public:

    MapEditorTerrainRemoveSpiceFieldOperation(int x, int y)
     :  x(x), y(y) {
    }

    ~MapEditorTerrainRemoveSpiceFieldOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    int x;
    int y;
};



class MapEditorSetTacticalPositionOperation : public MapEditorOperation {
public:

    MapEditorSetTacticalPositionOperation(int x, int y)
     :  x(x), y(y) {
    }

    ~MapEditorSetTacticalPositionOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    int x;
    int y;
};



class MapEditorStructurePlaceOperation : public MapEditorOperation {
public:

    MapEditorStructurePlaceOperation(int preferredID, Coord position, HOUSETYPE house, ItemID_enum itemID, int health)
     :  preferredID(preferredID), position(position), house(house), itemID(itemID), health(health) {
    }

    MapEditorStructurePlaceOperation(Coord position, HOUSETYPE house, ItemID_enum itemID, int health)
     :  preferredID(INVALID), position(position), house(house), itemID(itemID), health(health) {
    }

    ~MapEditorStructurePlaceOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    int             preferredID;
    Coord           position;
    HOUSETYPE       house;
    ItemID_enum itemID;
    int             health;
};

class MapEditorRemoveStructureOperation : public MapEditorOperation {
public:

    explicit MapEditorRemoveStructureOperation(int id)
     :  id(id) {
    }

    ~MapEditorRemoveStructureOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    int             id;
};

class MapEditorUnitPlaceOperation : public MapEditorOperation {
public:

    MapEditorUnitPlaceOperation(int preferredID, Coord position, HOUSETYPE house, ItemID_enum itemID, int health, ANGLETYPE angle, ATTACKMODE attackmode)
     :  preferredID(preferredID), position(position), house(house), itemID(itemID), health(health), angle(angle), attackmode(attackmode) {
    }

    MapEditorUnitPlaceOperation(Coord position, HOUSETYPE house, ItemID_enum itemID, int health, ANGLETYPE angle, ATTACKMODE attackmode)
     :  preferredID(INVALID), position(position), house(house), itemID(itemID), health(health), angle(angle), attackmode(attackmode) {
    }

    ~MapEditorUnitPlaceOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    int             preferredID;
    Coord           position;
    HOUSETYPE       house;
    ANGLETYPE       angle;
    ItemID_enum itemID;
    int             health;
    ATTACKMODE      attackmode;
};

class MapEditorRemoveUnitOperation : public MapEditorOperation {
public:

    explicit MapEditorRemoveUnitOperation(int id)
     :  id(id) {
    }

    ~MapEditorRemoveUnitOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    int             id;
};


class MapEditorEditStructureOperation : public MapEditorOperation {
public:

    MapEditorEditStructureOperation(int id, int health)
     :  id(id), health(health) {
    }

    ~MapEditorEditStructureOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    int             id;
    int             health;
};


class MapEditorEditUnitOperation : public MapEditorOperation {
public:

    MapEditorEditUnitOperation(int id, int health, ANGLETYPE angle, ATTACKMODE attackmode)
     :  id(id), health(health), angle(angle), attackmode(attackmode) {
    }

    ~MapEditorEditUnitOperation() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    int             id;
    int             health;
    ANGLETYPE       angle;
    ATTACKMODE      attackmode;
};

class MapEditorChangePlayer : public MapEditorOperation {
public:

    MapEditorChangePlayer(int playerNum, bool bActive, bool bAnyHouse, int credits, std::string  brain, int quota = 0, int maxunit = 0)
     : playerNum(playerNum), bActive(bActive), bAnyHouse(bAnyHouse), credits(credits), brain(std::move(brain)), quota(quota), maxunit(maxunit) {
    }

    ~MapEditorChangePlayer() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    int playerNum;
    bool bActive;
    bool bAnyHouse;
    int credits;
    std::string brain;
    int quota;
    int maxunit;
};


class MapEditorChangeChoam : public MapEditorOperation {
public:

    MapEditorChangeChoam(ItemID_enum itemID, int amount)
     : itemID(itemID), amount(amount) {
    }

    ~MapEditorChangeChoam() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    ItemID_enum itemID;
    int amount;
};


class MapEditorChangeReinforcements : public MapEditorOperation {
public:

    explicit MapEditorChangeReinforcements(std::vector<ReinforcementInfo>& reinforcements)
     : reinforcements(reinforcements) {
    }

    ~MapEditorChangeReinforcements() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    std::vector<ReinforcementInfo>  reinforcements;
};

class MapEditorChangeTeams : public MapEditorOperation {
public:

    explicit MapEditorChangeTeams(std::vector<AITeamInfo>& aiteams)
     : aiteams(aiteams) {
    }

    ~MapEditorChangeTeams() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    std::vector<AITeamInfo>  aiteams;
};

class MapEditorChangeMapInfo : public MapEditorOperation {
public:

    explicit MapEditorChangeMapInfo(MapInfo& mapInfo)
     : mapInfo(mapInfo) {
    }

    ~MapEditorChangeMapInfo() override = default;

    std::unique_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) override;

    MapInfo  mapInfo;
};



#endif // MAPEDITOROPERATION_H
