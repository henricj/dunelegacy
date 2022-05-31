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

#include <Menu/CustomGamePlayers.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/INIFile.h>
#include <FileClasses/TextManager.h>

#include <Network/NetworkManager.h>

#include <GUI/GUIStyle.h>
#include <GUI/MsgBox.h>
#include <GUI/Spacer.h>
#include <GUI/dune/DuneStyle.h>

#include <players/PlayerFactory.h>

#include <misc/FileSystem.h>
#include <misc/IMemoryStream.h>
#include <misc/draw_util.h>
#include <misc/string_util.h>

#include <INIMap/INIMapPreviewCreator.h>

#include <globals.h>
#include <sand.h>

#include <utility>

namespace {
constexpr auto PLAYER_HUMAN  = 0;
constexpr auto PLAYER_OPEN   = -1;
constexpr auto PLAYER_CLOSED = -2;
} // namespace

CustomGamePlayers::CustomGamePlayers(const GameInitSettings& newGameInitSettings, bool server, bool LANServer)
    : gameInitSettings(newGameInitSettings), bServer(server), bLANServer(LANServer) {

    // set up window

    CustomGamePlayers::setWindowWidget(&windowWidget);

    windowWidget.addWidget(&mainVBox, Point(24, 23), Point(getRendererWidth() - 48, getRendererHeight() - 32));

    captionLabel.setText(reinterpret_cast<const char*>(gameInitSettings.getFilename().u8string().c_str()));
    captionLabel.setAlignment(Alignment_HCenter);
    mainVBox.addWidget(&captionLabel, 24);
    mainVBox.addWidget(Widget::create<VSpacer>(24).release());

    mainVBox.addWidget(Widget::create<Spacer>().release(), 0.04);

    mainVBox.addWidget(&mainHBox, 0.6);

    mainHBox.addWidget(Widget::create<Spacer>().release(), 0.05);
    leftVBox.addWidget(Widget::create<Spacer>().release(), 0.1);
    leftVBox.addWidget(&playerListHBox);
    playerListHBox.addWidget(Widget::create<Spacer>().release(), 0.4);
    playerListHBox.addWidget(&playerListVBox, 0.2);
    playerListHBox.addWidget(Widget::create<Spacer>().release(), 0.4);
    mainHBox.addWidget(&leftVBox, 0.8);

    mainHBox.addWidget(Widget::create<Spacer>().release(), 0.05);

    mainHBox.addWidget(Widget::create<HSpacer>(8).release());

    mainHBox.addWidget(&rightVBox, 180);
    mainHBox.addWidget(Widget::create<Spacer>().release(), 0.05);
    minimap.setSurface(GUIStyle::getInstance().createButtonSurface(130, 130, _("Choose map"), true, false));
    rightVBox.addWidget(&minimap);

    if (gameInitSettings.getGameType() == GameType::CustomGame
        || gameInitSettings.getGameType() == GameType::CustomMultiplayer) {
        auto RWops = sdl2::RWops_ptr{
            SDL_RWFromConstMem(gameInitSettings.getFiledata().c_str(), gameInitSettings.getFiledata().size())};

        INIFile inimap(RWops.get());
        extractMapInfo(&inimap);
    } else if (gameInitSettings.getGameType() == GameType::LoadMultiplayer) {
        IMemoryStream memStream(gameInitSettings.getFiledata().c_str(), gameInitSettings.getFiledata().size());

        uint32_t magicNum = memStream.readUint32();
        if (magicNum != SAVEMAGIC) {
            sdl2::log_info("CustomGamePlayers: No valid savegame! Expected magic number %.8X, but got %.8X!", SAVEMAGIC,
                           magicNum);
        }

        uint32_t savegameVersion = memStream.readUint32();
        if (savegameVersion != SAVEGAMEVERSION) {
            sdl2::log_info("CustomGamePlayers: No valid savegame! Expected savegame version %d, but got %d!",
                           SAVEGAMEVERSION, savegameVersion);
        }

        memStream.readString(); // dune legacy version

        // read gameInitSettings
        GameInitSettings tmpGameInitSettings(memStream);

        uint32_t numHouseInfo = memStream.readUint32();
        for (uint32_t i = 0; i < numHouseInfo; i++) {
            houseInfoListSetup.push_back(GameInitSettings::HouseInfo(memStream));
        }

        auto RWops = sdl2::RWops_ptr{
            SDL_RWFromConstMem(tmpGameInitSettings.getFiledata().c_str(), tmpGameInitSettings.getFiledata().size())};

        INIFile inimap(RWops.get());
        extractMapInfo(&inimap);

        // adjust multiple players per house as this was not known before actually loading the saved game
        gameInitSettings.setMultiplePlayersPerHouse(tmpGameInitSettings.isMultiplePlayersPerHouse());

        // adjust numHouses to the actually used houses (which might be smaller than the houses on the map)
        numHouses = houseInfoListSetup.size();
    } else {
        INIFile inimap(gameInitSettings.getFilename());
        extractMapInfo(&inimap);
    }

    rightVBox.addWidget(Widget::create<VSpacer>(10).release());
    rightVBox.addWidget(&mapPropertiesHBox, 0.01);
    mapPropertiesHBox.addWidget(&mapPropertyNamesVBox, 75);
    mapPropertiesHBox.addWidget(&mapPropertyValuesVBox, 105);
    mapPropertyNamesVBox.addWidget(Label::create(_("Size") + ":").release());
    mapPropertyValuesVBox.addWidget(&mapPropertySize);
    mapPropertyNamesVBox.addWidget(Label::create(_("Players") + ":").release());
    mapPropertyValuesVBox.addWidget(&mapPropertyPlayers);
    mapPropertyNamesVBox.addWidget(Label::create(_("Author") + ":").release());
    mapPropertyValuesVBox.addWidget(&mapPropertyAuthors);
    mapPropertyNamesVBox.addWidget(Label::create(_("License") + ":").release());
    mapPropertyValuesVBox.addWidget(&mapPropertyLicense);
    rightVBox.addWidget(Widget::create<Spacer>().release());

    mainVBox.addWidget(Widget::create<Spacer>().release(), 0.04);

    mainVBox.addWidget(&buttonHBox, 0.32);

    buttonHBox.addWidget(Widget::create<HSpacer>(70).release());

    backButtonVBox.addWidget(Widget::create<Spacer>().release());
    backButton.setText(_("Back"));
    backButton.setOnClick([this] { onCancel(); });
    backButtonVBox.addWidget(&backButton, 24);
    backButtonVBox.addWidget(Widget::create<VSpacer>(14).release());
    buttonHBox.addWidget(&backButtonVBox, 0.1);

    buttonHBox.addWidget(Widget::create<Spacer>().release(), 0.0625);

    chatTextView.setTextFontSize(12);
    chatVBox.addWidget(&chatTextView, 0.77);
    if (getRendererHeight() <= 600) {
        chatTextBox.setTextFontSize(12);
    }
    chatTextBox.setOnReturn([this] { onSendChatMessage(); });
    chatVBox.addWidget(&chatTextBox, 0.2);
    chatVBox.addWidget(Widget::create<Spacer>().release(), 0.03);
    buttonHBox.addWidget(&chatVBox, 0.675);

    if (gameInitSettings.getGameType() != GameType::CustomMultiplayer
        && gameInitSettings.getGameType() != GameType::LoadMultiplayer) {
        chatVBox.setVisible(false);
        chatVBox.setEnabled(false);
    }

    bool bLoadMultiplayer = gameInitSettings.getGameType() == GameType::LoadMultiplayer;

    buttonHBox.addWidget(Widget::create<Spacer>().release(), 0.0625);

    nextButtonVBox.addWidget(Widget::create<Spacer>().release());
    nextButton.setText(_("Next"));
    nextButton.setOnClick([this] { onNext(); });
    nextButtonVBox.addWidget(&nextButton, 24);
    nextButtonVBox.addWidget(Widget::create<VSpacer>(14).release());
    if (!bServer) {
        nextButton.setEnabled(false);
        nextButton.setVisible(false);
    }
    buttonHBox.addWidget(&nextButtonVBox, 0.1);

    buttonHBox.addWidget(Widget::create<HSpacer>(90).release());

    auto boundHousesIt = boundHousesOnMap.begin();

    bool thisPlayerPlaced = false;

    const auto& playername = dune::globals::settings.general.playerName;

    for (auto i = 0; std::cmp_less(i, houseInfo.size()); ++i) {
        auto& curHouseInfo = houseInfo[i];

        // set up header row with Label "House", DropDown for house selection and DropDown for team selection
        curHouseInfo.houseLabel.setText(_("House"));
        curHouseInfo.houseHBox.addWidget(&curHouseInfo.houseLabel, 60);

        if (bLoadMultiplayer) {
            if (i < static_cast<int>(houseInfoListSetup.size())) {
                GameInitSettings::HouseInfo gisHouseInfo = houseInfoListSetup.at(i);

                if (gisHouseInfo.houseID == HOUSETYPE::HOUSE_INVALID) {
                    addToHouseDropDown(curHouseInfo.houseDropDown, HOUSETYPE::HOUSE_INVALID, true);
                } else {
                    addToHouseDropDown(curHouseInfo.houseDropDown, gisHouseInfo.houseID, true);
                }
            }

            curHouseInfo.houseDropDown.setEnabled(false);
        } else if (boundHousesIt != boundHousesOnMap.end()) {
            const auto housetype = *boundHousesIt;
            ++boundHousesIt;

            addToHouseDropDown(curHouseInfo.houseDropDown, housetype, true);
            curHouseInfo.houseDropDown.setEnabled(bServer);
        } else {
            addToHouseDropDown(curHouseInfo.houseDropDown, HOUSETYPE::HOUSE_INVALID, true);
            curHouseInfo.houseDropDown.setEnabled(bServer);
        }
        curHouseInfo.houseDropDown.setOnSelectionChange(
            [this, i](auto interactive) { onChangeHousesDropDownBoxes(interactive, i); });
        curHouseInfo.houseHBox.addWidget(&curHouseInfo.houseDropDown, 95);

        if (bLoadMultiplayer) {
            if (i < static_cast<int>(houseInfoListSetup.size())) {
                GameInitSettings::HouseInfo gisHouseInfo = houseInfoListSetup.at(i);

                curHouseInfo.teamDropDown.addEntry(_("Team") + " " + std::to_string(gisHouseInfo.team),
                                                   gisHouseInfo.team);
                curHouseInfo.teamDropDown.setSelectedItem(0);
            }
            curHouseInfo.teamDropDown.setEnabled(false);
            curHouseInfo.teamDropDown.setOnClickEnabled(false);
        } else {
            for (int team = 0; team < numHouses; team++) {
                curHouseInfo.teamDropDown.addEntry(_("Team") + " " + std::to_string(team + 1), team + 1);
            }
            curHouseInfo.teamDropDown.setSelectedItem(slotToTeam[i]);
            curHouseInfo.teamDropDown.setEnabled(bServer);
        }
        curHouseInfo.teamDropDown.setOnSelectionChange(
            [this, i](auto interactive) { onChangeTeamDropDownBoxes(interactive, i); });
        curHouseInfo.houseHBox.addWidget(Widget::create<HSpacer>(10).release());
        curHouseInfo.houseHBox.addWidget(&curHouseInfo.teamDropDown, 85);

        curHouseInfo.houseInfoVBox.addWidget(&curHouseInfo.houseHBox);

        // add 1. player
        curHouseInfo.player1ArrowLabel.setTexture(
            dune::globals::pGFXManager->getUIGraphic(UI_CustomGamePlayersArrowNeutral));
        curHouseInfo.playerHBox.addWidget(&curHouseInfo.player1ArrowLabel);
        curHouseInfo.player1Label.setText(_("Player") + (gameInitSettings.isMultiplePlayersPerHouse() ? " 1" : ""));
        curHouseInfo.player1Label.setTextFontSize(12);
        curHouseInfo.playerHBox.addWidget(&curHouseInfo.player1Label, 68);

        if (bLoadMultiplayer) {
            if (i < static_cast<int>(houseInfoListSetup.size())) {
                GameInitSettings::HouseInfo gisHouseInfo = houseInfoListSetup.at(i);

                if (gisHouseInfo.houseID == HOUSETYPE::HOUSE_UNUSED) {
                    curHouseInfo.player1DropDown.addEntry(_("closed"), PLAYER_CLOSED);
                } else if (!gisHouseInfo.playerInfoList.empty()) {
                    GameInitSettings::PlayerInfo playerInfo = gisHouseInfo.playerInfoList.front();
                    if (playerInfo.playerClass == HUMANPLAYERCLASS) {
                        if (!thisPlayerPlaced) {
                            curHouseInfo.player1DropDown.addEntry(playername, PLAYER_HUMAN);
                            curHouseInfo.player1DropDown.setSelectedItem(0);
                            thisPlayerPlaced = true;
                        } else {
                            curHouseInfo.player1DropDown.addEntry(_("open"), PLAYER_OPEN);
                            curHouseInfo.player1DropDown.setSelectedItem(0);
                        }
                    } else {
                        const PlayerFactory::PlayerData* pPlayerData =
                            PlayerFactory::getByPlayerClass(playerInfo.playerClass);
                        int index = PlayerFactory::getIndexByPlayerClass(playerInfo.playerClass);

                        if (pPlayerData != nullptr) {
                            curHouseInfo.player1DropDown.addEntry(pPlayerData->getName(), -(index + 2));
                            curHouseInfo.player1DropDown.setSelectedItem(0);
                            curHouseInfo.player1DropDown.setEnabled(false);
                            curHouseInfo.player1DropDown.setOnClickEnabled(false);
                        }
                    }
                }
            }

        } else if (bServer && !thisPlayerPlaced) {
            curHouseInfo.player1DropDown.addEntry(playername, PLAYER_HUMAN);
            curHouseInfo.player1DropDown.setSelectedItem(0);
            curHouseInfo.player1DropDown.setEnabled(bServer);
            thisPlayerPlaced = true;
        } else {
            curHouseInfo.player1DropDown.addEntry(_("open"), PLAYER_OPEN);
            curHouseInfo.player1DropDown.setSelectedItem(0);
            curHouseInfo.player1DropDown.addEntry(_("closed"), PLAYER_CLOSED);
            for (unsigned int k = 1; k < PlayerFactory::getList().size(); k++) {
                curHouseInfo.player1DropDown.addEntry(PlayerFactory::getByIndex(k)->getName(), k);
            }
            curHouseInfo.player1DropDown.setEnabled(bServer);
        }
        curHouseInfo.player1DropDown.setOnSelectionChange(
            [this, boxnum = 2 * i](bool interactive) { onChangePlayerDropDownBoxes(interactive, boxnum); });
        curHouseInfo.player1DropDown.setOnClick([this, capture0 = 2 * i] { onClickPlayerDropDownBox(capture0); });
        curHouseInfo.playerHBox.addWidget(&curHouseInfo.player1DropDown, 100);

        curHouseInfo.playerHBox.addWidget(Widget::create<HSpacer>(10).release());

        // add 2. player
        curHouseInfo.player2Label.setText(_("Player") + " 2");
        curHouseInfo.player2Label.setTextFontSize(12);
        curHouseInfo.playerHBox.addWidget(&curHouseInfo.player2Label, 68);

        if (bLoadMultiplayer) {
            if (i < static_cast<int>(houseInfoListSetup.size())) {
                GameInitSettings::HouseInfo gisHouseInfo = houseInfoListSetup.at(i);

                if (gisHouseInfo.houseID == HOUSETYPE::HOUSE_UNUSED) {
                    curHouseInfo.player2DropDown.addEntry(_("closed"), PLAYER_CLOSED);
                } else if (gisHouseInfo.playerInfoList.size() >= 2) {
                    GameInitSettings::PlayerInfo playerInfo = *++gisHouseInfo.playerInfoList.begin();
                    if (playerInfo.playerClass == HUMANPLAYERCLASS) {
                        if (!thisPlayerPlaced) {
                            curHouseInfo.player2DropDown.addEntry(playername, PLAYER_HUMAN);
                            curHouseInfo.player2DropDown.setSelectedItem(0);
                            thisPlayerPlaced = true;
                        } else {
                            curHouseInfo.player2DropDown.addEntry(_("open"), PLAYER_OPEN);
                            curHouseInfo.player2DropDown.setSelectedItem(0);
                        }
                    } else {
                        const PlayerFactory::PlayerData* pPlayerData =
                            PlayerFactory::getByPlayerClass(playerInfo.playerClass);
                        int index = PlayerFactory::getIndexByPlayerClass(playerInfo.playerClass);

                        if (pPlayerData != nullptr) {
                            curHouseInfo.player1DropDown.addEntry(pPlayerData->getName(), -(index + 2));
                            curHouseInfo.player1DropDown.setSelectedItem(0);
                            curHouseInfo.player1DropDown.setEnabled(false);
                            curHouseInfo.player1DropDown.setOnClickEnabled(false);
                        }
                    }
                }
            }
        } else if (bServer && !thisPlayerPlaced) {
            curHouseInfo.player2DropDown.addEntry(playername, PLAYER_HUMAN);
            curHouseInfo.player2DropDown.setSelectedItem(0);
            curHouseInfo.player2DropDown.setEnabled(bServer);
            thisPlayerPlaced = true;
        } else {
            curHouseInfo.player2DropDown.addEntry(_("open"), PLAYER_OPEN);
            curHouseInfo.player2DropDown.setSelectedItem(0);
            curHouseInfo.player2DropDown.addEntry(_("closed"), PLAYER_CLOSED);
            for (unsigned int k = 1; k < PlayerFactory::getList().size(); k++) {
                curHouseInfo.player2DropDown.addEntry(PlayerFactory::getByIndex(k)->getName(), k);
            }
            curHouseInfo.player2DropDown.setEnabled(bServer);
        }
        curHouseInfo.player2DropDown.setOnSelectionChange(
            [this, capture0 = 2 * i + 1](auto interactive) { onChangePlayerDropDownBoxes(interactive, capture0); });
        curHouseInfo.player2DropDown.setOnClick([this, capture0 = 2 * i + 1] { onClickPlayerDropDownBox(capture0); });
        curHouseInfo.playerHBox.addWidget(&curHouseInfo.player2DropDown, 100);

        curHouseInfo.houseInfoVBox.addWidget(&curHouseInfo.playerHBox);

        playerListVBox.addWidget(&curHouseInfo.houseInfoVBox, 0.01);

        playerListVBox.addWidget(Widget::create<VSpacer>(4).release(), 0.0);
        playerListVBox.addWidget(Widget::create<Spacer>().release(), 0.07);

        if (i >= numHouses) {
            curHouseInfo.houseInfoVBox.setEnabled(false);
            curHouseInfo.houseInfoVBox.setVisible(false);
        }
    }

    onChangeHousesDropDownBoxes(false);

    checkPlayerBoxes();

    leftVBox.addWidget(Widget::create<Spacer>().release(), 0.3);

    // maybe there is a better fitting slot if we are loading a map in the old format with Brain=CPU and Brain=Human
    if (brainEqHumanSlot >= 0) {
        DropDownBox& dropDownBox1 = houseInfo[brainEqHumanSlot].player1DropDown;
        DropDownBox& dropDownBox2 = houseInfo[brainEqHumanSlot].player2DropDown;

        if (dropDownBox1.getSelectedEntry() != playername && dropDownBox2.getSelectedEntry() != playername) {
            if (dropDownBox1.getSelectedEntryIntData() == PLAYER_OPEN) {
                setPlayer2Slot(playername, brainEqHumanSlot * 2);
            } else if (dropDownBox2.getSelectedEntryIntData() == PLAYER_OPEN
                       && gameInitSettings.isMultiplePlayersPerHouse()) {
                setPlayer2Slot(playername, brainEqHumanSlot * 2 + 1);
            }
        }
    }

    if (auto* const network_manager = dune::globals::pNetworkManager.get()) {
        if (bServer) {
            network_manager->startServer(bLANServer, gameInitSettings.getServername(), playername, &gameInitSettings, 1,
                                         gameInitSettings.isMultiplePlayersPerHouse() ? numHouses * 2 : numHouses);
        }

        network_manager->setOnPeerDisconnected(
            [this](const auto& playername, auto host, auto cause) { onPeerDisconnected(playername, host, cause); });
        network_manager->setOnReceiveChangeEventList([this](const auto& events) { onReceiveChangeEventList(events); });
        network_manager->setOnReceiveChatMessage(
            [this](const auto& name, const auto& message) { onReceiveChatMessage(name, message); });

        if (bServer) {
            network_manager->setGetChangeEventListForNewPlayerCallback(
                [this](const auto& newPlayerName) { return getChangeEventListForNewPlayer(newPlayerName); });
        } else {
            network_manager->setOnStartGame([this](auto timeleft) { onStartGame(timeleft); });
        }
    }
}

