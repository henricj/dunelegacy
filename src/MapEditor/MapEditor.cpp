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

#include <GUI/Window.h>
#include <Menu/MenuBase.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/INIFile.h>
#include <FileClasses/LoadSavePNG.h>
#include <FileClasses/TextManager.h>

#include <structures/Wall.h>

#include "misc/DrawingRectHelper.h"
#include <misc/FileSystem.h>
#include <misc/draw_util.h>

#include "ScreenBorder.h"
#include <Tile.h>
#include <globals.h>
#include <main.h>
#include <mmath.h>
#include <sand.h>

#include <config.h>

#include <SDL2/SDL_render.h>

#include <fmt/printf.h>

#include <algorithm>
#include <typeinfo>

MapEditor::MapEditor() {
    selectedMapItemCoord_.invalidate();

    dune::globals::currentZoomlevel = dune::globals::settings.video.preferredZoomLevel;

    const auto* const gfx = dune::globals::pGFXManager.get();

    sideBarPos_ = as_rect(calcAlignedDrawingRect(gfx->getUIGraphic(UI_SideBar), HAlign::Right, VAlign::Top));
    topBarPos_  = as_rect(calcAlignedDrawingRect(gfx->getUIGraphic(UI_TopBar), HAlign::Left, VAlign::Top));
    bottomBarPos_ =
        as_rect(calcAlignedDrawingRect(gfx->getUIGraphic(UI_MapEditor_BottomBar), HAlign::Left, VAlign::Bottom));

    SDL_FRect gameBoardRect{0, static_cast<float>(topBarPos_.h), static_cast<float>(sideBarPos_.x),
                            static_cast<float>(getRendererHeight() - topBarPos_.h - bottomBarPos_.h)};

    dune::globals::screenborder = std::make_unique<ScreenBorder>(gameBoardRect);

    setMap(MapData(128, 128, Terrain_Sand), MapInfo());
    setMirrorMode(MirrorModeNone);

    pInterface_ = std::make_unique<MapEditorInterface>(this);

    pInterface_->onNew();
}

MapEditor::~MapEditor() {
    dune::globals::screenborder.reset();
}

std::string MapEditor::generateMapname() {
    const auto numPlayers =
        std::ranges::count_if(players_, [](const MapEditor::Player& player) { return player.bActive_; });

    return std::to_string(numPlayers) + "P - " + std::to_string(map_.getSizeX()) + "x" + std::to_string(map_.getSizeY())
         + " - " + _("New Map");
}

void MapEditor::setMirrorMode(MirrorMode newMirrorMode) {
    currentMirrorMode_ = newMirrorMode;

    mapMirror_ = MapMirror::createMapMirror(currentMirrorMode_, map_.getSizeX(), map_.getSizeY());
}

void MapEditor::RunEditor(MenuBase::event_handler_type handler) {
    while (!bQuitEditor_) {
        const auto frameStart = dune::dune_clock::now();

        processInput(handler);
        drawScreen();

        if (dune::globals::settings.video.frameLimit) {
            const auto frameDuration = dune::dune_clock::now() - frameStart;
            const auto frameTime     = dune::as_milliseconds(frameDuration);

            static_assert(std::is_unsigned_v<decltype(frameTime)>);

            if (frameTime < 32) {
                SDL_Delay(32 - frameTime);
            }
        }
    }
}

void MapEditor::setMap(const MapData& mapdata, const MapInfo& newMapInfo) {
    map_     = mapdata;
    mapInfo_ = newMapInfo;

    dune::globals::screenborder->adjustScreenBorderToMapsize(map_.getSizeX(), map_.getSizeY());

    // reset tools
    selectedUnitID_      = INVALID;
    selectedStructureID_ = INVALID;
    selectedMapItemCoord_.invalidate();

    if (pInterface_ != nullptr) {
        pInterface_->deselectAll();
    }

    while (!redoOperationStack_.empty()) {
        redoOperationStack_.pop();
    }

    while (!undoOperationStack_.empty()) {
        undoOperationStack_.pop();
    }

    // reset other map properties
    loadedINIFile_.reset();
    lastSaveName_ = "";

    spiceBlooms_.clear();
    specialBlooms_.clear();
    spiceFields_.clear();
    choam_.clear();
    reinforcements_.clear();
    ai_teams_.clear();
    structures_.clear();
    units_.clear();
    players_.clear();

    // setup default players_
    if (getMapVersion() < 2) {
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_HARKONNEN), HOUSETYPE::HOUSE_HARKONNEN,
                              HOUSETYPE::HOUSE_HARKONNEN, true, false, "Human", 25);
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ATREIDES), HOUSETYPE::HOUSE_ATREIDES,
                              HOUSETYPE::HOUSE_ATREIDES, true, false, "CPU", 25);
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ORDOS), HOUSETYPE::HOUSE_ORDOS,
                              HOUSETYPE::HOUSE_ORDOS, true, false, "CPU", 25);
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_FREMEN), HOUSETYPE::HOUSE_FREMEN,
                              HOUSETYPE::HOUSE_FREMEN, false, false, "CPU", 25);
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_SARDAUKAR), HOUSETYPE::HOUSE_SARDAUKAR,
                              HOUSETYPE::HOUSE_SARDAUKAR, true, false, "CPU", 25);
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_MERCENARY), HOUSETYPE::HOUSE_MERCENARY,
                              HOUSETYPE::HOUSE_MERCENARY, false, false, "CPU", 25);
    } else {
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_HARKONNEN), HOUSETYPE::HOUSE_HARKONNEN,
                              HOUSETYPE::HOUSE_HARKONNEN, true, true, "Team1");
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ATREIDES), HOUSETYPE::HOUSE_ATREIDES,
                              HOUSETYPE::HOUSE_ATREIDES, true, true, "Team2");
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ORDOS), HOUSETYPE::HOUSE_ORDOS,
                              HOUSETYPE::HOUSE_ORDOS, true, true, "Team3");
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_FREMEN), HOUSETYPE::HOUSE_FREMEN,
                              HOUSETYPE::HOUSE_FREMEN, false, false, "Team4");
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_SARDAUKAR), HOUSETYPE::HOUSE_SARDAUKAR,
                              HOUSETYPE::HOUSE_SARDAUKAR, true, true, "Team5");
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_MERCENARY), HOUSETYPE::HOUSE_MERCENARY,
                              HOUSETYPE::HOUSE_MERCENARY, false, false, "Team6");
    }

    // setup default choam
    choam_[Unit_Carryall]    = 2;
    choam_[Unit_Harvester]   = 4;
    choam_[Unit_Launcher]    = 5;
    choam_[Unit_MCV]         = 2;
    choam_[Unit_Ornithopter] = 5;
    choam_[Unit_Quad]        = 5;
    choam_[Unit_SiegeTank]   = 6;
    choam_[Unit_Tank]        = 6;
    choam_[Unit_Trike]       = 5;

    if (pInterface_ != nullptr) {
        pInterface_->onNewMap();
        pInterface_->onHouseChanges();
    }

    currentEditorMode_ = EditorMode();

    bChangedSinceLastSave_ = true;
}

bool MapEditor::isTileBlocked(int x, int y, bool bSlabIsBlocking, bool bUnitsAreBlocking) const {
    for (const Structure& structure : structures_) {
        if (!bSlabIsBlocking && ((structure.itemID_ == Structure_Slab1) || (structure.itemID_ == Structure_Slab4))) {
            continue;
        }

        const Coord structureSize = getStructureSize(structure.itemID_);
        const Coord position      = structure.position_;
        if ((x >= position.x) && (x < position.x + structureSize.x) && (y >= position.y)
            && (y < position.y + structureSize.y)) {
            return true;
        }
    }

    if (bUnitsAreBlocking) {
        for (const Unit& unit : units_) {
            if ((x == unit.position_.x) && (y == unit.position_.y)) {
                return true;
            }
        }
    }

    return false;
}

MapEditor::Structure* MapEditor::getStructure(int structureID) {
    const auto it = std::ranges::find_if(structures_, [=](const auto& s) { return s.id_ == structureID; });

    return it == structures_.end() ? nullptr : &(*it);
}

const MapEditor::Structure* MapEditor::getStructure(int structureID) const {
    const auto it = std::ranges::find_if(structures_, [=](const auto& s) { return s.id_ == structureID; });

    return it == structures_.end() ? nullptr : &(*it);
}

std::vector<int> MapEditor::getMirrorStructures(int structureID) const {
    std::vector<int> mirrorStructures;

    const auto* pStructure = getStructure(structureID);

    if (pStructure == nullptr) {
        return mirrorStructures;
    }

    const Coord structureSize = getStructureSize(pStructure->itemID_);

    for (int i = 0; i < mapMirror_->getSize(); i++) {
        Coord position = mapMirror_->getCoord(pStructure->position_, i, structureSize);

        for (const Structure& structure : structures_) {
            if (structure.position_ == position) {
                mirrorStructures.push_back(structure.id_);
                break;
            }
        }
    }

    return mirrorStructures;
}

MapEditor::Unit* MapEditor::getUnit(int unitID) {
    const auto it = std::ranges::find_if(units_, [unitID](const auto& unit) { return unit.id_ == unitID; });

    if (it == units_.end())
        return nullptr;

    return &*it;
}

const MapEditor::Unit* MapEditor::getUnit(int unitID) const {
    const auto it = std::ranges::find_if(units_, [unitID](const auto& unit) { return unit.id_ == unitID; });

    if (it == units_.end())
        return nullptr;

    return &*it;
}

