#include <INIMap/INIMapEditorLoader.h>

#include <FileClasses/FileManager.h>

#include <AITeamInfo.h>
#include <MapEditor/MapEditor.h>
#include <MapEditor/ReinforcementInfo.h>

#include <MapSeed.h>
#include <ScreenBorder.h>

#include <fmt/printf.h>
#include <misc/exceptions.h>

#include <globals.h>
#include <sand.h>

#include <algorithm>
#include <limits>
#include <utility>

using namespace std::literals;

INIMapEditorLoader::INIMapEditorLoader(MapEditor* pMapEditor, INIMap::inifile_ptr pINIFile)
    : INIMap(std::move(pINIFile)), pMapEditor_(pMapEditor) {
    load();
}

INIMapEditorLoader::~INIMapEditorLoader() = default;

/**
    Loads a map from an INI-File.
*/
void INIMapEditorLoader::load() {
    checkFeatures();

    loadMap();
    loadHouses();
    loadUnits();
    loadStructures();
    loadReinforcements();
    loadChoam();
    loadAITeams();

    pMapEditor_->informPlayersChanged();
}

/**
    This method loads the game map. This is based on the [MAP] section in the INI file.
*/
void INIMapEditorLoader::loadMap() {

    version_ = inifile_->getIntValue("BASIC", "Version", 1);

    if (version_ < 2) {
        // we have all houses fixed
        for (auto& player : pMapEditor_->getPlayers()) {
            player.bAnyHouse_ = false;
        }
    }

    pMapEditor_->mapInfo_.author  = inifile_->getStringValue("BASIC", "Author");
    pMapEditor_->mapInfo_.license = inifile_->getStringValue("BASIC", "License");

    pMapEditor_->mapInfo_.winFlags  = inifile_->getIntValue("BASIC", "WinFlags", 3);
    pMapEditor_->mapInfo_.loseFlags = inifile_->getIntValue("BASIC", "LoseFlags", 1);

    pMapEditor_->mapInfo_.losePicture  = inifile_->getStringValue("BASIC", "LosePicture");
    pMapEditor_->mapInfo_.winPicture   = inifile_->getStringValue("BASIC", "WinPicture");
    pMapEditor_->mapInfo_.briefPicture = inifile_->getStringValue("BASIC", "BriefPicture");

    pMapEditor_->mapInfo_.timeout   = inifile_->getIntValue("BASIC", "TimeOut", 0);
    pMapEditor_->mapInfo_.techLevel = inifile_->getIntValue("BASIC", "TechLevel", 0);

    if (version_ < 2) {
        const int mapscale = inifile_->getIntValue("BASIC", "MapScale", 0);

        pMapEditor_->mapInfo_.mapSeed = inifile_->getIntValue("MAP", "Seed", 0);

        pMapEditor_->map_ = createMapWithSeed(pMapEditor_->mapInfo_.mapSeed, mapscale);

        switch (mapscale) {
            case 0: {
                sizeX_          = 62;
                sizeY_          = 62;
                logicalOffsetX_ = 1;
                logicalOffsetY_ = 1;
            } break;

            case 1: {
                sizeX_          = 32;
                sizeY_          = 32;
                logicalOffsetX_ = 16;
                logicalOffsetY_ = 16;
            } break;

            case 2: {
                sizeX_          = 21;
                sizeY_          = 21;
                logicalOffsetX_ = 11;
                logicalOffsetY_ = 11;
            } break;

            default: {
                logError(inifile_->getLineNumber("BASIC", "MapScale"), "Unknown MapScale '%d'!", mapscale);
            } break;
        }

        logicalSizeX_ = 64;
        logicalSizeY_ = 64;

        const int cursorPos               = inifile_->getIntValue("BASIC", "CursorPos", 0);
        pMapEditor_->mapInfo_.cursorPos   = Coord(getXPos(cursorPos), getYPos(cursorPos));
        const int tacticalPos             = inifile_->getIntValue("BASIC", "TacticalPos", 0);
        pMapEditor_->mapInfo_.tacticalPos = Coord(getXPos(tacticalPos), getYPos(tacticalPos));

        // field, spice bloom and special bloom

        const std::string BloomString = inifile_->getStringValue("MAP", "Bloom");
        if (!BloomString.empty()) {
            const auto BloomPositions = splitStringToStringVector(BloomString);

            for (const auto& BloomPosition : BloomPositions) {
                // set bloom
                int BloomPos = 0;
                if (parseString(BloomPosition, BloomPos)) {
                    int xpos = getXPos(BloomPos);
                    int ypos = getYPos(BloomPos);
                    pMapEditor_->getSpiceBlooms().emplace_back(xpos, ypos);
                } else {
                    logWarning(inifile_->getLineNumber("MAP", "Bloom"), "Invalid spice bloom position: '%s'!",
                               BloomPosition);
                }
            }
        }

        const auto SpecialString = inifile_->getStringValue("MAP", "Special");
        if (!SpecialString.empty()) {
            const auto SpecialPositions = splitStringToStringVector(SpecialString);

            for (auto& SpecialPosition : SpecialPositions) {
                // set special
                int SpecialPos = 0;
                if (parseString(SpecialPosition, SpecialPos)) {
                    int xpos = getXPos(SpecialPos);
                    int ypos = getYPos(SpecialPos);
                    pMapEditor_->getSpecialBlooms().emplace_back(xpos, ypos);
                } else {
                    logWarning(inifile_->getLineNumber("MAP", "Special"), "Invalid special bloom position: '%s'!",
                               SpecialPosition);
                }
            }
        }

        const std::string FieldString = inifile_->getStringValue("MAP", "Field");
        if (!FieldString.empty()) {
            const std::vector<std::string> FieldPositions = splitStringToStringVector(FieldString);

            for (const auto& FieldPosition : FieldPositions) {
                // set bloom
                int FieldPos = 0;
                if (parseString(FieldPosition, FieldPos)) {
                    int xpos = getXPos(FieldPos);
                    int ypos = getYPos(FieldPos);
                    pMapEditor_->getSpiceFields().emplace_back(xpos, ypos);
                } else {
                    logWarning(inifile_->getLineNumber("MAP", "Field"), "Invalid spice field position: '%s'!",
                               FieldPosition);
                }
            }
        }

    } else {
        // new map format with saved map

        pMapEditor_->mapInfo_.cursorPos   = Coord::Invalid();
        pMapEditor_->mapInfo_.tacticalPos = Coord::Invalid();
        pMapEditor_->mapInfo_.mapSeed     = INVALID;

        const int sizeX = inifile_->getIntValue("MAP", "SizeX", 0);
        const int sizeY = inifile_->getIntValue("MAP", "SizeY", 0);

        pMapEditor_->map_ = MapData(sizeX, sizeY);

        logicalSizeX_   = sizeX;
        logicalSizeY_   = sizeY;
        logicalOffsetX_ = 0;
        logicalOffsetY_ = 0;

        for (int y = 0; y < sizeY; y++) {
            const auto rowKey = fmt::sprintf("%.3d", y);

            if (!inifile_->hasKey("MAP", rowKey)) {
                logWarning(inifile_->getLineNumber("MAP"), "Map row %d does not exist!", y);
                continue;
            }

            const auto rowString = inifile_->getStringValue("MAP", rowKey);

            if (std::cmp_greater(rowString.size(), std::numeric_limits<int>::max())) {
                logWarning(inifile_->getLineNumber("MAP"), "Map row %d is too long!", y);
                continue;
            }

            auto rowLength = static_cast<int>(rowString.size());

            if (rowLength < sizeX) {
                logWarning(inifile_->getLineNumber("MAP", rowKey), "Map row %d is not long enough!", y);
            } else if (rowLength > sizeX) {
                logWarning(inifile_->getLineNumber("MAP", rowKey), "Map row %d is too long!", y);
                rowLength = sizeX;
            }

            for (auto x = 0; x < rowLength; x++) {
                auto type = TERRAINTYPE::Terrain_Sand;

                switch (rowString.at(x)) {
                    case '-': {
                        // Normal sand
                        type = TERRAINTYPE::Terrain_Sand;
                    } break;

                    case '^': {
                        // Sand dunes
                        type = TERRAINTYPE::Terrain_Dunes;
                    } break;

                    case '~': {
                        // Spice
                        type = TERRAINTYPE::Terrain_Spice;
                    } break;

                    case '+': {
                        // Thick spice
                        type = TERRAINTYPE::Terrain_ThickSpice;
                    } break;

                    case '%': {
                        // Rock
                        type = TERRAINTYPE::Terrain_Rock;
                    } break;

                    case '@': {
                        // Mountain
                        type = TERRAINTYPE::Terrain_Mountain;
                    } break;

                    case 'O': {
                        // Spice Bloom
                        type = TERRAINTYPE::Terrain_SpiceBloom;
                    } break;

                    case 'Q': {
                        // Special Bloom
                        type = TERRAINTYPE::Terrain_SpecialBloom;
                    } break;

                    default: {
                        logWarning(inifile_->getLineNumber("MAP", rowKey),
                                   "Unknown map tile type '{}' in map tile ({}, {})!", rowString.at(x), x, y);
                        type = TERRAINTYPE::Terrain_Sand;
                    } break;
                }

                pMapEditor_->map_(x, y) = type;
            }
        }
    }

    dune::globals::screenborder->adjustScreenBorderToMapsize(pMapEditor_->map_.getSizeX(),
                                                             pMapEditor_->map_.getSizeY());
}

