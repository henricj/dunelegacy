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
#include <GUI/MsgBox.h>
#include <GUI/Spacer.h>
#include <GUI/dune/GameOptionsWindow.h>

#include <main.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/INIFile.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/music/MusicPlayer.h>

#include <SoundPlayer.h>

#include <players/PlayerFactory.h>

#include <misc/FileSystem.h>
#include <misc/Scaler.h>

#include <fmt/printf.h>

#include <algorithm>

OptionsMenu::OptionsMenu() : currentGameOptions(settings.gameOptions) {
    determineAvailableScreenResolutions();

    const auto languagesList = getFileNamesList(getDuneLegacyDataDir() / "locale", "po", true, FileListOrder_Name_Asc);
    availLanguages.reserve(languagesList.size());

    for (const auto& ll : languagesList)
        availLanguages.emplace_back(reinterpret_cast<const char*>(ll.u8string().c_str()));

    // set up window
    const auto* const pBackground = pGFXManager->getUIGraphic(UI_MenuBackground);
    OptionsMenu::setBackground(pBackground);
    OptionsMenu::resize(getTextureSize(pBackground));

    OptionsMenu::setWindowWidget(&windowWidget);

    windowWidget.addWidget(&mainVBox, Point(50, 50), Point(getSize().x - 100, getSize().y - 100));

    mainVBox.addWidget(Spacer::create(), 0.2);

    NameHBox.addWidget(Spacer::create(), 0.5);
    NameHBox.addWidget(Label::create(_("Player Name")).release(), 190);
    nameTextBox.setMaximumTextLength(MAX_PLAYERNAMELENGTH);
    nameTextBox.setOnTextChange([this](auto interactive) { onChangeOption(interactive); });
    NameHBox.addWidget(&nameTextBox, 290);
    NameHBox.addWidget(Spacer::create(), 0.5);
    nameTextBox.setText(settings.general.playerName);

    mainVBox.addWidget(&NameHBox, 0.01);

    mainVBox.addWidget(VSpacer::create(5));

    gameOptionsHBox.addWidget(Spacer::create(), 0.5);

    gameOptionsHBox.addWidget(Label::create(_("Default Game Options")).release(), 190);
    gameOptionsButton.setText(_("Change..."));
    gameOptionsButton.setOnClick([this] { onGameOptions(); });
    gameOptionsHBox.addWidget(&gameOptionsButton, 130);

    gameOptionsHBox.addWidget(Spacer::create(), 160);

    gameOptionsHBox.addWidget(Spacer::create(), 0.5);

    mainVBox.addWidget(&gameOptionsHBox, 0.01);

    mainVBox.addWidget(Spacer::create(), 0.2);

    languageHBox.addWidget(Spacer::create(), 0.5);
    languageHBox.addWidget(Label::create(_("Language")).release(), 190);

    for (auto i = decltype(availLanguages.size()){0}; i < availLanguages.size(); i++) {
        languageDropDownBox.addEntry(availLanguages[i].substr(0, availLanguages[i].size() - 6), i);
        if (availLanguages[i].substr(availLanguages[i].size() - 5, 2) == settings.general.language) {
            languageDropDownBox.setSelectedItem(i);
        }
    }

    languageDropDownBox.setOnSelectionChange([this](auto interactive) { onChangeOption(interactive); });
    languageHBox.addWidget(&languageDropDownBox, 100);
    languageHBox.addWidget(Spacer::create(), 190);
    languageHBox.addWidget(Spacer::create(), 0.5);

    mainVBox.addWidget(&languageHBox, 0.01);

    mainVBox.addWidget(VSpacer::create(1));

    generalHBox.addWidget(Spacer::create(), 0.5);
    generalHBox.addWidget(Label::create(_("Campaign AI")).release(), 190);
    for (unsigned int i = 1; i < PlayerFactory::getList().size(); i++) {
        aiDropDownBox.addEntry(PlayerFactory::getByIndex(i)->getName(), i);
    }
    aiDropDownBox.setSelectedItem(PlayerFactory::getIndexByPlayerClass(settings.ai.campaignAI) - 1);
    aiDropDownBox.setOnSelectionChange([this](auto interactive) { onChangeOption(interactive); });
    generalHBox.addWidget(&aiDropDownBox, 140);
    generalHBox.addWidget(Spacer::create(), 20);
    introCheckbox.setText(_("Play Intro"));
    introCheckbox.setChecked(settings.general.playIntro);
    introCheckbox.setOnClick([this] { onChangeOption(true); });
    generalHBox.addWidget(&introCheckbox, 130);
    generalHBox.addWidget(Spacer::create(), 0.5);

    mainVBox.addWidget(&generalHBox, 0.01);

    mainVBox.addWidget(Spacer::create(), 0.2);

    resolutionHBox.addWidget(Spacer::create(), 0.5);
    resolutionHBox.addWidget(Label::create(_("Video Resolution")).release(), 190);

    int i = 0;
    for (const Coord& coord : availScreenRes) {
        int factor = getLogicalToPhysicalResolutionFactor(coord.x, coord.y);
        if (factor > 1) {
            resolutionDropDownBox.addEntry(fmt::sprintf("%d x %d @ %dx", coord.x, coord.y, factor), i);
        } else {
            resolutionDropDownBox.addEntry(fmt::sprintf("%d x %d", coord.x, coord.y), i);
        }
        if (coord.x == settings.video.physicalWidth && coord.y == settings.video.physicalHeight) {
            resolutionDropDownBox.setSelectedItem(i);
        }
        i++;
    }
    resolutionDropDownBox.setOnSelectionChange([this](auto interactive) { onChangeOption(interactive); });
    resolutionHBox.addWidget(&resolutionDropDownBox, 130);
    resolutionHBox.addWidget(Spacer::create(), 5);
    zoomlevelDropDownBox.addEntry("Zoom 1x", 0);
    zoomlevelDropDownBox.addEntry("Zoom 2x", 1);
    zoomlevelDropDownBox.addEntry("Zoom 3x", 2);
    zoomlevelDropDownBox.setSelectedItem(settings.video.preferredZoomLevel);
    zoomlevelDropDownBox.setOnSelectionChange([this](auto interactive) { onChangeOption(interactive); });
    resolutionHBox.addWidget(&zoomlevelDropDownBox, 72);
    resolutionHBox.addWidget(Spacer::create(), 5);
    for (int j = 0; j < Scaler::NumScaler; j++) {
        scalerDropDownBox.addEntry(Scaler::getScalerName(static_cast<Scaler::ScalerType>(j)));
    }
    const Scaler::ScalerType currentScaler = Scaler::getScalerByName(settings.video.scaler);
    scalerDropDownBox.setSelectedItem(currentScaler >= 0 ? currentScaler : Scaler::ScaleHD);
    scalerDropDownBox.setOnSelectionChange([this](auto interactive) { onChangeOption(interactive); });
    resolutionHBox.addWidget(&scalerDropDownBox, 78);

    resolutionHBox.addWidget(Spacer::create(), 0.5);

    mainVBox.addWidget(&resolutionHBox, 0.01);

    mainVBox.addWidget(VSpacer::create(1));

    videoHBox.addWidget(Spacer::create(), 0.5);
    fullScreenCheckbox.setText(_("Full Screen"));
    fullScreenCheckbox.setChecked(settings.video.fullscreen);
    fullScreenCheckbox.setOnClick([this] { onChangeOption(true); });
    videoHBox.addWidget(&fullScreenCheckbox, 240);
    showTutorialHintsCheckbox.setText(_("Show Tutorial Hints"));
    showTutorialHintsCheckbox.setChecked(settings.general.showTutorialHints);
    showTutorialHintsCheckbox.setOnClick([this] { onChangeOption(true); });
    videoHBox.addWidget(&showTutorialHintsCheckbox, 240);
    videoHBox.addWidget(Spacer::create(), 0.5);

    mainVBox.addWidget(&videoHBox, 0.01);

    mainVBox.addWidget(Spacer::create(), 0.2);

    audioHBox.addWidget(Spacer::create(), 0.5);
    playSFXCheckbox.setText(_("Play SFX"));
    playSFXCheckbox.setChecked(settings.audio.playSFX);
    playSFXCheckbox.setOnClick([this] { onChangeOption(true); });
    audioHBox.addWidget(&playSFXCheckbox, 240);
    playMusicCheckbox.setText(_("Play Music"));
    playMusicCheckbox.setChecked(settings.audio.playMusic);
    playMusicCheckbox.setOnClick([this] { onChangeOption(true); });
    audioHBox.addWidget(&playMusicCheckbox, 240);
    audioHBox.addWidget(Spacer::create(), 0.5);

    mainVBox.addWidget(&audioHBox, 0.01);

    mainVBox.addWidget(Spacer::create(), 0.2);

    networkPortHBox.addWidget(Spacer::create(), 0.5);
    networkPortHBox.addWidget(Label::create(_("Port")).release(), 190);
    portTextBox.setMaximumTextLength(5);
    portTextBox.setAllowedChars("0123456789");
    portTextBox.setOnTextChange([this](auto interactive) { onChangeOption(interactive); });
    networkPortHBox.addWidget(&portTextBox, 100);
    portTextBox.setText(std::to_string(settings.network.serverPort));
    networkPortHBox.addWidget(Spacer::create(), 190);
    networkPortHBox.addWidget(Spacer::create(), 0.5);
    mainVBox.addWidget(&networkPortHBox, 0.01);

    mainVBox.addWidget(VSpacer::create(5));

    networkMetaServerHBox.addWidget(Spacer::create(), 0.5);
    networkMetaServerHBox.addWidget(Label::create(_("MetaServer")).release(), 190);
    metaServerTextBox.setOnTextChange([this](auto interactive) { onChangeOption(interactive); });
    networkMetaServerHBox.addWidget(&metaServerTextBox, 290);
    metaServerTextBox.setText(settings.network.metaServer);
    networkMetaServerHBox.addWidget(Spacer::create(), 0.5);
    mainVBox.addWidget(&networkMetaServerHBox, 0.01);

    mainVBox.addWidget(Spacer::create(), 0.2);

    okCancelHBox.addWidget(Spacer::create());

    backButton.setText(_("Back"));
    backButton.setOnClick([this] { onOptionsCancel(); });
    okCancelHBox.addWidget(&backButton);

    okCancelHBox.addWidget(Spacer::create());

    acceptButton.setText(_("Accept"));
    acceptButton.setVisible(false);
    acceptButton.setOnClick([this] { onOptionsOK(); });
    okCancelHBox.addWidget(&acceptButton);

    okCancelHBox.addWidget(Spacer::create());

    mainVBox.addWidget(&okCancelHBox, 26);

    mainVBox.addWidget(Spacer::create(), 0.1);
}

