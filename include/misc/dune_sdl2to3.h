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

#ifndef DUNE_SDL2TO3_H
#define DUNE_SDL2TO3_H

/**
 * @file dune_sdl2to3.h
 * @brief Dune Legacy SDL2 to SDL3 compatibility macros for incremental migration.
 *
 * This header provides macro mappings from SDL2 API names to their SDL3 equivalents.
 * It allows the codebase to compile with SDL3 while individual files are migrated.
 *
 * MIGRATION STRATEGY:
 * 1. This header is included via dune_sdl.h
 * 2. As each file is migrated to native SDL3 APIs, remove the corresponding macros
 * 3. Once all files are migrated, delete this header entirely
 *
 * Each macro is marked with the file(s) that still depend on it.
 * When migrating a file, search for its name here and remove those macros.
 */

// ============================================================================
// EVENT TYPE COMPATIBILITY
// Files: MenuBase.cpp, Window.cpp, Game.cpp, GameInput.cpp, dune_events.cpp
// ============================================================================

#define SDL_KEYDOWN             SDL_EVENT_KEY_DOWN
#define SDL_KEYUP               SDL_EVENT_KEY_UP
#define SDL_QUIT                SDL_EVENT_QUIT
#define SDL_MOUSEMOTION         SDL_EVENT_MOUSE_MOTION
#define SDL_MOUSEBUTTONDOWN     SDL_EVENT_MOUSE_BUTTON_DOWN
#define SDL_MOUSEBUTTONUP       SDL_EVENT_MOUSE_BUTTON_UP
#define SDL_MOUSEWHEEL          SDL_EVENT_MOUSE_WHEEL
#define SDL_TEXTINPUT           SDL_EVENT_TEXT_INPUT
#define SDL_TEXTEDITING         SDL_EVENT_TEXT_EDITING

// Window events - SDL3 flattens these to individual event types
// Files: Window.cpp, dune_events.cpp
#define SDL_WINDOWEVENT                     SDL_EVENT_WINDOW_FIRST
#define SDL_WINDOWEVENT_SIZE_CHANGED        SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED
#define SDL_WINDOWEVENT_RESIZED             SDL_EVENT_WINDOW_RESIZED
#define SDL_WINDOWEVENT_DISPLAY_CHANGED     SDL_EVENT_WINDOW_DISPLAY_CHANGED
#define SDL_WINDOWEVENT_MOVED               SDL_EVENT_WINDOW_MOVED
#define SDL_WINDOWEVENT_SHOWN               SDL_EVENT_WINDOW_SHOWN
#define SDL_WINDOWEVENT_HIDDEN              SDL_EVENT_WINDOW_HIDDEN
#define SDL_WINDOWEVENT_EXPOSED             SDL_EVENT_WINDOW_EXPOSED
#define SDL_WINDOWEVENT_MINIMIZED           SDL_EVENT_WINDOW_MINIMIZED
#define SDL_WINDOWEVENT_MAXIMIZED           SDL_EVENT_WINDOW_MAXIMIZED
#define SDL_WINDOWEVENT_RESTORED            SDL_EVENT_WINDOW_RESTORED
#define SDL_WINDOWEVENT_ENTER               SDL_EVENT_WINDOW_MOUSE_ENTER
#define SDL_WINDOWEVENT_LEAVE               SDL_EVENT_WINDOW_MOUSE_LEAVE
#define SDL_WINDOWEVENT_FOCUS_GAINED        SDL_EVENT_WINDOW_FOCUS_GAINED
#define SDL_WINDOWEVENT_FOCUS_LOST          SDL_EVENT_WINDOW_FOCUS_LOST
#define SDL_WINDOWEVENT_CLOSE               SDL_EVENT_WINDOW_CLOSE_REQUESTED

// Display events
#define SDL_DISPLAYEVENT_ORIENTATION        SDL_EVENT_DISPLAY_ORIENTATION

// Render events  
#define SDL_RENDER_DEVICE_RESET             SDL_EVENT_RENDER_DEVICE_RESET

