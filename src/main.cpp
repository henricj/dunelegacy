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

#include <FileClasses/DuneConfig.h>
#include <FileClasses/FileManager.h>
#include <FileClasses/FontManager.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/INIFile.h>
#include <FileClasses/Palfile.h>
#include <FileClasses/SFXManager.h>
#include <FileClasses/TextManager.h>
#include <FileClasses/music/ADLPlayer.h>
#include <FileClasses/music/DirectoryPlayer.h>
#include <FileClasses/music/XMIPlayer.h>

#include <GUI/GUIStyle.h>
#include <GUI/dune/DuneStyle.h>

#include <Menu/MainMenu.h>

#include "Game.h"
#include "ScreenBorder.h"
#include "misc/dune_events.h"
#include "misc/sdl_support.h"
#include <misc/FileSystem.h>
#include <misc/SDL2pp.h>
#include <misc/Scaler.h>
#include <misc/exceptions.h>
#include <misc/fnkdat.h>
#include <misc/string_util.h>

#include <SoundPlayer.h>

#include <CutScenes/Intro.h>

#include "logging.h"

#include <SDL2/SDL_ttf.h>

#include <cmath>
#include <fcntl.h>
#include <future>
#include <iostream>
#include <random>
#include <typeinfo>

#ifdef _WIN32
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <Windows.h>

#    include <ShellScalingApi.h>

#    include <SDL_syswm.h>

#    ifdef DUNE_CRT_HEAP_DEBUG
#        include <crtdbg.h>
#    endif
#else
#    include <pwd.h>
#    include <sys/types.h>
#    include <unistd.h>
#endif

#ifdef __APPLE__
#    include <MacFunctions.h>
#endif

#if !defined(__GNUG__)                                                                                                 \
    || (defined(_GLIBCXX_HAS_GTHREADS) && defined(_GLIBCXX_USE_C99_STDINT_TR1) && (ATOMIC_INT_LOCK_FREE > 1)           \
        && !defined(_GLIBCXX_HAS_GTHREADS))
// g++ does not provide std::async on all platforms
#    define HAS_ASYNC
#endif

#if HAVE_CXXBI_H
#    include <cxxabi.h>
inline std::string demangleSymbol(const char* symbolname) {
    int status       = 0;
    std::size_t size = 0;
    char* result     = abi::__cxa_demangle(symbolname, nullptr, &size, &status);
    if (status != 0) {
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

void setVideoMode(int displayIndex) {
    dune::globals::screenTexture.reset();
    dune::globals::renderer.reset();
    dune::globals::window.reset();

    int videoFlags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;

    auto& video = dune::globals::settings.video;

    if (video.fullscreen)
        videoFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

    const SDL_DisplayMode targetDisplayMode{0, video.physicalWidth, video.physicalHeight, 0, nullptr};

    SDL_DisplayMode closestDisplayMode{};

    if (video.fullscreen) {
        if (SDL_GetClosestDisplayMode(displayIndex, &targetDisplayMode, &closestDisplayMode) == nullptr) {
            sdl2::log_info("Warning: Falling back to a display resolution of 640x480!");
            video.physicalWidth  = 640;
            video.physicalHeight = 480;
            video.width          = 640;
            video.height         = 480;
        } else {
            video.physicalWidth  = closestDisplayMode.w;
            video.physicalHeight = closestDisplayMode.h;
            video.width          = video.physicalWidth;
            video.height         = video.physicalHeight;
        }
    } else {
        SDL_DisplayMode displayMode{};
        SDL_GetDesktopDisplayMode(currentDisplayIndex, &displayMode);

        if (video.physicalWidth > displayMode.w || video.physicalHeight > displayMode.h) {
            video.physicalWidth  = displayMode.w;
            video.physicalHeight = displayMode.h;
        }

        video.width  = video.physicalWidth;
        video.height = video.physicalHeight;
    }

    sdl2::log_info("Creating %dx%d for %dx%d window with flags %08x", video.physicalWidth, video.physicalHeight,
                   video.width, video.height, videoFlags);

    sdl2::window_ptr window{SDL_CreateWindow("Dune Legacy", SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex),
                                             SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex), video.physicalWidth,
                                             video.physicalHeight, videoFlags)};

    if (!window)
        THROW(sdl_error, "Unable to create window: {}!", SDL_GetError());

    SDL_SetWindowMinimumSize(window.get(), GUIStyle::MINIMUM_WIDTH, GUIStyle::MINIMUM_HEIGHT);

    { // Scope
        const auto screen_format = SDL_GetWindowPixelFormat(window.get());

        sdl2::log_info("The window is using pixel format %s and the default format is %s",
                       SDL_GetPixelFormatName(screen_format), SDL_GetPixelFormatName(SCREEN_FORMAT));
    }

    sdl2::log_info("Available renderers:");

    { // Scope
        const auto n = SDL_GetNumRenderDrivers();

        for (auto i = 0; i < n; ++i) {
            SDL_RendererInfo info;
            if (0 == SDL_GetRenderDriverInfo(i, &info))
                sdl2::log_info("   %s", info.name);
        }
    }

    if (video.renderer != "default")
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, video.renderer.c_str());

    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
    // SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

