#pragma once

#include <SDL2/SDL.h>

namespace sdl2
{
    void SDL_LogRenderer(const SDL_RendererInfo * info);
}

#ifndef RESTRICT
#if _MSC_VER
#define RESTRICT __restrict
#elif 0
// TODO: Version check for GCC and clang is needed..?
#define RESTRICT __restrict__
#else
#define RESTRICT
#endif
#endif // RESTRICT


