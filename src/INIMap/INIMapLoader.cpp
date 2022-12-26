#include <INIMap/INIMapLoader.h>

#include <FileClasses/FileManager.h>

#include <Game.h>
#include <House.h>
#include <players/PlayerFactory.h>
#include <players/Player.h>
#include <players/AIPlayer.h>
#include <players/HumanPlayer.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <MapSeed.h>
#include <GameInitSettings.h>
#include <RadarView.h>

#include <structures/StructureBase.h>
#include <units/UnitBase.h>
#include <units/TankBase.h>

#include <Trigger/ReinforcementTrigger.h>
#include <Trigger/TimeoutTrigger.h>

#include <misc/format.h>
#include <misc/exceptions.h>

#include <sand.h>
#include <globals.h>

#include <algorithm>

INIMapLoader::INIMapLoader(Game* pGame, const std::string& mapname, const std::string& mapdata)
 : INIMap(pGame->gameType, mapname, mapdata), pGame(pGame)
{
    load();
}

INIMapLoader::~INIMapLoader() {
}


/**
    Loads a map from an INI-File.
*/
void INIMapLoader::load() {
    checkFeatures();

    loadMap();
    loadHouses();
    loadUnits();
    loadStructures();
    loadReinforcements();
    loadAITeams();
    loadView();
    loadChoam();
}

