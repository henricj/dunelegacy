// Derived from SDL2's src/test/SDL_test_common.c
/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "misc/dune_sdlpp.h"

#include <fmt/core.h>

static const char* SDLTest_PixelFormatName(SDL_PixelFormat format) {
    switch (format) {
        case SDL_PIXELFORMAT_UNKNOWN: return "Unknown";
        case SDL_PIXELFORMAT_INDEX1LSB: return "Index1LSB";
        case SDL_PIXELFORMAT_INDEX1MSB: return "Index1MSB";
        case SDL_PIXELFORMAT_INDEX4LSB: return "Index4LSB";
        case SDL_PIXELFORMAT_INDEX4MSB: return "Index4MSB";
        case SDL_PIXELFORMAT_INDEX8: return "Index8";
        case SDL_PIXELFORMAT_RGB332: return "RGB332";
        case SDL_PIXELFORMAT_XRGB4444: return "RGB444";
        case SDL_PIXELFORMAT_XRGB1555: return "RGB555";
        case SDL_PIXELFORMAT_XBGR1555: return "BGR555";
        case SDL_PIXELFORMAT_ARGB4444: return "ARGB4444";
        case SDL_PIXELFORMAT_ABGR4444: return "ABGR4444";
        case SDL_PIXELFORMAT_ARGB1555: return "ARGB1555";
        case SDL_PIXELFORMAT_ABGR1555: return "ABGR1555";
        case SDL_PIXELFORMAT_RGB565: return "RGB565";
        case SDL_PIXELFORMAT_BGR565: return "BGR565";
        case SDL_PIXELFORMAT_RGB24: return "RGB24";
        case SDL_PIXELFORMAT_BGR24: return "BGR24";
        case SDL_PIXELFORMAT_XRGB8888: return "RGB888";
        case SDL_PIXELFORMAT_XBGR8888: return "BGR888";
        case SDL_PIXELFORMAT_ARGB8888: return "ARGB8888";
        case SDL_PIXELFORMAT_RGBA8888: return "RGBA8888";
        case SDL_PIXELFORMAT_ABGR8888: return "ABGR8888";
        case SDL_PIXELFORMAT_BGRA8888: return "BGRA8888";
        case SDL_PIXELFORMAT_ARGB2101010: return "ARGB2101010";
        case SDL_PIXELFORMAT_YV12: return "YV12";
        case SDL_PIXELFORMAT_IYUV: return "IYUV";
        case SDL_PIXELFORMAT_YUY2: return "YUY2";
        case SDL_PIXELFORMAT_UYVY: return "UYVY";
        case SDL_PIXELFORMAT_YVYU: return "YVYU";
        case SDL_PIXELFORMAT_NV12: return "NV12";
        case SDL_PIXELFORMAT_NV21: return "NV21";
        default: return "Unknown";
    }
}

// SDL3: SDL_RendererInfo was removed. Use SDL_GetRendererName() and SDL_GetRendererProperties() instead.
void sdl2::SDL_LogRenderer(SDL_Renderer* renderer) {
    if (!renderer) {
        sdl2::log_info("  Renderer: (null)\n");
        return;
    }

    const char* name = SDL_GetRendererName(renderer);
    sdl2::log_info("  Renderer: {}\n", name ? name : "Unknown");

    // Get renderer output size
    int w = 0, h = 0;
    if (SDL_GetRenderOutputSize(renderer, &w, &h)) {
        sdl2::log_info("    Output Size: {}x{}\n", w, h);
    }

    // Get current render target info via properties
    SDL_PropertiesID props = SDL_GetRendererProperties(renderer);
    if (props) {
        // Log max texture size if available
        int maxW = static_cast<int>(SDL_GetNumberProperty(props, SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER, 0));
        if (maxW > 0) {
            sdl2::log_info("    Max Texture Size: {}x{}\n", maxW, maxW);
        }

        // Check if VSync is enabled
        bool vsync = SDL_GetNumberProperty(props, SDL_PROP_RENDERER_VSYNC_NUMBER, 0) != 0;
        sdl2::log_info("    VSync: {}\n", vsync ? "Enabled" : "Disabled");
    }
}
