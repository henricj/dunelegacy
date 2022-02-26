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
#include <misc/SDL2pp.h>

#include <SoundPlayer.h>

#include <mmath.h>

#include <CutScenes/Intro.h>

#include <SDL2/SDL_ttf.h>

#include <enet/enet.h>
#include <fmt/core.h>

#include <iostream>
#include <typeinfo>
#include <future>
#include <random>
#include <fcntl.h>

#ifdef _WIN32
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <Windows.h>

#    include <cstdio>
#    include <io.h>
#    ifndef STDOUT_FILENO
#        define STDOUT_FILENO 1
#    endif
#    ifndef STDERR_FILENO
#        define STDERR_FILENO 2
#    endif
#else
#    include <pwd.h>
#    include <sys/types.h>
#    include <unistd.h>
#endif

#ifdef __APPLE__
    #include <MacFunctions.h>
#endif

#if !defined(__GNUG__) || (defined(_GLIBCXX_HAS_GTHREADS) && defined(_GLIBCXX_USE_C99_STDINT_TR1) && (ATOMIC_INT_LOCK_FREE > 1) && !defined(_GLIBCXX_HAS_GTHREADS))
// g++ does not provide std::async on all platforms
#define HAS_ASYNC
#endif

#if HAVE_CXXBI_H
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

namespace {
int currentDisplayIndex = SCREEN_DEFAULT_DISPLAYINDEX;
}

void setVideoMode(int displayIndex);
void realign_buttons();

static void printUsage() {
    fprintf(stderr, "Usage:\n\tdunelegacy [--showlog] [--fullscreen|--window] [--PlayerName=X] [--ServerPort=X]\n");
}

int getLogicalToPhysicalResolutionFactor(int physicalWidth, int physicalHeight) {
    if(physicalWidth >= 1280*3 && physicalHeight >= 720*3) {
        return 3;
    } if(physicalWidth >= 640*2 && physicalHeight >= 480*2) {

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

    if(settings.video.fullscreen) {
        if(SDL_GetClosestDisplayMode(displayIndex, &targetDisplayMode, &closestDisplayMode) == nullptr) {
            sdl2::log_info("Warning: Falling back to a display resolution of 640x480!");
            settings.video.physicalWidth  = 640;
            settings.video.physicalHeight = 480;
            settings.video.width          = 640;
            settings.video.height         = 480;
        } else {
            settings.video.physicalWidth  = closestDisplayMode.w;
            settings.video.physicalHeight = closestDisplayMode.h;
            const auto factor =
                getLogicalToPhysicalResolutionFactor(settings.video.physicalWidth, settings.video.physicalHeight);
            settings.video.width  = settings.video.physicalWidth / factor;
            settings.video.height = settings.video.physicalHeight / factor;
        }
    } else {
        SDL_DisplayMode displayMode;
        SDL_GetDesktopDisplayMode(currentDisplayIndex, &displayMode);

        if(settings.video.physicalWidth > displayMode.w || settings.video.physicalHeight > displayMode.h) {
            settings.video.physicalWidth  = displayMode.w;
            settings.video.physicalHeight = displayMode.h;
        }

        const auto factor =
            getLogicalToPhysicalResolutionFactor(settings.video.physicalWidth, settings.video.physicalHeight);
        settings.video.width  = settings.video.physicalWidth / factor;
        settings.video.height = settings.video.physicalHeight / factor;
    }

    sdl2::log_info("Creating %dx%d for %dx%d window with flags %08x", settings.video.physicalWidth, settings.video.physicalHeight,
        settings.video.width, settings.video.height, videoFlags);

    window = SDL_CreateWindow("Dune Legacy",
                              SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex), SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex),
                              settings.video.physicalWidth, settings.video.physicalHeight,
                              videoFlags);

    sdl2::log_info("Available renderers:");

    { // Scope
        const auto n = SDL_GetNumRenderDrivers();

        for(auto i = 0; i < n; ++i) {
            SDL_RendererInfo info;
            if(0 == SDL_GetRenderDriverInfo(i, &info))
                sdl2::log_info("   %s", info.name);
        }
    }

    if(settings.video.renderer != "default")
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, settings.video.renderer.c_str());

    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");