CustomGamePlayers::~CustomGamePlayers() {
    auto* const network_manager = dune::globals::pNetworkManager.get();
    if (network_manager == nullptr)
        return;

    network_manager->disconnect();

    network_manager->setOnPeerDisconnected({});
    network_manager->setGetChangeEventListForNewPlayerCallback({});
    network_manager->setOnReceiveChangeEventList({});
    network_manager->setOnReceiveChatMessage({});
    network_manager->setOnStartGame({});

    if (bServer) {
        try {
            network_manager->stopServer();
        } catch (std::exception& e) {
            sdl2::log_info("CustomGamePlayers::~CustomGamePlayers(): %s", e.what());
        }
    }
}

void CustomGamePlayers::update() {
    if (startGameTime <= dune::dune_clock::time_point::min())
        return;

    const auto now = dune::dune_clock::now();
    if (now >= startGameTime) {
        startGameTime = now;

        auto* const network_manager = dune::globals::pNetworkManager.get();

        network_manager->setOnPeerDisconnected(std::function<void(const std::string&, bool, int)>());
        network_manager->setGetChangeEventListForNewPlayerCallback(
            std::function<ChangeEventList(const std::string&)>());
        network_manager->setOnReceiveChangeEventList(std::function<void(const ChangeEventList&)>());
        network_manager->setOnReceiveChatMessage(std::function<void(const std::string&, const std::string&)>());
        network_manager->setOnStartGame(std::function<void(dune::dune_clock::duration)>());

        if (bServer) {
            network_manager->stopServer();
        }

        addAllPlayersToGameInitSettings();
        startMultiPlayerGame(gameInitSettings, [&](const auto& e) { doInput(e); });

        quit(MENU_QUIT_GAME_FINISHED);
    } else {
        using namespace std::chrono_literals;

        static dune::dune_clock::duration lastSecondLeft{};
        const auto secondsLeft = 1s + (startGameTime - dune::dune_clock::now());
        if (lastSecondLeft != secondsLeft) {
            lastSecondLeft = secondsLeft;
            addInfoMessage("Starting game in "
                           + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(secondsLeft).count())
                           + "...");
        }
    }
}

