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

#include <MapEditor/MapGenerator.h>
#include <MapEditor/MapMirror.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/INIFile.h>
#include <FileClasses/LoadSavePNG.h>

#include <structures/Wall.h>

#include <misc/FileSystem.h>
#include <misc/draw_util.h>
#include <fmt/printf.h>

#include <globals.h>
#include <mmath.h>
#include <sand.h>
#include <main.h>
#include <Tile.h>

#include <config.h>

#include <typeinfo>
#include <algorithm>



MapEditor::MapEditor() : pInterface(nullptr) {
    bQuitEditor = false;
    scrollDownMode = false;
    scrollLeftMode = false;
    scrollRightMode = false;
    scrollUpMode = false;
    shift = false;

    bChangedSinceLastSave = false;

    bLeftMousePressed = false;
    lastTerrainEditPosX = -1;
    lastTerrainEditPosY = -1;

    selectedUnitID = INVALID;
    selectedStructureID = INVALID;
    selectedMapItemCoord.invalidate();

    currentZoomlevel = settings.video.preferredZoomLevel;

    sideBarPos = calcAlignedDrawingRect(pGFXManager->getUIGraphic(UI_SideBar), HAlign::Right, VAlign::Top);
    topBarPos = calcAlignedDrawingRect(pGFXManager->getUIGraphic(UI_TopBar), HAlign::Left, VAlign::Top);
    bottomBarPos = calcAlignedDrawingRect(pGFXManager->getUIGraphic(UI_MapEditor_BottomBar), HAlign::Left, VAlign::Bottom);

    SDL_Rect gameBoardRect = { 0, topBarPos.h, sideBarPos.x, getRendererHeight() - topBarPos.h - bottomBarPos.h };
    screenborder = std::make_unique<ScreenBorder>(gameBoardRect);

    setMap(MapData(128,128,Terrain_Sand), MapInfo());
    setMirrorMode(MirrorModeNone);

    pInterface = std::make_unique<MapEditorInterface>(this);

    pInterface->onNew();
}

MapEditor::~MapEditor() { screenborder.reset(); }

std::string MapEditor::generateMapname() {
    const auto numPlayers =
        std::count_if(players.begin(), players.end(), [](const MapEditor::Player& player) { return player.bActive; });

    return std::to_string(numPlayers) + "P - " + std::to_string(map.getSizeX()) + "x" + std::to_string(map.getSizeY()) +
           " - " + _("New Map");
}

void MapEditor::setMirrorMode(MirrorMode newMirrorMode) {
    currentMirrorMode = newMirrorMode;

    mapMirror = MapMirror::createMapMirror(currentMirrorMode,map.getSizeX(), map.getSizeY());
}

void MapEditor::RunEditor() {
    while(!bQuitEditor) {
        const int frameStart = static_cast<int>(SDL_GetTicks());

        processInput();
        drawScreen();

        const int frameTime = static_cast<int>(SDL_GetTicks()) - frameStart;
        if(settings.video.frameLimit) {
            if(frameTime >= 0 && frameTime < 32) {
                SDL_Delay(32 - frameTime);
            }
        }
    }
}

void MapEditor::setMap(const MapData& mapdata, const MapInfo& newMapInfo) {
    map = mapdata;
    mapInfo = newMapInfo;

    screenborder->adjustScreenBorderToMapsize(map.getSizeX(),map.getSizeY());

    // reset tools
    selectedUnitID = INVALID;
    selectedStructureID = INVALID;
    selectedMapItemCoord.invalidate();

    if(pInterface != nullptr) {
        pInterface->deselectAll();
    }

    while(!redoOperationStack.empty()) {
        redoOperationStack.pop();
    }

    while(!undoOperationStack.empty()) {
        undoOperationStack.pop();
    }

    // reset other map properties
    loadedINIFile.reset();
    lastSaveName = "";

    spiceBlooms.clear();
    specialBlooms.clear();
    spiceFields.clear();
    choam.clear();
    reinforcements.clear();
    aiteams.clear();
    structures.clear();
    units.clear();
    players.clear();

    // setup default players
    if(getMapVersion() < 2) {
        players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_HARKONNEN),HOUSETYPE::HOUSE_HARKONNEN,HOUSETYPE::HOUSE_HARKONNEN,true,false,"Human",25);
        players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ATREIDES),HOUSETYPE::HOUSE_ATREIDES,HOUSETYPE::HOUSE_ATREIDES,true,false,"CPU",25);
        players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ORDOS),HOUSETYPE::HOUSE_ORDOS,HOUSETYPE::HOUSE_ORDOS,true,false,"CPU",25);
        players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_FREMEN),HOUSETYPE::HOUSE_FREMEN,HOUSETYPE::HOUSE_FREMEN,false,false,"CPU",25);
        players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_SARDAUKAR),HOUSETYPE::HOUSE_SARDAUKAR,HOUSETYPE::HOUSE_SARDAUKAR,true,false,"CPU",25);
        players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_MERCENARY),HOUSETYPE::HOUSE_MERCENARY,HOUSETYPE::HOUSE_MERCENARY,false,false,"CPU",25);
    } else {
        players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_HARKONNEN),HOUSETYPE::HOUSE_HARKONNEN,HOUSETYPE::HOUSE_HARKONNEN,true,true,"Team1");
        players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ATREIDES),HOUSETYPE::HOUSE_ATREIDES,HOUSETYPE::HOUSE_ATREIDES,true,true,"Team2");
        players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ORDOS),HOUSETYPE::HOUSE_ORDOS,HOUSETYPE::HOUSE_ORDOS,true,true,"Team3");
        players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_FREMEN),HOUSETYPE::HOUSE_FREMEN,HOUSETYPE::HOUSE_FREMEN,false,false,"Team4");
        players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_SARDAUKAR),HOUSETYPE::HOUSE_SARDAUKAR,HOUSETYPE::HOUSE_SARDAUKAR,true,true,"Team5");
        players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_MERCENARY),HOUSETYPE::HOUSE_MERCENARY,HOUSETYPE::HOUSE_MERCENARY,false,false,"Team6");
    }

    // setup default choam
    choam[Unit_Carryall] = 2;
    choam[Unit_Harvester] = 4;
    choam[Unit_Launcher] = 5;
    choam[Unit_MCV] = 2;
    choam[Unit_Ornithopter] = 5;
    choam[Unit_Quad] = 5;
    choam[Unit_SiegeTank] = 6;
    choam[Unit_Tank] = 6;
    choam[Unit_Trike] = 5;

    if(pInterface != nullptr) {
        pInterface->onNewMap();
        pInterface->onHouseChanges();
    }

    currentEditorMode = EditorMode();

    bChangedSinceLastSave = true;
}

bool MapEditor::isTileBlocked(int x, int y, bool bSlabIsBlocking, bool bUnitsAreBlocking) const {
    for(const Structure& structure : structures) {
        if(!bSlabIsBlocking && ((structure.itemID == Structure_Slab1) || (structure.itemID == Structure_Slab4)) ) {
            continue;
        }

        Coord structureSize = getStructureSize(structure.itemID);
        Coord position = structure.position;
        if((x >= position.x) && (x < position.x+structureSize.x) && (y >= position.y) && (y < position.y+structureSize.y)) {
            return true;
        }
    }

    if(bUnitsAreBlocking) {
        for(const Unit& unit : units) {
            if((x == unit.position.x) && (y == unit.position.y)) {
                return true;
            }
        }
    }

    return false;
}

std::vector<int> MapEditor::getMirrorStructures(int structureID) const {
    std::vector<int> mirrorStructures;

    const auto* pStructure = getStructure(structureID);

    if(pStructure == nullptr) {
        return mirrorStructures;
    }

    const Coord structureSize = getStructureSize(pStructure->itemID);

    for(int i=0;i<mapMirror->getSize();i++) {
        Coord position = mapMirror->getCoord( pStructure->position, i, structureSize);

        for(const Structure& structure : structures) {
            if(structure.position == position) {
                mirrorStructures.push_back(structure.id);
                break;
            }
        }
    }

    return mirrorStructures;
}

std::vector<int> MapEditor::getMirrorUnits(int unitID, bool bAddMissingAsInvalid) const {
    std::vector<int> mirrorUnits;

    const Unit* pUnit = getUnit(unitID);

    if(pUnit == nullptr) {
        return mirrorUnits;
    }

    for(int i=0;i<mapMirror->getSize();i++) {
        Coord position = mapMirror->getCoord( pUnit->position, i);

        for(const Unit& unit : units) {
            if(unit.position == position) {
                mirrorUnits.push_back(unit.id);
                break;
            }
        }

        if(bAddMissingAsInvalid && (mirrorUnits.size() < static_cast<unsigned int>(i + 1) )) {
            mirrorUnits.push_back(INVALID);
        }
    }

    return mirrorUnits;
}

void MapEditor::setEditorMode(const EditorMode& newEditorMode) {

    if(pInterface != nullptr) {
        pInterface->deselectObject();
    }

    selectedUnitID = INVALID;
    selectedStructureID = INVALID;
    selectedMapItemCoord.invalidate();

    currentEditorMode = newEditorMode;
}

void MapEditor::startOperation() {
    if(undoOperationStack.empty() || !dynamic_cast<MapEditorStartOperation*>( undoOperationStack.top().get() )) {
        addUndoOperation(std::make_unique<MapEditorStartOperation>());
    }
}

void MapEditor::undoLastOperation() {
    if(!undoOperationStack.empty()) {
        redoOperationStack.push(std::make_unique<MapEditorStartOperation>());

        while((!undoOperationStack.empty()) && !dynamic_cast<MapEditorStartOperation*>( undoOperationStack.top().get() )) {
            redoOperationStack.push(undoOperationStack.top()->perform(this));
            undoOperationStack.pop();
        }

        if(!undoOperationStack.empty()) {
            undoOperationStack.pop();
        }
    }
}

void MapEditor::redoLastOperation() {
    if(!redoOperationStack.empty()) {
        undoOperationStack.push(std::make_unique<MapEditorStartOperation>());

        while((!redoOperationStack.empty()) && !dynamic_cast<MapEditorStartOperation*>( redoOperationStack.top().get() )) {
            undoOperationStack.push(redoOperationStack.top()->perform(this));
            redoOperationStack.pop();
        }

        if(!redoOperationStack.empty()) {
            redoOperationStack.pop();
        }
    }
}

