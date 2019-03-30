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

#include <globals.h>

#include <config.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/SFXManager.h>
#include <FileClasses/FontManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/INIFile.h>
#include <FileClasses/Palfile.h>
#include <FileClasses/music/DirectoryPlayer.h>
#include <FileClasses/music/ADLPlayer.h>
#include <FileClasses/music/XMIPlayer.h>

#include <GUI/GUIStyle.h>
#include <GUI/dune/DuneStyle.h>

#include <Menu/MainMenu.h>

#include <misc/fnkdat.h>
#include <misc/FileSystem.h>
#include <misc/Scaler.h>
#include <misc/string_util.h>
#include <misc/exceptions.h>
#include <misc/format.h>
#include <misc/SDL2pp.h>

#include <SoundPlayer.h>

#include <mmath.h>

#include <CutScenes/Intro.h>

#include <SDL_ttf.h>

#include <iostream>
#include <typeinfo>
#include <future>
#include <ctime>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>


#ifdef _WIN32
    #include <windows.h>
    #include <stdio.h>
    #include <io.h>
#else
    #include <sys/types.h>
    #include <pwd.h>
    #include <unistd.h>
#endif

#ifdef __APPLE__
    #include <MacFunctions.h>
#endif

#if !defined(__GNUG__) || (defined(_GLIBCXX_HAS_GTHREADS) && defined(_GLIBCXX_USE_C99_STDINT_TR1) && (ATOMIC_INT_LOCK_FREE > 1) && !defined(_GLIBCXX_HAS_GTHREADS))
// g++ does not provide std::async on all platforms
#define HAS_ASYNC
#endif

#if defined( __clang__ ) || defined(__GNUG__) || defined( __GLIBCXX__ ) || defined( __GLIBCPP__ )
#include <cxxabi.h>
inline std::string demangleSymbol(const char* symbolname) {
    int status = 0;
    std::size_t size = 0;
    char* result = abi::__cxa_demangle(symbolname, nullptr, &size, &status);
    if(status != 0) {
        return std::string(symbolname);
    } else {
        std::string name = std::string(result);
        std::free(result);
        return name;
    }
}
#else
inline std::string demangleSymbol(const char* symbolname) {
    return std::string(symbolname);
}
#endif

void setVideoMode(int displayIndex);
void realign_buttons();

static void printUsage() {
    fprintf(stderr, "Usage:\n\tdunelegacy [--showlog] [--fullscreen|--window] [--PlayerName=X] [--ServerPort=X]\n");
}

int getLogicalToPhysicalResolutionFactor(int physicalWidth, int physicalHeight) {
    if(physicalWidth >= 1280*3 && physicalHeight >= 720*3) {
        return 3;
    } else if(physicalWidth >= 640*2 && physicalHeight >= 480*2) {
        return 2;
    } else {
        return 1;
    }
}

void setVideoMode(int displayIndex)
{
    int videoFlags = 0;

    if(settings.video.fullscreen) {
        videoFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    SDL_DisplayMode targetDisplayMode = { 0, settings.video.physicalWidth, settings.video.physicalHeight, 0, nullptr};
    SDL_DisplayMode closestDisplayMode;

    if(SDL_GetClosestDisplayMode(displayIndex, &targetDisplayMode, &closestDisplayMode) == nullptr) {
        SDL_Log("Warning: Falling back to a display resolution of 640x480!");
        settings.video.physicalWidth = 640;
        settings.video.physicalHeight = 480;
        settings.video.width = 640;
        settings.video.height = 480;
    } else {
        settings.video.physicalWidth = closestDisplayMode.w;
        settings.video.physicalHeight = closestDisplayMode.h;
        int factor = getLogicalToPhysicalResolutionFactor(settings.video.physicalWidth, settings.video.physicalHeight);
        settings.video.width = settings.video.physicalWidth / factor;
        settings.video.height = settings.video.physicalHeight / factor;

    }

    window = SDL_CreateWindow("Dune Legacy",
                              SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex),
                              settings.video.physicalWidth, settings.video.physicalHeight,
                              videoFlags);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    SDL_RenderSetLogicalSize(renderer, settings.video.width, settings.video.height);
    screenTexture = SDL_CreateTexture(renderer, SCREEN_FORMAT, SDL_TEXTUREACCESS_TARGET, settings.video.width, settings.video.height);

    SDL_ShowCursor(SDL_DISABLE);
}

