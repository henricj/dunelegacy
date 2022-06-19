#include <INIMap/INIMapLoader.h>

#include <FileClasses/FileManager.h>

#include <Game.h>
#include <GameInitSettings.h>
#include <House.h>
#include <Map.h>
#include <MapSeed.h>
#include <RadarView.h>
#include <ScreenBorder.h>
#include <players/AIPlayer.h>
#include <players/HumanPlayer.h>
#include <players/Player.h>
#include <players/PlayerFactory.h>

#include <structures/StructureBase.h>
#include <units/TankBase.h>
#include <units/UnitBase.h>

#include <Trigger/ReinforcementTrigger.h>
#include <Trigger/TimeoutTrigger.h>

#include <fmt/printf.h>
#include <misc/exceptions.h>

#include <globals.h>
#include <sand.h>

#include <gsl/gsl>

#include <algorithm>
#include <limits>
#include <utility>

using namespace std::literals;

INIMapLoader::INIMapLoader(Game* pGame, const std::filesystem::path& mapname, const std::string& mapdata)
    : INIMap(pGame->gameType, mapname, mapdata), pGame(pGame) { }

INIMapLoader::~INIMapLoader() = default;

/**
    Loads a map from an INI-File.
*/
std::unique_ptr<Map> INIMapLoader::load() {
    checkFeatures();

    loadMap();

    // TODO: Facepalm.  globals...
    dune::globals::currentGameMap = map.get();
    auto cleanup                  = gsl::finally([] { dune::globals::currentGameMap = nullptr; });

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
    version_ = inifile_->getIntValue("BASIC", "Version", 1);

    pGame->winFlags  = inifile_->getIntValue("BASIC", "WinFlags", 3);
    pGame->loseFlags = inifile_->getIntValue("BASIC", "LoseFlags", 1);

    if (pGame->techLevel == 0) {
        pGame->techLevel = inifile_->getIntValue("BASIC", "TechLevel", 8);
    }

    const auto timeout = inifile_->getIntValue("BASIC", "TIMEOUT", 0);

    if ((timeout != 0) && ((pGame->winFlags & WINLOSEFLAGS_TIMEOUT) != 0)) {
        pGame->getTriggerManager().addTrigger(std::make_unique<TimeoutTrigger>(MILLI2CYCLES(timeout * 60 * 1000)));
    }

    if (version_ < 2) {
        if (!inifile_->hasKey("MAP", "Seed")) {
            logError("Cannot find seed value for this map!");
        }

        // old map format with seed value

        if (!inifile_->hasKey("BASIC", "MapScale")) {
            logError("Cannot find MapScale for this map!");
        }

        switch (const auto mapscale = inifile_->getIntValue("BASIC", "MapScale", 0)) {
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

        int SeedNum = inifile_->getIntValue("MAP", "Seed", -1);
        uint16_t SeedMap[64 * 64];
        createMapWithSeed(SeedNum, SeedMap);

        map = std::make_unique<Map>(*pGame, sizeX_, sizeY_);

        GameContext context{*pGame, *map, pGame->getObjectManager()};

        for (int j = 0; j < map->getSizeY(); j++) {
            for (int i = 0; i < map->getSizeX(); i++) {
                auto type = TERRAINTYPE::Terrain_Sand;
                const auto seedmaptype =
                    static_cast<uint8_t>(SeedMap[(j + logicalOffsetY_) * 64 + i + logicalOffsetX_] >> 4u);
                switch (seedmaptype) {

                    case 0x7: /* Sand */ type = TERRAINTYPE::Terrain_Sand; break;

                    case 0x2: /* Building */
                    case 0x8: /* Rock */ type = TERRAINTYPE::Terrain_Rock; break;

                    case 0x9: /* Dunes */ type = TERRAINTYPE::Terrain_Dunes; break;

                    case 0xa: /* Mountain */ type = TERRAINTYPE::Terrain_Mountain; break;

                    case 0xb: /* Spice */ type = TERRAINTYPE::Terrain_Spice; break;

                    case 0xc: /* ThickSpice */ type = TERRAINTYPE::Terrain_ThickSpice; break;

                    default:
                        logWarning(inifile_->getLineNumber("MAP", "Seed"), "Unknown map type '%d' for tile (%d, %d)!",
                                   seedmaptype, i, j);
                        type = TERRAINTYPE::Terrain_Sand;
                        break;
                }

                map->getTile(i, j)->setType(context, type);
            }
        }

        map->createSandRegions();

        const auto BloomString = inifile_->getStringValue("MAP", "Bloom");
        if (!BloomString.empty()) {
            const auto BloomPositions = splitStringToStringVector(BloomString);

            for (const auto& BloomPosition : BloomPositions) {
                // set bloom
                auto BloomPos = 0;
                if (parseString(BloomPosition, BloomPos)) {
                    int xpos   = getXPos(BloomPos);
                    int ypos   = getYPos(BloomPos);
                    auto* tile = map->tryGetTile(xpos, ypos);
                    if (tile) {
                        tile->setType(context, TERRAINTYPE::Terrain_SpiceBloom);
                    } else {
                        logWarning(inifile_->getLineNumber("MAP", "Bloom"),
                                   "Spice bloom position '" + BloomPosition + "' outside map!");
                    }
                } else {
                    logWarning(inifile_->getLineNumber("MAP", "Bloom"),
                               "Invalid spice bloom position: '" + BloomPosition + "'");
                }
            }
        }

        const auto SpecialString = inifile_->getStringValue("MAP", "Special");
        if (!SpecialString.empty()) {
            const auto SpecialPositions = splitStringToStringVector(SpecialString);

            for (const auto& SpecialPosition : SpecialPositions) {
                // set special
                int SpecialPos = 0;
                if (parseString(SpecialPosition, SpecialPos)) {
                    int xpos = getXPos(SpecialPos);
                    int ypos = getYPos(SpecialPos);
                    if (!map->trySetTileType(context, xpos, ypos, TERRAINTYPE::Terrain_SpecialBloom)) {
                        logWarning(inifile_->getLineNumber("MAP", "Special"),
                                   "Special bloom position '" + SpecialPosition + "' outside map!");
                    }
                } else {
                    logWarning(inifile_->getLineNumber("MAP", "Special"),
                               "Invalid special bloom position: '" + SpecialPosition + "'");
                }
            }
        }

        const auto FieldString = inifile_->getStringValue("MAP", "Field");
        if (!FieldString.empty()) {
            const auto FieldPositions = splitStringToStringVector(FieldString);

            for (const auto& FieldPosition : FieldPositions) {
                // set bloom
                int FieldPos = 0;
                if (parseString(FieldPosition, FieldPos)) {
                    int xpos = getXPos(FieldPos);
                    int ypos = getYPos(FieldPos);

                    map->createSpiceField(context, Coord(xpos, ypos), 5, true);
                } else {
                    logWarning(inifile_->getLineNumber("MAP", "Field"),
                               "Invalid spice field position: '" + FieldPosition + "'");
                }
            }
        }

    } else {
        // new map format with saved map

        if ((!inifile_->hasKey("MAP", "SizeX")) || (!inifile_->hasKey("MAP", "SizeY"))) {
            logError("SizeX and SizeY must be specified!");
        }

        sizeX_ = inifile_->getIntValue("MAP", "SizeX", 0);
        sizeY_ = inifile_->getIntValue("MAP", "SizeY", 0);

        if (sizeX_ <= 0) {
            logError(inifile_->getLineNumber("MAP", "SizeX"),
                     "Invalid map size: " + std::to_string(sizeX_) + "x" + std::to_string(sizeY_) + "!");
        }

        if (sizeY_ <= 0) {
            logError(inifile_->getLineNumber("MAP", "SizeY"),
                     "Invalid map size: " + std::to_string(sizeX_) + "x" + std::to_string(sizeY_) + "!");
        }

        logicalSizeX_   = sizeX_;
        logicalSizeY_   = sizeY_;
        logicalOffsetX_ = 0;
        logicalOffsetY_ = 0;

        map = std::make_unique<Map>(*pGame, sizeX_, sizeY_);

        GameContext context{*pGame, *map, pGame->getObjectManager()};

        for (int y = 0; y < sizeY_; y++) {
            const auto rowKey = fmt::sprintf("%.3d", y);

            if (!inifile_->hasKey("MAP", rowKey)) {
                logWarning(inifile_->getLineNumber("MAP"), "Map row %d does not exist!", y);
                continue;
            }

            const auto rowString = inifile_->getStringValue("MAP", rowKey);

            if (std::cmp_greater(rowString.size(), std::numeric_limits<int>::max())) {
                logWarning(inifile_->getLineNumber("MAP"), "Map row too long!");
                continue;
            }

            auto rowLength = static_cast<int>(rowString.size());

            if (rowLength < sizeX_) {
                logWarning(inifile_->getLineNumber("MAP", rowKey), "Map row %d is not long enough!", y);
            } else if (rowLength > sizeX_) {
                logWarning(inifile_->getLineNumber("MAP", rowKey), "Map row %d is too long!", y);
                rowLength = sizeX_;
            }

            for (auto x = 0; x < rowLength; ++x) {
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
                                   "Unknown map tile type '%s' in map tile (%d, %d)!", rowString.at(x), x, y);
                        type = TERRAINTYPE::Terrain_Sand;
                    } break;
                }

                if (auto* const tile = map->tryGetTile(x, y))
                    tile->setType(context, type);
            }
        }

        map->createSandRegions();
    }

    if (auto* const screen_border = dune::globals::screenborder.get())
        screen_border->adjustScreenBorderToMapsize(map->getSizeX(), map->getSizeY());
}