void CustomGamePlayers::onReceiveChangeEventList(const ChangeEventList& changeEventList) {
    for (const auto& changeEvent : changeEventList.changeEventList_) {

        switch (changeEvent.eventType_) {
            case ChangeEventList::ChangeEvent::EventType::ChangeHouse: {
                auto houseType = static_cast<HOUSETYPE>(changeEvent.newValue_);

                HouseInfo& curHouseInfo = houseInfo[changeEvent.slot_];

                for (int i = 0; i < curHouseInfo.houseDropDown.getNumEntries(); i++) {
                    if (curHouseInfo.houseDropDown.getEntryIntData(i) == static_cast<int>(houseType)) {
                        curHouseInfo.houseDropDown.setSelectedItem(i);
                        break;
                    }
                }
            } break;

            case ChangeEventList::ChangeEvent::EventType::ChangeTeam: {
                const int newTeam = static_cast<int>(changeEvent.newValue_);

                HouseInfo& curHouseInfo = houseInfo[changeEvent.slot_];

                for (int i = 0; i < curHouseInfo.teamDropDown.getNumEntries(); i++) {
                    if (curHouseInfo.teamDropDown.getEntryIntData(i) == newTeam) {
                        curHouseInfo.teamDropDown.setSelectedItem(i);
                        break;
                    }
                }
            } break;

            case ChangeEventList::ChangeEvent::EventType::ChangePlayer: {
                const int newPlayer = static_cast<int>(changeEvent.newValue_);

                HouseInfo& curHouseInfo = houseInfo[changeEvent.slot_ / 2];

                if (changeEvent.slot_ % 2 == 0) {
                    for (int i = 0; i < curHouseInfo.player1DropDown.getNumEntries(); i++) {
                        if (curHouseInfo.player1DropDown.getEntryIntData(i) == newPlayer) {
                            curHouseInfo.player1DropDown.setSelectedItem(i);
                            break;
                        }
                    }
                } else {
                    for (int i = 0; i < curHouseInfo.player2DropDown.getNumEntries(); i++) {
                        if (curHouseInfo.player2DropDown.getEntryIntData(i) == newPlayer) {
                            curHouseInfo.player2DropDown.setSelectedItem(i);
                            break;
                        }
                    }
                }

                checkPlayerBoxes();
            } break;

            case ChangeEventList::ChangeEvent::EventType::SetHumanPlayer: {
                const std::string& name = changeEvent.newStringValue_;
                const int slot          = changeEvent.slot_;

                setPlayer2Slot(name, slot);

                checkPlayerBoxes();
            } break;

            default: {
            } break;
        }
    }

    auto* const network_manager = dune::globals::pNetworkManager.get();
    if (network_manager != nullptr && bServer) {
        const ChangeEventList changeEventList2 = getChangeEventList();

        network_manager->sendChangeEventList(changeEventList2);
    }
}

