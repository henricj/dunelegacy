#ifndef MULTIPLAYERMENU_H
#define MULTIPLAYERMENU_H

#include "MenuBase.h"

#include <GUI/Window.h>
#include <GUI/StaticContainer.h>
#include <GUI/HBox.h>
#include <GUI/TextButton.h>
#include <GUI/Spacer.h>
#include <GUI/Label.h>
#include <GUI/TextBox.h>
#include <GUI/SymbolButton.h>
#include <GUI/ScrollBar.h>
#include <GUI/ListBox.h>
#include <GUI/ProgressBar.h>
#include <GUI/PictureLabel.h>
#include <GUI/InvisibleButton.h>
#include <GUI/ClickMap.h>
#include <GUI/VBox.h>

#include <Network/LANGameFinderAndAnnouncer.h>
#include <Network/ChangeEventList.h>

#include <GameInitSettings.h>

#include <list>

class MultiPlayerMenu : public MenuBase {
public:
    MultiPlayerMenu();
    ~MultiPlayerMenu();

    /**
        This method is called, when the child window is about to be closed.
        This child window will be closed after this method returns.
        \param  pChildWindow    The child window that will be closed
    */
    void onChildWindowClose(Window* pChildWindow) override;

private:
    void showDisconnectMessageBox(int cause);

    void onCreateLANGame();
    void onCreateInternetGame();
    void onConnect();
    void onJoin();
    void onQuit();

    void onPeerDisconnected(const std::string& playername, bool bHost, int cause);

    void onGameTypeChange(int buttonID);
    void onGameListSelectionChange(bool bInteractive);

    void onNewLANServer(GameServerInfo gameServerInfo);
    void onUpdateLANServer(GameServerInfo gameServerInfo);
    void onRemoveLANServer(GameServerInfo gameServerInfo);

    void onGameServerInfoList(const std::list<GameServerInfo>& gameServerInfoList);
    void onMetaServerError(int errorcause, const std::string& errorMessage);


    void onReceiveGameInfo(const GameInitSettings& gameInitSettings, const ChangeEventList& changeEventList);

    std::list<GameServerInfo> LANGameList;
    std::list<GameServerInfo> InternetGameList;

    StaticContainer windowWidget;

    VBox            mainVBox;
    HBox            mainHBox;

    Label           captionLabel;

    HBox            connectHBox;
    TextBox         connectHostTextBox;
    TextBox         connectPortTextBox;
    TextButton      connectButton;

    // left VBox with create game buttons
    VBox            leftVBox;
    TextButton      createLANGameButton;
    TextButton      createInternetGameButton;

    // right VBox with game list
    VBox            rightVBox;
    HBox            gameTypeButtonsHBox;
    TextButton      LANGamesButton;
    TextButton      internetGamesButton;
    ListBox         gameList;

    // bottom row of buttons
    HBox            buttonHBox;
    TextButton      joinButton;
    TextButton      backButton;
};

#endif // MULTIPLAYERMENU_H

