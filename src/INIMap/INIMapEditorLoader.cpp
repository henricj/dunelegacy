#include <INIMap/INIMapEditorLoader.h>

#include <FileClasses/FileManager.h>

#include <MapEditor/MapEditor.h>
#include <MapEditor/ReinforcementInfo.h>
#include <AITeamInfo.h>

#include <MapSeed.h>
#include <ScreenBorder.h>

#include <fmt/printf.h>
#include <misc/exceptions.h>

#include <sand.h>
#include <globals.h>

#include <algorithm>

INIMapEditorLoader::INIMapEditorLoader(MapEditor* pMapEditor, INIMap::inifile_ptr pINIFile)
 : INIMap(std::move(pINIFile)), pMapEditor(pMapEditor)
{
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

    if(version < 2) {
        // we have all houses fixed
        for(auto& player : pMapEditor->getPlayers()) {
            player.bAnyHouse = false;
        }
    }


    pMapEditor->mapInfo.author = inifile->getStringValue("BASIC", "Author");
    pMapEditor->mapInfo.license = inifile->getStringValue("BASIC", "License");

    pMapEditor->mapInfo.winFlags = inifile->getIntValue("BASIC", "WinFlags", 3);
    pMapEditor->mapInfo.loseFlags = inifile->getIntValue("BASIC", "LoseFlags", 1);

    pMapEditor->mapInfo.losePicture = inifile->getStringValue("BASIC", "LosePicture");
    pMapEditor->mapInfo.winPicture = inifile->getStringValue("BASIC", "WinPicture");
    pMapEditor->mapInfo.briefPicture = inifile->getStringValue("BASIC", "BriefPicture");

    pMapEditor->mapInfo.timeout = inifile->getIntValue("BASIC", "TimeOut", 0);
    pMapEditor->mapInfo.techLevel = inifile->getIntValue("BASIC", "TechLevel", 0);


    if(version < 2) {
        int mapscale = inifile->getIntValue("BASIC", "MapScale", 0);

        pMapEditor->mapInfo.mapSeed = inifile->getIntValue("MAP", "Seed", 0);

        pMapEditor->map = createMapWithSeed(pMapEditor->mapInfo.mapSeed, mapscale);

        switch(mapscale) {
            case 0: {
                sizeX = 62;
                sizeY = 62;
                logicalOffsetX = 1;
                logicalOffsetY = 1;
            } break;

            case 1: {
                sizeX = 32;
                sizeY = 32;
                logicalOffsetX = 16;
                logicalOffsetY = 16;
            } break;

            case 2: {
                sizeX = 21;
                sizeY = 21;
                logicalOffsetX = 11;
                logicalOffsetY = 11;
            } break;

            default: {
                 logError(inifile->getKey("BASIC", "MapScale")->getLineNumber(), "Unknown MapScale '" + std::to_string(mapscale) + "'!");
            } break;
        }

        logicalSizeX = 64;
        logicalSizeY = 64;




        int cursorPos = inifile->getIntValue("BASIC", "CursorPos", 0);
        pMapEditor->mapInfo.cursorPos = Coord(getXPos(cursorPos), getYPos(cursorPos));
        int tacticalPos = inifile->getIntValue("BASIC", "TacticalPos", 0);
        pMapEditor->mapInfo.tacticalPos = Coord(getXPos(tacticalPos), getYPos(tacticalPos));



        // field, spice bloom and special bloom

        std::string BloomString = inifile->getStringValue("MAP","Bloom");
        if(BloomString != "") {
            std::vector<std::string> BloomPositions  = splitStringToStringVector(BloomString);

            for(auto & BloomPosition : BloomPositions) {
                // set bloom
                int BloomPos = 0;
                if(parseString(BloomPosition, BloomPos)) {
                    int xpos = getXPos(BloomPos);
                    int ypos = getYPos(BloomPos);
                    pMapEditor->getSpiceBlooms().emplace_back(xpos,ypos);
                } else {
                    logWarning(inifile->getKey("MAP", "Bloom")->getLineNumber(), "Invalid spice bloom position: '" + BloomPosition + "'");
                }
            }

        }

        std::string SpecialString = inifile->getStringValue("MAP","Special");
        if(SpecialString != "") {
            std::vector<std::string> SpecialPositions  = splitStringToStringVector(SpecialString);

            for(auto & SpecialPosition : SpecialPositions) {
                // set special
                int SpecialPos = 0;
                if(parseString(SpecialPosition, SpecialPos)) {
                    int xpos = getXPos(SpecialPos);
                    int ypos = getYPos(SpecialPos);
                    pMapEditor->getSpecialBlooms().emplace_back(xpos,ypos);
                } else {
                    logWarning(inifile->getKey("MAP", "Special")->getLineNumber(), "Invalid special bloom position: '" + SpecialPosition + "'");
                }
            }

        }

        std::string FieldString = inifile->getStringValue("MAP","Field");
        if(FieldString != "") {
            std::vector<std::string> FieldPositions  = splitStringToStringVector(FieldString);

            for(auto & FieldPosition : FieldPositions) {
                // set bloom
                int FieldPos = 0;
                if(parseString(FieldPosition, FieldPos)) {
                    int xpos = getXPos(FieldPos);
                    int ypos = getYPos(FieldPos);
                    pMapEditor->getSpiceFields().emplace_back(xpos,ypos);
                } else {
                    logWarning(inifile->getKey("MAP", "Field")->getLineNumber(), "Invalid spice field position: '" + FieldPosition + "'");
                }
            }

        }

    } else {
        // new map format with saved map


        pMapEditor->mapInfo.cursorPos = Coord::Invalid();
        pMapEditor->mapInfo.tacticalPos = Coord::Invalid();
        pMapEditor->mapInfo.mapSeed = INVALID;

        int sizeX = inifile->getIntValue("MAP", "SizeX", 0);
        int sizeY = inifile->getIntValue("MAP", "SizeY", 0);

        pMapEditor->map = MapData(sizeX, sizeY);

        logicalSizeX = sizeX;
        logicalSizeY = sizeY;
        logicalOffsetX = 0;
        logicalOffsetY = 0;

        for(int y=0;y<sizeY;y++) {
            std::string rowKey = fmt::sprintf("%.3d", y);

            if(inifile->hasKey("MAP", rowKey) == false) {
                logWarning(inifile->getSection("MAP").getLineNumber(), "Map row " + std::to_string(y) + " does not exist!");
                continue;
            }

            std::string rowString = inifile->getStringValue("MAP",rowKey);

            int rowLength = rowString.size();

            if(rowLength < sizeX) {
                logWarning(inifile->getKey("MAP", rowKey)->getLineNumber(), "Map row " + std::to_string(y) + " is not long enough!");
            } else if(rowLength > sizeX) {
                logWarning(inifile->getKey("MAP", rowKey)->getLineNumber(), "Map row " + std::to_string(y) + " is too long!");
                rowLength = sizeX;
            }

            for(int x=0;x<rowLength;x++) {
                TERRAINTYPE type = Terrain_Sand;

                switch(rowString.at(x)) {
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
                        logWarning(inifile->getKey("MAP", rowKey)->getLineNumber(), std::string("Unknown map tile type '") + rowString.at(x) + "' in map tile (" + std::to_string(x) + ", " + std::to_string(y) + ")!");
                        type = Terrain_Sand;
                    } break;
                }

                pMapEditor->map(x,y) = type;
            }
        }
    }

    screenborder->adjustScreenBorderToMapsize(pMapEditor->map.getSizeX(), pMapEditor->map.getSizeY());

}