/**
    This method loads the game map. This is based on the [MAP] section in the INI file.
*/
void INIMapLoader::loadMap() {
    version = inifile->getIntValue("BASIC", "Version", 1);

    pGame->winFlags = inifile->getIntValue("BASIC","WinFlags",3);
    pGame->loseFlags = inifile->getIntValue("BASIC","LoseFlags",1);

    if(pGame->techLevel == 0) {
        pGame->techLevel = inifile->getIntValue("BASIC","TechLevel",8);
    }

    int timeout = inifile->getIntValue("BASIC","TIMEOUT",0);

    if((timeout != 0) && ((pGame->winFlags & WINLOSEFLAGS_TIMEOUT) != 0)) {
        pGame->getTriggerManager().addTrigger(std::make_unique<TimeoutTrigger>(MILLI2CYCLES(timeout * 60 * 1000)));
    }

    if(version < 2) {
        if(inifile->hasKey("MAP","Seed") == false) {
            logError("Cannot find seed value for this map!");
        }

        // old map format with seed value

        if(inifile->hasKey("BASIC","MapScale") == false) {
            logError("Cannot find MapScale for this map!");
        }

        int mapscale = inifile->getIntValue("BASIC","MapScale",0);

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

        currentGameMap = new Map(sizeX, sizeY);

        int SeedNum = inifile->getIntValue("MAP","Seed",-1);
        Uint16 SeedMap[64*64];
        createMapWithSeed(SeedNum,SeedMap);

        for (int j = 0; j < currentGameMap->getSizeY(); j++) {
            for (int i = 0; i < currentGameMap->getSizeX(); i++) {
                int type = Terrain_Sand;
                unsigned char seedmaptype = SeedMap[(j+logicalOffsetY)*64+i+logicalOffsetX] >> 4;
                switch(seedmaptype) {

                    case 0x7:   /* Sand */
                        type = Terrain_Sand;
                        break;

                    case 0x2:   /* Building */
                    case 0x8:   /* Rock */
                        type = Terrain_Rock;
                        break;

                    case 0x9:   /* Dunes */
                        type = Terrain_Dunes;
                        break;

                    case 0xa:   /* Mountain */
                        type = Terrain_Mountain;
                        break;

                    case 0xb:   /* Spice */
                        type = Terrain_Spice;
                        break;

                    case 0xc:   /* ThickSpice */
                        type = Terrain_ThickSpice;
                        break;

                    default:
                        logWarning(inifile->getKey("MAP", "Seed")->getLineNumber(), "Unknown map type '" + std::to_string(seedmaptype) + "' for tile (" + std::to_string(i) + ", " + std::to_string(j) + ")!");
                        type = Terrain_Sand;
                        break;
                }

                currentGameMap->getTile(i,j)->setType(type);
            }

        }

        currentGameMap->createSandRegions();

        std::string BloomString = inifile->getStringValue("MAP","Bloom");
        if(BloomString != "") {
            std::vector<std::string> BloomPositions  = splitStringToStringVector(BloomString);

            for(unsigned int i=0; i < BloomPositions.size();i++) {
                // set bloom
                int BloomPos;
                if(parseString(BloomPositions[i], BloomPos)) {
                    int xpos = getXPos(BloomPos);
                    int ypos = getYPos(BloomPos);
                    if(currentGameMap->tileExists(xpos, ypos)) {
                        currentGameMap->getTile(xpos,ypos)->setType(Terrain_SpiceBloom);
                    } else {
                        logWarning(inifile->getKey("MAP", "Bloom")->getLineNumber(), "Spice bloom position '" + BloomPositions[i] + "' outside map!");
                    }
                } else {
                    logWarning(inifile->getKey("MAP", "Bloom")->getLineNumber(), "Invalid spice bloom position: '" + BloomPositions[i] + "'");
                }
            }

        }

        std::string SpecialString = inifile->getStringValue("MAP","Special");
        if(SpecialString != "") {
            std::vector<std::string> SpecialPositions  = splitStringToStringVector(SpecialString);

            for(unsigned int i=0; i < SpecialPositions.size();i++) {
                // set special
                int SpecialPos;
                if(parseString(SpecialPositions[i], SpecialPos)) {
                    int xpos = getXPos(SpecialPos);
                    int ypos = getYPos(SpecialPos);
                    if(currentGameMap->tileExists(xpos, ypos)) {
                        currentGameMap->getTile(xpos,ypos)->setType(Terrain_SpecialBloom);
                    } else {
                        logWarning(inifile->getKey("MAP", "Special")->getLineNumber(), "Special bloom position '" + SpecialPositions[i] + "' outside map!");
                    }
                } else {
                    logWarning(inifile->getKey("MAP", "Special")->getLineNumber(), "Invalid special bloom position: '" + SpecialPositions[i] + "'");
                }
            }

        }

        std::string FieldString = inifile->getStringValue("MAP","Field");
        if(FieldString != "") {
            std::vector<std::string> FieldPositions  = splitStringToStringVector(FieldString);

            for(unsigned int i=0; i < FieldPositions.size();i++) {
                // set bloom
                int FieldPos;
                if(parseString(FieldPositions[i], FieldPos)) {
                    int xpos = getXPos(FieldPos);
                    int ypos = getYPos(FieldPos);

                    currentGameMap->createSpiceField(Coord(xpos, ypos), 5, true);
                } else {
                    logWarning(inifile->getKey("MAP", "Field")->getLineNumber(), "Invalid spice field position: '" + FieldPositions[i] + "'");
                }
            }

        }

    } else {
        // new map format with saved map

        if((inifile->hasKey("MAP","SizeX") == false) || (inifile->hasKey("MAP","SizeY") == false)) {
            logError("SizeX and SizeY must be specified!");
        }

        sizeX = inifile->getIntValue("MAP","SizeX", 0);
        sizeY = inifile->getIntValue("MAP","SizeY", 0);

        if(sizeX <= 0) {
            logError(inifile->getKey("MAP", "SizeX")->getLineNumber(), "Invalid map size: " + std::to_string(sizeX) + "x" + std::to_string(sizeY) + "!");
        }

        if(sizeY <= 0) {
            logError(inifile->getKey("MAP", "SizeY")->getLineNumber(), "Invalid map size: " + std::to_string(sizeX) + "x" + std::to_string(sizeY) + "!");
        }

        logicalSizeX = sizeX;
        logicalSizeY = sizeY;
        logicalOffsetX = 0;
        logicalOffsetY = 0;

        currentGameMap = new Map(sizeX, sizeY);

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
                int type = Terrain_Sand;

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

                currentGameMap->getTile(x,y)->setType(type);
            }
        }

        currentGameMap->createSandRegions();
    }

    screenborder->adjustScreenBorderToMapsize(currentGameMap->getSizeX(), currentGameMap->getSizeY());
}

