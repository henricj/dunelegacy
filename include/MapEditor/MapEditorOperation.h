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

#include <MapEditor/ReinforcementInfo.h>
#include <MapEditor/TeamInfo.h>
#include <MapEditor/MapInfo.h>

#include <vector>
#include <memory>

class MapEditor;

class MapEditorOperation {
public:

    MapEditorOperation() = default;

    virtual ~MapEditorOperation() = default;

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) = 0;
};

class MapEditorNoOperation : public MapEditorOperation {
public:

    MapEditorNoOperation() = default;

    virtual ~MapEditorNoOperation() = default;

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor) {
        return std::shared_ptr<MapEditorOperation>(new MapEditorNoOperation());
    };
};

class MapEditorStartOperation : public MapEditorOperation {
public:

    MapEditorStartOperation() : MapEditorOperation() {
    }

    virtual ~MapEditorStartOperation() = default;

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);
};

class MapEditorTerrainEditOperation : public MapEditorOperation {
public:

    MapEditorTerrainEditOperation(int x, int y, TERRAINTYPE terrainType)
     : MapEditorOperation(), x(x), y(y), terrainType(terrainType) {
    }

    virtual ~MapEditorTerrainEditOperation() {
    }

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    int x;
    int y;
    TERRAINTYPE terrainType;
};

class MapEditorTerrainAddSpiceBloomOperation : public MapEditorOperation {
public:

    MapEditorTerrainAddSpiceBloomOperation(int x, int y)
     : MapEditorOperation(), x(x), y(y) {
    }

    virtual ~MapEditorTerrainAddSpiceBloomOperation() = default;

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    int x;
    int y;
};

class MapEditorTerrainRemoveSpiceBloomOperation : public MapEditorOperation {
public:

    MapEditorTerrainRemoveSpiceBloomOperation(int x, int y)
     : MapEditorOperation(), x(x), y(y) {
    }

    virtual ~MapEditorTerrainRemoveSpiceBloomOperation() = default;

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    int x;
    int y;
};

class MapEditorTerrainAddSpecialBloomOperation : public MapEditorOperation {
public:

    MapEditorTerrainAddSpecialBloomOperation(int x, int y)
     : MapEditorOperation(), x(x), y(y) {
    }

    virtual ~MapEditorTerrainAddSpecialBloomOperation() = default;

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    int x;
    int y;
};

class MapEditorTerrainRemoveSpecialBloomOperation : public MapEditorOperation {
public:

    MapEditorTerrainRemoveSpecialBloomOperation(int x, int y)
     : MapEditorOperation(), x(x), y(y) {
    }

    virtual ~MapEditorTerrainRemoveSpecialBloomOperation() = default;

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    int x;
    int y;
};

class MapEditorTerrainAddSpiceFieldOperation : public MapEditorOperation {
public:

    MapEditorTerrainAddSpiceFieldOperation(int x, int y)
     : MapEditorOperation(), x(x), y(y) {
    }

    virtual ~MapEditorTerrainAddSpiceFieldOperation() {
    }

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    int x;
    int y;
};

class MapEditorTerrainRemoveSpiceFieldOperation : public MapEditorOperation {
public:

    MapEditorTerrainRemoveSpiceFieldOperation(int x, int y)
     : MapEditorOperation(), x(x), y(y) {
    }

    virtual ~MapEditorTerrainRemoveSpiceFieldOperation() {
    }

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    int x;
    int y;
};



class MapEditorSetTacticalPositionOperation : public MapEditorOperation {
public:

    MapEditorSetTacticalPositionOperation(int x, int y)
     : MapEditorOperation(), x(x), y(y) {
    }

    virtual ~MapEditorSetTacticalPositionOperation() = default;

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    int x;
    int y;
};



class MapEditorStructurePlaceOperation : public MapEditorOperation {
public:

    MapEditorStructurePlaceOperation(int preferredID, Coord position, HOUSETYPE house, int itemID, int health)
     : MapEditorOperation(), preferredID(preferredID), position(position), house(house), itemID(itemID), health(health) {
    }

    MapEditorStructurePlaceOperation(Coord position, HOUSETYPE house, int itemID, int health)
     : MapEditorOperation(), preferredID(INVALID), position(position), house(house), itemID(itemID), health(health) {
    }

