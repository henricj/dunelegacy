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

#include <globals.h>

#include <main.h>

#include <config.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/FontManager.h>
#include <FileClasses/GFXManager.h>
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
#include <misc/dune_sdl.h>
#include <misc/dune_sdl_mixer.h>
#include <misc/dune_sdl_ttf.h>
#include <misc/dune_sdlpp.h>
#include <misc/exceptions.h>
#include <misc/fnkdat.h>

// SDL3 main entry point support - provides WinMain on Windows
// This must be included in the file containing main()
#include <SDL3/SDL_main.h>

#include <SoundPlayer.h>

#include <CutScenes/Intro.h>

#include "logging.h"

#include <chrono>
#include <future>
#include <random>
#include <typeinfo>

#ifdef _WIN32
#    ifdef DUNE_CRT_HEAP_DEBUG
#        include <crtdbg.h>
#    endif
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

#ifdef DUNE_CRT_HEAP_DEBUG
struct DuneHeapDebug final {
    DuneHeapDebug() {
        auto tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
        tmpDbgFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
        // tmpDbgFlag |= _CRTDBG_CHECK_CRT_DF;
        tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
        // tmpDbgFlag |= _CRTDBG_CHECK_EVERY_1024_DF;
        _CrtSetDbgFlag(tmpDbgFlag);

        sdl2::log_info("Enabling CRT heap debugging ({})", tmpDbgFlag);
    }
    ~DuneHeapDebug() { _CrtDumpMemoryLeaks(); }
};
#else
struct DuneHeapDebug { };
#endif

struct SDL_handle final {
    SDL_handle(SDL_InitFlags flags) {
        if (!SDL_Init(flags))
            THROW(sdl_error, "Couldn't initialize SDL: {}!", SDL_GetError());
    }
    ~SDL_handle() { SDL_Quit(); }
};

struct TTF_handle final {
    TTF_handle() {
        if (!TTF_Init())
            THROW(sdl_error, "Couldn't initialize SDL3_ttf: {}!", SDL_GetError());
    }
    ~TTF_handle() { TTF_Quit(); }
};

// SDL3_mixer handle - the API has changed significantly in SDL3_mixer
struct MIX_handle final {
    MIX_handle() {
        if (!MIX_Init())
            THROW(sdl_error, "Couldn't initialize SDL3_mixer: {}!", SDL_GetError());
    }
    ~MIX_handle() { MIX_Quit(); }
};

} // namespace