ChangeEventList CustomGamePlayers::getChangeEventList() const {
    ChangeEventList changeEventList;

    for (int i = 0; std::cmp_less(i, houseInfo.size()); ++i) {
        auto& curHouseInfo = houseInfo[i];

        auto houseID = curHouseInfo.houseDropDown.getSelectedEntryIntData();
        auto team    = curHouseInfo.teamDropDown.getSelectedEntryIntData();
        auto player1 = curHouseInfo.player1DropDown.getSelectedEntryIntData();
        auto player2 = curHouseInfo.player2DropDown.getSelectedEntryIntData();

        changeEventList.changeEventList_.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangeHouse, i, houseID);
        changeEventList.changeEventList_.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangeTeam, i, team);

        if (player1 == PLAYER_HUMAN) {
            auto playername = curHouseInfo.player1DropDown.getSelectedEntry();
            changeEventList.changeEventList_.emplace_back(2 * i, std::move(playername));
        } else {
            changeEventList.changeEventList_.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangePlayer, 2 * i,
                                                          player1);
        }

        if (player2 == PLAYER_HUMAN) {
            auto playername = curHouseInfo.player2DropDown.getSelectedEntry();
            changeEventList.changeEventList_.emplace_back(2 * i + 1, std::move(playername));
        } else {
            changeEventList.changeEventList_.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangePlayer,
                                                          2 * i + 1, player2);
        }
    }

    return changeEventList;
}

ChangeEventList CustomGamePlayers::getChangeEventListForNewPlayer(const std::string& newPlayerName) {
    ChangeEventList changeEventList = getChangeEventList();

    // find a place for the new player

    int newPlayerSlot = INVALID;

    // look for an open left slot
    for (int i = 0; i < numHouses; i++) {
        HouseInfo& curHouseInfo = houseInfo[i];
        const int player1       = curHouseInfo.player1DropDown.getSelectedEntryIntData();

        if (player1 == PLAYER_OPEN) {
            newPlayerSlot = 2 * i;
            break;
        }
    }

    // if multiple players per house look for an open right slot
    if (newPlayerSlot == INVALID && gameInitSettings.isMultiplePlayersPerHouse()) {
        for (int i = 0; i < numHouses; i++) {
            HouseInfo& curHouseInfo = houseInfo[i];
            const int player2       = curHouseInfo.player2DropDown.getSelectedEntryIntData();

            if (player2 == PLAYER_OPEN) {
                newPlayerSlot = 2 * i + 1;
                break;
            }
        }
    }

    // as a fallback look for any non-human slot
    if (newPlayerSlot == INVALID) {
        for (int i = 0; i < numHouses; i++) {
            HouseInfo& curHouseInfo = houseInfo[i];
            const int player1       = curHouseInfo.player1DropDown.getSelectedEntryIntData();
            const int player2       = curHouseInfo.player2DropDown.getSelectedEntryIntData();

            if (player1 != PLAYER_HUMAN) {
                newPlayerSlot = 2 * i;
                break;
            }
            if (gameInitSettings.isMultiplePlayersPerHouse() && player2 != PLAYER_HUMAN) {

                newPlayerSlot = 2 * i + 1;

                break;
            }
        }
    }

    if (newPlayerSlot != INVALID) {
        setPlayer2Slot(newPlayerName, newPlayerSlot);

        ChangeEventList::ChangeEvent changeEvent(newPlayerSlot, newPlayerName);

        // add changeEvent to changeEventList for the new player
        changeEventList.changeEventList_.push_back(changeEvent);

        // send it to all other connected peers
        ChangeEventList changeEventList2;
        changeEventList2.changeEventList_.emplace_back(std::move(changeEvent));

        dune::globals::pNetworkManager->sendChangeEventList(changeEventList2);
    }

    return changeEventList;
}