// ============================================================================
// MOUSE BUTTON COMPATIBILITY
// Files: MentatMenu.cpp
// ============================================================================

// SDL_BUTTON was renamed to SDL_BUTTON_MASK in SDL3
#define SDL_BUTTON      SDL_BUTTON_MASK

// ============================================================================
// WINDOW FLAGS COMPATIBILITY
// Files: Fullscreen.cpp
// ============================================================================

// SDL3: SDL_WINDOW_FULLSCREEN_DESKTOP removed, use SDL_WINDOW_FULLSCREEN
#define SDL_WINDOW_FULLSCREEN_DESKTOP   SDL_WINDOW_FULLSCREEN

// ============================================================================
// DISPLAY MODE COMPATIBILITY
// Files: OptionsMenu.cpp
// ============================================================================

// SDL3: SDL_GetWindowDisplayIndex renamed to SDL_GetDisplayForWindow
#define SDL_GetWindowDisplayIndex   SDL_GetDisplayForWindow

// ============================================================================
// KEYBOARD MODIFIER COMPATIBILITY
// Files: MenuBase.cpp, Button.cpp, InGameMenu.cpp, Game.cpp
// ============================================================================

#define KMOD_NONE       SDL_KMOD_NONE
#define KMOD_LSHIFT     SDL_KMOD_LSHIFT
#define KMOD_RSHIFT     SDL_KMOD_RSHIFT
#define KMOD_SHIFT      SDL_KMOD_SHIFT
#define KMOD_LCTRL      SDL_KMOD_LCTRL
#define KMOD_RCTRL      SDL_KMOD_RCTRL
#define KMOD_CTRL       SDL_KMOD_CTRL
#define KMOD_LALT       SDL_KMOD_LALT
#define KMOD_RALT       SDL_KMOD_RALT
#define KMOD_ALT        SDL_KMOD_ALT
#define KMOD_LGUI       SDL_KMOD_LGUI
#define KMOD_RGUI       SDL_KMOD_RGUI
#define KMOD_GUI        SDL_KMOD_GUI
#define KMOD_NUM        SDL_KMOD_NUM
#define KMOD_CAPS       SDL_KMOD_CAPS
#define KMOD_MODE       SDL_KMOD_MODE
#define KMOD_SCROLL     SDL_KMOD_SCROLL

// ============================================================================
// KEYCODE COMPATIBILITY - SDL3 changed lowercase to uppercase
// Files: GameInput.cpp, Button.cpp, ListBox.cpp, etc.
// ============================================================================

// SDL3 renamed all lowercase keycodes to uppercase
// These are direct renames, not semantic changes
#define SDLK_a  SDLK_A
#define SDLK_b  SDLK_B
#define SDLK_c  SDLK_C
#define SDLK_d  SDLK_D
#define SDLK_e  SDLK_E
#define SDLK_f  SDLK_F
#define SDLK_g  SDLK_G
#define SDLK_h  SDLK_H
#define SDLK_i  SDLK_I
#define SDLK_j  SDLK_J
#define SDLK_k  SDLK_K
#define SDLK_l  SDLK_L
#define SDLK_m  SDLK_M
#define SDLK_n  SDLK_N
#define SDLK_o  SDLK_O
#define SDLK_p  SDLK_P
#define SDLK_q  SDLK_Q
#define SDLK_r  SDLK_R
#define SDLK_s  SDLK_S
#define SDLK_t  SDLK_T
#define SDLK_u  SDLK_U
#define SDLK_v  SDLK_V
#define SDLK_w  SDLK_W
#define SDLK_x  SDLK_X
#define SDLK_y  SDLK_Y
#define SDLK_z  SDLK_Z

// ============================================================================
// PALETTE COMPATIBILITY
// Files: Palette.cpp, Palfile.cpp
// ============================================================================

#define SDL_AllocPalette        SDL_CreatePalette
#define SDL_FreePalette         SDL_DestroyPalette