/**
    This method loads the houses on the map specified by the various house sections in the INI file ([Atreides],
   [Ordos], [Harkonnen]).
*/
void INIMapEditorLoader::loadHouses() {
    for (int houseID = 0; houseID < NUM_HOUSES; houseID++) {
        std::string houseName = getHouseNameByNumber(static_cast<HOUSETYPE>(houseID));

        if (inifile_->hasSection(houseName)) {
            auto& player = pMapEditor_->getPlayers()[houseID];

            player.bActive_   = true;
            player.bAnyHouse_ = false;
            player.credits_   = inifile_->getIntValue(houseName, "Credits", 0);
            player.quota_     = inifile_->getIntValue(houseName, "Quota", 0);
            player.maxunit_   = inifile_->hasKey(houseName, "MaxUnit") ? inifile_->getIntValue(houseName, "MaxUnit", 0)
                                                                       : inifile_->getIntValue(houseName, "MaxUnits", 0);

            std::string brain = inifile_->getStringValue(houseName, "Brain", "");
            if (!brain.empty()) {
                player.brain_ = brain;
            }
        }
    }

    for (int i = 1; i <= NUM_HOUSES; i++) {
        std::string sectionname = "player" + std::to_string(i);
        if (inifile_->hasSection(sectionname)) {
            for (int houseID = 0; houseID < NUM_HOUSES; houseID++) {
                auto& player = pMapEditor_->getPlayers()[houseID];

                if (!player.bActive_) {
                    convertToLower(sectionname);
                    housename2house_[sectionname] = static_cast<HOUSETYPE>(houseID);

                    player.bActive_   = true;
                    player.bAnyHouse_ = true;
                    player.credits_   = inifile_->getIntValue(sectionname, "Credits", 0);
                    player.quota_     = inifile_->getIntValue(sectionname, "Quota", 0);
                    player.maxunit_   = inifile_->hasKey(sectionname, "MaxUnit")
                                          ? inifile_->getIntValue(sectionname, "MaxUnit", 0)
                                          : inifile_->getIntValue(sectionname, "MaxUnits", 0);

                    std::string brain = inifile_->getStringValue(sectionname, "Brain", "");
                    if (!brain.empty()) {
                        player.brain_ = brain;
                    }

                    break;
                }
            }
        }
    }
}