void CustomGamePlayers::onReceiveChatMessage(const std::string& name, const std::string& message) {
    addChatMessage(name, message);
}

void CustomGamePlayers::onNext() {
    // check if we have at least two houses on the map and if we have more than one team
    int numUsedHouses = 0;
    int numTeams      = 0;
    for (auto i = 0U; i < houseInfo.size(); ++i) {
        auto& curHouseInfo = houseInfo[i];

        const int currentPlayer1 = curHouseInfo.player1DropDown.getSelectedEntryIntData();
        const int currentPlayer2 = curHouseInfo.player2DropDown.getSelectedEntryIntData();

        if ((currentPlayer1 != PLAYER_OPEN && currentPlayer1 != PLAYER_CLOSED)
            || (currentPlayer2 != PLAYER_OPEN && currentPlayer2 != PLAYER_CLOSED)) {
            numUsedHouses++;

            const int currentTeam = curHouseInfo.teamDropDown.getSelectedEntryIntData();
            bool bTeamFound       = false;
            for (auto j = 0U; j < i; ++j) {
                auto& house_info = houseInfo[j];

                const int player1 = house_info.player1DropDown.getSelectedEntryIntData();
                const int player2 = house_info.player2DropDown.getSelectedEntryIntData();

                if ((player1 != PLAYER_OPEN && player1 != PLAYER_CLOSED)
                    || (player2 != PLAYER_OPEN && player2 != PLAYER_CLOSED)) {

                    const int team = house_info.teamDropDown.getSelectedEntryIntData();
                    if (currentTeam == team) {
                        bTeamFound = true;
                    }
                }
            }

            if (!bTeamFound) {
                numTeams++;
            }
        }
    }

    if (numUsedHouses < 2) {
        // No game possible with only 1 house
        openWindow(MsgBox::create(_("At least 2 houses must be controlled\nby a human player or an AI player!")));
    } else if (numTeams < 2) {
        // No game possible with only 1 team
        openWindow(MsgBox::create(_("There must be at least two different teams!")));
    } else {
        // start game

        addAllPlayersToGameInitSettings();

        if (auto* const network_manager = dune::globals::pNetworkManager.get()) {
            using namespace std::chrono_literals;

            constexpr auto timeLeft = dune::as_dune_clock_duration(5000);

            startGameTime = dune::dune_clock::now() + timeLeft;

            network_manager->sendStartGame(dune::as_milliseconds(timeLeft));

            disableAllDropDownBoxes();
        } else {
            startSinglePlayerGame(gameInitSettings, [&](const auto& e) { doInput(e); });

            quit(MENU_QUIT_GAME_FINISHED);
        }
    }
}

void CustomGamePlayers::addAllPlayersToGameInitSettings() {
    gameInitSettings.clearHouseInfo();

    for (auto& curHouseInfo : houseInfo) {

        const auto houseID = curHouseInfo.houseDropDown.getSelectedEntryIntData();
        const auto team    = curHouseInfo.teamDropDown.getSelectedEntryIntData();
        const int player1  = curHouseInfo.player1DropDown.getSelectedEntryIntData();
        auto player1name   = curHouseInfo.player1DropDown.getSelectedEntry();
        const auto player2 = curHouseInfo.player2DropDown.getSelectedEntryIntData();
        auto player2name   = curHouseInfo.player2DropDown.getSelectedEntry();

        GameInitSettings::HouseInfo newHouseInfo(static_cast<HOUSETYPE>(houseID), team);

        bool bAdded = false;
        bAdded |= addPlayerToHouseInfo(newHouseInfo, player1, player1name);
        bAdded |= addPlayerToHouseInfo(newHouseInfo, player2, player2name);

        if (bAdded) {
            gameInitSettings.addHouseInfo(newHouseInfo);
        }
    }
}

bool CustomGamePlayers::addPlayerToHouseInfo(GameInitSettings::HouseInfo& newHouseInfo, int player,
                                             const std::string& playername) {
    std::string playerName;
    std::string playerClass;

    const auto houseID = newHouseInfo.houseID;

    switch (player) {
        case PLAYER_HUMAN: {
            playerName  = playername;
            playerClass = HUMANPLAYERCLASS;
        } break;

        case PLAYER_OPEN:
        case PLAYER_CLOSED: {
            return false;
        }

        default: {
            // AI player
            const PlayerFactory::PlayerData* pPlayerData = PlayerFactory::getByIndex(player);
            if (pPlayerData == nullptr) {
                return false;
            }

            playerName  = houseID == HOUSETYPE::HOUSE_INVALID ? pPlayerData->getName() : getHouseNameByNumber(houseID);
            playerClass = pPlayerData->getPlayerClass();

        } break;
    }

    newHouseInfo.addPlayerInfo({playerName, playerClass});
    return true;
}

void CustomGamePlayers::onCancel() {
    quit();
}

void CustomGamePlayers::onSendChatMessage() {
    const std::string message{chatTextBox.getText()};
    addChatMessage(dune::globals::settings.general.playerName, message);
    chatTextBox.setText("");

    dune::globals::pNetworkManager->sendChatMessage(message);
}

void CustomGamePlayers::addInfoMessage(const std::string& message) {
    std::string text = chatTextView.getText();
    if (text.length() > 0) {
        text += "\n";
    }
    text += "* " + message;
    chatTextView.setText(text);
    chatTextView.scrollToEnd();
}

void CustomGamePlayers::addChatMessage(const std::string& name, const std::string& message) {
    std::string text = chatTextView.getText();
    if (text.length() > 0) {
        text += "\n";
    }
    text += name + ": " + message;
    chatTextView.setText(text);
    chatTextView.scrollToEnd();
}