void toogleFullscreen()
{
    if(SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        // switch to windowed mode
        SDL_Log("Switching to windowed mode.");
        SDL_SetWindowFullscreen(window, (SDL_GetWindowFlags(window) ^ SDL_WINDOW_FULLSCREEN_DESKTOP));

        SDL_SetWindowSize(window, settings.video.physicalWidth, settings.video.physicalHeight);
        SDL_RenderSetLogicalSize(renderer, settings.video.width, settings.video.height);
    } else {
        // switch to fullscreen mode
        SDL_Log("Switching to fullscreen mode.");
        SDL_DisplayMode displayMode;
        SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(window), &displayMode);

        SDL_SetWindowFullscreen(window, (SDL_GetWindowFlags(window) ^ SDL_WINDOW_FULLSCREEN_DESKTOP));

        SDL_SetWindowSize(window, displayMode.w, displayMode.h);
        SDL_RenderSetLogicalSize(renderer, settings.video.width, settings.video.height);
    }

    // we just need to flush all events; otherwise we might get them twice
    SDL_PumpEvents();
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

    // wait a bit to avoid immediately switching back
    SDL_Delay(100);
}

std::string getConfigFilepath()
{
    // determine path to config file
    char tmp[FILENAME_MAX];
    fnkdat(CONFIGFILENAME, tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);

    return std::string(tmp);
}

std::string getLogFilepath()
{
    // determine path to config file
    char tmp[FILENAME_MAX];
    if(fnkdat(LOGFILENAME, tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT) < 0) {
        THROW(std::runtime_error, "fnkdat() failed!");
    }

    return std::string(tmp);
}

void createDefaultConfigFile(const std::string& configfilepath, const std::string& language) {
    SDL_Log("Creating config file '%s'", configfilepath.c_str());


    auto file = sdl2::RWops_ptr{ SDL_RWFromFile(configfilepath.c_str(), "w") };
    if(!file) {
        THROW(sdl_error, "Opening config file failed: %s!", SDL_GetError());
    }

    const char configfile[] =   "[General]\n"
                                "Play Intro = false          # Play the intro when starting the game?\n"
                                "Player Name = %s            # The name of the player\n"
                                "Language = %s               # en = English, fr = French, de = German\n"
                                "Scroll Speed = 50           # Amount to scroll the map when the cursor is near the screen border\n"
                                "Show Tutorial Hints = true  # Show tutorial hints during the game\n"
                                "\n"
                                "[Video]\n"
                                "# Minimum resolution is 640x480\n"
                                "Width = 640\n"
                                "Height = 480\n"
                                "Physical Width = 640\n"
                                "Physical Height = 480\n"
                                "Fullscreen = true\n"
                                "FrameLimit = true           # Limit the frame rate to save energy?\n"
                                "Preferred Zoom Level = 1    # 0 = no zooming, 1 = 2x, 2 = 3x\n"
                                "Scaler = ScaleHD            # Scaler to use: ScaleHD = apply manual drawn mask to upscale, Scale2x = smooth edges, ScaleNN = nearest neighbour, \n"
                                "RotateUnitGraphics = false  # Freely rotate unit graphics, e.g. carryall graphics\n"
                                "\n"
                                "[Audio]\n"
                                "# There are three different possibilities to play music\n"
                                "#  adl       - This option will use the Dune 2 music as used on e.g. SoundBlaster16 cards\n"
                                "#  xmi       - This option plays the xmi files of Dune 2. Sounds more midi-like\n"
                                "#  directory - Plays music from the \"music\"-directory inside your configuration directory\n"
                                "#              The \"music\"-directory should contain 5 subdirectories named attack, intro, peace, win and lose\n"
                                "#              Put any mp3, ogg or mid file there and it will be played in the particular situation\n"
                                "Music Type = adl\n"
                                "Play Music = true\n"
                                "Music Volume = 64           # Volume between 0 and 128\n"
                                "Play SFX = true\n"
                                "SFX Volume = 64             # Volume between 0 and 128\n"
                                "\n"
                                "[Network]\n"
                                "ServerPort = %d\n"
                                "MetaServer = %s\n"
                                "\n"
                                "[AI]\n"
                                "Campaign AI = qBotMedium\n"
                                "\n"
                                "[Game Options]\n"
                                "Game Speed = 16                         # The default speed of the game: 32 = very slow, 8 = very fast, 16 = default\n"
                                "Concrete Required = true                # If true building on bare rock will result in 50%% structure health penalty\n"
                                "Structures Degrade On Concrete = true   # If true structures will degrade on power shortage even if built on concrete\n"
                                "Fog of War = false                      # If true explored terrain will become foggy when no unit or structure is next to it\n"
                                "Start with Explored Map = false         # If true the complete map is unhidden at the beginning of the game\n"
                                "Instant Build = false                   # If true the building of structures and units does not take any time\n"
                                "Only One Palace = false                 # If true, only one palace can be build per house\n"
                                "Rocket-Turrets Need Power = false       # If true, rocket turrets are dysfunctional on power shortage\n"
                                "Sandworms Respawn = false               # If true, killed sandworms respawn after some time\n"
                                "Killed Sandworms Drop Spice = false     # If true, killed sandworms drop some spice\n"
                                "Manual Carryall Drops = false           # If true, player can request carryall to transport units\n"
                                "Maximum Number of Units Override = -1   # Override the maximum number of units each house is allowed to build (-1 = do not override)\n";

    char playername[MAX_PLAYERNAMELENGHT+1] = "Player";

#ifdef _WIN32
    DWORD playernameLength = MAX_PLAYERNAMELENGHT+1;
    GetUserName(playername, &playernameLength);
#else
    struct passwd* pwent = getpwuid(getuid());

    if(pwent != nullptr) {
        strncpy(playername, pwent->pw_name, MAX_PLAYERNAMELENGHT + 1);
        playername[MAX_PLAYERNAMELENGHT] = '\0';
    }
#endif

    playername[0] = toupper(playername[0]);

    // replace player name, language, server port and metaserver
    std::string strConfigfile = fmt::sprintf(configfile, playername, language, DEFAULT_PORT, DEFAULT_METASERVER);

    if(SDL_RWwrite(file.get(), strConfigfile.c_str(), 1, strConfigfile.length()) == 0) {
        THROW(sdl_error, "Writing config file failed: %s!", SDL_GetError());
    }
}

