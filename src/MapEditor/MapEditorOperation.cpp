
#include <MapEditor/MapEditorOperation.h>

#include <MapEditor/MapEditor.h>

#include <algorithm>

std::unique_ptr<MapEditorOperation> MapEditorStartOperation::perform([[maybe_unused]] MapEditor* pMapEditor) {
    return std::make_unique<MapEditorStartOperation>();
}

std::unique_ptr<MapEditorOperation> MapEditorTerrainEditOperation::perform(MapEditor* pMapEditor) {

    MapData& map = pMapEditor->getMap();

    auto& tile = map(x, y);

    TERRAINTYPE oldTerrainType = tile;

    tile = terrainType;

    return std::make_unique<MapEditorTerrainEditOperation>(x, y, oldTerrainType);
}

std::unique_ptr<MapEditorOperation> MapEditorTerrainAddSpiceBloomOperation::perform(MapEditor* pMapEditor) {

    auto& spiceBlooms = pMapEditor->getSpiceBlooms();

    if (std::ranges::find(spiceBlooms, Coord(x, y)) != spiceBlooms.end()) {
        return std::make_unique<MapEditorNoOperation>();
    }
    spiceBlooms.emplace_back(x, y);
    return std::make_unique<MapEditorTerrainRemoveSpiceBloomOperation>(x, y);
}

std::unique_ptr<MapEditorOperation> MapEditorTerrainRemoveSpiceBloomOperation::perform(MapEditor* pMapEditor) {

    std::vector<Coord>& spiceBlooms = pMapEditor->getSpiceBlooms();

    const auto iter = std::ranges::find(spiceBlooms, Coord(x, y));

    if (iter != spiceBlooms.end()) {
        spiceBlooms.erase(iter);
        return std::make_unique<MapEditorTerrainAddSpiceBloomOperation>(x, y);
    }
    return std::make_unique<MapEditorNoOperation>();
}

std::unique_ptr<MapEditorOperation> MapEditorTerrainAddSpecialBloomOperation::perform(MapEditor* pMapEditor) {

    std::vector<Coord>& specialBlooms = pMapEditor->getSpecialBlooms();

    if (std::ranges::find(specialBlooms, Coord(x, y)) != specialBlooms.end()) {
        return std::make_unique<MapEditorNoOperation>();
    }
    specialBlooms.emplace_back(x, y);
    return std::make_unique<MapEditorTerrainRemoveSpecialBloomOperation>(x, y);
}

std::unique_ptr<MapEditorOperation> MapEditorTerrainRemoveSpecialBloomOperation::perform(MapEditor* pMapEditor) {

    std::vector<Coord>& specialBlooms = pMapEditor->getSpecialBlooms();

    const auto iter = std::ranges::find(specialBlooms, Coord(x, y));

    if (iter != specialBlooms.end()) {
        specialBlooms.erase(iter);
        return std::make_unique<MapEditorTerrainAddSpecialBloomOperation>(x, y);
    }
    return std::make_unique<MapEditorNoOperation>();
}

std::unique_ptr<MapEditorOperation> MapEditorTerrainAddSpiceFieldOperation::perform(MapEditor* pMapEditor) {

    std::vector<Coord>& spiceFields = pMapEditor->getSpiceFields();

    if (std::ranges::find(spiceFields, Coord(x, y)) != spiceFields.end()) {
        return std::make_unique<MapEditorNoOperation>();
    }
    spiceFields.emplace_back(x, y);
    return std::make_unique<MapEditorTerrainRemoveSpiceFieldOperation>(x, y);
}

std::unique_ptr<MapEditorOperation> MapEditorTerrainRemoveSpiceFieldOperation::perform(MapEditor* pMapEditor) {

    std::vector<Coord>& spiceFields = pMapEditor->getSpiceFields();

    const auto iter = std::ranges::find(spiceFields, Coord(x, y));

    if (iter != spiceFields.end()) {
        spiceFields.erase(iter);
        return std::make_unique<MapEditorTerrainAddSpiceFieldOperation>(x, y);
    }
    return std::make_unique<MapEditorNoOperation>();
}

std::unique_ptr<MapEditorOperation> MapEditorSetTacticalPositionOperation::perform(MapEditor* pMapEditor) {
    Coord currentTacticalPos = pMapEditor->getMapInfo().tacticalPos;

    pMapEditor->getMapInfo().tacticalPos = Coord(x, y);

    return std::make_unique<MapEditorSetTacticalPositionOperation>(currentTacticalPos.x, currentTacticalPos.y);
}

std::unique_ptr<MapEditorOperation> MapEditorStructurePlaceOperation::perform(MapEditor* pMapEditor) {

    std::vector<MapEditor::Structure>& structures = pMapEditor->getStructureList();

    int maxID = 0;
    int minID = -1;
    for (const MapEditor::Structure& structure : structures) {
        maxID = std::max(maxID, structure.id_);
        minID = std::min(minID, structure.id_);
    }

    int newID = (preferredID_ != INVALID) ? preferredID_ : (maxID + 1);

    if ((itemID_ == Structure_Slab1) || (itemID_ == Structure_Slab4) || (itemID_ == Structure_Wall)) {
        newID = minID - 1;
    }

    structures.emplace_back(newID, house_, itemID_, health_, position_);

    return std::make_unique<MapEditorRemoveStructureOperation>(newID);
}