/**
    This method loads the houses on the map specified by the various house sections in the INI file ([Atreides], [Ordos], [Harkonnen]).
*/
void INIMapLoader::loadHouses()
{
    const GameInitSettings::HouseInfoList& houseInfoList = pGame->getGameInitSettings().getHouseInfoList();

    // find "player?" sections
    std::vector<std::string> playerSectionsOnMap;
    for(int i=1;i<=NUM_HOUSES;i++) {
        std::string sectionname = "player" + std::to_string(i);
        if(inifile->hasSection(sectionname)) {
            playerSectionsOnMap.push_back(sectionname);
        }
    }

    // find unbounded houses
    std::vector<HOUSETYPE> unboundedHouses;

    for(int h=0;h<NUM_HOUSES;h++) {
        bool bFound = false;
        for(const GameInitSettings::HouseInfo& houseInfo : houseInfoList) {
            if(houseInfo.houseID == (HOUSETYPE) h) {
                bFound = true;
                break;
            }
        }

        std::string houseName = getHouseNameByNumber((HOUSETYPE) h);
        if((bFound == false) && (inifile->hasSection(houseName) || (playerSectionsOnMap.empty() == false))) {
            unboundedHouses.push_back((HOUSETYPE) h);
        }
    }

    // init housename2house mapping with every house section marked as unused
    for(int i=0;i<NUM_HOUSES;i++) {
        std::string houseName = getHouseNameByNumber((HOUSETYPE) i);
        convertToLower(houseName);

        if(inifile->hasSection(houseName)) {
            housename2house[houseName] = HOUSE_UNUSED;
        }
    }

    // init housename2house mapping with every player section on map marked as unused
    for(const std::string& playSection : playerSectionsOnMap) {
        housename2house[playSection] = HOUSE_UNUSED;
    }

    // now set up all the houses
    for(const GameInitSettings::HouseInfo& houseInfo : houseInfoList) {
        HOUSETYPE houseID;

        pGame->houseInfoListSetup.push_back(houseInfo);

        if(houseInfo.houseID == HOUSE_INVALID) {
            // random house => select one unbound house
            if(unboundedHouses.empty()) {
                // skip this house
                continue;
            }
            int randomIndex = pGame->randomGen.rand(0, (int) unboundedHouses.size() - 1);
            houseID = unboundedHouses[randomIndex];
            unboundedHouses.erase(unboundedHouses.begin() + randomIndex);

            pGame->houseInfoListSetup.back().houseID = houseID;
        } else {
            houseID = houseInfo.houseID;
        }

        std::string houseName = getHouseNameByNumber(houseID);
        convertToLower(houseName);

        if(inifile->hasSection(houseName) == false) {
            // select one of the Player sections
            if(playerSectionsOnMap.empty()) {
                // skip this house
                continue;
            }

            int randomIndex = pGame->randomGen.rand(0, (int) playerSectionsOnMap.size() - 1);
            houseName = playerSectionsOnMap[randomIndex];
            playerSectionsOnMap.erase(playerSectionsOnMap.begin() + randomIndex);
        }

        housename2house[houseName] = houseID;

        int startingCredits = inifile->getIntValue(houseName,"Credits",DEFAULT_STARTINGCREDITS);

        int maxUnits = 0;
        if(currentGame->getGameInitSettings().getGameOptions().maximumNumberOfUnitsOverride >= 0) {
            maxUnits = currentGame->getGameInitSettings().getGameOptions().maximumNumberOfUnitsOverride;
        } else {
            int defaultMaxUnit = std::max(25, 25*(currentGameMap->getSizeX()*currentGameMap->getSizeY())/(64*64));
            int maxUnit = inifile->getIntValue(houseName,"MaxUnit",defaultMaxUnit);
            maxUnits = inifile->getIntValue(houseName,"MaxUnits",maxUnit);
        }

        int quota = inifile->getIntValue(houseName,"Quota",0);

        pGame->house[houseID] = std::make_unique<House>(houseID, startingCredits, maxUnits, houseInfo.team, quota);
        House* pNewHouse = pGame->house[houseID].get();

        // add players
        for(const GameInitSettings::PlayerInfo& playerInfo : houseInfo.playerInfoList) {
            const PlayerFactory::PlayerData* pPlayerData = PlayerFactory::getByPlayerClass(playerInfo.playerClass);
            if(pPlayerData == nullptr) {
                logWarning("Cannot load '" + playerInfo.playerClass + "', using default AI player!");
                pPlayerData = PlayerFactory::getByPlayerClass(DEFAULTAIPLAYERCLASS);
                if(pPlayerData == nullptr) {
                    logWarning("Cannot load default AI player!");
                    continue;
                }
            }

            auto pPlayer = pPlayerData->create(pNewHouse, playerInfo.playerName);

            if( ((pGame->getGameInitSettings().getGameType() != GameType::CustomMultiplayer) && (dynamic_cast<HumanPlayer*>(pPlayer.get()) != nullptr))
                || (playerInfo.playerName == pGame->getLocalPlayerName())) {
                pLocalHouse = pNewHouse;
                pLocalPlayer = dynamic_cast<HumanPlayer*>(pPlayer.get());
            }

            pNewHouse->addPlayer(std::move(pPlayer));
        }
    }
}

