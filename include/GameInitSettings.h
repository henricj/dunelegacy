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

#ifndef GAMEINITINFOCLASS_H
#define GAMEINITINFOCLASS_H

#include "Definitions.h"
#include "DataTypes.h"
#include <misc/InputStream.h>
#include <misc/OutputStream.h>

#include <string>
#include <utility>


class GameInitSettings
{
public:

    class PlayerInfo {
    public:
        PlayerInfo(std::string newPlayerName, std::string newPlayerClass)
         : playerName(std::move(newPlayerName)), playerClass(std::move(newPlayerClass)) {
        }

        explicit PlayerInfo(InputStream& stream) {
            playerName = stream.readString();
            playerClass = stream.readString();
        }

        void save(OutputStream& stream) const {
            stream.writeString(playerName);
            stream.writeString(playerClass);
        }

        std::string playerName;
        std::string playerClass;
    };

    class HouseInfo {
    public:
        HouseInfo(HOUSETYPE newHouseID, int newTeam)
         : houseID(newHouseID), team(newTeam) {
        }

        explicit HouseInfo(InputStream& stream) {
            houseID = static_cast<HOUSETYPE>(stream.readSint32());
            team = stream.readSint32();

            const auto numPlayerInfo = stream.readUint32();
            for(Uint32 i=0;i<numPlayerInfo;i++) {
                playerInfoList.push_back(PlayerInfo(stream));
            }
        }

        void save(OutputStream& stream) const {
            stream.writeSint32(houseID);
            stream.writeSint32(team);

            stream.writeUint32(playerInfoList.size());
            for(const auto& playerInfo : playerInfoList) {
                playerInfo.save(stream);
            }
        }

        void addPlayerInfo(const PlayerInfo& newPlayerInfo) { playerInfoList.push_back(newPlayerInfo); };

        typedef std::vector<PlayerInfo> PlayerInfoList;

        HOUSETYPE       houseID;
        int             team;
        PlayerInfoList  playerInfoList;
    };

    typedef std::vector<HouseInfo> HouseInfoList;


    /**
        Default constructor.
        The constructed GameInitSettings object is empty
    */
    GameInitSettings();

    /**
        Constructor for specifying the start of a campaign
        \param  newHouseID          the house to play the campaign with
        \param  gameOptions         the options for this game
    */
    GameInitSettings(HOUSETYPE newHouseID, const SettingsClass::GameOptionsClass& gameOptions);

    /**
        Constructor for continuing a campaign at the specified mission
        \param  prevGameInitInfoClass       the init settings of the previous mission in the campaign
        \param  nextMission                 the number of the mission to continue the campaign
        \param  alreadyPlayedRegions        a bit set describing which regions were already played (used to forbid playing these again)
        \param  alreadyShownTutorialHints   contains flags for each tutorial hint (see enum HumanPlayer::TutorialHint)
    */
    GameInitSettings(const GameInitSettings& prevGameInitInfoClass, int nextMission, Uint32 alreadyPlayedRegions, Uint32 alreadyShownTutorialHints);

    /**
        Constructor for specifying the start of a skirmish mission in the campaign
        \param  newHouseID          the house specifying from which campaign the mission is from
        \param  newMission          the number of the mission (1 - 22)
        \param  gameOptions         the options for this game
    */
    GameInitSettings(HOUSETYPE newHouseID, int newMission, const SettingsClass::GameOptionsClass& gameOptions);

    /**
        Constructor for specifying the start of a custom map
        \param  mapfile             the name of the map (without extension)
        \param  filedata            the data of the map file
        \param  multiplePlayersPerHouse     allow multiple players per house
        \param  gameOptions         the options for this game
    */
    GameInitSettings(std::string&& mapfile, std::string&& filedata, bool multiplePlayersPerHouse, const SettingsClass::GameOptionsClass& gameOptions);

    /**
        Constructor for specifying the start of a multiplayer custom map
        \param  mapfile             the name of the map (without extension)
        \param  filedata            the data of the map file
        \param  serverName          the name of the game server
        \param  multiplePlayersPerHouse     allow multiple players per house
        \param  gameOptions         the options for this game
    */
    GameInitSettings(std::string&& mapfile, std::string&& filedata, std::string&& serverName, bool multiplePlayersPerHouse, const SettingsClass::GameOptionsClass& gameOptions);

    /**
        Constructor for specifying the loading of a savegame. If the given filename contains no valid savegame
        an exception is thrown.
        \param  savegame    the name of the savegame
    */
    explicit GameInitSettings(std::string&& savegame);

    /**
        Constructor for specifying the loading of a network savegame. If the given filename contains no valid savegame
        an exception is thrown.
        \param  savegame    the name of the savegame
        \param  filedata    the data of the savegame file
        \param  serverName  the name of the game server
    */
    GameInitSettings(std::string&& savegame, std::string&& filedata, std::string&& serverName);

    /**
        Load the game init info from a stream
        \param  stream  the stream to load from
    */
    explicit GameInitSettings(InputStream& stream);

    ~GameInitSettings();

    void save(OutputStream& stream) const;

    GameType getGameType() const noexcept { return gameType; };
    HOUSETYPE getHouseID() const noexcept { return houseID; };
    int getMission() const noexcept { return mission; };
    Uint32 getAlreadyPlayedRegions() const noexcept { return alreadyPlayedRegions; };
    Uint32 getAlreadyShownTutorialHints() const noexcept { return alreadyShownTutorialHints; };
    const std::string& getFilename() const noexcept { return filename; };
    const std::string& getFiledata() const noexcept { return filedata; };
    const std::string& getServername() const noexcept { return servername; };
    Uint32 getRandomSeed() const noexcept { return randomSeed; };

    bool isMultiplePlayersPerHouse() const noexcept { return multiplePlayersPerHouse; };
    void setMultiplePlayersPerHouse(bool multiplePlayersPerHouse) noexcept { this->multiplePlayersPerHouse = multiplePlayersPerHouse; };
    const SettingsClass::GameOptionsClass& getGameOptions() const noexcept { return gameOptions; };
    void setGameSpeed(int gameSpeed) noexcept { gameOptions.gameSpeed = gameSpeed; };

    void addHouseInfo(const HouseInfo& newHouseInfo) { houseInfoList.push_back(newHouseInfo); };
    void clearHouseInfo() { houseInfoList.clear(); };
    const HouseInfoList& getHouseInfoList() const noexcept { return houseInfoList; };

    void setHouseID(HOUSETYPE houseID) noexcept { this->houseID = houseID; };

private:
    static std::string getScenarioFilename(HOUSETYPE newHouse, int mission);

    /**
        This method checks if it is possible to load a savegame and if the magic number is correct. If there is an error an exception is thrown.
        \param savegame the name of the file to check
    */
    static void checkSaveGame(const std::string& savegame);


    /**
        This method checks if it is possible to load a savegame and if the magic number is correct. If there is an error an exception is thrown.
        \param stream the strean to read the data from
    */
    static void checkSaveGame(InputStream& stream);


    GameType        gameType = GameType::Invalid;

    HOUSETYPE       houseID = HOUSE_INVALID;
    int             mission = 0;
    Uint32          alreadyPlayedRegions = 0;
    Uint32          alreadyShownTutorialHints = 0xFFFFFFFF;

    std::string     filename;
    std::string     filedata;
    std::string     servername;

    Uint32          randomSeed = 0;

    bool            multiplePlayersPerHouse = false;

    SettingsClass::GameOptionsClass gameOptions;


    HouseInfoList   houseInfoList;
};

#endif // GAMEINITINFOCLASS_H
