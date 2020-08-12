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

#include <GameInitSettings.h>

#include <misc/string_util.h>
#include <misc/exceptions.h>
#include <engine_mmath.h>

namespace Dune::Engine {

GameInitSettings::GameInitSettings() = default;

GameInitSettings::GameInitSettings(HOUSETYPE newHouseID, const GameOptionsClass& gameOptions)
    : gameType(GameType::Campaign), houseID(newHouseID), mission(1), alreadyShownTutorialHints(0),
      gameOptions(gameOptions) {
    filename = getScenarioFilename(houseID, mission);
}

GameInitSettings::GameInitSettings(const GameInitSettings& prevGameInitInfoClass, int nextMission,
                                   uint32_t alreadyPlayedRegions, uint32_t alreadyShownTutorialHints) {
    *this                           = prevGameInitInfoClass;
    mission                         = nextMission;
    this->alreadyPlayedRegions      = alreadyPlayedRegions;
    this->alreadyShownTutorialHints = alreadyShownTutorialHints;
    filename                        = getScenarioFilename(houseID, mission);
}

GameInitSettings::GameInitSettings(HOUSETYPE newHouseID, int newMission, const GameOptionsClass& gameOptions)
    : gameType(GameType::Skirmish), houseID(newHouseID), mission(newMission), gameOptions(gameOptions) {
    filename = getScenarioFilename(houseID, mission);
}

GameInitSettings::GameInitSettings(std::filesystem::path&& mapfile, std::string&& filedata, std::string&& serverName,
                                   bool multiplePlayersPerHouse, const GameOptionsClass& gameOptions)
    : gameType(GameType::CustomMultiplayer), filename(std::move(mapfile)), filedata(std::move(filedata)),
      servername(std::move(serverName)), multiplePlayersPerHouse(multiplePlayersPerHouse), gameOptions(gameOptions) { }

GameInitSettings::GameInitSettings(std::filesystem::path&& savegame) : gameType(GameType::LoadSavegame) {
    checkSaveGame(savegame);
    filename = std::move(savegame);
}

GameInitSettings::GameInitSettings(InputStream& stream) {
    gameType = static_cast<GameType>(stream.readSint8());
    houseID  = static_cast<HOUSETYPE>(stream.readSint8());

    filename = stream.readString();
    filedata = stream.readString();

    mission                   = stream.readUint8();
    alreadyPlayedRegions      = stream.readUint32();
    alreadyShownTutorialHints = stream.readUint32();
    randomSeed                = stream.readUint8Vector();

    multiplePlayersPerHouse                  = stream.readBool();
    gameOptions.gameSpeed                    = stream.readUint32();
    gameOptions.concreteRequired             = stream.readBool();
    gameOptions.structuresDegradeOnConcrete  = stream.readBool();
    gameOptions.fogOfWar                     = stream.readBool();
    gameOptions.startWithExploredMap         = stream.readBool();
    gameOptions.instantBuild                 = stream.readBool();
    gameOptions.onlyOnePalace                = stream.readBool();
    gameOptions.rocketTurretsNeedPower       = stream.readBool();
    gameOptions.sandwormsRespawn             = stream.readBool();
    gameOptions.killedSandwormsDropSpice     = stream.readBool();
    gameOptions.manualCarryallDrops          = stream.readBool();
    gameOptions.maximumNumberOfUnitsOverride = stream.readSint32();

    const auto numHouseInfo = stream.readUint32();
    for(uint32_t i = 0; i < numHouseInfo; i++) {
        houseInfoList.push_back(HouseInfo(stream));
    }
}

GameInitSettings::~GameInitSettings() = default;

void GameInitSettings::save(OutputStream& stream) const {
    stream.writeSint8(static_cast<int8_t>(gameType));
    stream.writeSint8(static_cast<int8_t>(houseID));

    stream.writeString(filename.u8string());
    stream.writeString(filedata);

    stream.writeUint8(mission);
    stream.writeUint32(alreadyPlayedRegions);
    stream.writeUint32(alreadyShownTutorialHints);
    stream.writeUint8Vector(randomSeed);

    stream.writeBool(multiplePlayersPerHouse);
    stream.writeUint32(gameOptions.gameSpeed);
    stream.writeBool(gameOptions.concreteRequired);
    stream.writeBool(gameOptions.structuresDegradeOnConcrete);
    stream.writeBool(gameOptions.fogOfWar);
    stream.writeBool(gameOptions.startWithExploredMap);
    stream.writeBool(gameOptions.instantBuild);
    stream.writeBool(gameOptions.onlyOnePalace);
    stream.writeBool(gameOptions.rocketTurretsNeedPower);
    stream.writeBool(gameOptions.sandwormsRespawn);
    stream.writeBool(gameOptions.killedSandwormsDropSpice);
    stream.writeBool(gameOptions.manualCarryallDrops);
    stream.writeSint32(gameOptions.maximumNumberOfUnitsOverride);

    stream.writeUint32(houseInfoList.size());
    for(const HouseInfo& houseInfo : houseInfoList) {
        houseInfo.save(stream);
    }
}

std::string GameInitSettings::getScenarioFilename(HOUSETYPE newHouse, int mission) {
    if((static_cast<int>(newHouse) < 0) || (newHouse >= HOUSETYPE::NUM_HOUSES)) {
        THROW(std::invalid_argument, "GameInitSettings::getScenarioFilename(): Invalid house id " +
                                         std::to_string(static_cast<int>(newHouse)) + ".");
    }

    if((mission < 0) || (mission > 22)) {
        THROW(std::invalid_argument,
              "GameInitSettings::getScenarioFilename(): There is no mission number " + std::to_string(mission) + ".");
    }

    std::string name = "SCEN?0??.INI";
    name[4]          = houseChar[static_cast<int>(newHouse)];

    name[6] = '0' + (mission / 10);
    name[7] = '0' + (mission % 10);

    return name;
}

void GameInitSettings::checkSaveGame(InputStream& stream) {
    uint32_t    magicNum        = 0;
    uint32_t    savegameVersion = 0;
    std::string duneVersion;
    try {
        magicNum        = stream.readUint32();
        savegameVersion = stream.readUint32();
        duneVersion     = stream.readString();
    } catch(std::exception&) {
        THROW(std::runtime_error, "Cannot load this savegame,\n because it seems to be truncated!");
    }

    if(magicNum != SAVEMAGIC) {
        THROW(std::runtime_error, "Cannot load this savegame,\n because it has a wrong magic number!");
    }

    if(savegameVersion < SAVEGAMEVERSION) {
        THROW(std::runtime_error,
              "Cannot load this savegame,\n because it was created with an older version:\n" + duneVersion);
    }

    if(savegameVersion > SAVEGAMEVERSION) {
        THROW(std::runtime_error,
              "Cannot load this savegame,\n because it was created with a newer version:\n" + duneVersion);
    }
}

} // namespace Dune::Engine
