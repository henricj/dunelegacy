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

using namespace std::literals;

INIMapEditorLoader::INIMapEditorLoader(MapEditor* pMapEditor, INIMap::inifile_ptr pINIFile)
    : INIMap(std::move(pINIFile)), pMapEditor(pMapEditor) {
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

    pMapEditor->informPlayersChanged();
}

/**
    This method loads the game map. This is based on the [MAP] section in the INI file.
*/
void INIMapEditorLoader::loadMap() {

    version = inifile->getIntValue("BASIC", "Version", 1);

    if (version < 2) {
        // we have all houses fixed
        for (auto& player : pMapEditor->getPlayers()) {
            player.bAnyHouse = false;
        }
    }

    pMapEditor->mapInfo.author  = inifile->getStringValue("BASIC", "Author");
    pMapEditor->mapInfo.license = inifile->getStringValue("BASIC", "License");

    pMapEditor->mapInfo.winFlags  = inifile->getIntValue("BASIC", "WinFlags", 3);
    pMapEditor->mapInfo.loseFlags = inifile->getIntValue("BASIC", "LoseFlags", 1);

    pMapEditor->mapInfo.losePicture  = inifile->getStringValue("BASIC", "LosePicture");
    pMapEditor->mapInfo.winPicture   = inifile->getStringValue("BASIC", "WinPicture");
    pMapEditor->mapInfo.briefPicture = inifile->getStringValue("BASIC", "BriefPicture");

    pMapEditor->mapInfo.timeout   = inifile->getIntValue("BASIC", "TimeOut", 0);
    pMapEditor->mapInfo.techLevel = inifile->getIntValue("BASIC", "TechLevel", 0);

    if (version < 2) {
        const int mapscale = inifile->getIntValue("BASIC", "MapScale", 0);

        pMapEditor->mapInfo.mapSeed = inifile->getIntValue("MAP", "Seed", 0);

        pMapEditor->map = createMapWithSeed(pMapEditor->mapInfo.mapSeed, mapscale);

        switch (mapscale) {
            case 0: {
                sizeX          = 62;
                sizeY          = 62;
                logicalOffsetX = 1;
                logicalOffsetY = 1;
            } break;

            case 1: {
                sizeX          = 32;
                sizeY          = 32;
                logicalOffsetX = 16;
                logicalOffsetY = 16;
            } break;

            case 2: {
                sizeX          = 21;
                sizeY          = 21;
                logicalOffsetX = 11;
                logicalOffsetY = 11;
            } break;

            default: {
                logError(inifile->getLineNumber("BASIC", "MapScale"), "Unknown MapScale '%d'!", mapscale);
            } break;
        }

        logicalSizeX = 64;
        logicalSizeY = 64;

        const int cursorPos             = inifile->getIntValue("BASIC", "CursorPos", 0);
        pMapEditor->mapInfo.cursorPos   = Coord(getXPos(cursorPos), getYPos(cursorPos));
        const int tacticalPos           = inifile->getIntValue("BASIC", "TacticalPos", 0);
        pMapEditor->mapInfo.tacticalPos = Coord(getXPos(tacticalPos), getYPos(tacticalPos));

        // field, spice bloom and special bloom

        const std::string BloomString = inifile->getStringValue("MAP", "Bloom");
        if (!BloomString.empty()) {
            const auto BloomPositions = splitStringToStringVector(BloomString);

            for (const auto& BloomPosition : BloomPositions) {
                // set bloom
                int BloomPos = 0;
                if (parseString(BloomPosition, BloomPos)) {
                    int xpos = getXPos(BloomPos);
                    int ypos = getYPos(BloomPos);
                    pMapEditor->getSpiceBlooms().emplace_back(xpos, ypos);
                } else {
                    logWarning(inifile->getLineNumber("MAP", "Bloom"), "Invalid spice bloom position: '%s'!",
                               BloomPosition);
                }
            }
        }

        const auto SpecialString = inifile->getStringValue("MAP", "Special");
        if (!SpecialString.empty()) {
            const auto SpecialPositions = splitStringToStringVector(SpecialString);

            for (auto& SpecialPosition : SpecialPositions) {
                // set special
                int SpecialPos = 0;
                if (parseString(SpecialPosition, SpecialPos)) {
                    int xpos = getXPos(SpecialPos);
                    int ypos = getYPos(SpecialPos);
                    pMapEditor->getSpecialBlooms().emplace_back(xpos, ypos);
                } else {
                    logWarning(inifile->getLineNumber("MAP", "Special"), "Invalid special bloom position: '%s'!",
                               SpecialPosition);
                }
            }
        }

        const std::string FieldString = inifile->getStringValue("MAP", "Field");
        if (!FieldString.empty()) {
            const std::vector<std::string> FieldPositions = splitStringToStringVector(FieldString);

            for (const auto& FieldPosition : FieldPositions) {
                // set bloom
                int FieldPos = 0;
                if (parseString(FieldPosition, FieldPos)) {
                    int xpos = getXPos(FieldPos);
                    int ypos = getYPos(FieldPos);
                    pMapEditor->getSpiceFields().emplace_back(xpos, ypos);
                } else {
                    logWarning(inifile->getLineNumber("MAP", "Field"), "Invalid spice field position: '%s'!",
                               FieldPosition);
                }
            }
        }

    } else {
        // new map format with saved map

        pMapEditor->mapInfo.cursorPos   = Coord::Invalid();
        pMapEditor->mapInfo.tacticalPos = Coord::Invalid();
        pMapEditor->mapInfo.mapSeed     = INVALID;

        const int sizeX = inifile->getIntValue("MAP", "SizeX", 0);
        const int sizeY = inifile->getIntValue("MAP", "SizeY", 0);

        pMapEditor->map = MapData(sizeX, sizeY);

        logicalSizeX   = sizeX;
        logicalSizeY   = sizeY;
        logicalOffsetX = 0;
        logicalOffsetY = 0;

        for (int y = 0; y < sizeY; y++) {
            const auto rowKey = fmt::sprintf("%.3d", y);

            if (!inifile->hasKey("MAP", rowKey)) {
                logWarning(inifile->getLineNumber("MAP"), "Map row %d does not exist!", y);
                continue;
            }

            const auto rowString = inifile->getStringValue("MAP", rowKey);

            auto rowLength = rowString.size();

            if (rowLength < sizeX) {
                logWarning(inifile->getLineNumber("MAP", rowKey), "Map row %d is not long enough!", y);
            } else if (rowLength > sizeX) {
                logWarning(inifile->getLineNumber("MAP", rowKey), "Map row %d is too long!", y);
                rowLength = sizeX;
            }

            for (int x = 0; x < rowLength; x++) {
                TERRAINTYPE type = Terrain_Sand;

                switch (rowString.at(x)) {
                    case '-': {
                        // Normal sand
                        type = Terrain_Sand;
                    } break;

                    case '^': {
                        // Sand dunes
                        type = Terrain_Dunes;
                    } break;

                    case '~': {
                        // Spice
                        type = Terrain_Spice;
                    } break;

                    case '+': {
                        // Thick spice
                        type = Terrain_ThickSpice;
                    } break;

                    case '%': {
                        // Rock
                        type = Terrain_Rock;
                    } break;

                    case '@': {
                        // Mountain
                        type = Terrain_Mountain;
                    } break;

                    case 'O': {
                        // Spice Bloom
                        type = Terrain_SpiceBloom;
                    } break;

                    case 'Q': {
                        // Special Bloom
                        type = Terrain_SpecialBloom;
                    } break;

                    default: {
                        logWarning(inifile->getLineNumber("MAP", rowKey),
                                   std::string("Unknown map tile type '") + rowString.at(x) + "' in map tile ("
                                       + std::to_string(x) + ", " + std::to_string(y) + ")!");
                        type = Terrain_Sand;
                    } break;
                }

                pMapEditor->map(x, y) = type;
            }
        }
    }

    dune::globals::screenborder->adjustScreenBorderToMapsize(pMapEditor->map.getSizeX(), pMapEditor->map.getSizeY());
}

