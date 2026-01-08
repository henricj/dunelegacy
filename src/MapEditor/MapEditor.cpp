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
#include <FileClasses/TextManager.h>

#include "misc/DrawingRectHelper.h"
#include "misc/dune_sdl.h"
#include <misc/FileSystem.h>

#include "Renderer/DuneRenderer.h"

#include "ScreenBorder.h"
#include <globals.h>
#include <sand.h>

#include <config.h>

#include <fmt/core.h>
#include <fmt/printf.h>

#include <algorithm>

MapEditor::MapEditor() {
    selectedMapItemCoord_.invalidate();

    dune::globals::currentZoomlevel = dune::globals::settings.video.preferredZoomLevel;

    const auto* const gfx = dune::globals::pGFXManager.get();

    sideBarPos_ = as_rect(calcAlignedDrawingRect(gfx->getUIGraphic(UI_SideBar), HAlign::Right, VAlign::Top));
    topBarPos_  = as_rect(calcAlignedDrawingRect(gfx->getUIGraphic(UI_TopBar), HAlign::Left, VAlign::Top));
    bottomBarPos_ =
        as_rect(calcAlignedDrawingRect(gfx->getUIGraphic(UI_MapEditor_BottomBar), HAlign::Left, VAlign::Bottom));

    SDL_FRect gameBoardRect{0,
                            static_cast<float>(topBarPos_.h),
                            static_cast<float>(sideBarPos_.x),
                            static_cast<float>(getRendererHeight() - topBarPos_.h - bottomBarPos_.h)};

    dune::globals::screenborder = std::make_unique<ScreenBorder>(gameBoardRect);

    setMap(MapData(128, 128, TERRAINTYPE::Terrain_Sand), MapInfo());
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

    return fmt::format("{}P - {}x{} - {}", numPlayers, map_.getSizeX(), map_.getSizeY(), _("New Map"));
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
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_HARKONNEN),
                              HOUSETYPE::HOUSE_HARKONNEN,
                              HOUSETYPE::HOUSE_HARKONNEN,
                              true,
                              false,
                              "Human",
                              25);
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ATREIDES),
                              HOUSETYPE::HOUSE_ATREIDES,
                              HOUSETYPE::HOUSE_ATREIDES,
                              true,
                              false,
                              "CPU",
                              25);
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ORDOS),
                              HOUSETYPE::HOUSE_ORDOS,
                              HOUSETYPE::HOUSE_ORDOS,
                              true,
                              false,
                              "CPU",
                              25);
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_FREMEN),
                              HOUSETYPE::HOUSE_FREMEN,
                              HOUSETYPE::HOUSE_FREMEN,
                              false,
                              false,
                              "CPU",
                              25);
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_SARDAUKAR),
                              HOUSETYPE::HOUSE_SARDAUKAR,
                              HOUSETYPE::HOUSE_SARDAUKAR,
                              true,
                              false,
                              "CPU",
                              25);
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_MERCENARY),
                              HOUSETYPE::HOUSE_MERCENARY,
                              HOUSETYPE::HOUSE_MERCENARY,
                              false,
                              false,
                              "CPU",
                              25);
    } else {
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_HARKONNEN),
                              HOUSETYPE::HOUSE_HARKONNEN,
                              HOUSETYPE::HOUSE_HARKONNEN,
                              true,
                              true,
                              "Team1");
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ATREIDES),
                              HOUSETYPE::HOUSE_ATREIDES,
                              HOUSETYPE::HOUSE_ATREIDES,
                              true,
                              true,
                              "Team2");
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ORDOS),
                              HOUSETYPE::HOUSE_ORDOS,
                              HOUSETYPE::HOUSE_ORDOS,
                              true,
                              true,
                              "Team3");
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_FREMEN),
                              HOUSETYPE::HOUSE_FREMEN,
                              HOUSETYPE::HOUSE_FREMEN,
                              false,
                              false,
                              "Team4");
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_SARDAUKAR),
                              HOUSETYPE::HOUSE_SARDAUKAR,
                              HOUSETYPE::HOUSE_SARDAUKAR,
                              true,
                              true,
                              "Team5");
        players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_MERCENARY),
                              HOUSETYPE::HOUSE_MERCENARY,
                              HOUSETYPE::HOUSE_MERCENARY,
                              false,
                              false,
                              "Team6");
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

