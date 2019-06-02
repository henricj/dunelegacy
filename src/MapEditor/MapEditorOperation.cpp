
#include <MapEditor/MapEditorOperation.h>

#include <MapEditor/MapEditor.h>

#include <algorithm>


std::unique_ptr<MapEditorOperation> MapEditorStartOperation::perform(MapEditor *pMapEditor) {
    return std::make_unique<MapEditorStartOperation>();
}



std::unique_ptr<MapEditorOperation> MapEditorTerrainEditOperation::perform(MapEditor *pMapEditor) {

    MapData& map = pMapEditor->getMap();

    TERRAINTYPE oldTerrainType = map(x,y);

    map(x,y) = terrainType;

    return std::make_unique<MapEditorTerrainEditOperation>(x, y, oldTerrainType);
}




std::unique_ptr<MapEditorOperation> MapEditorTerrainAddSpiceBloomOperation::perform(MapEditor *pMapEditor) {

    std::vector<Coord>& spiceBlooms = pMapEditor->getSpiceBlooms();

    if(std::find(spiceBlooms.begin(), spiceBlooms.end(), Coord(x,y)) != spiceBlooms.end()) {
        return std::make_unique<MapEditorNoOperation>();
    } else {
        spiceBlooms.emplace_back(x,y);
        return std::make_unique<MapEditorTerrainRemoveSpiceBloomOperation>(x, y);
    }
}


std::unique_ptr<MapEditorOperation> MapEditorTerrainRemoveSpiceBloomOperation::perform(MapEditor *pMapEditor) {

    std::vector<Coord>& spiceBlooms = pMapEditor->getSpiceBlooms();

    std::vector<Coord>::iterator iter = std::find(spiceBlooms.begin(), spiceBlooms.end(), Coord(x,y));

    if(iter != spiceBlooms.end()) {
        spiceBlooms.erase(iter);
        return std::make_unique<MapEditorTerrainAddSpiceBloomOperation>(x, y);
    } else {
        return std::make_unique<MapEditorNoOperation>();
    }
}



std::unique_ptr<MapEditorOperation> MapEditorTerrainAddSpecialBloomOperation::perform(MapEditor *pMapEditor) {

    std::vector<Coord>& specialBlooms = pMapEditor->getSpecialBlooms();

    if(std::find(specialBlooms.begin(), specialBlooms.end(), Coord(x,y)) != specialBlooms.end()) {
        return std::make_unique<MapEditorNoOperation>();
    } else {
        specialBlooms.emplace_back(x,y);
        return std::make_unique<MapEditorTerrainRemoveSpecialBloomOperation>(x, y);
    }
}


std::unique_ptr<MapEditorOperation> MapEditorTerrainRemoveSpecialBloomOperation::perform(MapEditor *pMapEditor) {

    std::vector<Coord>& specialBlooms = pMapEditor->getSpecialBlooms();

    std::vector<Coord>::iterator iter = std::find(specialBlooms.begin(), specialBlooms.end(), Coord(x,y));

    if(iter != specialBlooms.end()) {
        specialBlooms.erase(iter);
        return std::make_unique<MapEditorTerrainAddSpecialBloomOperation>(x, y);
    } else {
        return std::make_unique<MapEditorNoOperation>();
    }
}



std::unique_ptr<MapEditorOperation> MapEditorTerrainAddSpiceFieldOperation::perform(MapEditor *pMapEditor) {

    std::vector<Coord>& spiceFields = pMapEditor->getSpiceFields();

    if(std::find(spiceFields.begin(), spiceFields.end(), Coord(x,y)) != spiceFields.end()) {
        return std::make_unique<MapEditorNoOperation>();
    } else {
        spiceFields.emplace_back(x,y);
        return std::make_unique<MapEditorTerrainRemoveSpiceFieldOperation>(x, y);
    }
}


std::unique_ptr<MapEditorOperation> MapEditorTerrainRemoveSpiceFieldOperation::perform(MapEditor *pMapEditor) {

    std::vector<Coord>& spiceFields = pMapEditor->getSpiceFields();

    std::vector<Coord>::iterator iter = std::find(spiceFields.begin(), spiceFields.end(), Coord(x,y));

    if(iter != spiceFields.end()) {
        spiceFields.erase(iter);
        return std::make_unique<MapEditorTerrainAddSpiceFieldOperation>(x, y);
    } else {
        return std::make_unique<MapEditorNoOperation>();
    }
}



std::unique_ptr<MapEditorOperation> MapEditorSetTacticalPositionOperation::perform(MapEditor *pMapEditor) {
    Coord currentTacticalPos = pMapEditor->getMapInfo().tacticalPos;

    pMapEditor->getMapInfo().tacticalPos = Coord(x,y);

    return std::make_unique<MapEditorSetTacticalPositionOperation>(currentTacticalPos.x, currentTacticalPos.y);
}



std::unique_ptr<MapEditorOperation> MapEditorStructurePlaceOperation::perform(MapEditor *pMapEditor) {

    std::vector<MapEditor::Structure>& structures = pMapEditor->getStructureList();

    int maxID = 0;
    int minID = -1;
    for(const MapEditor::Structure& structure : structures) {
        maxID = std::max(maxID, structure.id);
        minID = std::min(minID, structure.id);
    }

    int newID = (preferredID != INVALID) ? preferredID : (maxID + 1);

    if((itemID == Structure_Slab1) || (itemID == Structure_Slab4) || (itemID == Structure_Wall)) {
        newID = minID-1;
    }

    structures.emplace_back(newID, house, itemID, health, position);

    return std::make_unique<MapEditorRemoveStructureOperation>(newID);
}