/**
    This method loads the houses on the map specified by the various house sections in the INI file ([Atreides],
   [Ordos], [Harkonnen]).
*/
void INIMapLoader::loadHouses(const GameContext& context) {
    const GameInitSettings::HouseInfoList& houseInfoList = pGame->getGameInitSettings().getHouseInfoList();

    // find "player?" sections
    std::vector<std::string> playerSectionsOnMap;
    for (int i = 1; i <= NUM_HOUSES; i++) {
        const auto sectionname = fmt::format("player{}", i);
        if (inifile_->hasSection(sectionname)) {
            playerSectionsOnMap.push_back(sectionname);
        }
    }

    // find unbounded houses
    std::vector<HOUSETYPE> unboundedHouses;

    for (int h = 0; h < NUM_HOUSES; h++) {
        bool bFound = false;
        for (const auto& houseInfo : houseInfoList) {
            if (houseInfo.houseID == static_cast<HOUSETYPE>(h)) {
                bFound = true;
                break;
            }
        }

        const auto houseName = getHouseNameByNumber(static_cast<HOUSETYPE>(h));
        if ((!bFound) && (inifile_->hasSection(houseName) || (!playerSectionsOnMap.empty()))) {
            unboundedHouses.push_back(static_cast<HOUSETYPE>(h));
        }
    }

    // init housename2house mapping with every house section marked as unused
    for (int i = 0; i < NUM_HOUSES; i++) {
        auto houseName = getHouseNameByNumber(static_cast<HOUSETYPE>(i));
        convertToLower(houseName);

        if (inifile_->hasSection(houseName)) {
            housename2house[houseName] = HOUSETYPE::HOUSE_UNUSED;
        }
    }

    // init housename2house mapping with every player section on map marked as unused
    for (const auto& playSection : playerSectionsOnMap) {
        housename2house[playSection] = HOUSETYPE::HOUSE_UNUSED;
    }

    // now set up all the houses
    for (const GameInitSettings::HouseInfo& houseInfo : houseInfoList) {
        HOUSETYPE houseID;

        pGame->houseInfoListSetup_.push_back(houseInfo);

        if (houseInfo.houseID == HOUSETYPE::HOUSE_INVALID) {
            // random house => select one unbound house
            if (unboundedHouses.empty()) {
                // skip this house
                continue;
            }
            const int randomIndex = pGame->randomGen.rand(0, static_cast<int>(unboundedHouses.size()) - 1);
            houseID               = unboundedHouses[randomIndex];
            unboundedHouses.erase(unboundedHouses.begin() + randomIndex);

            pGame->houseInfoListSetup_.back().houseID = houseID;
        } else {
            houseID = houseInfo.houseID;
        }

        std::string houseName = getHouseNameByNumber(houseID);
        convertToLower(houseName);

        if (!inifile_->hasSection(houseName)) {
            // select one of the Player sections
            if (playerSectionsOnMap.empty()) {
                // skip this house
                continue;
            }

            const int randomIndex = pGame->randomGen.rand(0, static_cast<int>(playerSectionsOnMap.size()) - 1);
            houseName             = playerSectionsOnMap[randomIndex];
            playerSectionsOnMap.erase(playerSectionsOnMap.begin() + randomIndex);
        }

        housename2house[houseName] = houseID;

        int startingCredits = inifile_->getIntValue(houseName, "Credits", DEFAULT_STARTINGCREDITS);

        int maxUnits = 0;
        if (pGame->getGameInitSettings().getGameOptions().maximumNumberOfUnitsOverride >= 0) {
            maxUnits = pGame->getGameInitSettings().getGameOptions().maximumNumberOfUnitsOverride;
        } else {
            const int defaultMaxUnit = std::max(25, 25 * (sizeX_ * sizeY_) / (64 * 64));
            const int maxUnit        = inifile_->getIntValue(houseName, "MaxUnit", defaultMaxUnit);
            maxUnits                 = inifile_->getIntValue(houseName, "MaxUnits", maxUnit);
        }

        int quota = inifile_->getIntValue(houseName, "Quota", 0);

        pGame->house_[static_cast<int>(houseID)] =
            std::make_unique<House>(context, houseID, startingCredits, maxUnits, houseInfo.team, quota);
        auto* const pNewHouse = pGame->getHouse(houseID);

        // add players
        for (const auto& playerInfo : houseInfo.playerInfoList) {
            const auto* pPlayerData = PlayerFactory::getByPlayerClass(playerInfo.playerClass);
            if (pPlayerData == nullptr) {
                logWarning("Cannot load '" + playerInfo.playerClass + "', using default AI player!");
                pPlayerData = PlayerFactory::getByPlayerClass(DEFAULTAIPLAYERCLASS);
                if (pPlayerData == nullptr) {
                    logWarning("Cannot load default AI player!");
                    continue;
                }
            }

            auto pPlayer = pPlayerData->create(context, pNewHouse, playerInfo.playerName);

            if (((pGame->getGameInitSettings().getGameType() != GameType::CustomMultiplayer)
                 && (dynamic_cast<HumanPlayer*>(pPlayer.get()) != nullptr))
                || (playerInfo.playerName == pGame->getLocalPlayerName())) {
                dune::globals::pLocalHouse  = pNewHouse;
                dune::globals::pLocalPlayer = dynamic_cast<HumanPlayer*>(pPlayer.get());
            }

            pNewHouse->addPlayer(std::move(pPlayer));
        }
    }
}