void CustomGamePlayers::extractMapInfo(INIFile* pMap) {
    int sizeX = 0;
    int sizeY = 0;

    if (pMap->hasKey("MAP", "Seed")) {
        // old map format with seed value
        const int mapscale = pMap->getIntValue("BASIC", "MapScale", -1);

        switch (mapscale) {
            case 0: {
                sizeX = 62;
                sizeY = 62;
            } break;

            case 1: {
                sizeX = 32;
                sizeY = 32;
            } break;

            case 2: {
                sizeX = 21;
                sizeY = 21;
            } break;

            default: {
                sizeX = 64;
                sizeY = 64;
            }
        }
    } else {
        // new map format with saved map
        sizeX = pMap->getIntValue("MAP", "SizeX", 0);
        sizeY = pMap->getIntValue("MAP", "SizeY", 0);
    }

    mapPropertySize.setText(std::to_string(sizeX) + " x " + std::to_string(sizeY));

    sdl2::surface_ptr pMapSurface = nullptr;
    try {
        INIMapPreviewCreator mapPreviewCreator(pMap);
        pMapSurface = mapPreviewCreator.createMinimapImageOfMap(1, DuneStyle::buttonBorderColor);
    } catch (...) {
        pMapSurface = sdl2::surface_ptr{GUIStyle::getInstance().createButtonSurface(130, 130, "Error", true, false)};
        nextButton.setEnabled(false);
    }
    minimap.setSurface(std::move(pMapSurface));

    // clang-format off
    boundHousesOnMap.clear();
    if(pMap->hasSection("Harkonnen"))   boundHousesOnMap.push_back(HOUSETYPE::HOUSE_HARKONNEN);
    if(pMap->hasSection("Atreides"))    boundHousesOnMap.push_back(HOUSETYPE::HOUSE_ATREIDES);
    if(pMap->hasSection("Ordos"))       boundHousesOnMap.push_back(HOUSETYPE::HOUSE_ORDOS);
    if(pMap->hasSection("Fremen"))      boundHousesOnMap.push_back(HOUSETYPE::HOUSE_FREMEN);
    if(pMap->hasSection("Sardaukar"))   boundHousesOnMap.push_back(HOUSETYPE::HOUSE_SARDAUKAR);
    if(pMap->hasSection("Mercenary"))   boundHousesOnMap.push_back(HOUSETYPE::HOUSE_MERCENARY);
    // clang-format on

    numHouses = boundHousesOnMap.size();
    if (pMap->hasSection("Player1"))
        numHouses++;
    if (pMap->hasSection("Player2"))
        numHouses++;
    if (pMap->hasSection("Player3"))
        numHouses++;
    if (pMap->hasSection("Player4"))
        numHouses++;
    if (pMap->hasSection("Player5"))
        numHouses++;
    if (pMap->hasSection("Player6"))
        numHouses++;

    mapPropertyPlayers.setText(std::to_string(numHouses));

    std::string authors = pMap->getStringValue("BASIC", "Author", "-");
    if (authors.size() > 11) {
        authors = authors.substr(0, 9) + "...";
    }
    mapPropertyAuthors.setText(authors);

    mapPropertyLicense.setText(pMap->getStringValue("BASIC", "License", "-"));

    // find Brain=Human
    int currentIndex = 0;
    for (const auto& houseType : boundHousesOnMap) {
        if (strToUpper(pMap->getStringValue(getHouseNameByNumber(houseType), "Brain", "")) == "HUMAN") {
            brainEqHumanSlot = currentIndex;
        }
        currentIndex++;
    }

    currentIndex    = 0;
    int currentTeam = 0;
    std::vector<std::string> teamNames;
    for (const auto& houseType : boundHousesOnMap) {
        std::string teamName = strToUpper(
            pMap->getStringValue(getHouseNameByNumber(houseType), "Brain", "Team " + std::to_string(currentIndex + 1)));

        const auto it = std::ranges::find(teamNames, teamName);
        if (it == teamNames.end()) {
            teamNames.push_back(teamName);
            slotToTeam[currentIndex] = currentTeam;
            currentTeam++;
        } else {
            slotToTeam[currentIndex] = slotToTeam[it - teamNames.begin()];
        }

        currentIndex++;
    }

    for (int p = 0; p < NUM_HOUSES && currentIndex < NUM_HOUSES; p++) {
        if (pMap->hasSection("Player" + std::to_string(p + 1))) {
            std::string teamName = strToUpper(pMap->getStringValue("Player" + std::to_string(p + 1), "Brain",
                                                                   "Team " + std::to_string(currentIndex + p + 1)));

            const auto it = std::ranges::find(teamNames, teamName);
            if (it == teamNames.end()) {
                teamNames.push_back(teamName);
                slotToTeam[currentIndex] = currentTeam;
                currentTeam++;
            } else {
                slotToTeam[currentIndex] = slotToTeam[it - teamNames.begin()];
            }

            currentIndex++;
        }
    }

    std::fill(slotToTeam.begin() + currentIndex, slotToTeam.end(), -1);
}

void CustomGamePlayers::onChangeHousesDropDownBoxes(bool bInteractive, int houseInfoNum) {
    auto* const network_manager = dune::globals::pNetworkManager.get();

    if (bInteractive && houseInfoNum >= 0 && network_manager != nullptr) {
        int selectedHouseID = houseInfo[houseInfoNum].houseDropDown.getSelectedEntryIntData();

        ChangeEventList changeEventList;
        changeEventList.changeEventList_.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangeHouse,
                                                      houseInfoNum, selectedHouseID);

        network_manager->sendChangeEventList(changeEventList);
    }

    const int numBoundHouses = boundHousesOnMap.size();
    int numUsedBoundHouses   = 0;
    int numUsedRandomHouses  = 0;
    for (int i = 0; i < numHouses; i++) {
        const auto selectedHouseID = static_cast<HOUSETYPE>(houseInfo[i].houseDropDown.getSelectedEntryIntData());

        if (selectedHouseID == HOUSETYPE::HOUSE_INVALID) {
            numUsedRandomHouses++;
        } else {
            if (isBoundedHouseOnMap(selectedHouseID)) {
                numUsedBoundHouses++;
            }
        }
    }

    for (int i = 0; i < numHouses; i++) {
        HouseInfo& curHouseInfo = houseInfo[i];

        const auto house = static_cast<HOUSETYPE>(curHouseInfo.houseDropDown.getSelectedEntryIntData());

        if (houseInfoNum == -1 || houseInfoNum == i) {

            const auto color =
                house == HOUSETYPE::HOUSE_INVALID
                    ? COLOR_RGB(20, 20, 40)
                    : SDL2RGB(dune::globals::palette[dune::globals::houseToPaletteIndex[static_cast<int>(house)] + 3]);
            curHouseInfo.houseLabel.setTextColor(color);
            curHouseInfo.houseDropDown.setColor(color);
            curHouseInfo.teamDropDown.setColor(color);
            curHouseInfo.player1Label.setTextColor(color);
            curHouseInfo.player1DropDown.setColor(color);
            curHouseInfo.player2Label.setTextColor(color);
            curHouseInfo.player2DropDown.setColor(color);

            const auto* const gfx = dune::globals::pGFXManager.get();

            if (house == HOUSETYPE::HOUSE_INVALID) {
                curHouseInfo.player1ArrowLabel.setTexture(gfx->getUIGraphic(UI_CustomGamePlayersArrowNeutral));
            } else {
                curHouseInfo.player1ArrowLabel.setTexture(gfx->getUIGraphic(UI_CustomGamePlayersArrow, house));
            }
        }

        if (gameInitSettings.getGameType() == GameType::LoadMultiplayer) {
            // no house changes possible
            continue;
        }

        addToHouseDropDown(curHouseInfo.houseDropDown, HOUSETYPE::HOUSE_INVALID);

        for (int h = 0; h < NUM_HOUSES; h++) {
            bool bAddHouse = 0;

            bool bCheck = 0;

            if (house == HOUSETYPE::HOUSE_INVALID || isBoundedHouseOnMap(house)) {
                bCheck = numUsedBoundHouses + numUsedRandomHouses - 1 >= numBoundHouses;
            } else {
                bCheck = numUsedBoundHouses + numUsedRandomHouses >= numBoundHouses;
            }

            if (bCheck) {
                bAddHouse = true;
                for (int j = 0; j < numHouses; j++) {
                    if (i != j) {
                        DropDownBox& tmpDropDown = houseInfo[j].houseDropDown;
                        if (tmpDropDown.getSelectedEntryIntData() == h) {
                            bAddHouse = false;
                            break;
                        }
                    }
                }
            } else {
                bAddHouse = h == static_cast<int>(house);

                if ((house == HOUSETYPE::HOUSE_INVALID || isBoundedHouseOnMap(house))
                    && isBoundedHouseOnMap(static_cast<HOUSETYPE>(h))) {
                    // check if this entry is random or a bounded house but is needed for a bounded house
                    bAddHouse = true;
                    for (int j = 0; j < numHouses; j++) {
                        if (i != j) {
                            DropDownBox& tmpDropDown = houseInfo[j].houseDropDown;
                            if (tmpDropDown.getSelectedEntryIntData() == h) {
                                bAddHouse = false;
                                break;
                            }
                        }
                    }
                }
            }

            if (bAddHouse) {
                addToHouseDropDown(curHouseInfo.houseDropDown, static_cast<HOUSETYPE>(h));
            } else {
                removeFromHouseDropDown(curHouseInfo.houseDropDown, static_cast<HOUSETYPE>(h));
            }
        }
    }
}

