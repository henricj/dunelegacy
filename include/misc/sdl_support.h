#pragma once

#include <memory>
#include <SDL2/SDL.h>

namespace sdl2
{
    class surface_lock final
    {
        SDL_Surface * const surface_;
        void* const pixels_;
    public:
        explicit surface_lock(SDL_Surface* surface) : surface_(SDL_MUSTLOCK(surface) ? surface : nullptr), pixels_(surface->pixels)
        {
            assert(surface);

            if (!surface_)
                return;

            if (0 == SDL_LockSurface(surface_))
                return;

            THROW(std::runtime_error, "Unable to lock SDL surface!");
        }
        ~surface_lock()
        {
            if (surface_)
                SDL_UnlockSurface(surface_);
        }

        void * pixels() const { return pixels_; }

        surface_lock(const surface_lock &) = delete;
        surface_lock(surface_lock &&) = delete;
        surface_lock& operator=(const surface_lock &) = delete;
        surface_lock& operator=(surface_lock &&) = delete;
    };

    class surface_try_lock final
    {
        SDL_Surface * surface_;
        void * pixels_;
    public:
        explicit surface_try_lock(SDL_Surface* surface) : surface_(SDL_MUSTLOCK(surface) ? surface : nullptr), pixels_(surface->pixels)
        {
            assert(surface);

            if (!surface_)
                return;

            if (0 == SDL_LockSurface(surface_))
                return;

            surface_ = nullptr;
            pixels_ = nullptr;
        }
        ~surface_try_lock()
        {
            if (surface_)
                SDL_UnlockSurface(surface_);
        }

        void * pixels() const { return pixels_; }

        surface_try_lock(const surface_try_lock &) = delete;
        surface_try_lock(surface_try_lock &&) = delete;
        surface_try_lock& operator=(const surface_try_lock &) = delete;
        surface_try_lock& operator=(surface_try_lock &&) = delete;
    };

    namespace implementation
    {
        template<typename T, void(*Delete)(T*)>
        struct deleter
        {
            void operator()(T* object) const { Delete(object); }
        };

        template<typename T, void(*Delete)(T*)>
        using unique_ptr_deleter = std::unique_ptr<T, deleter<T, Delete>>;

        template<typename T, typename TArg, void(*Delete)(TArg*)>
        struct arg_deleter
        {
            void operator()(TArg* object) const { Delete(object); }
        };

        template<typename T, typename TArg, void(*Delete)(TArg*)>
        using unique_ptr_arg_deleter = std::unique_ptr<T, arg_deleter<T, TArg, Delete>>;

        struct RWop_deleter
        {
            void operator()(SDL_RWops *RWop) const { if (RWop) { SDL_RWclose(RWop); } }
        };
    }

    template<typename T>
    using sdl_ptr = implementation::unique_ptr_arg_deleter<T, void, SDL_free>;

    typedef implementation::unique_ptr_deleter<SDL_Surface, SDL_FreeSurface> surface_ptr;
    typedef implementation::unique_ptr_deleter<SDL_Texture, SDL_DestroyTexture> texture_ptr;
    typedef implementation::unique_ptr_deleter<SDL_Palette, SDL_FreePalette> palette_ptr;
    typedef implementation::unique_ptr_deleter<SDL_PixelFormat, SDL_FreeFormat> pixel_format_ptr;
    typedef implementation::unique_ptr_deleter<SDL_Renderer, SDL_DestroyRenderer> renderer_ptr;
    typedef std::unique_ptr<SDL_RWops, implementation::RWop_deleter> RWop_ptr;
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


