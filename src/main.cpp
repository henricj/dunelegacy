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

#include <SoundPlayer.h>

#include <mmath.h>

#include <CutScenes/Intro.h>

#include <SDL.h>
#include <SDL_rwops.h>
#include <iostream>
#include <future>
#include <stdexcept>
#include <ctime>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>


#ifdef _WIN32
    #include <windows.h>
    #include <stdio.h>
    #include <io.h>
    extern "C" {
		__declspec(dllimport) int _fileno(FILE*);
    }
    #define fileno _fileno
    #define dup2 _dup2
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

void setVideoMode();
void realign_buttons();

static void printUsage() {
    fprintf(stderr, "Usage:\n\tdunelegacy [--showlog] [--fullscreen|--window] [--PlayerName=X] [--ServerPort=X]\n");
}

void setVideoMode()
{
    int videoFlags = 0;

    if(settings.video.fullscreen) {
        videoFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    SDL_DisplayMode targetDisplayMode = { 0, settings.video.width, settings.video.height, 0, nullptr};
    SDL_DisplayMode closestDisplayMode;

    if(SDL_GetClosestDisplayMode(SCREEN_DISPLAYINDEX, &targetDisplayMode, &closestDisplayMode) == nullptr) {
        fprintf(stderr, "WARNING: Falling back to 640x480!\n");
        settings.video.width = 640;
        settings.video.height = 480;
    } else {
        settings.video.width = closestDisplayMode.w;
        settings.video.height = closestDisplayMode.h;
    }

    window = SDL_CreateWindow("Dune Legacy",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              //settings.video.width, settings.video.height,
                              settings.video.width, settings.video.height,
                              videoFlags);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    SDL_RenderSetLogicalSize(renderer, settings.video.width, settings.video.height);
    screenTexture = SDL_CreateTexture(renderer, SCREEN_FORMAT, SDL_TEXTUREACCESS_TARGET, settings.video.width, settings.video.height);

    SDL_ShowCursor(SDL_DISABLE);
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
        fprintf(stderr, "getLogFilepath(): fnkdat failed!\n");
        return "";
    }

    return std::string(tmp);
}