/**
    This method loads the houses on the map specified by the various house sections in the INI file ([Atreides],
   [Ordos], [Harkonnen]).
*/
void INIMapEditorLoader::loadHouses() {
    for (int houseID = 0; houseID < NUM_HOUSES; houseID++) {
        std::string houseName = getHouseNameByNumber(static_cast<HOUSETYPE>(houseID));

        if (inifile->hasSection(houseName)) {
            auto& player = pMapEditor->getPlayers()[houseID];

            player.bActive   = true;
            player.bAnyHouse = false;
            player.credits   = inifile->getIntValue(houseName, "Credits", 0);
            player.quota     = inifile->getIntValue(houseName, "Quota", 0);
            player.maxunit   = inifile->hasKey(houseName, "MaxUnit") ? inifile->getIntValue(houseName, "MaxUnit", 0)
                                                                     : inifile->getIntValue(houseName, "MaxUnits", 0);

            std::string brain = inifile->getStringValue(houseName, "Brain", "");
            if (!brain.empty()) {
                player.brain = brain;
            }
        }
    }

    for (int i = 1; i <= NUM_HOUSES; i++) {
        std::string sectionname = "player" + std::to_string(i);
        if (inifile->hasSection(sectionname)) {
            for (int houseID = 0; houseID < NUM_HOUSES; houseID++) {
                auto& player = pMapEditor->getPlayers()[houseID];

                if (!player.bActive) {
                    convertToLower(sectionname);
                    housename2house[sectionname] = static_cast<HOUSETYPE>(houseID);

                    player.bActive   = true;
                    player.bAnyHouse = true;
                    player.credits   = inifile->getIntValue(sectionname, "Credits", 0);
                    player.quota     = inifile->getIntValue(sectionname, "Quota", 0);
                    player.maxunit   = inifile->hasKey(sectionname, "MaxUnit")
                                         ? inifile->getIntValue(sectionname, "MaxUnit", 0)
                                         : inifile->getIntValue(sectionname, "MaxUnits", 0);

                    std::string brain = inifile->getStringValue(sectionname, "Brain", "");
                    if (!brain.empty()) {
                        player.brain = brain;
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

    if (!inifile->hasSection(sectionname)) {
        return;
    }

    for (const auto& key : inifile->keys(sectionname)) {
        const auto UnitStr = key.getKeyName();

        auto unitID = getItemIDByName(UnitStr);
        if ((unitID == ItemID_Invalid) || !isUnit(unitID)) {
            logWarning(inifile->getLineNumber(sectionname, key.getKeyName()), "Invalid unit string: '%s'", UnitStr);
            continue;
        }

        int num = key.getIntValue(-2);
        if (num == -2) {
            logWarning(inifile->getLineNumber(sectionname, key.getKeyName()), "Invalid choam number!");
            continue;
        }

        if (num == -1) {
            num = 0;
        }

        pMapEditor->getChoam()[unitID] = num;
    }
}

/**
    This method loads the units specified by the [Units] section.
*/
void INIMapEditorLoader::loadUnits() {
    static constexpr auto sectionname = "UNITS"sv;

    if (!inifile->hasSection(sectionname)) {
        return;
    }

    for (const auto& key : inifile->keys(sectionname)) {
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
                logWarning(inifile->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid house string for '" + UnitStr + "': '" + HouseStr + "'!");
                continue;
            }

            int pos = 0;
            if (!parseString(PosStr, pos) || (pos < 0)) {
                logWarning(inifile->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid position string for '" + UnitStr + "': '" + PosStr + "'!");
                continue;
            }

            int int_angle = 0;
            if (!parseString(rotation, int_angle) || (int_angle < 0) || (int_angle > 255)) {
                logWarning(inifile->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid rotation string: '" + rotation + "'!");
                int_angle = 64;
            }
            int_angle  = (int_angle + 16) / 32;
            int_angle  = NUM_ANGLES - int_angle + 2;
            auto angle = normalizeAngle(static_cast<ANGLETYPE>(int_angle));

            ItemID_enum itemID = getItemIDByName(UnitStr);
            if ((itemID == ItemID_Invalid) || !isUnit(itemID)) {
                logWarning(inifile->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid unit string: '" + UnitStr + "'!");
                continue;
            }

            int iHealth = 0;
            if (!parseString(health, iHealth) || (iHealth < 0) || (iHealth > 256)) {
                logWarning(inifile->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid health string: '" + health + "'!");
                iHealth = 256;
            }

            ATTACKMODE attackmode = getAttackModeByName(mode);
            if (attackmode == ATTACKMODE_INVALID) {
                logWarning(inifile->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid attackmode string: '" + mode + "'!");
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

            pMapEditor->units.emplace_back(unitID, houseID, itemID, iHealth, Coord(getXPos(pos), getYPos(pos)), angle,
                                           attackmode);

        } else {
            logWarning(inifile->getLineNumber(sectionname, key.getKeyName()), "Invalid unit key: '%s'!",
                       key.getKeyName());
        }
    }
}

/**
    This method loads the structures specified by the [Structures] section.
*/
void INIMapEditorLoader::loadStructures() {
    static constexpr auto sectionname = "STRUCTURES"sv;

    if (!inifile->hasSection(sectionname)) {
        return;
    }

    int genID = -2;

    for (const auto& key : inifile->keys(sectionname)) {
        const auto tmpkey = key.getKeyName();
        const auto tmp    = key.getStringView();

        if (tmpkey.compare(0, 3, "GEN") == 0) {
            // Gen Object/Structure
            const auto PosStr = tmpkey.substr(3, tmpkey.size() - 3);
            int pos           = 0;
            if (!parseString(PosStr, pos) || (pos < 0)) {
                logWarning(inifile->getLineNumber(sectionname, key.getKeyName()), "Invalid position string: '%s'!",
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
                logWarning(inifile->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid house string for '%s': '%s'!", BuildingStr, HouseStr);
                continue;
            }

            auto itemID = getItemIDByName(BuildingStr);

            if ((itemID == Structure_Slab1) || (itemID == Structure_Slab4) || (itemID == Structure_Wall)) {
                pMapEditor->structures.emplace_back(genID, houseID, itemID, 256, Coord(getXPos(pos), getYPos(pos)));
                genID--;
            } else {
                logWarning(inifile->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid building string: '%s' for GEN-Placement!", BuildingStr);
            }

        } else if (tmpkey.compare(0, 2, "ID") == 0) {
            // other structure
            int structureID = 0;
            parseString(tmpkey.substr(2), structureID);

            std::string HouseStr, BuildingStr, health, PosStr;
            splitString(tmp, HouseStr, BuildingStr, health, PosStr);

            int pos = 0;
            if (!parseString(PosStr, pos) || (pos < 0)) {
                logWarning(inifile->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid position string for '%s': '%s'!", BuildingStr, PosStr);
                continue;
            }

            const auto houseID = getHouseID(HouseStr);
            if (houseID == HOUSETYPE::HOUSE_UNUSED) {
                // skip structure for unused house
                continue;
            }
            if (houseID == HOUSETYPE::HOUSE_INVALID) {
                logWarning(inifile->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid house string for '%s': '%s'!", BuildingStr, HouseStr);
                continue;
            }

            auto iHealth = 0;
            if (!parseString(health, iHealth) || (iHealth < 0) || (iHealth > 256)) {
                logWarning(inifile->getLineNumber(sectionname, key.getKeyName()), "Invalid health string: '%s'!",
                           health);
                iHealth = 256;
            }

            auto itemID = getItemIDByName(BuildingStr);

            if ((itemID == ItemID_Invalid) || !isStructure(itemID)) {
                logWarning(inifile->getLineNumber(sectionname, key.getKeyName()), "Invalid building string: '%s'!",
                           BuildingStr);
                continue;
            }

            pMapEditor->structures.emplace_back(structureID, houseID, itemID, iHealth,
                                                Coord(getXPos(pos), getYPos(pos)));
        } else {
            logWarning(inifile->getLineNumber(sectionname, key.getKeyName()), "Invalid structure key: '%s'!", tmpkey);
        }
    }
}

/**
    This method loads the reinforcements from the [REINFORCEMENTS] section.
*/
void INIMapEditorLoader::loadReinforcements() {
    static constexpr auto sectionname = "REINFORCEMENTS"sv;

    if (!inifile->hasSection(sectionname)) {
        return;
    }

    for (const auto& key : inifile->keys(sectionname)) {
        std::string strHouseName;
        std::string strUnitName;
        std::string strDropLocation;
        std::string strTime;
        std::string strPlus;

        if (!splitString(key.getStringView(), strHouseName, strUnitName, strDropLocation, strTime)) {
            if (!splitString(key.getStringView(), strHouseName, strUnitName, strDropLocation, strTime, strPlus)) {
                logWarning(inifile->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid reinforcement string: %s = %s", key.getKeyName(), key.getStringView());
                continue;
            }
        }

        const auto houseID = getHouseID(strHouseName);
        if (houseID == HOUSETYPE::HOUSE_UNUSED) {
            // skip reinforcement for unused house
            continue;
        }
        if (houseID == HOUSETYPE::HOUSE_INVALID) {
            logWarning(inifile->getLineNumber(sectionname, key.getKeyName()), "Invalid house string: '%s'!",
                       strHouseName);
            continue;
        }

        const auto unitID = getItemIDByName(strUnitName);
        if ((unitID == ItemID_Invalid) || !isUnit(unitID)) {
            logWarning(inifile->getLineNumber(sectionname, key.getKeyName()), "Invalid unit string: '%s'!",
                       strUnitName);
            continue;
        }

        auto dropLocation = getDropLocationByName(strDropLocation);
        if (dropLocation == DropLocation::Drop_Invalid) {
            logWarning(inifile->getLineNumber(sectionname, key.getKeyName()), "Invalid drop location string: '%s'!",
                       strDropLocation);
            dropLocation = DropLocation::Drop_Homebase;
        }

        auto droptime = 0;
        if (!parseString(strTime, droptime)) {
            logWarning(inifile->getLineNumber(sectionname, key.getKeyName()),
                       "Invalid drop time string: '" + strTime + "'!");
            continue;
        }

        auto bRepeat = (strTime.rfind('+') == (strTime.length() - 1)) || (strPlus == "+");

        pMapEditor->getReinforcements().emplace_back(houseID, unitID, dropLocation, droptime, bRepeat);
    }
}

/**
    This method loads the AI teams from the [TEAMS] section.
*/
void INIMapEditorLoader::loadAITeams() {
    static constexpr auto sectionname = "TEAMS"sv;

    if (!inifile->hasSection(sectionname)) {
        return;
    }

    for (const auto& key : inifile->keys(sectionname)) {
        std::string strHouseName;
        std::string strAITeamBehavior;
        std::string strAITeamType;
        std::string strMinUnits;
        std::string strMaxUnits;

        if (!splitString(key.getStringView(), strHouseName, strAITeamBehavior, strAITeamType, strMinUnits,
                         strMaxUnits)) {
            logWarning(inifile->getLineNumber(sectionname, key.getKeyName()), "Invalid teams string: %s = %s",
                       key.getKeyName(), key.getStringView());
            continue;
        }

        const auto houseID = getHouseID(strHouseName);
        if (houseID == HOUSETYPE::HOUSE_UNUSED) {
            // skip reinforcement for unused house
            continue;
        }
        if (houseID == HOUSETYPE::HOUSE_INVALID) {
            logWarning(inifile->getLineNumber(sectionname, key.getKeyName()), "Invalid house string: '%s'!",
                       strHouseName);
            continue;
        }

        auto aiTeamBehavior = getAITeamBehaviorByName(strAITeamBehavior);
        if (aiTeamBehavior == AITeamBehavior::AITeamBehavior_Invalid) {
            logWarning(inifile->getLineNumber(sectionname, key.getKeyName()), "Invalid team behavior string: '%s'!",
                       strAITeamBehavior);
            aiTeamBehavior = AITeamBehavior::AITeamBehavior_Normal;
        }

        auto aiTeamType = getAITeamTypeByName(strAITeamType);
        if (aiTeamType == AITeamType::AITeamType_Invalid) {
            logWarning(inifile->getLineNumber(sectionname, key.getKeyName()), "Invalid team type string: '%s'!",
                       strAITeamType);
            aiTeamType = AITeamType::AITeamType_Foot;
        }

        auto minUnits = 0;
        if (!parseString(strMinUnits, minUnits)) {
            logWarning(inifile->getLineNumber(sectionname, key.getKeyName()), "Invalid min units string: '%s'!",
                       strMinUnits);
            continue;
        }

        auto maxUnits = 0;
        if (!parseString(strMaxUnits, maxUnits)) {
            logWarning(inifile->getLineNumber(sectionname, key.getKeyName()), "Invalid max units string: '%s'!",
                       strMaxUnits);
            continue;
        }

        pMapEditor->getAITeams().emplace_back(houseID, aiTeamBehavior, aiTeamType, minUnits, maxUnits);
    }
}

HOUSETYPE INIMapEditorLoader::getHouseID(std::string_view name) {
    const auto lowerName = strToLower(name);

    const auto it = housename2house.find(lowerName);

    if (it != housename2house.end())
        return it->second;

    return getHouseByName(lowerName);
}