void MapEditor::loadMap(const std::filesystem::path& filepath) {
    // reset tools
    selectedUnitID = INVALID;
    selectedStructureID = INVALID;

    if(pInterface != nullptr) {
        pInterface->deselectAll();
    }

    while(!redoOperationStack.empty()) {
        redoOperationStack.pop();
    }

    while(!undoOperationStack.empty()) {
        undoOperationStack.pop();
    }

    // reset other map properties
    spiceBlooms.clear();
    specialBlooms.clear();
    spiceFields.clear();
    choam.clear();
    reinforcements.clear();
    aiteams.clear();
    structures.clear();
    units.clear();
    players.clear();

    players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_HARKONNEN),HOUSETYPE::HOUSE_HARKONNEN,HOUSETYPE::HOUSE_HARKONNEN,false,true,"Team1");
    players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ATREIDES),HOUSETYPE::HOUSE_ATREIDES,HOUSETYPE::HOUSE_ATREIDES,false,true,"Team2");
    players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ORDOS),HOUSETYPE::HOUSE_ORDOS,HOUSETYPE::HOUSE_ORDOS,false,true,"Team3");
    players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_FREMEN),HOUSETYPE::HOUSE_FREMEN,HOUSETYPE::HOUSE_FREMEN,false,false,"Team4");
    players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_SARDAUKAR),HOUSETYPE::HOUSE_SARDAUKAR,HOUSETYPE::HOUSE_SARDAUKAR,false,false,"Team5");
    players.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_MERCENARY),HOUSETYPE::HOUSE_MERCENARY,HOUSETYPE::HOUSE_MERCENARY,false,false,"Team6");

    // load map
    loadedINIFile = std::make_unique<INIFile>(filepath, false);
    lastSaveName = filepath;

    // do the actual loading
    INIMapEditorLoader INIMapEditorLoader(this, loadedINIFile.get());

    // update interface
    if(pInterface != nullptr) {
        pInterface->onNewMap();
        pInterface->onHouseChanges();
    }

    currentEditorMode = EditorMode();

    bChangedSinceLastSave = false;
}

void MapEditor::saveMap(const std::filesystem::path& filepath) {
    if(!loadedINIFile) {
        std::string comment = "Created with Dune Legacy " + std::string(VERSION) + " Map Editor.";
        loadedINIFile = std::make_unique<INIFile>(false, comment);
    }

    int version = (mapInfo.mapSeed == INVALID) ? 2 : 1;

    if(version > 1) {
        loadedINIFile->setIntValue("BASIC", "Version", version);
    }

    if(!mapInfo.license.empty()) {
        loadedINIFile->setStringValue("BASIC", "License", mapInfo.license);
    }

    if(!mapInfo.author.empty()) {
        loadedINIFile->setStringValue("BASIC", "Author", mapInfo.author);
    }

    if((version > 1) && (mapInfo.techLevel > 0)) {
        loadedINIFile->setIntValue("BASIC", "TechLevel", mapInfo.techLevel);
    }

    loadedINIFile->setIntValue("BASIC", "WinFlags", mapInfo.winFlags);
    loadedINIFile->setIntValue("BASIC", "LoseFlags", mapInfo.loseFlags);

    loadedINIFile->setStringValue("BASIC", "LosePicture", mapInfo.losePicture, false);
    loadedINIFile->setStringValue("BASIC", "WinPicture", mapInfo.winPicture, false);
    loadedINIFile->setStringValue("BASIC", "BriefPicture", mapInfo.briefPicture, false);

    loadedINIFile->setIntValue("BASIC","TimeOut", mapInfo.timeout);

    int logicalSizeX = 0;
    //int logicalSizeY;
    int logicalOffsetX = 0;
    int logicalOffsetY = 0;

    if(version < 2) {
        logicalSizeX = 64;
        //logicalSizeY = 64;

        int mapscale = 0;
        switch(map.getSizeX()) {
            case 21: {
                mapscale = 2;
                logicalOffsetX = logicalOffsetY = 11;
            } break;
            case 32: {
                mapscale = 1;
                logicalOffsetX = logicalOffsetY = 16;
            } break;
            case 62:
            default: {
                mapscale = 0;
                logicalOffsetX = logicalOffsetY = 1;
            } break;

        }

        loadedINIFile->setIntValue("BASIC", "MapScale", mapscale);

        int cursorPos = (logicalOffsetY+mapInfo.cursorPos.y) * logicalSizeX + (logicalOffsetX+mapInfo.cursorPos.x);
        loadedINIFile->setIntValue("BASIC", "CursorPos", cursorPos);
        int tacticalPos = (logicalOffsetY+mapInfo.tacticalPos.y) * logicalSizeX + (logicalOffsetX+mapInfo.tacticalPos.x);
        loadedINIFile->setIntValue("BASIC", "TacticalPos", tacticalPos);

        // field, spice bloom and special bloom
        std::string strSpiceBloom;
        for(size_t i=0;i<spiceBlooms.size();++i) {
            if(i>0) {
                strSpiceBloom += ",";
            }

            int position = (logicalOffsetY+spiceBlooms[i].y) * logicalSizeX + (logicalOffsetX+spiceBlooms[i].x);
            strSpiceBloom += std::to_string(position);
        }

        if(!strSpiceBloom.empty()) {
            loadedINIFile->setStringValue("MAP", "Bloom", strSpiceBloom, false);
        } else {
            loadedINIFile->removeKey("MAP", "Bloom");
        }


        std::string strSpecialBloom;
        for(size_t i=0;i<specialBlooms.size();++i) {
            if(i>0) {
                strSpecialBloom += ",";
            }

            int position = (logicalOffsetY+specialBlooms[i].y) * logicalSizeX + (logicalOffsetX+specialBlooms[i].x);
            strSpecialBloom += std::to_string(position);
        }

        if(!strSpecialBloom.empty()) {
            loadedINIFile->setStringValue("MAP", "Special", strSpecialBloom, false);
        } else {
            loadedINIFile->removeKey("MAP", "Special");
        }


        std::string strFieldBloom;
        for(size_t i=0;i<spiceFields.size();++i) {
            if(i>0) {
                strFieldBloom += ",";
            }

            int position = (logicalOffsetY+spiceFields[i].y) * logicalSizeX + (logicalOffsetX+spiceFields[i].x);
            strFieldBloom += std::to_string(position);
        }

        if(!strFieldBloom.empty()) {
            loadedINIFile->setStringValue("MAP", "Field", strFieldBloom, false);
        } else {
            loadedINIFile->removeKey("MAP", "Field");
        }


        loadedINIFile->setIntValue("MAP", "Seed", mapInfo.mapSeed);
    } else {
        logicalSizeX = map.getSizeX();
        //logicalSizeY = map.getSizeY();
        logicalOffsetX = logicalOffsetY = 0;

        loadedINIFile->clearSection("MAP");
        loadedINIFile->setIntValue("MAP", "SizeX", map.getSizeX());
        loadedINIFile->setIntValue("MAP", "SizeY", map.getSizeY());

        for(int y = 0; y < map.getSizeY(); y++) {
            std::string rowKey = fmt::sprintf("%.3d", y);

            std::string row;
            for(int x = 0; x < map.getSizeX(); x++) {
                switch(map(x,y)) {

                    case Terrain_Dunes: {
                        // Sand dunes
                        row += '^';
                    } break;

                    case Terrain_Spice: {
                        // Spice
                        row += '~';
                    } break;

                    case Terrain_ThickSpice: {
                        // Thick spice
                        row += '+';
                    } break;

                    case Terrain_Rock: {
                        // Rock
                        row += '%';
                    } break;

                    case Terrain_Mountain: {
                        // Mountain
                        row += '@';
                    } break;

                    case Terrain_SpiceBloom: {
                        // Spice Bloom
                        row += 'O';
                    } break;

                    case Terrain_SpecialBloom: {
                        // Special Bloom
                        row += 'Q';
                    } break;

                    case Terrain_Sand:
                    default: {
                        // Normal sand
                        row += '-';
                    } break;
                }
            }

            loadedINIFile->setStringValue("MAP", rowKey, row, false);
        }
    }


    for(int i = 1; i <= static_cast<int>(HOUSETYPE::NUM_HOUSES); i++) {
        loadedINIFile->removeSection("player" + std::to_string(i));
    }

    std::vector<std::string> house2housename;
    house2housename.reserve(players.size());

    int currentAnyHouseNumber = 1;
    for(const Player& player : players) {
        if(player.bAnyHouse) {
            house2housename.emplace_back("Player" + std::to_string(currentAnyHouseNumber));
        } else {
            house2housename.emplace_back(player.name);
        }

        if(player.bActive) {
            const auto& h2h = house2housename.back();
            if(version < 2) {
                loadedINIFile->setIntValue(h2h, "Quota", player.quota);
                loadedINIFile->setIntValue(h2h, "Credits", player.credits);
                loadedINIFile->setStringValue(h2h, "Brain", player.brain, false);
                loadedINIFile->setIntValue(h2h, "MaxUnit", player.maxunit);
            } else {
                if(player.quota > 0) {
                    loadedINIFile->setIntValue(h2h, "Quota", player.quota);
                } else {
                    loadedINIFile->removeKey(h2h, "Quota");
                }
                loadedINIFile->setIntValue(h2h, "Credits", player.credits);
                loadedINIFile->setStringValue(h2h, "Brain", player.brain, false);

                if(player.bAnyHouse) {
                    currentAnyHouseNumber++;
                }
            }

            if(player.bAnyHouse) {
                // remove corresponding house name
                loadedINIFile->removeSection(player.name);
            }

        } else {
            // remove corresponding house name
            loadedINIFile->removeSection(player.name);
        }
    }

    // remove players that are leftovers
    for(int i = currentAnyHouseNumber; i < static_cast<int>(HOUSETYPE::NUM_HOUSES); i++) {
        loadedINIFile->removeSection("Player" + std::to_string(i));
    }


    if(choam.empty()) {
        loadedINIFile->removeSection("CHOAM");
    } else {
        loadedINIFile->clearSection("CHOAM");

        for(auto& choamEntry : choam) {
            ItemID_enum itemID = choamEntry.first;
            int num = choamEntry.second;

            if(num == 0) {
                num = -1;
            }

            loadedINIFile->setIntValue("CHOAM", getItemNameByID(itemID), num);
        }
    }

    if(aiteams.empty()) {
        loadedINIFile->removeSection("TEAMS");
    } else {
        loadedINIFile->clearSection("TEAMS");

        // we start at 0 for version 1 maps if we have 16 entries to not overflow the table
        int currentIndex = ((getMapVersion() < 2) && (aiteams.size() >= 16)) ? 0 : 1;
        for(const AITeamInfo& aiteamInfo : aiteams) {
            std::string value = house2housename[static_cast<int>(aiteamInfo.houseID)] + "," +
                                getAITeamBehaviorNameByID(aiteamInfo.aiTeamBehavior) + "," +
                                getAITeamTypeNameByID(aiteamInfo.aiTeamType) + "," +
                                std::to_string(aiteamInfo.minUnits) + "," + std::to_string(aiteamInfo.maxUnits);
            loadedINIFile->setStringValue("TEAMS", std::to_string(currentIndex), value, false);
            currentIndex++;
        }
    }


    loadedINIFile->clearSection("UNITS");
    for(const Unit& unit : units) {
        if(unit.itemID < ItemID_enum::ItemID_FirstID || unit.itemID > ItemID_enum::ItemID_LastID) continue;

        std::string unitKey = fmt::sprintf("ID%.3d", unit.id);

        int position = (logicalOffsetY+unit.position.y) * logicalSizeX + (logicalOffsetX+unit.position.x);

        int angle = (int) unit.angle;

        angle = (((static_cast<int>(ANGLETYPE::NUM_ANGLES) - angle) + 2) % static_cast<int>(ANGLETYPE::NUM_ANGLES)) * 32;

        std::string unitValue = house2housename[static_cast<int>(unit.house)] + "," + getItemNameByID(unit.itemID) + "," + std::to_string(unit.health)
                                + "," + std::to_string(position) + "," + std::to_string(angle) + "," + getAttackModeNameByMode(unit.attackmode);

        loadedINIFile->setStringValue("UNITS", unitKey, unitValue, false);
    }

    loadedINIFile->clearSection("STRUCTURES");
    for(const Structure& structure : structures) {
        int position = (logicalOffsetY+structure.position.y) * logicalSizeX + (logicalOffsetX+structure.position.x);

        if((structure.itemID == Structure_Slab1) || (structure.itemID == Structure_Slab4) || (structure.itemID == Structure_Wall)) {
            std::string structureKey = fmt::sprintf("GEN%.3d", position);

            std::string structureValue = house2housename[static_cast<int>(structure.house)] + "," + getItemNameByID(structure.itemID);

            loadedINIFile->setStringValue("STRUCTURES", structureKey, structureValue, false);

        } else {

            std::string structureKey = fmt::sprintf("ID%.3d", structure.id);

            std::string structureValue = house2housename[static_cast<int>(structure.house)] + "," + getItemNameByID(structure.itemID) + "," + std::to_string(structure.health) + "," + std::to_string(position);

            loadedINIFile->setStringValue("STRUCTURES", structureKey, structureValue, false);
        }
    }

    if(reinforcements.empty()) {
        loadedINIFile->removeSection("REINFORCEMENTS");
    } else {
        loadedINIFile->clearSection("REINFORCEMENTS");

        // we start at 0 for version 1 maps if we have 16 entries to not overflow the table
        int currentIndex = ((getMapVersion() < 2) && (reinforcements.size() >= 16)) ? 0 : 1;
        for(const ReinforcementInfo& reinforcement : reinforcements) {
            std::string value = house2housename[static_cast<int>(reinforcement.houseID)] + "," + getItemNameByID(reinforcement.unitID) + "," + getDropLocationNameByID(reinforcement.dropLocation) + "," + std::to_string(reinforcement.droptime);
            if(reinforcement.bRepeat) {
                value += ",+";
            }
            loadedINIFile->setStringValue("REINFORCEMENTS", std::to_string(currentIndex), value, false);
            currentIndex++;
        }
    }

    if (!loadedINIFile->saveChangesTo(filepath, getMapVersion() < 2)) {
        sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Unable to save configuration file %s", filepath.u8string().c_str());
    }

    lastSaveName = filepath;
    bChangedSinceLastSave = false;
}

