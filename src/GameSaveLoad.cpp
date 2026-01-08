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

#include <Game.h>

#include <config.h>
#include <globals.h>

#include <Bullet.h>
#include <Explosion.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <misc/IFileStream.h>
#include <misc/OFileStream.h>
#include <misc/dune_sdlpp.h>
#include <misc/string_error.h>
#include <players/HumanPlayer.h>

bool Game::loadSaveGame(const std::filesystem::path& filename) {
    IFileStream fs;

    if (!fs.open(filename)) {
        return false;
    }

    const bool ret = loadSaveGame(fs);

    fs.close();

    return ret;
}

bool Game::loadSaveGame(InputStream& stream) {
    gameState = GameState::Loading;

    uint32_t magicNum = stream.readUint32();
    if (magicNum != SAVEMAGIC) {
        sdl2::log_info("Game::loadSaveGame(): No valid savegame! Expected magic number {:#8X}, but got {:#8X}!",
                       SAVEMAGIC,
                       magicNum);
        return false;
    }

    uint32_t savegameVersion = stream.readUint32();
    if (savegameVersion != SAVEGAMEVERSION) {
        sdl2::log_info("Game::loadSaveGame(): No valid savegame! Expected savegame version {}, but got {}!",
                       SAVEGAMEVERSION,
                       savegameVersion);
        return false;
    }

    std::string duneVersion = stream.readString();

    // if this is a multiplayer load we need to save some information before we overwrite gameInitSettings with
    // the settings saved in the savegame
    const bool bMultiplayerLoad = (gameInitSettings_.getGameType() == GameType::LoadMultiplayer);
    const GameInitSettings::HouseInfoList oldHouseInfoList = gameInitSettings_.getHouseInfoList();

    // read gameInitSettings
    gameInitSettings_ = GameInitSettings(stream);

    // read the actual house setup chosen at the beginning of the game
    const auto numHouseInfo = stream.readUint32();
    houseInfoListSetup_.reserve(numHouseInfo);
    for (uint32_t i = 0; i < numHouseInfo; i++) {
        houseInfoListSetup_.push_back(GameInitSettings::HouseInfo(stream));
    }

    // read map size
    const auto mapSizeX = static_cast<short>(stream.readUint32());
    const auto mapSizeY = static_cast<short>(stream.readUint32());

    // create the new map
    map_                          = std::make_unique<Map>(*this, mapSizeX, mapSizeY);
    dune::globals::currentGameMap = map_.get();

    const GameContext context{*this, *map_, this->getObjectManager()};

    // read GameCycleCount
    gameCycleCount_ = stream.readUint32();

    // read some settings
    gameType  = static_cast<GameType>(stream.readSint8());
    techLevel = stream.readUint8();
    randomFactory.setSeed(stream.readUint8Vector());
    auto seed = stream.readUint8Vector();
    randomGen.setState(seed);

    // read in the unit/structure data
    objectData.load(stream);

    // load the house(s) info
    for (auto i = 0; i < NUM_HOUSES; i++) {
        if (stream.readBool()) {
            // house in game
            house_[i] = std::make_unique<House>(context, stream);
        }
    }

    // we have to set the local player
    if (bMultiplayerLoad) {
        // get it from the gameInitSettings that started the game (not the one saved in the savegame)
        for (const auto& houseInfo : oldHouseInfoList) {

            // find the right house
            for (int i = 0; i < NUM_HOUSES; i++) {
                if ((house_[i] != nullptr) && (house_[i]->getHouseID() == houseInfo.houseID)) {
                    // iterate over all players
                    const auto& players = house_[i]->getPlayerList();
                    auto playerIter     = players.cbegin();
                    for (const auto& playerInfo : houseInfo.playerInfoList) {
                        if (playerInfo.playerClass == HUMANPLAYERCLASS) {
                            while (playerIter != players.cend()) {

                                if (auto* const pHumanPlayer = dynamic_cast<HumanPlayer*>(playerIter->get())) {
                                    // we have actually found a human player and now assign the first unused
                                    // name to it
                                    unregisterPlayer(pHumanPlayer);
                                    pHumanPlayer->setPlayername(playerInfo.playerName);
                                    registerPlayer(pHumanPlayer);

                                    if (playerInfo.playerName == getLocalPlayerName()) {
                                        dune::globals::pLocalHouse  = house_[i].get();
                                        dune::globals::pLocalPlayer = pHumanPlayer;
                                    }

                                    ++playerIter;
                                    break;
                                }
                                ++playerIter;
                            }
                        }
                    }
                }
            }
        }
    } else {
        // it is stored in the savegame, so set it up
        const auto localPlayerID    = stream.readUint8();
        dune::globals::pLocalPlayer = dynamic_cast<HumanPlayer*>(getPlayerByID(localPlayerID));
        if (!dune::globals::pLocalPlayer) {
            sdl2::log_info("Game::loadSaveGame(): No invalid playerID ({})!", localPlayerID);

            return false;
        }

        dune::globals::pLocalHouse =
            house_[static_cast<int>(dune::globals::pLocalPlayer->getHouse()->getHouseID())].get();
    }

    dune::globals::debug = stream.readBool();
    bCheatsEnabled_      = stream.readBool();

    winFlags  = stream.readUint32();
    loseFlags = stream.readUint32();

    map_->load(stream);

    // load the structures and units
    objectManager_.load(stream);

    const auto numBullets = stream.readUint32();
    dune::globals::bulletList.reserve(numBullets);
    for (auto i = 0u; i < numBullets; i++) {
        map_->add_bullet(stream);
    }

    const auto numExplosions = stream.readUint32();
    explosionList_.reserve(numExplosions);
    for (auto i = 0u; i < numExplosions; i++) {
        addExplosion(stream);
    }

    auto* const screenborder = dune::globals::screenborder.get();

    if (bMultiplayerLoad) {
        screenborder->adjustScreenBorderToMapsize(map_->getSizeX(), map_->getSizeY());

        screenborder->setNewScreenCenter(dune::globals::pLocalHouse->getCenterOfMainBase() * TILESIZE);

    } else {
        // load selection list
        selectedList_ = stream.readUint32Set();

        // load the screenborder info
        screenborder->adjustScreenBorderToMapsize(map_->getSizeX(), map_->getSizeY());
        screenborder->load(stream);
    }

    // load triggers
    triggerManager_.load(stream);

    // CommandManager is at the very end of the file. DO NOT CHANGE THIS!
    cmdManager_.load(stream);

    finished_ = false;

    return true;
}