std::vector<int> MapEditor::getMirrorUnits(int unitID, bool bAddMissingAsInvalid) const {
    std::vector<int> mirrorUnits;

    const Unit* pUnit = getUnit(unitID);

    if (pUnit == nullptr) {
        return mirrorUnits;
    }

    for (int i = 0; i < mapMirror_->getSize(); i++) {
        Coord position = mapMirror_->getCoord(pUnit->position_, i);

        for (const Unit& unit : units_) {
            if (unit.position_ == position) {
                mirrorUnits.push_back(unit.id_);
                break;
            }
        }

        if (bAddMissingAsInvalid && (mirrorUnits.size() < static_cast<unsigned int>(i + 1))) {
            mirrorUnits.push_back(INVALID);
        }
    }

    return mirrorUnits;
}

void MapEditor::setEditorMode(const EditorMode& newEditorMode) {

    if (pInterface_ != nullptr) {
        pInterface_->deselectObject();
    }

    selectedUnitID_      = INVALID;
    selectedStructureID_ = INVALID;
    selectedMapItemCoord_.invalidate();

    currentEditorMode_ = newEditorMode;
}

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

void MapEditor::loadMap(const std::filesystem::path& filepath) {
    // reset tools
    selectedUnitID_      = INVALID;
    selectedStructureID_ = INVALID;

    if (pInterface_ != nullptr) {
        pInterface_->deselectAll();
    }

    while (!redoOperationStack_.empty()) {
        redoOperationStack_.pop();
    }

    while (!undoOperationStack_.empty()) {
        undoOperationStack_.pop();
    }

    // reset other map properties
    spiceBlooms_.clear();
    specialBlooms_.clear();
    spiceFields_.clear();
    choam_.clear();
    reinforcements_.clear();
    ai_teams_.clear();
    structures_.clear();
    units_.clear();
    players_.clear();

    players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_HARKONNEN), HOUSETYPE::HOUSE_HARKONNEN,
                          HOUSETYPE::HOUSE_HARKONNEN, false, true, "Team1");
    players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ATREIDES), HOUSETYPE::HOUSE_ATREIDES,
                          HOUSETYPE::HOUSE_ATREIDES, false, true, "Team2");
    players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ORDOS), HOUSETYPE::HOUSE_ORDOS, HOUSETYPE::HOUSE_ORDOS,
                          false, true, "Team3");
    players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_FREMEN), HOUSETYPE::HOUSE_FREMEN,
                          HOUSETYPE::HOUSE_FREMEN, false, false, "Team4");
    players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_SARDAUKAR), HOUSETYPE::HOUSE_SARDAUKAR,
                          HOUSETYPE::HOUSE_SARDAUKAR, false, false, "Team5");
    players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_MERCENARY), HOUSETYPE::HOUSE_MERCENARY,
                          HOUSETYPE::HOUSE_MERCENARY, false, false, "Team6");

    // load map
    loadedINIFile_ = std::make_unique<INIFile>(filepath, false);
    lastSaveName_  = filepath;

    // do the actual loading
    INIMapEditorLoader INIMapEditorLoader(this, loadedINIFile_.get());

    // update interface
    if (pInterface_ != nullptr) {
        pInterface_->onNewMap();
        pInterface_->onHouseChanges();
    }

    currentEditorMode_ = EditorMode();

    bChangedSinceLastSave_ = false;
}