/**
    This method loads the choam section of the INI file
*/
void INIMapLoader::loadChoam() {
    static constexpr auto sectionname = "CHOAM"sv;

    if (!inifile_->hasSection(sectionname)) {
        return;
    }

    for (const auto& key : inifile_->keys(sectionname)) {
        const auto unitID = getItemIDByName(key.getKeyName());
        if ((unitID == ItemID_Invalid) || !isUnit(unitID)) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid unit string: '%s'",
                       key.getKeyName());
            continue;
        }

        auto num = key.getValue(-2);
        if (num == -2) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid choam number!");
            continue;
        }

        if (num == -1) {
            num = 0;
        }

        pGame->for_each_house([=](auto& house) { house.getChoam().addItem(unitID, num); });
    }
}

/**
    This method loads the units specified by the [Units] section.
*/
void INIMapLoader::loadUnits(const GameContext& context) {
    static constexpr auto sectionname = "UNITS"sv;

    if (!inifile_->hasSection(sectionname)) {
        return;
    }

    std::array<bool, NUM_HOUSES> nextSpecialUnitIsSonicTank;
    std::ranges::fill(nextSpecialUnitIsSonicTank, true);

    for (const auto& key : inifile_->keys(sectionname)) {
        if (key.getKeyName().find("ID") == 0) {
            const auto keyView = key.getStringView();

            std::string_view HouseStr, UnitStr, health, PosStr, rotation, mode;
            if (!splitString(keyView, &HouseStr, &UnitStr, &health, &PosStr, &rotation, &mode)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           fmt::format("Invalid unit string '{}'!", key.getStringView()));
                continue;
            }

            const auto houseID = getHouseID(HouseStr);
            if (houseID == HOUSETYPE::HOUSE_UNUSED) {
                // skip unit for unused house
                continue;
            }
            if (houseID == HOUSETYPE::HOUSE_INVALID) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           fmt::format("Invalid house string for '{}': '{}'!", UnitStr, HouseStr));
                continue;
            }

            int pos = 0;
            if (!parseString(PosStr, pos) || (pos < 0)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           fmt::format("Invalid position string for '{}': '{}'!", UnitStr, PosStr));
                continue;
            }

            int int_angle = 0;
            if (!parseString(rotation, int_angle) || (int_angle < 0) || (int_angle > 255)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           fmt::format("Invalid rotation string: '{}'!", rotation));
                int_angle = 64;
            }
            int_angle        = (int_angle + 16) / 32;
            int_angle        = NUM_ANGLES - int_angle + 2;
            const auto angle = normalizeAngle(static_cast<ANGLETYPE>(int_angle));

            int Num2Place      = 1;
            ItemID_enum itemID = getItemIDByName(UnitStr);
            if ((itemID == ItemID_Invalid) || !isUnit(itemID)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           fmt::format("Invalid unit string: '{}'!", UnitStr));
                continue;
            }

            if (itemID == Unit_Infantry) {
                // make three
                itemID    = Unit_Soldier;
                Num2Place = 3;
            } else if (itemID == Unit_Troopers) {
                // make three
                itemID    = Unit_Trooper;
                Num2Place = 3;
            } else if (itemID == Unit_Special) {
                switch (houseID) {

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
                        if (nextSpecialUnitIsSonicTank[static_cast<int>(houseID)]
                            && pGame->objectData.data[Unit_SonicTank][static_cast<int>(houseID)].enabled) {
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
                    }
                }
            }

            if (!pGame->objectData.data[itemID][static_cast<int>(houseID)].enabled) {
                continue;
            }

            int iHealth = 0;
            if (!parseString(health, iHealth) || (iHealth < 0) || (iHealth > 256)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           fmt::format("Invalid health string: '{}'!", health));
                iHealth = 256;
            }

            const auto percentHealth = std::min(FixPoint(iHealth) / 256, FixPoint(1));

            auto attackmode = getAttackModeByName(mode);
            if (attackmode == ATTACKMODE_INVALID) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           fmt::format("Invalid attackmode string: '{}'!", mode));
                attackmode = AREAGUARD;
            }

            if (auto* house = getOrCreateHouse(context, houseID)) {
                for (auto i = 0; i < Num2Place; i++) {
                    auto* const newUnit = house->placeUnit(itemID, getXPos(pos), getYPos(pos), true);
                    if (newUnit == nullptr) {
                        auto warning =
                            fmt::format("Invalid or occupied position for '{}': '{}' ({}x{}/{}) after parsing {}!",
                                        UnitStr, PosStr, getXPos(pos), getYPos(pos), PosStr, keyView);
                        logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), warning);
                        continue;
                    }