void logOutputFunction(void *userdata, int category, SDL_LogPriority priority, const char *message) {
    /*
    static const char* priorityStrings[] = {
        nullptr,
        "VERBOSE ",
        "DEBUG   ",
        "INFO    ",
        "WARN    ",
        "ERROR   ",
        "CRITICAL"
    };
    fprintf(stderr, "%s:   %s\n", priorityStrings[priority], message);
    */
    fprintf(stderr, "%s\n", message);
    fflush(stderr);
}

void showMissingFilesMessageBox() {
    SDL_ShowCursor(SDL_ENABLE);

    std::string instruction = "Dune Legacy uses the data files from original Dune II. The following files are missing:\n";

    for(const std::string& missingFile : FileManager::getMissingFiles()) {
        instruction += " " + missingFile + "\n";
    }

    instruction += "\nPut them in one of the following directories and restart Dune Legacy:\n";
    for(const std::string& searchPath : FileManager::getSearchPath()) {
        instruction += " " + searchPath + "\n";
    }

    instruction += "\nYou may want to add GERMAN.PAK or FRENCH.PAK for playing in these languages.";

    if(!SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Dune Legacy", instruction.c_str(), nullptr)) {
        fprintf(stderr, "%s\n", instruction.c_str());
    }
}

std::string getUserLanguage() {
    const char* pLang = nullptr;

#ifdef _WIN32
    char ISO639_LanguageName[10];
    if(GetLocaleInfo(GetUserDefaultLCID(), LOCALE_SISO639LANGNAME, ISO639_LanguageName, sizeof(ISO639_LanguageName)) == 0) {
        return "";
    } else {

        pLang = ISO639_LanguageName;
    }

#elif defined (__APPLE__)
    pLang = getMacLanguage();
    if(pLang == nullptr) {
        return "";
    }

#else
    // should work on most unices
    pLang = getenv("LC_ALL");
    if(pLang == nullptr) {
        // try LANG
        pLang = getenv("LANG");
        if(pLang == nullptr) {
            return "";
        }
    }
#endif

    SDL_Log("User locale is '%s'", pLang);

    if(strlen(pLang) < 2) {
        return "";
    } else {
        return strToLower(std::string(pLang, 2));
    }
}