#if defined(_WIN32)
    // Prefer DX11 on Windows
    if(settings.video.renderer == "default" || nullptr == SDL_GetHint(SDL_HINT_RENDER_DRIVER))
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11");
#    if defined(_DEBUG)
    SDL_SetHint(SDL_HINT_RENDER_DIRECT3D11_DEBUG, "1");
#    endif
#endif

    { // Scope
        const auto* const render_driver_hint = SDL_GetHint(SDL_HINT_RENDER_DRIVER);

        if(render_driver_hint)
            sdl2::log_info("   requested render driver: %s", render_driver_hint);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    { // Scope
        SDL_RendererInfo info;
        if (0 == SDL_GetRendererInfo(renderer, &info)) {
            sdl2::SDL_LogRenderer(&info);

            auto *const end = info.texture_formats + info.num_texture_formats;
            auto *const sf_ptr = std::find(info.texture_formats, end, SCREEN_FORMAT);

            if (sf_ptr == end)
                sdl2::log_warn(SDL_LOG_CATEGORY_RENDER, "The SCREEN_FORMAT is not in the renderer's texture_formats");
        }
        else {
            const auto *const error = SDL_GetError();

            sdl2::log_error(SDL_LOG_CATEGORY_RENDER, "Unable to get render info: %s", error);
        }
    }
    SDL_RenderSetLogicalSize(renderer, settings.video.width, settings.video.height);
    screenTexture = SDL_CreateTexture(renderer, SCREEN_FORMAT, SDL_TEXTUREACCESS_TARGET, settings.video.width, settings.video.height);

    Uint32 screen_format = 0;
    int screen_access = 0;
    if (0 == SDL_QueryTexture(screenTexture, &screen_format, &screen_access, nullptr, nullptr)) {
        if (screen_format != SCREEN_FORMAT)
            sdl2::log_warn(SDL_LOG_CATEGORY_RENDER, "Actual screen format: %s", std::to_string(screen_format).c_str());
    }

    SDL_ShowCursor(SDL_DISABLE);
}