/**
    This method loads the choam section of the INI file
*/
void INIMapLoader::loadChoam()
{
    if(!inifile->hasSection("CHOAM")) {
        return;
    }

    for(const INIFile::Key& key : inifile->getSection("CHOAM")) {
        Uint32 unitID = getItemIDByName(key.getKeyName());
        if((unitID == ItemID_Invalid) || !isUnit(unitID)) {
            logWarning(key.getLineNumber(), "Invalid unit string: '" + key.getKeyName() + "'");
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

        for(int i=0;i<NUM_HOUSES;i++) {
            if(pGame->house[i] != nullptr) {
                pGame->house[i]->getChoam().addItem(unitID, num);
            }
        }
    }
}

/**
    This method loads the units specified by the [Units] section.
*/
void INIMapLoader::loadUnits()
{
    if(!inifile->hasSection("UNITS")) {
        return;
    }

    bool nextSpecialUnitIsSonicTank[NUM_HOUSES];
    for(int i=0;i<NUM_HOUSES;i++) {
        nextSpecialUnitIsSonicTank[i] = true;
    }

    for(const INIFile::Key& key : inifile->getSection("UNITS")) {
        if(key.getKeyName().find("ID") == 0) {
            std::string HouseStr, UnitStr, health, PosStr, rotation, mode;
            splitString(key.getStringValue(), HouseStr, UnitStr, health, PosStr, rotation, mode);

            int houseID = getHouseID(HouseStr);
            if(houseID == HOUSE_UNUSED) {
                // skip unit for unused house
                continue;
            } else if(houseID == HOUSE_INVALID) {
                logWarning(key.getLineNumber(), "Invalid house string for '" + UnitStr + "': '" + HouseStr + "'!");
                continue;
            }

            int pos;
            if(!parseString(PosStr, pos) || (pos < 0)) {
                logWarning(key.getLineNumber(), "Invalid position string for '" + UnitStr + "': '" + PosStr + "'!");
                continue;
            }

            int angle;
            if(!parseString(rotation, angle) || (angle < 0) || (angle > 255)) {
                logWarning(key.getLineNumber(), "Invalid rotation string: '" + rotation + "'!");
                angle = 64;
            }
            angle = (angle+16)/32;
            angle = ((NUM_ANGLES - angle) + 2) % NUM_ANGLES;


            int Num2Place = 1;
            int itemID = getItemIDByName(UnitStr);
            if((itemID == ItemID_Invalid) || !isUnit(itemID)) {
                logWarning(key.getLineNumber(), "Invalid unit string: '" + UnitStr + "'!");
                continue;
            }

            if(itemID == Unit_Infantry) {
                // make three
                itemID = Unit_Soldier;
                Num2Place = 3;
            } else if(itemID == Unit_Troopers) {
                // make three
                itemID = Unit_Trooper;
                Num2Place = 3;
            } else if(itemID == Unit_Special) {
                switch(houseID) {

                    case HOUSE_HARKONNEN: {
                        itemID = Unit_Devastator;
                    } break;
                    case HOUSE_ATREIDES: {
                        itemID = Unit_SonicTank;
                    } break;

                    case HOUSE_ORDOS: {
                        itemID = Unit_Deviator;
                    } break;

                    case HOUSE_FREMEN:
                    case HOUSE_SARDAUKAR:
                    case HOUSE_MERCENARY: {
                        if(nextSpecialUnitIsSonicTank[houseID] == true && pGame->objectData.data[Unit_SonicTank][houseID].enabled) {
                            itemID = Unit_SonicTank;
                            nextSpecialUnitIsSonicTank[houseID] = !pGame->objectData.data[Unit_Devastator][houseID].enabled;
                        } else {
                            itemID = Unit_Devastator;
                            nextSpecialUnitIsSonicTank[houseID] = true;
                        }
                    } break;

                    default: {
                        // should never be reached
                        continue;
                    } break;
                }
            }

            if(!pGame->objectData.data[itemID][houseID].enabled) {
                continue;
            }

            int iHealth;
            if(!parseString(health, iHealth) || (iHealth < 0) || (iHealth > 256)) {
                logWarning(key.getLineNumber(), "Invalid health string: '" + health + "'!");
                iHealth = 256;
            }

            FixPoint percentHealth = std::min(FixPoint(iHealth) / 256, FixPoint(1));

            ATTACKMODE attackmode = getAttackModeByName(mode);
            if(attackmode == ATTACKMODE_INVALID) {
                logWarning(key.getLineNumber(), "Invalid attackmode string: '" + mode + "'!");
                attackmode = AREAGUARD;
            }

            for(int i = 0; i < Num2Place; i++) {
                UnitBase* newUnit = getOrCreateHouse(houseID)->placeUnit(itemID, getXPos(pos), getYPos(pos), true);
                if(newUnit == nullptr) {
                    logWarning(key.getLineNumber(), "Invalid or occupied position for '" + UnitStr + "': '" + std::to_string(pos) + "'!");
                    continue;
                } else {
                    newUnit->setHealth((newUnit->getMaxHealth() * percentHealth));
                    newUnit->doSetAttackMode(attackmode);
                    newUnit->setAngle(angle);

                    TankBase* pTankBase = dynamic_cast<TankBase*>(newUnit);
                    if(pTankBase != nullptr) {
                        pTankBase->setTurretAngle(angle);
                    }
                }
            }
        } else {
            logWarning(key.getLineNumber(), "Invalid unit key: '" + key.getKeyName() + "'!");
            continue;
        }
    }
}

/**
    This method loads the structures specified by the [Structures] section.
*/
void INIMapLoader::loadStructures()
{
    if(!inifile->hasSection("STRUCTURES")) {
        return;
    }

    for(const INIFile::Key& key : inifile->getSection("STRUCTURES")) {
        std::string tmpkey = key.getKeyName();
        std::string tmp = key.getStringValue();

        if(tmpkey.compare(0,3,"GEN") == 0) {
            // Gen Object/Structure
            std::string PosStr = tmpkey.substr(3,tmpkey.size()-3);
            int pos;
            if(!parseString(PosStr, pos) || (pos < 0)) {
                logWarning(key.getLineNumber(), "Invalid position string: '" + PosStr + "'!");
                continue;
            }

            std::string HouseStr, BuildingStr;
            splitString(tmp, HouseStr, BuildingStr);

            int houseID = getHouseID(HouseStr);
            if(houseID == HOUSE_UNUSED) {
                // skip structure for unused house
                continue;
            } else if(houseID == HOUSE_INVALID) {
                logWarning(key.getLineNumber(), "Invalid house string for '" + BuildingStr + "': '" + HouseStr + "'!");
                continue;
            }

            if(BuildingStr == "Concrete" && pGame->objectData.data[Structure_Slab1][houseID].enabled) {
                getOrCreateHouse(houseID)->placeStructure(NONE_ID, Structure_Slab1, getXPos(pos), getYPos(pos), true);
            } else if(BuildingStr == "Wall" && pGame->objectData.data[Structure_Wall][houseID].enabled) {
                if(getOrCreateHouse(houseID)->placeStructure(NONE_ID, Structure_Wall, getXPos(pos), getYPos(pos), true) == nullptr) {
                    logWarning(key.getLineNumber(), "Invalid or occupied position for '" + BuildingStr + "': '" + PosStr + "'!");
                    continue;
                }
            } else if((BuildingStr != "Concrete") && (BuildingStr != "Wall")) {
                logWarning(key.getLineNumber(), "Invalid building string: '" + BuildingStr + "'!");
                continue;
            }
        } else if(tmpkey.compare(0,2,"ID") == 0) {
            // other structure
            std::string HouseStr, BuildingStr, health, PosStr;
            splitString(tmp, HouseStr, BuildingStr, health, PosStr);

            int pos;
            if(!parseString(PosStr, pos) || (pos < 0)) {
                logWarning(key.getLineNumber(), "Invalid position string for '" + BuildingStr + "': '" + PosStr + "'!");
                continue;
            }

            int houseID = getHouseID(HouseStr);
            if(houseID == HOUSE_UNUSED) {
                // skip structure for unused house
                continue;
            } else if(houseID == HOUSE_INVALID) {
                logWarning(key.getLineNumber(), "Invalid house string for '" + BuildingStr + "': '" + HouseStr + "'!");
                continue;
            }

            int iHealth;
            if(!parseString(health, iHealth) || (iHealth < 0) || (iHealth > 256)) {
                logWarning(key.getLineNumber(), "Invalid health string: '" + health + "'!");
                iHealth = 256;
            }
            FixPoint percentHealth = std::min(FixPoint(iHealth) / 256, FixPoint(1));

            int itemID = getItemIDByName(BuildingStr);

            if((itemID == ItemID_Invalid) || !isStructure(itemID)) {
                logWarning(key.getLineNumber(), "Invalid building string: '" + BuildingStr + "'!");
                continue;
            }

            if (itemID != 0 && pGame->objectData.data[itemID][houseID].enabled) {
                ObjectBase* newStructure = getOrCreateHouse(houseID)->placeStructure(NONE_ID, itemID, getXPos(pos), getYPos(pos), true);
                if(newStructure == nullptr) {
                    logWarning(key.getLineNumber(), "Invalid or occupied position for '" + BuildingStr + "': '" + PosStr + "'!");
                    continue;
                } else {
                    newStructure->setHealth(newStructure->getMaxHealth() * percentHealth);
                }
            }
        } else {
            logWarning(key.getLineNumber(), "Invalid structure key: '" + tmpkey + "'!");
            continue;
        }
    }
}

/**
    This method loads the reinforcements from the [REINFORCEMENTS] section.
*/
void INIMapLoader::loadReinforcements()
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

        int houseID = getHouseID(strHouseName);
        if(houseID == HOUSE_UNUSED) {
            // skip reinforcement for unused house
            continue;
        } else if(houseID == HOUSE_INVALID) {
            logWarning(key.getLineNumber(), "Invalid house string: '" + strHouseName + "'!");
            continue;
        }

        int Num2Drop = 1;
        Uint32 itemID = getItemIDByName(strUnitName);
        if((itemID == ItemID_Invalid) || !isUnit(itemID)) {
            logWarning(key.getLineNumber(), "Invalid unit string: '" + strUnitName + "'!");
            continue;
        }

        if(itemID == Unit_Infantry) {
            // make three
            itemID = Unit_Soldier;
            Num2Drop = 3;
        } else if(itemID == Unit_Troopers) {
            // make three
            itemID = Unit_Trooper;
            Num2Drop = 3;
        }

        if(!pGame->objectData.data[itemID][houseID].enabled) {
            continue;
        }

        DropLocation dropLocation = getDropLocationByName(strDropLocation);
        if(dropLocation == Drop_Invalid) {
            logWarning(key.getLineNumber(), "Invalid drop location string: '" + strDropLocation + "'!");
            dropLocation = Drop_Homebase;
        }

        Uint32 droptime;
        if(!parseString(strTime, droptime)) {
            logWarning(key.getLineNumber(), "Invalid drop time string: '" + strTime + "'!");
            continue;
        }
        Uint32 dropCycle = MILLI2CYCLES(droptime * 60 * 1000);

        bool bRepeat = (strTime.rfind('+') == (strTime.length() - 1)) || (strPlus == "+");

        for(int i=0;i<Num2Drop;i++) {
            // check if there is a similar trigger at the same time

            bool bInserted = false;
            for(const auto& pTrigger : pGame->getTriggerManager().getTriggers()) {
                ReinforcementTrigger* pReinforcementTrigger = dynamic_cast<ReinforcementTrigger*>(pTrigger.get());

                if(pReinforcementTrigger != nullptr
                    && pReinforcementTrigger->getCycleNumber() == dropCycle
                    && pReinforcementTrigger->getHouseID() == houseID
                    && pReinforcementTrigger->isRepeat() == bRepeat
                    && pReinforcementTrigger->getDropLocation() == dropLocation) {

                    // add the new reinforcement to this reinforcement (call only one carryall)
                    pReinforcementTrigger->addUnit(itemID);
                    bInserted = true;
                    break;
                }
            }

            if(bInserted == false) {
                getOrCreateHouse(houseID);  // create house if not yet available
                pGame->getTriggerManager().addTrigger(std::make_unique<ReinforcementTrigger>(houseID, itemID, dropLocation, bRepeat, dropCycle));
            }
        }
    }
}

