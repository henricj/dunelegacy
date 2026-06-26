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

#include <Bullet.h>
#include <Explosion.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <misc/IFileStream.h>
#include <misc/OFileStream.h>
#include <misc/OHashStream.h>
#include <misc/string_error.h>
#include <players/HumanPlayer.h>
#include <structures/StructureBase.h>
#include <units/UnitBase.h>

GameCore::GameCore(Game& game) noexcept : game_(game) { }

void GameCore::processObjects() {
    // update all tiles
    game_.map_->for_all([](Tile& t) { t.update(); });

    const GameContext context{game_, *dune::globals::currentGameMap, game_.objectManager_};

    for (auto* pStructure : dune::globals::structureList) {
        pStructure->update(context);
    }

    for (auto* pUnit : dune::globals::unitList) {
        pUnit->update(context);
    }

    auto selection_changed = false;

    game_.map_->consume_removed_objects([&](uint32_t objectID) {
        auto* object = game_.objectManager_.getObject(objectID);

        if (!object)
            return;

        if (game_.removeFromSelectionLists(object))
            selection_changed = true;
    });

    game_.objectManager_.consume_pending_deletes([&](auto& object) {
        object->cleanup(context, dune::globals::pLocalPlayer);

        if (game_.removeFromSelectionLists(object.get()))
            selection_changed = true;

        game_.removeFromQuickSelectionLists(object->getObjectID());
    });

    if (selection_changed)
        game_.selectionChanged();

    std::erase_if(dune::globals::bulletList, [&](auto& b) { return b->update(context); });

    std::erase_if(game_.explosionList_, [](auto& e) { return e->update(); });
}

void GameCore::updateGame(const GameContext& context) {
    game_.cmdManager_.executeCommands(context, game_.gameCycleCount_);

    // sdl2::log_info("cycle {} : {}", gameCycleCount, context.game.randomGen.getSeed());

#ifdef TEST_SYNC
    // add every gamecycles one test sync command
    if (game_.bReplay_ == false) {
        game_.cmdManager_.addCommand(
            Command(dune::globals::pLocalPlayer->getPlayerID(), CMD_TEST_SYNC, game_.randomGen.getSeed()));
    }
#endif

    std::ranges::for_each(game_.house_, [](auto& h) {
        if (h)
            h->update();
    });

    game_.triggerManager_.trigger(context, game_.gameCycleCount_);

    processObjects();

    game_.gameCycleCount_++;

    // Publish an immutable per-player visibility snapshot after each simulation tick through the triple-buffered handoff.
    game_.frameBuilder_.buildFrame(game_);

    // Transfer the built frame to the write slot and publish it through the handoff ring.
    PublishedFrame& writeFrame = game_.frameHandoff_.beginWrite();
    writeFrame                 = game_.frameBuilder_.getLatestFrame();
    game_.frameHandoff_.publishWrittenFrame();
}

void GameCore::stepSimulation(uint32_t ticks) {
    Map* const map = game_.getMap();
    if (!map)
        return;

    const GameContext context{game_, *map, game_.objectManager_};
    for (uint32_t i = 0; i < ticks; ++i)
        updateGame(context);
}

bool GameCore::loadSaveGame(const std::filesystem::path& filename) {
    IFileStream fs;

    if (!fs.open(filename)) {
        return false;
    }

    const bool ret = loadSaveGame(fs);

    fs.close();

    return ret;
}

