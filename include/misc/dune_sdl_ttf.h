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

#ifndef DUNE_LEGACY_SDL_TTF_H
#define DUNE_LEGACY_SDL_TTF_H

/**
 * @file dune_sdl_ttf.h
 * @brief Centralized SDL_ttf include header for Dune Legacy.
 *
 * This header provides a single point of control for SDL_ttf includes.
 * Do NOT include SDL_ttf headers directly in other source files.
 */

#include <misc/dune_sdl.h>

#include <SDL3_ttf/SDL_ttf.h>

// ============================================================================
// SDL3_ttf Compatibility Layer
// 
// SDL3_ttf has API changes from SDL2_ttf:
// - TTF_OpenFontRW -> TTF_OpenFontIO
// - TTF_RenderUTF8_Blended -> TTF_RenderText_Blended (now takes const char*, not std::string)
// - TTF_SizeUTF8 -> TTF_GetStringSize
// - TTF_GetError -> SDL_GetError (unified error handling)
// - TTF_FontLineSkip -> TTF_GetFontLineSkip
// ============================================================================

// TTF_GetError is now SDL_GetError in SDL3
#define TTF_GetError SDL_GetError

// TTF_FontLineSkip -> TTF_GetFontLineSkip
#define TTF_FontLineSkip TTF_GetFontLineSkip

// TTF_OpenFontRW compatibility wrapper
// SDL2: TTF_OpenFontRW(SDL_RWops*, int freesrc, int ptsize)
// SDL3: TTF_OpenFontIO(SDL_IOStream*, bool closeio, float ptsize)
inline TTF_Font* dune_compat_TTF_OpenFontRW(SDL_IOStream* src, int freesrc, int ptsize) {
    return TTF_OpenFontIO(src, freesrc != 0, static_cast<float>(ptsize));
}
#define TTF_OpenFontRW dune_compat_TTF_OpenFontRW

// TTF_RenderUTF8_Blended compatibility wrapper
// SDL2: TTF_RenderUTF8_Blended(font, text, fg)
// SDL3: TTF_RenderText_Blended(font, text, length, fg) - length 0 means null-terminated
inline SDL_Surface* dune_compat_TTF_RenderUTF8_Blended(TTF_Font* font, const char* text, SDL_Color fg) {
    return TTF_RenderText_Blended(font, text, 0, fg);
}
#define TTF_RenderUTF8_Blended dune_compat_TTF_RenderUTF8_Blended

// TTF_SizeUTF8 compatibility wrapper
// SDL2: int TTF_SizeUTF8(font, text, *w, *h) - returns 0 on success, -1 on error
// SDL3: bool TTF_GetStringSize(font, text, length, *w, *h) - returns true on success
inline int dune_compat_TTF_SizeUTF8(TTF_Font* font, const char* text, int* w, int* h) {
    return TTF_GetStringSize(font, text, 0, w, h) ? 0 : -1;
}
#define TTF_SizeUTF8 dune_compat_TTF_SizeUTF8

#endif // DUNE_LEGACY_SDL_TTF_H