/**
    This method loads the AI teams from the [TEAMS] section.
*/
void INIMapLoader::loadAITeams()
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

        int houseID = getHouseID(strHouseName);
        if(houseID == HOUSE_UNUSED) {
            // skip reinforcement for unused house
            continue;
        } else if(houseID == HOUSE_INVALID) {
            logWarning(key.getLineNumber(), "Invalid house string: '" + strHouseName + "'!");
            continue;
        }

        AITeamBehavior aiTeamBehavior = getAITeamBehaviorByName(strAITeamBehavior);
        if(aiTeamBehavior == AITeamBehavior_Invalid) {
            logWarning(key.getLineNumber(), "Invalid team behavior string: '" + strAITeamBehavior + "'!");
            aiTeamBehavior = AITeamBehavior_Normal;
        }

        AITeamType aiTeamType = getAITeamTypeByName(strAITeamType);
        if(aiTeamType == AITeamType_Invalid) {
            logWarning(key.getLineNumber(), "Invalid team type string: '" + strAITeamType + "'!");
            aiTeamType = AITeamType_Foot;
        }

        int minUnits;
        if(!parseString(strMinUnits, minUnits)) {
            logWarning(key.getLineNumber(), "Invalid min units string: '" + strMinUnits + "'!");
            continue;
        }

        int maxUnits;
        if(!parseString(strMaxUnits, maxUnits)) {
            logWarning(key.getLineNumber(), "Invalid max units string: '" + strMaxUnits + "'!");
            continue;
        }

        getOrCreateHouse(houseID)->addAITeam(aiTeamBehavior, aiTeamType, minUnits, maxUnits);
    }
}