    virtual ~MapEditorStructurePlaceOperation() = default;

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    int             preferredID;
    Coord           position;
    HOUSETYPE       house;
    int             itemID;
    int             health;
};

class MapEditorRemoveStructureOperation : public MapEditorOperation {
public:

    explicit MapEditorRemoveStructureOperation(int id)
     : MapEditorOperation(), id(id) {
    }

    virtual ~MapEditorRemoveStructureOperation() = default;

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    int             id;
};

class MapEditorUnitPlaceOperation : public MapEditorOperation {
public:

    MapEditorUnitPlaceOperation(int preferredID, Coord position, HOUSETYPE house, int itemID, int health, unsigned char angle, ATTACKMODE attackmode)
     : MapEditorOperation(), preferredID(preferredID), position(position), house(house), itemID(itemID), health(health), angle(angle), attackmode(attackmode) {
    }

    MapEditorUnitPlaceOperation(Coord position, HOUSETYPE house, int itemID, int health, unsigned char angle, ATTACKMODE attackmode)
     : MapEditorOperation(), preferredID(INVALID), position(position), house(house), itemID(itemID), health(health), angle(angle), attackmode(attackmode) {
    }

    virtual ~MapEditorUnitPlaceOperation() = default;

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    int             preferredID;
    Coord           position;
    HOUSETYPE       house;
    int             itemID;
    int             health;
    unsigned char   angle;
    ATTACKMODE      attackmode;
};

class MapEditorRemoveUnitOperation : public MapEditorOperation {
public:

    explicit MapEditorRemoveUnitOperation(int id)
     : MapEditorOperation(), id(id) {
    }

    virtual ~MapEditorRemoveUnitOperation() = default;

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    int             id;
};


class MapEditorEditStructureOperation : public MapEditorOperation {
public:

    MapEditorEditStructureOperation(int id, int health)
     : MapEditorOperation(), id(id), health(health) {
    }

    virtual ~MapEditorEditStructureOperation() = default;

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    int             id;
    int             health;
};


class MapEditorEditUnitOperation : public MapEditorOperation {
public:

    MapEditorEditUnitOperation(int id, int health, unsigned char angle, ATTACKMODE attackmode)
     : MapEditorOperation(), id(id), health(health), angle(angle), attackmode(attackmode) {
    }

    virtual ~MapEditorEditUnitOperation() {
    }

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    int             id;
    int             health;
    unsigned char  angle;
    ATTACKMODE      attackmode;
};

class MapEditorChangePlayer : public MapEditorOperation {
public:

    MapEditorChangePlayer(int playerNum, bool bActive, bool bAnyHouse, int credits, const std::string& brain, int quota = 0, int maxunit = 0)
     : playerNum(playerNum), bActive(bActive), bAnyHouse(bAnyHouse), credits(credits), brain(brain), quota(quota), maxunit(maxunit) {
    }

    virtual ~MapEditorChangePlayer() {
    }

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

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

    MapEditorChangeChoam(int itemID, int amount)
     : itemID(itemID), amount(amount) {
    }

    virtual ~MapEditorChangeChoam() = default;

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    int itemID;
    int amount;
};


class MapEditorChangeReinforcements : public MapEditorOperation {
public:

    explicit MapEditorChangeReinforcements(std::vector<ReinforcementInfo>& reinforcements)
     : reinforcements(reinforcements) {
    }

    virtual ~MapEditorChangeReinforcements() {
    }

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    std::vector<ReinforcementInfo>  reinforcements;
};

class MapEditorChangeTeams : public MapEditorOperation {
public:

    explicit MapEditorChangeTeams(std::vector<TeamInfo>& teams)
     : teams(teams) {
    }

    virtual ~MapEditorChangeTeams() = default;

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    std::vector<TeamInfo>  teams;
};

class MapEditorChangeMapInfo : public MapEditorOperation {
public:

    explicit MapEditorChangeMapInfo(MapInfo& mapInfo)
     : mapInfo(mapInfo) {
    }

    virtual ~MapEditorChangeMapInfo() = default;

    virtual std::shared_ptr<MapEditorOperation> perform(MapEditor *pMapEditor);

    MapInfo  mapInfo;
};



#endif // MAPEDITOROPERATION_H