std::unique_ptr<MapEditorOperation> MapEditorRemoveStructureOperation::perform(MapEditor *pMapEditor) {

    std::vector<MapEditor::Structure>& structures = pMapEditor->getStructureList();

    for(auto iter = structures.begin(); iter != structures.end(); ++iter) {
        if(iter->id == id) {
            auto redoOperation = std::make_unique<MapEditorStructurePlaceOperation>(iter->id, iter->position, iter->house, iter->itemID, iter->health);

            structures.erase(iter);

            return std::move(redoOperation);
        }
    }

    return std::make_unique<MapEditorNoOperation>();
}



std::unique_ptr<MapEditorOperation> MapEditorUnitPlaceOperation::perform(MapEditor *pMapEditor) {

    std::vector<MapEditor::Unit>& units = pMapEditor->getUnitList();

    int maxID = 0;
    for(const MapEditor::Unit& unit : units) {
        maxID = std::max(maxID, unit.id );
    }

    int newID = (preferredID != INVALID) ? preferredID : (maxID + 1);

    units.emplace_back(newID, house, itemID, health, position, angle, attackmode);

    return std::make_unique<MapEditorRemoveUnitOperation>(newID);
}



std::unique_ptr<MapEditorOperation> MapEditorRemoveUnitOperation::perform(MapEditor *pMapEditor) {

    std::vector<MapEditor::Unit>& units = pMapEditor->getUnitList();

    for(auto iter = units.begin(); iter != units.end(); ++iter) {
        if(iter->id == id) {
            auto redoOperation = std::make_unique<MapEditorUnitPlaceOperation>(iter->id, iter->position, iter->house, iter->itemID, iter->health, iter->angle, iter->attackmode);

            units.erase(iter);

            return redoOperation;
        }
    }

    return std::make_unique<MapEditorNoOperation>();
}



std::unique_ptr<MapEditorOperation> MapEditorEditStructureOperation::perform(MapEditor *pMapEditor) {

    MapEditor::Structure* pStructure = pMapEditor->getStructure(id);

    int oldHealth = pStructure->health;

    pStructure->health = health;

    return std::make_unique<MapEditorEditStructureOperation>(id, oldHealth);
}



std::unique_ptr<MapEditorOperation> MapEditorEditUnitOperation::perform(MapEditor *pMapEditor) {

    MapEditor::Unit* pUnit = pMapEditor->getUnit(id);

    int oldHealth = pUnit->health;
    unsigned char oldAngle = pUnit->angle;
    ATTACKMODE oldAttackmode = pUnit->attackmode;

    pUnit->health = health;
    pUnit->angle = angle;
    pUnit->attackmode = attackmode;

    return std::make_unique<MapEditorEditUnitOperation>(id, oldHealth, oldAngle, oldAttackmode);
}



std::unique_ptr<MapEditorOperation> MapEditorChangePlayer::perform(MapEditor *pMapEditor) {

    MapEditor::Player& player = pMapEditor->getPlayers()[playerNum];

    bool bOldActive = player.bActive;
    bool bOldAnyHouse = player.bAnyHouse;
    int oldCredits = player.credits;
    std::string oldBrain = player.brain;
    int oldQuota = player.quota;
    int oldMaxunit = player.maxunit;

    player.bActive = bActive;
    player.bAnyHouse = bAnyHouse;
    player.credits = credits;
    player.brain = brain;
    player.quota = quota;
    player.maxunit = maxunit;

    pMapEditor->informPlayersChanged();

    return std::make_unique<MapEditorChangePlayer>(playerNum, bOldActive, bOldAnyHouse, oldCredits, oldBrain, oldQuota, oldMaxunit);
}



std::unique_ptr<MapEditorOperation> MapEditorChangeChoam::perform(MapEditor *pMapEditor) {

    std::map<int,int>& choam = pMapEditor->getChoam();

    int oldAmount = choam.count(itemID) > 0 ? choam[itemID] : -1;

    if(amount >= 0) {
        choam[itemID] = amount;
    } else {
        // for -1 or other negative values we remove this item from choam
        // (THIS IS DIFFERENT TO -1 IN THE INI FILE WHERE -1 MEANS AN AMOUNT OF 0)
        choam.erase(itemID);
    }

    return std::make_unique<MapEditorChangeChoam>(itemID, oldAmount);
}



std::unique_ptr<MapEditorOperation> MapEditorChangeReinforcements::perform(MapEditor *pMapEditor) {

    std::vector<ReinforcementInfo>  oldReinforcements = pMapEditor->getReinforcements();

    pMapEditor->setReinforcements(reinforcements);

    return std::make_unique<MapEditorChangeReinforcements>(oldReinforcements);
}



std::unique_ptr<MapEditorOperation> MapEditorChangeTeams::perform(MapEditor *pMapEditor) {

    std::vector<AITeamInfo>  oldAITeams = pMapEditor->getAITeams();

    pMapEditor->setAITeams(aiteams);

    return std::make_unique<MapEditorChangeTeams>(oldAITeams);
}



std::unique_ptr<MapEditorOperation> MapEditorChangeMapInfo::perform(MapEditor *pMapEditor) {

    MapInfo  oldMapInfo = pMapEditor->getMapInfo();

    pMapEditor->setMapInfo(mapInfo);

    return std::make_unique<MapEditorChangeMapInfo>(oldMapInfo);
}
