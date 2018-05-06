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

#ifndef CUSTOMGAMEPLAYERS_H
#define CUSTOMGAMEPLAYERS_H

#include <GameInitSettings.h>

#include <GUI/StaticContainer.h>
#include <GUI/VBox.h>
#include <GUI/HBox.h>
#include <GUI/Label.h>
#include <GUI/DropDownBox.h>
#include <GUI/TextButton.h>
#include <GUI/ListBox.h>
#include <GUI/PictureLabel.h>
#include <GUI/Checkbox.h>
#include <GUI/TextView.h>
#include <GUI/TextBox.h>

#include <Network/ChangeEventList.h>

#include <DataTypes.h>

#include <string>
#include <list>

#include "MenuBase.h"

#define MENU_QUIT_GAME_FINISHED     (-2)

class INIFile;

class CustomGamePlayers : public MenuBase
{
public:
    CustomGamePlayers(const GameInitSettings& newGameInitSettings, bool server = true, bool LANServer = true);
    virtual ~CustomGamePlayers();

    void onReceiveChangeEventList(const ChangeEventList& changeEventList);

    ChangeEventList getChangeEventListForNewPlayer(const std::string& newPlayerName);

    void update() override;

private:
    ChangeEventList getChangeEventList();

    void onReceiveChatMessage(const std::string& name, const std::string& message);
    void onPeerDisconnected(const std::string& playername, bool bHost, int cause);

    void extractMapInfo(INIFile* pMap);

    void setPlayer2Slot(const std::string& playername, int slot);

    void onChangeHousesDropDownBoxes(bool bInteractive, int houseInfoNum = -1);
    void onChangeTeamDropDownBoxes(bool bInteractive, int houseInfoNum = -1);
    void onChangePlayerDropDownBoxes(bool bInteractive, int boxnum);
    void onClickPlayerDropDownBox(int boxnum);
    void onStartGame(unsigned int timeLeft);
    void addToHouseDropDown(DropDownBox& houseDropDownBox, int house, bool bSelect = false);
    void removeFromHouseDropDown(DropDownBox& houseDropDownBox, int house);

    bool isBoundedHouseOnMap(HOUSETYPE houseID);

    void checkPlayerBoxes();
    void addAllPlayersToGameInitSettings();
    bool addPlayerToHouseInfo(GameInitSettings::HouseInfo& newHouseInfo, int player, const std::string& playername);

    void onNext();
    void onCancel();

    void onSendChatMessage();
    void addInfoMessage(const std::string& message);
    void addChatMessage(const std::string& name, const std::string& message);

    void disableAllDropDownBoxes();

    GameInitSettings                gameInitSettings;
    GameInitSettings::HouseInfoList houseInfoListSetup;     ///< only used if we are loading a savegame

    StaticContainer windowWidget;
    VBox            mainVBox;

    Label           captionLabel;

    HBox            mainHBox;

    // left VBox with player list and map options
    VBox            leftVBox;
    HBox            playerListHBox;
    VBox            playerListVBox;

    // right VBox with mini map
    VBox            rightVBox;
    PictureLabel    minimap;
    HBox            mapPropertiesHBox;
    VBox            mapPropertyNamesVBox;
    VBox            mapPropertyValuesVBox;
    Label           mapPropertySize;
    Label           mapPropertyPlayers;
    Label           mapPropertyAuthors;
    Label           mapPropertyLicense;

    // bottom row of buttons
    HBox            buttonHBox;
    VBox            backButtonVBox;
    TextButton      backButton;
    VBox            chatVBox;
    TextView        chatTextView;
    TextBox         chatTextBox;
    VBox            nextButtonVBox;
    TextButton      nextButton;

    class HouseInfo {
    public:
        VBox            houseInfoVBox;
        HBox            houseHBox;
        Label           houseLabel;
        DropDownBox     houseDropDown;
        DropDownBox     teamDropDown;
        HBox            playerHBox;
        PictureLabel    player1ArrowLabel;
        Label           player1Label;
        DropDownBox     player1DropDown;
        Label           player2Label;
        DropDownBox     player2DropDown;
    };

    bool                    bServer;
    bool                    bLANServer;
    HouseInfo               houseInfo[NUM_HOUSES];
    int                     numHouses;
    std::list<HOUSETYPE>    boundHousesOnMap;
    Uint32                  startGameTime;
    int                     brainEqHumanSlot;           ///< If we have an old map with Brain=Human and Brain=CPU, store index of Brain=Human here
    int                     slotToTeam[NUM_HOUSES];     ///< Maps the slot number to a team number (both zero-based indices)
};

#endif //CUSTOMGAMEPLAYERS_H