std::unique_ptr<MapEditorOperation> MapEditorRemoveStructureOperation::perform(MapEditor* pMapEditor) {

    std::vector<MapEditor::Structure>& structures = pMapEditor->getStructureList();

    for (auto iter = structures.begin(); iter != structures.end(); ++iter) {
        if (iter->id_ == id) {
            auto redoOperation = std::make_unique<MapEditorStructurePlaceOperation>(
                iter->id_, iter->position_, iter->house_, iter->itemID_, iter->health_);

            structures.erase(iter);

            return std::move(redoOperation);
        }
    }

    return std::make_unique<MapEditorNoOperation>();
}

std::unique_ptr<MapEditorOperation> MapEditorUnitPlaceOperation::perform(MapEditor* pMapEditor) {

    std::vector<MapEditor::Unit>& units = pMapEditor->getUnitList();

    int maxID = 0;
    for (const MapEditor::Unit& unit : units) {
        maxID = std::max(maxID, unit.id_);
    }

    int newID = (preferredID_ != INVALID) ? preferredID_ : (maxID + 1);

    units.emplace_back(newID, house_, itemID_, health_, position_, angle_, attack_mode_);

    return std::make_unique<MapEditorRemoveUnitOperation>(newID);
}

std::unique_ptr<MapEditorOperation> MapEditorRemoveUnitOperation::perform(MapEditor* pMapEditor) {

    std::vector<MapEditor::Unit>& units = pMapEditor->getUnitList();

    for (auto iter = units.begin(); iter != units.end(); ++iter) {
        if (iter->id_ == id_) {
            auto redoOperation =
                std::make_unique<MapEditorUnitPlaceOperation>(iter->id_, iter->position_, iter->house_, iter->itemID_,
                                                              iter->health_, iter->angle_, iter->attack_mode_);

            units.erase(iter);

            return redoOperation;
        }
    }

    return std::make_unique<MapEditorNoOperation>();
}

std::unique_ptr<MapEditorOperation> MapEditorEditStructureOperation::perform(MapEditor* pMapEditor) {

    MapEditor::Structure* pStructure = pMapEditor->getStructure(id_);

    int oldHealth = pStructure->health_;

    pStructure->health_ = health_;

    return std::make_unique<MapEditorEditStructureOperation>(id_, oldHealth);
}

std::unique_ptr<MapEditorOperation> MapEditorEditUnitOperation::perform(MapEditor* pMapEditor) {

    MapEditor::Unit* pUnit = pMapEditor->getUnit(id_);

    int oldHealth            = pUnit->health_;
    ANGLETYPE oldAngle       = pUnit->angle_;
    ATTACKMODE oldAttackmode = pUnit->attack_mode_;

    pUnit->health_      = health_;
    pUnit->angle_       = angle_;
    pUnit->attack_mode_ = attack_mode_;

    return std::make_unique<MapEditorEditUnitOperation>(id_, oldHealth, oldAngle, oldAttackmode);
}

std::unique_ptr<MapEditorOperation> MapEditorChangePlayer::perform(MapEditor* pMapEditor) {

    auto& player = pMapEditor->getPlayers()[playerNum_];

    bool bOldActive      = player.bActive_;
    bool bOldAnyHouse    = player.bAnyHouse_;
    int oldCredits       = player.credits_;
    std::string oldBrain = player.brain_;
    int oldQuota         = player.quota_;
    int oldMaxunit       = player.maxunit_;

    player.bActive_   = bActive_;
    player.bAnyHouse_ = bAnyHouse_;
    player.credits_   = credits_;
    player.brain_     = brain_;
    player.quota_     = quota_;
    player.maxunit_   = max_unit_;

    pMapEditor->informPlayersChanged();

    return std::make_unique<MapEditorChangePlayer>(playerNum_, bOldActive, bOldAnyHouse, oldCredits, oldBrain, oldQuota,
                                                   oldMaxunit);
}

std::unique_ptr<MapEditorOperation> MapEditorChangeChoam::perform(MapEditor* pMapEditor) {

    auto& choam = pMapEditor->getChoam();

    int oldAmount = choam.contains(itemID_) ? choam[itemID_] : -1;

    if (amount_ >= 0) {
        choam[itemID_] = amount_;
    } else {
        // for -1 or other negative values we remove this item from choam
        // (THIS IS DIFFERENT TO -1 IN THE INI FILE WHERE -1 MEANS AN AMOUNT OF 0)
        choam.erase(itemID_);
    }

    return std::make_unique<MapEditorChangeChoam>(itemID_, oldAmount);
}

std::unique_ptr<MapEditorOperation> MapEditorChangeReinforcements::perform(MapEditor* pMapEditor) {

    std::vector<ReinforcementInfo> oldReinforcements = pMapEditor->getReinforcements();

    pMapEditor->setReinforcements(reinforcements_);

    return std::make_unique<MapEditorChangeReinforcements>(oldReinforcements);
}

std::unique_ptr<MapEditorOperation> MapEditorChangeTeams::perform(MapEditor* pMapEditor) {

    std::vector<AITeamInfo> oldAITeams = pMapEditor->getAITeams();

    pMapEditor->setAITeams(ai_teams_);

    return std::make_unique<MapEditorChangeTeams>(oldAITeams);
}

std::unique_ptr<MapEditorOperation> MapEditorChangeMapInfo::perform(MapEditor* pMapEditor) {

    MapInfo oldMapInfo = pMapEditor->getMapInfo();

    pMapEditor->setMapInfo(mapInfo_);

    return std::make_unique<MapEditorChangeMapInfo>(oldMapInfo);
}