// ============================================================================
// SURFACE FUNCTION COMPATIBILITY
// Files: draw_util.cpp, Scaler.cpp, BlendBlitter.cpp, SurfaceLoader.cpp, 
//        PictureFactory.cpp, dune_sdlpp.h
// ============================================================================

#define SDL_FreeSurface             SDL_DestroySurface
#define SDL_HasColorKey             SDL_SurfaceHasColorKey
#define SDL_SetColorKey             SDL_SetSurfaceColorKey
#define SDL_GetColorKey             SDL_GetSurfaceColorKey
#define SDL_FillRect                SDL_FillSurfaceRect
#define SDL_FillRects               SDL_FillSurfaceRects
#define SDL_SetClipRect             SDL_SetSurfaceClipRect
#define SDL_GetClipRect             SDL_GetSurfaceClipRect
#define SDL_UpperBlit               SDL_BlitSurface
#define SDL_BlitScaled              SDL_BlitSurfaceScaled

// ============================================================================
// RWOPS → IOSTREAM COMPATIBILITY (simple macros only)
// ============================================================================

#define SDL_RWops               SDL_IOStream
#define SDL_RWFromFile          SDL_IOFromFile
#define SDL_RWFromMem           SDL_IOFromMem
#define SDL_RWFromConstMem      SDL_IOFromConstMem
#define SDL_RWclose             SDL_CloseIO
#define SDL_RWsize              SDL_GetIOSize
#define SDL_RWseek              SDL_SeekIO
#define SDL_RWtell              SDL_TellIO

// Seek constants
#define RW_SEEK_SET             SDL_IO_SEEK_SET
#define RW_SEEK_CUR             SDL_IO_SEEK_CUR
#define RW_SEEK_END             SDL_IO_SEEK_END

// ============================================================================
// RENDERER FUNCTION COMPATIBILITY
// Files: DuneRenderer.cpp, DuneTexture.cpp, draw_util.cpp, MenuBase.cpp
// ============================================================================

#define SDL_RenderSetClipRect       SDL_SetRenderClipRect
#define SDL_RenderGetClipRect       SDL_GetRenderClipRect
#define SDL_GetRendererOutputSize   SDL_GetRenderOutputSize

// SDL_RenderIsClipEnabled renamed to SDL_RenderClipEnabled in SDL3
#define SDL_RenderIsClipEnabled     SDL_RenderClipEnabled

// SDL_HasIntersection renamed to SDL_HasRectIntersection in SDL3
#define SDL_HasIntersection         SDL_HasRectIntersection

// SDL_RenderFillRectF was renamed to SDL_RenderFillRect in SDL3
// (SDL3 uses float rects by default)
#define SDL_RenderFillRectF         SDL_RenderFillRect

// SDL_RenderFillRectsF was renamed to SDL_RenderFillRects in SDL3
#define SDL_RenderFillRectsF        SDL_RenderFillRects

// SDL_RenderFlush renamed
#define SDL_RenderFlush             SDL_FlushRenderer

// SDL_RendererFlip is now SDL_FlipMode
#define SDL_RendererFlip        SDL_FlipMode

// ============================================================================
// SDL_BlendMode COMPATIBILITY
// SDL3 changed SDL_BlendMode from an enum to defines (flags)
// ============================================================================

// In SDL3, SDL_BLENDMODE_* are not in an enum class/namespace
// They are just #defines, so SDL_BlendMode::SDL_BLENDMODE_BLEND doesn't work
// Use SDL_BLENDMODE_BLEND directly
// Manually update code to remove SDL_BlendMode:: scope

// ============================================================================
// AUDIO FORMAT COMPATIBILITY
// Files: SoundPlayer.cpp, SFXManager.cpp, sound_util.cpp, ADLPlayer.cpp
// ============================================================================