#if defined(DEBUG)
                    sdl2::log_info(fmt::format("Placed unit {} of type {} at {}x{} ({}/{}) after parsing {}",
                                               newUnit->getObjectID(), itemID, newUnit->getLocation().x,
                                               newUnit->getLocation().y, pos, PosStr, keyView));
#endif // defined(DEBUG)

                    newUnit->setHealth((newUnit->getMaxHealth() * percentHealth));
                    newUnit->doSetAttackMode(context, attackmode);
                    newUnit->setAngle(angle);

                    if (auto* pTankBase = dune_cast<TankBase>(newUnit))
                        pTankBase->setTurretAngle(angle);
                }
            } else {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           fmt::format("Unable to get or create house {}!", static_cast<int>(houseID)));
            }
        } else {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid unit key: '%s'!",
                       key.getKeyName());
        }
    }
}

/**
    This method loads the structures specified by the [Structures] section.
*/
void INIMapLoader::loadStructures(const GameContext& context) {
    static constexpr auto sectionname = "STRUCTURES"sv;

    if (!inifile_->hasSection(sectionname)) {
        return;
    }

    for (const auto& key : inifile_->keys(sectionname)) {
        const auto tmpkey = key.getKeyName();
        const auto tmp    = key.getStringView();

        const auto& object_data = pGame->objectData;

        if (tmpkey.compare(0, 3, "GEN") == 0) {
            // Gen Object/Structure
            const auto PosStr = tmpkey.substr(3, tmpkey.size() - 3);
            int pos           = 0;
            if (!parseString(PosStr, pos) || (pos < 0)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid position string: '%s'!",
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
                           "Invalid house string for '" + BuildingStr + "': '" + HouseStr + "'!");
                continue;
            }

            if (BuildingStr == "Concrete" && object_data.data[Structure_Slab1][static_cast<int>(houseID)].enabled) {
                getOrCreateHouse(context, houseID)
                    ->placeStructure(NONE_ID, Structure_Slab1, getXPos(pos), getYPos(pos), true);
            } else if (BuildingStr == "Wall" && object_data.data[Structure_Wall][static_cast<int>(houseID)].enabled) {
                const auto* structure = getOrCreateHouse(context, houseID)
                                            ->placeStructure(NONE_ID, Structure_Wall, getXPos(pos), getYPos(pos), true);
                if (structure == nullptr) {
                    logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                               fmt::format("Invalid or occupied position for '{}': '{}'!", BuildingStr, PosStr));
                }
            } else if ((BuildingStr != "Concrete") && (BuildingStr != "Wall")) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid building string: '" + BuildingStr + "'!");
            }
        } else if (tmpkey.compare(0, 2, "ID") == 0) {
            // other structure
            std::string HouseStr, BuildingStr, health, PosStr;
            splitString(tmp, HouseStr, BuildingStr, health, PosStr);

            int pos = 0;
            if (!parseString(PosStr, pos) || (pos < 0)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           fmt::format("Invalid position string for '{}': '{}'!", BuildingStr, PosStr));
                continue;
            }

            const auto houseID = getHouseID(HouseStr);
            if (houseID == HOUSETYPE::HOUSE_UNUSED) {
                // skip structure for unused house
                continue;
            }
            if (houseID == HOUSETYPE::HOUSE_INVALID) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid house string for '" + BuildingStr + "': '" + HouseStr + "'!");
                continue;
            }

            int iHealth = 0;
            if (!parseString(health, iHealth) || (iHealth < 0) || (iHealth > 256)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid health string: '" + health + "'!");
                iHealth = 256;
            }
            FixPoint percentHealth = std::min(FixPoint(iHealth) / 256, FixPoint(1));

            ItemID_enum itemID = getItemIDByName(BuildingStr);

            if ((itemID == ItemID_Invalid) || !isStructure(itemID)) {
                logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                           "Invalid building string: '" + BuildingStr + "'!");
                continue;
            }

            if (itemID != 0 && object_data.data[itemID][static_cast<int>(houseID)].enabled) {
                auto* const newStructure = getOrCreateHouse(context, houseID)
                                               ->placeStructure(NONE_ID, itemID, getXPos(pos), getYPos(pos), true);
                if (newStructure == nullptr) {
                    logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                               fmt::format("Invalid or occupied position for '{}': '{}'!", BuildingStr, PosStr));
                    continue;
                }
                newStructure->setHealth(newStructure->getMaxHealth() * percentHealth);
            }
        } else {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid structure key: '%s'!", tmpkey);
        }
    }
}

