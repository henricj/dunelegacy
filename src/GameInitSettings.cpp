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

#include <globals.h>

#include <stdlib.h>
#include <string.h>
#include <stdexcept>

GameInitSettings::GameInitSettings()
 : gameType(GAMETYPE_INVALID), houseID(HOUSE_INVALID), mission(0), multiplePlayersPerHouse(false) {
    randomSeed = rand();
}

GameInitSettings::GameInitSettings(HOUSETYPE newHouseID, const SettingsClass::GameOptionsClass& gameOptions)
 : gameType(GAMETYPE_CAMPAIGN), houseID(newHouseID), mission(1), multiplePlayersPerHouse(false), gameOptions(gameOptions) {
    filename = getScenarioFilename(houseID, mission);
    randomSeed = rand();
}

GameInitSettings::GameInitSettings(const GameInitSettings& prevGameInitInfoClass, int nextMission) {
    *this = prevGameInitInfoClass;
    mission = nextMission;
    filename = getScenarioFilename(houseID, mission);
    randomSeed = rand();
}

GameInitSettings::GameInitSettings(HOUSETYPE newHouseID, int newMission, const SettingsClass::GameOptionsClass& gameOptions)
 : gameType(GAMETYPE_SKIRMISH), houseID(newHouseID), mission(newMission), multiplePlayersPerHouse(false), gameOptions(gameOptions) {
    filename = getScenarioFilename(houseID, mission);
    randomSeed = rand();
}

GameInitSettings::GameInitSettings(std::string mapfile, bool multiplePlayersPerHouse, const SettingsClass::GameOptionsClass& gameOptions)
 : gameType(GAMETYPE_CUSTOM), houseID(HOUSE_INVALID), mission(0), filename(mapfile), multiplePlayersPerHouse(multiplePlayersPerHouse), gameOptions(gameOptions) {
    randomSeed = rand();
}

GameInitSettings::GameInitSettings(std::string mapfile, std::string filedata, std::string serverName, bool multiplePlayersPerHouse, const SettingsClass::GameOptionsClass& gameOptions)
 : gameType(GAMETYPE_CUSTOM_MULTIPLAYER), houseID(HOUSE_INVALID), mission(0), filename(mapfile), filedata(filedata), servername(serverName), multiplePlayersPerHouse(multiplePlayersPerHouse), gameOptions(gameOptions) {
    randomSeed = rand();
}

GameInitSettings::GameInitSettings(std::string savegame)
 : gameType(GAMETYPE_LOAD_SAVEGAME), houseID(HOUSE_INVALID), mission(0) {
    checkSaveGame(savegame);
    filename = savegame;
}

GameInitSettings::GameInitSettings(std::string savegame, std::string filedata, std::string serverName)
 : gameType(GAMETYPE_LOAD_MULTIPLAYER), houseID(HOUSE_INVALID), mission(0), filename(savegame), filedata(filedata), servername(serverName) {
    IMemoryStream memStream(filedata.c_str(), filedata.size());
    checkSaveGame(memStream);
}

GameInitSettings::GameInitSettings(InputStream& stream) {
    gameType = (GAMETYPE) stream.readSint8();
    houseID = (HOUSETYPE) stream.readSint8();

    filename = stream.readString();
    filedata = stream.readString();

    mission = stream.readUint8();
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


    Uint32 numHouseInfo = stream.readUint32();
    for(Uint32 i=0;i<numHouseInfo;i++) {
        houseInfoList.push_back(HouseInfo(stream));
    }
}

GameInitSettings::~GameInitSettings() {
}

void GameInitSettings::save(OutputStream& stream) const {
    stream.writeSint8(gameType);
    stream.writeSint8(houseID);

    stream.writeString(filename);
    stream.writeString(filedata);

    stream.writeUint8(mission);
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

    stream.writeUint32(houseInfoList.size());
    HouseInfoList::const_iterator iter;
    for(iter = houseInfoList.begin(); iter != houseInfoList.end(); ++iter) {
        iter->save(stream);
    }
}



std::string GameInitSettings::getScenarioFilename(HOUSETYPE newHouse, int mission) {
    std::string name = "SCEN?0??.INI";

    if( (mission < 0) || (mission > 22)) {
        throw std::invalid_argument("GameInitSettings::getScenarioFilename(): There is no mission number " + stringify(mission) + ".");
    }

    name[4] = houseChar[newHouse];

    name[6] = '0' + (mission / 10);
    name[7] = '0' + (mission % 10);

    return name;
}

void GameInitSettings::checkSaveGame(std::string savegame) {
    IFileStream fs;

    if(fs.open(savegame) == false) {
        throw std::runtime_error("Cannot open savegame. Make sure you have read access to this savegame!");
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
        throw std::runtime_error("Cannot load this savegame,\n because it seems to be truncated!");
    }

    if(magicNum != SAVEMAGIC) {
        throw std::runtime_error("Cannot load this savegame,\n because it has a wrong magic number!");
    }

    if(savegameVersion < SAVEGAMEVERSION) {
        throw std::runtime_error("Cannot load this savegame,\n because it was created with an older version:\n" + duneVersion);
    }

    if(savegameVersion > SAVEGAMEVERSION) {
        throw std::runtime_error("Cannot load this savegame,\n because it was created with a newer version:\n" + duneVersion);
    }
}