void MapEditor::performMapEdit(int xpos, int ypos, bool bRepeated) {
    switch(currentEditorMode.mode) {
        case EditorMode::EditorMode_Terrain: {
            clearRedoOperations();

            if(!bRepeated) {
                startOperation();
            }

            if(getMapVersion() < 2) {
                // classic map
                if(!bRepeated && map.isInsideMap(xpos, ypos)) {
                    TERRAINTYPE terrainType = currentEditorMode.terrainType;

                    switch(terrainType) {
                        case Terrain_SpiceBloom: {
                            MapEditorTerrainAddSpiceBloomOperation editOperation(xpos, ypos);
                            addUndoOperation(editOperation.perform(this));
                        } break;

                        case Terrain_SpecialBloom: {
                            MapEditorTerrainAddSpecialBloomOperation editOperation(xpos, ypos);
                            addUndoOperation(editOperation.perform(this));
                        } break;

                        case Terrain_Spice: {
                            MapEditorTerrainAddSpiceFieldOperation editOperation(xpos, ypos);
                            addUndoOperation(editOperation.perform(this));
                        } break;

                        default: {
                        } break;
                    }


                }

            } else {
                for(int i=0;i<mapMirror->getSize();i++) {

                    Coord position = mapMirror->getCoord( Coord(xpos, ypos), i);

                    int halfsize = currentEditorMode.pensize/2;
                    for(int y = position.y - halfsize; y <= position.y + halfsize; y++) {
                        for(int x = position.x - halfsize; x <= position.x + halfsize; x++) {
                            if(map.isInsideMap(x, y)) {
                                performTerrainChange(x, y, currentEditorMode.terrainType);
                            }
                        }
                    }

                }
            }

        } break;

        case EditorMode::EditorMode_Structure: {
            if(!bRepeated || currentEditorMode.itemID == Structure_Slab1 || currentEditorMode.itemID == Structure_Wall) {

                Coord structureSize = getStructureSize(currentEditorMode.itemID);

                if(!mapMirror->mirroringPossible( Coord(xpos, ypos), structureSize)) {
                    return;
                }

                // check if all places are free
                for(int i=0;i<mapMirror->getSize();i++) {
                    Coord position = mapMirror->getCoord( Coord(xpos, ypos), i, structureSize);

                    for(int x = position.x; x < position.x + structureSize.x; x++) {
                        for(int y = position.y; y < position.y + structureSize.y; y++) {
                            if(!map.isInsideMap(x,y) || isTileBlocked(x, y, true, (currentEditorMode.itemID != Structure_Slab1) )) {
                                return;
                            }
                        }
                    }
                }

                clearRedoOperations();

                if(!bRepeated) {
                    startOperation();
                }

                auto currentHouse = currentEditorMode.house;
                bool bHouseIsActive = players[static_cast<int>(currentHouse)].bActive;
                for(int i=0;i<mapMirror->getSize();i++) {

                    auto nextHouse = HOUSETYPE::HOUSE_INVALID;
                    for(int k = static_cast<int>(currentHouse); k < static_cast<int>(currentHouse) + static_cast<int>(HOUSETYPE::NUM_HOUSES); k++) {
                        if(players[k % static_cast<int>(HOUSETYPE::NUM_HOUSES)].bActive == bHouseIsActive) {
                            nextHouse = static_cast<HOUSETYPE>(k % static_cast<int>(HOUSETYPE::NUM_HOUSES));
                            break;
                        }
                    }

                    if(nextHouse != HOUSETYPE::HOUSE_INVALID) {
                        Coord position = mapMirror->getCoord( Coord(xpos, ypos), i, structureSize);

                        MapEditorStructurePlaceOperation placeOperation(position, nextHouse, currentEditorMode.itemID, currentEditorMode.health);

                        addUndoOperation(placeOperation.perform(this));

                        currentHouse = static_cast<HOUSETYPE>((static_cast<int>(nextHouse) + 1) % static_cast<int>(HOUSETYPE::NUM_HOUSES));
                    }
                }
            }
        } break;

        case EditorMode::EditorMode_Unit: {
            if(!bRepeated) {

                // first check if all places are free
                for(int i=0;i<mapMirror->getSize();i++) {
                    Coord position = mapMirror->getCoord( Coord(xpos, ypos), i);

                    if(!map.isInsideMap(position.x,position.y) || isTileBlocked(position.x, position.y, false, true)) {
                        return;
                    }
                }

                clearRedoOperations();

                startOperation();


                auto currentHouse = currentEditorMode.house;
                bool bHouseIsActive = players[static_cast<int>(currentHouse)].bActive;
                for(int i=0;i<mapMirror->getSize();i++) {

                    auto nextHouse = HOUSETYPE::HOUSE_INVALID;
                    for(int k = static_cast<int>(currentHouse);
                        k < static_cast<int>(currentHouse) + static_cast<int>(HOUSETYPE::NUM_HOUSES); k++) {
                        if(players[k % static_cast<int>(HOUSETYPE::NUM_HOUSES)].bActive == bHouseIsActive) {
                            nextHouse = static_cast<HOUSETYPE>(k % static_cast<int>(HOUSETYPE::NUM_HOUSES));
                            break;
                        }
                    }

                    if(nextHouse != HOUSETYPE::HOUSE_INVALID) {
                        Coord position = mapMirror->getCoord( Coord(xpos, ypos), i);

                        const auto angle =  mapMirror->getAngle(currentEditorMode.angle, i);

                        MapEditorUnitPlaceOperation placeOperation(position, nextHouse, currentEditorMode.itemID, currentEditorMode.health, angle, currentEditorMode.attackmode);

                        addUndoOperation(placeOperation.perform(this));
                        currentHouse = static_cast<HOUSETYPE>((static_cast<int>(nextHouse) + 1) % static_cast<int>(HOUSETYPE::NUM_HOUSES));
                    }
                }
            }
        } break;

        case EditorMode::EditorMode_TacticalPos: {
            if(!map.isInsideMap(xpos,ypos)) {
                return;
            }

            clearRedoOperations();

            startOperation();

            MapEditorSetTacticalPositionOperation setOperation(xpos,ypos);

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

    switch(terrainType) {
        case Terrain_Mountain: {
            if(map.isInsideMap(x-1, y) && (map(x-1,y) != Terrain_Mountain) && (map(x-1,y) != Terrain_Rock))     performTerrainChange(x-1,y,Terrain_Rock);
            if(map.isInsideMap(x, y-1) && (map(x,y-1) != Terrain_Mountain) && (map(x,y-1) != Terrain_Rock))     performTerrainChange(x,y-1,Terrain_Rock);
            if(map.isInsideMap(x+1, y) && (map(x+1,y) != Terrain_Mountain) && (map(x+1,y) != Terrain_Rock))     performTerrainChange(x+1,y,Terrain_Rock);
            if(map.isInsideMap(x, y+1) && (map(x,y+1) != Terrain_Mountain) && (map(x,y+1) != Terrain_Rock))     performTerrainChange(x,y+1,Terrain_Rock);
        } break;

        case Terrain_ThickSpice: {
            if(map.isInsideMap(x-1, y) && (map(x-1,y) != Terrain_ThickSpice) && (map(x-1,y) != Terrain_Spice))     performTerrainChange(x-1,y,Terrain_Spice);
            if(map.isInsideMap(x, y-1) && (map(x,y-1) != Terrain_ThickSpice) && (map(x,y-1) != Terrain_Spice))     performTerrainChange(x,y-1,Terrain_Spice);
            if(map.isInsideMap(x+1, y) && (map(x+1,y) != Terrain_ThickSpice) && (map(x+1,y) != Terrain_Spice))     performTerrainChange(x+1,y,Terrain_Spice);
            if(map.isInsideMap(x, y+1) && (map(x,y+1) != Terrain_ThickSpice) && (map(x,y+1) != Terrain_Spice))     performTerrainChange(x,y+1,Terrain_Spice);
        } break;

        case Terrain_Rock: {
            if(map.isInsideMap(x-1, y) && (map(x-1,y) == Terrain_ThickSpice))     performTerrainChange(x-1,y,Terrain_Spice);
            if(map.isInsideMap(x, y-1) && (map(x,y-1) == Terrain_ThickSpice))     performTerrainChange(x,y-1,Terrain_Spice);
            if(map.isInsideMap(x+1, y) && (map(x+1,y) == Terrain_ThickSpice))     performTerrainChange(x+1,y,Terrain_Spice);
            if(map.isInsideMap(x, y+1) && (map(x,y+1) == Terrain_ThickSpice))     performTerrainChange(x,y+1,Terrain_Spice);
        } break;

        case Terrain_Spice: {
            if(map.isInsideMap(x-1, y) && (map(x-1,y) == Terrain_Mountain))     performTerrainChange(x-1,y,Terrain_Rock);
            if(map.isInsideMap(x, y-1) && (map(x,y-1) == Terrain_Mountain))     performTerrainChange(x,y-1,Terrain_Rock);
            if(map.isInsideMap(x+1, y) && (map(x+1,y) == Terrain_Mountain))     performTerrainChange(x+1,y,Terrain_Rock);
            if(map.isInsideMap(x, y+1) && (map(x,y+1) == Terrain_Mountain))     performTerrainChange(x,y+1,Terrain_Rock);
        } break;

        case Terrain_Sand:
        case Terrain_Dunes:
        case Terrain_SpiceBloom:
        case Terrain_SpecialBloom: {
            if(map.isInsideMap(x-1, y) && (map(x-1,y) == Terrain_Mountain))     performTerrainChange(x-1,y,Terrain_Rock);
            if(map.isInsideMap(x, y-1) && (map(x,y-1) == Terrain_Mountain))     performTerrainChange(x,y-1,Terrain_Rock);
            if(map.isInsideMap(x+1, y) && (map(x+1,y) == Terrain_Mountain))     performTerrainChange(x+1,y,Terrain_Rock);
            if(map.isInsideMap(x, y+1) && (map(x,y+1) == Terrain_Mountain))     performTerrainChange(x,y+1,Terrain_Rock);

            if(map.isInsideMap(x-1, y) && (map(x-1,y) == Terrain_ThickSpice))     performTerrainChange(x-1,y,Terrain_Spice);
            if(map.isInsideMap(x, y-1) && (map(x,y-1) == Terrain_ThickSpice))     performTerrainChange(x,y-1,Terrain_Spice);
            if(map.isInsideMap(x+1, y) && (map(x+1,y) == Terrain_ThickSpice))     performTerrainChange(x+1,y,Terrain_Spice);
            if(map.isInsideMap(x, y+1) && (map(x,y+1) == Terrain_ThickSpice))     performTerrainChange(x,y+1,Terrain_Spice);
        } break;

        default: {
        } break;
    }

}

void MapEditor::drawScreen() {
    // clear whole screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    //the actuall map
    drawMap(screenborder.get(), false);

    pInterface->draw(Point(0,0));
    pInterface->drawOverlay(Point(0,0));

    // Cursor
    drawCursor();

    Dune_RenderPresent(renderer);
}

void MapEditor::processInput() {
    SDL_Event event;

    while(SDL_PollEvent(&event)) {

        // first of all update mouse
        if(event.type == SDL_MOUSEMOTION) {
            SDL_MouseMotionEvent* mouse = &event.motion;
            drawnMouseX = std::max(0, std::min(mouse->x, settings.video.width-1));
            drawnMouseY = std::max(0, std::min(mouse->y, settings.video.height-1));
        }

        if(pInterface->hasChildWindow()) {
            pInterface->handleInput(event);
        } else {
            switch (event.type) {
                case SDL_KEYDOWN:
                {
                    switch(event.key.keysym.sym) {

                        case SDLK_RETURN: {
                            if(SDL_GetModState() & KMOD_ALT) {
                                toogleFullscreen();
                            }
                        } break;

                        case SDLK_TAB: {
                            if(SDL_GetModState() & KMOD_ALT) {
                                SDL_MinimizeWindow(window);
                            }
                        } break;

                        case SDLK_F1: {
                            Coord oldCenterCoord = screenborder->getCurrentCenter();
                            currentZoomlevel = 0;
                            screenborder->adjustScreenBorderToMapsize(map.getSizeX(), map.getSizeY());
                            screenborder->setNewScreenCenter(oldCenterCoord);
                        } break;

                        case SDLK_F2: {
                            Coord oldCenterCoord = screenborder->getCurrentCenter();
                            currentZoomlevel = 1;
                            screenborder->adjustScreenBorderToMapsize(map.getSizeX(), map.getSizeY());
                            screenborder->setNewScreenCenter(oldCenterCoord);
                        } break;

                        case SDLK_F3: {
                            Coord oldCenterCoord = screenborder->getCurrentCenter();
                            currentZoomlevel = 2;
                            screenborder->adjustScreenBorderToMapsize(map.getSizeX(), map.getSizeY());
                            screenborder->setNewScreenCenter(oldCenterCoord);
                        } break;

                        case SDLK_p: {
                            if(SDL_GetModState() & KMOD_CTRL) {
                                saveMapshot();
                            }
                        } break;

                        case SDLK_PRINTSCREEN:
                        case SDLK_SYSREQ: {
                            saveMapshot();
                        } break;

                        case SDLK_z: {
                            if(SDL_GetModState() & KMOD_CTRL) {
                                    pInterface->onUndo();
                            }
                        } break;

                        case SDLK_y: {
                            if(SDL_GetModState() & KMOD_CTRL) {
                                    pInterface->onRedo();
                            }
                        } break;

                        default:
                            break;
                    }

                } break;

                case SDL_KEYUP:
                {
                    switch(event.key.keysym.sym) {
                        case SDLK_ESCAPE: {
                            // quiting
                            pInterface->onQuit();
                        } break;

                        case SDLK_DELETE:
                        case SDLK_BACKSPACE: {

                            // check units first
                            if(selectedUnitID != INVALID) {
                                clearRedoOperations();
                                startOperation();

                                std::vector<int> selectedUnits = getMirrorUnits(selectedUnitID);

                                for(int selectedUnit : selectedUnits) {
                                    MapEditorRemoveUnitOperation removeOperation(selectedUnit);
                                    addUndoOperation(removeOperation.perform(this));
                                }
                                selectedUnitID = INVALID;

                                pInterface->deselectAll();
                            } else if(selectedStructureID != INVALID) {
                                // We only try deleting structures if we had not yet deleted a unit (e.g. a unit on concrete)
                                clearRedoOperations();
                                startOperation();

                                std::vector<int> selectedStructures = getMirrorStructures(selectedStructureID);

                                for(int selectedStructure : selectedStructures) {
                                    MapEditorRemoveStructureOperation removeOperation(selectedStructure);
                                    addUndoOperation(removeOperation.perform(this));
                                }
                                selectedStructureID = INVALID;

                                pInterface->deselectAll();
                            } else if(selectedMapItemCoord.isValid()) {
                                auto iter = std::find(specialBlooms.begin(), specialBlooms.end(), selectedMapItemCoord);

                                if(iter != specialBlooms.end()) {
                                    clearRedoOperations();
                                    startOperation();
                                    MapEditorTerrainRemoveSpecialBloomOperation removeOperation(iter->x, iter->y);
                                    addUndoOperation(removeOperation.perform(this));

                                    selectedMapItemCoord.invalidate();
                                } else {
                                    iter = std::find(spiceBlooms.begin(), spiceBlooms.end(), selectedMapItemCoord);

                                    if(iter != spiceBlooms.end()) {
                                        clearRedoOperations();
                                        startOperation();
                                        MapEditorTerrainRemoveSpiceBloomOperation removeOperation(iter->x, iter->y);
                                        addUndoOperation(removeOperation.perform(this));

                                        selectedMapItemCoord.invalidate();
                                    } else {
                                        iter = std::find(spiceFields.begin(), spiceFields.end(), selectedMapItemCoord);

                                        if(iter != spiceFields.end()) {
                                            clearRedoOperations();
                                            startOperation();
                                            MapEditorTerrainRemoveSpiceFieldOperation removeOperation(iter->x, iter->y);
                                            addUndoOperation(removeOperation.perform(this));

                                            selectedMapItemCoord.invalidate();
                                        }
                                    }
                                }
                            }

                        } break;

                        default:
                            break;
                    }
                } break;

                case SDL_MOUSEMOTION:
                {
                    pInterface->handleMouseMovement(drawnMouseX,drawnMouseY);

                    if(bLeftMousePressed) {
                        if(screenborder->isScreenCoordInsideMap(drawnMouseX, drawnMouseY)) {
                            //if mouse is not over side bar

                            int xpos = screenborder->screen2MapX(drawnMouseX);
                            int ypos = screenborder->screen2MapY(drawnMouseY);

                            if((xpos != lastTerrainEditPosX) || (ypos != lastTerrainEditPosY)) {
                                performMapEdit(xpos, ypos, true);
                            }
                        }
                    }
                } break;

                case SDL_MOUSEWHEEL: {
                    if (event.wheel.y != 0) {
                        if(screenborder->isScreenCoordInsideMap(drawnMouseX, drawnMouseY)) {
                            //if mouse is not over side bar
                            int xpos = screenborder->screen2MapX(drawnMouseX);
                            int ypos = screenborder->screen2MapY(drawnMouseY);

                            for(const Unit& unit : units) {
                                Coord position = unit.position;
                                if((position.x == xpos) && (position.y == ypos)) {
                                    if(event.wheel.y > 0) {
                                        pInterface->onUnitRotateLeft(unit.id);
                                    } else {
                                        pInterface->onUnitRotateRight(unit.id);
                                    }
                                    break;
                                }
                            }
                        }

                        pInterface->handleMouseWheel(drawnMouseX,drawnMouseY,(event.wheel.y > 0));
                    }
                } break;

                case SDL_MOUSEBUTTONDOWN:
                {
                    SDL_MouseButtonEvent* mouse = &event.button;

                    switch(mouse->button) {
                        case SDL_BUTTON_LEFT: {
                            if(!pInterface->handleMouseLeft(mouse->x, mouse->y, true)) {

                                bLeftMousePressed = true;

                                if(screenborder->isScreenCoordInsideMap(mouse->x, mouse->y)) {
                                    //if mouse is not over side bar

                                    int xpos = screenborder->screen2MapX(mouse->x);
                                    int ypos = screenborder->screen2MapY(mouse->y);

                                    performMapEdit(xpos, ypos, false);
                                }
                            }

                        } break;

                        case SDL_BUTTON_RIGHT: {
                            if(!pInterface->handleMouseRight(mouse->x, mouse->y, true)) {

                                if(screenborder->isScreenCoordInsideMap(mouse->x, mouse->y)) {
                                    //if mouse is not over side bar
                                    setEditorMode(EditorMode());
                                    pInterface->deselectAll();
                                }
                            }
                        } break;

                    }
                } break;

                case SDL_MOUSEBUTTONUP:
                {
                    SDL_MouseButtonEvent* mouse = &event.button;

                    switch(mouse->button) {
                        case SDL_BUTTON_LEFT: {

                            pInterface->handleMouseLeft(mouse->x, mouse->y, false);

                            if(bLeftMousePressed) {

                                bLeftMousePressed = false;
                                lastTerrainEditPosX = -1;
                                lastTerrainEditPosY = -1;

                                if(currentEditorMode.mode == EditorMode::EditorMode_Selection) {
                                    if(screenborder->isScreenCoordInsideMap(mouse->x, mouse->y)) {
                                        //if mouse is not over side bar

                                        int xpos = screenborder->screen2MapX(mouse->x);
                                        int ypos = screenborder->screen2MapY(mouse->y);

                                        selectedUnitID = INVALID;
                                        selectedStructureID = INVALID;
                                        selectedMapItemCoord.invalidate();

                                        bool bUnitSelected = false;

                                        for(const Unit& unit : units) {
                                            Coord position = unit.position;

                                            if((position.x == xpos) && (position.y == ypos)) {
                                                selectedUnitID = unit.id;
                                                bUnitSelected = true;
                                                pInterface->onObjectSelected();
                                                mapInfo.cursorPos = position;
                                                break;
                                            }
                                        }

                                        bool bStructureSelected = false;

                                        for(const Structure& structure : structures) {
                                            const Coord& position = structure.position;
                                            Coord structureSize = getStructureSize(structure.itemID);

                                            if(!bUnitSelected && (xpos >= position.x) && (xpos < position.x+structureSize.x) && (ypos >= position.y) && (ypos < position.y+structureSize.y)) {
                                                selectedStructureID = structure.id;
                                                bStructureSelected = true;
                                                pInterface->onObjectSelected();
                                                mapInfo.cursorPos = position;
                                                break;
                                            }
                                        }

                                        if(!bUnitSelected && !bStructureSelected) {
                                            pInterface->deselectAll();

                                            // find map items (spice bloom, special bloom or spice field)
                                            if( (std::find(spiceBlooms.begin(), spiceBlooms.end(), Coord(xpos,ypos)) != spiceBlooms.end())
                                                || (std::find(specialBlooms.begin(), specialBlooms.end(), Coord(xpos,ypos)) != specialBlooms.end())
                                                || (std::find(spiceFields.begin(), spiceFields.end(), Coord(xpos,ypos)) != spiceFields.end())) {
                                                selectedMapItemCoord = Coord(xpos, ypos);
                                            }
                                        }


                                    }
                                }
                            }
                        } break;

                        case SDL_BUTTON_RIGHT: {
                            pInterface->handleMouseRight(mouse->x, mouse->y, false);
                        } break;
                    }
                } break;

                case SDL_QUIT:
                {
                    bQuitEditor = true;
                } break;
            }
        }
    }

    if((!pInterface->hasChildWindow()) && (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS)) {
        const Uint8 *keystate = SDL_GetKeyboardState(nullptr);
        scrollDownMode =  (drawnMouseY >= getRendererHeight()-1-SCROLLBORDER) || keystate[SDL_SCANCODE_DOWN];
        scrollLeftMode = (drawnMouseX <= SCROLLBORDER) || keystate[SDL_SCANCODE_LEFT];
        scrollRightMode = (drawnMouseX >= getRendererWidth()-1-SCROLLBORDER) || keystate[SDL_SCANCODE_RIGHT];
        scrollUpMode = (drawnMouseY <= SCROLLBORDER) || keystate[SDL_SCANCODE_UP];

        if(scrollLeftMode && scrollRightMode) {
            // do nothing
        } else if(scrollLeftMode) {
            scrollLeftMode = screenborder->scrollLeft();
        } else if(scrollRightMode) {
            scrollRightMode = screenborder->scrollRight();
        }

        if(scrollDownMode && scrollUpMode) {
            // do nothing
        } else if(scrollDownMode) {
            scrollDownMode = screenborder->scrollDown();
        } else if(scrollUpMode) {
            scrollUpMode = screenborder->scrollUp();
        }
    } else {
        scrollDownMode = false;
        scrollLeftMode = false;
        scrollRightMode = false;
        scrollUpMode = false;
    }
}

void MapEditor::drawCursor() {

    if(!(SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)) {
        return;
    }

    const DuneTexture* pCursor = nullptr;
    SDL_Rect dest = { 0, 0, 0, 0};
    if(scrollLeftMode || scrollRightMode || scrollUpMode || scrollDownMode) {
        if(scrollLeftMode && !scrollRightMode) {
            pCursor = pGFXManager->getUIGraphic(UI_CursorLeft);
            dest = calcDrawingRect(pCursor, drawnMouseX, drawnMouseY-5, HAlign::Left, VAlign::Top);
        } else if(scrollRightMode && !scrollLeftMode) {
            pCursor = pGFXManager->getUIGraphic(UI_CursorRight);
            dest = calcDrawingRect(pCursor, drawnMouseX, drawnMouseY-5, HAlign::Center, VAlign::Top);
        }

        if(pCursor == nullptr) {
            if(scrollUpMode && !scrollDownMode) {
                pCursor = pGFXManager->getUIGraphic(UI_CursorUp);
                dest = calcDrawingRect(pCursor, drawnMouseX-5, drawnMouseY, HAlign::Left, VAlign::Top);
            } else if(scrollDownMode && !scrollUpMode) {
                pCursor = pGFXManager->getUIGraphic(UI_CursorDown);
                dest = calcDrawingRect(pCursor, drawnMouseX-5, drawnMouseY, HAlign::Left, VAlign::Center);
            } else {
                pCursor = pGFXManager->getUIGraphic(UI_CursorNormal);
                dest = calcDrawingRect(pCursor, drawnMouseX, drawnMouseY, HAlign::Left, VAlign::Top);
            }
        }
    } else {
        pCursor = pGFXManager->getUIGraphic(UI_CursorNormal);
        dest = calcDrawingRect(pCursor, drawnMouseX, drawnMouseY, HAlign::Left, VAlign::Top);

        if((drawnMouseX < sideBarPos.x) && (drawnMouseY > topBarPos.h) && (currentMirrorMode != MirrorModeNone) && (!pInterface->hasChildWindow())) {

            const DuneTexture* pMirrorIcon = nullptr;
            switch(currentMirrorMode) {
                case MirrorModeHorizontal:  pMirrorIcon = pGFXManager->getUIGraphic(UI_MapEditor_MirrorHorizontalIcon);  break;
                case MirrorModeVertical:    pMirrorIcon = pGFXManager->getUIGraphic(UI_MapEditor_MirrorVerticalIcon);    break;
                case MirrorModeBoth:        pMirrorIcon = pGFXManager->getUIGraphic(UI_MapEditor_MirrorBothIcon);        break;
                case MirrorModePoint:       pMirrorIcon = pGFXManager->getUIGraphic(UI_MapEditor_MirrorPointIcon);       break;
                default:                    pMirrorIcon = pGFXManager->getUIGraphic(UI_MapEditor_MirrorNoneIcon);       break;
            }

            if (pMirrorIcon) pMirrorIcon->draw(renderer, drawnMouseX + 5, drawnMouseY + 5);
        }
    }

    if (pCursor) pCursor->draw(renderer, dest.x, dest.y);
}

TERRAINTYPE MapEditor::getTerrain(int x, int y) const {
    TERRAINTYPE terrainType = map(x,y);

    if(map(x,y) == Terrain_Sand) {
        if(std::find(spiceFields.begin(), spiceFields.end(), Coord(x,y)) != spiceFields.end()) {
            terrainType = Terrain_ThickSpice;
        } else if(std::find_if(spiceFields.begin(), spiceFields.end(),
            [center = Coord(x, y)](const auto& coord) { return distanceFrom(center, coord) < 5; }) != spiceFields.end()) {
            terrainType = Terrain_Spice;
        }
    }

    // check for classic map items (spice blooms, special blooms)
    if(std::find(spiceBlooms.begin(), spiceBlooms.end(), Coord(x,y)) != spiceBlooms.end()) {
        terrainType = Terrain_SpiceBloom;
    }

    if(std::find(specialBlooms.begin(), specialBlooms.end(), Coord(x,y)) != specialBlooms.end()) {
        terrainType = Terrain_SpecialBloom;
    }

    return terrainType;
}

void MapEditor::drawMap(ScreenBorder* pScreenborder, bool bCompleteMap) const {
    int zoomedTilesize = world2zoomedWorld(TILESIZE);

    Coord TopLeftTile = pScreenborder->getTopLeftTile();
    Coord BottomRightTile = pScreenborder->getBottomRightTile();

    // extend the view a little bit to avoid graphical glitches
    TopLeftTile.x = std::max(0, TopLeftTile.x - 1);
    TopLeftTile.y = std::max(0, TopLeftTile.y - 1);
    BottomRightTile.x = std::min(map.getSizeX()-1, BottomRightTile.x + 1);
    BottomRightTile.y = std::min(map.getSizeY()-1, BottomRightTile.y + 1);

    // Load Terrain Surface
    const auto* const terrainSprite = pGFXManager->getZoomedObjPic(ObjPic_Terrain, currentZoomlevel);

    /* draw ground */
    for(int y = TopLeftTile.y; y <= BottomRightTile.y; y++) {
        for(int x = TopLeftTile.x; x <= BottomRightTile.x; x++) {

            int tile = 0;

            switch(getTerrain(x,y)) {
                case Terrain_Slab: {
                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Slab);
                } break;

                case Terrain_Sand: {
                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Sand);
                } break;

                case Terrain_Rock: {
                    //determine which surrounding tiles are rock
                    const int up = (y-1 < 0) || (getTerrain(x, y-1) == Terrain_Rock) || (getTerrain(x, y-1) == Terrain_Slab) || (getTerrain(x, y-1) == Terrain_Mountain);
                    const int right = (x+1 >= map.getSizeX()) || (getTerrain(x+1, y) == Terrain_Rock) || (getTerrain(x+1, y) == Terrain_Slab) || (getTerrain(x+1, y) == Terrain_Mountain);
                    const int down = (y+1 >= map.getSizeY()) || (getTerrain(x, y+1) == Terrain_Rock) || (getTerrain(x, y+1) == Terrain_Slab) || (getTerrain(x, y+1) == Terrain_Mountain);
                    const int left = (x-1 < 0) || (getTerrain(x-1, y) == Terrain_Rock) || (getTerrain(x-1, y) == Terrain_Slab) || (getTerrain(x-1, y) == Terrain_Mountain);

                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Rock) + (up | (right << 1) | (down << 2) | (left << 3));
                } break;

                case Terrain_Dunes: {
                    //determine which surrounding tiles are dunes
                    const int up = (y-1 < 0) || (getTerrain(x, y-1) == Terrain_Dunes);
                    const int right = (x+1 >= map.getSizeX()) || (getTerrain(x+1, y) == Terrain_Dunes);
                    const int down = (y+1 >= map.getSizeY()) || (getTerrain(x, y+1) == Terrain_Dunes);
                    const int left = (x-1 < 0) || (getTerrain(x-1, y) == Terrain_Dunes);

                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Dunes) + (up | (right << 1) | (down << 2) | (left << 3));
                } break;

                case Terrain_Mountain: {
                    //determine which surrounding tiles are mountains
                    const int up = (y-1 < 0) || (getTerrain(x, y-1) == Terrain_Mountain);
                    const int right = (x+1 >= map.getSizeX()) || (getTerrain(x+1, y) == Terrain_Mountain);
                    const int down = (y+1 >= map.getSizeY()) || (getTerrain(x, y+1) == Terrain_Mountain);
                    const int left = (x-1 < 0) || (getTerrain(x-1, y) == Terrain_Mountain);

                    tile =
                        static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Mountain) + (up | (right << 1) | (down << 2) | (left << 3));
                } break;

                case Terrain_Spice: {
                    //determine which surrounding tiles are spice
                    const int up = (y-1 < 0) || (getTerrain(x, y-1) == Terrain_Spice) || (getTerrain(x, y-1) == Terrain_ThickSpice);
                    const int right = (x+1 >= map.getSizeX()) || (getTerrain(x+1, y) == Terrain_Spice) || (getTerrain(x+1, y) == Terrain_ThickSpice);
                    const int down = (y+1 >= map.getSizeY()) || (getTerrain(x, y+1) == Terrain_Spice) || (getTerrain(x, y+1) == Terrain_ThickSpice);
                    const int left = (x-1 < 0) || (getTerrain(x-1, y) == Terrain_Spice) || (getTerrain(x-1, y) == Terrain_ThickSpice);

                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Spice) + (up | (right << 1) | (down << 2) | (left << 3));
                } break;

                case Terrain_ThickSpice: {
                    //determine which surrounding tiles are thick spice
                    const int up = (y-1 < 0) || (getTerrain(x, y-1) == Terrain_ThickSpice);
                    const int right = (x+1 >= map.getSizeX()) || (getTerrain(x+1, y) == Terrain_ThickSpice);
                    const int down = (y+1 >= map.getSizeY()) || (getTerrain(x, y+1) == Terrain_ThickSpice);
                    const int left = (x-1 < 0) || (getTerrain(x-1, y) == Terrain_ThickSpice);

                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_ThickSpice) +
                           (up | (right << 1) | (down << 2) | (left << 3));
                } break;

                case Terrain_SpiceBloom: {
                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_SpiceBloom);
                } break;

                case Terrain_SpecialBloom: {
                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_SpecialBloom);
                } break;

                default: {
                    THROW(std::runtime_error, "MapEditor::DrawMap(): Invalid terrain type");
                } break;
            }

            //draw map[x][y]
            SDL_Rect source = { (tile % NUM_TERRAIN_TILES_X)*zoomedTilesize, (tile / NUM_TERRAIN_TILES_X)*zoomedTilesize,
                                zoomedTilesize, zoomedTilesize };
            SDL_FRect drawLocation = {   pScreenborder->world2screenX(x*TILESIZE), pScreenborder->world2screenY(y*TILESIZE),
                                        zoomedTilesize, zoomedTilesize };
            Dune_RenderCopyF(renderer, terrainSprite, &source, &drawLocation);
        }
    }


    std::vector<int> selectedStructures = getMirrorStructures(selectedStructureID);

    for(const Structure& structure : structures) {

        Coord position = structure.position;

        SDL_Rect selectionDest;
        if(structure.itemID == Structure_Slab1) {
            // Load Terrain sprite
            const auto* const terrainSprite = pGFXManager->getZoomedObjPic(ObjPic_Terrain, currentZoomlevel);

            SDL_Rect source = {static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Slab) * zoomedTilesize, 0, zoomedTilesize,
                               zoomedTilesize};
            SDL_FRect dest = { pScreenborder->world2screenX(position.x*TILESIZE), pScreenborder->world2screenY(position.y*TILESIZE), zoomedTilesize, zoomedTilesize };

            Dune_RenderCopyF(renderer, terrainSprite, &source, &dest);

            selectionDest = SDL_Rect{static_cast<int>(dest.x), static_cast<int>(dest.y), static_cast<int>(dest.w), static_cast<int>(dest.h)};
        } else if(structure.itemID == Structure_Slab4) {
            // Load Terrain Surface
            const auto* const terrainSprite = pGFXManager->getZoomedObjPic(ObjPic_Terrain, currentZoomlevel);

            for(int y = position.y; y < position.y+2; y++) {
                for(int x = position.x; x < position.x+2; x++) {
                    SDL_Rect source = {static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Slab) * zoomedTilesize, 0, zoomedTilesize,
                                       zoomedTilesize};
                    SDL_FRect dest = { pScreenborder->world2screenX(x*TILESIZE), pScreenborder->world2screenY(y*TILESIZE), zoomedTilesize, zoomedTilesize };

                    Dune_RenderCopyF(renderer, terrainSprite, &source, &dest);
                }
            }

            selectionDest.x = pScreenborder->world2screenX(position.x*TILESIZE);
            selectionDest.y = pScreenborder->world2screenY(position.y*TILESIZE);
            selectionDest.w = world2zoomedWorld(2*TILESIZE);
            selectionDest.h = world2zoomedWorld(2*TILESIZE);
        } else if(structure.itemID == Structure_Wall) {
            bool left = false;
            bool down = false;
            bool right = false;
            bool up = false;
            for(const Structure& structure1 : structures) {
                if(structure1.itemID == Structure_Wall) {
                    if((structure1.position.x == position.x - 1) && (structure1.position.y == position.y))  left = true;
                    if((structure1.position.x == position.x) && (structure1.position.y == position.y + 1))  down = true;
                    if((structure1.position.x == position.x + 1) && (structure1.position.y == position.y))  right = true;
                    if((structure1.position.x == position.x) && (structure1.position.y == position.y - 1))  up = true;
                }
            }

            auto maketile = 0;
            if((left) && (right) && (up) && (down)) {
                maketile = Wall::Wall_Full; //solid wall
            } else if((!left) && (right) && (up) && (down)) {
                maketile = Wall::Wall_UpDownRight; //missing left edge
            } else if((left) && (!right)&& (up) && (down)) {
                maketile = Wall::Wall_UpDownLeft; //missing right edge
            } else if((left) && (right) && (!up) && (down)) {
                maketile = Wall::Wall_DownLeftRight; //missing top edge
            } else if((left) && (right) && (up) && (!down)) {
                maketile = Wall::Wall_UpLeftRight; //missing bottom edge
            } else if((!left) && (right) && (!up) && (down)) {
                maketile = Wall::Wall_DownRight; //missing top left edge
            } else if((left) && (!right) && (up) && (!down)) {
                maketile = Wall::Wall_UpLeft; //missing bottom right edge
            } else if((left) && (!right) && (!up) && (down)) {
                maketile = Wall::Wall_DownLeft; //missing top right edge
            } else if((!left) && (right) && (up) && (!down)) {
                maketile = Wall::Wall_UpRight; //missing bottom left edge
            } else if((left) && (!right) && (!up) && (!down)) {
                maketile = Wall::Wall_LeftRight; //missing above, right and below
            } else if((!left) && (right) && (!up) && (!down)) {
                maketile = Wall::Wall_LeftRight; //missing above, left and below
            } else if((!left) && (!right) && (up) && (!down)) {
                maketile = Wall::Wall_UpDown; //only up
            } else if((!left) && (!right) && (!up) && (down)) {
                maketile = Wall::Wall_UpDown; //only down
            } else if((left) && (right) && (!up) && (!down)) {
                maketile = Wall::Wall_LeftRight; //missing above and below
            } else if((!left) && (!right) && (up) && (down)) {
                maketile = Wall::Wall_UpDown; //missing left and right
            } else if((!left) && (!right) && (!up) && (!down)) {
                maketile = Wall::Wall_Standalone; //missing left and right
            }

            // Load Wall texture
            const auto* const WallSprite = pGFXManager->getZoomedObjPic(ObjPic_Wall, currentZoomlevel);

            SDL_Rect source = { maketile * zoomedTilesize, 0, zoomedTilesize, zoomedTilesize };
            SDL_FRect dest = { pScreenborder->world2screenX(position.x*TILESIZE), pScreenborder->world2screenY(position.y*TILESIZE), zoomedTilesize, zoomedTilesize };

            Dune_RenderCopyF(renderer, WallSprite, &source, &dest);

            selectionDest = SDL_Rect{static_cast<int>(dest.x), static_cast<int>(dest.y), static_cast<int>(dest.w),
                                     static_cast<int>(dest.h)};
        } else {

            int objectPic = 0;
            // clang-format off
            switch(structure.itemID) {
                case Structure_Barracks:            objectPic = ObjPic_Barracks;            break;
                case Structure_ConstructionYard:    objectPic = ObjPic_ConstructionYard;    break;
                case Structure_GunTurret:           objectPic = ObjPic_GunTurret;           break;
                case Structure_HeavyFactory:        objectPic = ObjPic_HeavyFactory;        break;
                case Structure_HighTechFactory:     objectPic = ObjPic_HighTechFactory;     break;
                case Structure_IX:                  objectPic = ObjPic_IX;                  break;
                case Structure_LightFactory:        objectPic = ObjPic_LightFactory;        break;
                case Structure_Palace:              objectPic = ObjPic_Palace;              break;
                case Structure_Radar:               objectPic = ObjPic_Radar;               break;
                case Structure_Refinery:            objectPic = ObjPic_Refinery;            break;
                case Structure_RepairYard:          objectPic = ObjPic_RepairYard;          break;
                case Structure_RocketTurret:        objectPic = ObjPic_RocketTurret;        break;
                case Structure_Silo:                objectPic = ObjPic_Silo;                break;
                case Structure_StarPort:            objectPic = ObjPic_Starport;            break;
                case Structure_Wall:                objectPic = ObjPic_Wall;                break;
                case Structure_WindTrap:            objectPic = ObjPic_Windtrap;            break;
                case Structure_WOR:                 objectPic = ObjPic_WOR;                 break;
                default:                            objectPic = 0;                          break;
            }
            // clang-format on

            const auto* ObjectSprite = pGFXManager->getZoomedObjPic(objectPic, structure.house, currentZoomlevel);

            Coord frameSize = world2zoomedWorld(getStructureSize(structure.itemID)*TILESIZE);

            SDL_Rect source = { frameSize.x*(structure.itemID == Structure_WindTrap ? 9 : 2), 0, frameSize.x, frameSize.y };
            SDL_FRect dest = { pScreenborder->world2screenX(position.x*TILESIZE), pScreenborder->world2screenY(position.y*TILESIZE), frameSize.x, frameSize.y };

            Dune_RenderCopyF(renderer, ObjectSprite, &source, &dest);

            selectionDest = SDL_Rect{static_cast<int>(dest.x), static_cast<int>(dest.y), static_cast<int>(dest.w),
                                     static_cast<int>(dest.h)};
        }

        // draw selection frame
        if(!bCompleteMap && (std::find(selectedStructures.begin(), selectedStructures.end(), structure.id) != selectedStructures.end()) ) {
            //now draw the selection box thing, with parts at all corners of structure

            DuneDrawSelectionBox(renderer, selectionDest);
        }

    }

    for(const Unit& unit : units) {

        const Coord& position = unit.position;

        constexpr Coord tankTurretOffset[] =    {   Coord(0, 0),
                                                Coord(0, 0),
                                                Coord(0, 0),
                                                Coord(0, 0),
                                                Coord(0, 0),
                                                Coord(0, 0),
                                                Coord(0, 0),
                                                Coord(0, 0)
                                            };

        constexpr Coord siegeTankTurretOffset[] =   {   Coord(8, -12),
                                                    Coord(0, -20),
                                                    Coord(0, -20),
                                                    Coord(-4, -20),
                                                    Coord(-8, -12),
                                                    Coord(-8, -4),
                                                    Coord(-4, -12),
                                                    Coord(8, -4)
                                            };

        constexpr Coord sonicTankTurretOffset[] =   {   Coord(0, -8),
                                                    Coord(0, -8),
                                                    Coord(0, -8),
                                                    Coord(0, -8),
                                                    Coord(0, -8),
                                                    Coord(0, -8),
                                                    Coord(0, -8),
                                                    Coord(0, -8)
                                                };

        constexpr Coord launcherTurretOffset[] =    {   Coord(0, -12),
                                                    Coord(0, -8),
                                                    Coord(0, -8),
                                                    Coord(0, -8),
                                                    Coord(0, -12),
                                                    Coord(0, -8),
                                                    Coord(0, -8),
                                                    Coord(0, -8)
                                                };

        constexpr Coord devastatorTurretOffset[] =  {
                                                    Coord(8, -16),
                                                    Coord(-4, -12),
                                                    Coord(0, -16),
                                                    Coord(4, -12),
                                                    Coord(-8, -16),
                                                    Coord(0, -12),
                                                    Coord(-4, -12),
                                                    Coord(0, -12)
                                                };



        int objectPicBase = 0;
        int framesX = static_cast<int>(ANGLETYPE::NUM_ANGLES);
        int framesY = 1;
        int objectPicGun = -1;
        const Coord* gunOffset = nullptr;

        // clang-format off
        switch(unit.itemID) {
            case Unit_Carryall:         objectPicBase = ObjPic_Carryall;        framesY = 2;                                                                    break;
            case Unit_Devastator:       objectPicBase = ObjPic_Devastator_Base; objectPicGun = ObjPic_Devastator_Gun;   gunOffset = devastatorTurretOffset;     break;
            case Unit_Deviator:         objectPicBase = ObjPic_Tank_Base;       objectPicGun = ObjPic_Launcher_Gun;     gunOffset = launcherTurretOffset;       break;
            case Unit_Frigate:          objectPicBase = ObjPic_Frigate;                                                                                         break;
            case Unit_Harvester:        objectPicBase = ObjPic_Harvester;                                                                                       break;
            case Unit_Soldier:          objectPicBase = ObjPic_Soldier;         framesX = 4;    framesY = 3;                                                    break;
            case Unit_Launcher:         objectPicBase = ObjPic_Tank_Base;       objectPicGun = ObjPic_Launcher_Gun;     gunOffset = launcherTurretOffset;       break;
            case Unit_MCV:              objectPicBase = ObjPic_MCV;                                                                                             break;
            case Unit_Ornithopter:      objectPicBase = ObjPic_Ornithopter;     framesY = 3;                                                                    break;
            case Unit_Quad:             objectPicBase = ObjPic_Quad;                                                                                            break;
            case Unit_Saboteur:         objectPicBase = ObjPic_Saboteur;        framesX = 4;    framesY = 3;                                                    break;
            case Unit_Sandworm:         objectPicBase = ObjPic_Sandworm;        framesX = 1;    framesY = 9;                                                    break;
            case Unit_SiegeTank:        objectPicBase = ObjPic_Siegetank_Base;  objectPicGun = ObjPic_Siegetank_Gun;    gunOffset = siegeTankTurretOffset;      break;
            case Unit_SonicTank:        objectPicBase = ObjPic_Tank_Base;       objectPicGun = ObjPic_Sonictank_Gun;    gunOffset = sonicTankTurretOffset;      break;
            case Unit_Tank:             objectPicBase = ObjPic_Tank_Base;       objectPicGun = ObjPic_Tank_Gun;         gunOffset = tankTurretOffset;           break;
            case Unit_Trike:            objectPicBase = ObjPic_Trike;                                                                                           break;
            case Unit_RaiderTrike:      objectPicBase = ObjPic_Trike;                                                                                           break;
            case Unit_Trooper:          objectPicBase = ObjPic_Trooper;         framesX = 4;    framesY = 3;                                                    break;
            case Unit_Special:          objectPicBase = ObjPic_Devastator_Base; objectPicGun = ObjPic_Devastator_Gun;   gunOffset = devastatorTurretOffset;     break;
            case Unit_Infantry:         objectPicBase = ObjPic_Infantry;         framesX = 4;    framesY = 4;                                                   break;
            case Unit_Troopers:         objectPicBase = ObjPic_Troopers;         framesX = 4;    framesY = 4;                                                   break;
        }
        // clang-format on

        const auto* const pObjectSprite = pGFXManager->getZoomedObjPic(objectPicBase, unit.house, currentZoomlevel);

        int angle = static_cast<int>(unit.angle) / (static_cast<int>(ANGLETYPE::NUM_ANGLES)/framesX);

        int frame = (unit.itemID == Unit_Sandworm) ? 5 : 0;

        auto source = calcSpriteSourceRect(pObjectSprite, angle, framesX, frame, framesY);
        int frameSizeX = source.w;
        int frameSizeY = source.h;
        const auto drawLocation = calcSpriteDrawingRectF(  pObjectSprite,
                                                        pScreenborder->world2screenX((position.x*TILESIZE)+(TILESIZE/2)),
                                                        pScreenborder->world2screenY((position.y*TILESIZE)+(TILESIZE/2)),
                                                        framesX, framesY, HAlign::Center, VAlign::Center);

        Dune_RenderCopyF(renderer, pObjectSprite, &source, &drawLocation);

        if(objectPicGun >= 0) {
            const auto* const pGunSprite = pGFXManager->getZoomedObjPic(objectPicGun, unit.house, currentZoomlevel);

            auto source2 = calcSpriteSourceRect(pGunSprite, static_cast<int>(unit.angle), static_cast<int>(ANGLETYPE::NUM_ANGLES));

            const auto& gun = gunOffset[static_cast<int>(unit.angle)];
            const auto  sx  = pScreenborder->world2screenX((position.x * TILESIZE) + (TILESIZE / 2) + gun.x);
            const auto  sy  = pScreenborder->world2screenY((position.y * TILESIZE) + (TILESIZE / 2) + gun.y);

            const auto drawLocation2 = calcSpriteDrawingRectF(pGunSprite, sx, sy, static_cast<int>(ANGLETYPE::NUM_ANGLES), 1,
                                                           HAlign::Center, VAlign::Center);

            Dune_RenderCopyF(renderer, pGunSprite, &source2, &drawLocation2);
        }

        if(unit.itemID == Unit_RaiderTrike || unit.itemID == Unit_Deviator || unit.itemID == Unit_Special) {
            const auto* const pStarSprite = pGFXManager->getZoomedObjPic(ObjPic_Star, currentZoomlevel);

            auto drawLocation2 = calcDrawingRect(   pStarSprite,
                                                        pScreenborder->world2screenX((position.x*TILESIZE)+(TILESIZE/2)) + frameSizeX/2 - 1,
                                                        pScreenborder->world2screenY((position.y*TILESIZE)+(TILESIZE/2)) + frameSizeY/2 - 1,
                                                        HAlign::Right, VAlign::Bottom);

            pStarSprite->draw(renderer, drawLocation2.x, drawLocation2.y);
        }
    }

    // draw tactical pos rectangle (the starting screen)
    if(!bCompleteMap && getMapVersion() < 2 && mapInfo.tacticalPos.isValid()) {

        SDL_FRect dest;
        dest.x = pScreenborder->world2screenX( mapInfo.tacticalPos.x*TILESIZE);
        dest.y = pScreenborder->world2screenY( mapInfo.tacticalPos.y*TILESIZE);
        dest.w = world2zoomedWorld(15*TILESIZE);
        dest.h = world2zoomedWorld(10*TILESIZE);

        renderDrawRectF(renderer, &dest, COLOR_DARKGREY);
    }

    const DuneTexture* validPlace = nullptr;
    const DuneTexture* invalidPlace = nullptr;
    const DuneTexture* greyPlace    = nullptr;

    switch(currentZoomlevel) {
        case 0: {
            validPlace = pGFXManager->getUIGraphic(UI_ValidPlace_Zoomlevel0);
            invalidPlace = pGFXManager->getUIGraphic(UI_InvalidPlace_Zoomlevel0);
            greyPlace = pGFXManager->getUIGraphic(UI_GreyPlace_Zoomlevel0);
        } break;

        case 1: {
            validPlace = pGFXManager->getUIGraphic(UI_ValidPlace_Zoomlevel1);
            invalidPlace = pGFXManager->getUIGraphic(UI_InvalidPlace_Zoomlevel1);
            greyPlace = pGFXManager->getUIGraphic(UI_GreyPlace_Zoomlevel1);
        } break;

        case 2:
        default: {
            validPlace = pGFXManager->getUIGraphic(UI_ValidPlace_Zoomlevel2);
            invalidPlace = pGFXManager->getUIGraphic(UI_InvalidPlace_Zoomlevel2);
            greyPlace = pGFXManager->getUIGraphic(UI_GreyPlace_Zoomlevel2);
        } break;
    }

    if(!bCompleteMap && !pInterface->hasChildWindow() && pScreenborder->isScreenCoordInsideMap(drawnMouseX, drawnMouseY)) {

        int xPos = pScreenborder->screen2MapX(drawnMouseX);
        int yPos = pScreenborder->screen2MapY(drawnMouseY);

        if(currentEditorMode.mode == EditorMode::EditorMode_Terrain) {

            int halfsize = currentEditorMode.pensize/2;

            for(int m=0;m<mapMirror->getSize();m++) {

                Coord position = mapMirror->getCoord( Coord(xPos, yPos), m);

                SDL_Rect dest;
                dest.x = pScreenborder->world2screenX( (position.x-halfsize)*TILESIZE);
                dest.y = pScreenborder->world2screenY( (position.y-halfsize)*TILESIZE);
                dest.w = world2zoomedWorld(currentEditorMode.pensize*TILESIZE);
                dest.h = world2zoomedWorld(currentEditorMode.pensize*TILESIZE);

                DuneDrawSelectionBox(renderer, dest);
            }

        } else if(currentEditorMode.mode == EditorMode::EditorMode_Structure) {
            Coord structureSize = getStructureSize(currentEditorMode.itemID);

            for(int m=0;m<mapMirror->getSize();m++) {

                Coord position = mapMirror->getCoord( Coord(xPos, yPos), m, structureSize);

                for(int x = position.x; x < (position.x + structureSize.x); x++) {
                    for(int y = position.y; y < (position.y + structureSize.y); y++) {
                        const auto* image = validPlace;

                        // check if mirroring of the original (!) position is possible
                        if(!mapMirror->mirroringPossible( Coord(xPos, yPos), structureSize)) {
                            image = invalidPlace;
                        }

                        // check all mirrored places
                        for(int k=0;k<mapMirror->getSize();k++) {
                            Coord pos = mapMirror->getCoord( Coord(x, y), k);

                            if(!map.isInsideMap(pos.x,pos.y) || isTileBlocked(pos.x, pos.y, true, (currentEditorMode.itemID != Structure_Slab1) )) {
                                image = invalidPlace;
                            } else if((image != invalidPlace) && (map(pos.x,pos.y) != Terrain_Rock)) {
                                image = greyPlace;
                            }
                        }

                        SDL_Rect drawLocation = {   pScreenborder->world2screenX(x*TILESIZE), pScreenborder->world2screenY(y*TILESIZE),
                                                    zoomedTilesize, zoomedTilesize };
                        Dune_RenderCopy(renderer, image, nullptr, &drawLocation);
                    }
                }
            }
        } else if(currentEditorMode.mode == EditorMode::EditorMode_Unit) {
            for(int m=0;m<mapMirror->getSize();m++) {

                Coord position = mapMirror->getCoord( Coord(xPos, yPos), m);

                const auto* image = validPlace;
                // check all mirrored places
                for(int k=0;k<mapMirror->getSize();k++) {
                    Coord pos = mapMirror->getCoord( position, k);

                    if(!map.isInsideMap(pos.x,pos.y) || isTileBlocked(pos.x, pos.y, false, true)) {
                        image = invalidPlace;
                    }
                }
                auto drawLocation = calcDrawingRect(image, pScreenborder->world2screenX(position.x*TILESIZE), pScreenborder->world2screenY(position.y*TILESIZE));
                Dune_RenderCopy(renderer, image, nullptr, &drawLocation);
            }
        } else if(currentEditorMode.mode == EditorMode::EditorMode_TacticalPos) {
            // draw tactical pos rectangle (the starting screen)
            if(mapInfo.tacticalPos.isValid()) {

                SDL_Rect dest = {   pScreenborder->world2screenX(xPos*TILESIZE), pScreenborder->world2screenY(yPos*TILESIZE),
                                    world2zoomedWorld(15*TILESIZE), world2zoomedWorld(10*TILESIZE) };
                renderDrawRect(renderer, &dest, COLOR_WHITE);
            }
        }
    }

    // draw selection rect for units (selection rect for structures is already drawn)
    if(!bCompleteMap) {
        std::vector<int> selectedUnits = getMirrorUnits(selectedUnitID);

        for(const Unit& unit : units) {
            if(std::find(selectedUnits.begin(), selectedUnits.end(), unit.id) != selectedUnits.end()) {
                const Coord& position = unit.position;

                const DuneTexture* selectionBox = nullptr;

                switch(currentZoomlevel) {
                    case 0:     selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel0);   break;
                    case 1:     selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel1);   break;
                    case 2:
                    default:    selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel2);   break;
                }

                SDL_Rect dest = calcDrawingRect(selectionBox,
                                                pScreenborder->world2screenX((position.x*TILESIZE)+(TILESIZE/2)),
                                                pScreenborder->world2screenY((position.y*TILESIZE)+(TILESIZE/2)),
                                                HAlign::Center, VAlign::Center);

                Dune_RenderCopy(renderer, selectionBox, nullptr, &dest);
            }
        }
    }

    // draw selection rect for map items (spice bloom, special bloom or spice field)
    if(!bCompleteMap && selectedMapItemCoord.isValid()
        && ( (std::find(spiceBlooms.begin(), spiceBlooms.end(), selectedMapItemCoord) != spiceBlooms.end())
            || (std::find(specialBlooms.begin(), specialBlooms.end(), selectedMapItemCoord) != specialBlooms.end())
            || (std::find(spiceFields.begin(), spiceFields.end(), selectedMapItemCoord) != spiceFields.end()) )) {

        const DuneTexture* selectionBox = nullptr;

        switch(currentZoomlevel) {
            case 0:     selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel0);   break;
            case 1:     selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel1);   break;
            case 2:
            default:    selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel2);   break;
        }

        SDL_Rect dest = calcDrawingRect(selectionBox,
                                        pScreenborder->world2screenX((selectedMapItemCoord.x*TILESIZE)+(TILESIZE/2)),
                                        pScreenborder->world2screenY((selectedMapItemCoord.y*TILESIZE)+(TILESIZE/2)),
                                        HAlign::Center, VAlign::Center);

        Dune_RenderCopy(renderer, selectionBox, nullptr, &dest);
    }
}