OptionsMenu::~OptionsMenu() = default;

void OptionsMenu::onChangeOption(bool bInteractive) {
    bool bChanged = false;

    bChanged |= settings.general.playerName != nameTextBox.getText();
    const int languageIndex = languageDropDownBox.getSelectedEntryIntData();
    if (languageIndex >= 0 && languageIndex < availLanguages.size()) {
        const std::string languageFilename = availLanguages[languageIndex];
        bChanged |= settings.general.language != languageFilename.substr(languageFilename.size() - 5, 2);
    }
    const PlayerFactory::PlayerData* pPlayerData = PlayerFactory::getByIndex(aiDropDownBox.getSelectedEntryIntData());
    bChanged |= pPlayerData == nullptr || settings.ai.campaignAI != pPlayerData->getPlayerClass();
    bChanged |= settings.general.playIntro != introCheckbox.isChecked();
    bChanged |= settings.general.showTutorialHints != showTutorialHintsCheckbox.isChecked();

    const int selectedResolution = resolutionDropDownBox.getSelectedEntryIntData();
    if (selectedResolution >= 0) {
        bChanged |= settings.video.physicalWidth != availScreenRes[selectedResolution].x;
        bChanged |= settings.video.physicalHeight != availScreenRes[selectedResolution].y;
    }
    bChanged |= settings.video.preferredZoomLevel != zoomlevelDropDownBox.getSelectedEntryIntData();
    bChanged |= settings.video.fullscreen != fullScreenCheckbox.isChecked();
    bChanged |= settings.video.scaler != scalerDropDownBox.getSelectedEntry();

    bChanged |= settings.audio.playSFX != playSFXCheckbox.isChecked();
    bChanged |= settings.audio.playMusic != playMusicCheckbox.isChecked();

    bChanged |= settings.gameOptions != currentGameOptions;

    int port;
    if (parseString(portTextBox.getText(), port))
        bChanged |= settings.network.serverPort != port;

    bChanged |= settings.network.metaServer != metaServerTextBox.getText();

    acceptButton.setVisible(bChanged);
}