void CustomGamePlayers::onChangeTeamDropDownBoxes(bool bInteractive, int houseInfoNum) {

    auto* const network_manager = dune::globals::pNetworkManager.get();

    if (bInteractive && houseInfoNum >= 0 && network_manager != nullptr) {
        int selectedTeam = houseInfo[houseInfoNum].teamDropDown.getSelectedEntryIntData();

        ChangeEventList changeEventList;
        changeEventList.changeEventList_.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangeTeam, houseInfoNum,
                                                      selectedTeam);

        network_manager->sendChangeEventList(changeEventList);
    }
}

void CustomGamePlayers::onChangePlayerDropDownBoxes(bool bInteractive, int boxnum) {
    auto* const network_manager = dune::globals::pNetworkManager.get();

    if (bInteractive && boxnum >= 0 && network_manager != nullptr) {
        const DropDownBox& dropDownBox =
            boxnum % 2 == 0 ? houseInfo[boxnum / 2].player1DropDown : houseInfo[boxnum / 2].player2DropDown;

        int selectedPlayer = dropDownBox.getSelectedEntryIntData();

        ChangeEventList changeEventList;
        changeEventList.changeEventList_.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangePlayer, boxnum,
                                                      selectedPlayer);

        network_manager->sendChangeEventList(changeEventList);
    }

    checkPlayerBoxes();
}

void CustomGamePlayers::onClickPlayerDropDownBox(int boxnum) {
    const DropDownBox& dropDownBox =
        boxnum % 2 == 0 ? houseInfo[boxnum / 2].player1DropDown : houseInfo[boxnum / 2].player2DropDown;

    if (dropDownBox.getSelectedEntryIntData() == PLAYER_CLOSED) {
        return;
    }

    auto playername = dune::globals::settings.general.playerName;

    setPlayer2Slot(playername, boxnum);

    if (boxnum >= 0) {
        if (auto* const network_manager = dune::globals::pNetworkManager.get()) {
            ChangeEventList changeEventList;
            changeEventList.changeEventList_.emplace_back(boxnum, std::move(playername));

            network_manager->sendChangeEventList(changeEventList);
        }
    }
}

void CustomGamePlayers::onPeerDisconnected(const std::string& playername, bool bHost, int cause) {
    if (bHost) {
        quit(cause);
        return;
    }

    for (int i = 0; i < numHouses * 2; i++) {
        DropDownBox& curDropDownBox = i % 2 == 0 ? houseInfo[i / 2].player1DropDown : houseInfo[i / 2].player2DropDown;

        if (curDropDownBox.getSelectedEntryIntData() == PLAYER_HUMAN
            && curDropDownBox.getSelectedEntry() == playername) {

            curDropDownBox.clearAllEntries();
            curDropDownBox.addEntry(_("open"), PLAYER_OPEN);
            curDropDownBox.setSelectedItem(0);

            if (gameInitSettings.getGameType() != GameType::LoadMultiplayer) {
                curDropDownBox.addEntry(_("closed"), PLAYER_CLOSED);
                for (unsigned int k = 1; k < PlayerFactory::getList().size(); k++) {
                    curDropDownBox.addEntry(PlayerFactory::getByIndex(k)->getName(), k);
                }
            }

            break;
        }
    }

    if (!bServer) {
        for (int i = 0; i < numHouses; i++) {
            bool bIsThisPlayer = false;
            if (houseInfo[i].player1DropDown.getSelectedEntryIntData() == PLAYER_HUMAN) {
                if (houseInfo[i].player1DropDown.getSelectedEntry() == dune::globals::settings.general.playerName) {
                    bIsThisPlayer = true;
                }
            }

            if (houseInfo[i].player2DropDown.getSelectedEntryIntData() == PLAYER_HUMAN) {
                if (houseInfo[i].player2DropDown.getSelectedEntry() == dune::globals::settings.general.playerName) {
                    bIsThisPlayer = true;
                }
            }

            houseInfo[i].player1DropDown.setEnabled(bIsThisPlayer);
            houseInfo[i].player2DropDown.setEnabled(bIsThisPlayer);
            houseInfo[i].houseDropDown.setEnabled(bIsThisPlayer);
            houseInfo[i].teamDropDown.setEnabled(bIsThisPlayer);
        }
    }

    checkPlayerBoxes();

    addInfoMessage(playername + " disconnected!");
}

void CustomGamePlayers::onStartGame(dune::dune_clock::duration timeLeft) {
    startGameTime = dune::dune_clock::now() + timeLeft;
    disableAllDropDownBoxes();
}