/**
    This method loads the reinforcements from the [REINFORCEMENTS] section.
*/
void INIMapLoader::loadReinforcements(const GameContext& context) {
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
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid house string: '%s'!",
                       strHouseName);
            continue;
        }

        auto Num2Drop = 1;
        auto itemID   = getItemIDByName(strUnitName);
        if ((itemID == ItemID_Invalid) || !isUnit(itemID)) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid unit string: '%s'!",
                       strUnitName);
            continue;
        }

        if (itemID == Unit_Infantry) {
            // make three
            itemID   = Unit_Soldier;
            Num2Drop = 3;
        } else if (itemID == Unit_Troopers) {
            // make three
            itemID   = Unit_Trooper;
            Num2Drop = 3;
        }

        if (!pGame->objectData.data[itemID][static_cast<int>(houseID)].enabled) {
            continue;
        }

        auto dropLocation = getDropLocationByName(strDropLocation);
        if (dropLocation == DropLocation::Drop_Invalid) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid drop location string: '%s'!",
                       strDropLocation);
            dropLocation = DropLocation::Drop_Homebase;
        }

        auto bRepeat = (strPlus == "+");
        if (strTime.rfind('+') == (strTime.length() - 1)) {
            strTime.resize(strTime.length() - 1);
            bRepeat = true;
        }

        auto droptime = 0u;
        if (!parseString(strTime, droptime)) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid drop time string: '%s'!",
                       strTime);
            continue;
        }
        const auto dropCycle = MILLI2CYCLES(droptime * 60U * 1000U);

        for (auto i = 0; i < Num2Drop; i++) {
            // check if there is a similar trigger at the same time

            bool bInserted = false;
            for (const auto& pTrigger : pGame->getTriggerManager().getTriggers()) {
                auto* pReinforcementTrigger = dynamic_cast<ReinforcementTrigger*>(pTrigger.get());

                if (pReinforcementTrigger != nullptr && pReinforcementTrigger->getCycleNumber() == dropCycle
                    && pReinforcementTrigger->getHouseID() == houseID && pReinforcementTrigger->isRepeat() == bRepeat
                    && pReinforcementTrigger->getDropLocation() == dropLocation) {

                    // add the new reinforcement to this reinforcement (call only one carryall)
                    pReinforcementTrigger->addUnit(itemID);
                    bInserted = true;
                    break;
                }
            }

            if (!bInserted) {
                getOrCreateHouse(context, houseID); // create house if not yet available
                pGame->getTriggerManager().addTrigger(
                    std::make_unique<ReinforcementTrigger>(houseID, itemID, dropLocation, bRepeat, dropCycle));
            }
        }
    }
}

