#include <SDL2/SDL.h>

static void SDL_snprintfcat(SDL_OUT_Z_CAP(maxlen) char* text, size_t maxlen, SDL_PRINTF_FORMAT_STRING const char* fmt, ...) {
    const size_t length = SDL_strlen(text);
    va_list ap;

    va_start(ap, fmt);
    text += length;
    maxlen -= length;
    SDL_vsnprintf(text, maxlen, fmt, ap);
    va_end(ap);
}

static void
SDLTest_PrintRendererFlag(char* text, size_t maxlen, uint32_t flag) {
    switch (flag) {
        case SDL_RENDERER_SOFTWARE:
            SDL_snprintfcat(text, maxlen, "Software");
            break;
        case SDL_RENDERER_ACCELERATED:
            SDL_snprintfcat(text, maxlen, "Accelerated");
            break;
        case SDL_RENDERER_PRESENTVSYNC:
            SDL_snprintfcat(text, maxlen, "PresentVSync");
            break;
        case SDL_RENDERER_TARGETTEXTURE:
            SDL_snprintfcat(text, maxlen, "TargetTexturesSupported");
            break;
        default:
            SDL_snprintfcat(text, maxlen, "0x%8.8x", flag);
            break;
    }
}

static void
SDLTest_PrintPixelFormat(char* text, size_t maxlen, uint32_t format) {
    switch (format) {
        case SDL_PIXELFORMAT_UNKNOWN:
            SDL_snprintfcat(text, maxlen, "Unknown");
            break;
        case SDL_PIXELFORMAT_INDEX1LSB:
            SDL_snprintfcat(text, maxlen, "Index1LSB");
            break;
        case SDL_PIXELFORMAT_INDEX1MSB:
            SDL_snprintfcat(text, maxlen, "Index1MSB");
            break;
        case SDL_PIXELFORMAT_INDEX4LSB:
            SDL_snprintfcat(text, maxlen, "Index4LSB");
            break;
        case SDL_PIXELFORMAT_INDEX4MSB:
            SDL_snprintfcat(text, maxlen, "Index4MSB");
            break;
        case SDL_PIXELFORMAT_INDEX8:
            SDL_snprintfcat(text, maxlen, "Index8");
            break;
        case SDL_PIXELFORMAT_RGB332:
            SDL_snprintfcat(text, maxlen, "RGB332");
            break;
        case SDL_PIXELFORMAT_RGB444:
            SDL_snprintfcat(text, maxlen, "RGB444");
            break;
        case SDL_PIXELFORMAT_RGB555:
            SDL_snprintfcat(text, maxlen, "RGB555");
            break;
        case SDL_PIXELFORMAT_BGR555:
            SDL_snprintfcat(text, maxlen, "BGR555");
            break;
        case SDL_PIXELFORMAT_ARGB4444:
            SDL_snprintfcat(text, maxlen, "ARGB4444");
            break;
        case SDL_PIXELFORMAT_ABGR4444:
            SDL_snprintfcat(text, maxlen, "ABGR4444");
            break;
        case SDL_PIXELFORMAT_ARGB1555:
            SDL_snprintfcat(text, maxlen, "ARGB1555");
            break;
        case SDL_PIXELFORMAT_ABGR1555:
            SDL_snprintfcat(text, maxlen, "ABGR1555");
            break;
        case SDL_PIXELFORMAT_RGB565:
            SDL_snprintfcat(text, maxlen, "RGB565");
            break;
        case SDL_PIXELFORMAT_BGR565:
            SDL_snprintfcat(text, maxlen, "BGR565");
            break;
        case SDL_PIXELFORMAT_RGB24:
            SDL_snprintfcat(text, maxlen, "RGB24");
            break;
        case SDL_PIXELFORMAT_BGR24:
            SDL_snprintfcat(text, maxlen, "BGR24");
            break;
        case SDL_PIXELFORMAT_RGB888:
            SDL_snprintfcat(text, maxlen, "RGB888");
            break;
        case SDL_PIXELFORMAT_BGR888:
            SDL_snprintfcat(text, maxlen, "BGR888");
            break;
        case SDL_PIXELFORMAT_ARGB8888:
            SDL_snprintfcat(text, maxlen, "ARGB8888");
            break;
        case SDL_PIXELFORMAT_RGBA8888:
            SDL_snprintfcat(text, maxlen, "RGBA8888");
            break;
        case SDL_PIXELFORMAT_ABGR8888:
            SDL_snprintfcat(text, maxlen, "ABGR8888");
            break;
        case SDL_PIXELFORMAT_BGRA8888:
            SDL_snprintfcat(text, maxlen, "BGRA8888");
            break;
        case SDL_PIXELFORMAT_ARGB2101010:
            SDL_snprintfcat(text, maxlen, "ARGB2101010");
            break;
        case SDL_PIXELFORMAT_YV12:
            SDL_snprintfcat(text, maxlen, "YV12");
            break;
        case SDL_PIXELFORMAT_IYUV:
            SDL_snprintfcat(text, maxlen, "IYUV");
            break;
        case SDL_PIXELFORMAT_YUY2:
            SDL_snprintfcat(text, maxlen, "YUY2");
            break;
        case SDL_PIXELFORMAT_UYVY:
            SDL_snprintfcat(text, maxlen, "UYVY");
            break;
        case SDL_PIXELFORMAT_YVYU:
            SDL_snprintfcat(text, maxlen, "YVYU");
            break;
        case SDL_PIXELFORMAT_NV12:
            SDL_snprintfcat(text, maxlen, "NV12");
            break;
        case SDL_PIXELFORMAT_NV21:
            SDL_snprintfcat(text, maxlen, "NV21");
            break;
        default:
            SDL_snprintfcat(text, maxlen, "0x%8.8x", format);
            break;
    }
}

namespace sdl2 {
void SDL_LogRenderer(const SDL_RendererInfo* info) {
    char text[1024];

    sdl2::log_info("  Renderer %s:\n", info->name);

    SDL_snprintf(text, sizeof text, "    Flags: 0x%8.8X", info->flags);
    SDL_snprintfcat(text, sizeof text, " (");
    int count = 0;
    for (auto i = 0; i < sizeof info->flags * 8; ++i) {
        const uint32_t flag = 1 << i;
        if (info->flags & flag) {
            if (count > 0) {
                SDL_snprintfcat(text, sizeof text, " | ");
            }
            SDLTest_PrintRendererFlag(text, sizeof text, flag);
            ++count;
        }
    }
    SDL_snprintfcat(text, sizeof text, ")");
    sdl2::log_info("%s\n", text);

    SDL_snprintf(text, sizeof text, "    Texture formats (%d): ", info->num_texture_formats);
    for (auto i = 0; i < static_cast<int>(info->num_texture_formats); ++i) {
        if (i > 0) {
            SDL_snprintfcat(text, sizeof text, ", ");
        }
        SDLTest_PrintPixelFormat(text, sizeof text, info->texture_formats[i]);
    }
    sdl2::log_info("%s\n", text);

    if (info->max_texture_width || info->max_texture_height) {
        sdl2::log_info("    Max Texture Size: %dx%d\n",
                       info->max_texture_width, info->max_texture_height);
    }
}
} // namespace sdl2