bool run_game(int argc, char* argv[]) {
    const auto startup_start = std::chrono::steady_clock::now();

    bool bExitGame       = false;
    bool bFirstInit      = true;
    bool bFirstGamestart = false;
    bool bAudioEnabled   = false;

    dune::globals::debug       = false;
    dune::globals::cursorFrame = UI_CursorNormal;

    do {
        { // Scope
            DisplayCleanup display_cleanup;
            GlobalCleanup text_cleanup{dune::globals::pTextManager};

            if (configure_game(argc, argv, bFirstInit, currentDisplayIndex))
                bFirstGamestart = true;

            if (bFirstInit) {
                sdl2::log_info("Initializing audio...");

                // SDL3_mixer version info
                const int compiledVersion = SDL_MIXER_VERSION;
                const int linkedVersion   = MIX_VERSION;
                sdl2::log_info("SDL Mixer compile-time v{}.{}.{}",
                               SDL_VERSIONNUM_MAJOR(compiledVersion),
                               SDL_VERSIONNUM_MINOR(compiledVersion),
                               SDL_VERSIONNUM_MICRO(compiledVersion));
                sdl2::log_info("SDL Mixer runtime v{}.{}.{}",
                               SDL_VERSIONNUM_MAJOR(linkedVersion),
                               SDL_VERSIONNUM_MINOR(linkedVersion),
                               SDL_VERSIONNUM_MICRO(linkedVersion));

                // TODO: SDL3_mixer audio initialization
                // SDL3_mixer has a completely new API using MIX_Mixer, MIX_Track, MIX_Audio
                // instead of Mix_OpenAudio/Mix_AllocateChannels/Mix_Chunk/Mix_Music
                // This requires a significant refactoring of the audio system

                // For now, try to initialize SDL3_mixer
                if (MIX_Init()) {
                    bAudioEnabled = true;
                    sdl2::log_info("SDL3_mixer initialized successfully");
                } else {
                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING,
                                             "Dune Legacy: Warning",
                                             "Audio subsystem could not be initialized.",
                                             nullptr);
                    sdl2::log_warn("Audio subsystem could not be initialized: {}", SDL_GetError());
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

            const char* rendererName = SDL_GetRendererName(renderer);
            int maxW = 0, maxH = 0;
            SDL_GetRenderOutputSize(renderer, &maxW, &maxH);
            sdl2::log_info("Renderer: {} (output size: {}x{})", rendererName ? rendererName : "unknown", maxW, maxH);

            static constexpr auto video_default_typeface = "Philosopher-Bold.ttf";

            const auto typeface = settings.video.typeface.empty() || "default" == settings.video.typeface
                                    ? video_default_typeface
                                    : settings.video.typeface;

            sdl2::log_info("Loading fonts from typeface {}...", typeface);
            GlobalCleanup font_cleanup{dune::globals::pFontManager};
            dune::globals::pFontManager = std::make_unique<FontManager>(typeface);

            GUIStyle::setGUIStyle(std::make_unique<DuneStyle>(dune::globals::pFontManager.get()));

            update_display_scale(dune::globals::window.get());

            { // Scope
                int w = 0, h = 0;
                SDL_GetCurrentRenderOutputSize(renderer, &w, &h);
                GUIStyle::getInstance().setLogicalSize(renderer, w, h);
            }

            sdl2::log_info("Loading graphics and sounds...");

            GlobalCleanup sfx_cleanup{dune::globals::pSFXManager};
#ifdef HAVE_STD_ASYNC
            // If we have async, initialize the sounds on another thread while we initialize GFX on this one.
            std::future<std::pair<std::unique_ptr<SFXManager>, std::chrono::nanoseconds>> sfxManagerFut;

            if (bAudioEnabled) {
                sfxManagerFut = std::async(std::launch::async | std::launch::deferred, [] {
                    const auto start   = std::chrono::steady_clock::now();
                    auto ret           = std::make_unique<SFXManager>();
                    const auto elapsed = std::chrono::steady_clock::now() - start;
                    return std::make_pair(std::move(ret), elapsed);
                });
            }
#else
            if (bAudioEnabled)
                dune::globals::pSFXManager = std::make_unique<SFXManager>();
#endif

            GlobalCleanup gfx_cleanup{dune::globals::pGFXManager};
            const auto start = std::chrono::steady_clock::now();
            { // Scope
                const auto size            = getRendererSizePoint();
                dune::globals::pGFXManager = std::make_unique<GFXManager>(renderer, size.x, size.y);
            }
            const auto elapsed = std::chrono::steady_clock::now() - start;

            sdl2::log_info("GFXManager time: {}", std::chrono::duration<double>(elapsed).count());

            const auto* const pGFXManager = dune::globals::pGFXManager.get();

            if (auto* cursor = pGFXManager->getCursor(UI_CursorNormal))
                SDL_SetCursor(cursor);

#ifdef HAVE_STD_ASYNC
            if (sfxManagerFut.valid()) {
                try {
                    auto sfxResult             = sfxManagerFut.get();
                    dune::globals::pSFXManager = std::move(sfxResult.first);
                    sdl2::log_info("SFXManager time: {}", std::chrono::duration<double>(sfxResult.second).count());
                } catch (const std::exception& e) {
                    dune::globals::pSFXManager.reset();
                    const auto message = fmt::sprintf("The sound manager was unable to initialize: '%s' was "
                                                      "thrown:\n\n%s\n\nDune Legacy is unable to play sound!",
                                                      demangleSymbol(typeid(e).name()),
                                                      e.what());
                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Dune Legacy: Warning", message.c_str(), nullptr);
                }
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
            }

            // Playing intro
            if (((bFirstGamestart) || (settings.general.playIntro)) && (bFirstInit)) {
                sdl2::log_info("Playing intro...");
                Intro().run();
            }

            bFirstInit = false;

            sdl2::log_info("Startup time: {:.3f}s",
                           std::chrono::duration<double>(std::chrono::steady_clock::now() - startup_start).count());
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
                // TODO: SDL3_mixer cleanup - MIX_Quit() is called via MIX_handle destructor
                if (bAudioEnabled) {
                    MIX_Quit();
                }
            } else {
                // save the current display index for later reuse
                SDL_DisplayID displayID = SDL_GetDisplayForWindow(dune::globals::window.get());
                // Convert displayID back to index
                int numDisplays         = 0;
                SDL_DisplayID* displays = SDL_GetDisplays(&numDisplays);
                if (displays) {
                    for (int i = 0; i < numDisplays; ++i) {
                        if (displays[i] == displayID) {
                            currentDisplayIndex = i;
                            break;
                        }
                    }
                    SDL_free(displays);
                }
            }
        }

        sdl2::log_info("Deinitialization finished!");
    } while (!bExitGame);

    return true;
}

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
        if (!parseCommandLine(argc, argv, bShowDebugLog)) {
            return EXIT_FAILURE;
        }

        dune::logging_configure(!bShowDebugLog);

        // First check for missing files
        const CaseInsensitiveFilesystemCache filesystemCache(FileManager::getSearchPath());

        const auto missingFiles = PakFileConfiguration::getMissingFiles(filesystemCache);

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

        SDL_handle sdl_handle{SDL_INIT_VIDEO};

        const int compiledVersion = SDL_VERSION;
        const int linkedVersion   = SDL_GetVersion();
        sdl2::log_info("SDL runtime v{}.{}.{}",
                       SDL_VERSIONNUM_MAJOR(linkedVersion),
                       SDL_VERSIONNUM_MINOR(linkedVersion),
                       SDL_VERSIONNUM_MICRO(linkedVersion));
        sdl2::log_info("SDL compile-time v{}.{}.{}",
                       SDL_VERSIONNUM_MAJOR(compiledVersion),
                       SDL_VERSIONNUM_MINOR(compiledVersion),
                       SDL_VERSIONNUM_MICRO(compiledVersion));

        TTF_handle ttf_handle;

        const int TTFCompiledVersion = SDL_TTF_VERSION;
        const int TTFLinkedVersion   = TTF_Version();
        sdl2::log_info("SDL3_ttf runtime v{}.{}.{}",
                       SDL_VERSIONNUM_MAJOR(TTFLinkedVersion),
                       SDL_VERSIONNUM_MINOR(TTFLinkedVersion),
                       SDL_VERSIONNUM_MICRO(TTFLinkedVersion));
        sdl2::log_info("SDL3_ttf compile-time v{}.{}.{}",
                       SDL_VERSIONNUM_MAJOR(TTFCompiledVersion),
                       SDL_VERSIONNUM_MINOR(TTFCompiledVersion),
                       SDL_VERSIONNUM_MICRO(TTFCompiledVersion));

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
                                         demangleSymbol(typeid(e).name()),
                                         e.what());
        sdl2::log_error(SDL_LOG_CATEGORY_APPLICATION, "Dune Legacy: Unrecoverable error: {}", message);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Dune Legacy: Unrecoverable error", message.c_str(), nullptr);

        return EXIT_FAILURE;
    }
}