/**
    This method loads the AI teams from the [TEAMS] section.
*/
void INIMapLoader::loadAITeams(const GameContext& context) {
    static constexpr auto sectionname = "TEAMS"sv;

    if (!inifile_->hasSection(sectionname)) {
        return;
    }

    for (const INIFile::Key& key : inifile_->keys(sectionname)) {
        std::string strHouseName;
        std::string strAITeamBehavior;
        std::string strAITeamType;
        std::string strMinUnits;
        std::string strMaxUnits;

        if (!splitString(key.getStringView(), strHouseName, strAITeamBehavior, strAITeamType, strMinUnits,
                         strMaxUnits)) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()), "Invalid teams string: %s = %s",
                       key.getKeyName(), key.getStringView());
            continue;
        }

        const auto houseID = getHouseID(strHouseName);
        if (houseID == HOUSETYPE::HOUSE_UNUSED) {
            // skip reinforcement for unused house
            continue;
        }
        if (houseID == HOUSETYPE::HOUSE_INVALID) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                       "Invalid house string: '" + strHouseName + "'!");
            continue;
        }

        AITeamBehavior aiTeamBehavior = getAITeamBehaviorByName(strAITeamBehavior);
        if (aiTeamBehavior == AITeamBehavior::AITeamBehavior_Invalid) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                       "Invalid team behavior string: '" + strAITeamBehavior + "'!");
            aiTeamBehavior = AITeamBehavior::AITeamBehavior_Normal;
        }

        AITeamType aiTeamType = getAITeamTypeByName(strAITeamType);
        if (aiTeamType == AITeamType::AITeamType_Invalid) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                       "Invalid team type string: '" + strAITeamType + "'!");
            aiTeamType = AITeamType::AITeamType_Foot;
        }

        int minUnits = 0;
        if (!parseString(strMinUnits, minUnits)) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                       "Invalid min units string: '" + strMinUnits + "'!");
            continue;
        }

        int maxUnits = 0;
        if (!parseString(strMaxUnits, maxUnits)) {
            logWarning(inifile_->getLineNumber(sectionname, key.getKeyName()),
                       "Invalid max units string: '" + strMaxUnits + "'!");
            continue;
        }

        getOrCreateHouse(context, houseID)->addAITeam(aiTeamBehavior, aiTeamType, minUnits, maxUnits);
    }
}