#if defined(_WIN32)
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

    // Prefer DX11 on Windows
    if (video.renderer == "default" || nullptr == SDL_GetHint(SDL_HINT_RENDER_DRIVER))
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11");
#    if defined(_DEBUG)
    SDL_SetHint(SDL_HINT_RENDER_DIRECT3D11_DEBUG, "1");
#    endif
#endif

    // Scope
    if (const auto* const render_driver_hint = SDL_GetHint(SDL_HINT_RENDER_DRIVER))
        sdl2::log_info("   requested render driver: %s", render_driver_hint);

    sdl2::renderer_ptr renderer{
        SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE)};

    if (!renderer)
        THROW(sdl_error, "Unable to create renderer: {}!", SDL_GetError());

    { // Scope
        SDL_RendererInfo info;
        if (0 == SDL_GetRendererInfo(renderer.get(), &info)) {
            sdl2::SDL_LogRenderer(&info);

            const auto begin = std::begin(info.texture_formats);
            const auto end   = std::end(info.texture_formats);

            const auto sf_ptr = std::find(begin, end, static_cast<Uint32>(SCREEN_FORMAT));

            if (sf_ptr == end)
                sdl2::log_warn(SDL_LOG_CATEGORY_RENDER, "The SCREEN_FORMAT is not in the renderer's texture_formats");
        } else {
            const auto* const error = SDL_GetError();

            sdl2::log_error(SDL_LOG_CATEGORY_RENDER, "Unable to get render info: %s", error);
        }
    }

    sdl2::texture_ptr screenTexture{
        SDL_CreateTexture(renderer.get(), SCREEN_FORMAT, SDL_TEXTUREACCESS_TARGET, video.width, video.height)};

    uint32_t screen_format = 0;
    int screen_access      = 0;
    if (0 == SDL_QueryTexture(screenTexture.get(), &screen_format, &screen_access, nullptr, nullptr)) {
        if (screen_format != SCREEN_FORMAT)
            sdl2::log_warn(SDL_LOG_CATEGORY_RENDER, "Actual screen format: %s", SDL_GetPixelFormatName(screen_format));
    }

    dune::globals::window        = std::move(window);
    dune::globals::renderer      = std::move(renderer);
    dune::globals::screenTexture = std::move(screenTexture);
}

void showMissingFilesMessageBox(const CaseInsensitiveFilesystemCache& filesystemCache) {
    SDL_ShowCursor(SDL_ENABLE);

    std::string instruction =
        "Dune Legacy uses the data files from original Dune II. The following files are missing:\n";

    for (const auto& missingFile : PakFileConfiguration::getMissingFiles(filesystemCache)) {
        instruction += fmt::sprintf(" %s\n", reinterpret_cast<const char*>(missingFile.u8string().c_str()));
        sdl2::log_error("missing required %s", reinterpret_cast<const char*>(missingFile.u8string().c_str()));
    }

    instruction += "\nPut them in one of the following directories and restart Dune Legacy:\n";
    for (const auto& searchPath : FileManager::getSearchPath()) {
        instruction += fmt::sprintf(" %s\n", reinterpret_cast<const char*>(searchPath.u8string().c_str()));
        sdl2::log_info("search path %s", reinterpret_cast<const char*>(searchPath.u8string().c_str()));
    }

    instruction += "\nYou may want to add GERMAN.PAK or FRENCH.PAK for playing in these languages.";

    if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Dune Legacy", instruction.c_str(), nullptr)) {
        const auto error = SDL_GetError();
        sdl2::log_error("message box failed: %s", error);

        fprintf(stderr, "%s\n", instruction.c_str());
    }
}

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

    sdl2::log_info("User locale is '%s'", pLang);

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