/**
    This method loads the choam section of the INI file
*/
void INIMapEditorLoader::loadChoam() {
    static constexpr auto sectionname = "CHOAM"sv;

    if (!inifile_->hasSection(sectionname)) {
        return;
    }

    for (const auto& key : inifile_->keys(sectionname)) {
        const auto UnitStr = key.getKeyName();

        auto unitID = getItemIDByName(UnitStr);
        if ((unitID == ItemID_Invalid) || !isUnit(unitID)) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid unit string: '%s'", UnitStr);
            continue;
        }

        int num = key.getValue(-2);
        if (num == -2) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid choam number!");
            continue;
        }

        if (num == -1) {
            num = 0;
        }

        pMapEditor_->getChoam()[unitID] = num;
    }
}

/**
    This method loads the units specified by the [Units] section.
*/
void INIMapEditorLoader::loadUnits() {
    static constexpr auto sectionname = "UNITS"sv;

    if (!inifile_->hasSection(sectionname)) {
        return;
    }

    for (const auto& key : inifile_->keys(sectionname)) {
        if (key.getKeyName().find("ID") == 0) {
            int unitID = 0;
            parseString(key.getKeyName().substr(2), unitID);

            std::string HouseStr, UnitStr, health, PosStr, rotation, mode;
            splitString(key.getStringView(), HouseStr, UnitStr, health, PosStr, rotation, mode);

            HOUSETYPE houseID = getHouseID(HouseStr);
            if (houseID == HOUSETYPE::HOUSE_UNUSED) {
                // skip unit for unused house
                continue;
            }
            if (houseID == HOUSETYPE::HOUSE_INVALID) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid house string for '{}': '{}'!", UnitStr, HouseStr);
                continue;
            }

            int pos = 0;
            if (!parseString(PosStr, pos) || (pos < 0)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid position string for '{}': '{}'!", UnitStr, PosStr);
                continue;
            }

            int int_angle = 0;
            if (!parseString(rotation, int_angle) || (int_angle < 0) || (int_angle > 255)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid rotation string: '{}'!", rotation);
                int_angle = 64;
            }
            int_angle  = (int_angle + 16) / 32;
            int_angle  = NUM_ANGLES - int_angle + 2;
            auto angle = normalizeAngle(static_cast<ANGLETYPE>(int_angle));

            ItemID_enum itemID = getItemIDByName(UnitStr);
            if ((itemID == ItemID_Invalid) || !isUnit(itemID)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid unit string: '{}'!", UnitStr);
                continue;
            }

            int iHealth = 0;
            if (!parseString(health, iHealth) || (iHealth < 0) || (iHealth > 256)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid health string: '{}'!", health);
                iHealth = 256;
            }

            ATTACKMODE attackmode = getAttackModeByName(mode);
            if (attackmode == ATTACKMODE_INVALID) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid attackmode string: '{}'!", mode);
                attackmode = AREAGUARD;
            }

            if (itemID == Unit_Soldier || itemID == Unit_Saboteur || itemID == Unit_Trooper || itemID == Unit_Infantry
                || itemID == Unit_Troopers) {
                if (angle == ANGLETYPE::UP) {
                    angle = ANGLETYPE::UP;
                } else if (angle == ANGLETYPE::DOWN) {
                    angle = ANGLETYPE::DOWN;
                } else if (angle == ANGLETYPE::LEFTUP || angle == ANGLETYPE::LEFTDOWN || angle == ANGLETYPE::LEFT) {
                    angle = ANGLETYPE::LEFT;
                } else /*(angle == ANGLETYPE::RIGHT)*/ {
                    angle = ANGLETYPE::RIGHT;
                }
            }

            pMapEditor_->units_.emplace_back(unitID, houseID, itemID, iHealth, Coord(getXPos(pos), getYPos(pos)), angle,
                                             attackmode);

        } else {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid unit key: '{}'!",
                       key.getKeyName());
        }
    }
}

