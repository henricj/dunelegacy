
#include <MapEditor/MapEditorOperation.h>

#include <MapEditor/MapEditor.h>

#include <algorithm>


std::shared_ptr<MapEditorOperation> MapEditorStartOperation::perform(MapEditor *pMapEditor) {
    return std::shared_ptr<MapEditorOperation>(new MapEditorStartOperation());
}



std::shared_ptr<MapEditorOperation> MapEditorTerrainEditOperation::perform(MapEditor *pMapEditor) {

    MapData& map = pMapEditor->getMap();

    TERRAINTYPE oldTerrainType = map(x,y);

    map(x,y) = terrainType;

    return std::shared_ptr<MapEditorOperation>(new MapEditorTerrainEditOperation(x, y, oldTerrainType));
}




std::shared_ptr<MapEditorOperation> MapEditorTerrainAddSpiceBloomOperation::perform(MapEditor *pMapEditor) {

    std::vector<Coord>& spiceBlooms = pMapEditor->getSpiceBlooms();

    if(std::find(spiceBlooms.begin(), spiceBlooms.end(), Coord(x,y)) != spiceBlooms.end()) {
        return std::shared_ptr<MapEditorOperation>(new MapEditorNoOperation());
    } else {
        spiceBlooms.push_back(Coord(x,y));
        return std::shared_ptr<MapEditorOperation>(new MapEditorTerrainRemoveSpiceBloomOperation(x, y));
    }
}


std::shared_ptr<MapEditorOperation> MapEditorTerrainRemoveSpiceBloomOperation::perform(MapEditor *pMapEditor) {

    std::vector<Coord>& spiceBlooms = pMapEditor->getSpiceBlooms();

    std::vector<Coord>::iterator iter = std::find(spiceBlooms.begin(), spiceBlooms.end(), Coord(x,y));

    if(iter != spiceBlooms.end()) {
        spiceBlooms.erase(iter);
        return std::shared_ptr<MapEditorOperation>(new MapEditorTerrainAddSpiceBloomOperation(x, y));
    } else {
        return std::shared_ptr<MapEditorOperation>(new MapEditorNoOperation());
    }
}



std::shared_ptr<MapEditorOperation> MapEditorTerrainAddSpecialBloomOperation::perform(MapEditor *pMapEditor) {

    std::vector<Coord>& specialBlooms = pMapEditor->getSpecialBlooms();

    if(std::find(specialBlooms.begin(), specialBlooms.end(), Coord(x,y)) != specialBlooms.end()) {
        return std::shared_ptr<MapEditorOperation>(new MapEditorNoOperation());
    } else {
        specialBlooms.push_back(Coord(x,y));
        return std::shared_ptr<MapEditorOperation>(new MapEditorTerrainRemoveSpecialBloomOperation(x, y));
    }
}


std::shared_ptr<MapEditorOperation> MapEditorTerrainRemoveSpecialBloomOperation::perform(MapEditor *pMapEditor) {

    std::vector<Coord>& specialBlooms = pMapEditor->getSpecialBlooms();

    std::vector<Coord>::iterator iter = std::find(specialBlooms.begin(), specialBlooms.end(), Coord(x,y));

    if(iter != specialBlooms.end()) {
        specialBlooms.erase(iter);
        return std::shared_ptr<MapEditorOperation>(new MapEditorTerrainAddSpecialBloomOperation(x, y));
    } else {
        return std::shared_ptr<MapEditorOperation>(new MapEditorNoOperation());
    }
}



std::shared_ptr<MapEditorOperation> MapEditorTerrainAddSpiceFieldOperation::perform(MapEditor *pMapEditor) {

    std::vector<Coord>& spiceFields = pMapEditor->getSpiceFields();

    if(std::find(spiceFields.begin(), spiceFields.end(), Coord(x,y)) != spiceFields.end()) {
        return std::shared_ptr<MapEditorOperation>(new MapEditorNoOperation());
    } else {
        spiceFields.push_back(Coord(x,y));
        return std::shared_ptr<MapEditorOperation>(new MapEditorTerrainRemoveSpiceFieldOperation(x, y));
    }
}


std::shared_ptr<MapEditorOperation> MapEditorTerrainRemoveSpiceFieldOperation::perform(MapEditor *pMapEditor) {

    std::vector<Coord>& spiceFields = pMapEditor->getSpiceFields();

    std::vector<Coord>::iterator iter = std::find(spiceFields.begin(), spiceFields.end(), Coord(x,y));

    if(iter != spiceFields.end()) {
        spiceFields.erase(iter);
        return std::shared_ptr<MapEditorOperation>(new MapEditorTerrainAddSpiceFieldOperation(x, y));
    } else {
        return std::shared_ptr<MapEditorOperation>(new MapEditorNoOperation());
    }
}



std::shared_ptr<MapEditorOperation> MapEditorSetTacticalPositionOperation::perform(MapEditor *pMapEditor) {
    Coord currentTacticalPos = pMapEditor->getMapInfo().tacticalPos;

    pMapEditor->getMapInfo().tacticalPos = Coord(x,y);

    return std::shared_ptr<MapEditorOperation>(new MapEditorSetTacticalPositionOperation(currentTacticalPos.x, currentTacticalPos.y));
}



std::shared_ptr<MapEditorOperation> MapEditorStructurePlaceOperation::perform(MapEditor *pMapEditor) {

    std::vector<MapEditor::Structure>& structures = pMapEditor->getStructureList();

    int maxID = 0;
    int minID = -1;
    std::vector<MapEditor::Structure>::const_iterator iter;
    for(iter = structures.begin(); iter != structures.end(); ++iter) {
        maxID = std::max(maxID, iter->id );
        minID = std::min(minID, iter->id );
    }

    int newID = (preferredID != INVALID) ? preferredID : (maxID + 1);

    if((itemID == Structure_Slab1) || (itemID == Structure_Slab4) || (itemID == Structure_Wall)) {
        newID = minID-1;
    }

    structures.push_back(MapEditor::Structure(newID, house, itemID, health, position));

    return std::shared_ptr<MapEditorOperation>(new MapEditorRemoveStructureOperation(newID));
}



