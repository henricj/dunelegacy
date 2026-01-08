#pragma once

#include <misc/dune_sdl.h>

namespace sdl2 {

// SDL3: SDL_RendererInfo was removed. Renderer properties are accessed via SDL_GetRendererInfo()
// which returns const char* for the name. This function is deprecated for SDL3 migration.
void SDL_LogRenderer(SDL_Renderer* renderer);

}