bool GameCore::loadSaveGame(InputStream& stream) {
    game_.gameState = GameState::Loading;

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
    const bool bMultiplayerLoad = (game_.gameInitSettings_.getGameType() == GameType::LoadMultiplayer);
    const GameInitSettings::HouseInfoList oldHouseInfoList = game_.gameInitSettings_.getHouseInfoList();

    // read gameInitSettings
    game_.gameInitSettings_ = GameInitSettings(stream);

    // read the actual house setup chosen at the beginning of the game
    const auto numHouseInfo = stream.readUint32();
    game_.houseInfoListSetup_.reserve(numHouseInfo);
    for (uint32_t i = 0; i < numHouseInfo; i++) {
        game_.houseInfoListSetup_.push_back(GameInitSettings::HouseInfo(stream));
    }

    // read map size
    const auto mapSizeX = static_cast<short>(stream.readUint32());
    const auto mapSizeY = static_cast<short>(stream.readUint32());

    // create the new map
    game_.map_                    = std::make_unique<Map>(game_, mapSizeX, mapSizeY);
    dune::globals::currentGameMap = game_.map_.get();

    const GameContext context{game_, *game_.map_, game_.getObjectManager()};

    // read GameCycleCount
    game_.gameCycleCount_ = stream.readUint32();

    // read some settings
    game_.gameType  = static_cast<GameType>(stream.readSint8());
    game_.techLevel = stream.readUint8();
    game_.randomFactory.setSeed(stream.readUint8Vector());
    auto seed = stream.readUint8Vector();
    game_.randomGen.setState(seed);

    // read in the unit/structure data
    game_.objectData.load(stream);

    // load the house(s) info
    for (auto i = 0; i < NUM_HOUSES; i++) {
        if (stream.readBool()) {
            // house in game
            game_.house_[i] = std::make_unique<House>(context, stream);
        }
    }

    // we have to set the local player
    if (bMultiplayerLoad) {
        // get it from the gameInitSettings that started the game (not the one saved in the savegame)
        for (const auto& houseInfo : oldHouseInfoList) {

            // find the right house
            for (int i = 0; i < NUM_HOUSES; i++) {
                if ((game_.house_[i] != nullptr) && (game_.house_[i]->getHouseID() == houseInfo.houseID)) {
                    // iterate over all players
                    const auto& players = game_.house_[i]->getPlayerList();
                    auto playerIter     = players.cbegin();
                    for (const auto& playerInfo : houseInfo.playerInfoList) {
                        if (playerInfo.playerClass == HUMANPLAYERCLASS) {
                            while (playerIter != players.cend()) {

                                if (auto* const pHumanPlayer = dynamic_cast<HumanPlayer*>(playerIter->get())) {
                                    // we have actually found a human player and now assign the first unused
                                    // name to it
                                    game_.unregisterPlayer(pHumanPlayer);
                                    pHumanPlayer->setPlayername(playerInfo.playerName);
                                    game_.registerPlayer(pHumanPlayer);

                                    if (playerInfo.playerName == game_.getLocalPlayerName()) {
                                        dune::globals::pLocalHouse  = game_.house_[i].get();
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
        dune::globals::pLocalPlayer = dynamic_cast<HumanPlayer*>(game_.getPlayerByID(localPlayerID));
        if (!dune::globals::pLocalPlayer) {
            sdl2::log_info("Game::loadSaveGame(): No invalid playerID ({})!", localPlayerID);

            return false;
        }

        dune::globals::pLocalHouse =
            game_.house_[static_cast<int>(dune::globals::pLocalPlayer->getHouse()->getHouseID())].get();
    }

    dune::globals::debug  = stream.readBool();
    game_.bCheatsEnabled_ = stream.readBool();

    game_.winFlags  = stream.readUint32();
    game_.loseFlags = stream.readUint32();

    game_.map_->load(stream);

    // load the structures and units
    game_.objectManager_.load(stream);

    const auto numBullets = stream.readUint32();
    dune::globals::bulletList.reserve(numBullets);
    for (auto i = 0u; i < numBullets; i++) {
        game_.map_->add_bullet(stream);
    }

    const auto numExplosions = stream.readUint32();
    game_.explosionList_.reserve(numExplosions);
    for (auto i = 0u; i < numExplosions; i++) {
        game_.addExplosion(stream);
    }

    auto* const screenborder = dune::globals::screenborder.get();

    if (bMultiplayerLoad) {
        screenborder->adjustScreenBorderToMapsize(game_.map_->getSizeX(), game_.map_->getSizeY());

        screenborder->setNewScreenCenter(dune::globals::pLocalHouse->getCenterOfMainBase() * TILESIZE);

    } else {
        // load selection list
        game_.selectedList_ = stream.readUint32Set();

        // load the screenborder info
        screenborder->adjustScreenBorderToMapsize(game_.map_->getSizeX(), game_.map_->getSizeY());
        screenborder->load(stream);
    }

    // load triggers
    game_.triggerManager_.load(stream);

    // CommandManager is at the very end of the file. DO NOT CHANGE THIS!
    game_.cmdManager_.load(stream);

    game_.finished_ = false;

    return true;
}

bool GameCore::saveGame(const std::filesystem::path& filename) {
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
    game_.gameInitSettings_.save(fs);

    fs.writeUint32(gsl::narrow<uint32_t>(game_.houseInfoListSetup_.size()));
    for (const GameInitSettings::HouseInfo& houseInfo : game_.houseInfoListSetup_) {
        houseInfo.save(fs);
    }

    // write the map size
    fs.writeUint32(game_.map_->getSizeX());
    fs.writeUint32(game_.map_->getSizeY());

    // write GameCycleCount
    fs.writeUint32(game_.gameCycleCount_);

    // write some settings
    fs.writeSint8(static_cast<int8_t>(game_.gameType));
    fs.writeUint8(static_cast<uint8_t>(game_.techLevel));
    fs.writeUint8Vector(game_.randomFactory.getSeed());
    fs.writeUint8Vector(game_.randomGen.getState());

    // write out the unit/structure data
    game_.objectData.save(fs);

    // write the house(s) info
    for (int i = 0; i < NUM_HOUSES; i++) {
        fs.writeBool(game_.house_[i] != nullptr);

        if (game_.house_[i] != nullptr) {
            game_.house_[i]->save(fs);
        }
    }

    if (game_.gameInitSettings_.getGameType() != GameType::CustomMultiplayer) {
        fs.writeUint8(dune::globals::pLocalPlayer->getPlayerID());
    }

    fs.writeBool(dune::globals::debug);
    fs.writeBool(game_.bCheatsEnabled_);

    fs.writeUint32(game_.winFlags);
    fs.writeUint32(game_.loseFlags);

    game_.map_->save(fs, game_.getGameCycleCount());

    // save the structures and units
    game_.objectManager_.save(fs);

    fs.writeUint32(gsl::narrow<uint32_t>(dune::globals::bulletList.size()));
    for (const auto& pBullet : dune::globals::bulletList) {
        pBullet->save(fs);
    }

    fs.writeUint32(gsl::narrow<uint32_t>(game_.explosionList_.size()));
    for (const auto& pExplosion : game_.explosionList_) {
        pExplosion->save(fs);
    }

    if (game_.gameInitSettings_.getGameType() != GameType::CustomMultiplayer) {
        // save selection lists

        // write out selected units list
        fs.writeUint32Set(game_.selectedList_);

        // write the screenborder info
        dune::globals::screenborder->save(fs);
    }

    // save triggers
    game_.triggerManager_.save(fs);

    // CommandManager is at the very end of the file. DO NOT CHANGE THIS!
    game_.cmdManager_.save(fs);

    fs.close();

    return true;
}

void GameCore::serializeCanonicalState(OutputStream& stream) const {
    // Game cycle and mode
    stream.writeUint32(game_.gameCycleCount_);
    stream.writeSint8(static_cast<int8_t>(game_.gameType));
    stream.writeUint8(static_cast<uint8_t>(game_.techLevel));

    // RNG state — critical for determinism across runs
    stream.writeUint8Vector(game_.randomFactory.getSeed());
    stream.writeUint8Vector(game_.randomGen.getState());

    // Gameplay flags
    stream.writeBool(dune::globals::debug);
    stream.writeBool(game_.bCheatsEnabled_);
    stream.writeUint32(static_cast<uint32_t>(game_.winFlags));
    stream.writeUint32(static_cast<uint32_t>(game_.loseFlags));

    // House states in fixed order
    for (int i = 0; i < NUM_HOUSES; ++i) {
        stream.writeBool(game_.house_[i] != nullptr);
        if (game_.house_[i] != nullptr)
            game_.house_[i]->save(stream);
    }

    // Objects sorted by ID for deterministic ordering
    game_.objectManager_.saveCanonical(stream);

    // In-flight bullets
    stream.writeUint32(static_cast<uint32_t>(dune::globals::bulletList.size()));
    for (const auto& pBullet : dune::globals::bulletList)
        pBullet->save(stream);

    // Map tile state
    game_.map_->save(stream, game_.gameCycleCount_);

    // Pending triggers
    game_.triggerManager_.save(stream);

    // Pending commands (drive future simulation)
    game_.cmdManager_.save(stream);
}

std::array<uint8_t, 48> GameCore::computeStateHash() const {
    OHashStream stream;
    serializeCanonicalState(stream);
    return stream.finish();
}