int main(int argc, char *argv[]) {
    SDL_LogSetOutputFunction(logOutputFunction, nullptr);
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);

    // global try/catch around everything
    try {

        // init fnkdat
        if(fnkdat(nullptr, nullptr, 0, FNKDAT_INIT) < 0) {
            THROW(std::runtime_error, "Cannot initialize fnkdat!");
        }

        bool bShowDebugLog = false;
        for(int i=1; i < argc; i++) {
            //check for overiding params
            std::string parameter(argv[i]);

            if(parameter == "--showlog") {
                // special parameter which does not overwrite settings
                bShowDebugLog = true;
            } else if((parameter == "-f") || (parameter == "--fullscreen") || (parameter == "-w") || (parameter == "--window") || (parameter.compare(0, 13, "--PlayerName=") == 0) || (parameter.compare(0, 13, "--ServerPort=") == 0)) {
                // normal parameter for overwriting settings
                // handle later
            } else {
                printUsage();
                exit(EXIT_FAILURE);
            }
        }

        if(bShowDebugLog == false) {
            // get utf8-encoded log file path
            std::string logfilePath = getLogFilepath();
            char* pLogfilePath = (char*) logfilePath.c_str();

            #ifdef _WIN32

            // on win32 we need an ansi-encoded filepath
            WCHAR szwLogPath[MAX_PATH];
            char szLogPath[MAX_PATH];

            if(MultiByteToWideChar(CP_UTF8, 0, pLogfilePath, -1, szwLogPath, MAX_PATH) == 0) {
                THROW(std::runtime_error, "Conversion of logfile path from utf-8 to utf-16 failed!");
            }

            if(WideCharToMultiByte(CP_ACP, 0, szwLogPath, -1, szLogPath, MAX_PATH, nullptr, nullptr) == 0) {
                THROW(std::runtime_error, "Conversion of logfile path from utf-16 to ansi failed!");
            }

            pLogfilePath = szLogPath;

            if(freopen(pLogfilePath, "w", stdout) == NULL) {
                THROW(io_error, "Reopening logfile '%s' as stdout failed!", pLogfilePath);
            }
            setbuf(stdout, nullptr);   // No buffering

            if(freopen(pLogfilePath, "w", stderr) == NULL) {
                // use stdout in this error case as stderr is not yet ready
                THROW(io_error, "Reopening logfile '%s' as stderr failed!", pLogfilePath);
            }
            setbuf(stderr, nullptr);   // No buffering

            if(dup2(fileno(stdout), fileno(stderr)) < 0) {
                THROW(io_error, "Redirecting stderr to stdout failed!");
            }

            #else

            int d = open(pLogfilePath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(d < 0) {
                THROW(io_error, "Opening logfile '%s' failed!", pLogfilePath);
            }
            // Hint: fileno(stdout) != STDOUT_FILENO on Win32
            if(dup2(d, fileno(stdout)) < 0) {
                THROW(io_error, "Redirecting stdout failed!");
            }

            // Hint: fileno(stderr) != STDERR_FILENO on Win32
            if(dup2(d, fileno(stderr)) < 0) {
                THROW(io_error, "Redirecting stderr failed!");
            }

            #endif
        }

        SDL_Log("Starting Dune Legacy %s on %s", VERSION, SDL_GetPlatform());

        // First check for missing files
        std::vector<std::string> missingFiles = FileManager::getMissingFiles();

        if(!missingFiles.empty()) {
            // create data directory inside config directory
            char tmp[FILENAME_MAX];
            fnkdat("data/", tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);

            showMissingFilesMessageBox();

            return EXIT_FAILURE;
        }

        bool bExitGame = false;
        bool bFirstInit = true;
        bool bFirstGamestart = false;

        debug = false;
        cursorFrame = UI_CursorNormal;

        int currentDisplayIndex = SCREEN_DEFAULT_DISPLAYINDEX;

        do {
            // we do not use rand() but maybe some library does; thus we shall initialize it
            unsigned int seed = (unsigned int) time(nullptr);
            srand(seed);

            // check if configfile exists
            std::string configfilepath = getConfigFilepath();
            if(existsFile(configfilepath) == false) {
                std::string userLanguage = getUserLanguage();
                if(userLanguage.empty()) {
                    userLanguage = "en";
                }

                bFirstGamestart = true;
                createDefaultConfigFile(configfilepath, userLanguage);
            }

            INIFile myINIFile(configfilepath);

            settings.general.playIntro = myINIFile.getBoolValue("General","Play Intro",false);
            settings.general.playerName = myINIFile.getStringValue("General","Player Name","Player");
            settings.general.language = myINIFile.getStringValue("General","Language","en");
            settings.general.scrollSpeed = myINIFile.getIntValue("General","Scroll Speed",50);
            settings.general.showTutorialHints = myINIFile.getBoolValue("General","Show Tutorial Hints",true);
            settings.video.width = myINIFile.getIntValue("Video","Width",640);
            settings.video.height = myINIFile.getIntValue("Video","Height",480);
            settings.video.physicalWidth= myINIFile.getIntValue("Video","Physical Width",640);
            settings.video.physicalHeight = myINIFile.getIntValue("Video","Physical Height",480);
            settings.video.fullscreen = myINIFile.getBoolValue("Video","Fullscreen",false);
            settings.video.frameLimit = myINIFile.getBoolValue("Video","FrameLimit",true);
            settings.video.preferredZoomLevel = myINIFile.getIntValue("Video","Preferred Zoom Level", 0);
            settings.video.scaler = myINIFile.getStringValue("Video","Scaler","ScaleHD");
            settings.video.rotateUnitGraphics = myINIFile.getBoolValue("Video","RotateUnitGraphics",false);
            settings.audio.musicType = myINIFile.getStringValue("Audio","Music Type","adl");
            settings.audio.playMusic = myINIFile.getBoolValue("Audio","Play Music", true);
            settings.audio.musicVolume = myINIFile.getIntValue("Audio","Music Volume", 64);
            settings.audio.playSFX = myINIFile.getBoolValue("Audio","Play SFX", true);
            settings.audio.sfxVolume = myINIFile.getIntValue("Audio","SFX Volume", 64);

            settings.network.serverPort = myINIFile.getIntValue("Network","ServerPort",DEFAULT_PORT);
            settings.network.metaServer = myINIFile.getStringValue("Network","MetaServer",DEFAULT_METASERVER);
            settings.network.debugNetwork = myINIFile.getBoolValue("Network","Debug Network",false);

            settings.ai.campaignAI = myINIFile.getStringValue("AI","Campaign AI",DEFAULTAIPLAYERCLASS);

            settings.gameOptions.gameSpeed = myINIFile.getIntValue("Game Options","Game Speed",GAMESPEED_DEFAULT);
            settings.gameOptions.concreteRequired = myINIFile.getBoolValue("Game Options","Concrete Required",true);
            settings.gameOptions.structuresDegradeOnConcrete = myINIFile.getBoolValue("Game Options","Structures Degrade On Concrete",true);
            settings.gameOptions.fogOfWar = myINIFile.getBoolValue("Game Options","Fog of War",false);
            settings.gameOptions.startWithExploredMap = myINIFile.getBoolValue("Game Options","Start with Explored Map",false);
            settings.gameOptions.instantBuild = myINIFile.getBoolValue("Game Options","Instant Build",false);
            settings.gameOptions.onlyOnePalace = myINIFile.getBoolValue("Game Options","Only One Palace",false);
            settings.gameOptions.rocketTurretsNeedPower = myINIFile.getBoolValue("Game Options","Rocket-Turrets Need Power",false);
            settings.gameOptions.sandwormsRespawn = myINIFile.getBoolValue("Game Options","Sandworms Respawn",false);
            settings.gameOptions.killedSandwormsDropSpice = myINIFile.getBoolValue("Game Options","Killed Sandworms Drop Spice",false);
            settings.gameOptions.manualCarryallDrops = myINIFile.getBoolValue("Game Options","Manual Carryall Drops",false);
            settings.gameOptions.maximumNumberOfUnitsOverride = myINIFile.getIntValue("Game Options","Maximum Number of Units Override",-1);

            pTextManager = std::make_unique<TextManager>();

            missingFiles = FileManager::getMissingFiles();
            if(!missingFiles.empty()) {
                // set back to English
                std::string setBackToEnglishWarning = fmt::sprintf("The following files are missing for language \"%s\":\n",_("LanguageFileExtension"));
                for(const std::string& filename : missingFiles) {
                    setBackToEnglishWarning += filename + "\n";
                }
                setBackToEnglishWarning += "\nLanguage is changed to English!";
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Dune Legacy", setBackToEnglishWarning.c_str(), NULL);

                SDL_Log("Warning: Language is changed to English!");

                settings.general.language = "en";
                myINIFile.setStringValue("General","Language",settings.general.language);
                myINIFile.saveChangesTo(configfilepath);

                // reinit text manager
                pTextManager = std::make_unique<TextManager>();
            }

            for(int i=1; i < argc; i++) {
                //check for overiding params
                std::string parameter(argv[i]);

                if((parameter == "-f") || (parameter == "--fullscreen")) {
                    settings.video.fullscreen = true;
                } else if((parameter == "-w") || (parameter == "--window")) {
                    settings.video.fullscreen = false;
                } else if(parameter.compare(0, 13, "--PlayerName=") == 0) {
                    settings.general.playerName = parameter.substr(strlen("--PlayerName="));
                } else if(parameter.compare(0, 13, "--ServerPort=") == 0) {
                    settings.network.serverPort = atol(argv[i] + strlen("--ServerPort="));
                }
            }

            if(bFirstInit == true) {
                SDL_Log("Initializing SDL...");

                if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
                    THROW(sdl_error, "Couldn't initialize SDL: %s!", SDL_GetError());
                }

                SDL_version compiledVersion;
                SDL_version linkedVersion;
                SDL_VERSION(&compiledVersion);
                SDL_GetVersion(&linkedVersion);
                SDL_Log("SDL runtime v%d.%d.%d", linkedVersion.major, linkedVersion.minor, linkedVersion.patch);
                SDL_Log("SDL compile-time v%d.%d.%d", compiledVersion.major, compiledVersion.minor, compiledVersion.patch);

                if(TTF_Init() < 0) {
                    THROW(sdl_error, "Couldn't initialize SDL2_ttf: %s!", TTF_GetError());
                }

                SDL_version TTFCompiledVersion;
                SDL_TTF_VERSION(&TTFCompiledVersion);
                const SDL_version* pTTFLinkedVersion = TTF_Linked_Version();
                SDL_Log("SDL2_ttf runtime v%d.%d.%d", pTTFLinkedVersion->major, pTTFLinkedVersion->minor, pTTFLinkedVersion->patch);
                SDL_Log("SDL2_ttf compile-time v%d.%d.%d", TTFCompiledVersion.major, TTFCompiledVersion.minor, TTFCompiledVersion.patch);
            }

            if(bFirstGamestart == true && bFirstInit == true) {
                SDL_DisplayMode displayMode;
                SDL_GetDesktopDisplayMode(currentDisplayIndex, &displayMode);

                int factor = getLogicalToPhysicalResolutionFactor(displayMode.w, displayMode.h);
                settings.video.physicalWidth = displayMode.w;
                settings.video.physicalHeight = displayMode.h;
                settings.video.width = displayMode.w / factor;
                settings.video.height = displayMode.h / factor;
                settings.video.preferredZoomLevel = 1;

                myINIFile.setIntValue("Video","Width",settings.video.width);
                myINIFile.setIntValue("Video","Height",settings.video.height);
                myINIFile.setIntValue("Video","Physical Width",settings.video.physicalWidth);
                myINIFile.setIntValue("Video","Physical Height",settings.video.physicalHeight);
                myINIFile.setIntValue("Video","Preferred Zoom Level",1);

                myINIFile.saveChangesTo(getConfigFilepath());
            }

            Scaler::setDefaultScaler(Scaler::getScalerByName(settings.video.scaler));

            if(bFirstInit == true) {
                SDL_Log("Initializing audio...");
                if( Mix_OpenAudio(AUDIO_FREQUENCY, AUDIO_S16SYS, 2, 1024) < 0 ) {
                    SDL_Quit();
                    THROW(sdl_error, "Couldn't set %d Hz 16-bit audio. Reason: %s!", AUDIO_FREQUENCY, SDL_GetError());
                } else {
                    SDL_Log("%d audio channels were allocated.", Mix_AllocateChannels(28));
                }
            }

            pFileManager = std::make_unique<FileManager>();

            // now we can finish loading texts
            pTextManager->loadData();

            palette = LoadPalette_RW(pFileManager->openFile("IBM.PAL").get());

            SDL_Log("Setting video mode...");
            setVideoMode(currentDisplayIndex);
            SDL_RendererInfo rendererInfo;
            SDL_GetRendererInfo(renderer, &rendererInfo);
            SDL_Log("Renderer: %s (max texture size: %dx%d)", rendererInfo.name, rendererInfo.max_texture_width, rendererInfo.max_texture_height);


            SDL_Log("Loading fonts...");
            pFontManager = std::make_unique<FontManager>();

            SDL_Log("Loading graphics and sounds...");

#ifdef HAS_ASYNC
            auto gfxManagerFut = std::async(std::launch::async, []() { return std::make_unique<GFXManager>(); } );
            auto sfxManagerFut = std::async(std::launch::async, []() { return std::make_unique<SFXManager>(); } );

            pGFXManager = gfxManagerFut.get();
            pSFXManager = sfxManagerFut.get();
#else
            // g++ does not provide std::launch::async on all platforms
            pGFXManager = std::make_unique<GFXManager>();
            pSFXManager = std::make_unique<SFXManager>();
#endif

            GUIStyle::setGUIStyle(std::make_unique<DuneStyle>());

            if(bFirstInit == true) {
                SDL_Log("Starting sound player...");
                soundPlayer = std::make_unique<SoundPlayer>();

                if(settings.audio.musicType == "directory") {
                    SDL_Log("Starting directory music player...");
                    musicPlayer = std::make_unique<DirectoryPlayer>();
                } else if(settings.audio.musicType == "adl") {
                    SDL_Log("Starting ADL music player...");
                    musicPlayer = std::make_unique<ADLPlayer>();
                } else if(settings.audio.musicType == "xmi") {
                    SDL_Log("Starting XMI music player...");
                    musicPlayer = std::make_unique<XMIPlayer>();
                } else {
                    THROW(std::runtime_error, "Invalid music type: '%'", settings.audio.musicType);
                }

                //musicPlayer->changeMusic(MUSIC_INTRO);
            }

            // Playing intro
            if(((bFirstGamestart == true) || (settings.general.playIntro == true)) && (bFirstInit==true)) {
                SDL_Log("Playing intro...");
                Intro().run();
            }

            bFirstInit = false;

            SDL_Log("Starting main menu...");
            { // Scope
                if (MainMenu().showMenu() == MENU_QUIT_DEFAULT) {
                    bExitGame = true;
                }
            }

            SDL_Log("Deinitialize...");

            GUIStyle::destroyGUIStyle();

            // clear everything
            if(bExitGame == true) {
                musicPlayer.reset();
                soundPlayer.reset();
                Mix_HaltMusic();
                Mix_CloseAudio();
            } else {
                // save the current display index for later reuse
                currentDisplayIndex = SDL_GetWindowDisplayIndex(window);
            }

            pTextManager.reset();
            pSFXManager.reset();
            pGFXManager.reset();
            pFontManager.reset();
            pFileManager.reset();

            SDL_DestroyTexture(screenTexture);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);

            if(bExitGame == true) {
                TTF_Quit();
                SDL_Quit();
            }
            SDL_Log("Deinitialization finished!");
        } while(bExitGame == false);

        // deinit fnkdat
        if(fnkdat(nullptr, nullptr, 0, FNKDAT_UNINIT) < 0) {
            THROW(std::runtime_error, "Cannot uninitialize fnkdat!");
        }
    } catch(const std::exception& e) {
        std::string message = std::string("An unhandled exception of type \'") + demangleSymbol(typeid(e).name()) + std::string("\' was thrown:\n\n") + e.what() + std::string("\n\nDune Legacy will now be terminated!");
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Dune Legacy: Unrecoverable error", message.c_str(), nullptr);

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
