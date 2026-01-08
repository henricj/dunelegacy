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

#include <FileClasses/FileManager.h>

#include <GUI/GUIStyle.h>

#include <misc/dune_sdl.h>
#include <misc/dune_sdlpp.h>
#include <misc/exceptions.h>

#include "logging.h"

#ifdef _WIN32
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <Windows.h>

#    include <ShellScalingApi.h>

#endif // _WIN32

#include <algorithm>
#include <cmath>

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

    if (!shcore || !get_dpi_for_monitor)
        return default_dpi;

    // Get HWND from SDL window using SDL3 properties API
    const auto props = SDL_GetWindowProperties(sdl_window);
    if (!props)
        return default_dpi;

    const auto hwnd = static_cast<HWND>(SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

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

void setVideoMode(int displayIndex) {
    dune::globals::screenTexture.reset();
    dune::globals::renderer.reset();
    dune::globals::window.reset();

    auto& video = dune::globals::settings.video;

    SDL_PropertiesID props = SDL_CreateProperties();

    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Dune Legacy");
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, video.physicalWidth);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, video.physicalHeight);

    Uint64 videoFlags = SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE;

    if (video.fullscreen)
        videoFlags |= SDL_WINDOW_FULLSCREEN;

    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, videoFlags);

    // Get displays
    int numDisplays         = 0;
    SDL_DisplayID* displays = SDL_GetDisplays(&numDisplays);
    SDL_DisplayID displayID =
        (displays && displayIndex < numDisplays) ? displays[displayIndex] : SDL_GetPrimaryDisplay();
    SDL_free(displays);

    // For fullscreen mode, try to find a matching display mode
    if (video.fullscreen) {
        SDL_DisplayMode closestMode{};
        // SDL_GetClosestFullscreenDisplayMode returns bool and fills the output parameter
        if (!SDL_GetClosestFullscreenDisplayMode(
                displayID, video.physicalWidth, video.physicalHeight, 0.0f, false, &closestMode)) {
            sdl2::log_info("Warning: Falling back to a display resolution of 640x480!");
            video.physicalWidth  = 640;
            video.physicalHeight = 480;
            video.width          = 640;
            video.height         = 480;
        } else {
            video.physicalWidth  = closestMode.w;
            video.physicalHeight = closestMode.h;
            video.width          = video.physicalWidth;
            video.height         = video.physicalHeight;
        }
    } else {
        const SDL_DisplayMode* displayMode = SDL_GetDesktopDisplayMode(displayID);

        if (displayMode) {
            if (video.physicalWidth > displayMode->w || video.physicalHeight > displayMode->h) {
                video.physicalWidth  = displayMode->w;
                video.physicalHeight = displayMode->h;
            }
        }

        video.width  = video.physicalWidth;
        video.height = video.physicalHeight;
    }

    sdl2::log_info("Creating {}x{} for {}x{} window with flags {:#08x}",
                   video.physicalWidth,
                   video.physicalHeight,
                   video.width,
                   video.height,
                   static_cast<unsigned>(videoFlags));

    sdl2::window_ptr window{SDL_CreateWindowWithProperties(props)};
    SDL_DestroyProperties(props);

    if (!window)
        THROW(sdl_error, "Unable to create window: {}!", SDL_GetError());

    SDL_SetWindowMinimumSize(window.get(), GUIStyle::MINIMUM_WIDTH, GUIStyle::MINIMUM_HEIGHT);

    { // Scope
        const auto screen_format = SDL_GetWindowPixelFormat(window.get());

        sdl2::log_info("The window is using pixel format {} and the default format is {}",
                       SDL_GetPixelFormatName(screen_format),
                       SDL_GetPixelFormatName(SCREEN_FORMAT));
    }

    sdl2::log_info("Available renderers:");

    { // Scope
        const int n = SDL_GetNumRenderDrivers();

        for (int i = 0; i < n; ++i) {
            const char* driverName = SDL_GetRenderDriver(i);
            if (driverName)
                sdl2::log_info("   {}", driverName);
        }
    }

    if (video.renderer != "default")
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, video.renderer.c_str());