/**
    This method loads the structures specified by the [Structures] section.
*/
void INIMapEditorLoader::loadStructures() {
    static constexpr auto sectionname = "STRUCTURES"sv;

    if (!inifile_->hasSection(sectionname)) {
        return;
    }

    int genID = -2;

    for (const auto& key : inifile_->keys(sectionname)) {
        const auto tmpkey = key.getKeyName();
        const auto tmp    = key.getStringView();

        if (tmpkey.compare(0, 3, "GEN") == 0) {
            // Gen Object/Structure
            const auto PosStr = tmpkey.substr(3, tmpkey.size() - 3);
            int pos           = 0;
            if (!parseString(PosStr, pos) || (pos < 0)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid position string: '{}'!",
                           PosStr);
                continue;
            }

            std::string HouseStr, BuildingStr;
            splitString(tmp, HouseStr, BuildingStr);

            const auto houseID = getHouseID(HouseStr);
            if (houseID == HOUSETYPE::HOUSE_UNUSED) {
                // skip structure for unused house
                continue;
            }
            if (houseID == HOUSETYPE::HOUSE_INVALID) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid house string for '{}': '{}'!", BuildingStr, HouseStr);
                continue;
            }

            auto itemID = getItemIDByName(BuildingStr);

            if ((itemID == Structure_Slab1) || (itemID == Structure_Slab4) || (itemID == Structure_Wall)) {
                pMapEditor_->structures_.emplace_back(genID, houseID, itemID, 256, Coord(getXPos(pos), getYPos(pos)));
                genID--;
            } else {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid building string: '{}' for GEN-Placement!", BuildingStr);
            }

        } else if (tmpkey.compare(0, 2, "ID") == 0) {
            // other structure
            int structureID = 0;
            parseString(tmpkey.substr(2), structureID);

            std::string HouseStr, BuildingStr, health, PosStr;
            splitString(tmp, HouseStr, BuildingStr, health, PosStr);

            int pos = 0;
            if (!parseString(PosStr, pos) || (pos < 0)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid position string for '{}': '{}'!", BuildingStr, PosStr);
                continue;
            }

            const auto houseID = getHouseID(HouseStr);
            if (houseID == HOUSETYPE::HOUSE_UNUSED) {
                // skip structure for unused house
                continue;
            }
            if (houseID == HOUSETYPE::HOUSE_INVALID) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid house string for '{}': '{}'!", BuildingStr, HouseStr);
                continue;
            }

            auto iHealth = 0;
            if (!parseString(health, iHealth) || (iHealth < 0) || (iHealth > 256)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid health string: '{}'!",
                           health);
                iHealth = 256;
            }

            auto itemID = getItemIDByName(BuildingStr);

            if ((itemID == ItemID_Invalid) || !isStructure(itemID)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid building string: '{}'!",
                           BuildingStr);
                continue;
            }

            pMapEditor_->structures_.emplace_back(structureID, houseID, itemID, iHealth,
                                                  Coord(getXPos(pos), getYPos(pos)));
        } else {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid structure key: '{}'!", tmpkey);
        }
    }
}