#define AUDIO_S8        SDL_AUDIO_S8
#define AUDIO_U8        SDL_AUDIO_U8
#define AUDIO_S16       SDL_AUDIO_S16LE
#define AUDIO_S16LSB    SDL_AUDIO_S16LE
#define AUDIO_S16MSB    SDL_AUDIO_S16BE
#define AUDIO_S16SYS    SDL_AUDIO_S16
#define AUDIO_S32       SDL_AUDIO_S32LE
#define AUDIO_S32LSB    SDL_AUDIO_S32LE
#define AUDIO_S32MSB    SDL_AUDIO_S32BE
#define AUDIO_S32SYS    SDL_AUDIO_S32
#define AUDIO_F32       SDL_AUDIO_F32LE
#define AUDIO_F32LSB    SDL_AUDIO_F32LE
#define AUDIO_F32MSB    SDL_AUDIO_F32BE
#define AUDIO_F32SYS    SDL_AUDIO_F32

// SDL3 removed unsigned 16-bit audio formats
// Map to signed equivalents (will need conversion in audio code)
#define AUDIO_U16LSB    SDL_AUDIO_S16LE
#define AUDIO_U16MSB    SDL_AUDIO_S16BE
#define AUDIO_U16SYS    SDL_AUDIO_S16
#define AUDIO_U16       SDL_AUDIO_S16LE

// ============================================================================
// MISCELLANEOUS COMPATIBILITY
// ============================================================================

#define SDL_TRUE    true
#define SDL_FALSE   false

// Threading types renamed
#define SDL_sem     SDL_Semaphore
#define SDL_mutex   SDL_Mutex
#define SDL_cond    SDL_Condition

// Threading functions renamed
#define SDL_SemPost     SDL_SignalSemaphore

// ============================================================================
// BYTE SWAPPING COMPATIBILITY
// Files: Cpsfile.cpp, Shpfile.cpp, etc.
// ============================================================================

#define SDL_SwapLE16    SDL_Swap16LE
#define SDL_SwapLE32    SDL_Swap32LE
#define SDL_SwapLE64    SDL_Swap64LE
#define SDL_SwapBE16    SDL_Swap16BE
#define SDL_SwapBE32    SDL_Swap32BE
#define SDL_SwapBE64    SDL_Swap64BE

// ============================================================================
// CURSOR COMPATIBILITY
// Files: GFXManager.cpp
// ============================================================================

#define SDL_SYSTEM_CURSOR_ARROW     SDL_SYSTEM_CURSOR_DEFAULT

// ============================================================================
// IO SIZE COMPATIBILITY
// Files: FontManager.cpp
// ============================================================================

#define SDL_SizeIO      SDL_GetIOSize

// ============================================================================
// C++ ONLY COMPATIBILITY WRAPPERS
// These require C++ features and should only be used in C++ code
// ============================================================================

#ifdef __cplusplus

// SDL_ConvertSurfaceFormat compatibility
// SDL2: SDL_ConvertSurfaceFormat(surface, format, flags)
// SDL3: SDL_ConvertSurface(surface, format) - flags removed
inline SDL_Surface* dune_compat_SDL_ConvertSurfaceFormat(
    SDL_Surface* surface,
    SDL_PixelFormat format,
    Uint32 flags) {
    (void)flags;
    return SDL_ConvertSurface(surface, format);
}
#define SDL_ConvertSurfaceFormat dune_compat_SDL_ConvertSurfaceFormat

// SDL_CreateRGBSurfaceWithFormat compatibility
// SDL2: SDL_CreateRGBSurfaceWithFormat(flags, w, h, depth, format)
// SDL3: SDL_CreateSurface(w, h, format)
inline SDL_Surface* dune_compat_SDL_CreateRGBSurfaceWithFormat(
    Uint32 flags, 
    int w, int h, 
    int depth, 
    SDL_PixelFormat format) {
    (void)flags;
    (void)depth;
    return SDL_CreateSurface(w, h, format);
}
#define SDL_CreateRGBSurfaceWithFormat dune_compat_SDL_CreateRGBSurfaceWithFormat