void CustomGamePlayers::setPlayer2Slot(const std::string& playername, int slot) {
    DropDownBox& dropDownBox =
        slot % 2 == 0 ? houseInfo[slot / 2].player1DropDown : houseInfo[slot / 2].player2DropDown;

    std::string oldPlayerName;
    const int oldPlayerType = dropDownBox.getSelectedEntryIntData();
    if (oldPlayerType == PLAYER_HUMAN) {
        oldPlayerName = dropDownBox.getSelectedEntry();
    }

    for (int i = 0; i < numHouses * 2; i++) {
        DropDownBox& curDropDownBox = i % 2 == 0 ? houseInfo[i / 2].player1DropDown : houseInfo[i / 2].player2DropDown;

        if (curDropDownBox.getSelectedEntryIntData() == PLAYER_HUMAN
            && curDropDownBox.getSelectedEntry() == playername) {

            if (oldPlayerType == PLAYER_HUMAN) {
                curDropDownBox.clearAllEntries();
                curDropDownBox.addEntry(oldPlayerName, PLAYER_HUMAN);
                curDropDownBox.setSelectedItem(0);
            } else {
                curDropDownBox.clearAllEntries();
                curDropDownBox.addEntry(_("open"), PLAYER_OPEN);

                if (gameInitSettings.getGameType() != GameType::LoadMultiplayer) {
                    curDropDownBox.addEntry(_("closed"), PLAYER_CLOSED);
                    for (unsigned int k = 1; k < PlayerFactory::getList().size(); k++) {
                        curDropDownBox.addEntry(PlayerFactory::getByIndex(k)->getName(), k);
                    }
                }

                for (int j = 0; j < curDropDownBox.getNumEntries(); j++) {
                    if (curDropDownBox.getEntryIntData(j) == oldPlayerType) {
                        curDropDownBox.setSelectedItem(j);
                        break;
                    }
                }
            }

            break;
        }
    }

    dropDownBox.clearAllEntries();
    dropDownBox.addEntry(playername, PLAYER_HUMAN);
    dropDownBox.setSelectedItem(0);

    if (!bServer) {
        for (int i = 0; i < numHouses; i++) {
            bool bIsThisPlayer = false;

            if (gameInitSettings.getGameType() != GameType::LoadMultiplayer) {
                const auto& player_name = dune::globals::settings.general.playerName;

                if (houseInfo[i].player1DropDown.getSelectedEntryIntData() == PLAYER_HUMAN) {
                    if (houseInfo[i].player1DropDown.getSelectedEntry() == player_name) {
                        bIsThisPlayer = true;
                    }
                }

                if (houseInfo[i].player2DropDown.getSelectedEntryIntData() == PLAYER_HUMAN) {
                    if (houseInfo[i].player2DropDown.getSelectedEntry() == player_name) {
                        bIsThisPlayer = true;
                    }
                }
            }

            houseInfo[i].player1DropDown.setEnabled(bIsThisPlayer);
            houseInfo[i].player2DropDown.setEnabled(bIsThisPlayer);
            houseInfo[i].houseDropDown.setEnabled(bIsThisPlayer);
            houseInfo[i].teamDropDown.setEnabled(bIsThisPlayer);
        }
    }

    checkPlayerBoxes();
}

void CustomGamePlayers::checkPlayerBoxes() {
    int numPlayers = 0;

    for (int i = 0; i < numHouses; i++) {
        HouseInfo& curHouseInfo = houseInfo[i];

        const int player1 = curHouseInfo.player1DropDown.getSelectedEntryIntData();
        const int player2 = curHouseInfo.player2DropDown.getSelectedEntryIntData();

        if (player1 != PLAYER_OPEN) {
            numPlayers++;
        }

        if (gameInitSettings.isMultiplePlayersPerHouse() && player2 != PLAYER_OPEN) {
            numPlayers++;
        }

        if (!gameInitSettings.isMultiplePlayersPerHouse() || (player1 == PLAYER_OPEN && player2 == PLAYER_OPEN)
            || curHouseInfo.player2DropDown.getNumEntries() == 0) {
            curHouseInfo.player2DropDown.setVisible(false);
            curHouseInfo.player2DropDown.setEnabled(false);
            curHouseInfo.player2Label.setVisible(false);
        } else {
            curHouseInfo.player2DropDown.setVisible(true);

            bool bEnableDropDown2 = bServer;
            if (!bServer) {
                const auto& player_name = dune::globals::settings.general.playerName;

                if (player1 == PLAYER_HUMAN) {
                    if (curHouseInfo.player1DropDown.getSelectedEntry() == player_name) {
                        bEnableDropDown2 = true;
                    }
                }

                if (player2 == PLAYER_HUMAN) {
                    if (curHouseInfo.player2DropDown.getSelectedEntry() == player_name) {
                        bEnableDropDown2 = true;
                    }
                }
            }

            if (gameInitSettings.getGameType() == GameType::LoadMultiplayer && player2 != PLAYER_OPEN
                && player2 != PLAYER_HUMAN) {
                curHouseInfo.player2DropDown.setEnabled(false);
            } else {
                curHouseInfo.player2DropDown.setEnabled(bEnableDropDown2);
            }
            curHouseInfo.player2Label.setVisible(true);
        }
    }

    if (auto* const network_manager = dune::globals::pNetworkManager.get()) {
        if (bServer)
            network_manager->updateServer(numPlayers);
    }
}

void CustomGamePlayers::addToHouseDropDown(DropDownBox& houseDropDownBox, HOUSETYPE house, bool bSelect) {

    if (houseDropDownBox.getNumEntries() == 0) {
        if (house == HOUSETYPE::HOUSE_INVALID) {
            houseDropDownBox.addEntry(_("Random"), static_cast<int>(HOUSETYPE::HOUSE_INVALID));
        } else {
            houseDropDownBox.addEntry(getHouseNameByNumber(house), static_cast<int>(house));
        }

        if (bSelect) {
            houseDropDownBox.setSelectedItem(0);
        }
    } else {

        if (house == HOUSETYPE::HOUSE_INVALID) {
            if (houseDropDownBox.getEntryIntData(0) != static_cast<int>(HOUSETYPE::HOUSE_INVALID)) {
                houseDropDownBox.insertEntry(0, _("Random"), static_cast<int>(HOUSETYPE::HOUSE_INVALID));
            }

            if (bSelect) {
                houseDropDownBox.setSelectedItem(houseDropDownBox.getNumEntries() - 1);
            }
        } else {

            int currentItemIndex =
                houseDropDownBox.getEntryIntData(0) == static_cast<int>(HOUSETYPE::HOUSE_INVALID) ? 1 : 0;

            for (int h = 0; h < NUM_HOUSES; h++) {
                if (currentItemIndex < houseDropDownBox.getNumEntries()
                    && houseDropDownBox.getEntryIntData(currentItemIndex) == h) {
                    if (h == static_cast<int>(house)) {
                        if (bSelect) {
                            houseDropDownBox.setSelectedItem(currentItemIndex);
                        }
                        break;
                    }

                    currentItemIndex++;
                } else {
                    if (h == static_cast<int>(house)) {
                        houseDropDownBox.insertEntry(currentItemIndex, getHouseNameByNumber(static_cast<HOUSETYPE>(h)),
                                                     h);

                        if (bSelect) {
                            houseDropDownBox.setSelectedItem(currentItemIndex);
                        }
                        break;
                    }
                }
            }
        }
    }
}

void CustomGamePlayers::removeFromHouseDropDown(DropDownBox& houseDropDownBox, HOUSETYPE house) {

    for (int i = 0; i < houseDropDownBox.getNumEntries(); i++) {
        if (houseDropDownBox.getEntryIntData(i) == static_cast<int>(house)) {
            houseDropDownBox.removeEntry(i);
            break;
        }
    }
}

bool CustomGamePlayers::isBoundedHouseOnMap(HOUSETYPE houseID) {
    return std::ranges::find(boundHousesOnMap, houseID) != boundHousesOnMap.end();
}

void CustomGamePlayers::disableAllDropDownBoxes() {
    for (auto& curHouseInfo : houseInfo) {
        curHouseInfo.houseDropDown.setEnabled(false);
        curHouseInfo.houseDropDown.setOnClickEnabled(false);
        curHouseInfo.teamDropDown.setEnabled(false);
        curHouseInfo.teamDropDown.setOnClickEnabled(false);
        curHouseInfo.player1DropDown.setEnabled(false);
        curHouseInfo.player1DropDown.setOnClickEnabled(false);
        curHouseInfo.player2DropDown.setEnabled(false);
        curHouseInfo.player2DropDown.setOnClickEnabled(false);
    }

    backButton.setEnabled(false);
    nextButton.setEnabled(false);
}