/**
    This method loads the reinforcements from the [REINFORCEMENTS] section.
*/
void INIMapEditorLoader::loadReinforcements() {
    static constexpr auto sectionname = "REINFORCEMENTS"sv;

    if (!inifile_->hasSection(sectionname)) {
        return;
    }

    for (const auto& key : inifile_->keys(sectionname)) {
        std::string strHouseName;
        std::string strUnitName;
        std::string strDropLocation;
        std::string strTime;
        std::string strPlus;

        // N.b. valid reinforcements strings are of one of:
        // "3=Sardaukar,Troopers,Enemybase,20" (nonrepeating reinforcement at 20 minutes mark)
        // "3=Sardaukar,Troopers,Enemybase,20+" (repeating reinforcement every 20 minutes)
        // "3=Sardaukar,Troopers,Enemybase,20,+" (same as above, just with an alternate syntax)
        if (!splitString(key.getStringView(), strHouseName, strUnitName, strDropLocation, strTime)) {
            if (!splitString(key.getStringView(), strHouseName, strUnitName, strDropLocation, strTime, strPlus)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid reinforcement string: {} = {}", key.getKeyName(), key.getStringView());
                continue;
            }
        }

        const auto houseID = getHouseID(strHouseName);
        if (houseID == HOUSETYPE::HOUSE_UNUSED) {
            // skip reinforcement for unused house
            continue;
        }
        if (houseID == HOUSETYPE::HOUSE_INVALID) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid house string: '{}'!",
                       strHouseName);
            continue;
        }

        const auto unitID = getItemIDByName(strUnitName);
        if ((unitID == ItemID_Invalid) || !isUnit(unitID)) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid unit string: '{}'!",
                       strUnitName);
            continue;
        }

        auto dropLocation = getDropLocationByName(strDropLocation);
        if (dropLocation == DropLocation::Drop_Invalid) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid drop location string: '{}'!",
                       strDropLocation);
            dropLocation = DropLocation::Drop_Homebase;
        }

        auto bRepeat = (strPlus == "+");
        if (strTime.rfind('+') == (strTime.length() - 1)) {
            strTime.resize(strTime.length() - 1);
            bRepeat = true;
        }

        auto droptime = 0;
        if (!parseString(strTime, droptime)) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                       "Invalid drop time string: '{}'!", strTime);
            continue;
        }

        pMapEditor_->getReinforcements().emplace_back(houseID, unitID, dropLocation, droptime, bRepeat);
    }
}

