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

#include <Menu/OptionsMenu.h>

#include <globals.h>

#include <GUI/Label.h>
#include <GUI/Spacer.h>
#include <GUI/dune/GameOptionsWindow.h>
#include <GUI/MsgBox.h>

#include <main.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/INIFile.h>
#include <FileClasses/music/MusicPlayer.h>

#include <SoundPlayer.h>

#include <players/PlayerFactory.h>

#include <misc/Scaler.h>
#include <misc/string_util.h>
#include <misc/FileSystem.h>

#include <config.h>

#include <string.h>
#include <algorithm>

OptionsMenu::OptionsMenu() : MenuBase()
{
    determineAvailableScreenResolutions();

    std::list<std::string> languagesList = getFileNamesList(getDuneLegacyDataDir() + "/locale", "po", true, FileListOrder_Name_Asc);
    availLanguages = std::vector<std::string>(languagesList.begin(), languagesList.end());

    currentGameOptions = settings.gameOptions;

    // set up window
    SDL_Texture *pBackground = pGFXManager->getUIGraphic(UI_MenuBackground);
    setBackground(pBackground, false);
    resize(getTextureSize(pBackground));

    setWindowWidget(&windowWidget);

    windowWidget.addWidget(&mainVBox, Point(50,50), Point(getSize().x - 100,getSize().y - 100));

    mainVBox.addWidget(Spacer::create(), 0.2);

    NameHBox.addWidget(Spacer::create(), 0.5);
    NameHBox.addWidget(Label::create(_("Player Name")), 190);
    nameTextBox.setMaximumTextLength(MAX_PLAYERNAMELENGHT);
    nameTextBox.setOnTextChange(std::bind(&OptionsMenu::onChangeOption, this, std::placeholders::_1));
    NameHBox.addWidget(&nameTextBox, 290);
    NameHBox.addWidget(Spacer::create(), 0.5);
    nameTextBox.setText(settings.general.playerName);

    mainVBox.addWidget(&NameHBox, 0.01);

    mainVBox.addWidget(VSpacer::create(5));

    gameOptionsHBox.addWidget(Spacer::create(), 0.5);

    gameOptionsHBox.addWidget(Label::create(_("Default Game Options")), 190);
    gameOptionsButton.setText(_("Change..."));
    gameOptionsButton.setOnClick(std::bind(&OptionsMenu::onGameOptions, this));
    gameOptionsHBox.addWidget(&gameOptionsButton, 130);

    gameOptionsHBox.addWidget(Spacer::create(), 160);

    gameOptionsHBox.addWidget(Spacer::create(), 0.5);

    mainVBox.addWidget(&gameOptionsHBox, 0.01);

    mainVBox.addWidget(Spacer::create(), 0.2);

    languageHBox.addWidget(Spacer::create(), 0.5);
    languageHBox.addWidget(Label::create(_("Language")), 190);

    for(size_t i = 0; i < availLanguages.size(); i++) {
        languageDropDownBox.addEntry(availLanguages[i].substr(0, availLanguages[i].size()-6), i);
        if(availLanguages[i].substr(availLanguages[i].size()-5,2) == settings.general.language) {
            languageDropDownBox.setSelectedItem(i);
        }
    }

    languageDropDownBox.setOnSelectionChange(std::bind(&OptionsMenu::onChangeOption, this, std::placeholders::_1));
    languageHBox.addWidget(&languageDropDownBox, 100);
    languageHBox.addWidget(Spacer::create(), 190);
    languageHBox.addWidget(Spacer::create(), 0.5);

    mainVBox.addWidget(&languageHBox, 0.01);

    mainVBox.addWidget(VSpacer::create(1));

    generalHBox.addWidget(Spacer::create(), 0.5);
    generalHBox.addWidget(Label::create(_("Campaign AI")), 190);
    for(unsigned int i=1; i<PlayerFactory::getList().size(); i++) {
        aiDropDownBox.addEntry(PlayerFactory::getByIndex(i)->getName(), i);
    }
    aiDropDownBox.setSelectedItem(PlayerFactory::getIndexByPlayerClass(settings.ai.campaignAI) - 1);
    aiDropDownBox.setOnSelectionChange(std::bind(&OptionsMenu::onChangeOption, this, std::placeholders::_1));
    generalHBox.addWidget(&aiDropDownBox, 140);
    generalHBox.addWidget(Spacer::create(), 20);
    introCheckbox.setText(_("Play Intro"));
    introCheckbox.setChecked(settings.general.playIntro);
    introCheckbox.setOnClick(std::bind(&OptionsMenu::onChangeOption, this, true));
    generalHBox.addWidget(&introCheckbox, 130);
    generalHBox.addWidget(Spacer::create(), 0.5);

    mainVBox.addWidget(&generalHBox, 0.01);

    mainVBox.addWidget(Spacer::create(), 0.2);

    resolutionHBox.addWidget(Spacer::create(), 0.5);
    resolutionHBox.addWidget(Label::create(_("Video Resolution")), 190);

    std::vector<Coord>::const_iterator iter;
    int i = 0;
    for(iter = availScreenRes.begin(); iter != availScreenRes.end(); ++iter, ++i) {
        char temp[20];
        snprintf(temp, 20, "%d x %d", iter->x, iter->y);
        resolutionDropDownBox.addEntry(temp, i);
        if(iter->x == settings.video.width && iter->y == settings.video.height) {
            resolutionDropDownBox.setSelectedItem(i);
        }
    }
    resolutionDropDownBox.setOnSelectionChange(std::bind(&OptionsMenu::onChangeOption, this, std::placeholders::_1));
    resolutionHBox.addWidget(&resolutionDropDownBox, 100);
    resolutionHBox.addWidget(Spacer::create(), 5);
    zoomlevelDropDownBox.addEntry("Zoomlevel x1", 0);
    zoomlevelDropDownBox.addEntry("Zoomlevel x2", 1);
    zoomlevelDropDownBox.addEntry("Zoomlevel x3", 2);
    zoomlevelDropDownBox.setSelectedItem(settings.video.preferredZoomLevel);
    zoomlevelDropDownBox.setOnSelectionChange(std::bind(&OptionsMenu::onChangeOption, this, std::placeholders::_1));
    resolutionHBox.addWidget(&zoomlevelDropDownBox, 102);
    resolutionHBox.addWidget(Spacer::create(), 5);
    for(int i = 0; i < Scaler::NumScaler; i++) {
        scalerDropDownBox.addEntry(Scaler::getScalerName((Scaler::ScalerType) i));
    }
    Scaler::ScalerType currentScaler = Scaler::getScalerByName(settings.video.scaler);
    scalerDropDownBox.setSelectedItem(currentScaler >= 0 ? currentScaler : Scaler::ScaleHD);
    scalerDropDownBox.setOnSelectionChange(std::bind(&OptionsMenu::onChangeOption, this, std::placeholders::_1));
    resolutionHBox.addWidget(&scalerDropDownBox, 78);

    resolutionHBox.addWidget(Spacer::create(), 0.5);

    mainVBox.addWidget(&resolutionHBox, 0.01);

    mainVBox.addWidget(VSpacer::create(1));

    videoHBox.addWidget(Spacer::create(), 0.5);
    fullScreenCheckbox.setText(_("Full Screen"));
    fullScreenCheckbox.setChecked(settings.video.fullscreen);
    fullScreenCheckbox.setOnClick(std::bind(&OptionsMenu::onChangeOption, this, true));
    videoHBox.addWidget(&fullScreenCheckbox, 240);
    videoHBox.addWidget(HSpacer::create(240));
    videoHBox.addWidget(Spacer::create(), 0.5);

    mainVBox.addWidget(&videoHBox, 0.01);

    mainVBox.addWidget(Spacer::create(), 0.2);

    audioHBox.addWidget(Spacer::create(), 0.5);
    playSFXCheckbox.setText(_("Play SFX"));
    playSFXCheckbox.setChecked(settings.audio.playSFX);
    playSFXCheckbox.setOnClick(std::bind(&OptionsMenu::onChangeOption, this, true));
    audioHBox.addWidget(&playSFXCheckbox, 240);
    playMusicCheckbox.setText(_("Play Music"));
    playMusicCheckbox.setChecked(settings.audio.playMusic);
    playMusicCheckbox.setOnClick(std::bind(&OptionsMenu::onChangeOption, this, true));
    audioHBox.addWidget(&playMusicCheckbox, 240);
    audioHBox.addWidget(Spacer::create(), 0.5);

    mainVBox.addWidget(&audioHBox, 0.01);

    mainVBox.addWidget(Spacer::create(), 0.2);

    networkPortHBox.addWidget(Spacer::create(), 0.5);
    networkPortHBox.addWidget(Label::create(_("Port")), 190);
    portTextBox.setMaximumTextLength(5);
    portTextBox.setAllowedChars("0123456789");
    portTextBox.setOnTextChange(std::bind(&OptionsMenu::onChangeOption, this, std::placeholders::_1));
    networkPortHBox.addWidget(&portTextBox, 100);
    portTextBox.setText(stringify<int>(settings.network.serverPort));
    networkPortHBox.addWidget(Spacer::create(), 190);
    networkPortHBox.addWidget(Spacer::create(), 0.5);
    mainVBox.addWidget(&networkPortHBox, 0.01);

    mainVBox.addWidget(VSpacer::create(5));

    networkMetaServerHBox.addWidget(Spacer::create(), 0.5);
    networkMetaServerHBox.addWidget(Label::create(_("MetaServer")), 190);
    metaServerTextBox.setOnTextChange(std::bind(&OptionsMenu::onChangeOption, this, std::placeholders::_1));
    networkMetaServerHBox.addWidget(&metaServerTextBox, 290);
    metaServerTextBox.setText(settings.network.metaServer);
    networkMetaServerHBox.addWidget(Spacer::create(), 0.5);
    mainVBox.addWidget(&networkMetaServerHBox, 0.01);

    mainVBox.addWidget(Spacer::create(), 0.2);

    okCancelHBox.addWidget(Spacer::create());

    backButton.setText(_("Back"));
    backButton.setOnClick(std::bind(&OptionsMenu::onOptionsCancel, this));
    okCancelHBox.addWidget(&backButton);

    okCancelHBox.addWidget(Spacer::create());

    acceptButton.setText(_("Accept"));
    acceptButton.setVisible(false);
    acceptButton.setOnClick(std::bind(&OptionsMenu::onOptionsOK, this));
    okCancelHBox.addWidget(&acceptButton);

    okCancelHBox.addWidget(Spacer::create());

    mainVBox.addWidget(&okCancelHBox, 26);

    mainVBox.addWidget(Spacer::create(), 0.1);
}