/**
    This method sets up the view specified by "TacticalPos" in the [BASIC] section.
*/
void INIMapLoader::loadView()
{
    if(inifile->hasKey("BASIC", "TacticalPos")) {
        int tacticalPosInt = inifile->getIntValue("BASIC","TacticalPos",-10000) + 64*5 + 7;
        Coord tacticalPos(getXPos(tacticalPosInt), getYPos(tacticalPosInt));

        if(tacticalPos.x < 0 || tacticalPos.x >= sizeX || tacticalPos.y < 0 || tacticalPos.y >= sizeY) {
            logWarning(inifile->getKey("BASIC", "TacticalPos")->getLineNumber(), "Invalid TacticalPos: '" + std::to_string(tacticalPosInt) + "'!");
            pGame->setupView();
        } else {
            screenborder->setNewScreenCenter(tacticalPos*TILESIZE);
        }
    } else {
        pGame->setupView();
    }
}

/**
    This method returns the house object of the specified house id. If it does not already exist a new AI Player is created and returned.
    \param houseID the house to return or create
    \return the house specified by house
*/
House* INIMapLoader::getOrCreateHouse(int houseID) {
    if(pGame->house[houseID]) {
        return pGame->house[houseID].get();
    } else {
        Uint8 team = 0;
        if(pGame->gameType == GameType::Campaign || pGame->gameType == GameType::Skirmish) {
            // in campaign all "other" units are in the same team as the AI
            team = 2;
        }

        int maxUnits = 0;
        if(currentGame->getGameInitSettings().getGameOptions().maximumNumberOfUnitsOverride >= 0) {
            maxUnits = currentGame->getGameInitSettings().getGameOptions().maximumNumberOfUnitsOverride;
        } else {
            maxUnits = std::min(40, 20 * (currentGameMap->getSizeX() * currentGameMap->getSizeY()) / (32 * 32));
        }
        auto pNewHouse = std::make_unique<House>(houseID, 0, maxUnits, team, 0);

        const GameInitSettings::HouseInfoList& houseInfoList = pGame->getGameInitSettings().getHouseInfoList();

        for(const GameInitSettings::HouseInfo& houseInfo : houseInfoList) {
            if(houseInfo.houseID == houseID) {
                for(const GameInitSettings::PlayerInfo& playerInfo : houseInfo.playerInfoList) {
                    const PlayerFactory::PlayerData* pPlayerData = PlayerFactory::getByPlayerClass(playerInfo.playerClass);
                    if(pPlayerData == nullptr) {
                        logWarning("Cannot load '" + playerInfo.playerClass + "', using default AI player!");
                        pPlayerData = PlayerFactory::getByPlayerClass(DEFAULTAIPLAYERCLASS);
                        if(pPlayerData == nullptr) {
                            logWarning("Cannot load default AI player!");
                            continue;
                        }
                    }

                    auto pPlayer = pPlayerData->create(pNewHouse.get(), playerInfo.playerName);

                    if(playerInfo.playerName == pGame->getLocalPlayerName()) {
                        pLocalHouse = pNewHouse.get();
                        pLocalPlayer = dynamic_cast<HumanPlayer*>(pPlayer.get());
                    }

                    pNewHouse->addPlayer(std::move(pPlayer));
                }
                break;
            }
        }

        /*
        // probably not a good idea to treat any "anonymous" house as an AI player
        if(pNewHouse->getPlayerList().empty()) {
            const PlayerFactory::PlayerData* pPlayerData = PlayerFactory::getByPlayerClass(DEFAULTAIPLAYERCLASS);
            Player* pPlayer = pPlayerData->create(pNewHouse.get(), getHouseNameByNumber((HOUSETYPE) pNewHouse->getHouseID()));
            pNewHouse->addPlayer(std::unique_ptr<Player>(pPlayer));
        }
        */

        pGame->house[houseID] = std::move(pNewHouse);
        return pGame->house[houseID].get();
    }
}

HOUSETYPE INIMapLoader::getHouseID(const std::string& name) {
    std::string lowerName = strToLower(name);

    if(housename2house.count(lowerName) > 0) {
        return housename2house[lowerName];
    } else {
        return getHouseByName(lowerName);
    }
}