#if defined(_WIN32)
    // Prefer DX12 on Windows
    if (video.renderer == "default" || nullptr == SDL_GetHint(SDL_HINT_RENDER_DRIVER))
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d12");
#    if defined(_DEBUG)
    SDL_SetHint(SDL_HINT_RENDER_DIRECT3D11_DEBUG, "1");
#    endif
#endif

    // Scope
    if (const auto* const render_driver_hint = SDL_GetHint(SDL_HINT_RENDER_DRIVER))
        sdl2::log_info("   requested render driver: {}", render_driver_hint);

    sdl2::renderer_ptr renderer{SDL_CreateRenderer(window.get(), nullptr)};

    if (!renderer)
        THROW(sdl_error, "Unable to create renderer: {}!", SDL_GetError());

    { // Scope
        const char* rendererName = SDL_GetRendererName(renderer.get());
        if (rendererName) {
            sdl2::log_info("Using renderer: {}", rendererName);
        }
    }

    sdl2::texture_ptr screenTexture{
        SDL_CreateTexture(renderer.get(), SCREEN_FORMAT, SDL_TEXTUREACCESS_TARGET, video.width, video.height)};

    if (screenTexture) {
        SDL_PixelFormat screen_format = SDL_PIXELFORMAT_UNKNOWN;
        float tw = 0, th = 0;
        SDL_GetTextureSize(screenTexture.get(), &tw, &th);
        SDL_PropertiesID texProps = SDL_GetTextureProperties(screenTexture.get());
        if (texProps) {
            screen_format = static_cast<SDL_PixelFormat>(
                SDL_GetNumberProperty(texProps, SDL_PROP_TEXTURE_FORMAT_NUMBER, SDL_PIXELFORMAT_UNKNOWN));
        }
        if (screen_format != SCREEN_FORMAT)
            sdl2::log_warn(SDL_LOG_CATEGORY_RENDER, "Actual screen format: {}", SDL_GetPixelFormatName(screen_format));
    }

    dune::globals::window        = std::move(window);
    dune::globals::renderer      = std::move(renderer);
    dune::globals::screenTexture = std::move(screenTexture);
}

void update_display_scale(SDL_Window* sdl_window) {
    // A 14" 640x480 display had a DPI of about 57 (e.g., the IBM 8514 Color Monitor)
    static constexpr auto dune_dpi = 57.f;

    const auto actual_dpi = physical_dpi(sdl_window);

    auto& gui = GUIStyle::getInstance();

    // SDL3: Use display content scale instead of GetDisplayDPI
    const auto displayID = SDL_GetDisplayForWindow(sdl_window);
    float contentScale   = SDL_GetDisplayContentScale(displayID);
    float dpi            = default_dpi * contentScale;

    gui.setDisplayDpi((dpi * actual_dpi) / (default_dpi * dune_dpi));

    auto* sdl_renderer = SDL_GetRenderer(sdl_window);

    float scaleX = NAN;
    float scaleY = NAN;
    SDL_GetRenderScale(sdl_renderer, &scaleX, &scaleY);

    gui.setZoom(scaleX);
}

void showMissingFilesMessageBox(const CaseInsensitiveFilesystemCache& filesystemCache) {
    SDL_ShowCursor();

    std::string instruction =
        "Dune Legacy uses the data files from original Dune II. The following files are missing:\n";

    for (const auto& missingFile : PakFileConfiguration::getMissingFiles(filesystemCache)) {
        instruction += fmt::sprintf(" %s\n", reinterpret_cast<const char*>(missingFile.u8string().c_str()));
        sdl2::log_error("missing required {}", reinterpret_cast<const char*>(missingFile.u8string().c_str()));
    }

    instruction += "\nPut them in one of the following directories and restart Dune Legacy:\n";
    for (const auto& searchPath : FileManager::getSearchPath()) {
        instruction += fmt::sprintf(" %s\n", reinterpret_cast<const char*>(searchPath.u8string().c_str()));
        sdl2::log_info("search path {}", reinterpret_cast<const char*>(searchPath.u8string().c_str()));
    }

    instruction += "\nYou may want to add GERMAN.PAK or FRENCH.PAK for playing in these languages.";

    if (!SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Dune Legacy", instruction.c_str(), nullptr)) {
        const auto error = SDL_GetError();
        sdl2::log_error("message box failed: {}", error);

        fprintf(stderr, "%s\n", instruction.c_str());
    }
}