OptionsMenu::~OptionsMenu()
{
    ;
}

void OptionsMenu::onChangeOption(bool bInteractive) {
    bool bChanged = false;

    bChanged |= (settings.general.playerName != nameTextBox.getText());
    int languageIndex = languageDropDownBox.getSelectedEntryIntData();
    if(languageIndex >= 0) {
        std::string languageFilename = availLanguages[languageIndex];
        bChanged |= (settings.general.language != languageFilename.substr(languageFilename.size()-5,2));
    }
    const PlayerFactory::PlayerData* pPlayerData = PlayerFactory::getByIndex(aiDropDownBox.getSelectedEntryIntData());
    bChanged |= ((pPlayerData == nullptr) || (settings.ai.campaignAI != pPlayerData->getPlayerClass()));
    bChanged |= (settings.general.playIntro != introCheckbox.isChecked());

    int selectedResolution = resolutionDropDownBox.getSelectedEntryIntData();
    if(selectedResolution >= 0) {
        bChanged |= (settings.video.width != availScreenRes[selectedResolution].x);
        bChanged |= (settings.video.height != availScreenRes[selectedResolution].y);
    }
    bChanged |= (settings.video.preferredZoomLevel != zoomlevelDropDownBox.getSelectedEntryIntData());
    bChanged |= (settings.video.fullscreen != fullScreenCheckbox.isChecked());
    bChanged |= (settings.video.scaler != scalerDropDownBox.getSelectedEntry());

    bChanged |= (settings.audio.playSFX != playSFXCheckbox.isChecked());
    bChanged |= (settings.audio.playMusic != playMusicCheckbox.isChecked());

    bChanged |= (settings.gameOptions != currentGameOptions);

    bChanged |= (settings.network.serverPort != atoi(portTextBox.getText().c_str()));
    bChanged |= (settings.network.metaServer != metaServerTextBox.getText());

    acceptButton.setVisible(bChanged);
}