void MapEditor::saveMapshot() {
    const int oldCurrentZoomlevel = currentZoomlevel;
    currentZoomlevel = 0;

    auto mapshotFilename =
        (lastSaveName.empty() ? std::filesystem::path{generateMapname()} : getBasename(lastSaveName, true));

    mapshotFilename += ".png";

    const auto sizeX = world2zoomedWorld(map.getSizeX()*TILESIZE);
    const auto sizeY = world2zoomedWorld(map.getSizeY()*TILESIZE);

    const SDL_Rect board = { 0, 0, sizeX, sizeY };

    ScreenBorder tmpScreenborder(board);
    tmpScreenborder.adjustScreenBorderToMapsize(map.getSizeX(), map.getSizeY());

    auto renderTarget = sdl2::texture_ptr{ SDL_CreateTexture(renderer, SCREEN_FORMAT, SDL_TEXTUREACCESS_TARGET, sizeX, sizeY) };
    if(renderTarget == nullptr) {
        sdl2::log_info("SDL_CreateTexture() failed: %s", SDL_GetError());
        currentZoomlevel = oldCurrentZoomlevel;
        return;
    }

    SDL_Texture* oldRenderTarget = SDL_GetRenderTarget(renderer);
    if(SDL_SetRenderTarget(renderer, renderTarget.get()) != 0) {
        sdl2::log_info("SDL_SetRenderTarget() failed: %s", SDL_GetError());
        SDL_SetRenderTarget(renderer, oldRenderTarget);
        currentZoomlevel = oldCurrentZoomlevel;
        return;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    drawMap(&tmpScreenborder, true);

    sdl2::surface_ptr pMapshotSurface = renderReadSurface(renderer);
    SavePNG(pMapshotSurface.get(), mapshotFilename.u8string().c_str());

    SDL_SetRenderTarget(renderer, oldRenderTarget);

    currentZoomlevel = oldCurrentZoomlevel;
}