std::shared_ptr<MapEditorOperation> MapEditorRemoveStructureOperation::perform(MapEditor *pMapEditor) {

    std::vector<MapEditor::Structure>& structures = pMapEditor->getStructureList();

    std::vector<MapEditor::Structure>::iterator iter;
    for(iter = structures.begin(); iter != structures.end(); ++iter) {

        if(iter->id == id) {
            std::shared_ptr<MapEditorOperation> redoOperation(new MapEditorStructurePlaceOperation(iter->id, iter->position, iter->house, iter->itemID, iter->health));

            structures.erase(iter);

            return redoOperation;
        }
    }

    return std::shared_ptr<MapEditorOperation>(new MapEditorNoOperation());
}



std::shared_ptr<MapEditorOperation> MapEditorUnitPlaceOperation::perform(MapEditor *pMapEditor) {

    std::vector<MapEditor::Unit>& units = pMapEditor->getUnitList();

    int maxID = 0;
    std::vector<MapEditor::Unit>::const_iterator iter;
    for(iter = units.begin(); iter != units.end(); ++iter) {
        maxID = std::max(maxID, iter->id );
    }

    int newID = (preferredID != INVALID) ? preferredID : (maxID + 1);

    units.push_back(MapEditor::Unit(newID, house, itemID, health, position, angle, attackmode));

    return std::shared_ptr<MapEditorOperation>(new MapEditorRemoveUnitOperation(newID));
}



std::shared_ptr<MapEditorOperation> MapEditorRemoveUnitOperation::perform(MapEditor *pMapEditor) {

    std::vector<MapEditor::Unit>& units = pMapEditor->getUnitList();

    std::vector<MapEditor::Unit>::iterator iter;
    for(iter = units.begin(); iter != units.end(); ++iter) {

        if(iter->id == id) {
            std::shared_ptr<MapEditorOperation> redoOperation(new MapEditorUnitPlaceOperation(iter->id, iter->position, iter->house, iter->itemID, iter->health, iter->angle, iter->attackmode));

            units.erase(iter);

            return redoOperation;
        }
    }

    return std::shared_ptr<MapEditorOperation>(new MapEditorNoOperation());
}



std::shared_ptr<MapEditorOperation> MapEditorEditStructureOperation::perform(MapEditor *pMapEditor) {

    MapEditor::Structure* pStructure = pMapEditor->getStructure(id);

    int oldHealth = pStructure->health;

    pStructure->health = health;

    return std::shared_ptr<MapEditorOperation>(new MapEditorEditStructureOperation(id, oldHealth));
}



std::shared_ptr<MapEditorOperation> MapEditorEditUnitOperation::perform(MapEditor *pMapEditor) {

    MapEditor::Unit* pUnit = pMapEditor->getUnit(id);

    int oldHealth = pUnit->health;
    unsigned char oldAngle = pUnit->angle;
    ATTACKMODE oldAttackmode = pUnit->attackmode;

    pUnit->health = health;
    pUnit->angle = angle;
    pUnit->attackmode = attackmode;

    return std::shared_ptr<MapEditorOperation>(new MapEditorEditUnitOperation(id, oldHealth, oldAngle, oldAttackmode));
}



std::shared_ptr<MapEditorOperation> MapEditorChangePlayer::perform(MapEditor *pMapEditor) {

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

    return std::shared_ptr<MapEditorOperation>(new MapEditorChangePlayer(playerNum, bOldActive, bOldAnyHouse, oldCredits, oldBrain, oldQuota, oldMaxunit));
}



std::shared_ptr<MapEditorOperation> MapEditorChangeChoam::perform(MapEditor *pMapEditor) {

    std::map<int,int>& choam = pMapEditor->getChoam();

    bool oldAmount = choam.count(itemID) > 0 ? choam[itemID] : -1;

    if(amount >= 0) {
        choam[itemID] = amount;
    } else {
        // for -1 or other negative values we remove this item from choam
        // (THIS IS DIFFERENT TO -1 IN THE INI FILE WHERE -1 MEANS AN AMOUNT OF 0)
        choam.erase(itemID);
    }

    return std::shared_ptr<MapEditorOperation>(new MapEditorChangeChoam(itemID, oldAmount));
}



std::shared_ptr<MapEditorOperation> MapEditorChangeReinforcements::perform(MapEditor *pMapEditor) {

    std::vector<ReinforcementInfo>  oldReinforcements = pMapEditor->getReinforcements();

    pMapEditor->setReinforcements(reinforcements);

    return std::shared_ptr<MapEditorOperation>(new MapEditorChangeReinforcements(oldReinforcements));
}



std::shared_ptr<MapEditorOperation> MapEditorChangeTeams::perform(MapEditor *pMapEditor) {

    std::vector<TeamInfo>  oldTeams = pMapEditor->getTeams();

    pMapEditor->setTeams(teams);

    return std::shared_ptr<MapEditorOperation>(new MapEditorChangeTeams(oldTeams));
}



std::shared_ptr<MapEditorOperation> MapEditorChangeMapInfo::perform(MapEditor *pMapEditor) {

    MapInfo  oldMapInfo = pMapEditor->getMapInfo();

    pMapEditor->setMapInfo(mapInfo);

    return std::shared_ptr<MapEditorOperation>(new MapEditorChangeMapInfo(oldMapInfo));
}