void OptionsMenu::onOptionsOK() {
    std::string playername = nameTextBox.getText();
    if(playername.empty()) {
        openWindow(MsgBox::create(_("Please enter a Player Name.")));
        return;
    }

    int serverport;
    if(!parseString(portTextBox.getText(), serverport) || serverport <= 0 || serverport > 65535) {
        openWindow(MsgBox::create(strprintf(_("Server Port must be between 1 and 65535!\nDefault Server Port is %d!"), DEFAULT_PORT)));
        return;
    }

    std::string metaserver = metaServerTextBox.getText();
    if(metaserver.empty()) {
        openWindow(MsgBox::create(_("Please enter a MetaServer.")));
        return;
    }

    settings.general.playerName = playername;

    std::string languageFilename = availLanguages[languageDropDownBox.getSelectedEntryIntData()];
    settings.general.language = languageFilename.substr(languageFilename.size()-5,2);
    settings.general.playIntro = introCheckbox.isChecked();

    const PlayerFactory::PlayerData* pPlayerData = PlayerFactory::getByIndex(aiDropDownBox.getSelectedEntryIntData());
    settings.ai.campaignAI = ((pPlayerData != nullptr) ? pPlayerData->getPlayerClass() : DEFAULTAIPLAYERCLASS);

    int selectedResolution = resolutionDropDownBox.getSelectedEntryIntData();
    settings.video.width = availScreenRes[selectedResolution].x;
    settings.video.height = availScreenRes[selectedResolution].y;
    int selectedZoomlevel = zoomlevelDropDownBox.getSelectedEntryIntData();
    settings.video.preferredZoomLevel = selectedZoomlevel;
    settings.video.scaler = scalerDropDownBox.getSelectedEntry();
    settings.video.fullscreen = fullScreenCheckbox.isChecked();

    settings.audio.playSFX = playSFXCheckbox.isChecked();
    settings.audio.playMusic = playMusicCheckbox.isChecked();

    settings.gameOptions = currentGameOptions;

    settings.network.serverPort = serverport;
    settings.network.metaServer = metaserver;

    saveConfiguration2File();

    // sound is not reinitialized when restarting
    // => music and sound player do not reload settings
    soundPlayer->setSound(settings.audio.playSFX);
    if(musicPlayer->isMusicOn() != settings.audio.playMusic) {
        musicPlayer->setMusic(settings.audio.playMusic);
        musicPlayer->changeMusic(MUSIC_INTRO);
    }

    quit(MENU_QUIT_REINITIALIZE);
}

