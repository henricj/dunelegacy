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
#include <FileClasses/TextManager.h>
#include <FileClasses/INIFile.h>

#include <Network/NetworkManager.h>

#include <GUI/Spacer.h>
#include <GUI/GUIStyle.h>
#include <GUI/MsgBox.h>
#include <GUI/dune/DuneStyle.h>

#include <players/PlayerFactory.h>

#include <misc/fnkdat.h>
#include <misc/FileSystem.h>
#include <misc/draw_util.h>
#include <misc/string_util.h>
#include <misc/IMemoryStream.h>

#include <INIMap/INIMapPreviewCreator.h>

#include <sand.h>
#include <globals.h>


#define PLAYER_HUMAN        0
#define PLAYER_OPEN         -1
#define PLAYER_CLOSED       -2


CustomGamePlayers::CustomGamePlayers(const GameInitSettings& newGameInitSettings, bool server, bool LANServer)
 : MenuBase(), gameInitSettings(newGameInitSettings), bServer(server), bLANServer(LANServer), startGameTime(0), brainEqHumanSlot(-1) {

    // set up window
    SDL_Texture *pBackground = pGFXManager->getUIGraphic(UI_MenuBackground);
    setBackground(pBackground);
    resize(getTextureSize(pBackground));

    setWindowWidget(&windowWidget);

    windowWidget.addWidget(&mainVBox, Point(24,23), Point(getRendererWidth() - 48, getRendererHeight() - 32));

    captionLabel.setText(getBasename(gameInitSettings.getFilename(), true));
    captionLabel.setAlignment(Alignment_HCenter);
    mainVBox.addWidget(&captionLabel, 24);
    mainVBox.addWidget(VSpacer::create(24));

    mainVBox.addWidget(Spacer::create(), 0.04);

    mainVBox.addWidget(&mainHBox, 0.6);

    mainHBox.addWidget(Spacer::create(), 0.05);
    leftVBox.addWidget(Spacer::create(), 0.1);
    leftVBox.addWidget(&playerListHBox);
    playerListHBox.addWidget(Spacer::create(), 0.4);
    playerListHBox.addWidget(&playerListVBox, 0.2);
    playerListHBox.addWidget(Spacer::create(), 0.4);
    mainHBox.addWidget(&leftVBox, 0.8);

    mainHBox.addWidget(Spacer::create(), 0.05);

    mainHBox.addWidget(HSpacer::create(8));

    mainHBox.addWidget(&rightVBox, 180);
    mainHBox.addWidget(Spacer::create(), 0.05);
    minimap.setSurface( GUIStyle::getInstance().createButtonSurface(130,130,_("Choose map"), true, false) );
    rightVBox.addWidget(&minimap);

    if(gameInitSettings.getGameType() == GameType::CustomGame || gameInitSettings.getGameType() == GameType::CustomMultiplayer) {
        auto RWops = sdl2::RWops_ptr{ SDL_RWFromConstMem(gameInitSettings.getFiledata().c_str(), gameInitSettings.getFiledata().size()) };

        INIFile inimap(RWops.get());
        extractMapInfo(&inimap);
    } else if(gameInitSettings.getGameType() == GameType::LoadMultiplayer) {
        IMemoryStream memStream(gameInitSettings.getFiledata().c_str(), gameInitSettings.getFiledata().size());

        Uint32 magicNum = memStream.readUint32();
        if(magicNum != SAVEMAGIC) {
            SDL_Log("CustomGamePlayers: No valid savegame! Expected magic number %.8X, but got %.8X!", SAVEMAGIC, magicNum);
        }

        Uint32 savegameVersion = memStream.readUint32();
        if (savegameVersion != SAVEGAMEVERSION) {
            SDL_Log("CustomGamePlayers: No valid savegame! Expected savegame version %d, but got %d!", SAVEGAMEVERSION, savegameVersion);
        }

        memStream.readString();     // dune legacy version

        // read gameInitSettings
        GameInitSettings tmpGameInitSettings(memStream);

        Uint32 numHouseInfo = memStream.readUint32();
        for(Uint32 i=0;i<numHouseInfo;i++) {
            houseInfoListSetup.push_back(GameInitSettings::HouseInfo(memStream));
        }

        auto RWops = sdl2::RWops_ptr{ SDL_RWFromConstMem(tmpGameInitSettings.getFiledata().c_str(), tmpGameInitSettings.getFiledata().size()) };

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

    rightVBox.addWidget(VSpacer::create(10));
    rightVBox.addWidget(&mapPropertiesHBox, 0.01);
    mapPropertiesHBox.addWidget(&mapPropertyNamesVBox, 75);
    mapPropertiesHBox.addWidget(&mapPropertyValuesVBox, 105);
    mapPropertyNamesVBox.addWidget(Label::create(_("Size") + ":"));
    mapPropertyValuesVBox.addWidget(&mapPropertySize);
    mapPropertyNamesVBox.addWidget(Label::create(_("Players") + ":"));
    mapPropertyValuesVBox.addWidget(&mapPropertyPlayers);
    mapPropertyNamesVBox.addWidget(Label::create(_("Author") + ":"));
    mapPropertyValuesVBox.addWidget(&mapPropertyAuthors);
    mapPropertyNamesVBox.addWidget(Label::create(_("License") + ":"));
    mapPropertyValuesVBox.addWidget(&mapPropertyLicense);
    rightVBox.addWidget(Spacer::create());

    mainVBox.addWidget(Spacer::create(), 0.04);

    mainVBox.addWidget(&buttonHBox, 0.32);

    buttonHBox.addWidget(HSpacer::create(70));

    backButtonVBox.addWidget(Spacer::create());
    backButton.setText(_("Back"));
    backButton.setOnClick(std::bind(&CustomGamePlayers::onCancel, this));
    backButtonVBox.addWidget(&backButton, 24);
    backButtonVBox.addWidget(VSpacer::create(14));
    buttonHBox.addWidget(&backButtonVBox, 0.1);

    buttonHBox.addWidget(Spacer::create(), 0.0625);

    chatTextView.setTextFontSize(12);
    chatVBox.addWidget(&chatTextView, 0.77);
    if(getRendererHeight() <= 600) {
        chatTextBox.setTextFontSize(12);
    }
    chatTextBox.setOnReturn(std::bind(&CustomGamePlayers::onSendChatMessage, this));
    chatVBox.addWidget(&chatTextBox, 0.2);
    chatVBox.addWidget(Spacer::create(), 0.03);
    buttonHBox.addWidget(&chatVBox, 0.675);

    if(gameInitSettings.getGameType() != GameType::CustomMultiplayer && gameInitSettings.getGameType() != GameType::LoadMultiplayer) {
        chatVBox.setVisible(false);
        chatVBox.setEnabled(false);
    }

    bool bLoadMultiplayer = (gameInitSettings.getGameType() == GameType::LoadMultiplayer);

    buttonHBox.addWidget(Spacer::create(), 0.0625);

    nextButtonVBox.addWidget(Spacer::create());
    nextButton.setText(_("Next"));
    nextButton.setOnClick(std::bind(&CustomGamePlayers::onNext, this));
    nextButtonVBox.addWidget(&nextButton, 24);
    nextButtonVBox.addWidget(VSpacer::create(14));
    if(bServer == false) {
        nextButton.setEnabled(false);
        nextButton.setVisible(false);
    }
    buttonHBox.addWidget(&nextButtonVBox, 0.1);

    buttonHBox.addWidget(HSpacer::create(90));

    std::list<HOUSETYPE>  tmpBoundHousesOnMap = boundHousesOnMap;

    bool thisPlayerPlaced = false;

    for(int i=0;i<NUM_HOUSES;i++) {
        HouseInfo& curHouseInfo = houseInfo[i];

        // set up header row with Label "House", DropDown for house selection and DropDown for team selection
        curHouseInfo.houseLabel.setText(_("House"));
        curHouseInfo.houseHBox.addWidget(&curHouseInfo.houseLabel, 60);

        if(bLoadMultiplayer) {
            if(i < (int) houseInfoListSetup.size()) {
                GameInitSettings::HouseInfo gisHouseInfo = houseInfoListSetup.at(i);

                if(gisHouseInfo.houseID == HOUSE_INVALID) {
                    addToHouseDropDown(curHouseInfo.houseDropDown, HOUSE_INVALID, true);
                } else {
                    addToHouseDropDown(curHouseInfo.houseDropDown, gisHouseInfo.houseID, true);
                }
            }

            curHouseInfo.houseDropDown.setEnabled(false);
        } else if(tmpBoundHousesOnMap.empty() == false) {
            HOUSETYPE housetype = tmpBoundHousesOnMap.front();
            tmpBoundHousesOnMap.pop_front();

            addToHouseDropDown(curHouseInfo.houseDropDown, housetype, true);
            curHouseInfo.houseDropDown.setEnabled(bServer);
        } else {
            addToHouseDropDown(curHouseInfo.houseDropDown, HOUSE_INVALID, true);
            curHouseInfo.houseDropDown.setEnabled(bServer);
        }
        curHouseInfo.houseDropDown.setOnSelectionChange(std::bind(&CustomGamePlayers::onChangeHousesDropDownBoxes, this, std::placeholders::_1, i));
        curHouseInfo.houseHBox.addWidget(&curHouseInfo.houseDropDown, 95);

        if(bLoadMultiplayer) {
            if(i < (int) houseInfoListSetup.size()) {
                GameInitSettings::HouseInfo gisHouseInfo = houseInfoListSetup.at(i);

                curHouseInfo.teamDropDown.addEntry(_("Team") + " " + std::to_string(gisHouseInfo.team), gisHouseInfo.team);
                curHouseInfo.teamDropDown.setSelectedItem(0);
            }
            curHouseInfo.teamDropDown.setEnabled(false);
            curHouseInfo.teamDropDown.setOnClickEnabled(false);
        } else {
            for(int team = 0 ; team<numHouses ; team++) {
                curHouseInfo.teamDropDown.addEntry(_("Team") + " " + std::to_string(team+1), team+1);
            }
            curHouseInfo.teamDropDown.setSelectedItem(slotToTeam[i]);
            curHouseInfo.teamDropDown.setEnabled(bServer);
        }
        curHouseInfo.teamDropDown.setOnSelectionChange(std::bind(&CustomGamePlayers::onChangeTeamDropDownBoxes, this, std::placeholders::_1, i));
        curHouseInfo.houseHBox.addWidget(HSpacer::create(10));
        curHouseInfo.houseHBox.addWidget(&curHouseInfo.teamDropDown, 85);

        curHouseInfo.houseInfoVBox.addWidget(&curHouseInfo.houseHBox);

        // add 1. player
        curHouseInfo.player1ArrowLabel.setTexture(pGFXManager->getUIGraphic(UI_CustomGamePlayersArrowNeutral));
        curHouseInfo.playerHBox.addWidget(&curHouseInfo.player1ArrowLabel);
        curHouseInfo.player1Label.setText(_("Player") + (gameInitSettings.isMultiplePlayersPerHouse() ? " 1" : ""));
        curHouseInfo.player1Label.setTextFontSize(12);
        curHouseInfo.playerHBox.addWidget(&curHouseInfo.player1Label, 68);

        if(bLoadMultiplayer) {
            if(i < (int) houseInfoListSetup.size()) {
                GameInitSettings::HouseInfo gisHouseInfo = houseInfoListSetup.at(i);

                if(gisHouseInfo.houseID == HOUSE_UNUSED) {
                    curHouseInfo.player1DropDown.addEntry(_("closed"), PLAYER_CLOSED);
                } else if(gisHouseInfo.playerInfoList.empty() == false) {
                    GameInitSettings::PlayerInfo playerInfo = gisHouseInfo.playerInfoList.front();
                    if(playerInfo.playerClass == HUMANPLAYERCLASS) {
                        if(thisPlayerPlaced == false) {
                            curHouseInfo.player1DropDown.addEntry(settings.general.playerName, PLAYER_HUMAN);
                            curHouseInfo.player1DropDown.setSelectedItem(0);
                            thisPlayerPlaced = true;
                        } else {
                            curHouseInfo.player1DropDown.addEntry(_("open"), PLAYER_OPEN);
                            curHouseInfo.player1DropDown.setSelectedItem(0);
                        }
                    } else {
                        const PlayerFactory::PlayerData* pPlayerData = PlayerFactory::getByPlayerClass(playerInfo.playerClass);
                        int index = PlayerFactory::getIndexByPlayerClass(playerInfo.playerClass);

                        if(pPlayerData != nullptr) {
                            curHouseInfo.player1DropDown.addEntry(pPlayerData->getName(), -(index+2));
                            curHouseInfo.player1DropDown.setSelectedItem(0);
                            curHouseInfo.player1DropDown.setEnabled(false);
                            curHouseInfo.player1DropDown.setOnClickEnabled(false);
                        }
                    }
                }

            }

        } else if(bServer && (thisPlayerPlaced == false)) {
            curHouseInfo.player1DropDown.addEntry(settings.general.playerName, PLAYER_HUMAN);
            curHouseInfo.player1DropDown.setSelectedItem(0);
            curHouseInfo.player1DropDown.setEnabled(bServer);
            thisPlayerPlaced = true;
        } else {
            curHouseInfo.player1DropDown.addEntry(_("open"), PLAYER_OPEN);
            curHouseInfo.player1DropDown.setSelectedItem(0);
            curHouseInfo.player1DropDown.addEntry(_("closed"), PLAYER_CLOSED);
            for(unsigned int k = 1; k < PlayerFactory::getList().size(); k++) {
                curHouseInfo.player1DropDown.addEntry(PlayerFactory::getByIndex(k)->getName(), k);
            }
            curHouseInfo.player1DropDown.setEnabled(bServer);
        }
        curHouseInfo.player1DropDown.setOnSelectionChange(std::bind(&CustomGamePlayers::onChangePlayerDropDownBoxes, this, std::placeholders::_1, 2*i));
        curHouseInfo.player1DropDown.setOnClick(std::bind(&CustomGamePlayers::onClickPlayerDropDownBox, this, 2*i));
        curHouseInfo.playerHBox.addWidget(&curHouseInfo.player1DropDown, 100);

        curHouseInfo.playerHBox.addWidget(HSpacer::create(10));

        // add 2. player
        curHouseInfo.player2Label.setText(_("Player") + " 2");
        curHouseInfo.player2Label.setTextFontSize(12);
        curHouseInfo.playerHBox.addWidget(&curHouseInfo.player2Label, 68);

        if(bLoadMultiplayer) {
            if(i < (int) houseInfoListSetup.size()) {
                GameInitSettings::HouseInfo gisHouseInfo = houseInfoListSetup.at(i);

                if(gisHouseInfo.houseID == HOUSE_UNUSED) {
                    curHouseInfo.player2DropDown.addEntry(_("closed"), PLAYER_CLOSED);
                } else if(gisHouseInfo.playerInfoList.size() >= 2) {
                    GameInitSettings::PlayerInfo playerInfo = *(++gisHouseInfo.playerInfoList.begin());
                    if(playerInfo.playerClass == HUMANPLAYERCLASS) {
                        if(thisPlayerPlaced == false) {
                            curHouseInfo.player2DropDown.addEntry(settings.general.playerName, PLAYER_HUMAN);
                            curHouseInfo.player2DropDown.setSelectedItem(0);
                            thisPlayerPlaced = true;
                        } else {
                            curHouseInfo.player2DropDown.addEntry(_("open"), PLAYER_OPEN);
                            curHouseInfo.player2DropDown.setSelectedItem(0);
                        }
                    } else {
                        const PlayerFactory::PlayerData* pPlayerData = PlayerFactory::getByPlayerClass(playerInfo.playerClass);
                        int index = PlayerFactory::getIndexByPlayerClass(playerInfo.playerClass);

                        if(pPlayerData != nullptr) {
                            curHouseInfo.player1DropDown.addEntry(pPlayerData->getName(), -(index+2));
                            curHouseInfo.player1DropDown.setSelectedItem(0);
                            curHouseInfo.player1DropDown.setEnabled(false);
                            curHouseInfo.player1DropDown.setOnClickEnabled(false);
                        }
                    }
                }
            }
        } else if(bServer && (thisPlayerPlaced == false)) {
            curHouseInfo.player2DropDown.addEntry(settings.general.playerName, PLAYER_HUMAN);
            curHouseInfo.player2DropDown.setSelectedItem(0);
            curHouseInfo.player2DropDown.setEnabled(bServer);
            thisPlayerPlaced = true;
        } else {
            curHouseInfo.player2DropDown.addEntry(_("open"), PLAYER_OPEN);
            curHouseInfo.player2DropDown.setSelectedItem(0);
            curHouseInfo.player2DropDown.addEntry(_("closed"), PLAYER_CLOSED);
            for(unsigned int k = 1; k < PlayerFactory::getList().size(); k++) {
                curHouseInfo.player2DropDown.addEntry(PlayerFactory::getByIndex(k)->getName(), k);
            }
            curHouseInfo.player2DropDown.setEnabled(bServer);
        }
        curHouseInfo.player2DropDown.setOnSelectionChange(std::bind(&CustomGamePlayers::onChangePlayerDropDownBoxes, this, std::placeholders::_1, 2*i + 1));
        curHouseInfo.player2DropDown.setOnClick(std::bind(&CustomGamePlayers::onClickPlayerDropDownBox, this, 2*i + 1));
        curHouseInfo.playerHBox.addWidget(&curHouseInfo.player2DropDown, 100);

        curHouseInfo.houseInfoVBox.addWidget(&curHouseInfo.playerHBox);

        playerListVBox.addWidget(&curHouseInfo.houseInfoVBox, 0.01);

        playerListVBox.addWidget(VSpacer::create(4), 0.0);
        playerListVBox.addWidget(Spacer::create(), 0.07);

        if(i >= numHouses) {
            curHouseInfo.houseInfoVBox.setEnabled(false);
            curHouseInfo.houseInfoVBox.setVisible(false);
        }
    }

    onChangeHousesDropDownBoxes(false);

    checkPlayerBoxes();

    leftVBox.addWidget(Spacer::create(), 0.3);

    // maybe there is a better fitting slot if we are loading a map in the old format with Brain=CPU and Brain=Human
    if(brainEqHumanSlot >= 0) {
        DropDownBox& dropDownBox1 = houseInfo[brainEqHumanSlot].player1DropDown;
        DropDownBox& dropDownBox2 = houseInfo[brainEqHumanSlot].player2DropDown;

        if(dropDownBox1.getSelectedEntry() != settings.general.playerName && dropDownBox2.getSelectedEntry() != settings.general.playerName) {
            if(dropDownBox1.getSelectedEntryIntData() == PLAYER_OPEN) {
                setPlayer2Slot(settings.general.playerName, brainEqHumanSlot*2);
            } else if((dropDownBox2.getSelectedEntryIntData() == PLAYER_OPEN) && gameInitSettings.isMultiplePlayersPerHouse()) {
                setPlayer2Slot(settings.general.playerName, brainEqHumanSlot*2+1);
            }
        }
    }

    if(pNetworkManager != nullptr) {
        if(bServer) {
            pNetworkManager->startServer(bLANServer, gameInitSettings.getServername(), settings.general.playerName, &gameInitSettings, 1, gameInitSettings.isMultiplePlayersPerHouse() ? numHouses*2 : numHouses);
        }

        pNetworkManager->setOnPeerDisconnected(std::bind(&CustomGamePlayers::onPeerDisconnected, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        pNetworkManager->setOnReceiveChangeEventList(std::bind(&CustomGamePlayers::onReceiveChangeEventList, this, std::placeholders::_1));
        pNetworkManager->setOnReceiveChatMessage(std::bind(&CustomGamePlayers::onReceiveChatMessage, this, std::placeholders::_1, std::placeholders::_2));

        if(bServer) {
            pNetworkManager->setGetChangeEventListForNewPlayerCallback(std::bind(&CustomGamePlayers::getChangeEventListForNewPlayer, this, std::placeholders::_1));
        } else {
            pNetworkManager->setOnStartGame(std::bind(&CustomGamePlayers::onStartGame, this, std::placeholders::_1));
        }
    }
}

CustomGamePlayers::~CustomGamePlayers()
{
    if(pNetworkManager != nullptr) {
        pNetworkManager->disconnect();

        pNetworkManager->setOnPeerDisconnected(std::function<void (const std::string&, bool, int)>());
        pNetworkManager->setGetChangeEventListForNewPlayerCallback(std::function<ChangeEventList (const std::string&)>());
        pNetworkManager->setOnReceiveChangeEventList(std::function<void (const ChangeEventList&)>());
        pNetworkManager->setOnReceiveChatMessage(std::function<void (const std::string&, const std::string&)>());
        pNetworkManager->setOnStartGame(std::function<void (unsigned int)>());

        if(bServer) {
            try {
                pNetworkManager->stopServer();
            } catch(std::exception& e) {
                SDL_Log("CustomGamePlayers::~CustomGamePlayers(): %s", e.what());
            }
        }
    }
}

void CustomGamePlayers::update() {
    if(startGameTime > 0) {
        if(SDL_GetTicks() >= startGameTime) {
            startGameTime = 0;

            pNetworkManager->setOnPeerDisconnected(std::function<void (const std::string&, bool, int)>());
            pNetworkManager->setGetChangeEventListForNewPlayerCallback(std::function<ChangeEventList (const std::string&)>());
            pNetworkManager->setOnReceiveChangeEventList(std::function<void (const ChangeEventList&)>());
            pNetworkManager->setOnReceiveChatMessage(std::function<void (const std::string&, const std::string&)>());
            pNetworkManager->setOnStartGame(std::function<void (unsigned int)>());

            if(bServer) {
                pNetworkManager->stopServer();
            }

            addAllPlayersToGameInitSettings();
            startMultiPlayerGame(gameInitSettings);

            quit(MENU_QUIT_GAME_FINISHED);
        } else {
            static Uint32 lastSecondLeft = 0;
            Uint32 secondsLeft = ((startGameTime - SDL_GetTicks())/1000) + 1;
            if(lastSecondLeft != secondsLeft) {
                lastSecondLeft = secondsLeft;
                addInfoMessage("Starting game in " + std::to_string(secondsLeft) + "...");
            }
        }
    }
}

void CustomGamePlayers::onReceiveChangeEventList(const ChangeEventList& changeEventList)
{
    for(const ChangeEventList::ChangeEvent& changeEvent : changeEventList.changeEventList) {

        switch(changeEvent.eventType) {
            case ChangeEventList::ChangeEvent::EventType::ChangeHouse: {
                HOUSETYPE houseType = (HOUSETYPE) changeEvent.newValue;

                HouseInfo& curHouseInfo = houseInfo[changeEvent.slot];

                for(int i=0;i<curHouseInfo.houseDropDown.getNumEntries();i++) {
                    if(curHouseInfo.houseDropDown.getEntryIntData(i) == houseType) {
                        curHouseInfo.houseDropDown.setSelectedItem(i);
                        break;
                    }
                }
            } break;

            case ChangeEventList::ChangeEvent::EventType::ChangeTeam: {
                int newTeam = (int) changeEvent.newValue;

                HouseInfo& curHouseInfo = houseInfo[changeEvent.slot];

                for(int i=0;i<curHouseInfo.teamDropDown.getNumEntries();i++) {
                    if(curHouseInfo.teamDropDown.getEntryIntData(i) == newTeam) {
                        curHouseInfo.teamDropDown.setSelectedItem(i);
                        break;
                    }
                }
            } break;

            case ChangeEventList::ChangeEvent::EventType::ChangePlayer: {
                int newPlayer = (int) changeEvent.newValue;

                HouseInfo& curHouseInfo = houseInfo[changeEvent.slot/2];

                if(changeEvent.slot % 2 == 0) {
                    for(int i=0;i<curHouseInfo.player1DropDown.getNumEntries();i++) {
                        if(curHouseInfo.player1DropDown.getEntryIntData(i) == newPlayer) {
                            curHouseInfo.player1DropDown.setSelectedItem(i);
                            break;
                        }
                    }
                } else {
                    for(int i=0;i<curHouseInfo.player2DropDown.getNumEntries();i++) {
                        if(curHouseInfo.player2DropDown.getEntryIntData(i) == newPlayer) {
                            curHouseInfo.player2DropDown.setSelectedItem(i);
                            break;
                        }
                    }
                }

                checkPlayerBoxes();
            } break;

            case ChangeEventList::ChangeEvent::EventType::SetHumanPlayer: {
                const std::string& name = changeEvent.newStringValue;
                int slot = changeEvent.slot;

                setPlayer2Slot(name, slot);

                checkPlayerBoxes();
            } break;

            default: {
            } break;
        }
    }

    if((pNetworkManager != nullptr) && bServer) {
        ChangeEventList changeEventList2 = getChangeEventList();

        pNetworkManager->sendChangeEventList(changeEventList2);
    }
}

ChangeEventList CustomGamePlayers::getChangeEventList()
{
    ChangeEventList changeEventList;

    for(int i=0;i<NUM_HOUSES;i++) {
        HouseInfo& curHouseInfo = houseInfo[i];

        int houseID = curHouseInfo.houseDropDown.getSelectedEntryIntData();
        int team = curHouseInfo.teamDropDown.getSelectedEntryIntData();
        int player1 = curHouseInfo.player1DropDown.getSelectedEntryIntData();
        int player2 = curHouseInfo.player2DropDown.getSelectedEntryIntData();

        changeEventList.changeEventList.push_back(ChangeEventList::ChangeEvent(ChangeEventList::ChangeEvent::EventType::ChangeHouse, i, houseID));
        changeEventList.changeEventList.push_back(ChangeEventList::ChangeEvent(ChangeEventList::ChangeEvent::EventType::ChangeTeam, i, team));

        if(player1 == PLAYER_HUMAN) {
            std::string playername = curHouseInfo.player1DropDown.getSelectedEntry();
            changeEventList.changeEventList.push_back(ChangeEventList::ChangeEvent(2*i, playername));
        } else {
            changeEventList.changeEventList.push_back(ChangeEventList::ChangeEvent(ChangeEventList::ChangeEvent::EventType::ChangePlayer, 2*i, player1));
        }

        if(player2 == PLAYER_HUMAN) {
            std::string playername = curHouseInfo.player2DropDown.getSelectedEntry();
            changeEventList.changeEventList.emplace_back(2*i+1, playername);
        } else {
            changeEventList.changeEventList.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangePlayer, 2*i+1, player2);
        }
    }

    return changeEventList;
}

ChangeEventList CustomGamePlayers::getChangeEventListForNewPlayer(const std::string& newPlayerName)
{
    ChangeEventList changeEventList = getChangeEventList();

    // find a place for the new player

    int newPlayerSlot = INVALID;

    // look for an open left slot
    for(int i=0;i<numHouses;i++) {
        HouseInfo& curHouseInfo = houseInfo[i];
        int player1 = curHouseInfo.player1DropDown.getSelectedEntryIntData();

        if(player1 == PLAYER_OPEN) {
            newPlayerSlot = 2*i;
            break;
        }
    }

    // if multiple players per house look for an open right slot
    if((newPlayerSlot == INVALID) && gameInitSettings.isMultiplePlayersPerHouse()) {
        for(int i=0;i<numHouses;i++) {
            HouseInfo& curHouseInfo = houseInfo[i];
            int player2 = curHouseInfo.player2DropDown.getSelectedEntryIntData();

            if(player2 == PLAYER_OPEN) {
                newPlayerSlot = 2*i + 1;
                break;
            }
        }
    }

    // as a fallback look for any non-human slot
    if(newPlayerSlot == INVALID) {
        for(int i=0;i<numHouses;i++) {
            HouseInfo& curHouseInfo = houseInfo[i];
            int player1 = curHouseInfo.player1DropDown.getSelectedEntryIntData();
            int player2 = curHouseInfo.player2DropDown.getSelectedEntryIntData();

            if(player1 != PLAYER_HUMAN) {
                newPlayerSlot = 2*i;
                break;
            } else if(gameInitSettings.isMultiplePlayersPerHouse() && (player2 != PLAYER_HUMAN)) {
                newPlayerSlot = 2*i + 1;
                break;
            }
        }
    }

    if(newPlayerSlot != INVALID) {
        setPlayer2Slot(newPlayerName, newPlayerSlot);

        ChangeEventList::ChangeEvent changeEvent(newPlayerSlot, newPlayerName);

        // add changeEvent to changeEventList for the new player
        changeEventList.changeEventList.push_back(changeEvent);

        // send it to all other connected peers
        ChangeEventList changeEventList2;
        changeEventList2.changeEventList.push_back(changeEvent);

        pNetworkManager->sendChangeEventList(changeEventList2);
    }

    return changeEventList;
}

void CustomGamePlayers::onReceiveChatMessage(const std::string& name, const std::string& message) {
    addChatMessage(name, message);
}

void CustomGamePlayers::onNext()
{
    // check if we have at least two houses on the map and if we have more than one team
    int numUsedHouses = 0;
    int numTeams = 0;
    for(int i=0;i<NUM_HOUSES;i++) {
        HouseInfo& curHouseInfo = houseInfo[i];

        int currentPlayer1 = curHouseInfo.player1DropDown.getSelectedEntryIntData();
        int currentPlayer2 = curHouseInfo.player2DropDown.getSelectedEntryIntData();

        if((currentPlayer1 != PLAYER_OPEN && currentPlayer1 != PLAYER_CLOSED) || (currentPlayer2 != PLAYER_OPEN && currentPlayer2 != PLAYER_CLOSED)) {
            numUsedHouses++;

            int currentTeam = curHouseInfo.teamDropDown.getSelectedEntryIntData();
            bool bTeamFound = false;
            for(int j=0;j<i;j++) {
                int player1 = houseInfo[j].player1DropDown.getSelectedEntryIntData();
                int player2 = houseInfo[j].player2DropDown.getSelectedEntryIntData();

                if((player1 != PLAYER_OPEN && player1 != PLAYER_CLOSED) || (player2 != PLAYER_OPEN && player2 != PLAYER_CLOSED)) {

                    int team = houseInfo[j].teamDropDown.getSelectedEntryIntData();
                    if(currentTeam == team) {
                        bTeamFound = true;
                    }
                }
            }

            if(bTeamFound == false) {
                numTeams++;
            }
        }
    }

    if(numUsedHouses < 2) {
        // No game possible with only 1 house
        openWindow(MsgBox::create(_("At least 2 houses must be controlled\nby a human player or an AI player!")));
    } else if(numTeams < 2) {
        // No game possible with only 1 team
        openWindow(MsgBox::create(_("There must be at least two different teams!")));
    } else {
        // start game

        addAllPlayersToGameInitSettings();

        if(pNetworkManager != nullptr) {
            unsigned int timeLeft = 5000;
            startGameTime = SDL_GetTicks() + timeLeft;
            pNetworkManager->sendStartGame(timeLeft);

            disableAllDropDownBoxes();
        } else {
            startSinglePlayerGame(gameInitSettings);

            quit(MENU_QUIT_GAME_FINISHED);
        }
    }
}

void CustomGamePlayers::addAllPlayersToGameInitSettings()
{
    gameInitSettings.clearHouseInfo();

    for(int i=0;i<NUM_HOUSES;i++) {
        HouseInfo& curHouseInfo = houseInfo[i];

        int houseID = curHouseInfo.houseDropDown.getSelectedEntryIntData();
        int team = curHouseInfo.teamDropDown.getSelectedEntryIntData();
        int player1 = curHouseInfo.player1DropDown.getSelectedEntryIntData();
        std::string player1name = curHouseInfo.player1DropDown.getSelectedEntry();
        int player2 = curHouseInfo.player2DropDown.getSelectedEntryIntData();
        std::string player2name = curHouseInfo.player2DropDown.getSelectedEntry();

        GameInitSettings::HouseInfo newHouseInfo((HOUSETYPE) houseID, team);

        bool bAdded = false;
        bAdded |= addPlayerToHouseInfo(newHouseInfo, player1, player1name);
        bAdded |= addPlayerToHouseInfo(newHouseInfo, player2, player2name);

        if(bAdded == true) {
            gameInitSettings.addHouseInfo(newHouseInfo);
        }
    }
}

bool CustomGamePlayers::addPlayerToHouseInfo(GameInitSettings::HouseInfo& newHouseInfo, int player, const std::string& playername)
{
    std::string playerName;
    std::string playerClass;

    HOUSETYPE houseID = newHouseInfo.houseID;

    switch(player) {
        case PLAYER_HUMAN: {
            playerName = playername;
            playerClass = HUMANPLAYERCLASS;
        } break;

        case PLAYER_OPEN:
        case PLAYER_CLOSED: {
            return false;
        } break;


        default: {
            // AI player
            const PlayerFactory::PlayerData* pPlayerData = PlayerFactory::getByIndex(player);
            if(pPlayerData == nullptr) {
                return false;
            }

            playerName = (houseID == HOUSE_INVALID) ? pPlayerData->getName() : getHouseNameByNumber(houseID);
            playerClass = pPlayerData->getPlayerClass();

        } break;
    }

    newHouseInfo.addPlayerInfo(GameInitSettings::PlayerInfo(playerName, playerClass));
    return true;
}

void CustomGamePlayers::onCancel()
{
    quit();
}

void CustomGamePlayers::onSendChatMessage()
{
    std::string message = chatTextBox.getText();
    addChatMessage(settings.general.playerName, message);
    chatTextBox.setText("");

    pNetworkManager->sendChatMessage(message);

}

void CustomGamePlayers::addInfoMessage(const std::string& message) {
    std::string text = chatTextView.getText();
    if(text.length() > 0) {
        text += "\n";
    }
    text += "* " + message;
    chatTextView.setText(text);
    chatTextView.scrollToEnd();
}

void CustomGamePlayers::addChatMessage(const std::string& name, const std::string& message)
{
    std::string text = chatTextView.getText();
    if(text.length() > 0) {
        text += "\n";
    }
    text += name + ": " + message;
    chatTextView.setText(text);
    chatTextView.scrollToEnd();
}

void CustomGamePlayers::extractMapInfo(INIFile* pMap)
{
    int sizeX = 0;
    int sizeY = 0;

    if(pMap->hasKey("MAP","Seed")) {
        // old map format with seed value
        int mapscale = pMap->getIntValue("BASIC", "MapScale", -1);

        switch(mapscale) {
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
        sizeX = pMap->getIntValue("MAP","SizeX", 0);
        sizeY = pMap->getIntValue("MAP","SizeY", 0);
    }

    mapPropertySize.setText(std::to_string(sizeX) + " x " + std::to_string(sizeY));

    sdl2::surface_ptr pMapSurface = nullptr;
    try {
        INIMapPreviewCreator mapPreviewCreator(pMap);
        pMapSurface = mapPreviewCreator.createMinimapImageOfMap(1, DuneStyle::buttonBorderColor);
    } catch(...) {
        pMapSurface = sdl2::surface_ptr{ GUIStyle::getInstance().createButtonSurface(130, 130, "Error", true, false) };
        nextButton.setEnabled(false);
    }
    minimap.setSurface(std::move(pMapSurface));


    boundHousesOnMap.clear();
    if(pMap->hasSection("Harkonnen")) boundHousesOnMap.push_back(HOUSE_HARKONNEN);
    if(pMap->hasSection("Atreides"))  boundHousesOnMap.push_back(HOUSE_ATREIDES);
    if(pMap->hasSection("Ordos"))     boundHousesOnMap.push_back(HOUSE_ORDOS);
    if(pMap->hasSection("Fremen"))    boundHousesOnMap.push_back(HOUSE_FREMEN);
    if(pMap->hasSection("Sardaukar")) boundHousesOnMap.push_back(HOUSE_SARDAUKAR);
    if(pMap->hasSection("Mercenary")) boundHousesOnMap.push_back(HOUSE_MERCENARY);

    numHouses = boundHousesOnMap.size();
    if(pMap->hasSection("Player1"))   numHouses++;
    if(pMap->hasSection("Player2"))   numHouses++;
    if(pMap->hasSection("Player3"))   numHouses++;
    if(pMap->hasSection("Player4"))   numHouses++;
    if(pMap->hasSection("Player5"))   numHouses++;
    if(pMap->hasSection("Player6"))   numHouses++;

    mapPropertyPlayers.setText(std::to_string(numHouses));

    std::string authors = pMap->getStringValue("BASIC","Author", "-");
    if(authors.size() > 11) {
        authors = authors.substr(0,9) + "...";
    }
    mapPropertyAuthors.setText(authors);


    mapPropertyLicense.setText(pMap->getStringValue("BASIC","License", "-"));

    // find Brain=Human
    int currentIndex = 0;
    for(const HOUSETYPE& houseType : boundHousesOnMap) {
        if(strToUpper(pMap->getStringValue(getHouseNameByNumber(houseType),"Brain","")) == "HUMAN") {
            brainEqHumanSlot = currentIndex;
        }
        currentIndex++;
    }

    currentIndex = 0;
    int currentTeam = 0;
    std::vector<std::string> teamNames;
    for(const HOUSETYPE& houseType : boundHousesOnMap) {
        std::string teamName = strToUpper(pMap->getStringValue(getHouseNameByNumber(houseType),"Brain","Team " + std::to_string(currentIndex+1)));
        teamNames.push_back(teamName);
        slotToTeam[currentIndex] = currentTeam;
        currentTeam++;

        // let's see if we can reuse an old team number
        for(unsigned int i = 0; i < teamNames.size()-1; i++) {
            if(teamNames[i] == teamName) {
                slotToTeam[currentIndex] = slotToTeam[i];
                currentTeam--;
                break;
            }
        }

        currentIndex++;
    }

    for(int p = 0; (p < NUM_HOUSES) && (currentIndex < NUM_HOUSES); p++) {
        if(pMap->hasSection("Player" + std::to_string(p+1))) {
            std::string teamName = strToUpper(pMap->getStringValue("Player" + std::to_string(p+1),"Brain","Team " + std::to_string(currentIndex+p+1)));
            teamNames.push_back(teamName);
            slotToTeam[currentIndex] = currentTeam;
            currentTeam++;

            // let's see if we can reuse an old team number
            for(unsigned int i = 0; i < teamNames.size()-1; i++) {
                if(teamNames[i] == teamName) {
                    slotToTeam[currentIndex] = slotToTeam[i];
                    currentTeam--;
                    break;
                }
            }

            currentIndex++;
        }
    }

    for(;currentIndex < NUM_HOUSES; currentIndex++) {
        slotToTeam[currentIndex] = -1;
    }
}

void CustomGamePlayers::onChangeHousesDropDownBoxes(bool bInteractive, int houseInfoNum) {
    if(bInteractive && houseInfoNum >= 0 && pNetworkManager != nullptr) {
        int selectedHouseID = houseInfo[houseInfoNum].houseDropDown.getSelectedEntryIntData();

        ChangeEventList changeEventList;
        changeEventList.changeEventList.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangeHouse, houseInfoNum, selectedHouseID);

        pNetworkManager->sendChangeEventList(changeEventList);
    }

    int numBoundHouses = boundHousesOnMap.size();
    int numUsedBoundHouses = 0;
    int numUsedRandomHouses = 0;
    for(int i=0;i<numHouses;i++) {
        int selectedHouseID = houseInfo[i].houseDropDown.getSelectedEntryIntData();

        if(selectedHouseID == HOUSE_INVALID) {
            numUsedRandomHouses++;
        } else {
            if(isBoundedHouseOnMap((HOUSETYPE) selectedHouseID)) {
                numUsedBoundHouses++;
            }
        }
    }


    for(int i=0;i<numHouses;i++) {
        HouseInfo& curHouseInfo = houseInfo[i];

        int house = curHouseInfo.houseDropDown.getSelectedEntryIntData();

        if(houseInfoNum == -1 || houseInfoNum == i) {

            Uint32 color = (house == HOUSE_INVALID) ? COLOR_RGB(20,20,40) : SDL2RGB(palette[houseToPaletteIndex[house] + 3]);
            curHouseInfo.houseLabel.setTextColor(color);
            curHouseInfo.houseDropDown.setColor(color);
            curHouseInfo.teamDropDown.setColor(color);
            curHouseInfo.player1Label.setTextColor(color);
            curHouseInfo.player1DropDown.setColor(color);
            curHouseInfo.player2Label.setTextColor(color);
            curHouseInfo.player2DropDown.setColor(color);

            if(house == HOUSE_INVALID) {
                curHouseInfo.player1ArrowLabel.setTexture(pGFXManager->getUIGraphic(UI_CustomGamePlayersArrowNeutral));
            } else {
                curHouseInfo.player1ArrowLabel.setTexture(pGFXManager->getUIGraphic(UI_CustomGamePlayersArrow, house));
            }
        }

        if(gameInitSettings.getGameType() == GameType::LoadMultiplayer) {
            // no house changes possible
            continue;
        }

        addToHouseDropDown(curHouseInfo.houseDropDown, HOUSE_INVALID);

        for(int h=0;h<NUM_HOUSES;h++) {
            bool bAddHouse;

            bool bCheck;

            if((house == HOUSE_INVALID) || (isBoundedHouseOnMap((HOUSETYPE) house))) {
                if(numUsedBoundHouses + numUsedRandomHouses - 1 >= numBoundHouses) {
                    bCheck = true;
                } else {
                    bCheck = false;
                }
            } else {
                if(numUsedBoundHouses + numUsedRandomHouses >= numBoundHouses) {
                    bCheck = true;
                } else {
                    bCheck = false;
                }
            }

            if(bCheck == true) {
                bAddHouse = true;
                for(int j = 0; j < numHouses; j++) {
                    if(i != j) {
                        DropDownBox& tmpDropDown = houseInfo[j].houseDropDown;
                        if(tmpDropDown.getSelectedEntryIntData() == h) {
                            bAddHouse = false;
                            break;
                        }
                    }
                }
            } else {
                bAddHouse = (h == house);

                if(((house == HOUSE_INVALID) || (isBoundedHouseOnMap((HOUSETYPE) house))) && isBoundedHouseOnMap((HOUSETYPE) h)) {
                    // check if this entry is random or a bounded house but is needed for a bounded house
                    bAddHouse = true;
                    for(int j = 0; j < numHouses; j++) {
                        if(i != j) {
                            DropDownBox& tmpDropDown = houseInfo[j].houseDropDown;
                            if(tmpDropDown.getSelectedEntryIntData() == h) {
                                bAddHouse = false;
                                break;
                            }
                        }
                    }
                }
            }

            if(bAddHouse == true) {
                addToHouseDropDown(curHouseInfo.houseDropDown, h);
            } else {
                removeFromHouseDropDown(curHouseInfo.houseDropDown, h);
            }
        }
    }
}

void CustomGamePlayers::onChangeTeamDropDownBoxes(bool bInteractive, int houseInfoNum) {

    if(bInteractive && houseInfoNum >= 0 && pNetworkManager != nullptr) {
        int selectedTeam = houseInfo[houseInfoNum].teamDropDown.getSelectedEntryIntData();

        ChangeEventList changeEventList;
        changeEventList.changeEventList.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangeTeam, houseInfoNum, selectedTeam);

        pNetworkManager->sendChangeEventList(changeEventList);
    }
}

void CustomGamePlayers::onChangePlayerDropDownBoxes(bool bInteractive, int boxnum) {
    if(bInteractive && boxnum >= 0 && pNetworkManager != nullptr) {
        DropDownBox& dropDownBox = (boxnum % 2 == 0) ? houseInfo[boxnum / 2].player1DropDown : houseInfo[boxnum / 2].player2DropDown;

        int selectedPlayer = dropDownBox.getSelectedEntryIntData();

        ChangeEventList changeEventList;
        changeEventList.changeEventList.emplace_back(ChangeEventList::ChangeEvent::EventType::ChangePlayer, boxnum, selectedPlayer);

        pNetworkManager->sendChangeEventList(changeEventList);
    }

    checkPlayerBoxes();
}

void CustomGamePlayers::onClickPlayerDropDownBox(int boxnum) {
    DropDownBox& dropDownBox = (boxnum % 2 == 0) ? houseInfo[boxnum / 2].player1DropDown : houseInfo[boxnum / 2].player2DropDown;

    if(dropDownBox.getSelectedEntryIntData() == PLAYER_CLOSED) {
        return;
    }

    setPlayer2Slot(settings.general.playerName, boxnum);

    if(boxnum >= 0 && pNetworkManager != nullptr) {
        ChangeEventList changeEventList;
        changeEventList.changeEventList.emplace_back(boxnum, settings.general.playerName);

        pNetworkManager->sendChangeEventList(changeEventList);
    }
}

void CustomGamePlayers::onPeerDisconnected(const std::string& playername, bool bHost, int cause) {
    if(bHost) {
        quit(cause);
    } else {
        for(int i=0;i<numHouses*2;i++) {
            DropDownBox& curDropDownBox = (i % 2 == 0) ? houseInfo[i / 2].player1DropDown : houseInfo[i / 2].player2DropDown;

            if(curDropDownBox.getSelectedEntryIntData() == PLAYER_HUMAN && curDropDownBox.getSelectedEntry() == playername) {

                curDropDownBox.clearAllEntries();
                curDropDownBox.addEntry(_("open"), PLAYER_OPEN);
                curDropDownBox.setSelectedItem(0);

                if(gameInitSettings.getGameType() != GameType::LoadMultiplayer) {
                    curDropDownBox.addEntry(_("closed"), PLAYER_CLOSED);
                    for(unsigned int k = 1; k < PlayerFactory::getList().size(); k++) {
                        curDropDownBox.addEntry(PlayerFactory::getByIndex(k)->getName(), k);
                    }
                }

                break;
            }
        }

        if(bServer == false) {
            for(int i=0;i<numHouses;i++) {
                bool bIsThisPlayer = false;
                if(houseInfo[i].player1DropDown.getSelectedEntryIntData() == PLAYER_HUMAN) {
                    if(houseInfo[i].player1DropDown.getSelectedEntry() == settings.general.playerName) {
                        bIsThisPlayer = true;
                    }
                }

                if(houseInfo[i].player2DropDown.getSelectedEntryIntData() == PLAYER_HUMAN) {
                    if(houseInfo[i].player2DropDown.getSelectedEntry() == settings.general.playerName) {
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
}

void CustomGamePlayers::onStartGame(unsigned int timeLeft) {
    startGameTime = SDL_GetTicks() + timeLeft;
    disableAllDropDownBoxes();
}

void CustomGamePlayers::setPlayer2Slot(const std::string& playername, int slot) {
    DropDownBox& dropDownBox = (slot % 2 == 0) ? houseInfo[slot / 2].player1DropDown : houseInfo[slot / 2].player2DropDown;

    std::string oldPlayerName = "";
    int oldPlayerType = dropDownBox.getSelectedEntryIntData();
    if(oldPlayerType == PLAYER_HUMAN) {
        oldPlayerName = dropDownBox.getSelectedEntry();
    }

    for(int i=0;i<numHouses*2;i++) {
        DropDownBox& curDropDownBox = (i % 2 == 0) ? houseInfo[i / 2].player1DropDown : houseInfo[i / 2].player2DropDown;

        if(curDropDownBox.getSelectedEntryIntData() == PLAYER_HUMAN && curDropDownBox.getSelectedEntry() == playername) {

            if(oldPlayerType == PLAYER_HUMAN) {
                curDropDownBox.clearAllEntries();
                curDropDownBox.addEntry(oldPlayerName, PLAYER_HUMAN);
                curDropDownBox.setSelectedItem(0);
            } else {
                curDropDownBox.clearAllEntries();
                curDropDownBox.addEntry(_("open"), PLAYER_OPEN);

                if(gameInitSettings.getGameType() != GameType::LoadMultiplayer) {
                    curDropDownBox.addEntry(_("closed"), PLAYER_CLOSED);
                    for(unsigned int k = 1; k < PlayerFactory::getList().size(); k++) {
                        curDropDownBox.addEntry(PlayerFactory::getByIndex(k)->getName(), k);
                    }
                }

                for(int j = 0; j < curDropDownBox.getNumEntries(); j++) {
                    if(curDropDownBox.getEntryIntData(j) == oldPlayerType) {
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

    if(bServer == false) {
        for(int i=0;i<numHouses;i++) {
            bool bIsThisPlayer = false;

            if(gameInitSettings.getGameType() != GameType::LoadMultiplayer) {
                if(houseInfo[i].player1DropDown.getSelectedEntryIntData() == PLAYER_HUMAN) {
                    if(houseInfo[i].player1DropDown.getSelectedEntry() == settings.general.playerName) {
                        bIsThisPlayer = true;
                    }
                }

                if(houseInfo[i].player2DropDown.getSelectedEntryIntData() == PLAYER_HUMAN) {
                    if(houseInfo[i].player2DropDown.getSelectedEntry() == settings.general.playerName) {
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

    for(int i=0;i<numHouses;i++) {
        HouseInfo& curHouseInfo = houseInfo[i];

        int player1 = curHouseInfo.player1DropDown.getSelectedEntryIntData();
        int player2 = curHouseInfo.player2DropDown.getSelectedEntryIntData();

        if(player1 != PLAYER_OPEN) {
            numPlayers++;
        }

        if(gameInitSettings.isMultiplePlayersPerHouse() && player2 != PLAYER_OPEN) {
            numPlayers++;
        }

        if((gameInitSettings.isMultiplePlayersPerHouse() == false) || (player1 == PLAYER_OPEN && player2 == PLAYER_OPEN) || (curHouseInfo.player2DropDown.getNumEntries() == 0)) {
            curHouseInfo.player2DropDown.setVisible(false);
            curHouseInfo.player2DropDown.setEnabled(false);
            curHouseInfo.player2Label.setVisible(false);
        } else {
            curHouseInfo.player2DropDown.setVisible(true);

            bool bEnableDropDown2 = bServer;
            if(bServer == false) {
                if(player1 == PLAYER_HUMAN) {
                    if(curHouseInfo.player1DropDown.getSelectedEntry() == settings.general.playerName) {
                        bEnableDropDown2 = true;
                    }
                }

                if(player2 == PLAYER_HUMAN) {
                    if(curHouseInfo.player2DropDown.getSelectedEntry() == settings.general.playerName) {
                        bEnableDropDown2 = true;
                    }
                }
            }

            if((gameInitSettings.getGameType() == GameType::LoadMultiplayer) && (player2 != PLAYER_OPEN) && (player2 != PLAYER_HUMAN)) {
                curHouseInfo.player2DropDown.setEnabled(false);
            } else {
                curHouseInfo.player2DropDown.setEnabled(bEnableDropDown2);
            }
            curHouseInfo.player2Label.setVisible(true);
        }
    }

    if(pNetworkManager != nullptr) {
        if(bServer) {
            pNetworkManager->updateServer(numPlayers);
        }
    }
}


void CustomGamePlayers::addToHouseDropDown(DropDownBox& houseDropDownBox, int house, bool bSelect) {

    if(houseDropDownBox.getNumEntries() == 0) {
        if(house == HOUSE_INVALID) {
            houseDropDownBox.addEntry(_("Random"), HOUSE_INVALID);
        } else {
            houseDropDownBox.addEntry(getHouseNameByNumber((HOUSETYPE) house), house);
        }

        if(bSelect) {
            houseDropDownBox.setSelectedItem(0);
        }
    } else {

        if(house == HOUSE_INVALID) {
            if(houseDropDownBox.getEntryIntData(0) != HOUSE_INVALID) {
                houseDropDownBox.insertEntry(0, _("Random"), HOUSE_INVALID);
            }

            if(bSelect) {
                houseDropDownBox.setSelectedItem(houseDropDownBox.getNumEntries()-1);
            }
        } else {

            int currentItemIndex = (houseDropDownBox.getEntryIntData(0) == HOUSE_INVALID) ? 1 : 0;

            for(int h = 0; h < NUM_HOUSES; h++) {
                if(currentItemIndex < houseDropDownBox.getNumEntries() && houseDropDownBox.getEntryIntData(currentItemIndex) == h) {
                    if(h == house) {
                        if(bSelect) {
                            houseDropDownBox.setSelectedItem(currentItemIndex);
                        }
                        break;
                    }

                    currentItemIndex++;
                } else {
                    if(h == house) {
                        houseDropDownBox.insertEntry(currentItemIndex, getHouseNameByNumber((HOUSETYPE) h), h);

                        if(bSelect) {
                            houseDropDownBox.setSelectedItem(currentItemIndex);
                        }
                        break;
                    }
                }


            }
        }
    }
}

void CustomGamePlayers::removeFromHouseDropDown(DropDownBox& houseDropDownBox, int house) {

    for(int i=0;i<houseDropDownBox.getNumEntries(); i++) {
        if(houseDropDownBox.getEntryIntData(i) == house) {
            houseDropDownBox.removeEntry(i);
            break;
        }
    }
}

bool CustomGamePlayers::isBoundedHouseOnMap(HOUSETYPE houseID) {
    return (std::find(boundHousesOnMap.begin(), boundHousesOnMap.end(), houseID) != boundHousesOnMap.end());
}

void CustomGamePlayers::disableAllDropDownBoxes() {
    for(int i=0;i<NUM_HOUSES;i++) {
        HouseInfo& curHouseInfo = houseInfo[i];

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