bool configure_game(int argc, char* argv[], bool bFirstInit) {
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
            sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Unable to save configuration file %s",
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
        SDL_DisplayMode displayMode;
        SDL_GetDesktopDisplayMode(currentDisplayIndex, &displayMode);

        const auto factor = getLogicalToPhysicalResolutionFactor(displayMode.w, displayMode.h);
        GUIStyle::getInstance().setZoom(static_cast<float>(factor));

        settings.video.physicalWidth  = displayMode.w;
        settings.video.physicalHeight = displayMode.h;
        settings.video.width          = displayMode.w;

        settings.video.height = displayMode.h;

        settings.video.preferredZoomLevel = 1;

        myINIFile.setIntValue("Video", "Width", settings.video.width);
        myINIFile.setIntValue("Video", "Height", settings.video.height);
        myINIFile.setIntValue("Video", "Physical Width", settings.video.physicalWidth);
        myINIFile.setIntValue("Video", "Physical Height", settings.video.physicalHeight);
        myINIFile.setIntValue("Video", "Preferred Zoom Level", 1);

        if (!myINIFile.saveChangesTo(getConfigFilepath())) {
            sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Unable to save configuration file %s",
                            reinterpret_cast<const char*>(getConfigFilepath().u8string().c_str()));
        }
    }

    Scaler::setDefaultScaler(Scaler::getScalerByName(settings.video.scaler));

    return bFirstGamestart;
}

namespace {

inline constexpr auto default_dpi =
#if defined(__APPLE__)
    72.0f;
#elif defined(USER_DEFAULT_SCREEN_DPI)
    static_cast<float>(USER_DEFAULT_SCREEN_DPI); // Windows
#else
    96.0f; // This is true for Windows, but what about others?
#endif

#if defined(_WIN32)
float physical_dpi(SDL_Window* sdl_window) {
    using fn_ptr =
        HRESULT(WINAPI*)(_In_ HMONITOR hmonitor, _In_ MONITOR_DPI_TYPE dpiType, _Out_ UINT * dpiX, _Out_ UINT * dpiY);

    static auto shcore = LoadLibraryA("Shcore.dll");

    // We disable C4191 here since GetProcAddress does return FARPROC, but the actual function really isn't FARPROC.
#    pragma warning(push)
#    pragma warning(disable : 4191)
    static auto get_dpi_for_monitor =
        nullptr == shcore ? nullptr : reinterpret_cast<fn_ptr>(GetProcAddress(shcore, "GetDpiForMonitor"));
#    pragma warning(pop)

    if (!shcore)
        return default_dpi;

    SDL_SysWMinfo wmInfo{};
    SDL_VERSION(&wmInfo.version)
    SDL_GetWindowWMInfo(sdl_window, &wmInfo);

    const auto hwnd = wmInfo.info.win.window;

    if (nullptr == hwnd)
        return default_dpi;

    const auto monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

    if (nullptr == monitor)
        return default_dpi;

    UINT dpiX          = 0;
    UINT dpiY          = 0;
    const auto hresult = get_dpi_for_monitor(monitor, MONITOR_DPI_TYPE::MDT_RAW_DPI, &dpiX, &dpiY);

    if (FAILED(hresult))
        return default_dpi;

    return static_cast<float>(std::max(dpiX, dpiY));
}
#else
auto physical_dpi(SDL_Window*) {
    return default_dpi;
}
#endif

} // namespace
void update_display_scale(SDL_Window* sdl_window) {
    // A 14" 640x480 display had a DPI of about 57 (e.g., the IBM 8514 Color Monitor)
    static constexpr auto dune_dpi = 57.f;

    const auto actual_dpi = physical_dpi(sdl_window);

    auto& gui = GUIStyle::getInstance();

    const auto displayIndex = SDL_GetWindowDisplayIndex(sdl_window);
    float dpi               = NAN;
    if (0 != SDL_GetDisplayDPI(displayIndex, nullptr, &dpi, nullptr)) {
        dpi = 1;
    }

    gui.setDisplayDpi((dpi * actual_dpi) / (default_dpi * dune_dpi));

    auto* sdl_renderer = SDL_GetRenderer(sdl_window);

    float scaleX = NAN;
    float scaleY = NAN;
    SDL_RenderGetScale(sdl_renderer, &scaleX, &scaleY);

    gui.setZoom(scaleX);
}