void OptionsMenu::onOptionsOK() {
    const std::string playername{nameTextBox.getText()};
    if (playername.empty()) {
        openWindow(MsgBox::create(_("Please enter a Player Name.")));
        return;
    }

    int serverport = 0;
    if (!parseString(portTextBox.getText(), serverport) || serverport <= 0 || serverport > 65535) {
        openWindow(MsgBox::create(
            fmt::sprintf(_("Server Port must be between 1 and 65535!\nDefault Server Port is %d!"), DEFAULT_PORT)));
        return;
    }

    const std::string metaserver{metaServerTextBox.getText()};
    if (metaserver.empty()) {
        openWindow(MsgBox::create(_("Please enter a MetaServer.")));
        return;
    }

    settings.general.playerName        = playername;
    const auto selectedLanguage        = languageDropDownBox.getSelectedEntryIntData();
    const std::string languageFilename = selectedLanguage < 0 || selectedLanguage >= availLanguages.size()
                                           ? "English.en.po"
                                           : availLanguages[selectedLanguage];
    settings.general.language          = languageFilename.substr(languageFilename.size() - 5, 2);
    settings.general.playIntro         = introCheckbox.isChecked();
    settings.general.showTutorialHints = showTutorialHintsCheckbox.isChecked();

    const PlayerFactory::PlayerData* pPlayerData = PlayerFactory::getByIndex(aiDropDownBox.getSelectedEntryIntData());
    settings.ai.campaignAI = pPlayerData != nullptr ? pPlayerData->getPlayerClass() : DEFAULTAIPLAYERCLASS;

    const int selectedResolution  = resolutionDropDownBox.getSelectedEntryIntData();
    settings.video.physicalWidth  = selectedResolution >= 0 ? availScreenRes[selectedResolution].x : 0;
    settings.video.physicalHeight = selectedResolution >= 0 ? availScreenRes[selectedResolution].y : 0;

    settings.video.width              = settings.video.physicalWidth;
    settings.video.height             = settings.video.physicalHeight;;
    settings.video.preferredZoomLevel = zoomlevelDropDownBox.getSelectedEntryIntData();
    settings.video.scaler             = scalerDropDownBox.getSelectedEntry();
    settings.video.fullscreen         = fullScreenCheckbox.isChecked();

    settings.audio.playSFX   = playSFXCheckbox.isChecked();
    settings.audio.playMusic = playMusicCheckbox.isChecked();

    settings.gameOptions = currentGameOptions;

    settings.network.serverPort = serverport;
    settings.network.metaServer = metaserver;

    saveConfiguration2File();

    // sound is not reinitialized when restarting
    // => music and sound player do not reload settings
    soundPlayer->setSound(settings.audio.playSFX);
    if (musicPlayer->isMusicOn() != settings.audio.playMusic) {
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

    myINIFile.setBoolValue("General", "Play Intro", settings.general.playIntro);
    myINIFile.setBoolValue("General", "Show Tutorial Hints", settings.general.showTutorialHints);

    myINIFile.setIntValue("Video", "Physical Width", settings.video.physicalWidth);
    myINIFile.setIntValue("Video", "Physical Height", settings.video.physicalHeight);
    myINIFile.setIntValue("Video", "Width", settings.video.width);
    myINIFile.setIntValue("Video", "Height", settings.video.height);
    myINIFile.setBoolValue("Video", "Fullscreen", settings.video.fullscreen);
    myINIFile.setIntValue("Video", "Preferred Zoom Level", settings.video.preferredZoomLevel);
    myINIFile.setStringValue("Video", "Scaler", settings.video.scaler);
    myINIFile.setBoolValue("Video", "RotateUnitGraphics", settings.video.rotateUnitGraphics);

    myINIFile.setStringValue("General", "Player Name", settings.general.playerName);
    myINIFile.setStringValue("General", "Language", settings.general.language);

    myINIFile.setStringValue("AI", "Campaign AI", settings.ai.campaignAI);

    myINIFile.setBoolValue("Audio", "Play SFX", settings.audio.playSFX);
    myINIFile.setBoolValue("Audio", "Play Music", settings.audio.playMusic);

    myINIFile.setIntValue("Game Options", "Game Speed", settings.gameOptions.gameSpeed);
    myINIFile.setBoolValue("Game Options", "Concrete Required", settings.gameOptions.concreteRequired);
    myINIFile.setBoolValue("Game Options", "Structures Degrade On Concrete",
                           settings.gameOptions.structuresDegradeOnConcrete);
    myINIFile.setBoolValue("Game Options", "Fog of War", settings.gameOptions.fogOfWar);
    myINIFile.setBoolValue("Game Options", "Start with Explored Map", settings.gameOptions.startWithExploredMap);
    myINIFile.setBoolValue("Game Options", "Instant Build", settings.gameOptions.instantBuild);
    myINIFile.setBoolValue("Game Options", "Only One Palace", settings.gameOptions.onlyOnePalace);
    myINIFile.setBoolValue("Game Options", "Rocket-Turrets Need Power", settings.gameOptions.rocketTurretsNeedPower);
    myINIFile.setBoolValue("Game Options", "Sandworms Respawn", settings.gameOptions.sandwormsRespawn);
    myINIFile.setBoolValue("Game Options", "Killed Sandworms Drop Spice",
                           settings.gameOptions.killedSandwormsDropSpice);
    myINIFile.setBoolValue("Game Options", "Manual Carryall Drops", settings.gameOptions.manualCarryallDrops);
    myINIFile.setIntValue("Game Options", "Maximum Number of Units Override",
                          settings.gameOptions.maximumNumberOfUnitsOverride);

    myINIFile.setIntValue("Network", "ServerPort", settings.network.serverPort);
    myINIFile.setStringValue("Network", "MetaServer", settings.network.metaServer);

    if (!myINIFile.saveChangesTo(getConfigFilepath())) {
        sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Unable to save configuration file %s",
                        reinterpret_cast<const char*>(getConfigFilepath().u8string().c_str()));
    }
}