// SDL_CreateRGBSurface compatibility
// SDL2: SDL_CreateRGBSurface(flags, w, h, depth, Rmask, Gmask, Bmask, Amask)
// SDL3: Use SDL_CreateSurface with appropriate format
// NOTE: SDL3 doesn't automatically create a palette for 8-bit surfaces, we must do it manually
inline SDL_Surface* dune_compat_SDL_CreateRGBSurface(
    Uint32 flags,
    int w, int h, int depth,
    Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {
    (void)flags;
    SDL_PixelFormat format = SDL_GetPixelFormatForMasks(depth, Rmask, Gmask, Bmask, Amask);
    SDL_Surface* surface = SDL_CreateSurface(w, h, format);
    
    // SDL3 doesn't automatically create a palette for indexed surfaces
    // We need to create one for 8-bit (and lower) surfaces
    if (surface && depth <= 8) {
        int ncolors = 1 << depth;
        SDL_Palette* palette = SDL_CreatePalette(ncolors);
        if (palette) {
            // Initialize to grayscale ramp (similar to what SDL2 did)
            for (int i = 0; i < ncolors; ++i) {
                palette->colors[i].r = static_cast<Uint8>(i * 255 / (ncolors - 1));
                palette->colors[i].g = static_cast<Uint8>(i * 255 / (ncolors - 1));
                palette->colors[i].b = static_cast<Uint8>(i * 255 / (ncolors - 1));
                palette->colors[i].a = 255;
            }
            SDL_SetSurfacePalette(surface, palette);
            SDL_DestroyPalette(palette); // Surface now owns a reference
        }
    }
    
    return surface;
}
#define SDL_CreateRGBSurface dune_compat_SDL_CreateRGBSurface

// SDL_RWread compatibility wrapper
// SDL2: size_t SDL_RWread(ctx, ptr, size, maxnum) - returns items read
// SDL3: size_t SDL_ReadIO(ctx, ptr, size) - returns bytes read
inline size_t dune_compat_SDL_RWread(
    SDL_IOStream* ctx,
    void* ptr,
    size_t size,
    size_t maxnum) {
    const size_t total_bytes = size * maxnum;
    const size_t bytes_read = SDL_ReadIO(ctx, ptr, total_bytes);
    return (size > 0) ? (bytes_read / size) : 0;
}
#define SDL_RWread dune_compat_SDL_RWread

// SDL_RWwrite compatibility wrapper
// SDL2: size_t SDL_RWwrite(ctx, ptr, size, num) - returns items written
// SDL3: size_t SDL_WriteIO(ctx, ptr, size) - returns bytes written
inline size_t dune_compat_SDL_RWwrite(
    SDL_IOStream* ctx,
    const void* ptr,
    size_t size,
    size_t num) {
    const size_t total_bytes = size * num;
    const size_t bytes_written = SDL_WriteIO(ctx, ptr, total_bytes);
    return (size > 0) ? (bytes_written / size) : 0;
}
#define SDL_RWwrite dune_compat_SDL_RWwrite

// SDL_RenderCopy compatibility
// SDL2: int SDL_RenderCopy(renderer, texture, srcrect, dstrect) - uses SDL_Rect
// SDL3: bool SDL_RenderTexture(renderer, texture, srcrect, dstrect) - uses SDL_FRect
inline int dune_compat_SDL_RenderCopy(
    SDL_Renderer* renderer,
    SDL_Texture* texture,
    const SDL_Rect* srcrect,
    const SDL_Rect* dstrect) {
    SDL_FRect fsrc{}, fdst{};
    SDL_FRect* pfsrc = nullptr;
    SDL_FRect* pfdst = nullptr;
    
    if (srcrect) {
        fsrc.x = static_cast<float>(srcrect->x);
        fsrc.y = static_cast<float>(srcrect->y);
        fsrc.w = static_cast<float>(srcrect->w);
        fsrc.h = static_cast<float>(srcrect->h);
        pfsrc = &fsrc;
    }
    if (dstrect) {
        fdst.x = static_cast<float>(dstrect->x);
        fdst.y = static_cast<float>(dstrect->y);
        fdst.w = static_cast<float>(dstrect->w);
        fdst.h = static_cast<float>(dstrect->h);
        pfdst = &fdst;
    }
    
    return SDL_RenderTexture(renderer, texture, pfsrc, pfdst) ? 0 : -1;
}
#define SDL_RenderCopy dune_compat_SDL_RenderCopy

// SDL_RenderCopyEx compatibility
// SDL2: int SDL_RenderCopyEx(renderer, texture, srcrect, dstrect, angle, center, flip)
// SDL3: bool SDL_RenderTextureRotated(renderer, texture, srcrect, dstrect, angle, center, flip)
inline int dune_compat_SDL_RenderCopyEx(
    SDL_Renderer* renderer,
    SDL_Texture* texture,
    const SDL_Rect* srcrect,
    const SDL_Rect* dstrect,
    double angle,
    const SDL_Point* center,
    SDL_FlipMode flip) {
    SDL_FRect fsrc{}, fdst{};
    SDL_FRect* pfsrc = nullptr;
    SDL_FRect* pfdst = nullptr;
    SDL_FPoint fcenter{};
    SDL_FPoint* pfcenter = nullptr;

    if (srcrect) {
        fsrc.x = static_cast<float>(srcrect->x);
        fsrc.y = static_cast<float>(srcrect->y);
        fsrc.w = static_cast<float>(srcrect->w);
        fsrc.h = static_cast<float>(srcrect->h);
        pfsrc = &fsrc;
    }
    if (dstrect) {
        fdst.x = static_cast<float>(dstrect->x);
        fdst.y = static_cast<float>(dstrect->y);
        fdst.w = static_cast<float>(dstrect->w);
        fdst.h = static_cast<float>(dstrect->h);
        pfdst = &fdst;
    }
    if (center) {
        fcenter.x = static_cast<float>(center->x);
        fcenter.y = static_cast<float>(center->y);
        pfcenter = &fcenter;
    }

    return SDL_RenderTextureRotated(renderer, texture, pfsrc, pfdst, angle, pfcenter, flip) ? 0 : -1;
}
#define SDL_RenderCopyEx dune_compat_SDL_RenderCopyEx

// SDL_RenderCopyExF compatibility
// SDL2: int SDL_RenderCopyExF(renderer, texture, srcrect, dstrect, angle, center, flip)
// SDL3: bool SDL_RenderTextureRotated(renderer, texture, srcrect, dstrect, angle, center, flip)
inline int dune_compat_SDL_RenderCopyExF(
    SDL_Renderer* renderer,
    SDL_Texture* texture,
    const SDL_Rect* srcrect,
    const SDL_FRect* dstrect,
    double angle,
    const SDL_FPoint* center,
    SDL_FlipMode flip) {
    SDL_FRect fsrc{};
    SDL_FRect* pfsrc = nullptr;

    if (srcrect) {
        fsrc.x = static_cast<float>(srcrect->x);
        fsrc.y = static_cast<float>(srcrect->y);
        fsrc.w = static_cast<float>(srcrect->w);
        fsrc.h = static_cast<float>(srcrect->h);
        pfsrc = &fsrc;
    }

    return SDL_RenderTextureRotated(renderer, texture, pfsrc, dstrect, angle, center, flip) ? 0 : -1;
}
#define SDL_RenderCopyExF dune_compat_SDL_RenderCopyExF

// SDL_WaitSemaphore compatibility
// SDL2: int SDL_WaitSemaphore(sem) (returns 0 on success)
// SDL3: void SDL_WaitSemaphore(sem)
inline int dune_compat_SDL_WaitSemaphore(SDL_Semaphore* sem) {
    SDL_WaitSemaphore(sem);
    return 0; // SDL3's version doesn't return status, assume success if it returns
}
#define SDL_WaitSemaphore dune_compat_SDL_WaitSemaphore

// SDL_RenderCopyF compatibility
// SDL2: int SDL_RenderCopyF(renderer, texture, srcrect, dstrect) - uses SDL_FRect
// SDL3: bool SDL_RenderTexture(renderer, texture, srcrect, dstrect) - uses SDL_FRect
inline int dune_compat_SDL_RenderCopyF(
    SDL_Renderer* renderer,
    SDL_Texture* texture,
    const SDL_Rect* srcrect,
    const SDL_FRect* dstrect) {
    SDL_FRect fsrc{};
    SDL_FRect* pfsrc = nullptr;
    
    if (srcrect) {
        fsrc.x = static_cast<float>(srcrect->x);
        fsrc.y = static_cast<float>(srcrect->y);
        fsrc.w = static_cast<float>(srcrect->w);
        fsrc.h = static_cast<float>(srcrect->h);
        pfsrc = &fsrc;
    }
    
    return SDL_RenderTexture(renderer, texture, pfsrc, dstrect) ? 0 : -1;
}
#define SDL_RenderCopyF dune_compat_SDL_RenderCopyF

// SDL_QueryTexture compatibility wrapper
// SDL2: int SDL_QueryTexture(texture, format, access, w, h)
// SDL3: SDL_QueryTexture is REMOVED - use SDL_GetTextureSize() and SDL_GetTextureProperties()
inline int dune_compat_SDL_QueryTexture(
    SDL_Texture* texture,
    Uint32* format,
    int* access,
    int* w,
    int* h) {
    if (w || h) {
        float fw = 0, fh = 0;
        if (!SDL_GetTextureSize(texture, &fw, &fh)) {
            return -1;
        }
        if (w) *w = static_cast<int>(fw);
        if (h) *h = static_cast<int>(fh);
    }
    if (format || access) {
        SDL_PropertiesID props = SDL_GetTextureProperties(texture);
        if (props == 0) {
            return -1;
        }
        if (format) {
            *format = static_cast<Uint32>(SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_FORMAT_NUMBER, SDL_PIXELFORMAT_UNKNOWN));
        }
        if (access) {
            *access = static_cast<int>(SDL_GetNumberProperty(props, SDL_PROP_TEXTURE_ACCESS_NUMBER, SDL_TEXTUREACCESS_STATIC));
        }
    }
    return 0;
}
#define SDL_QueryTexture dune_compat_SDL_QueryTexture