void OptionsMenu::onOptionsCancel() {
    quit();
}

void OptionsMenu::onGameOptions() {
    openWindow(GameOptionsWindow::create(currentGameOptions));
}

void OptionsMenu::saveConfiguration2File() {
    INIFile myINIFile(getConfigFilepath());

    myINIFile.setBoolValue("General","Play Intro",settings.general.playIntro);

    myINIFile.setIntValue("Video","Width",settings.video.width);
    myINIFile.setIntValue("Video","Height",settings.video.height);
    myINIFile.setBoolValue("Video","Fullscreen",settings.video.fullscreen);
    myINIFile.setIntValue("Video","Preferred Zoom Level",settings.video.preferredZoomLevel);
    myINIFile.setStringValue("Video","Scaler",settings.video.scaler);

    myINIFile.setStringValue("General","Player Name",settings.general.playerName);
    myINIFile.setStringValue("General","Language",settings.general.language);

    myINIFile.setStringValue("AI","Campaign AI",settings.ai.campaignAI);

    myINIFile.setBoolValue("Audio","Play SFX",settings.audio.playSFX);
    myINIFile.setBoolValue("Audio","Play Music",settings.audio.playMusic);

    myINIFile.setIntValue("Game Options","Game Speed",settings.gameOptions.gameSpeed);
    myINIFile.setBoolValue("Game Options","Concrete Required",settings.gameOptions.concreteRequired);
    myINIFile.setBoolValue("Game Options","Structures Degrade On Concrete",settings.gameOptions.structuresDegradeOnConcrete);
    myINIFile.setBoolValue("Game Options","Fog of War",settings.gameOptions.fogOfWar);
    myINIFile.setBoolValue("Game Options","Start with Explored Map",settings.gameOptions.startWithExploredMap);
    myINIFile.setBoolValue("Game Options","Instant Build",settings.gameOptions.instantBuild);
    myINIFile.setBoolValue("Game Options","Only One Palace",settings.gameOptions.onlyOnePalace);
    myINIFile.setBoolValue("Game Options","Rocket-Turrets Need Power",settings.gameOptions.rocketTurretsNeedPower);
    myINIFile.setBoolValue("Game Options","Sandworms Respawn",settings.gameOptions.sandwormsRespawn);
    myINIFile.setBoolValue("Game Options","Killed Sandworms Drop Spice",settings.gameOptions.killedSandwormsDropSpice);
    myINIFile.setBoolValue("Game Options","Manual Carryall Drops",settings.gameOptions.manualCarryallDrops);
    myINIFile.setIntValue("Game Options","Maximum Number of Units",settings.gameOptions.maximumNumberOfUnits);

    myINIFile.setIntValue("Network","ServerPort",settings.network.serverPort);
    myINIFile.setStringValue("Network","MetaServer",settings.network.metaServer);

    myINIFile.saveChangesTo(getConfigFilepath());
}