void OptionsMenu::addResolution(const Coord& screenRes) {
    if (screenRes.x >= SCREEN_MIN_WIDTH && screenRes.y >= SCREEN_MIN_HEIGHT) {
        if (std::ranges::find(availScreenRes, screenRes) == availScreenRes.end()) {
            // not yet in the list (might happen if e.g. multiple refresh rates are reported)
            availScreenRes.push_back(screenRes);
        }
    }
}

void OptionsMenu::determineAvailableScreenResolutions() {
    availScreenRes.clear();

    // Make it possible to open a window on a RDP session that isn't
    // full screen.

    SDL_DisplayMode displayMode;
    const int displayIndex    = SDL_GetWindowDisplayIndex(window);
    const int numDisplayModes = SDL_GetNumDisplayModes(displayIndex);
    for (int i = numDisplayModes - 1; i >= 0; i--) {
        if (SDL_GetDisplayMode(displayIndex, i, &displayMode) == 0) {
            addResolution(Coord{displayMode.w, displayMode.h});
        }
    }

    if (availScreenRes.empty()) {
        // Not possible or not available
        // try some standard resolutions

        availScreenRes.emplace_back(640, 480);   // VGA (4:3)
        availScreenRes.emplace_back(800, 480);   // WVGA (5:3)
        availScreenRes.emplace_back(800, 600);   // SVGA (4:3)
        availScreenRes.emplace_back(960, 540);   // Quarter HD (16:9)
        availScreenRes.emplace_back(960, 720);   // ? (4:3)
        availScreenRes.emplace_back(1024, 576);  // WSVGA (16:9)
        availScreenRes.emplace_back(1024, 640);  // ? (16:10)
        availScreenRes.emplace_back(1024, 768);  // XGA (4:3)
        availScreenRes.emplace_back(1152, 864);  // XGA+ (4:3)
        availScreenRes.emplace_back(1280, 720);  // WXGA (16:9)
        availScreenRes.emplace_back(1280, 768);  // WXGA (5:3)
        availScreenRes.emplace_back(1280, 800);  // WXGA (16:10)
        availScreenRes.emplace_back(1280, 960);  // SXGA- (4:3)
        availScreenRes.emplace_back(1280, 1024); // SXGA (5:4)
        availScreenRes.emplace_back(1366, 768);  // HDTV 720p (~16:9)
        availScreenRes.emplace_back(1400, 1050); // SXGA+ (4:3)
        availScreenRes.emplace_back(1440, 900);  // WXGA+ (16:10)
        availScreenRes.emplace_back(1440, 1080); // ? (4:3)
        availScreenRes.emplace_back(1600, 900);  // WSXGA (16:9)
        availScreenRes.emplace_back(1600, 1200); // UXGA (4:3)
        availScreenRes.emplace_back(1680, 1050); // WSXGA+ (16:10)
        availScreenRes.emplace_back(1920, 1080); // 1080p (16:9)
        availScreenRes.emplace_back(1920, 1200); // WUXGA (16:10)
        availScreenRes.emplace_back(2560, 1440); // WQHD (16:9)
        availScreenRes.emplace_back(2560, 1600); // WQXGA (16:10)
        availScreenRes.emplace_back(3840, 2160); // 2160p (16:9)
    } else {
        // We append a few more resolutions in case we only have on
        // screen resolution but still want to run in a window.  Fo
        // example, when running in an RDP session on Windows.

        auto appendRes = [&](int x, int y) {
            const auto can_fit = std::ranges::any_of(
                availScreenRes, [&](const auto& screenRes) { return screenRes.x > x && screenRes.y > y; });

            if (can_fit)
                addResolution(Coord{x, y});
        };

        appendRes(640, 480);
        appendRes(800, 600);
        appendRes(1024, 768);
        appendRes(1280, 1024);
        appendRes(1600, 900);
        appendRes(1600, 1200);
        appendRes(1920, 1080);
        appendRes(1920, 1200);
    }

    const Coord currentRes(settings.video.physicalWidth, settings.video.physicalHeight);

    addResolution(currentRes);
}

void OptionsMenu::onChildWindowClose(Window* pChildWindow) {
    auto* pGameOptionsWindow = dynamic_cast<GameOptionsWindow*>(pChildWindow);
    if (pGameOptionsWindow != nullptr) {
        currentGameOptions = pGameOptionsWindow->getGameOptions();

        onChangeOption(true);
    }
}
