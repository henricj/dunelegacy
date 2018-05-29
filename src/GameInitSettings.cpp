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

#include <misc/IFileStream.h>
#include <misc/IMemoryStream.h>
#include <misc/string_util.h>
#include <misc/exceptions.h>
#include <mmath.h>

#include <globals.h>

GameInitSettings::GameInitSettings() {
    randomSeed = getRandomInt();
}

GameInitSettings::GameInitSettings(HOUSETYPE newHouseID, const SettingsClass::GameOptionsClass& gameOptions)
 : gameType(GameType::Campaign), houseID(newHouseID), mission(1), alreadyShownTutorialHints(0), gameOptions(gameOptions) {
    filename = getScenarioFilename(houseID, mission);
    randomSeed = getRandomInt();
}

GameInitSettings::GameInitSettings(const GameInitSettings& prevGameInitInfoClass, int nextMission, Uint32 alreadyPlayedRegions, Uint32 alreadyShownTutorialHints) {
    *this = prevGameInitInfoClass;
    mission = nextMission;
    this->alreadyPlayedRegions = alreadyPlayedRegions;
    this->alreadyShownTutorialHints = alreadyShownTutorialHints;
    filename = getScenarioFilename(houseID, mission);
    randomSeed = getRandomInt();
}

GameInitSettings::GameInitSettings(HOUSETYPE newHouseID, int newMission, const SettingsClass::GameOptionsClass& gameOptions)
 : gameType(GameType::Skirmish), houseID(newHouseID), mission(newMission), gameOptions(gameOptions) {
    filename = getScenarioFilename(houseID, mission);
    randomSeed = getRandomInt();
}

GameInitSettings::GameInitSettings(const std::string& mapfile, const std::string& filedata, bool multiplePlayersPerHouse, const SettingsClass::GameOptionsClass& gameOptions)
 : gameType(GameType::CustomGame), filename(mapfile), filedata(filedata), multiplePlayersPerHouse(multiplePlayersPerHouse), gameOptions(gameOptions) {
    randomSeed = getRandomInt();
}

GameInitSettings::GameInitSettings(const std::string& mapfile, const std::string& filedata, const std::string& serverName, bool multiplePlayersPerHouse, const SettingsClass::GameOptionsClass& gameOptions)
 : gameType(GameType::CustomMultiplayer), filename(mapfile), filedata(filedata), servername(serverName), multiplePlayersPerHouse(multiplePlayersPerHouse), gameOptions(gameOptions) {
    randomSeed = getRandomInt();
}

GameInitSettings::GameInitSettings(const std::string& savegame)
 : gameType(GameType::LoadSavegame) {
    checkSaveGame(savegame);
    filename = savegame;
}

GameInitSettings::GameInitSettings(const std::string& savegame, const std::string& filedata, const std::string& serverName)
 : gameType(GameType::LoadMultiplayer), filename(savegame), filedata(filedata), servername(serverName) {
    IMemoryStream memStream(filedata.c_str(), filedata.size());
    checkSaveGame(memStream);
}

GameInitSettings::GameInitSettings(InputStream& stream) {
    gameType = static_cast<GameType>(stream.readSint8());
    houseID = static_cast<HOUSETYPE>(stream.readSint8());

    filename = stream.readString();
    filedata = stream.readString();

    mission = stream.readUint8();
    alreadyPlayedRegions = stream.readUint32();
    alreadyShownTutorialHints = stream.readUint32();
    randomSeed = stream.readUint32();

    multiplePlayersPerHouse = stream.readBool();
    gameOptions.gameSpeed = stream.readUint32();
    gameOptions.concreteRequired = stream.readBool();
    gameOptions.structuresDegradeOnConcrete = stream.readBool();
    gameOptions.fogOfWar = stream.readBool();
    gameOptions.startWithExploredMap = stream.readBool();
    gameOptions.instantBuild = stream.readBool();
    gameOptions.onlyOnePalace = stream.readBool();
    gameOptions.rocketTurretsNeedPower = stream.readBool();
    gameOptions.sandwormsRespawn = stream.readBool();
    gameOptions.killedSandwormsDropSpice = stream.readBool();
    gameOptions.manualCarryallDrops = stream.readBool();
    gameOptions.maximumNumberOfUnitsOverride = stream.readSint32();

    Uint32 numHouseInfo = stream.readUint32();
    for(Uint32 i=0;i<numHouseInfo;i++) {
        houseInfoList.push_back(HouseInfo(stream));
    }
}

GameInitSettings::~GameInitSettings() {
}

void GameInitSettings::save(OutputStream& stream) const {
    stream.writeSint8(static_cast<Sint8>(gameType));
    stream.writeSint8(houseID);

    stream.writeString(filename);
    stream.writeString(filedata);

    stream.writeUint8(mission);
    stream.writeUint32(alreadyPlayedRegions);
    stream.writeUint32(alreadyShownTutorialHints);
    stream.writeUint32(randomSeed);

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
    if( (newHouse < 0) || (newHouse >= NUM_HOUSES)) {
        THROW(std::invalid_argument, "GameInitSettings::getScenarioFilename(): Invalid house id " + std::to_string(newHouse) + ".");
    }

    if( (mission < 0) || (mission > 22)) {
        THROW(std::invalid_argument, "GameInitSettings::getScenarioFilename(): There is no mission number " + std::to_string(mission) + ".");
    }

    std::string name = "SCEN?0??.INI";
    name[4] = houseChar[newHouse];

    name[6] = '0' + (mission / 10);
    name[7] = '0' + (mission % 10);

    return name;
}

void GameInitSettings::checkSaveGame(const std::string& savegame) {
    IFileStream fs;

    if(fs.open(savegame) == false) {
        THROW(std::runtime_error, "Cannot open savegame. Make sure you have read access to this savegame!");
    }

    checkSaveGame(fs);

    fs.close();
}


void GameInitSettings::checkSaveGame(InputStream& stream) {
    Uint32 magicNum;
    Uint32 savegameVersion;
    std::string duneVersion;
    try {
        magicNum = stream.readUint32();
        savegameVersion = stream.readUint32();
        duneVersion = stream.readString();
    } catch (std::exception&) {
        THROW(std::runtime_error, "Cannot load this savegame,\n because it seems to be truncated!");
    }

    if(magicNum != SAVEMAGIC) {
        THROW(std::runtime_error, "Cannot load this savegame,\n because it has a wrong magic number!");
    }

    if(savegameVersion < SAVEGAMEVERSION) {
        THROW(std::runtime_error, "Cannot load this savegame,\n because it was created with an older version:\n" + duneVersion);
    }

    if(savegameVersion > SAVEGAMEVERSION) {
        THROW(std::runtime_error, "Cannot load this savegame,\n because it was created with a newer version:\n" + duneVersion);
    }
}