namespace {

template<typename TPtr>
class GlobalCleanup final {
public:
    GlobalCleanup(std::unique_ptr<TPtr>& pointer) : pointer_{pointer} { }
    ~GlobalCleanup() { pointer_.reset(); }

private:
    std::unique_ptr<TPtr>& pointer_;
};

struct DisplayCleanup final {
    ~DisplayCleanup() {
        dune::globals::screenTexture.reset();
        dune::globals::renderer.reset();
        dune::globals::window.reset();
    }
};

} // namespace

bool run_game(int argc, char* argv[]) {
    bool bExitGame       = false;
    bool bFirstInit      = true;
    bool bFirstGamestart = false;

    dune::globals::debug       = false;
    dune::globals::cursorFrame = UI_CursorNormal;

    do {
        { // Scope
            DisplayCleanup display_cleanup;
            GlobalCleanup text_cleanup{dune::globals::pTextManager};

            if (configure_game(argc, argv, bFirstInit))
                bFirstGamestart = true;

            if (bFirstInit) {
                sdl2::log_info("Initializing audio...");
                if (Mix_OpenAudio(AUDIO_FREQUENCY, AUDIO_S16SYS, 2, 1024) < 0) {
                    // SDL_Quit();
                    // THROW(sdl_error, "Couldn't set {} Hz 16-bit audio. Reason: {}!", AUDIO_FREQUENCY,
                    // SDL_GetError());
                } else {
                    sdl2::log_info("%d audio channels were allocated.", Mix_AllocateChannels(28));

                    SDL_version compiledVersion{};
                    SDL_MIXER_VERSION(&compiledVersion)
                    const auto* const linkedVersion = Mix_Linked_Version();
                    sdl2::log_info("SDL Mixer runtime v%d.%d.%d", linkedVersion->major, linkedVersion->minor,
                                   linkedVersion->patch);
                    sdl2::log_info("SDL Mixer compile-time v%d.%d.%d", compiledVersion.major, compiledVersion.minor,
                                   compiledVersion.patch);
                }
            }

            GlobalCleanup file_cleanup{dune::globals::pFileManager};
            dune::globals::pFileManager = std::make_unique<FileManager>();

            const auto* const pFileManager = dune::globals::pFileManager.get();
            auto* const pTextManager       = dune::globals::pTextManager.get();
            auto& settings                 = dune::globals::settings;

            // now we can finish loading texts
            pTextManager->loadData();

            dune::globals::palette = LoadPalette_RW(pFileManager->openFile("IBM.PAL").get());

            sdl2::log_info("Setting video mode...");
            setVideoMode(currentDisplayIndex);

            auto* const renderer = dune::globals::renderer.get();

            SDL_RendererInfo rendererInfo;
            SDL_GetRendererInfo(renderer, &rendererInfo);
            sdl2::log_info("Renderer: %s (max texture size: %dx%d)", rendererInfo.name, rendererInfo.max_texture_width,
                           rendererInfo.max_texture_height);

            static constexpr auto video_default_typeface = "Philosopher-Bold.ttf";

            const auto typeface = settings.video.typeface.empty() || "default" == settings.video.typeface
                                    ? video_default_typeface
                                    : settings.video.typeface;

            sdl2::log_info("Loading fonts from typeface %s...", typeface);
            GlobalCleanup font_cleanup{dune::globals::pFontManager};
            dune::globals::pFontManager = std::make_unique<FontManager>(typeface);

            GUIStyle::setGUIStyle(std::make_unique<DuneStyle>(dune::globals::pFontManager.get()));

            update_display_scale(dune::globals::window.get());

            { // Scope
                int w = 0, h = 0;
                SDL_GetRendererOutputSize(renderer, &w, &h);
                GUIStyle::getInstance().setLogicalSize(renderer, w, h);
            }

            sdl2::log_info("Loading graphics and sounds...");

            GlobalCleanup sfx_cleanup{dune::globals::pSFXManager};
#ifdef HAS_ASYNC
            // If we have async, initialize the sounds on another thread while we initialize GFX on this one.
            auto sfxManagerFut = std::async(std::launch::async | std::launch::deferred, [] {
                const auto start   = std::chrono::steady_clock::now();
                auto ret           = std::make_unique<SFXManager>();
                const auto elapsed = std::chrono::steady_clock::now() - start;
                return std::make_pair(std::move(ret), elapsed);
            });
#else
            // g++ does not provide std::launch::async on all platforms
            dune::globals::pSFXManager = std::make_unique<SFXManager>();
#endif

            GlobalCleanup gfx_cleanup{dune::globals::pGFXManager};
            const auto start = std::chrono::steady_clock::now();
            { // Scope
                const auto size            = getRendererSizePoint();
                dune::globals::pGFXManager = std::make_unique<GFXManager>(renderer, size.x, size.y);
            }
            const auto elapsed = std::chrono::steady_clock::now() - start;

            sdl2::log_info("GFXManager time: %f", std::chrono::duration<double>(elapsed).count());

            const auto* const pGFXManager = dune::globals::pGFXManager.get();

            if (auto* cursor = pGFXManager->getCursor(UI_CursorNormal))
                SDL_SetCursor(cursor);

#ifdef HAS_ASYNC
            try {
                auto sfxResult             = sfxManagerFut.get();
                dune::globals::pSFXManager = std::move(sfxResult.first);
                sdl2::log_info("SFXManager time: %f", std::chrono::duration<double>(sfxResult.second).count());
            } catch (const std::exception& e) {
                dune::globals::pSFXManager.reset();
                const auto message = fmt::sprintf("The sound manager was unable to initialize: '%s' was "
                                                  "thrown:\n\n%s\n\nDune Legacy is unable to play sound!",
                                                  demangleSymbol(typeid(e).name()), e.what());
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Dune Legacy: Warning", message.c_str(), nullptr);
            }
#endif

            if (bFirstInit) {
                sdl2::log_info("Starting sound player...");
                dune::globals::soundPlayer = std::make_unique<SoundPlayer>();

                if (settings.audio.musicType == "directory") {
                    sdl2::log_info("Starting directory music player...");
                    dune::globals::musicPlayer = std::make_unique<DirectoryPlayer>();
                } else if (settings.audio.musicType == "adl") {
                    sdl2::log_info("Starting ADL music player...");
                    dune::globals::musicPlayer = std::make_unique<ADLPlayer>();
                } else if (settings.audio.musicType == "xmi") {
                    sdl2::log_info("Starting XMI music player...");
                    dune::globals::musicPlayer = std::make_unique<XMIPlayer>();
                } else {
                    THROW(std::runtime_error, "Invalid music type: '{}'", settings.audio.musicType);
                }

                // musicPlayer->changeMusic(MUSIC_INTRO);
            }

            // Playing intro
            if (((bFirstGamestart) || (settings.general.playIntro)) && (bFirstInit)) {
                sdl2::log_info("Playing intro...");
                Intro().run();
            }

            bFirstInit = false;

            sdl2::log_info("Starting main menu...");

            { // Scope
                GlobalCleanup game_cleanup{dune::globals::currentGame};
                GlobalCleanup border_cleanup{dune::globals::screenborder};

                if (MainMenu().showMenu({}) == MENU_QUIT_DEFAULT) {
                    bExitGame = true;
                }
            }

            sdl2::log_info("Deinitialize...");

            GUIStyle::destroyGUIStyle();

            // clear everything
            if (bExitGame) {
                dune::globals::musicPlayer.reset();
                dune::globals::soundPlayer.reset();
                Mix_HaltMusic();
                Mix_CloseAudio();
            } else {
                // save the current display index for later reuse
                currentDisplayIndex = SDL_GetWindowDisplayIndex(dune::globals::window.get());
            }
        }

        sdl2::log_info("Deinitialization finished!");
    } while (!bExitGame);

    return true;
}