// SDL_RenderSetLogicalSize compatibility
// SDL2: int SDL_RenderSetLogicalSize(renderer, w, h)
// SDL3: bool SDL_SetRenderLogicalPresentation(renderer, w, h, mode)
inline int dune_compat_SDL_RenderSetLogicalSize(
    SDL_Renderer* renderer,
    int w,
    int h) {
    // Use letterbox mode for logical presentation (matches SDL2 behavior)
    return SDL_SetRenderLogicalPresentation(renderer, w, h, SDL_LOGICAL_PRESENTATION_LETTERBOX) ? 0 : -1;
}
#undef SDL_RenderSetLogicalSize
#define SDL_RenderSetLogicalSize dune_compat_SDL_RenderSetLogicalSize

// ============================================================================
// SURFACE FORMAT ACCESS COMPATIBILITY
// SDL3 changed surface->format from SDL_PixelFormat* to SDL_PixelFormat (value)
// and removed BitsPerPixel, palette from the format struct
// ============================================================================

// Helper to get pixel format from surface
inline SDL_PixelFormat dune_compat_GetSurfaceFormat(SDL_Surface* surface) {
    return surface->format;
}

// Helper to get bits per pixel from surface
inline int dune_compat_GetSurfaceBitsPerPixel(SDL_Surface* surface) {
    return SDL_BITSPERPIXEL(surface->format);
}

// Helper to get bytes per pixel from surface
inline int dune_compat_GetSurfaceBytesPerPixel(SDL_Surface* surface) {
    return SDL_BYTESPERPIXEL(surface->format);
}

// Helper to get palette from surface (SDL3: use SDL_GetSurfacePalette)
inline SDL_Palette* dune_compat_GetSurfacePalette(SDL_Surface* surface) {
    return SDL_GetSurfacePalette(surface);
}

#endif // __cplusplus

#endif // DUNE_SDL2TO3_H