/**
    This method loads the houses on the map specified by the various house sections in the INI file ([Atreides], [Ordos], [Harkonnen]).
*/
void INIMapEditorLoader::loadHouses()
{
    for(int houseID = 0; houseID < static_cast<int>(HOUSETYPE::NUM_HOUSES); houseID++) {
        std::string houseName = getHouseNameByNumber((HOUSETYPE) houseID);

        if(inifile->hasSection(houseName)) {
            auto& player = pMapEditor->getPlayers()[houseID];

            player.bActive = true;
            player.bAnyHouse = false;
            player.credits = inifile->getIntValue(houseName, "Credits", 0);
            player.quota = inifile->getIntValue(houseName, "Quota", 0);
            player.maxunit = inifile->hasKey(houseName, "MaxUnit") ? inifile->getIntValue(houseName, "MaxUnit", 0) : inifile->getIntValue(houseName, "MaxUnits", 0);

            std::string brain = inifile->getStringValue(houseName, "Brain", "");
            if(brain != "") {
                player.brain = brain;
            }
        }
    }

    for(int i=1;i<=static_cast<int>(HOUSETYPE::NUM_HOUSES);i++) {
        std::string sectionname = "player" + std::to_string(i);
        if(inifile->hasSection(sectionname)) {
            for(int houseID = 0; houseID < static_cast<int>(HOUSETYPE::NUM_HOUSES); houseID++) {
                auto& player = pMapEditor->getPlayers()[houseID];

                if(player.bActive == false) {
                    convertToLower(sectionname);
                    housename2house[sectionname] = (HOUSETYPE) houseID;

                    player.bActive = true;
                    player.bAnyHouse = true;
                    player.credits = inifile->getIntValue(sectionname, "Credits", 0);
                    player.quota = inifile->getIntValue(sectionname, "Quota", 0);
                    player.maxunit = inifile->hasKey(sectionname, "MaxUnit") ? inifile->getIntValue(sectionname, "MaxUnit", 0) : inifile->getIntValue(sectionname, "MaxUnits", 0);

                    std::string brain = inifile->getStringValue(sectionname, "Brain", "");
                    if(brain != "") {
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
void INIMapEditorLoader::loadChoam()
{
    if(!inifile->hasSection("CHOAM")) {
        return;
    }

    for(const INIFile::Key& key : inifile->getSection("CHOAM")) {
        std::string UnitStr = key.getKeyName();

        Uint32 unitID = getItemIDByName(UnitStr);
        if((unitID == ItemID_Invalid) || !isUnit(unitID)) {
            logWarning(key.getLineNumber(), "Invalid unit string: '" + UnitStr + "'");
            continue;
        }

        int num = key.getIntValue(-2);
        if(num == -2) {
            logWarning(key.getLineNumber(), "Invalid choam number!");
            continue;

        }

        if(num == -1) {
            num = 0;
        }

        pMapEditor->getChoam()[unitID] = num;
    }
}

/**
    This method loads the units specified by the [Units] section.
*/
void INIMapEditorLoader::loadUnits()
{
    if(!inifile->hasSection("UNITS")) {
        return;
    }

    for(const INIFile::Key& key : inifile->getSection("UNITS")) {
        if(key.getKeyName().find("ID") == 0) {
            int unitID = 0;
            parseString(key.getKeyName().substr(2), unitID);

            std::string HouseStr, UnitStr, health, PosStr, rotation, mode;
            splitString(key.getStringValue(), HouseStr, UnitStr, health, PosStr, rotation, mode);

            HOUSETYPE houseID = getHouseID(HouseStr);
            if(houseID == HOUSETYPE::HOUSE_UNUSED) {
                // skip unit for unused house
                continue;
            } else if(houseID == HOUSETYPE::HOUSE_INVALID) {
                logWarning(key.getLineNumber(), "Invalid house string for '" + UnitStr + "': '" + HouseStr + "'!");
                continue;
            }

            int pos = 0;
            if(!parseString(PosStr, pos) || (pos < 0)) {
                logWarning(key.getLineNumber(), "Invalid position string for '" + UnitStr + "': '" + PosStr + "'!");
                continue;
            }

            int int_angle = 0;
            if(!parseString(rotation, int_angle) || (int_angle < 0) || (int_angle > 255)) {
                logWarning(key.getLineNumber(), "Invalid rotation string: '" + rotation + "'!");
                int_angle = 64;
            }
            int_angle = (int_angle+16)/32;
            int_angle = static_cast<int>(ANGLETYPE::NUM_ANGLES) - int_angle + 2;
            auto angle = normalizeAngle(static_cast<ANGLETYPE>(int_angle));


            int itemID = getItemIDByName(UnitStr);
            if((itemID == ItemID_Invalid) || !isUnit(itemID)) {
                logWarning(key.getLineNumber(), "Invalid unit string: '" + UnitStr + "'!");
                continue;
            }

            int iHealth = 0;
            if(!parseString(health, iHealth) || (iHealth < 0) || (iHealth > 256)) {
                logWarning(key.getLineNumber(), "Invalid health string: '" + health + "'!");
                iHealth = 256;
            }

            ATTACKMODE attackmode = getAttackModeByName(mode);
            if(attackmode == ATTACKMODE_INVALID) {
                logWarning(key.getLineNumber(), "Invalid attackmode string: '" + mode + "'!");
                attackmode = AREAGUARD;
            }

            if(itemID == Unit_Soldier || itemID == Unit_Saboteur || itemID == Unit_Trooper || itemID == Unit_Infantry || itemID == Unit_Troopers) {
                if(angle == ANGLETYPE::UP) {
                    angle = ANGLETYPE::UP;
                } else if(angle == ANGLETYPE::DOWN) {
                    angle = ANGLETYPE::DOWN;
                } else if(angle == ANGLETYPE::LEFTUP || angle == ANGLETYPE::LEFTDOWN || angle == ANGLETYPE::LEFT) {
                    angle = ANGLETYPE::LEFT;
                } else /*(angle == ANGLETYPE::RIGHT)*/ {
                    angle = ANGLETYPE::RIGHT;
                }
            }

            pMapEditor->units.emplace_back(unitID, houseID, itemID, iHealth, Coord(getXPos(pos),getYPos(pos)), angle, attackmode);

        } else {
            logWarning(key.getLineNumber(), "Invalid unit key: '" + key.getKeyName() + "'!");
            continue;
        }
    }
}

/**
    This method loads the structures specified by the [Structures] section.
*/
void INIMapEditorLoader::loadStructures()
{
    if(!inifile->hasSection("STRUCTURES")) {
        return;
    }

    int genID = -2;

    for(const INIFile::Key& key : inifile->getSection("STRUCTURES")) {
        std::string tmpkey = key.getKeyName();
        std::string tmp = key.getStringValue();

        if(tmpkey.compare(0,3,"GEN") == 0) {
            // Gen Object/Structure
            std::string PosStr = tmpkey.substr(3,tmpkey.size()-3);
            int pos = 0;
            if(!parseString(PosStr, pos) || (pos < 0)) {
                logWarning(key.getLineNumber(), "Invalid position string: '" + PosStr + "'!");
                continue;
            }

            std::string HouseStr, BuildingStr;
            splitString(tmp, HouseStr, BuildingStr);

            HOUSETYPE houseID = getHouseID(HouseStr);
            if(houseID == HOUSETYPE::HOUSE_UNUSED) {
                // skip structure for unused house
                continue;
            } else if(houseID == HOUSETYPE::HOUSE_INVALID) {
                logWarning(key.getLineNumber(), "Invalid house string for '" + BuildingStr + "': '" + HouseStr + "'!");
                continue;
            }

            int itemID = getItemIDByName(BuildingStr);

            if((itemID == Structure_Slab1) || (itemID == Structure_Slab4) || (itemID == Structure_Wall)) {
                pMapEditor->structures.emplace_back(genID, houseID, itemID, 256, Coord(getXPos(pos),getYPos(pos)));
                genID--;
            } else {
                logWarning(key.getLineNumber(), "Invalid building string: '" + BuildingStr + "' for GEN-Placement!");
                continue;
            }

        } else if(tmpkey.compare(0,2,"ID") == 0) {
            // other structure
            int structureID = 0;
            parseString(tmpkey.substr(2), structureID);

            std::string HouseStr, BuildingStr, health, PosStr;
            splitString(tmp, HouseStr, BuildingStr, health, PosStr);

            int pos = 0;
            if(!parseString(PosStr, pos) || (pos < 0)) {
                logWarning(key.getLineNumber(), "Invalid position string for '" + BuildingStr + "': '" + PosStr + "'!");
                continue;
            }

            HOUSETYPE houseID = getHouseID(HouseStr);
            if(houseID == HOUSETYPE::HOUSE_UNUSED) {
                // skip structure for unused house
                continue;
            } else if(houseID == HOUSETYPE::HOUSE_INVALID) {
                logWarning(key.getLineNumber(), "Invalid house string for '" + BuildingStr + "': '" + HouseStr + "'!");
                continue;
            }

            int iHealth = 0;
            if(!parseString(health, iHealth) || (iHealth < 0) || (iHealth > 256)) {
                logWarning(key.getLineNumber(), "Invalid health string: '" + health + "'!");
                iHealth = 256;
            }

            int itemID = getItemIDByName(BuildingStr);

            if((itemID == ItemID_Invalid) || !isStructure(itemID)) {
                logWarning(key.getLineNumber(), "Invalid building string: '" + BuildingStr + "'!");
                continue;
            }

            pMapEditor->structures.emplace_back(structureID, houseID, itemID, iHealth, Coord(getXPos(pos),getYPos(pos)));
        } else {
            logWarning(key.getLineNumber(), "Invalid structure key: '" + tmpkey + "'!");
            continue;
        }
    }
}

/**
    This method loads the reinforcements from the [REINFORCEMENTS] section.
*/
void INIMapEditorLoader::loadReinforcements()
{
    if(!inifile->hasSection("REINFORCEMENTS")) {
        return;
    }

    for(const INIFile::Key& key : inifile->getSection("REINFORCEMENTS")) {
        std::string strHouseName;
        std::string strUnitName;
        std::string strDropLocation;
        std::string strTime;
        std::string strPlus;

        if(splitString(key.getStringValue(), strHouseName, strUnitName, strDropLocation, strTime) == false) {
            if(splitString(key.getStringValue(), strHouseName, strUnitName, strDropLocation, strTime, strPlus) == false) {
                logWarning(key.getLineNumber(), "Invalid reinforcement string: " + key.getKeyName() + " = " + key.getStringValue());
                continue;
            }
        }

        const auto houseID = getHouseID(strHouseName);
        if(houseID == HOUSETYPE::HOUSE_UNUSED) {
            // skip reinforcement for unused house
            continue;
        } else if(houseID == HOUSETYPE::HOUSE_INVALID) {
            logWarning(key.getLineNumber(), "Invalid house string: '" + strHouseName + "'!");
            continue;
        }

        Uint32 unitID = getItemIDByName(strUnitName);
        if((unitID == ItemID_Invalid) || !isUnit(unitID)) {
            logWarning(key.getLineNumber(), "Invalid unit string: '" + strUnitName + "'!");
            continue;
        }

        auto dropLocation = getDropLocationByName(strDropLocation);
        if(dropLocation == DropLocation::Drop_Invalid) {
            logWarning(key.getLineNumber(), "Invalid drop location string: '" + strDropLocation + "'!");
            dropLocation = DropLocation::Drop_Homebase;
        }

        Uint32 droptime = 0;
        if(!parseString(strTime, droptime)) {
            logWarning(key.getLineNumber(), "Invalid drop time string: '" + strTime + "'!");
            continue;
        }

        bool bRepeat = (strTime.rfind('+') == (strTime.length() - 1)) || (strPlus == "+");

        pMapEditor->getReinforcements().emplace_back(houseID, unitID, dropLocation, droptime, bRepeat);
    }
}

/**
    This method loads the AI teams from the [TEAMS] section.
*/
void INIMapEditorLoader::loadAITeams()
{
    if(!inifile->hasSection("TEAMS")) {
        return;
    }

    for(const INIFile::Key& key : inifile->getSection("TEAMS")) {
        std::string strHouseName;
        std::string strAITeamBehavior;
        std::string strAITeamType;
        std::string strMinUnits;
        std::string strMaxUnits;

        if(splitString(key.getStringValue(), strHouseName, strAITeamBehavior, strAITeamType, strMinUnits, strMaxUnits) == false) {
            logWarning(key.getLineNumber(), "Invalid teams string: " + key.getKeyName() + " = " + key.getStringValue());
            continue;
        }

        const auto houseID = getHouseID(strHouseName);
        if(houseID == HOUSETYPE::HOUSE_UNUSED) {
            // skip reinforcement for unused house
            continue;
        } else if(houseID == HOUSETYPE::HOUSE_INVALID) {
            logWarning(key.getLineNumber(), "Invalid house string: '" + strHouseName + "'!");
            continue;
        }

        AITeamBehavior aiTeamBehavior = getAITeamBehaviorByName(strAITeamBehavior);
        if(aiTeamBehavior == AITeamBehavior::AITeamBehavior_Invalid) {
            logWarning(key.getLineNumber(), "Invalid team behavior string: '" + strAITeamBehavior + "'!");
            aiTeamBehavior = AITeamBehavior::AITeamBehavior_Normal;
        }

        AITeamType aiTeamType = getAITeamTypeByName(strAITeamType);
        if(aiTeamType == AITeamType::AITeamType_Invalid) {
            logWarning(key.getLineNumber(), "Invalid team type string: '" + strAITeamType + "'!");
            aiTeamType = AITeamType::AITeamType_Foot;
        }

        int minUnits = 0;
        if(!parseString(strMinUnits, minUnits)) {
            logWarning(key.getLineNumber(), "Invalid min units string: '" + strMinUnits + "'!");
            continue;
        }

        int maxUnits = 0;
        if(!parseString(strMaxUnits, maxUnits)) {
            logWarning(key.getLineNumber(), "Invalid max units string: '" + strMaxUnits + "'!");
            continue;
        }

        pMapEditor->getAITeams().emplace_back(houseID, aiTeamBehavior, aiTeamType, minUnits, maxUnits);
    }
}

HOUSETYPE INIMapEditorLoader::getHouseID(const std::string& name) {
    std::string lowerName = strToLower(name);

    if(housename2house.count(lowerName) > 0) {
        return housename2house[lowerName];
    } else {
        return getHouseByName(lowerName);
    }
}
