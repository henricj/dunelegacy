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

#include "misc/SDL2pp.h"

#include <SDL2/SDL.h>

#include <fmt/core.h>

template<typename... Args>
static void
SDL_snprintfcat(SDL_OUT_Z_CAP(maxlen) char* text, size_t maxlen, fmt::format_string<Args...> fmt, Args&&... args) {
    const size_t length = SDL_strlen(text);

    if (length >= maxlen)
        THROW(std::invalid_argument, "Output buffer overflow!");

    text += length;
    maxlen -= length;

    auto [out, _] = fmt::format_to_n(text, maxlen - 1, fmt, std::forward<Args>(args)...);

    *out = '\0';
}

static void SDLTest_PrintRendererFlag(char* text, size_t maxlen, uint32_t flag) {
    switch (flag) {
        case SDL_RENDERER_SOFTWARE: SDL_snprintfcat(text, maxlen, "Software"); break;
        case SDL_RENDERER_ACCELERATED: SDL_snprintfcat(text, maxlen, "Accelerated"); break;
        case SDL_RENDERER_PRESENTVSYNC: SDL_snprintfcat(text, maxlen, "PresentVSync"); break;
        case SDL_RENDERER_TARGETTEXTURE: SDL_snprintfcat(text, maxlen, "TargetTexturesSupported"); break;
        default: SDL_snprintfcat(text, maxlen, "{:#08x}", flag); break;
    }
}

static void SDLTest_PrintPixelFormat(char* text, size_t maxlen, uint32_t format) {
    switch (format) {
        case SDL_PIXELFORMAT_UNKNOWN: SDL_snprintfcat(text, maxlen, "Unknown"); break;
        case SDL_PIXELFORMAT_INDEX1LSB: SDL_snprintfcat(text, maxlen, "Index1LSB"); break;
        case SDL_PIXELFORMAT_INDEX1MSB: SDL_snprintfcat(text, maxlen, "Index1MSB"); break;
        case SDL_PIXELFORMAT_INDEX4LSB: SDL_snprintfcat(text, maxlen, "Index4LSB"); break;
        case SDL_PIXELFORMAT_INDEX4MSB: SDL_snprintfcat(text, maxlen, "Index4MSB"); break;
        case SDL_PIXELFORMAT_INDEX8: SDL_snprintfcat(text, maxlen, "Index8"); break;
        case SDL_PIXELFORMAT_RGB332: SDL_snprintfcat(text, maxlen, "RGB332"); break;
        case SDL_PIXELFORMAT_RGB444: SDL_snprintfcat(text, maxlen, "RGB444"); break;
        case SDL_PIXELFORMAT_RGB555: SDL_snprintfcat(text, maxlen, "RGB555"); break;
        case SDL_PIXELFORMAT_BGR555: SDL_snprintfcat(text, maxlen, "BGR555"); break;
        case SDL_PIXELFORMAT_ARGB4444: SDL_snprintfcat(text, maxlen, "ARGB4444"); break;
        case SDL_PIXELFORMAT_ABGR4444: SDL_snprintfcat(text, maxlen, "ABGR4444"); break;
        case SDL_PIXELFORMAT_ARGB1555: SDL_snprintfcat(text, maxlen, "ARGB1555"); break;
        case SDL_PIXELFORMAT_ABGR1555: SDL_snprintfcat(text, maxlen, "ABGR1555"); break;
        case SDL_PIXELFORMAT_RGB565: SDL_snprintfcat(text, maxlen, "RGB565"); break;
        case SDL_PIXELFORMAT_BGR565: SDL_snprintfcat(text, maxlen, "BGR565"); break;
        case SDL_PIXELFORMAT_RGB24: SDL_snprintfcat(text, maxlen, "RGB24"); break;
        case SDL_PIXELFORMAT_BGR24: SDL_snprintfcat(text, maxlen, "BGR24"); break;
        case SDL_PIXELFORMAT_RGB888: SDL_snprintfcat(text, maxlen, "RGB888"); break;
        case SDL_PIXELFORMAT_BGR888: SDL_snprintfcat(text, maxlen, "BGR888"); break;
        case SDL_PIXELFORMAT_ARGB8888: SDL_snprintfcat(text, maxlen, "ARGB8888"); break;
        case SDL_PIXELFORMAT_RGBA8888: SDL_snprintfcat(text, maxlen, "RGBA8888"); break;
        case SDL_PIXELFORMAT_ABGR8888: SDL_snprintfcat(text, maxlen, "ABGR8888"); break;
        case SDL_PIXELFORMAT_BGRA8888: SDL_snprintfcat(text, maxlen, "BGRA8888"); break;
        case SDL_PIXELFORMAT_ARGB2101010: SDL_snprintfcat(text, maxlen, "ARGB2101010"); break;
        case SDL_PIXELFORMAT_YV12: SDL_snprintfcat(text, maxlen, "YV12"); break;
        case SDL_PIXELFORMAT_IYUV: SDL_snprintfcat(text, maxlen, "IYUV"); break;
        case SDL_PIXELFORMAT_YUY2: SDL_snprintfcat(text, maxlen, "YUY2"); break;
        case SDL_PIXELFORMAT_UYVY: SDL_snprintfcat(text, maxlen, "UYVY"); break;
        case SDL_PIXELFORMAT_YVYU: SDL_snprintfcat(text, maxlen, "YVYU"); break;
        case SDL_PIXELFORMAT_NV12: SDL_snprintfcat(text, maxlen, "NV12"); break;
        case SDL_PIXELFORMAT_NV21: SDL_snprintfcat(text, maxlen, "NV21"); break;
        default: SDL_snprintfcat(text, maxlen, "0x%8.8x", format); break;
    }
}

namespace sdl2 {
void SDL_LogRenderer(const SDL_RendererInfo* info) {
    std::array<char, 1024> text{};

    sdl2::log_info("  Renderer {}:\n", info->name);

    SDL_snprintfcat(text.data(), text.size(), "    Flags: {:#08X}", info->flags);
    SDL_snprintfcat(text.data(), text.size(), " (");
    int count = 0;
    for (auto i = 0U; i < 8U * sizeof info->flags; ++i) {
        const auto flag = 1U << i;
        if (info->flags & flag) {
            if (count > 0) {
                SDL_snprintfcat(text.data(), text.size(), " | ");
            }
            SDLTest_PrintRendererFlag(text.data(), text.size(), flag);
            ++count;
        }
    }
    SDL_snprintfcat(text.data(), text.size(), ")");
    sdl2::log_info("{}\n", text.data());

    text[0] = '\0';
    SDL_snprintfcat(text.data(), text.size(), "    Texture formats ({}): ", info->num_texture_formats);
    for (auto i = 0; i < static_cast<int>(info->num_texture_formats); ++i) {
        if (i > 0) {
            SDL_snprintfcat(text.data(), text.size(), ", ");
        }
        SDLTest_PrintPixelFormat(text.data(), text.size(), info->texture_formats[i]);
    }
    sdl2::log_info("{}\n", text.data());

    if (info->max_texture_width || info->max_texture_height) {
        sdl2::log_info("    Max Texture Size: {}x{}\n", info->max_texture_width, info->max_texture_height);
    }
}
} // namespace sdl2