namespace {
#ifdef DUNE_CRT_HEAP_DEBUG
struct DuneHeapDebug final {
    DuneHeapDebug() {
        auto tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
        tmpDbgFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
        // tmpDbgFlag |= _CRTDBG_CHECK_CRT_DF;
        tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
        // tmpDbgFlag |= _CRTDBG_CHECK_EVERY_1024_DF;
        _CrtSetDbgFlag(tmpDbgFlag);

        sdl2::log_info("Enabling CRT heap debugging (%x)", tmpDbgFlag);
    }
    ~DuneHeapDebug() { _CrtDumpMemoryLeaks(); }
};
#else
struct DuneHeapDebug { };
#endif

struct SDL_handle final {
    SDL_handle(Uint32 flags) {
        if (SDL_Init(flags) < 0)
            THROW(sdl_error, "Couldn't initialize SDL: {}!", SDL_GetError());
    }
    ~SDL_handle() { SDL_Quit(); }
};

struct TTF_handle final {
    TTF_handle() {
        if (TTF_Init() < 0)
            THROW(sdl_error, "Couldn't initialize SDL2_ttf: {}!", TTF_GetError());
    }
    ~TTF_handle() { TTF_Quit(); }
};

} // namespace

int main(int argc, char* argv[]) {
    [[maybe_unused]] DuneHeapDebug heap_debug;

    dune::logging_initialize();

    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

    // global try/catch around everything
    try {
        GlobalCleanup sound_cleanup{dune::globals::soundPlayer};
        GlobalCleanup music_cleanup{dune::globals::musicPlayer};

        // init fnkdat
        { // Scope
            auto [ok, tmp] = fnkdat(FNKDAT_INIT);
            if (!ok)
                THROW(std::runtime_error, "Cannot initialize fnkdat!");
        }

        bool bShowDebugLog = false;
        for (int i = 1; i < argc; i++) {
            // check for overriding params
            std::string parameter(argv[i]);

            if (parameter == "--showlog") {
                // special parameter which does not overwrite settings
                bShowDebugLog = true;
            } else if ((parameter == "-f") || (parameter == "--fullscreen") || (parameter == "-w")
                       || (parameter == "--window") || (parameter.compare(0, 13, "--PlayerName=") == 0)
                       || (parameter.compare(0, 13, "--ServerPort=") == 0)) {
                // normal parameter for overwriting settings
                // handle later
            } else {
                printUsage();
                exit(EXIT_FAILURE);
            }
        }

        dune::logging_configure(!bShowDebugLog);

        // First check for missing files
        const CaseInsensitiveFilesystemCache filesystemCache(FileManager::getSearchPath());

        const auto missingFiles = PakFileConfiguration ::getMissingFiles(filesystemCache);

        if (!missingFiles.empty()) {
            // create data directory inside config directory
            auto [ok, tmp] = fnkdat("data", FNKDAT_USER | FNKDAT_CREAT);

            showMissingFilesMessageBox(filesystemCache);

            return EXIT_FAILURE;
        }

        // we do not use rand() but maybe some library does; thus we shall initialize it
        const auto seed = std::random_device()() ^ static_cast<unsigned>(time(nullptr));
        srand(seed);

        sdl2::log_info("Initializing SDL...");

        SDL_handle sdl_handle{SDL_INIT_TIMER | SDL_INIT_VIDEO};

        SDL_version compiledVersion{};
        SDL_version linkedVersion;
        SDL_VERSION(&compiledVersion)
        SDL_GetVersion(&linkedVersion);
        sdl2::log_info("SDL runtime v%d.%d.%d", linkedVersion.major, linkedVersion.minor, linkedVersion.patch);
        sdl2::log_info("SDL compile-time v%d.%d.%d", compiledVersion.major, compiledVersion.minor,
                       compiledVersion.patch);

        TTF_handle ttf_handle;

        SDL_version TTFCompiledVersion{};
        SDL_TTF_VERSION(&TTFCompiledVersion);
        const SDL_version* pTTFLinkedVersion = TTF_Linked_Version();
        sdl2::log_info("SDL2_ttf runtime v%d.%d.%d", pTTFLinkedVersion->major, pTTFLinkedVersion->minor,
                       pTTFLinkedVersion->patch);
        sdl2::log_info("SDL2_ttf compile-time v%d.%d.%d", TTFCompiledVersion.major, TTFCompiledVersion.minor,
                       TTFCompiledVersion.patch);

        // Look out for windows DPI messages.
        dune::DuneEventWatcher event_watcher;

        const auto okay = run_game(argc, argv);

        // deinit fnkdat
        auto [ok2, tmp2] = fnkdat(FNKDAT_UNINIT);
        if (!ok2) {
            THROW(std::runtime_error, "Cannot uninitialize fnkdat!");
        }

        dune::logging_complete();

        return okay ? EXIT_SUCCESS : EXIT_FAILURE;
    } catch (const std::exception& e) {
        const auto message = fmt::format("An unhandled exception of type \'{}\' was thrown:\n\n"
                                         "{}\n\nDune Legacy will now be terminated!",
                                         demangleSymbol(typeid(e).name()), e.what());
        sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Dune Legacy: Unrecoverable error: %s", message);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Dune Legacy: Unrecoverable error", message.c_str(), nullptr);

        return EXIT_FAILURE;
    }
}