/**
    This method sets up the view specified by "TacticalPos" in the [BASIC] section.
*/
void INIMapLoader::loadView(const GameContext& context) {
    if (inifile_->hasKey("BASIC", "TacticalPos")) {
        const auto tacticalPosInt = inifile_->getIntValue("BASIC", "TacticalPos", -10000) + 64 * 5 + 7;
        const Coord tacticalPos(getXPos(tacticalPosInt), getYPos(tacticalPosInt));

        if (tacticalPos.x < 0 || tacticalPos.x >= sizeX_ || tacticalPos.y < 0 || tacticalPos.y >= sizeY_) {
            logWarning(inifile_->getLineNumber("BASIC", "TacticalPos"),
                       "Invalid TacticalPos: '" + std::to_string(tacticalPosInt) + "'!");
            context.game.setupView(context);
        } else {
            if (auto* const screen_border = dune::globals::screenborder.get())
                screen_border->setNewScreenCenter(tacticalPos * TILESIZE);
        }
    } else {
        context.game.setupView(context);
    }
}

/**
    This method returns the house object of the specified house id. If it does not already exist a new AI Player is
   created and returned. \param houseID the house to return or create \return the house specified by house
*/
House* INIMapLoader::getOrCreateHouse(const GameContext& context, HOUSETYPE houseID) {
    auto& pHouse = pGame->house_[pGame->getHouseIndex(houseID)];

    if (pHouse)
        return pHouse.get();

    uint8_t team = 0;
    if (pGame->gameType == GameType::Campaign || pGame->gameType == GameType::Skirmish) {
        // in campaign all "other" units are in the same team as the AI
        team = 2;
    }

    int maxUnits = 0;
    if (pGame->getGameInitSettings().getGameOptions().maximumNumberOfUnitsOverride >= 0) {
        maxUnits = pGame->getGameInitSettings().getGameOptions().maximumNumberOfUnitsOverride;
    } else {
        maxUnits = std::max(25, 25 * (map->getSizeX() * map->getSizeY()) / (64 * 64));
    }
    auto pNewHouse = std::make_unique<House>(context, houseID, 0, maxUnits, team, 0);

    const auto& houseInfoList = pGame->getGameInitSettings().getHouseInfoList();

    for (const auto& houseInfo : houseInfoList) {
        if (houseInfo.houseID != houseID)
            continue;

        for (const auto& playerInfo : houseInfo.playerInfoList) {
            const auto* pPlayerData = PlayerFactory::getByPlayerClass(playerInfo.playerClass);
            if (pPlayerData == nullptr) {
                logWarning("Cannot load '" + playerInfo.playerClass + "', using default AI player!");
                pPlayerData = PlayerFactory::getByPlayerClass(DEFAULTAIPLAYERCLASS);
                if (pPlayerData == nullptr) {
                    logWarning("Cannot load default AI player!");
                    continue;
                }
            }

            auto pPlayer = pPlayerData->create(context, pNewHouse.get(), playerInfo.playerName);

            if (playerInfo.playerName == pGame->getLocalPlayerName()) {
                dune::globals::pLocalHouse  = pNewHouse.get();
                dune::globals::pLocalPlayer = dynamic_cast<HumanPlayer*>(pPlayer.get());
            }

            pNewHouse->addPlayer(std::move(pPlayer));
        }

        break;
    }

    /*
        // probably not a good idea to treat any "anonymous" house as an AI player
        if(pNewHouse->getPlayerList().empty()) {
            const PlayerFactory::PlayerData* pPlayerData = PlayerFactory::getByPlayerClass(DEFAULTAIPLAYERCLASS);
            Player* pPlayer = pPlayerData->create(pNewHouse.get(), getHouseNameByNumber((HOUSETYPE)
       pNewHouse->getHouseID())); pNewHouse->addPlayer(std::unique_ptr<Player>(pPlayer));
        }
        */

    pHouse = std::move(pNewHouse);

    return pHouse.get();
}

HOUSETYPE INIMapLoader::getHouseID(std::string_view name) {
    const auto lowerName = strToLower(name);

    const auto it = housename2house.find(lowerName);
    if (it != housename2house.end())
        return it->second;

    return getHouseByName(lowerName);
}
