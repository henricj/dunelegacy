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

#include <fmt/printf.h>
#include <misc/exceptions.h>

#include <sand.h>
#include <globals.h>

#include <algorithm>

#include <gsl/gsl>

INIMapLoader::INIMapLoader(Game* pGame, const std::filesystem::path& mapname, const std::string& mapdata)
 : INIMap(pGame->gameType, mapname, mapdata), pGame(pGame)
{ }

INIMapLoader::~INIMapLoader() = default;


/**
    Loads a map from an INI-File.
*/
std::unique_ptr<Map> INIMapLoader::load() {
    checkFeatures();

    loadMap();

    // TODO: Facepalm.  globals...
    currentGameMap = map.get();
    auto cleanup   = gsl::finally([]{ currentGameMap = nullptr; });

    const GameContext context{*pGame, *map, pGame->getObjectManager()};

    loadHouses(context);
    loadUnits(context);
    loadStructures(context);
    loadReinforcements(context);
    loadAITeams(context);
    loadView(context);
    loadChoam();

    return std::move(map);
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

    const auto timeout = inifile->getIntValue("BASIC","TIMEOUT",0);

    if((timeout != 0) && ((pGame->winFlags & WINLOSEFLAGS_TIMEOUT) != 0)) {
        pGame->getTriggerManager().addTrigger(std::make_unique<TimeoutTrigger>(MILLI2CYCLES(timeout * 60 * 1000)));
    }

    if(version < 2) {
        if(!inifile->hasKey("MAP","Seed")) {
            logError("Cannot find seed value for this map!");
        }

        // old map format with seed value

        if(!inifile->hasKey("BASIC","MapScale")) {
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

        int SeedNum = inifile->getIntValue("MAP","Seed",-1);
        Uint16 SeedMap[64*64];
        createMapWithSeed(SeedNum,SeedMap);

        map = std::make_unique<Map>(sizeX, sizeY);

        GameContext context{*pGame, *map, pGame->getObjectManager()};

        for(int j = 0; j < map->getSizeY(); j++) {
            for (int i = 0; i < map->getSizeX(); i++) {
                auto type = Terrain_Sand;
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

                map->getTile(i, j)->setType(context, type);
            }
        }

        map->createSandRegions();

        const auto BloomString = inifile->getStringValue("MAP", "Bloom");
        if(!BloomString.empty()) {
            std::vector<std::string> BloomPositions = splitStringToStringVector(BloomString);

            for(const auto& BloomPosition : BloomPositions) {
                // set bloom
                int BloomPos = 0;
                if(parseString(BloomPosition, BloomPos)) {
                    int   xpos = getXPos(BloomPos);
                    int   ypos = getYPos(BloomPos);
                    auto* tile = map->tryGetTile(xpos, ypos);
                    if(tile) {
                        tile->setType(context, Terrain_SpiceBloom);
                    } else {
                        logWarning(inifile->getKey("MAP", "Bloom")->getLineNumber(),
                                   "Spice bloom position '" + BloomPosition + "' outside map!");
                    }
                } else {
                    logWarning(inifile->getKey("MAP", "Bloom")->getLineNumber(),
                               "Invalid spice bloom position: '" + BloomPosition + "'");
                }
            }
        }

        const auto SpecialString = inifile->getStringValue("MAP","Special");
        if(!SpecialString.empty()) {
            std::vector<std::string> SpecialPositions = splitStringToStringVector(SpecialString);

            for(const auto& SpecialPosition : SpecialPositions) {
                // set special
                int SpecialPos = 0;
                if(parseString(SpecialPosition, SpecialPos)) {
                    int xpos = getXPos(SpecialPos);
                    int ypos = getYPos(SpecialPos);
                    if(!map->trySetTileType(context, xpos, ypos, Terrain_SpecialBloom)) {
                        logWarning(inifile->getKey("MAP", "Special")->getLineNumber(), "Special bloom position '" + SpecialPosition + "' outside map!");
                    }
                } else {
                    logWarning(inifile->getKey("MAP", "Special")->getLineNumber(), "Invalid special bloom position: '" + SpecialPosition + "'");
                }
            }

        }

        const auto FieldString = inifile->getStringValue("MAP","Field");
        if(!FieldString.empty()) {
            const auto FieldPositions = splitStringToStringVector(FieldString);

            for(const auto& FieldPosition : FieldPositions) {
                // set bloom
                int FieldPos = 0;
                if(parseString(FieldPosition, FieldPos)) {
                    int xpos = getXPos(FieldPos);
                    int ypos = getYPos(FieldPos);

                    map->createSpiceField(context, Coord(xpos, ypos), 5, true);
                } else {
                    logWarning(inifile->getKey("MAP", "Field")->getLineNumber(), "Invalid spice field position: '" + FieldPosition + "'");
                }
            }
        }

    } else {
        // new map format with saved map

        if((!inifile->hasKey("MAP","SizeX")) || (!inifile->hasKey("MAP","SizeY"))) {
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

        map = std::make_unique<Map>(sizeX, sizeY);

        GameContext context{*pGame, *map, pGame->getObjectManager()};

        for(int y = 0; y < sizeY; y++) {
            std::string rowKey = fmt::sprintf("%.3d", y);

            if(!inifile->hasKey("MAP", rowKey)) {
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
                auto type = Terrain_Sand;

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

                auto* const tile = map->tryGetTile(x, y);
                if(tile) tile->setType(context, type);
            }
        }

        map->createSandRegions();
    }

    screenborder->adjustScreenBorderToMapsize(map->getSizeX(), map->getSizeY());
}

/**
    This method loads the houses on the map specified by the various house sections in the INI file ([Atreides], [Ordos], [Harkonnen]).
*/
void INIMapLoader::loadHouses(const GameContext& context)
{
    const GameInitSettings::HouseInfoList& houseInfoList = pGame->getGameInitSettings().getHouseInfoList();

    // find "player?" sections
    std::vector<std::string> playerSectionsOnMap;
    for(int i = 1; i <= static_cast<int>(HOUSETYPE::NUM_HOUSES); i++) {
        std::string sectionname = "player" + std::to_string(i);
        if(inifile->hasSection(sectionname)) {
            playerSectionsOnMap.push_back(sectionname);
        }
    }

    // find unbounded houses
    std::vector<HOUSETYPE> unboundedHouses;

    for(int h = 0; h < static_cast<int>(HOUSETYPE::NUM_HOUSES); h++) {
        bool bFound = false;
        for(const auto& houseInfo : houseInfoList) {
            if(houseInfo.houseID == static_cast<HOUSETYPE>(h)) {
                bFound = true;
                break;
            }
        }

        const auto houseName = getHouseNameByNumber(static_cast<HOUSETYPE>(h));
        if((!bFound) && (inifile->hasSection(houseName) || (!playerSectionsOnMap.empty()))) {
            unboundedHouses.push_back(static_cast<HOUSETYPE>(h));
        }
    }

    // init housename2house mapping with every house section marked as unused
    for(int i = 0; i < static_cast<int>(HOUSETYPE::NUM_HOUSES); i++) {
        auto houseName = getHouseNameByNumber(static_cast<HOUSETYPE>(i));
        convertToLower(houseName);

        if(inifile->hasSection(houseName)) {
            housename2house[houseName] = HOUSETYPE::HOUSE_UNUSED;
        }
    }

    // init housename2house mapping with every player section on map marked as unused
    for(const auto& playSection : playerSectionsOnMap) {
        housename2house[playSection] = HOUSETYPE::HOUSE_UNUSED;
    }

    // now set up all the houses
    for(const GameInitSettings::HouseInfo& houseInfo : houseInfoList) {
        HOUSETYPE houseID;

        pGame->houseInfoListSetup.push_back(houseInfo);

        if(houseInfo.houseID == HOUSETYPE::HOUSE_INVALID) {
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

        if(!inifile->hasSection(houseName)) {
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
        if(pGame->getGameInitSettings().getGameOptions().maximumNumberOfUnitsOverride >= 0) {
            maxUnits = pGame->getGameInitSettings().getGameOptions().maximumNumberOfUnitsOverride;
        } else {
            int defaultMaxUnit = std::max(25, 25*(sizeX*sizeY)/(64*64));
            int maxUnit = inifile->getIntValue(houseName,"MaxUnit",defaultMaxUnit);
            maxUnits = inifile->getIntValue(houseName,"MaxUnits",maxUnit);
        }

        int quota = inifile->getIntValue(houseName,"Quota",0);

        pGame->house[static_cast<int>(houseID)] = std::make_unique<House>(
            context, houseID, startingCredits, maxUnits, houseInfo.team, quota);
        auto *const pNewHouse = pGame->getHouse(houseID);

        // add players
        for(const auto& playerInfo : houseInfo.playerInfoList) {
            const auto *pPlayerData = PlayerFactory::getByPlayerClass(playerInfo.playerClass);
            if(pPlayerData == nullptr) {
                logWarning("Cannot load '" + playerInfo.playerClass + "', using default AI player!");
                pPlayerData = PlayerFactory::getByPlayerClass(DEFAULTAIPLAYERCLASS);
                if(pPlayerData == nullptr) {
                    logWarning("Cannot load default AI player!");
                    continue;
                }
            }

            auto pPlayer = pPlayerData->create(context, pNewHouse, playerInfo.playerName);

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
        const auto unitID = getItemIDByName(key.getKeyName());
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

        pGame->for_each_house([=](auto& house) { house.getChoam().addItem(unitID, num); });
    }
}

/**
    This method loads the units specified by the [Units] section.
*/
void INIMapLoader::loadUnits(const GameContext& context) {
    if(!inifile->hasSection("UNITS")) {
        return;
    }

    std::array<bool, static_cast<int>(HOUSETYPE::NUM_HOUSES)> nextSpecialUnitIsSonicTank;
    std::fill(nextSpecialUnitIsSonicTank.begin(), nextSpecialUnitIsSonicTank.end(), true);

    for(const INIFile::Key& key : inifile->getSection("UNITS")) {
        if(key.getKeyName().find("ID") == 0) {
            std::string HouseStr, UnitStr, health, PosStr, rotation, mode;
            splitString(key.getStringValue(), HouseStr, UnitStr, health, PosStr, rotation, mode);

            const auto houseID = getHouseID(HouseStr);
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
            const auto angle = normalizeAngle(static_cast<ANGLETYPE>(int_angle));


            int Num2Place = 1;
            ItemID_enum itemID = getItemIDByName(UnitStr);
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

                    case HOUSETYPE::HOUSE_HARKONNEN: {
                        itemID = Unit_Devastator;
                    } break;
                    case HOUSETYPE::HOUSE_ATREIDES: {
                        itemID = Unit_SonicTank;
                    } break;

                    case HOUSETYPE::HOUSE_ORDOS: {
                        itemID = Unit_Deviator;
                    } break;

                    case HOUSETYPE::HOUSE_FREMEN:
                    case HOUSETYPE::HOUSE_SARDAUKAR:
                    case HOUSETYPE::HOUSE_MERCENARY: {
                        if(nextSpecialUnitIsSonicTank[static_cast<int>(houseID)] &&
                           pGame->objectData.data[Unit_SonicTank][static_cast<int>(houseID)].enabled) {
                            itemID = Unit_SonicTank;
                            nextSpecialUnitIsSonicTank[static_cast<int>(houseID)] =
                                !pGame->objectData.data[Unit_Devastator][static_cast<int>(houseID)].enabled;
                        } else {
                            itemID                                                = Unit_Devastator;
                            nextSpecialUnitIsSonicTank[static_cast<int>(houseID)] = true;
                        }
                    } break;

                    default: {
                        // should never be reached
                        continue;
                    } break;
                }
            }

            if(!pGame->objectData.data[itemID][static_cast<int>(houseID)].enabled) {
                continue;
            }

            int iHealth = 0;
            if(!parseString(health, iHealth) || (iHealth < 0) || (iHealth > 256)) {
                logWarning(key.getLineNumber(), "Invalid health string: '" + health + "'!");
                iHealth = 256;
            }

            const auto percentHealth = std::min(FixPoint(iHealth) / 256, FixPoint(1));

            ATTACKMODE attackmode = getAttackModeByName(mode);
            if(attackmode == ATTACKMODE_INVALID) {
                logWarning(key.getLineNumber(), "Invalid attackmode string: '" + mode + "'!");
                attackmode = AREAGUARD;
            }

            for(int i = 0; i < Num2Place; i++) {
                auto* newUnit = getOrCreateHouse(context, houseID)->placeUnit(static_cast<ItemID_enum>(itemID), getXPos(pos), getYPos(pos), true);
                if(newUnit == nullptr) {
                    logWarning(key.getLineNumber(), fmt::format("Invalid or occupied position for '{}': '{}' ({}x{})!",
                                                                UnitStr, PosStr, getXPos(pos), getYPos(pos)));
                    continue;
                } else {
                    newUnit->setHealth((newUnit->getMaxHealth() * percentHealth));
                    newUnit->doSetAttackMode(context, attackmode);
                    newUnit->setAngle(angle);

                    if(auto* pTankBase = dune_cast<TankBase>(newUnit)) { pTankBase->setTurretAngle(angle); }
                }
            } else {
                logWarning(key.getLineNumber(), fmt::format("Unable to get or create house {}!", houseID));
            }
        } else {
            logWarning(key.getLineNumber(), "Invalid unit key: '" + key.getKeyName() + "'!");
        }
    }
}

/**
    This method loads the structures specified by the [Structures] section.
*/
void INIMapLoader::loadStructures(const GameContext& context) {
    if(!inifile->hasSection("STRUCTURES")) {
        return;
    }

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

            const auto houseID = getHouseID(HouseStr);
            if(houseID == HOUSETYPE::HOUSE_UNUSED) {
                // skip structure for unused house
                continue;
            } else if(houseID == HOUSETYPE::HOUSE_INVALID) {
                logWarning(key.getLineNumber(), "Invalid house string for '" + BuildingStr + "': '" + HouseStr + "'!");
                continue;
            }

            if(BuildingStr == "Concrete" && pGame->objectData.data[Structure_Slab1][static_cast<int>(houseID)].enabled) {
                getOrCreateHouse(context, houseID)->placeStructure(NONE_ID, Structure_Slab1, getXPos(pos), getYPos(pos), true);
            } else if(BuildingStr == "Wall" && pGame->objectData.data[Structure_Wall][static_cast<int>(houseID)].enabled) {
                if(getOrCreateHouse(context, houseID)->placeStructure(NONE_ID, Structure_Wall, getXPos(pos), getYPos(pos), true) == nullptr) {
                    logWarning(key.getLineNumber(), fmt::format("Invalid or occupied position for '{}': '{}'!", BuildingStr, PosStr));
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

            int pos = 0;
            if(!parseString(PosStr, pos) || (pos < 0)) {
                logWarning(key.getLineNumber(), fmt::format("Invalid position string for '{}': '{}'!", BuildingStr, PosStr));
                continue;
            }

            const auto houseID = getHouseID(HouseStr);
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
            FixPoint percentHealth = std::min(FixPoint(iHealth) / 256, FixPoint(1));

            ItemID_enum itemID = getItemIDByName(BuildingStr);

            if((itemID == ItemID_Invalid) || !isStructure(itemID)) {
                logWarning(key.getLineNumber(), "Invalid building string: '" + BuildingStr + "'!");
                continue;
            }

            if (itemID != 0 && pGame->objectData.data[itemID][static_cast<int>(houseID)].enabled) {
                ObjectBase* newStructure = getOrCreateHouse(context, houseID)->placeStructure(NONE_ID, static_cast<ItemID_enum>(itemID), getXPos(pos), getYPos(pos), true);
                if(newStructure == nullptr) {
                    logWarning(key.getLineNumber(), fmt::format("Invalid or occupied position for '{}': '{}'!", BuildingStr, PosStr));
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
void INIMapLoader::loadReinforcements(const GameContext& context) {
    if(!inifile->hasSection("REINFORCEMENTS")) {
        return;
    }

    for(const auto& key : inifile->getSection("REINFORCEMENTS")) {
        std::string strHouseName;
        std::string strUnitName;
        std::string strDropLocation;
        std::string strTime;
        std::string strPlus;

        if(!splitString(key.getStringValue(), strHouseName, strUnitName, strDropLocation, strTime)) {
            if(!splitString(key.getStringValue(), strHouseName, strUnitName, strDropLocation, strTime, strPlus)) {
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

        auto Num2Drop = 1;
        auto itemID = getItemIDByName(strUnitName);
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

        if(!pGame->objectData.data[itemID][static_cast<int>(houseID)].enabled) {
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
        const auto dropCycle = MILLI2CYCLES(droptime * 60 * 1000);

        const auto bRepeat = (strTime.rfind('+') == (strTime.length() - 1)) || (strPlus == "+");

        for(auto i=0;i<Num2Drop;i++) {
            // check if there is a similar trigger at the same time

            bool bInserted = false;
            for(const auto& pTrigger : pGame->getTriggerManager().getTriggers()) {
                auto* pReinforcementTrigger = dynamic_cast<ReinforcementTrigger*>(pTrigger.get());

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

            if(!bInserted) {
                getOrCreateHouse(context, houseID);  // create house if not yet available
                pGame->getTriggerManager().addTrigger(std::make_unique<ReinforcementTrigger>(houseID, itemID, dropLocation, bRepeat, dropCycle));
            }
        }
    }
}

/**
    This method loads the AI teams from the [TEAMS] section.
*/
void INIMapLoader::loadAITeams(const GameContext& context) {
    if(!inifile->hasSection("TEAMS")) {
        return;
    }

    for(const INIFile::Key& key : inifile->getSection("TEAMS")) {
        std::string strHouseName;
        std::string strAITeamBehavior;
        std::string strAITeamType;
        std::string strMinUnits;
        std::string strMaxUnits;

        if(!splitString(key.getStringValue(), strHouseName, strAITeamBehavior, strAITeamType, strMinUnits, strMaxUnits)) {
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

        getOrCreateHouse(context, houseID)->addAITeam(aiTeamBehavior, aiTeamType, minUnits, maxUnits);
    }
}

/**
    This method sets up the view specified by "TacticalPos" in the [BASIC] section.
*/
void INIMapLoader::loadView(const GameContext& context)
{
    if(inifile->hasKey("BASIC", "TacticalPos")) {
        const auto tacticalPosInt = inifile->getIntValue("BASIC","TacticalPos",-10000) + 64*5 + 7;
        const Coord tacticalPos(getXPos(tacticalPosInt), getYPos(tacticalPosInt));

        if(tacticalPos.x < 0 || tacticalPos.x >= sizeX || tacticalPos.y < 0 || tacticalPos.y >= sizeY) {
            logWarning(inifile->getKey("BASIC", "TacticalPos")->getLineNumber(), "Invalid TacticalPos: '" + std::to_string(tacticalPosInt) + "'!");
            context.game.setupView(context);
        } else {
            screenborder->setNewScreenCenter(tacticalPos*TILESIZE);
        }
    } else {
        context.game.setupView(context);
    }
}

/**
    This method returns the house object of the specified house id. If it does not already exist a new AI Player is created and returned.
    \param houseID the house to return or create
    \return the house specified by house
*/
House* INIMapLoader::getOrCreateHouse(const GameContext& context, HOUSETYPE houseID) {
    auto& pHouse = pGame->house[pGame->getHouseIndex(houseID)];

    if(pHouse)
        return pHouse.get();

    Uint8 team = 0;
    if(pGame->gameType == GameType::Campaign || pGame->gameType == GameType::Skirmish) {
        // in campaign all "other" units are in the same team as the AI
        team = 2;
    }

    int maxUnits = 0;
    if(pGame->getGameInitSettings().getGameOptions().maximumNumberOfUnitsOverride >= 0) {
        maxUnits = pGame->getGameInitSettings().getGameOptions().maximumNumberOfUnitsOverride;
    } else {
        maxUnits = std::max(25, 25*(map->getSizeX()*map->getSizeY())/(64*64));
    }
    auto pNewHouse = std::make_unique<House>(context, houseID, 0, maxUnits, team, 0);

    const auto& houseInfoList = pGame->getGameInitSettings().getHouseInfoList();

    for(const auto& houseInfo : houseInfoList) {
        if(houseInfo.houseID != houseID) continue;

        for(const auto& playerInfo : houseInfo.playerInfoList) {
            const auto *pPlayerData = PlayerFactory::getByPlayerClass(playerInfo.playerClass);
            if(pPlayerData == nullptr) {
                logWarning("Cannot load '" + playerInfo.playerClass + "', using default AI player!");
                pPlayerData = PlayerFactory::getByPlayerClass(DEFAULTAIPLAYERCLASS);
                if(pPlayerData == nullptr) {
                    logWarning("Cannot load default AI player!");
                    continue;
                }
            }

            auto pPlayer = pPlayerData->create(context, pNewHouse.get(), playerInfo.playerName);

            if(playerInfo.playerName == pGame->getLocalPlayerName()) {
                pLocalHouse = pNewHouse.get();
                pLocalPlayer = dynamic_cast<HumanPlayer*>(pPlayer.get());
            }

            pNewHouse->addPlayer(std::move(pPlayer));
        }

        break;
    }

    /*
        // probably not a good idea to treat any "anonymous" house as an AI player
        if(pNewHouse->getPlayerList().empty()) {
            const PlayerFactory::PlayerData* pPlayerData = PlayerFactory::getByPlayerClass(DEFAULTAIPLAYERCLASS);
            Player* pPlayer = pPlayerData->create(pNewHouse.get(), getHouseNameByNumber((HOUSETYPE) pNewHouse->getHouseID()));
            pNewHouse->addPlayer(std::unique_ptr<Player>(pPlayer));
        }
        */

    pHouse = std::move(pNewHouse);

    return pHouse.get();
}

HOUSETYPE INIMapLoader::getHouseID(const std::string& name) {
    const auto lowerName = strToLower(name);

    if(housename2house.count(lowerName) > 0) {
        return housename2house[lowerName];
    }         return getHouseByName(lowerName);

   
}