void createDefaultConfigFile(std::string configfilepath, std::string language) {
    fprintf(stdout,"Creating config file...\t\t"); fflush(stdout);


    SDL_RWops* file = SDL_RWFromFile(configfilepath.c_str(), "w");
    if(file == nullptr) {
        fprintf(stderr,"Failed to open config file: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    const char configfile[] =   "[General]\n"
                                "Play Intro = false\t\t\t# Play the intro when starting the game?\n"
                                "Player Name = %s\t\t\t# The name of the player\n"
                                "Language = %s\t\t\t\t# en = English, fr = French, de = German\n"
                                "Scroll Speed = 50\t\t\t\t# Amount to scroll the map when the cursor is near the screen border\n"
                                "\n"
                                "[Video]\n"
                                "# You may decide to use half the resolution of your monitor, e.g. monitor has 1600x1200 => 800x600\n"
                                "# Minimum resolution is 640x480\n"
                                "Width = 640\n"
                                "Height = 480\n"
                                "Fullscreen = false\n"
                                "FrameLimit = true\t\t\t# Limit the frame rate to save energy?\n"
                                "Preferred Zoom Level = 0\t\t# 0 = no zooming, 1 = 2x, 2 = 3x\n"
                                "Scaler = ScaleHQ\t\t\t# Scaler to use: ScaleHD = apply manual drawn mask to upscale, Scale2x = smooth edges, ScaleNN = nearest neighbour, \n"
                                "\n"
                                "[Audio]\n"
                                "# There are three different possibilities to play music\n"
                                "#  adl\t\t- This option will use the Dune 2 music as used on e.g. SoundBlaster16 cards\n"
                                "#  xmi\t\t- This option plays the xmi files of Dune 2. Sounds more midi-like\n"
                                "#  directory\t- Plays music from the \"music\"-directory inside your configuration directory\n"
                                "#\t\t  The \"music\"-directory should contain 5 subdirectories named attack, intro, peace, win and lose\n"
                                "#\t\t  Put any mp3, ogg or mid file there and it will be played in the particular situation\n"
                                "Music Type = adl\n"
                                "Play Music = true\n"
                                "Music Volume = 64\t\t\t\t# Volume between 0 and 128\n"
                                "Play SFX = true\n"
                                "SFX Volume = 64\t\t\t\t# Volume between 0 and 128\n"
                                "\n"
                                "[Network]\n"
                                "ServerPort = %d\n"
                                "MetaServer = %s\n"
                                "\n"
                                "[AI]\n"
                                "Campaign AI = qBotEasy"
                                "\n"
                                "[Game Options]\n"
                                "Game Speed = 16\t\t\t\t# The default speed of the game: 32 = very slow, 8 = very fast, 16 = default\n"
                                "Concrete Required = true\t\t# If true building on bare rock will result in 50%% structure health penalty\n"
                                "Structures Degrade On Concrete = true\t# If true structures will degrade on power shortage even if built on concrete\n"
                                "Fog of War = false\t\t\t# If true explored terrain will become foggy when no unit or structure is next to it\n"
                                "Start with Explored Map = false\t\t# If true the complete map is unhidden at the beginning of the game\n"
                                "Instant Build = false\t\t\t#If true the building of structures and units does not take any time\n"
                                "Only One Palace = false \t\t\t#If true, only one palace can be build per house\n"
                                "Rocket-Turrets Need Power = false \t\t\t#If true, rocket turrets are dysfunctional on power shortage\n"
                                "Sandworms Respawn = false\t\t\t\t#If true, killed sandworms respawn after some time\n"
                                "Killed Sandworms Drop Spice = false \t\t\t#If true, killed sandworms drop some spice\n"
                                "Manual Carryall Drops = false \t\t\t#If true, player can request carryall to transport units\n";

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
    std::string strConfigfile = strprintf(configfile, playername, language.c_str(), DEFAULT_PORT, DEFAULT_METASERVER);

    if(SDL_RWwrite(file, strConfigfile.c_str(), 1, strConfigfile.length()) == 0) {
        fprintf(stderr,"Failed to write to config file: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_RWclose(file);

    fprintf(stdout,"finished\n"); fflush(stdout);
}

void logOutputFunction(void *userdata, int category, SDL_LogPriority priority, const char *message) {
    static const char* priorityStrings[] = {
        nullptr,
        "VERBOSE",
        "DEBUG",
        "INFO",
        "WARN",
        "ERROR",
        "CRITICAL"
    };
    fprintf(stderr, "%s: %s\n", priorityStrings[priority], message);
    fflush(stderr);
}

void printMissingFilesToScreen() {
    SDL_ShowCursor(SDL_ENABLE);

    std::string instruction = "Dune Legacy uses the data files from original Dune II. The following files are missing:\n";

    std::vector<std::string> MissingFiles = FileManager::getMissingFiles();

    std::vector<std::string>::const_iterator iter;
    for(iter = MissingFiles.begin(); iter != MissingFiles.end(); ++iter) {
        instruction += " " + *iter + "\n";
    }

    instruction += "\nPut them in one of the following directories:\n";
    std::vector<std::string> searchPath = FileManager::getSearchPath();
    std::vector<std::string>::const_iterator searchPathIter;
    for(searchPathIter = searchPath.begin(); searchPathIter != searchPath.end(); ++searchPathIter) {
        instruction += " " + *searchPathIter + "\n";
    }

    instruction += "\nYou may want to add GERMAN.PAK or FRENCH.PAK for playing in these languages.\n";
    instruction += "\n\nPress ESC to exit.";

    SDL_Texture* pTextTexture = pFontManager->createTextureWithMultilineText(instruction, COLOR_BLACK, FONT_STD12);

    SDL_Event   event;
    bool quiting = false;
    while(!quiting) {
        SDL_Delay(20);

        setRenderDrawColor(renderer, DuneStyle::buttonBackgroundColor);
        SDL_RenderClear(renderer);

        SDL_Rect dest = calcDrawingRect(pTextTexture, 30, 30);
        SDL_RenderCopy(renderer, pTextTexture, nullptr, &dest);

        SDL_RenderPresent(renderer);

        while(SDL_PollEvent(&event)) {
            //check the events
            switch (event.type)
            {
                case (SDL_KEYDOWN): // Look for a keypress
                {
                    switch(event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            quiting = true;
                            break;

                        default:
                            break;
                    }
                } break;


                case SDL_QUIT:
                    quiting = true;
                    break;

                default:
                    break;
            }
        }
    }

    SDL_DestroyTexture(pTextTexture);
}

std::string getUserLanguage() {
    const char* pLang = nullptr;

    fprintf(stdout,"Detecting locale...\t\t"); fflush(stdout);

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

    fprintf(stderr, "'%s'\n", pLang); fflush(stdout);

    if(strlen(pLang) < 2) {
        return "";
    } else {
        return strToLower(std::string(pLang, 2));
    }
}



int main(int argc, char *argv[]) {
    SDL_LogSetOutputFunction(logOutputFunction, nullptr);
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);

    // init fnkdat
    if(fnkdat(nullptr, nullptr, 0, FNKDAT_INIT) < 0) {
      perror("Could not initialize fnkdat");
      exit(EXIT_FAILURE);
    }

    bool bShowDebugLog = false;
    for(int i=1; i < argc; i++) {
        //check for overiding params
        std::string parameter(argv[i]);

        if(parameter == "--showlog") {
            // special parameter which does not overwrite settings
            bShowDebugLog = true;
        } else if((parameter == "-f") || (parameter == "--fullscreen") || (parameter == "-w") || (parameter == "--window") || (parameter.find("--PlayerName=") == 0) || (parameter.find("--ServerPort=") == 0)) {
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
            fprintf(stderr, "Conversion of logfile path from utf-8 to utf-16 failed\n");
            exit(EXIT_FAILURE);
        }

        if(WideCharToMultiByte(CP_ACP, 0, szwLogPath, -1, szLogPath, MAX_PATH, nullptr, nullptr) == 0) {
            fprintf(stderr, "Conversion of logfile path from utf-16 to ansi failed\n");
            exit(EXIT_FAILURE);
        }

        pLogfilePath = szLogPath;

        if(freopen(pLogfilePath, "w", stdout) == NULL) {
            fprintf(stderr, "Reopening logfile '%s' as stdout failed\n", pLogfilePath);
            exit(EXIT_FAILURE);
        }
        setbuf(stdout, nullptr);   // No buffering

        if(freopen(pLogfilePath, "w", stderr) == NULL) {
            // use stdout in this error case as stderr is not yet ready
            fprintf(stdout, "Reopening logfile '%s' as stderr failed\n", pLogfilePath);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        setbuf(stderr, nullptr);   // No buffering

        if(dup2(fileno(stdout), fileno(stderr)) < 0) {
            fprintf(stderr, "Redirecting stderr to stdout failed\n");
            exit(EXIT_FAILURE);
        }

        #else

        int d = open(pLogfilePath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if(d < 0) {
            fprintf(stderr, "Opening logfile '%s' failed\n", pLogfilePath);
            exit(EXIT_FAILURE);
        }
        // Hint: fileno(stdout) != STDOUT_FILENO on Win32
        if(dup2(d, fileno(stdout)) < 0) {
            fprintf(stderr, "Redirecting stdout failed\n");
            exit(EXIT_FAILURE);
        }

        // Hint: fileno(stderr) != STDERR_FILENO on Win32
        if(dup2(d, fileno(stderr)) < 0) {
            fprintf(stderr, "Redirecting stderr failed\n");
            exit(EXIT_FAILURE);
        }

        #endif
    }

    fprintf(stdout, "Starting Dune Legacy %s on %s...\n", VERSION, SDL_GetPlatform()); fflush(stdout);

    // First check for missing files
    std::vector<std::string> missingFiles = FileManager::getMissingFiles();

    if(missingFiles.empty() == false) {
        // create data directory inside config directory
        char tmp[FILENAME_MAX];
        fnkdat("data/", tmp, FILENAME_MAX, FNKDAT_USER | FNKDAT_CREAT);

        bool cannotShowMissingScreen = false;
        fprintf(stderr,"The following files are missing:\n");
        std::vector<std::string>::const_iterator iter;
        for(iter = missingFiles.begin() ; iter != missingFiles.end(); ++iter) {
            fprintf(stderr," %s\n",iter->c_str());
            if(iter->find("LEGACY.PAK") != std::string::npos) {
                cannotShowMissingScreen = true;
            }
        }

        fprintf(stderr,"Put them in one of the following directories:\n");
        std::vector<std::string> searchPath = FileManager::getSearchPath();
        std::vector<std::string>::const_iterator searchPathIter;
        for(searchPathIter = searchPath.begin(); searchPathIter != searchPath.end(); ++searchPathIter) {
            fprintf(stderr," %s\n",searchPathIter->c_str());
        }

        if(cannotShowMissingScreen == true) {
            return EXIT_FAILURE;
        }
    }

    bool bExitGame = false;
    bool bFirstInit = true;
    bool bFirstGamestart = false;

    debug = false;
    cursorFrame = UI_CursorNormal;

    do {
        unsigned int seed = (unsigned int) time(nullptr);
        srand(seed);

        // check if configfile exists
        std::string configfilepath = getConfigFilepath();
        if(existsFile(configfilepath) == false) {
            std::string userLanguage = getUserLanguage();
            if(userLanguage.empty()) {
                userLanguage = "en";
            }

            if(missingFiles.empty() == true) {
                // if all pak files were found we can create the ini file
                bFirstGamestart = true;
                createDefaultConfigFile(configfilepath, userLanguage);
            }
        }

        INIFile myINIFile(configfilepath);

        settings.general.playIntro = myINIFile.getBoolValue("General","Play Intro",false);
        settings.general.playerName = myINIFile.getStringValue("General","Player Name","Player");
        settings.general.language = myINIFile.getStringValue("General","Language","en");
        settings.general.scrollSpeed = myINIFile.getIntValue("General","Scroll Speed",50);
        settings.video.width = myINIFile.getIntValue("Video","Width",640);
        settings.video.height = myINIFile.getIntValue("Video","Height",480);
        settings.video.fullscreen = myINIFile.getBoolValue("Video","Fullscreen",false);
        settings.video.frameLimit = myINIFile.getBoolValue("Video","FrameLimit",true);
        settings.video.preferredZoomLevel = myINIFile.getIntValue("Video","Preferred Zoom Level", 0);
        settings.video.scaler = myINIFile.getStringValue("Video","Scaler", "ScaleHD");
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

        fprintf(stdout, "loading texts....."); fflush(stdout);
        pTextManager = new TextManager();
        fprintf(stdout, "\t\tfinished\n"); fflush(stdout);

        if(FileManager::getMissingFiles().size() > 0) {
            // set back to english
            std::vector<std::string> missingFiles = FileManager::getMissingFiles();
            fprintf(stderr,"The following files are missing for language \"%s\":\n",_("LanguageFileExtension").c_str());
            std::vector<std::string>::const_iterator iter;
            for(iter = missingFiles.begin(); iter != missingFiles.end(); ++iter) {
                fprintf(stderr," %s\n",iter->c_str());
            }
            fprintf(stderr,"Language is changed to English!\n");
            settings.general.language = "en";
        }

        for(int i=1; i < argc; i++) {
            //check for overiding params
            std::string parameter(argv[i]);

            if((parameter == "-f") || (parameter == "--fullscreen")) {
                settings.video.fullscreen = true;
            } else if((parameter == "-w") || (parameter == "--window")) {
                settings.video.fullscreen = false;
            } else if(parameter.find("--PlayerName=") == 0) {
                settings.general.playerName = parameter.substr(strlen("--PlayerName="));
            } else if(parameter.find("--ServerPort=") == 0) {
                settings.network.serverPort = atol(argv[i] + strlen("--ServerPort="));
            }
        }

        if(bFirstInit == true) {
            fprintf(stdout, "initializing SDL..... \t\t"); fflush(stdout);
            if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
                fprintf(stderr, "ERROR: Couldn't initialise SDL: %s\n", SDL_GetError());
                exit(EXIT_FAILURE);
            }
            fprintf(stdout, "finished\n"); fflush(stdout);
        }

        if(bFirstGamestart == true && bFirstInit == true) {
            // find screen resolution bigger or equal to 800x600 (otherwise use 640x480)
            SDL_DisplayMode targetDisplayMode = { 0, 800, 600, 0, nullptr};
            SDL_DisplayMode closestDisplayMode;

            if(SDL_GetClosestDisplayMode(SCREEN_DISPLAYINDEX, &targetDisplayMode, &closestDisplayMode) != nullptr) {
                settings.video.width = closestDisplayMode.w;
                settings.video.height = closestDisplayMode.h;
                settings.video.preferredZoomLevel = 1;

                myINIFile.setIntValue("Video","Width",settings.video.width);
                myINIFile.setIntValue("Video","Height",settings.video.height);
                myINIFile.setIntValue("Video","Preferred Zoom Level",1);

                myINIFile.saveChangesTo(getConfigFilepath());
            }
        }

        Scaler::setDefaultScaler(Scaler::getScalerByName(settings.video.scaler));

        if(bFirstInit == true) {
            fprintf(stdout, "initializing sound..... \t");fflush(stdout);
            if( Mix_OpenAudio(AUDIO_FREQUENCY, AUDIO_S16SYS, 2, 1024) < 0 ) {
                SDL_Quit();
                fprintf(stderr,"Warning: Couldn't set %d Hz 16-bit audio\n- Reason: %s\n",AUDIO_FREQUENCY,SDL_GetError());
                exit(EXIT_FAILURE);
            } else {
                fprintf(stdout, "allocated %d channels.\n", Mix_AllocateChannels(6)); fflush(stdout);
            }
        }

        pFileManager = new FileManager( !missingFiles.empty() );

        // now we can finish loading texts
        if(missingFiles.empty()) {
            pTextManager->loadData();
        }

        if(pFileManager->exists("IBM.PAL") == true) {
            palette = LoadPalette_RW(pFileManager->openFile("IBM.PAL"), true);
        }

        fprintf(stdout, "SDL rendering backend...");fflush(stdout);
        setVideoMode();
        SDL_RendererInfo rendererInfo;
        SDL_GetRendererInfo(renderer, &rendererInfo);
        fprintf(stdout, "\t%s\n", rendererInfo.name); fflush(stdout);
        fprintf(stdout, "Maximum texture size...\t\t%dx%d\n", rendererInfo.max_texture_width, rendererInfo.max_texture_height); fflush(stdout);


        fprintf(stdout, "loading fonts...");fflush(stdout);
        pFontManager = new FontManager();
        fprintf(stdout, "\t\tfinished\n"); fflush(stdout);

        if(!missingFiles.empty()) {
            // some files are missing
            bExitGame = true;
            printMissingFilesToScreen();
            fprintf(stdout, "Deinitialize....."); fflush(stdout);
        } else {
            // everything is just fine and we can start the game
            fprintf(stdout, "loading graphics and sounds..."); fflush(stdout);

#ifdef HAS_ASYNC
            auto gfxManagerFut = std::async(std::launch::async, []() { return new GFXManager(); } );
            auto sfxManagerFut = std::async(std::launch::async, []() { return new SFXManager(); } );

            pGFXManager = gfxManagerFut.get();
            pSFXManager = sfxManagerFut.get();
#else
            // g++ does not provide std::launch::async on all platforms
            pGFXManager = new GFXManager();
            pSFXManager = new SFXManager();
#endif

            fprintf(stdout, "\tfinished\n"); fflush(stdout);

            GUIStyle::setGUIStyle(new DuneStyle);

            if(bFirstInit == true) {
                fprintf(stdout, "starting sound player..."); fflush(stdout);
                soundPlayer = new SoundPlayer();
                fprintf(stdout, "\tfinished\n");

                fprintf(stdout, "starting music player...\t"); fflush(stdout);
                if(settings.audio.musicType == "directory") {
                    fprintf(stdout, "playing from music directory\n"); fflush(stdout);
                    musicPlayer = new DirectoryPlayer();
                } else if(settings.audio.musicType == "adl") {
                    fprintf(stdout, "playing ADL files\n"); fflush(stdout);
                    musicPlayer = new ADLPlayer();
                } else if(settings.audio.musicType == "xmi") {
                    fprintf(stdout, "playing XMI files\n"); fflush(stdout);
                    musicPlayer = new XMIPlayer();
                } else {
                    fprintf(stdout, "failed\n"); fflush(stdout);
                    exit(EXIT_FAILURE);
                }

                //musicPlayer->changeMusic(MUSIC_INTRO);
            }

            // Playing intro
            if(((bFirstGamestart == true) || (settings.general.playIntro == true)) && (bFirstInit==true)) {
                fprintf(stdout, "playing intro.....");fflush(stdout);
                Intro* pIntro = new Intro();
                pIntro->run();
                delete pIntro;
                fprintf(stdout, "\t\tfinished\n"); fflush(stdout);
            }

            bFirstInit = false;

            fprintf(stdout, "starting main menu...");fflush(stdout);
            MainMenu * myMenu = new MainMenu();
            fprintf(stdout, "\t\tfinished\n"); fflush(stdout);
            if(myMenu->showMenu() == MENU_QUIT_DEFAULT) {
                bExitGame = true;
            }
            delete myMenu;

            fprintf(stdout, "Deinitialize....."); fflush(stdout);

            GUIStyle::destroyGUIStyle();

            // clear everything
            if(bExitGame == true) {
                delete musicPlayer;
                delete soundPlayer;
                Mix_HaltMusic();
                Mix_CloseAudio();
            }

            delete pTextManager;
            delete pSFXManager;
            delete pGFXManager;
        }

        delete pFontManager;
        delete pFileManager;

        SDL_DestroyTexture(screenTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        if(bExitGame == true) {
            SDL_Quit();
        }
        fprintf(stdout, "\t\tfinished\n"); fflush(stdout);
    } while(bExitGame == false);

    // deinit fnkdat
    if(fnkdat(nullptr, nullptr, 0, FNKDAT_UNINIT) < 0) {
        perror("Could not uninitialize fnkdat");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