void toogleFullscreen()
{
    if(SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        // switch to windowed mode
        sdl2::log_info("Switching to windowed mode.");
        SDL_SetWindowFullscreen(window, (SDL_GetWindowFlags(window) ^ SDL_WINDOW_FULLSCREEN_DESKTOP));

        SDL_SetWindowSize(window, settings.video.physicalWidth, settings.video.physicalHeight);
        SDL_RenderSetLogicalSize(renderer, settings.video.width, settings.video.height);
    } else {
        // switch to fullscreen mode
        sdl2::log_info("Switching to fullscreen mode.");
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

std::filesystem::path getConfigFilepath()
{
    // determine path to config file
    auto [ok, tmp] = fnkdat(CONFIGFILENAME, FNKDAT_USER | FNKDAT_CREAT);

    return tmp;
}

std::filesystem::path getLogFilepath()
{
    // determine path to config file
    auto [ok, tmp] = fnkdat(LOGFILENAME, FNKDAT_USER | FNKDAT_CREAT);

    if(!ok) {
        THROW(std::runtime_error, "fnkdat() failed!");
    }

    return tmp;
}

void createDefaultConfigFile(const std::filesystem::path& configfilepath, const std::string& language) {
    sdl2::log_info("Creating config file '%s'", configfilepath.u8string());

    const auto file = sdl2::RWops_ptr{ SDL_RWFromFile(configfilepath.u8string().c_str(), "w") };
    if(!file) {
        THROW(sdl_error, "Opening config file failed: %s!", SDL_GetError());
    }

    static constexpr char configfile[] =
                                "[General]\n"
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

    char playername[MAX_PLAYERNAMELENGTH+1] = "Player";

#ifdef _WIN32
    DWORD playernameLength = MAX_PLAYERNAMELENGTH+1;
    GetUserName(playername, &playernameLength);
#else
    struct passwd* pwent = getpwuid(getuid());

    if(pwent != nullptr) {
        strncpy(playername, pwent->pw_name, MAX_PLAYERNAMELENGTH + 1);
        playername[MAX_PLAYERNAMELENGTH] = '\0';
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
    const auto output = fmt::format("{}\n", message);
#ifdef _WIN32
    OutputDebugStringA(output.c_str());
#endif
    fmt::fprintf(stderr, output);
    fflush(stderr);
}

void showMissingFilesMessageBox() {
    SDL_ShowCursor(SDL_ENABLE);

    std::string instruction = "Dune Legacy uses the data files from original Dune II. The following files are missing:\n";

    for(const auto& missingFile : FileManager::getMissingFiles()) {
        instruction += " " + missingFile.u8string() + "\n";
    }

    instruction += "\nPut them in one of the following directories and restart Dune Legacy:\n";
    for(const auto& searchPath : FileManager::getSearchPath()) {
        instruction += " " + searchPath.u8string() + "\n";
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

    sdl2::log_info("User locale is '%s'", pLang);

    if(strlen(pLang) < 2) {
        return "";
    } else {
        return strToLower(std::string(pLang, 2));
    }
}

#if defined(__clang_version__)
void log_clang() {
    sdl2::log_info("   Compiler: clang " __clang_version__);
}
#elif defined(__GNUC_PATCHLEVEL__)
void log_gcc() {
#    ifdef __MINGW32__
    sdl2::log_info("   Compiler: MinGW %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#    else
    sdl2::log_info("   Compiler: GCC %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#    endif
}
#elif defined(_MSC_VER)
void log_msvc() {
#    if defined(_MSC_FULL_VER)
    sdl2::log_info("   Compiler: MSVC %02d.%02d.%05d.%02d", _MSC_VER / 100, _MSC_VER % 100, _MSC_FULL_VER % 100000,
                   _MSC_BUILD);

    sdl2::log_info("   MSVC runtime: "
#        if defined(_MT)
                   "MT "
#        endif
#        if defined(_DLL)
                   "DLL"
#        else
                   "Static"
#        endif
    );
#    endif // _MSC_FULL_VER

    sdl2::log_info("   Instruction set: "
#    if defined(_M_IX86)
                   "x86"
#    elif defined(_M_X64)
                   "x64"
#    elif defined(_M_ARM64)
                   "ARM64"
#    elif defined(_M_ARM)
                   "ARM"
#    else
                   "Unknown"
#    endif
                   "/"
#    if defined(__AVX512F__)
                   "AVX512"
#    elif defined(__AVX2__)
                   "AVX2"
#    elif defined(__AVX__)
                   "AVX"
#    elif defined(_M_IX86_FP)
#        if _M_IX86_FP == 0
                   "x87 FPU"
#        elif _M_IX86_FP == 1
                   "SSE"
#        elif _M_IX86_FP == 2
                   "SSE2"
#        else
                   "Unknown"
#        endif
#    else
                   "Default"
#    endif
    );

#    if defined(_CONTROL_FLOW_GUARD)
    sdl2::log_info("   Control flow guard");
#endif
}
#endif

void log_build_info() {
    sdl2::log_info("   %d bit build, C++ standard %d", 8 * sizeof(void*), __cplusplus);

#if defined(__clang_version__)
    log_clang();
#elif defined(__GNUC_PATCHLEVEL__)
    log_gcc();
#elif defined(_MSC_VER)
    log_msvc();
#endif // _MSC_VER

#if defined(FMT_VERSION)
    sdl2::log_info("   fmt %d.%d.%d", FMT_VERSION / 10000, (FMT_VERSION / 100) % 100, FMT_VERSION % 100);
#endif

#if defined(ENET_VERSION_MAJOR) && defined(ENET_VERSION_MINOR) && defined(ENET_VERSION_PATCH)
    sdl2::log_info("   enet %d.%d.%d", ENET_VERSION_MAJOR, ENET_VERSION_MINOR, ENET_VERSION_PATCH);
#endif
}

int main(int argc, char *argv[]) {
    SDL_LogSetOutputFunction(logOutputFunction, nullptr);
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);

    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

    // global try/catch around everything
    try {

        // init fnkdat
        auto [ok, tmp] = fnkdat(FNKDAT_INIT);
        if(!ok) {
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

        if(!bShowDebugLog) {
            // get utf8-encoded log file path
            auto logfilePath = getLogFilepath();

            #ifdef _WIN32

            FILE* discard = nullptr;
            if(freopen_s(&discard, R"(\\.\NUL)", "r", stdin))
                THROW(io_error, "Intitializing stdin failed!");
            if(freopen_s(&discard, R"(\\.\NUL)", "a", stdout))
                THROW(io_error, "Intitializing stdout failed!");
            if(freopen_s(&discard, R"(\\.\NUL)", "a", stderr))
                THROW(io_error, "Intitializing stderr failed!");

            const auto fn_out = _fileno(stdout);
            const auto fn_err = _fileno(stderr);

            const auto wLogFilePath = logfilePath.wstring();

            const auto log_handle = CreateFileW(wLogFilePath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                                                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);

            if(log_handle == INVALID_HANDLE_VALUE) {
                // use stdout in this error case as stderr is not yet ready
                THROW(io_error, "Opening logfile '%s' as stdout failed!", logfilePath.string().c_str());
            }

            if(!SetStdHandle(STD_OUTPUT_HANDLE, log_handle))
                THROW(io_error, "Intitializing output handle failed!");
            if(!SetStdHandle(STD_ERROR_HANDLE, log_handle))
                THROW(io_error, "Intitializing error handle failed!");

            const auto log_fd = _open_osfhandle(reinterpret_cast<intptr_t>(log_handle), _O_TEXT);

            if(_dup2(log_fd, fn_out))
                THROW(io_error, "Redirecting output failed!");
            if(_dup2(log_fd, fn_err))
                THROW(io_error, "Redirecting error failed!");

            // No buffering
            setvbuf(stdout, nullptr, _IONBF, 0);
            setvbuf(stderr, nullptr, _IONBF, 0);

            #else

            char* pLogfilePath = (char*)logfilePath.c_str();

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

        sdl2::log_info("Starting Dune Legacy %s on %s", VERSION, SDL_GetPlatform());

        log_build_info();

        // First check for missing files
        auto missingFiles = FileManager::getMissingFiles();

        if(!missingFiles.empty()) {
            // create data directory inside config directory
            auto [ok, tmp] = fnkdat("data", FNKDAT_USER | FNKDAT_CREAT);

            showMissingFilesMessageBox();

            return EXIT_FAILURE;
        }

        bool bExitGame = false;
        bool bFirstInit = true;
        bool bFirstGamestart = false;

        debug = false;
        cursorFrame = UI_CursorNormal;

        do {
            // we do not use rand() but maybe some library does; thus we shall initialize it
            auto seed = static_cast<unsigned>(std::random_device()()) ^ static_cast<unsigned>(time(nullptr));
            srand(seed);

            // check if configfile exists
            auto configfilepath = getConfigFilepath();
            if(!existsFile(configfilepath)) {
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
            settings.video.rotateUnitGraphics  = myINIFile.getBoolValue("Video", "RotateUnitGraphics", false);
            settings.video.renderer = myINIFile.getStringValue("Video", "Renderer", "default");
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
                auto setBackToEnglishWarning = fmt::sprintf("The following files are missing for language \"%s\":\n",_("LanguageFileExtension"));
                for(const auto& filename : missingFiles) {
                    setBackToEnglishWarning += filename.u8string() + "\n";
                }
                setBackToEnglishWarning += "\nLanguage is changed to English!";
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Dune Legacy", setBackToEnglishWarning.c_str(), nullptr);

                sdl2::log_info("Warning: Language is changed to English!");

                settings.general.language = "en";
                myINIFile.setStringValue("General","Language",settings.general.language);
                if(!myINIFile.saveChangesTo(configfilepath)) {
                    sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Unable to save configuration file %s",
                                 configfilepath.u8string().c_str());
                }

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

            if(bFirstInit) {
                sdl2::log_info("Initializing SDL...");

                if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
                    THROW(sdl_error, "Couldn't initialize SDL: %s!", SDL_GetError());
                }

                SDL_version compiledVersion;
                SDL_version linkedVersion;
                SDL_VERSION(&compiledVersion);
                SDL_GetVersion(&linkedVersion);
                sdl2::log_info("SDL runtime v%d.%d.%d", linkedVersion.major, linkedVersion.minor, linkedVersion.patch);
                sdl2::log_info("SDL compile-time v%d.%d.%d", compiledVersion.major, compiledVersion.minor, compiledVersion.patch);

                if(TTF_Init() < 0) {
                    THROW(sdl_error, "Couldn't initialize SDL2_ttf: %s!", TTF_GetError());
                }

                SDL_version TTFCompiledVersion;
                SDL_TTF_VERSION(&TTFCompiledVersion);
                const SDL_version* pTTFLinkedVersion = TTF_Linked_Version();
                sdl2::log_info("SDL2_ttf runtime v%d.%d.%d", pTTFLinkedVersion->major, pTTFLinkedVersion->minor, pTTFLinkedVersion->patch);
                sdl2::log_info("SDL2_ttf compile-time v%d.%d.%d", TTFCompiledVersion.major, TTFCompiledVersion.minor, TTFCompiledVersion.patch);
            }

            if(bFirstGamestart && bFirstInit) {
                SDL_DisplayMode displayMode;
                SDL_GetDesktopDisplayMode(currentDisplayIndex, &displayMode);

                const auto factor = getLogicalToPhysicalResolutionFactor(displayMode.w, displayMode.h);
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

                if(!myINIFile.saveChangesTo(getConfigFilepath())) {
                    sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Unable to save configuration file %s",
                                 getConfigFilepath().u8string().c_str());
                }
            }

            Scaler::setDefaultScaler(Scaler::getScalerByName(settings.video.scaler));

            if(bFirstInit) {
                sdl2::log_info("Initializing audio...");
                if( Mix_OpenAudio(AUDIO_FREQUENCY, AUDIO_S16SYS, 2, 1024) < 0 ) {
                    //SDL_Quit();
                    //THROW(sdl_error, "Couldn't set %d Hz 16-bit audio. Reason: %s!", AUDIO_FREQUENCY, SDL_GetError());
                } else {
                    sdl2::log_info("%d audio channels were allocated.", Mix_AllocateChannels(28));
                }
            }

            pFileManager = std::make_unique<FileManager>();

            // now we can finish loading texts
            pTextManager->loadData();

            palette = LoadPalette_RW(pFileManager->openFile("IBM.PAL").get());

            sdl2::log_info("Setting video mode...");
            setVideoMode(currentDisplayIndex);
            SDL_RendererInfo rendererInfo;
            SDL_GetRendererInfo(renderer, &rendererInfo);
            sdl2::log_info("Renderer: %s (max texture size: %dx%d)", rendererInfo.name, rendererInfo.max_texture_width, rendererInfo.max_texture_height);


            sdl2::log_info("Loading fonts...");
            pFontManager = std::make_unique<FontManager>();

            sdl2::log_info("Loading graphics and sounds...");

#ifdef HAS_ASYNC
            // If we have async, initialize the sounds on another thread while we initialize GFX on this one.
            auto sfxManagerFut = std::async(std::launch::async | std::launch::deferred, [] {
                const auto start   = std::chrono::steady_clock::now();
                auto       ret     = std::make_unique<SFXManager>();
                const auto elapsed = std::chrono::steady_clock::now() - start;
                return std::make_pair(std::move(ret), elapsed);
            });
#else
            // g++ does not provide std::launch::async on all platforms
            pSFXManager = std::make_unique<SFXManager>();
#endif

            const auto start = std::chrono::steady_clock::now();
            pGFXManager = std::make_unique<GFXManager>();
            const auto elapsed = std::chrono::steady_clock::now() - start;

            sdl2::log_info("GFXManager time: %s", std::to_string(std::chrono::duration<double>(elapsed).count()).c_str());

#ifdef HAS_ASYNC
            try {
                auto sfxResult = sfxManagerFut.get();
                pSFXManager = std::move(sfxResult.first);
                sdl2::log_info("SFXManager time: %s", std::to_string(std::chrono::duration<double>(sfxResult.second).count()).c_str());
            } catch(const std::exception& e) {
                pSFXManager = nullptr;
                const auto message = fmt::sprintf("The sound manager was unable to initialize: '%s' was thrown:\n\n%s\n\nDune Legacy is unable to play sound!", demangleSymbol(typeid(e).name()), e.what());
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Dune Legacy: Warning", message.c_str(), nullptr);
            }
#endif

            GUIStyle::setGUIStyle(std::make_unique<DuneStyle>());

            if(bFirstInit) {
                sdl2::log_info("Starting sound player...");
                soundPlayer = std::make_unique<SoundPlayer>();

                if(settings.audio.musicType == "directory") {
                    sdl2::log_info("Starting directory music player...");
                    musicPlayer = std::make_unique<DirectoryPlayer>();
                } else if(settings.audio.musicType == "adl") {
                    sdl2::log_info("Starting ADL music player...");
                    musicPlayer = std::make_unique<ADLPlayer>();
                } else if(settings.audio.musicType == "xmi") {
                    sdl2::log_info("Starting XMI music player...");
                    musicPlayer = std::make_unique<XMIPlayer>();
                } else {
                    THROW(std::runtime_error, "Invalid music type: '%'", settings.audio.musicType);
                }

                //musicPlayer->changeMusic(MUSIC_INTRO);
            }

            // Playing intro
            if(((bFirstGamestart) || (settings.general.playIntro)) && (bFirstInit)) {
                sdl2::log_info("Playing intro...");
                Intro().run();
            }

            bFirstInit = false;

            sdl2::log_info("Starting main menu...");
            { // Scope
                if (MainMenu().showMenu() == MENU_QUIT_DEFAULT) {
                    bExitGame = true;
                }
            }

            sdl2::log_info("Deinitialize...");

            GUIStyle::destroyGUIStyle();

            // clear everything
            if(bExitGame) {
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

            if(bExitGame) {
                TTF_Quit();
                SDL_Quit();
            }
            sdl2::log_info("Deinitialization finished!");
        } while(!bExitGame);

        // deinit fnkdat
        auto [ok2, tmp2] = fnkdat(FNKDAT_UNINIT);
        if(!ok2) {
            THROW(std::runtime_error, "Cannot uninitialize fnkdat!");
        }
    } catch(const std::exception& e) {
        std::string message = std::string("An unhandled exception of type \'") + demangleSymbol(typeid(e).name()) + std::string("\' was thrown:\n\n") + e.what() + std::string("\n\nDune Legacy will now be terminated!");
        sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Dune Legacy: Unrecoverable error: %s", message.c_str());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Dune Legacy: Unrecoverable error", message.c_str(), nullptr);

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