void MapEditor::saveMap(const std::filesystem::path& filepath) {
    if (!loadedINIFile_) {
        std::string comment = "Created with Dune Legacy " + std::string(VERSION) + " Map Editor.";
        loadedINIFile_      = std::make_unique<INIFile>(false, comment);
    }

    int version = (mapInfo_.mapSeed == INVALID) ? 2 : 1;

    if (version > 1) {
        loadedINIFile_->setIntValue("BASIC", "Version", version);
    }

    if (!mapInfo_.license.empty()) {
        loadedINIFile_->setStringValue("BASIC", "License", mapInfo_.license);
    }

    if (!mapInfo_.author.empty()) {
        loadedINIFile_->setStringValue("BASIC", "Author", mapInfo_.author);
    }

    if ((version > 1) && (mapInfo_.techLevel > 0)) {
        loadedINIFile_->setIntValue("BASIC", "TechLevel", mapInfo_.techLevel);
    }

    loadedINIFile_->setIntValue("BASIC", "WinFlags", mapInfo_.winFlags);
    loadedINIFile_->setIntValue("BASIC", "LoseFlags", mapInfo_.loseFlags);

    loadedINIFile_->setStringValue("BASIC", "LosePicture", mapInfo_.losePicture, false);
    loadedINIFile_->setStringValue("BASIC", "WinPicture", mapInfo_.winPicture, false);
    loadedINIFile_->setStringValue("BASIC", "BriefPicture", mapInfo_.briefPicture, false);

    loadedINIFile_->setIntValue("BASIC", "TimeOut", mapInfo_.timeout);

    int logicalSizeX = 0;
    // int logicalSizeY;
    int logicalOffsetX = 0;
    int logicalOffsetY = 0;

    if (version < 2) {
        logicalSizeX = 64;
        // logicalSizeY = 64;

        int mapscale = 0;
        switch (map_.getSizeX()) {
            case 21: {
                mapscale       = 2;
                logicalOffsetX = logicalOffsetY = 11;
            } break;
            case 32: {
                mapscale       = 1;
                logicalOffsetX = logicalOffsetY = 16;
            } break;
            case 62:
            default: {
                mapscale       = 0;
                logicalOffsetX = logicalOffsetY = 1;
            } break;
        }

        loadedINIFile_->setIntValue("BASIC", "MapScale", mapscale);

        int cursorPos =
            (logicalOffsetY + mapInfo_.cursorPos.y) * logicalSizeX + (logicalOffsetX + mapInfo_.cursorPos.x);
        loadedINIFile_->setIntValue("BASIC", "CursorPos", cursorPos);
        int tacticalPos =
            (logicalOffsetY + mapInfo_.tacticalPos.y) * logicalSizeX + (logicalOffsetX + mapInfo_.tacticalPos.x);
        loadedINIFile_->setIntValue("BASIC", "TacticalPos", tacticalPos);

        // field, spice bloom and special bloom
        std::string strSpiceBloom;
        for (size_t i = 0; i < spiceBlooms_.size(); ++i) {
            if (i > 0) {
                strSpiceBloom += ",";
            }

            int position = (logicalOffsetY + spiceBlooms_[i].y) * logicalSizeX + (logicalOffsetX + spiceBlooms_[i].x);
            strSpiceBloom += std::to_string(position);
        }

        if (!strSpiceBloom.empty()) {
            loadedINIFile_->setStringValue("MAP", "Bloom", strSpiceBloom, false);
        } else {
            loadedINIFile_->removeKey("MAP", "Bloom");
        }

        std::string strSpecialBloom;
        for (size_t i = 0; i < specialBlooms_.size(); ++i) {
            if (i > 0) {
                strSpecialBloom += ",";
            }

            int position =
                (logicalOffsetY + specialBlooms_[i].y) * logicalSizeX + (logicalOffsetX + specialBlooms_[i].x);
            strSpecialBloom += std::to_string(position);
        }

        if (!strSpecialBloom.empty()) {
            loadedINIFile_->setStringValue("MAP", "Special", strSpecialBloom, false);
        } else {
            loadedINIFile_->removeKey("MAP", "Special");
        }

        std::string strFieldBloom;
        for (size_t i = 0; i < spiceFields_.size(); ++i) {
            if (i > 0) {
                strFieldBloom += ",";
            }

            int position = (logicalOffsetY + spiceFields_[i].y) * logicalSizeX + (logicalOffsetX + spiceFields_[i].x);
            strFieldBloom += std::to_string(position);
        }

        if (!strFieldBloom.empty()) {
            loadedINIFile_->setStringValue("MAP", "Field", strFieldBloom, false);
        } else {
            loadedINIFile_->removeKey("MAP", "Field");
        }

        loadedINIFile_->setIntValue("MAP", "Seed", mapInfo_.mapSeed);
    } else {
        logicalSizeX = map_.getSizeX();
        // logicalSizeY = map.getSizeY();
        logicalOffsetX = logicalOffsetY = 0;

        loadedINIFile_->clearSection("MAP");
        loadedINIFile_->setIntValue("MAP", "SizeX", map_.getSizeX());
        loadedINIFile_->setIntValue("MAP", "SizeY", map_.getSizeY());

        for (int y = 0; y < map_.getSizeY(); y++) {
            std::string rowKey = fmt::sprintf("%.3d", y);

            std::string row;
            for (int x = 0; x < map_.getSizeX(); x++) {
                switch (map_(x, y)) {

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

            loadedINIFile_->setStringValue("MAP", rowKey, row, false);
        }
    }

    for (int i = 1; i <= NUM_HOUSES; i++) {
        loadedINIFile_->removeSection("player" + std::to_string(i));
    }

    std::vector<std::string> house2housename;
    house2housename.reserve(players_.size());

    int currentAnyHouseNumber = 1;
    for (const Player& player : players_) {
        if (player.bAnyHouse_) {
            house2housename.emplace_back("Player" + std::to_string(currentAnyHouseNumber));
        } else {
            house2housename.emplace_back(player.name_);
        }

        if (player.bActive_) {
            const auto& h2h = house2housename.back();
            if (version < 2) {
                loadedINIFile_->setIntValue(h2h, "Quota", player.quota_);
                loadedINIFile_->setIntValue(h2h, "Credits", player.credits_);
                loadedINIFile_->setStringValue(h2h, "Brain", player.brain_, false);
                loadedINIFile_->setIntValue(h2h, "MaxUnit", player.maxunit_);
            } else {
                if (player.quota_ > 0) {
                    loadedINIFile_->setIntValue(h2h, "Quota", player.quota_);
                } else {
                    loadedINIFile_->removeKey(h2h, "Quota");
                }
                loadedINIFile_->setIntValue(h2h, "Credits", player.credits_);
                loadedINIFile_->setStringValue(h2h, "Brain", player.brain_, false);

                if (player.bAnyHouse_) {
                    currentAnyHouseNumber++;
                }
            }

            if (player.bAnyHouse_) {
                // remove corresponding house_ name
                loadedINIFile_->removeSection(player.name_);
            }

        } else {
            // remove corresponding house_ name
            loadedINIFile_->removeSection(player.name_);
        }
    }

    // remove players_ that are leftovers
    for (int i = currentAnyHouseNumber; i < NUM_HOUSES; i++) {
        loadedINIFile_->removeSection("Player" + std::to_string(i));
    }

    if (choam_.empty()) {
        loadedINIFile_->removeSection("CHOAM");
    } else {
        loadedINIFile_->clearSection("CHOAM");

        for (auto& [itemID, num] : choam_) {
            if (num == 0) {
                num = -1;
            }

            loadedINIFile_->setIntValue("CHOAM", getItemNameByID(itemID), num);
        }
    }

    if (ai_teams_.empty()) {
        loadedINIFile_->removeSection("TEAMS");
    } else {
        loadedINIFile_->clearSection("TEAMS");

        // we start at 0 for version 1 maps if we have 16 entries to not overflow the table
        int currentIndex = ((getMapVersion() < 2) && (ai_teams_.size() >= 16)) ? 0 : 1;
        for (const AITeamInfo& aiteamInfo : ai_teams_) {
            std::string value = house2housename[static_cast<int>(aiteamInfo.houseID)] + ","
                              + getAITeamBehaviorNameByID(aiteamInfo.aiTeamBehavior) + ","
                              + getAITeamTypeNameByID(aiteamInfo.aiTeamType) + "," + std::to_string(aiteamInfo.minUnits)
                              + "," + std::to_string(aiteamInfo.maxUnits);
            loadedINIFile_->setStringValue("TEAMS", std::to_string(currentIndex), value, false);
            currentIndex++;
        }
    }

    loadedINIFile_->clearSection("UNITS");
    for (const Unit& unit : units_) {
        if (unit.itemID_ < ItemID_enum::ItemID_FirstID || unit.itemID_ > ItemID_enum::ItemID_LastID)
            continue;

        std::string unitKey = fmt::sprintf("ID%.3d", unit.id_);

        int position = (logicalOffsetY + unit.position_.y) * logicalSizeX + (logicalOffsetX + unit.position_.x);

        int angle_ = static_cast<int>(unit.angle_);

        angle_ = (((NUM_ANGLES - angle_) + 2) % NUM_ANGLES) * 32;

        std::string unitValue = house2housename[static_cast<int>(unit.house_)] + "," + getItemNameByID(unit.itemID_)
                              + "," + std::to_string(unit.health_) + "," + std::to_string(position) + ","
                              + std::to_string(angle_) + "," + getAttackModeNameByMode(unit.attack_mode_);

        loadedINIFile_->setStringValue("UNITS", unitKey, unitValue, false);
    }

    loadedINIFile_->clearSection("STRUCTURES");
    for (const Structure& structure : structures_) {
        int position =
            (logicalOffsetY + structure.position_.y) * logicalSizeX + (logicalOffsetX + structure.position_.x);

        if ((structure.itemID_ == Structure_Slab1) || (structure.itemID_ == Structure_Slab4)
            || (structure.itemID_ == Structure_Wall)) {
            std::string structureKey = fmt::sprintf("GEN%.3d", position);

            std::string structureValue =
                house2housename[static_cast<int>(structure.house_)] + "," + getItemNameByID(structure.itemID_);

            loadedINIFile_->setStringValue("STRUCTURES", structureKey, structureValue, false);

        } else {

            std::string structureKey = fmt::sprintf("ID%.3d", structure.id_);

            std::string structureValue = house2housename[static_cast<int>(structure.house_)] + ","
                                       + getItemNameByID(structure.itemID_) + "," + std::to_string(structure.health_)
                                       + "," + std::to_string(position);

            loadedINIFile_->setStringValue("STRUCTURES", structureKey, structureValue, false);
        }
    }

    if (reinforcements_.empty()) {
        loadedINIFile_->removeSection("REINFORCEMENTS");
    } else {
        loadedINIFile_->clearSection("REINFORCEMENTS");

        // we start at 0 for version 1 maps if we have 16 entries to not overflow the table
        int currentIndex = ((getMapVersion() < 2) && (reinforcements_.size() >= 16)) ? 0 : 1;
        for (const ReinforcementInfo& reinforcement : reinforcements_) {
            std::string value = house2housename[static_cast<int>(reinforcement.houseID)] + ","
                              + getItemNameByID(reinforcement.unitID) + ","
                              + getDropLocationNameByID(reinforcement.dropLocation) + ","
                              + std::to_string(reinforcement.droptime);
            if (reinforcement.bRepeat) {
                value += ",+";
            }
            loadedINIFile_->setStringValue("REINFORCEMENTS", std::to_string(currentIndex), value, false);
            currentIndex++;
        }
    }

    if (!loadedINIFile_->saveChangesTo(filepath, getMapVersion() < 2)) {
        sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Unable to save configuration file %s",
                        reinterpret_cast<const char*>(filepath.u8string().c_str()));
    }

    lastSaveName_          = filepath;
    bChangedSinceLastSave_ = false;
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
                    const TERRAINTYPE terrainType = currentEditorMode_.terrainType_;

                    switch (terrainType) {
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

                        MapEditorStructurePlaceOperation placeOperation(position, nextHouse, currentEditorMode_.itemID_,
                                                                        currentEditorMode_.health_);

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

                        MapEditorUnitPlaceOperation placeOperation(position, nextHouse, currentEditorMode_.itemID_,
                                                                   currentEditorMode_.health_, angle_,
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
        case Terrain_Mountain: {
            if (map_.isInsideMap(x - 1, y) && (map_(x - 1, y) != Terrain_Mountain) && (map_(x - 1, y) != Terrain_Rock))
                performTerrainChange(x - 1, y, Terrain_Rock);
            if (map_.isInsideMap(x, y - 1) && (map_(x, y - 1) != Terrain_Mountain) && (map_(x, y - 1) != Terrain_Rock))
                performTerrainChange(x, y - 1, Terrain_Rock);
            if (map_.isInsideMap(x + 1, y) && (map_(x + 1, y) != Terrain_Mountain) && (map_(x + 1, y) != Terrain_Rock))
                performTerrainChange(x + 1, y, Terrain_Rock);
            if (map_.isInsideMap(x, y + 1) && (map_(x, y + 1) != Terrain_Mountain) && (map_(x, y + 1) != Terrain_Rock))
                performTerrainChange(x, y + 1, Terrain_Rock);
        } break;

        case Terrain_ThickSpice: {
            if (map_.isInsideMap(x - 1, y) && (map_(x - 1, y) != Terrain_ThickSpice)
                && (map_(x - 1, y) != Terrain_Spice))
                performTerrainChange(x - 1, y, Terrain_Spice);
            if (map_.isInsideMap(x, y - 1) && (map_(x, y - 1) != Terrain_ThickSpice)
                && (map_(x, y - 1) != Terrain_Spice))
                performTerrainChange(x, y - 1, Terrain_Spice);
            if (map_.isInsideMap(x + 1, y) && (map_(x + 1, y) != Terrain_ThickSpice)
                && (map_(x + 1, y) != Terrain_Spice))
                performTerrainChange(x + 1, y, Terrain_Spice);
            if (map_.isInsideMap(x, y + 1) && (map_(x, y + 1) != Terrain_ThickSpice)
                && (map_(x, y + 1) != Terrain_Spice))
                performTerrainChange(x, y + 1, Terrain_Spice);
        } break;

        case Terrain_Rock: {
            if (map_.isInsideMap(x - 1, y) && (map_(x - 1, y) == Terrain_ThickSpice))
                performTerrainChange(x - 1, y, Terrain_Spice);
            if (map_.isInsideMap(x, y - 1) && (map_(x, y - 1) == Terrain_ThickSpice))
                performTerrainChange(x, y - 1, Terrain_Spice);
            if (map_.isInsideMap(x + 1, y) && (map_(x + 1, y) == Terrain_ThickSpice))
                performTerrainChange(x + 1, y, Terrain_Spice);
            if (map_.isInsideMap(x, y + 1) && (map_(x, y + 1) == Terrain_ThickSpice))
                performTerrainChange(x, y + 1, Terrain_Spice);
        } break;

        case Terrain_Spice: {
            if (map_.isInsideMap(x - 1, y) && (map_(x - 1, y) == Terrain_Mountain))
                performTerrainChange(x - 1, y, Terrain_Rock);
            if (map_.isInsideMap(x, y - 1) && (map_(x, y - 1) == Terrain_Mountain))
                performTerrainChange(x, y - 1, Terrain_Rock);
            if (map_.isInsideMap(x + 1, y) && (map_(x + 1, y) == Terrain_Mountain))
                performTerrainChange(x + 1, y, Terrain_Rock);
            if (map_.isInsideMap(x, y + 1) && (map_(x, y + 1) == Terrain_Mountain))
                performTerrainChange(x, y + 1, Terrain_Rock);
        } break;

        case Terrain_Sand:
        case Terrain_Dunes:
        case Terrain_SpiceBloom:
        case Terrain_SpecialBloom: {
            if (map_.isInsideMap(x - 1, y) && (map_(x - 1, y) == Terrain_Mountain))
                performTerrainChange(x - 1, y, Terrain_Rock);
            if (map_.isInsideMap(x, y - 1) && (map_(x, y - 1) == Terrain_Mountain))
                performTerrainChange(x, y - 1, Terrain_Rock);
            if (map_.isInsideMap(x + 1, y) && (map_(x + 1, y) == Terrain_Mountain))
                performTerrainChange(x + 1, y, Terrain_Rock);
            if (map_.isInsideMap(x, y + 1) && (map_(x, y + 1) == Terrain_Mountain))
                performTerrainChange(x, y + 1, Terrain_Rock);

            if (map_.isInsideMap(x - 1, y) && (map_(x - 1, y) == Terrain_ThickSpice))
                performTerrainChange(x - 1, y, Terrain_Spice);
            if (map_.isInsideMap(x, y - 1) && (map_(x, y - 1) == Terrain_ThickSpice))
                performTerrainChange(x, y - 1, Terrain_Spice);
            if (map_.isInsideMap(x + 1, y) && (map_(x + 1, y) == Terrain_ThickSpice))
                performTerrainChange(x + 1, y, Terrain_Spice);
            if (map_.isInsideMap(x, y + 1) && (map_(x, y + 1) == Terrain_ThickSpice))
                performTerrainChange(x, y + 1, Terrain_Spice);
        } break;

        default: {
        } break;
    }
}

void MapEditor::drawScreen() {
    auto* const renderer = dune::globals::renderer.get();

    // clear whole screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // the actual map
    drawMap(dune::globals::screenborder.get(), false);

    pInterface_->draw(Point(0, 0));
    pInterface_->drawOverlay(Point(0, 0));

    // Cursor
    drawCursor();

    Dune_RenderPresent(renderer);
}

void MapEditor::processInput(MenuBase::event_handler_type handler) {
    SDL_Event event;

    auto* const screenborder = dune::globals::screenborder.get();

    while (SDL_PollEvent(&event)) {

        // first of all update mouse
        if (event.type == SDL_MOUSEMOTION) {
            const auto& mouse = event.motion;

            const auto& video = dune::globals::settings.video;

            dune::globals::drawnMouseX = std::max(0, std::min(mouse.x, video.width - 1));
            dune::globals::drawnMouseY = std::max(0, std::min(mouse.y, video.height - 1));
        }

        if (pInterface_->hasChildWindow()) {
            pInterface_->handleInput(event);
        } else {
            switch (event.type) {
                case SDL_KEYDOWN: {
                    switch (event.key.keysym.sym) {

                        case SDLK_RETURN: {
                            if (SDL_GetModState() & KMOD_ALT) {
                                toggleFullscreen();
                            }
                        } break;

                        case SDLK_TAB: {
                            if (SDL_GetModState() & KMOD_ALT) {
                                SDL_MinimizeWindow(dune::globals::window.get());
                            }
                        } break;

                        case SDLK_F1: {
                            Coord oldCenterCoord            = screenborder->getCurrentCenter();
                            dune::globals::currentZoomlevel = 0;
                            screenborder->adjustScreenBorderToMapsize(map_.getSizeX(), map_.getSizeY());
                            screenborder->setNewScreenCenter(oldCenterCoord);
                        } break;

                        case SDLK_F2: {
                            Coord oldCenterCoord            = screenborder->getCurrentCenter();
                            dune::globals::currentZoomlevel = 1;
                            screenborder->adjustScreenBorderToMapsize(map_.getSizeX(), map_.getSizeY());
                            screenborder->setNewScreenCenter(oldCenterCoord);
                        } break;

                        case SDLK_F3: {
                            Coord oldCenterCoord            = screenborder->getCurrentCenter();
                            dune::globals::currentZoomlevel = 2;
                            screenborder->adjustScreenBorderToMapsize(map_.getSizeX(), map_.getSizeY());
                            screenborder->setNewScreenCenter(oldCenterCoord);
                        } break;

                        case SDLK_p: {
                            if (SDL_GetModState() & KMOD_CTRL) {
                                saveMapshot();
                            }
                        } break;

                        case SDLK_PRINTSCREEN:
                        case SDLK_SYSREQ: {
                            saveMapshot();
                        } break;

                        case SDLK_z: {
                            if (SDL_GetModState() & KMOD_CTRL) {
                                pInterface_->onUndo();
                            }
                        } break;

                        case SDLK_y: {
                            if (SDL_GetModState() & KMOD_CTRL) {
                                pInterface_->onRedo();
                            }
                        } break;

                        default: break;
                    }

                } break;

                case SDL_KEYUP: {
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE: {
                            // quitting
                            pInterface_->onQuit();
                        } break;

                        case SDLK_DELETE:
                        case SDLK_BACKSPACE: {

                            // check units first
                            if (selectedUnitID_ != INVALID) {
                                clearRedoOperations();
                                startOperation();

                                std::vector<int> selectedUnits = getMirrorUnits(selectedUnitID_);

                                for (const int selectedUnit : selectedUnits) {
                                    MapEditorRemoveUnitOperation removeOperation(selectedUnit);
                                    addUndoOperation(removeOperation.perform(this));
                                }
                                selectedUnitID_ = INVALID;

                                pInterface_->deselectAll();
                            } else if (selectedStructureID_ != INVALID) {
                                // We only try deleting structures_ if we had not yet deleted a unit (e.g. a unit on
                                // concrete)
                                clearRedoOperations();
                                startOperation();

                                std::vector<int> selectedStructures = getMirrorStructures(selectedStructureID_);

                                for (const int selectedStructure : selectedStructures) {
                                    MapEditorRemoveStructureOperation removeOperation(selectedStructure);
                                    addUndoOperation(removeOperation.perform(this));
                                }
                                selectedStructureID_ = INVALID;

                                pInterface_->deselectAll();
                            } else if (selectedMapItemCoord_.isValid()) {
                                auto iter = std::ranges::find(specialBlooms_, selectedMapItemCoord_);

                                if (iter != specialBlooms_.end()) {
                                    clearRedoOperations();
                                    startOperation();
                                    MapEditorTerrainRemoveSpecialBloomOperation removeOperation(iter->x, iter->y);
                                    addUndoOperation(removeOperation.perform(this));

                                    selectedMapItemCoord_.invalidate();
                                } else {
                                    iter = std::ranges::find(spiceBlooms_, selectedMapItemCoord_);

                                    if (iter != spiceBlooms_.end()) {
                                        clearRedoOperations();
                                        startOperation();
                                        MapEditorTerrainRemoveSpiceBloomOperation removeOperation(iter->x, iter->y);
                                        addUndoOperation(removeOperation.perform(this));

                                        selectedMapItemCoord_.invalidate();
                                    } else {
                                        iter = std::ranges::find(spiceFields_, selectedMapItemCoord_);

                                        if (iter != spiceFields_.end()) {
                                            clearRedoOperations();
                                            startOperation();
                                            MapEditorTerrainRemoveSpiceFieldOperation removeOperation(iter->x, iter->y);
                                            addUndoOperation(removeOperation.perform(this));

                                            selectedMapItemCoord_.invalidate();
                                        }
                                    }
                                }
                            }

                        } break;

                        default: break;
                    }
                } break;

                case SDL_MOUSEMOTION: {
                    pInterface_->handleMouseMovement(dune::globals::drawnMouseX, dune::globals::drawnMouseY);

                    if (bLeftMousePressed_) {
                        if (screenborder->isScreenCoordInsideMap(dune::globals::drawnMouseX,
                                                                 dune::globals::drawnMouseY)) {
                            // if mouse is not over side bar

                            const int xpos = screenborder->screen2MapX(dune::globals::drawnMouseX);
                            const int ypos = screenborder->screen2MapY(dune::globals::drawnMouseY);

                            if ((xpos != lastTerrainEditPosX_) || (ypos != lastTerrainEditPosY_)) {
                                performMapEdit(xpos, ypos, true);
                            }
                        }
                    }
                } break;

                case SDL_MOUSEWHEEL: {
                    if (event.wheel.y != 0) {
                        if (screenborder->isScreenCoordInsideMap(dune::globals::drawnMouseX,
                                                                 dune::globals::drawnMouseY)) {
                            // if mouse is not over side bar
                            const int xpos = screenborder->screen2MapX(dune::globals::drawnMouseX);
                            const int ypos = screenborder->screen2MapY(dune::globals::drawnMouseY);

                            for (const Unit& unit : units_) {
                                const Coord position = unit.position_;
                                if ((position.x == xpos) && (position.y == ypos)) {
                                    if (event.wheel.y > 0) {
                                        pInterface_->onUnitRotateLeft(unit.id_);
                                    } else {
                                        pInterface_->onUnitRotateRight(unit.id_);
                                    }
                                    break;
                                }
                            }
                        }

                        pInterface_->handleMouseWheel(dune::globals::drawnMouseX, dune::globals::drawnMouseY,
                                                      (event.wheel.y > 0));
                    }
                } break;

                case SDL_MOUSEBUTTONDOWN: {
                    const SDL_MouseButtonEvent* mouse = &event.button;

                    switch (mouse->button) {
                        case SDL_BUTTON_LEFT: {
                            if (!pInterface_->handleMouseLeft(mouse->x, mouse->y, true)) {

                                bLeftMousePressed_ = true;

                                if (screenborder->isScreenCoordInsideMap(mouse->x, mouse->y)) {
                                    // if mouse is not over side bar

                                    const int xpos = screenborder->screen2MapX(mouse->x);
                                    const int ypos = screenborder->screen2MapY(mouse->y);

                                    performMapEdit(xpos, ypos, false);
                                }
                            }

                        } break;

                        case SDL_BUTTON_RIGHT: {
                            if (!pInterface_->handleMouseRight(mouse->x, mouse->y, true)) {

                                if (screenborder->isScreenCoordInsideMap(mouse->x, mouse->y)) {
                                    // if mouse is not over side bar
                                    setEditorMode(EditorMode());
                                    pInterface_->deselectAll();
                                }
                            }
                        } break;
                    }
                } break;

                case SDL_MOUSEBUTTONUP: {
                    const SDL_MouseButtonEvent* mouse = &event.button;

                    switch (mouse->button) {
                        case SDL_BUTTON_LEFT: {

                            pInterface_->handleMouseLeft(mouse->x, mouse->y, false);

                            if (bLeftMousePressed_) {

                                bLeftMousePressed_   = false;
                                lastTerrainEditPosX_ = -1;
                                lastTerrainEditPosY_ = -1;

                                if (currentEditorMode_.mode_ == EditorMode::EditorMode_Selection) {
                                    if (screenborder->isScreenCoordInsideMap(mouse->x, mouse->y)) {
                                        // if mouse is not over side bar

                                        const int xpos = screenborder->screen2MapX(mouse->x);
                                        const int ypos = screenborder->screen2MapY(mouse->y);

                                        selectedUnitID_      = INVALID;
                                        selectedStructureID_ = INVALID;
                                        selectedMapItemCoord_.invalidate();

                                        bool bUnitSelected = false;

                                        for (const Unit& unit : units_) {
                                            const Coord position = unit.position_;

                                            if ((position.x == xpos) && (position.y == ypos)) {
                                                selectedUnitID_ = unit.id_;
                                                bUnitSelected   = true;
                                                pInterface_->onObjectSelected();
                                                mapInfo_.cursorPos = position;
                                                break;
                                            }
                                        }

                                        bool bStructureSelected = false;

                                        for (const Structure& structure : structures_) {
                                            const Coord& position     = structure.position_;
                                            const Coord structureSize = getStructureSize(structure.itemID_);

                                            if (!bUnitSelected && (xpos >= position.x)
                                                && (xpos < position.x + structureSize.x) && (ypos >= position.y)
                                                && (ypos < position.y + structureSize.y)) {
                                                selectedStructureID_ = structure.id_;
                                                bStructureSelected   = true;
                                                pInterface_->onObjectSelected();
                                                mapInfo_.cursorPos = position;
                                                break;
                                            }
                                        }

                                        if (!bUnitSelected && !bStructureSelected) {
                                            pInterface_->deselectAll();

                                            // find map items (spice bloom, special bloom or spice field)
                                            if ((std::ranges::find(spiceBlooms_, Coord(xpos, ypos))
                                                 != spiceBlooms_.end())
                                                || (std::ranges::find(specialBlooms_, Coord(xpos, ypos))
                                                    != specialBlooms_.end())
                                                || (std::ranges::find(spiceFields_, Coord(xpos, ypos))
                                                    != spiceFields_.end())) {
                                                selectedMapItemCoord_ = Coord(xpos, ypos);
                                            }
                                        }
                                    }
                                }
                            }
                        } break;

                        case SDL_BUTTON_RIGHT: {
                            pInterface_->handleMouseRight(mouse->x, mouse->y, false);
                        } break;
                    }
                } break;

                case SDL_QUIT: {
                    bQuitEditor_ = true;
                } break;
            }
        }

        if (handler && Window::isBroadcastEventType(event.type))
            handler(event);
    }

    if ((!pInterface_->hasChildWindow())
        && (SDL_GetWindowFlags(dune::globals::window.get()) & SDL_WINDOW_INPUT_FOCUS)) {
        const uint8_t* const keystate = SDL_GetKeyboardState(nullptr);
        scrollDownMode_ =
            (dune::globals::drawnMouseY >= getRendererHeight() - 1 - SCROLLBORDER) || keystate[SDL_SCANCODE_DOWN];
        scrollLeftMode_ = (dune::globals::drawnMouseX <= SCROLLBORDER) || keystate[SDL_SCANCODE_LEFT];
        scrollRightMode_ =
            (dune::globals::drawnMouseX >= getRendererWidth() - 1 - SCROLLBORDER) || keystate[SDL_SCANCODE_RIGHT];
        scrollUpMode_ = (dune::globals::drawnMouseY <= SCROLLBORDER) || keystate[SDL_SCANCODE_UP];

        if (scrollLeftMode_ && scrollRightMode_) {
            // do nothing
        } else if (scrollLeftMode_) {
            scrollLeftMode_ = screenborder->scrollLeft();
        } else if (scrollRightMode_) {
            scrollRightMode_ = screenborder->scrollRight();
        }

        if (scrollDownMode_ && scrollUpMode_) {
            // do nothing
        } else if (scrollDownMode_) {
            scrollDownMode_ = screenborder->scrollDown();
        } else if (scrollUpMode_) {
            scrollUpMode_ = screenborder->scrollUp();
        }
    } else {
        scrollDownMode_  = false;
        scrollLeftMode_  = false;
        scrollRightMode_ = false;
        scrollUpMode_    = false;
    }
}

void MapEditor::drawCursor() {

    if (!(SDL_GetWindowFlags(dune::globals::window.get()) & SDL_WINDOW_MOUSE_FOCUS))
        return;

    const auto* const gfx = dune::globals::pGFXManager.get();
    using dune::globals::drawnMouseX;
    using dune::globals::drawnMouseY;

    const DuneTexture* pCursor = nullptr;
    SDL_FRect dest{};

    if (scrollLeftMode_ || scrollRightMode_ || scrollUpMode_ || scrollDownMode_) {
        if (scrollLeftMode_ && !scrollRightMode_) {
            pCursor = gfx->getUIGraphic(UI_CursorLeft);
            dest    = calcDrawingRect(pCursor, drawnMouseX, dune::globals::drawnMouseY - 5, HAlign::Left, VAlign::Top);
        } else if (scrollRightMode_ && !scrollLeftMode_) {
            pCursor = gfx->getUIGraphic(UI_CursorRight);
            dest = calcDrawingRect(pCursor, drawnMouseX, dune::globals::drawnMouseY - 5, HAlign::Center, VAlign::Top);
        }

        if (pCursor == nullptr) {
            if (scrollUpMode_ && !scrollDownMode_) {
                pCursor = gfx->getUIGraphic(UI_CursorUp);
                dest = calcDrawingRect(pCursor, drawnMouseX - 5, dune::globals::drawnMouseY, HAlign::Left, VAlign::Top);
            } else if (scrollDownMode_ && !scrollUpMode_) {
                pCursor = gfx->getUIGraphic(UI_CursorDown);
                dest =
                    calcDrawingRect(pCursor, drawnMouseX - 5, dune::globals::drawnMouseY, HAlign::Left, VAlign::Center);
            } else {
                pCursor = gfx->getUIGraphic(UI_CursorNormal);
                dest    = calcDrawingRect(pCursor, drawnMouseX, dune::globals::drawnMouseY, HAlign::Left, VAlign::Top);
            }
        }
    } else {
        pCursor = gfx->getUIGraphic(UI_CursorNormal);
        dest    = calcDrawingRect(pCursor, drawnMouseX, dune::globals::drawnMouseY, HAlign::Left, VAlign::Top);

        if ((drawnMouseX < sideBarPos_.x) && (dune::globals::drawnMouseY > topBarPos_.h)
            && (currentMirrorMode_ != MirrorModeNone) && (!pInterface_->hasChildWindow())) {

            const DuneTexture* pMirrorIcon = nullptr;
            switch (currentMirrorMode_) {
                case MirrorModeHorizontal: pMirrorIcon = gfx->getUIGraphic(UI_MapEditor_MirrorHorizontalIcon); break;
                case MirrorModeVertical: pMirrorIcon = gfx->getUIGraphic(UI_MapEditor_MirrorVerticalIcon); break;
                case MirrorModeBoth: pMirrorIcon = gfx->getUIGraphic(UI_MapEditor_MirrorBothIcon); break;
                case MirrorModePoint: pMirrorIcon = gfx->getUIGraphic(UI_MapEditor_MirrorPointIcon); break;
                default: pMirrorIcon = gfx->getUIGraphic(UI_MapEditor_MirrorNoneIcon); break;
            }

            if (pMirrorIcon)
                pMirrorIcon->draw(dune::globals::renderer.get(), drawnMouseX + 5, dune::globals::drawnMouseY + 5);
        }
    }

    if (pCursor)
        pCursor->draw(dune::globals::renderer.get(), dest.x, dest.y);
}

TERRAINTYPE MapEditor::getTerrain(int x, int y) const {
    TERRAINTYPE terrainType = map_(x, y);

    if (map_(x, y) == Terrain_Sand) {
        if (std::ranges::find(spiceFields_, Coord{x, y}) != spiceFields_.end()) {
            terrainType = Terrain_ThickSpice;
        } else if (std::ranges::any_of(spiceFields_, [center = Coord(x, y)](const auto& coord) {
                       return distanceFrom(center, coord) < 5;
                   })) {
            terrainType = Terrain_Spice;
        }
    }

    // check for classic map items (spice blooms, special blooms)
    if (std::ranges::find(spiceBlooms_, Coord(x, y)) != spiceBlooms_.end()) {
        terrainType = Terrain_SpiceBloom;
    }

    if (std::ranges::find(specialBlooms_, Coord(x, y)) != specialBlooms_.end()) {
        terrainType = Terrain_SpecialBloom;
    }

    return terrainType;
}

void MapEditor::drawMap(ScreenBorder* pScreenborder, bool bCompleteMap) const {
    const auto zoomedTilesize = world2zoomedWorld(TILESIZE);

    Coord TopLeftTile     = pScreenborder->getTopLeftTile();
    Coord BottomRightTile = pScreenborder->getBottomRightTile();

    // extend the view a little bit to avoid graphical glitches
    TopLeftTile.x     = std::max(0, TopLeftTile.x - 1);
    TopLeftTile.y     = std::max(0, TopLeftTile.y - 1);
    BottomRightTile.x = std::min(map_.getSizeX() - 1, BottomRightTile.x + 1);
    BottomRightTile.y = std::min(map_.getSizeY() - 1, BottomRightTile.y + 1);

    const auto zoom = dune::globals::currentZoomlevel;

    // Load Terrain Surface
    const auto* const terrainSprite = dune::globals::pGFXManager->getZoomedObjPic(ObjPic_Terrain, zoom);

    /* draw ground */
    for (int y = TopLeftTile.y; y <= BottomRightTile.y; y++) {
        for (int x = TopLeftTile.x; x <= BottomRightTile.x; x++) {

            int tile = 0;

            switch (getTerrain(x, y)) {
                case Terrain_Slab: {
                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Slab);
                } break;

                case Terrain_Sand: {
                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Sand);
                } break;

                case Terrain_Rock: {
                    // determine which surrounding tiles are rock
                    const int up = (y - 1 < 0) || (getTerrain(x, y - 1) == Terrain_Rock)
                                || (getTerrain(x, y - 1) == Terrain_Slab) || (getTerrain(x, y - 1) == Terrain_Mountain);
                    const int right = (x + 1 >= map_.getSizeX()) || (getTerrain(x + 1, y) == Terrain_Rock)
                                   || (getTerrain(x + 1, y) == Terrain_Slab)
                                   || (getTerrain(x + 1, y) == Terrain_Mountain);
                    const int down = (y + 1 >= map_.getSizeY()) || (getTerrain(x, y + 1) == Terrain_Rock)
                                  || (getTerrain(x, y + 1) == Terrain_Slab)
                                  || (getTerrain(x, y + 1) == Terrain_Mountain);
                    const int left = (x - 1 < 0) || (getTerrain(x - 1, y) == Terrain_Rock)
                                  || (getTerrain(x - 1, y) == Terrain_Slab)
                                  || (getTerrain(x - 1, y) == Terrain_Mountain);

                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Rock)
                         + (up | (right << 1) | (down << 2) | (left << 3));
                } break;

                case Terrain_Dunes: {
                    // determine which surrounding tiles are dunes
                    const int up    = (y - 1 < 0) || (getTerrain(x, y - 1) == Terrain_Dunes);
                    const int right = (x + 1 >= map_.getSizeX()) || (getTerrain(x + 1, y) == Terrain_Dunes);
                    const int down  = (y + 1 >= map_.getSizeY()) || (getTerrain(x, y + 1) == Terrain_Dunes);
                    const int left  = (x - 1 < 0) || (getTerrain(x - 1, y) == Terrain_Dunes);

                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Dunes)
                         + (up | (right << 1) | (down << 2) | (left << 3));
                } break;

                case Terrain_Mountain: {
                    // determine which surrounding tiles are mountains
                    const int up    = (y - 1 < 0) || (getTerrain(x, y - 1) == Terrain_Mountain);
                    const int right = (x + 1 >= map_.getSizeX()) || (getTerrain(x + 1, y) == Terrain_Mountain);
                    const int down  = (y + 1 >= map_.getSizeY()) || (getTerrain(x, y + 1) == Terrain_Mountain);
                    const int left  = (x - 1 < 0) || (getTerrain(x - 1, y) == Terrain_Mountain);

                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Mountain)
                         + (up | (right << 1) | (down << 2) | (left << 3));
                } break;

                case Terrain_Spice: {
                    // determine which surrounding tiles are spice
                    const int up = (y - 1 < 0) || (getTerrain(x, y - 1) == Terrain_Spice)
                                || (getTerrain(x, y - 1) == Terrain_ThickSpice);
                    const int right = (x + 1 >= map_.getSizeX()) || (getTerrain(x + 1, y) == Terrain_Spice)
                                   || (getTerrain(x + 1, y) == Terrain_ThickSpice);
                    const int down = (y + 1 >= map_.getSizeY()) || (getTerrain(x, y + 1) == Terrain_Spice)
                                  || (getTerrain(x, y + 1) == Terrain_ThickSpice);
                    const int left = (x - 1 < 0) || (getTerrain(x - 1, y) == Terrain_Spice)
                                  || (getTerrain(x - 1, y) == Terrain_ThickSpice);

                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Spice)
                         + (up | (right << 1) | (down << 2) | (left << 3));
                } break;

                case Terrain_ThickSpice: {
                    // determine which surrounding tiles are thick spice
                    const int up    = (y - 1 < 0) || (getTerrain(x, y - 1) == Terrain_ThickSpice);
                    const int right = (x + 1 >= map_.getSizeX()) || (getTerrain(x + 1, y) == Terrain_ThickSpice);
                    const int down  = (y + 1 >= map_.getSizeY()) || (getTerrain(x, y + 1) == Terrain_ThickSpice);
                    const int left  = (x - 1 < 0) || (getTerrain(x - 1, y) == Terrain_ThickSpice);

                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_ThickSpice)
                         + (up | (right << 1) | (down << 2) | (left << 3));
                } break;

                case Terrain_SpiceBloom: {
                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_SpiceBloom);
                } break;

                case Terrain_SpecialBloom: {
                    tile = static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_SpecialBloom);
                } break;

                default: {
                    THROW(std::runtime_error, "MapEditor::DrawMap(): Invalid terrain type");
                }
            }

            // draw map[x][y]
            const SDL_Rect source{(tile % NUM_TERRAIN_TILES_X) * zoomedTilesize,
                                  (tile / NUM_TERRAIN_TILES_X) * zoomedTilesize, zoomedTilesize, zoomedTilesize};
            const SDL_FRect drawLocation{(pScreenborder->world2screenX(x * TILESIZE)),
                                         (pScreenborder->world2screenY(y * TILESIZE)),
                                         static_cast<float>(zoomedTilesize), static_cast<float>(zoomedTilesize)};
            Dune_RenderCopyF(dune::globals::renderer.get(), terrainSprite, &source, &drawLocation);
        }
    }

    std::vector<int> selectedStructures = getMirrorStructures(selectedStructureID_);

    auto* const gfx      = dune::globals::pGFXManager.get();
    auto* const renderer = dune::globals::renderer.get();

    for (const auto& structure : structures_) {

        Coord position = structure.position_;

        SDL_Rect selectionDest;
        if (structure.itemID_ == Structure_Slab1) {
            // Load Terrain sprite
            SDL_Rect source = {static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Slab) * zoomedTilesize, 0,
                               zoomedTilesize, zoomedTilesize};
            SDL_FRect dest  = {(pScreenborder->world2screenX(position.x * TILESIZE)),
                               (pScreenborder->world2screenY(position.y * TILESIZE)), static_cast<float>(zoomedTilesize),
                               static_cast<float>(zoomedTilesize)};

            Dune_RenderCopyF(renderer, terrainSprite, &source, &dest);

            selectionDest = SDL_Rect{static_cast<int>(dest.x), static_cast<int>(dest.y), static_cast<int>(dest.w),
                                     static_cast<int>(dest.h)};
        } else if (structure.itemID_ == Structure_Slab4) {
            // Load Terrain Surface
            for (int y = position.y; y < position.y + 2; y++) {
                for (int x = position.x; x < position.x + 2; x++) {
                    SDL_Rect source = {static_cast<int>(Tile::TERRAINTILETYPE::TerrainTile_Slab) * zoomedTilesize, 0,
                                       zoomedTilesize, zoomedTilesize};
                    SDL_FRect dest  = {(pScreenborder->world2screenX(x * TILESIZE)),
                                       (pScreenborder->world2screenY(y * TILESIZE)), static_cast<float>(zoomedTilesize),
                                       static_cast<float>(zoomedTilesize)};

                    Dune_RenderCopyF(renderer, terrainSprite, &source, &dest);
                }
            }

            selectionDest.x = pScreenborder->world2screenX(position.x * TILESIZE);
            selectionDest.y = pScreenborder->world2screenY(position.y * TILESIZE);
            selectionDest.w = world2zoomedWorld(2 * TILESIZE);
            selectionDest.h = world2zoomedWorld(2 * TILESIZE);
        } else if (structure.itemID_ == Structure_Wall) {
            bool left  = false;
            bool down  = false;
            bool right = false;
            bool up    = false;
            for (const Structure& structure1 : structures_) {
                if (structure1.itemID_ == Structure_Wall) {
                    if ((structure1.position_.x == position.x - 1) && (structure1.position_.y == position.y))
                        left = true;
                    if ((structure1.position_.x == position.x) && (structure1.position_.y == position.y + 1))
                        down = true;
                    if ((structure1.position_.x == position.x + 1) && (structure1.position_.y == position.y))
                        right = true;
                    if ((structure1.position_.x == position.x) && (structure1.position_.y == position.y - 1))
                        up = true;
                }
            }

            auto maketile = 0;
            if ((left) && (right) && (up) && (down)) {
                maketile = Wall::Wall_Full; // solid wall
            } else if ((!left) && (right) && (up) && (down)) {
                maketile = Wall::Wall_UpDownRight; // missing left edge
            } else if ((left) && (!right) && (up) && (down)) {
                maketile = Wall::Wall_UpDownLeft; // missing right edge
            } else if ((left) && (right) && (!up) && (down)) {
                maketile = Wall::Wall_DownLeftRight; // missing top edge
            } else if ((left) && (right) && (up) && (!down)) {
                maketile = Wall::Wall_UpLeftRight; // missing bottom edge
            } else if ((!left) && (right) && (!up) && (down)) {
                maketile = Wall::Wall_DownRight; // missing top left edge
            } else if ((left) && (!right) && (up) && (!down)) {
                maketile = Wall::Wall_UpLeft; // missing bottom right edge
            } else if ((left) && (!right) && (!up) && (down)) {
                maketile = Wall::Wall_DownLeft; // missing top right edge
            } else if ((!left) && (right) && (up) && (!down)) {
                maketile = Wall::Wall_UpRight; // missing bottom left edge
            } else if ((left) && (!right) && (!up) && (!down)) {
                maketile = Wall::Wall_LeftRight; // missing above, right and below
            } else if ((!left) && (right) && (!up) && (!down)) {
                maketile = Wall::Wall_LeftRight; // missing above, left and below
            } else if ((!left) && (!right) && (up) && (!down)) {
                maketile = Wall::Wall_UpDown; // only up
            } else if ((!left) && (!right) && (!up) && (down)) {
                maketile = Wall::Wall_UpDown; // only down
            } else if ((left) && (right) && (!up) && (!down)) {
                maketile = Wall::Wall_LeftRight; // missing above and below
            } else if ((!left) && (!right) && (up) && (down)) {
                maketile = Wall::Wall_UpDown; // missing left and right
            } else if ((!left) && (!right) && (!up) && (!down)) {
                maketile = Wall::Wall_Standalone; // missing left and right
            }

            // Load Wall texture
            const auto* const WallSprite = gfx->getZoomedObjPic(ObjPic_Wall, zoom);

            const SDL_Rect source{maketile * zoomedTilesize, 0, zoomedTilesize, zoomedTilesize};
            const SDL_FRect dest{(pScreenborder->world2screenX(position.x * TILESIZE)),
                                 (pScreenborder->world2screenY(position.y * TILESIZE)),
                                 static_cast<float>(zoomedTilesize), static_cast<float>(zoomedTilesize)};

            Dune_RenderCopyF(renderer, WallSprite, &source, &dest);

            selectionDest = SDL_Rect{static_cast<int>(dest.x), static_cast<int>(dest.y), static_cast<int>(dest.w),
                                     static_cast<int>(dest.h)};
        } else {

            ObjPic_enum objectPic{};

            // clang-format off
            switch(structure.itemID_) {
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
                default:                            objectPic = {};                         break;
            }
            // clang-format on

            const auto* ObjectSprite = gfx->getZoomedObjPic(objectPic, structure.house_, zoom);

            Coord frameSize = world2zoomedWorld(getStructureSize(structure.itemID_) * TILESIZE);

            SDL_Rect source = {frameSize.x * (structure.itemID_ == Structure_WindTrap ? 9 : 2), 0, frameSize.x,
                               frameSize.y};
            SDL_FRect dest  = {(pScreenborder->world2screenX(position.x * TILESIZE)),
                               (pScreenborder->world2screenY(position.y * TILESIZE)), static_cast<float>(frameSize.x),
                               static_cast<float>(frameSize.y)};

            Dune_RenderCopyF(renderer, ObjectSprite, &source, &dest);

            selectionDest = SDL_Rect{static_cast<int>(dest.x), static_cast<int>(dest.y), static_cast<int>(dest.w),
                                     static_cast<int>(dest.h)};
        }

        // draw selection frame
        if (!bCompleteMap && (std::ranges::find(selectedStructures, structure.id_) != selectedStructures.end())) {
            // now draw the selection box thing, with parts at all corners of structure

            DuneDrawSelectionBox(renderer, selectionDest);
        }
    }

    for (const Unit& unit : units_) {

        const Coord& position = unit.position_;

        static constexpr auto tankTurretOffset =
            std::to_array<Coord>({{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}});

        static constexpr auto siegeTankTurretOffset =
            std::to_array<Coord>({{8, -12}, {0, -20}, {0, -20}, {-4, -20}, {-8, -12}, {-8, -4}, {-4, -12}, {8, -4}});

        static constexpr auto sonicTankTurretOffset =
            std::to_array<Coord>({{0, -8}, {0, -8}, {0, -8}, {0, -8}, {0, -8}, {0, -8}, {0, -8}, {0, -8}});

        static constexpr auto launcherTurretOffset =
            std::to_array<Coord>({{0, -12}, {0, -8}, {0, -8}, {0, -8}, {0, -12}, {0, -8}, {0, -8}, {0, -8}});

        static constexpr auto devastatorTurretOffset =
            std::to_array<Coord>({{8, -16}, {-4, -12}, {0, -16}, {4, -12}, {-8, -16}, {0, -12}, {-4, -12}, {0, -12}});

        ObjPic_enum objectPicBase{};
        auto framesX = NUM_ANGLES;
        auto framesY = 1;

        auto objectPicGun     = static_cast<ObjPic_enum>(-1);
        const auto* gunOffset = static_cast<decltype(tankTurretOffset)*>(nullptr);

        // clang-format off
        switch(unit.itemID_) {
            case Unit_Carryall:         objectPicBase = ObjPic_Carryall;        framesY = 2;                                                                    break;
            case Unit_Devastator:       objectPicBase = ObjPic_Devastator_Base; objectPicGun = ObjPic_Devastator_Gun;   gunOffset = &devastatorTurretOffset;     break;
            case Unit_Deviator:         objectPicBase = ObjPic_Tank_Base;       objectPicGun = ObjPic_Launcher_Gun;     gunOffset = &launcherTurretOffset;       break;
            case Unit_Frigate:          objectPicBase = ObjPic_Frigate;                                                                                         break;
            case Unit_Harvester:        objectPicBase = ObjPic_Harvester;                                                                                       break;
            case Unit_Soldier:          objectPicBase = ObjPic_Soldier;         framesX = 4;    framesY = 3;                                                    break;
            case Unit_Launcher:         objectPicBase = ObjPic_Tank_Base;       objectPicGun = ObjPic_Launcher_Gun;     gunOffset = &launcherTurretOffset;       break;
            case Unit_MCV:              objectPicBase = ObjPic_MCV;                                                                                             break;
            case Unit_Ornithopter:      objectPicBase = ObjPic_Ornithopter;     framesY = 3;                                                                    break;
            case Unit_Quad:             objectPicBase = ObjPic_Quad;                                                                                            break;
            case Unit_Saboteur:         objectPicBase = ObjPic_Saboteur;        framesX = 4;    framesY = 3;                                                    break;
            case Unit_Sandworm:         objectPicBase = ObjPic_Sandworm;        framesX = 1;    framesY = 9;                                                    break;
            case Unit_SiegeTank:        objectPicBase = ObjPic_Siegetank_Base;  objectPicGun = ObjPic_Siegetank_Gun;    gunOffset = &siegeTankTurretOffset;      break;
            case Unit_SonicTank:        objectPicBase = ObjPic_Tank_Base;       objectPicGun = ObjPic_Sonictank_Gun;    gunOffset = &sonicTankTurretOffset;      break;
            case Unit_Tank:             objectPicBase = ObjPic_Tank_Base;       objectPicGun = ObjPic_Tank_Gun;         gunOffset = &tankTurretOffset;           break;
            case Unit_Trike:
            case Unit_RaiderTrike:      objectPicBase = ObjPic_Trike;                                                                                           break;
            case Unit_Trooper:          objectPicBase = ObjPic_Trooper;         framesX = 4;    framesY = 3;                                                    break;
            case Unit_Special:          objectPicBase = ObjPic_Devastator_Base; objectPicGun = ObjPic_Devastator_Gun;   gunOffset = &devastatorTurretOffset;     break;
            case Unit_Infantry:         objectPicBase = ObjPic_Infantry;         framesX = 4;    framesY = 4;                                                   break;
            case Unit_Troopers:         objectPicBase = ObjPic_Troopers;         framesX = 4;    framesY = 4;                                                   break;
        }
        // clang-format on

        const auto* const pObjectSprite = gfx->getZoomedObjPic(objectPicBase, unit.house_, zoom);

        const auto angle_int = static_cast<int>(unit.angle_);
        int angle_           = angle_int / (NUM_ANGLES / framesX);

        int frame = (unit.itemID_ == Unit_Sandworm) ? 5 : 0;

        auto source = calcSpriteSourceRect(pObjectSprite, angle_, framesX, frame, framesY);

        int frameSizeX = source.w;
        int frameSizeY = source.h;

        const auto drawLocation = calcSpriteDrawingRectF(
            pObjectSprite, pScreenborder->world2screenX((position.x * TILESIZE) + (TILESIZE / 2)),
            pScreenborder->world2screenY((position.y * TILESIZE) + (TILESIZE / 2)), framesX, framesY, HAlign::Center,
            VAlign::Center);

        Dune_RenderCopyF(renderer, pObjectSprite, &source, &drawLocation);

        if (objectPicGun >= 0) {
            const auto* const pGunSprite = gfx->getZoomedObjPic(objectPicGun, unit.house_, zoom);

            auto source2 = calcSpriteSourceRect(pGunSprite, angle_int, NUM_ANGLES);

            const auto& gun = (*gunOffset)[angle_int];
            const auto sx   = pScreenborder->world2screenX((position.x * TILESIZE) + (TILESIZE / 2) + gun.x);
            const auto sy   = pScreenborder->world2screenY((position.y * TILESIZE) + (TILESIZE / 2) + gun.y);

            const auto drawLocation2 =
                calcSpriteDrawingRectF(pGunSprite, sx, sy, NUM_ANGLES, 1, HAlign::Center, VAlign::Center);

            Dune_RenderCopyF(renderer, pGunSprite, &source2, &drawLocation2);
        }

        if (unit.itemID_ == Unit_RaiderTrike || unit.itemID_ == Unit_Deviator || unit.itemID_ == Unit_Special) {
            const auto* const pStarSprite = gfx->getZoomedObjPic(ObjPic_Star, zoom);

            auto drawLocation2 = calcDrawingRect(
                pStarSprite,
                pScreenborder->world2screenX((position.x * TILESIZE) + (TILESIZE / 2)) + frameSizeX / 2 - 1,
                pScreenborder->world2screenY((position.y * TILESIZE) + (TILESIZE / 2)) + frameSizeY / 2 - 1,
                HAlign::Right, VAlign::Bottom);

            pStarSprite->draw(renderer, drawLocation2.x, drawLocation2.y);
        }
    }

    // draw tactical pos rectangle (the starting screen)
    if (!bCompleteMap && getMapVersion() < 2 && mapInfo_.tacticalPos.isValid()) {

        SDL_FRect dest;
        dest.x = pScreenborder->world2screenX(mapInfo_.tacticalPos.x * TILESIZE);
        dest.y = pScreenborder->world2screenY(mapInfo_.tacticalPos.y * TILESIZE);
        dest.w = world2zoomedWorld(15 * TILESIZE);
        dest.h = world2zoomedWorld(10 * TILESIZE);

        renderDrawRectF(renderer, &dest, COLOR_DARKGREY);
    }

    const DuneTexture* validPlace   = nullptr;
    const DuneTexture* invalidPlace = nullptr;
    const DuneTexture* greyPlace    = nullptr;

    switch (zoom) {
        case 0: {
            validPlace   = gfx->getUIGraphic(UI_ValidPlace_Zoomlevel0);
            invalidPlace = gfx->getUIGraphic(UI_InvalidPlace_Zoomlevel0);
            greyPlace    = gfx->getUIGraphic(UI_GreyPlace_Zoomlevel0);
        } break;

        case 1: {
            validPlace   = gfx->getUIGraphic(UI_ValidPlace_Zoomlevel1);
            invalidPlace = gfx->getUIGraphic(UI_InvalidPlace_Zoomlevel1);
            greyPlace    = gfx->getUIGraphic(UI_GreyPlace_Zoomlevel1);
        } break;

        case 2:
        default: {
            validPlace   = gfx->getUIGraphic(UI_ValidPlace_Zoomlevel2);
            invalidPlace = gfx->getUIGraphic(UI_InvalidPlace_Zoomlevel2);
            greyPlace    = gfx->getUIGraphic(UI_GreyPlace_Zoomlevel2);
        } break;
    }

    if (!bCompleteMap && !pInterface_->hasChildWindow()
        && pScreenborder->isScreenCoordInsideMap(dune::globals::drawnMouseX, dune::globals::drawnMouseY)) {

        int xPos = pScreenborder->screen2MapX(dune::globals::drawnMouseX);
        int yPos = pScreenborder->screen2MapY(dune::globals::drawnMouseY);

        if (currentEditorMode_.mode_ == EditorMode::EditorMode_Terrain) {

            int halfsize = currentEditorMode_.pen_size_ / 2;

            for (int m = 0; m < mapMirror_->getSize(); m++) {

                Coord position = mapMirror_->getCoord(Coord(xPos, yPos), m);

                SDL_Rect dest;
                dest.x = pScreenborder->world2screenX((position.x - halfsize) * TILESIZE);
                dest.y = pScreenborder->world2screenY((position.y - halfsize) * TILESIZE);
                dest.w = world2zoomedWorld(currentEditorMode_.pen_size_ * TILESIZE);
                dest.h = world2zoomedWorld(currentEditorMode_.pen_size_ * TILESIZE);

                DuneDrawSelectionBox(renderer, dest);
            }

        } else if (currentEditorMode_.mode_ == EditorMode::EditorMode_Structure) {
            Coord structureSize = getStructureSize(currentEditorMode_.itemID_);

            for (int m = 0; m < mapMirror_->getSize(); m++) {

                Coord position = mapMirror_->getCoord(Coord(xPos, yPos), m, structureSize);

                for (int x = position.x; x < (position.x + structureSize.x); x++) {
                    for (int y = position.y; y < (position.y + structureSize.y); y++) {
                        const auto* image = validPlace;

                        // check if mirroring of the original (!) position is possible
                        if (!mapMirror_->mirroringPossible(Coord(xPos, yPos), structureSize)) {
                            image = invalidPlace;
                        }

                        // check all mirrored places
                        for (int k = 0; k < mapMirror_->getSize(); k++) {
                            Coord pos = mapMirror_->getCoord(Coord(x, y), k);

                            if (!map_.isInsideMap(pos.x, pos.y)
                                || isTileBlocked(pos.x, pos.y, true, (currentEditorMode_.itemID_ != Structure_Slab1))) {
                                image = invalidPlace;
                            } else if ((image != invalidPlace) && (map_(pos.x, pos.y) != Terrain_Rock)) {
                                image = greyPlace;
                            }
                        }

                        SDL_FRect drawLocation{pScreenborder->world2screenX(x * TILESIZE),
                                               pScreenborder->world2screenY(y * TILESIZE),
                                               static_cast<float>(zoomedTilesize), static_cast<float>(zoomedTilesize)};
                        Dune_RenderCopyF(renderer, image, nullptr, &drawLocation);
                    }
                }
            }
        } else if (currentEditorMode_.mode_ == EditorMode::EditorMode_Unit) {
            for (int m = 0; m < mapMirror_->getSize(); m++) {

                Coord position = mapMirror_->getCoord(Coord(xPos, yPos), m);

                const auto* image = validPlace;
                // check all mirrored places
                for (int k = 0; k < mapMirror_->getSize(); k++) {
                    Coord pos = mapMirror_->getCoord(position, k);

                    if (!map_.isInsideMap(pos.x, pos.y) || isTileBlocked(pos.x, pos.y, false, true)) {
                        image = invalidPlace;
                    }
                }
                auto drawLocation = calcDrawingRect(image, pScreenborder->world2screenX(position.x * TILESIZE),
                                                    pScreenborder->world2screenY(position.y * TILESIZE));
                Dune_RenderCopyF(renderer, image, nullptr, &drawLocation);
            }
        } else if (currentEditorMode_.mode_ == EditorMode::EditorMode_TacticalPos) {
            // draw tactical pos rectangle (the starting screen)
            if (mapInfo_.tacticalPos.isValid()) {

                SDL_FRect dest = {pScreenborder->world2screenX(xPos * TILESIZE),
                                  pScreenborder->world2screenY(yPos * TILESIZE),
                                  static_cast<float>(world2zoomedWorld(15 * TILESIZE)),
                                  static_cast<float>(world2zoomedWorld(10 * TILESIZE))};
                renderDrawRectF(renderer, &dest, COLOR_WHITE);
            }
        }
    }

    // draw selection rect for units (selection rect for structures_ is already drawn)
    if (!bCompleteMap) {
        std::vector<int> selectedUnits = getMirrorUnits(selectedUnitID_);

        for (const Unit& unit : units_) {
            if (std::ranges::find(selectedUnits, unit.id_) != selectedUnits.end()) {
                const Coord& position = unit.position_;

                const DuneTexture* selectionBox = nullptr;

                switch (dune::globals::currentZoomlevel) {
                    case 0: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel0); break;
                    case 1: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel1); break;
                    case 2:
                    default: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel2); break;
                }

                auto dest = calcDrawingRect(selectionBox,
                                            pScreenborder->world2screenX((position.x * TILESIZE) + (TILESIZE / 2)),
                                            pScreenborder->world2screenY((position.y * TILESIZE) + (TILESIZE / 2)),
                                            HAlign::Center, VAlign::Center);

                Dune_RenderCopyF(renderer, selectionBox, nullptr, &dest);
            }
        }
    }

    // draw selection rect for map items (spice bloom, special bloom or spice field)
    if (!bCompleteMap && selectedMapItemCoord_.isValid()
        && ((std::ranges::find(spiceBlooms_, selectedMapItemCoord_) != spiceBlooms_.end())
            || (std::ranges::find(specialBlooms_, selectedMapItemCoord_) != specialBlooms_.end())
            || (std::ranges::find(spiceFields_, selectedMapItemCoord_) != spiceFields_.end()))) {

        const DuneTexture* selectionBox = nullptr;

        switch (zoom) {
            case 0: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel0); break;
            case 1: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel1); break;
            case 2:
            default: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel2); break;
        }

        auto dest = calcDrawingRect(selectionBox,
                                    pScreenborder->world2screenX((selectedMapItemCoord_.x * TILESIZE) + (TILESIZE / 2)),
                                    pScreenborder->world2screenY((selectedMapItemCoord_.y * TILESIZE) + (TILESIZE / 2)),
                                    HAlign::Center, VAlign::Center);

        Dune_RenderCopyF(renderer, selectionBox, nullptr, &dest);
    }
}