bool Game::saveGame(const std::filesystem::path& filename) {
    OFileStream fs;

    if (!fs.open(filename)) {
        sdl2::log_info("Game::saveGame(): {}", dune::string_error(errno));
        dune::globals::currentGame->addToNewsTicker(std::string("Game NOT saved: Cannot open \"")
                                                    + reinterpret_cast<const char*>(filename.u8string().c_str())
                                                    + "\".");
        return false;
    }

    fs.writeUint32(SAVEMAGIC);

    fs.writeUint32(SAVEGAMEVERSION);

    fs.writeString(VERSIONSTRING);

    // write gameInitSettings
    gameInitSettings_.save(fs);

    fs.writeUint32(gsl::narrow<uint32_t>(houseInfoListSetup_.size()));
    for (const GameInitSettings::HouseInfo& houseInfo : houseInfoListSetup_) {
        houseInfo.save(fs);
    }

    // write the map size
    fs.writeUint32(map_->getSizeX());
    fs.writeUint32(map_->getSizeY());

    // write GameCycleCount
    fs.writeUint32(gameCycleCount_);

    // write some settings
    fs.writeSint8(static_cast<int8_t>(gameType));
    fs.writeUint8(static_cast<uint8_t>(techLevel));
    fs.writeUint8Vector(randomFactory.getSeed());
    fs.writeUint8Vector(randomGen.getState());

    // write out the unit/structure data
    objectData.save(fs);

    // write the house(s) info
    for (int i = 0; i < NUM_HOUSES; i++) {
        fs.writeBool(house_[i] != nullptr);

        if (house_[i] != nullptr) {
            house_[i]->save(fs);
        }
    }

    if (gameInitSettings_.getGameType() != GameType::CustomMultiplayer) {
        fs.writeUint8(dune::globals::pLocalPlayer->getPlayerID());
    }

    fs.writeBool(dune::globals::debug);
    fs.writeBool(bCheatsEnabled_);

    fs.writeUint32(winFlags);
    fs.writeUint32(loseFlags);

    map_->save(fs, getGameCycleCount());

    // save the structures and units
    objectManager_.save(fs);

    fs.writeUint32(gsl::narrow<uint32_t>(dune::globals::bulletList.size()));
    for (const auto& pBullet : dune::globals::bulletList) {
        pBullet->save(fs);
    }

    fs.writeUint32(gsl::narrow<uint32_t>(explosionList_.size()));
    for (const auto& pExplosion : explosionList_) {
        pExplosion->save(fs);
    }

    if (gameInitSettings_.getGameType() != GameType::CustomMultiplayer) {
        // save selection lists

        // write out selected units list
        fs.writeUint32Set(selectedList_);

        // write the screenborder info
        dune::globals::screenborder->save(fs);
    }

    // save triggers
    triggerManager_.save(fs);

    // CommandManager is at the very end of the file. DO NOT CHANGE THIS!
    cmdManager_.save(fs);

    fs.close();

    return true;
}

void Game::saveObject(OutputStream& stream, ObjectBase* obj) {
    if (obj == nullptr)
        return;

    stream.writeUint32(obj->getItemID());
    obj->save(stream);
}
