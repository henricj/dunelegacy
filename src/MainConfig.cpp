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

#include <main.h>

#include <config.h>
#include <globals.h>

#include <FileClasses/DuneConfig.h>
#include <FileClasses/FileManager.h>
#include <FileClasses/INIFile.h>
#include <FileClasses/TextManager.h>

#include <GUI/GUIStyle.h>

#include <misc/FileSystem.h>
#include <misc/Scaler.h>
#include <misc/dune_sdl.h>
#include <misc/string_util.h>

#include "logging.h"

#include <cstring>

#ifdef _WIN32
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <Windows.h>
#elif defined(__APPLE__)
#    include <MacFunctions.h>
#else
#    include <cstdlib>
#endif

std::string getUserLanguage() {
    const char* pLang = nullptr;

#ifdef _WIN32
    char ISO639_LanguageName[10];
    if (GetLocaleInfo(GetUserDefaultLCID(), LOCALE_SISO639LANGNAME, ISO639_LanguageName, sizeof(ISO639_LanguageName))
        == 0) {
        return "";
    }
    pLang = ISO639_LanguageName;

#elif defined(__APPLE__)
    pLang = getMacLanguage();
    if (pLang == nullptr) {
        return "";
    }

#else
    // should work on most unices
    pLang = getenv("LC_ALL");
    if (pLang == nullptr) {
        // try LANG
        pLang = getenv("LANG");
        if (pLang == nullptr) {
            return "";
        }
    }
#endif

    sdl2::log_info("User locale is '{}'", pLang);

    if (strlen(pLang) < 2) {
        return "";
    }
    return strToLower(std::string(pLang, 2));
}

void load_settings(const INIFile& myINIFile) {
    auto& settings = dune::globals::settings;

    settings.general.playIntro         = myINIFile.getBoolValue("General", "Play Intro", false);
    settings.general.playerName        = myINIFile.getStringValue("General", "Player Name", "Player");
    settings.general.language          = myINIFile.getStringValue("General", "Language", "en");
    settings.general.scrollSpeed       = myINIFile.getIntValue("General", "Scroll Speed", 50);
    settings.general.showTutorialHints = myINIFile.getBoolValue("General", "Show Tutorial Hints", true);
    settings.video.width               = myINIFile.getIntValue("Video", "Width", 640);
    settings.video.height              = myINIFile.getIntValue("Video", "Height", 480);
    settings.video.physicalWidth       = myINIFile.getIntValue("Video", "Physical Width", 640);
    settings.video.physicalHeight      = myINIFile.getIntValue("Video", "Physical Height", 480);
    settings.video.fullscreen          = myINIFile.getBoolValue("Video", "Fullscreen", false);
    settings.video.frameLimit          = myINIFile.getBoolValue("Video", "FrameLimit", true);
    settings.video.preferredZoomLevel  = myINIFile.getIntValue("Video", "Preferred Zoom Level", 0);
    settings.video.scaler              = myINIFile.getStringValue("Video", "Scaler", "ScaleHD");
    settings.video.rotateUnitGraphics  = myINIFile.getBoolValue("Video", "RotateUnitGraphics", false);
    settings.video.renderer            = myINIFile.getStringValue("Video", "Renderer", "default");
    settings.video.typeface            = myINIFile.getStringValue("Video", "Typeface", "default");
    settings.audio.musicType           = myINIFile.getStringValue("Audio", "Music Type", "adl");
    settings.audio.playMusic           = myINIFile.getBoolValue("Audio", "Play Music", true);
    settings.audio.musicVolume         = myINIFile.getIntValue("Audio", "Music Volume", 64);
    settings.audio.playSFX             = myINIFile.getBoolValue("Audio", "Play SFX", true);
    settings.audio.sfxVolume           = myINIFile.getIntValue("Audio", "SFX Volume", 64);

    settings.network.serverPort   = myINIFile.getIntValue("Network", "ServerPort", DEFAULT_PORT);
    settings.network.metaServer   = myINIFile.getStringValue("Network", "MetaServer", DEFAULT_METASERVER);
    settings.network.debugNetwork = myINIFile.getBoolValue("Network", "Debug Network", false);

    settings.ai.campaignAI = myINIFile.getStringValue("AI", "Campaign AI", DEFAULTAIPLAYERCLASS);

    settings.gameOptions.gameSpeed        = myINIFile.getIntValue("Game Options", "Game Speed", GAMESPEED_DEFAULT);
    settings.gameOptions.concreteRequired = myINIFile.getBoolValue("Game Options", "Concrete Required", true);
    settings.gameOptions.structuresDegradeOnConcrete =
        myINIFile.getBoolValue("Game Options", "Structures Degrade On Concrete", true);
    settings.gameOptions.fogOfWar = myINIFile.getBoolValue("Game Options", "Fog of War", false);
    settings.gameOptions.startWithExploredMap =
        myINIFile.getBoolValue("Game Options", "Start with Explored Map", false);
    settings.gameOptions.instantBuild  = myINIFile.getBoolValue("Game Options", "Instant Build", false);
    settings.gameOptions.onlyOnePalace = myINIFile.getBoolValue("Game Options", "Only One Palace", false);
    settings.gameOptions.rocketTurretsNeedPower =
        myINIFile.getBoolValue("Game Options", "Rocket-Turrets Need Power", false);
    settings.gameOptions.sandwormsRespawn = myINIFile.getBoolValue("Game Options", "Sandworms Respawn", false);
    settings.gameOptions.killedSandwormsDropSpice =
        myINIFile.getBoolValue("Game Options", "Killed Sandworms Drop Spice", false);
    settings.gameOptions.manualCarryallDrops = myINIFile.getBoolValue("Game Options", "Manual Carryall Drops", false);
    settings.gameOptions.maximumNumberOfUnitsOverride =
        myINIFile.getIntValue("Game Options", "Maximum Number of Units Override", -1);
}