void OptionsMenu::determineAvailableScreenResolutions() {
    availScreenRes.clear();

    SDL_DisplayMode displayMode;
    int numDisplayModes = SDL_GetNumDisplayModes(SCREEN_DISPLAYINDEX);
    for(int i = numDisplayModes-1; i >=0; i--) {
        if(SDL_GetDisplayMode(SCREEN_DISPLAYINDEX, i, &displayMode) == 0) {
            Coord screenRes(displayMode.w, displayMode.h);
            if(screenRes.x >= SCREEN_MIN_WIDTH && screenRes.y >= SCREEN_MIN_HEIGHT) {
                if(std::find(availScreenRes.begin(), availScreenRes.end(), screenRes) == availScreenRes.end()) {
                    // not yet in the list (might happen if e.g. multiple refresh rates are reported)
                    availScreenRes.push_back(screenRes);
                }
            }
        }
    }

    if(availScreenRes.empty()) {
        // Not possible or not available
        // try some standard resolutions

        availScreenRes.push_back( Coord(640, 480) );    // VGA (4:3)
        availScreenRes.push_back( Coord(800, 480) );    // WVGA (5:3)
        availScreenRes.push_back( Coord(800, 600) );    // SVGA (4:3)
        availScreenRes.push_back( Coord(960, 540) );    // Quarter HD (16:9)
        availScreenRes.push_back( Coord(960, 720) );    // ? (4:3)
        availScreenRes.push_back( Coord(1024, 576) );   // WSVGA (16:9)
        availScreenRes.push_back( Coord(1024, 640) );   // ? (16:10)
        availScreenRes.push_back( Coord(1024, 768) );   // XGA (4:3)
        availScreenRes.push_back( Coord(1152, 864) );   // XGA+ (4:3)
        availScreenRes.push_back( Coord(1280, 720 ) );  // WXGA (16:9)
        availScreenRes.push_back( Coord(1280, 768) );   // WXGA (5:3)
        availScreenRes.push_back( Coord(1280, 800) );   // WXGA (16:10)
        availScreenRes.push_back( Coord(1280, 960) );   // SXGA- (4:3)
        availScreenRes.push_back( Coord(1280, 1024) );  // SXGA (5:4)
        availScreenRes.push_back( Coord(1366, 768) );   // HDTV 720p (~16:9)
        availScreenRes.push_back( Coord(1400, 1050) );  // SXGA+ (4:3)
        availScreenRes.push_back( Coord(1440, 900) );   // WXGA+ (16:10)
        availScreenRes.push_back( Coord(1440, 1080) );  // ? (4:3)
        availScreenRes.push_back( Coord(1600, 900) );   // WSXGA (16:9)
        availScreenRes.push_back( Coord(1600, 1200) );  // UXGA (4:3)
        availScreenRes.push_back( Coord(1680, 1050) );  // WSXGA+ (16:10)
        availScreenRes.push_back( Coord(1920, 1080) );  // 1080p (16:9)
        availScreenRes.push_back( Coord(1920, 1200) );  // WUXGA (16:10)
    }

    Coord currentRes(settings.video.width, settings.video.height);

    if(std::find(availScreenRes.begin(), availScreenRes.end(), currentRes) == availScreenRes.end()) {
        // not yet in the list
        availScreenRes.insert(availScreenRes.begin(), currentRes);
    }
}

void OptionsMenu::onChildWindowClose(Window* pChildWindow) {
    GameOptionsWindow* pGameOptionsWindow = dynamic_cast<GameOptionsWindow*>(pChildWindow);
    if(pGameOptionsWindow != nullptr) {
        currentGameOptions = pGameOptionsWindow->getGameOptions();

        onChangeOption(true);
    }
}