TERRAINTYPE MapEditor::getTerrain(int x, int y) const {
    TERRAINTYPE terrainType = map_(x, y);

    if (terrainType == TERRAINTYPE::Terrain_Sand) {
        // Check for spice blooms
        if (std::ranges::find(spiceBlooms_, Coord(x, y)) != spiceBlooms_.end()) {
            return TERRAINTYPE::Terrain_SpiceBloom;
        }

        // Check for special blooms
        if (std::ranges::find(specialBlooms_, Coord(x, y)) != specialBlooms_.end()) {
            return TERRAINTYPE::Terrain_SpecialBloom;
        }

        // Check for spice fields
        if (std::ranges::find(spiceFields_, Coord(x, y)) != spiceFields_.end()) {
            return TERRAINTYPE::Terrain_Spice;
        }
    }

    // Check for slabs
    for (const Structure& structure : structures_) {
        if (structure.itemID_ == Structure_Slab1) {
            if (structure.position_.x == x && structure.position_.y == y) {
                return TERRAINTYPE::Terrain_Slab;
            }
        } else if (structure.itemID_ == Structure_Slab4) {
            if (x >= structure.position_.x && x < structure.position_.x + 2 && y >= structure.position_.y
                && y < structure.position_.y + 2) {
                return TERRAINTYPE::Terrain_Slab;
            }
        }
    }

    return terrainType;
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

    players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_HARKONNEN),
                          HOUSETYPE::HOUSE_HARKONNEN,
                          HOUSETYPE::HOUSE_HARKONNEN,
                          false,
                          true,
                          "Team1");
    players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ATREIDES),
                          HOUSETYPE::HOUSE_ATREIDES,
                          HOUSETYPE::HOUSE_ATREIDES,
                          false,
                          true,
                          "Team2");
    players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_ORDOS),
                          HOUSETYPE::HOUSE_ORDOS,
                          HOUSETYPE::HOUSE_ORDOS,
                          false,
                          true,
                          "Team3");
    players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_FREMEN),
                          HOUSETYPE::HOUSE_FREMEN,
                          HOUSETYPE::HOUSE_FREMEN,
                          false,
                          false,
                          "Team4");
    players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_SARDAUKAR),
                          HOUSETYPE::HOUSE_SARDAUKAR,
                          HOUSETYPE::HOUSE_SARDAUKAR,
                          false,
                          false,
                          "Team5");
    players_.emplace_back(getHouseNameByNumber(HOUSETYPE::HOUSE_MERCENARY),
                          HOUSETYPE::HOUSE_MERCENARY,
                          HOUSETYPE::HOUSE_MERCENARY,
                          false,
                          false,
                          "Team6");

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

    int logicalSizeX   = 0;
    int logicalOffsetX = 0;
    int logicalOffsetY = 0;

    if (version < 2) {
        logicalSizeX = 64;

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
        logicalSizeX   = map_.getSizeX();
        logicalOffsetX = logicalOffsetY = 0;

        loadedINIFile_->clearSection("MAP");
        loadedINIFile_->setIntValue("MAP", "SizeX", map_.getSizeX());
        loadedINIFile_->setIntValue("MAP", "SizeY", map_.getSizeY());

        for (int y = 0; y < map_.getSizeY(); y++) {
            std::string rowKey = fmt::sprintf("%.3d", y);

            std::string row;
            for (int x = 0; x < map_.getSizeX(); x++) {
                switch (map_(x, y)) {

                    case TERRAINTYPE::Terrain_Dunes: {
                        // Sand dunes
                        row += '^';
                    } break;

                    case TERRAINTYPE::Terrain_Spice: {
                        // Spice
                        row += '~';
                    } break;

                    case TERRAINTYPE::Terrain_ThickSpice: {
                        // Thick spice
                        row += '+';
                    } break;

                    case TERRAINTYPE::Terrain_Rock: {
                        // Rock
                        row += '%';
                    } break;

                    case TERRAINTYPE::Terrain_Mountain: {
                        // Mountain
                        row += '@';
                    } break;

                    case TERRAINTYPE::Terrain_SpiceBloom: {
                        // Spice Bloom
                        row += 'O';
                    } break;

                    case TERRAINTYPE::Terrain_SpecialBloom: {
                        // Special Bloom
                        row += 'Q';
                    } break;

                    case TERRAINTYPE::Terrain_Sand:
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
    for (const auto& unit : units_) {
        if (unit.itemID_ < ItemID_enum::ItemID_FirstID || unit.itemID_ > ItemID_enum::ItemID_LastID)
            continue;

        std::string unitKey = fmt::sprintf("ID%.3d", unit.id_);

        int position = (logicalOffsetY + unit.position_.y) * logicalSizeX + (logicalOffsetX + unit.position_.x);

        int angle_ = static_cast<int>(unit.angle_);

        angle_ = (((NUM_ANGLES - angle_) + 2) % NUM_ANGLES) * 32;

        const auto unitValue = fmt::format("{},{},{},{},{},{}",
                                           house2housename[static_cast<int>(unit.house_)],
                                           getItemNameByID(unit.itemID_),
                                           unit.health_,
                                           position,
                                           angle_,
                                           getAttackModeNameByMode(unit.attack_mode_));

        loadedINIFile_->setStringValue("UNITS", unitKey, unitValue, false);
    }

    loadedINIFile_->clearSection("STRUCTURES");
    for (const auto& structure : structures_) {
        int position =
            (logicalOffsetY + structure.position_.y) * logicalSizeX + (logicalOffsetX + structure.position_.x);

        if ((structure.itemID_ == Structure_Slab1) || (structure.itemID_ == Structure_Slab4)
            || (structure.itemID_ == Structure_Wall)) {
            std::string structureKey = fmt::sprintf("GEN%.3d", position);

            const auto structureValue = fmt::format(
                "{},{}", house2housename[static_cast<int>(structure.house_)], getItemNameByID(structure.itemID_));

            loadedINIFile_->setStringValue("STRUCTURES", structureKey, structureValue, false);

        } else {

            const auto structureKey = fmt::sprintf("ID%.3d", structure.id_);

            const auto structureValue = fmt::format("{},{},{},{}",
                                                    house2housename[static_cast<int>(structure.house_)],
                                                    getItemNameByID(structure.itemID_),
                                                    structure.health_,
                                                    position);

            loadedINIFile_->setStringValue("STRUCTURES", structureKey, structureValue, false);
        }
    }

    if (reinforcements_.empty()) {
        loadedINIFile_->removeSection("REINFORCEMENTS");
    } else {
        loadedINIFile_->clearSection("REINFORCEMENTS");

        // we start at 0 for version 1 maps if we have 16 entries to not overflow the table
        int currentIndex = ((getMapVersion() < 2) && (reinforcements_.size() >= 16)) ? 0 : 1;
        for (const auto& reinforcement : reinforcements_) {
            auto value = fmt::format("{},{},{},{}",
                                     house2housename[static_cast<int>(reinforcement.houseID)],
                                     getItemNameByID(reinforcement.unitID),
                                     getDropLocationNameByID(reinforcement.dropLocation),
                                     reinforcement.droptime);
            if (reinforcement.bRepeat) {
                value += ",+";
            }
            loadedINIFile_->setStringValue("REINFORCEMENTS", std::to_string(currentIndex), value, false);
            currentIndex++;
        }
    }

    if (!loadedINIFile_->saveChangesTo(filepath, getMapVersion() < 2)) {
        sdl2::log_error("Unable to save configuration file {}", filepath.string());
    }

    lastSaveName_          = filepath;
    bChangedSinceLastSave_ = false;
}