bool configure_game(int argc, char* argv[], bool bFirstInit, int currentDisplayIndex) {
    auto bFirstGamestart = false;

    // check if configfile exists
    const auto config_filepath = getConfigFilepath();

    if (!existsFile(config_filepath)) {
        std::string userLanguage = getUserLanguage();
        if (userLanguage.empty()) {
            userLanguage = "en";
        }

        bFirstGamestart = true;
        createDefaultConfigFile(config_filepath, userLanguage);
    }

    INIFile myINIFile(config_filepath);

    load_settings(myINIFile);

    auto& settings = dune::globals::settings;

    dune::globals::pTextManager = std::make_unique<TextManager>(settings.general.language);

    const CaseInsensitiveFilesystemCache filesystemCache(FileManager::getSearchPath());

    const auto missingFiles = PakFileConfiguration ::getMissingFiles(filesystemCache);
    if (!missingFiles.empty()) {
        // set back to English
        auto setBackToEnglishWarning =
            fmt::sprintf("The following files are missing for language \"%s\":\n", _("LanguageFileExtension"));
        for (const auto& filename : missingFiles) {
            setBackToEnglishWarning += reinterpret_cast<const char*>(filename.u8string().c_str());
            setBackToEnglishWarning += "\n";
        }
        setBackToEnglishWarning += "\nLanguage is changed to English!";
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Dune Legacy", setBackToEnglishWarning.c_str(), nullptr);

        sdl2::log_info("Warning: Language is changed to English!");

        settings.general.language = "en";
        myINIFile.setStringValue("General", "Language", settings.general.language);
        if (!myINIFile.saveChangesTo(config_filepath)) {
            sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION,
                            "Unable to save configuration file {}",
                            reinterpret_cast<const char*>(config_filepath.u8string().c_str()));
        }

        // reinit text manager
        dune::globals::pTextManager = std::make_unique<TextManager>(settings.general.language);
    }

    for (int i = 1; i < argc; i++) {
        // check for overriding params
        std::string parameter(argv[i]);

        if ((parameter == "-f") || (parameter == "--fullscreen")) {
            settings.video.fullscreen = true;
        } else if ((parameter == "-w") || (parameter == "--window")) {
            settings.video.fullscreen = false;
        } else if (parameter.compare(0, 13, "--PlayerName=") == 0) {
            settings.general.playerName = parameter.substr(strlen("--PlayerName="));
        } else if (parameter.compare(0, 13, "--ServerPort=") == 0) {
            settings.network.serverPort = atol(argv[i] + strlen("--ServerPort="));
        }
    }

    if (bFirstGamestart && bFirstInit) {
        // SDL3: Get displays and use display mode
        int numDisplays         = 0;
        SDL_DisplayID* displays = SDL_GetDisplays(&numDisplays);
        if (displays && currentDisplayIndex < numDisplays) {
            SDL_DisplayID displayID            = displays[currentDisplayIndex];
            const SDL_DisplayMode* displayMode = SDL_GetDesktopDisplayMode(displayID);

            if (displayMode) {
                const auto factor = getLogicalToPhysicalResolutionFactor(displayMode->w, displayMode->h);
                GUIStyle::getInstance().setZoom(static_cast<float>(factor));

                settings.video.physicalWidth  = displayMode->w;
                settings.video.physicalHeight = displayMode->h;
                settings.video.width          = displayMode->w;

                settings.video.height = displayMode->h;

                settings.video.preferredZoomLevel = 1;

                myINIFile.setIntValue("Video", "Width", settings.video.width);
                myINIFile.setIntValue("Video", "Height", settings.video.height);
                myINIFile.setIntValue("Video", "Physical Width", settings.video.physicalWidth);
                myINIFile.setIntValue("Video", "Physical Height", settings.video.physicalHeight);
                myINIFile.setIntValue("Video", "Preferred Zoom Level", 1);

                if (!myINIFile.saveChangesTo(getConfigFilepath())) {
                    sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION,
                                    "Unable to save configuration file {}",
                                    reinterpret_cast<const char*>(getConfigFilepath().u8string().c_str()));
                }
            }
        }
        SDL_free(displays);
    }

    Scaler::setDefaultScaler(Scaler::getScalerByName(settings.video.scaler));

    return bFirstGamestart;
}

static void printUsage() {
    fprintf(stderr, "Usage:\n\tdunelegacy [--showlog] [--fullscreen|--window] [--PlayerName=X] [--ServerPort=X]\n");
}

bool parseCommandLine(int argc, char* argv[], bool& bShowDebugLog) {
    bShowDebugLog = false;

    for (int i = 1; i < argc; i++) {
        std::string parameter(argv[i]);

        if (parameter == "--showlog") {
            bShowDebugLog = true;
        } else if ((parameter == "-f") || (parameter == "--fullscreen") || (parameter == "-w")
                   || (parameter == "--window") || (parameter.compare(0, 13, "--PlayerName=") == 0)
                   || (parameter.compare(0, 13, "--ServerPort=") == 0)) {
            // normal parameter for overwriting settings - handle later
        } else {
            printUsage();
            return false;
        }
    }

    return true;
}