/**
    This method loads the AI teams from the [TEAMS] section.
*/
void INIMapEditorLoader::loadAITeams() {
    static constexpr auto sectionname = "TEAMS"sv;

    if (!inifile_->hasSection(sectionname)) {
        return;
    }

    for (const auto& key : inifile_->keys(sectionname)) {
        std::string strHouseName;
        std::string strAITeamBehavior;
        std::string strAITeamType;
        std::string strMinUnits;
        std::string strMaxUnits;

        if (!splitString(key.getStringView(), strHouseName, strAITeamBehavior, strAITeamType, strMinUnits,
                         strMaxUnits)) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid teams string: {} = {}",
                       key.getKeyName(), key.getStringView());
            continue;
        }

        const auto houseID = getHouseID(strHouseName);
        if (houseID == HOUSETYPE::HOUSE_UNUSED) {
            // skip reinforcement for unused house
            continue;
        }
        if (houseID == HOUSETYPE::HOUSE_INVALID) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid house string: '{}'!",
                       strHouseName);
            continue;
        }

        auto aiTeamBehavior = getAITeamBehaviorByName(strAITeamBehavior);
        if (aiTeamBehavior == AITeamBehavior::AITeamBehavior_Invalid) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid team behavior string: '{}'!",
                       strAITeamBehavior);
            aiTeamBehavior = AITeamBehavior::AITeamBehavior_Normal;
        }

        auto aiTeamType = getAITeamTypeByName(strAITeamType);
        if (aiTeamType == AITeamType::AITeamType_Invalid) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid team type string: '{}'!",
                       strAITeamType);
            aiTeamType = AITeamType::AITeamType_Foot;
        }

        auto minUnits = 0;
        if (!parseString(strMinUnits, minUnits)) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid min units string: '{}'!",
                       strMinUnits);
            continue;
        }

        auto maxUnits = 0;
        if (!parseString(strMaxUnits, maxUnits)) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid max units string: '{}'!",
                       strMaxUnits);
            continue;
        }

        pMapEditor_->getAITeams().emplace_back(houseID, aiTeamBehavior, aiTeamType, minUnits, maxUnits);
    }
}

HOUSETYPE INIMapEditorLoader::getHouseID(std::string_view name) {
    const auto lowerName = strToLower(name);

    const auto it = housename2house_.find(lowerName);

    if (it != housename2house_.end())
        return it->second;

    return getHouseByName(lowerName);
}