void MapEditor::saveMapshot() {
    const int oldCurrentZoomlevel = dune::globals::currentZoomlevel;

    dune::globals::currentZoomlevel = 0;

    auto mapshotFilename =
        lastSaveName_.empty() ? std::filesystem::path{generateMapname()} : getBasename(lastSaveName_, true);

    mapshotFilename += ".png";

    const auto sizeX = world2zoomedWorld(map_.getSizeX() * TILESIZE);
    const auto sizeY = world2zoomedWorld(map_.getSizeY() * TILESIZE);

    const SDL_FRect board{0, 0, static_cast<float>(sizeX), static_cast<float>(sizeY)};

    ScreenBorder tmpScreenborder(board);
    tmpScreenborder.adjustScreenBorderToMapsize(map_.getSizeX(), map_.getSizeY());

    auto* const renderer = dune::globals::renderer.get();

    const auto renderTarget =
        sdl2::texture_ptr{SDL_CreateTexture(renderer, SCREEN_FORMAT, SDL_TEXTUREACCESS_TARGET, sizeX, sizeY)};
    if (renderTarget == nullptr) {
        sdl2::log_info("SDL_CreateTexture() failed: %s", SDL_GetError());
        dune::globals::currentZoomlevel = oldCurrentZoomlevel;
        return;
    }

    auto* const oldRenderTarget = SDL_GetRenderTarget(renderer);
    if (SDL_SetRenderTarget(renderer, renderTarget.get()) != 0) {
        sdl2::log_info("SDL_SetRenderTarget() failed: %s", SDL_GetError());
        SDL_SetRenderTarget(renderer, oldRenderTarget);
        dune::globals::currentZoomlevel = oldCurrentZoomlevel;
        return;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    drawMap(&tmpScreenborder, true);

    const auto pMapshotSurface = renderReadSurface(renderer);
    SavePNG(pMapshotSurface.get(), mapshotFilename.u8string().c_str());

    SDL_SetRenderTarget(renderer, oldRenderTarget);

    dune::globals::currentZoomlevel = oldCurrentZoomlevel;
}
