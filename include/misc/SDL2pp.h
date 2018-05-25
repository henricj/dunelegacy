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

#ifndef SDL2PP_H
#define SDL2PP_H

#include <memory>
#include <assert.h>
#include <misc/exceptions.h>
#include <misc/unique_or_nonowning_ptr.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_endian.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_mixer.h>

namespace sdl2
{
    class texture_lock final
    {
        SDL_Texture * const texture_;
        void* pixels_;
        int pitch_;
    public:
        explicit texture_lock(SDL_Texture* texture) : texture_(texture)
        {
            assert(texture);

            if (!texture_)
                return;

            if (0 == SDL_LockTexture(texture_, nullptr, &pixels_, &pitch_))
                return;

            THROW(std::runtime_error, "Unable to lock SDL texture!");
        }
        explicit texture_lock(SDL_Texture* texture, SDL_Rect& rect) : texture_(texture)
        {
            assert(texture);

            if (!texture_)
                return;

            if (0 == SDL_LockTexture(texture_, &rect, &pixels_, &pitch_))
                return;

            THROW(std::runtime_error, "Unable to lock SDL texture!");
        }
        ~texture_lock()
        {
            if (texture_)
                SDL_UnlockTexture(texture_);
        }

        void * pixels() const { return pixels_; }

        texture_lock(const texture_lock &) = delete;
        texture_lock(texture_lock &&) = delete;
        texture_lock& operator=(const texture_lock &) = delete;
        texture_lock& operator=(texture_lock &&) = delete;
    };

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

        template<typename T, void(*Delete)(T*)>
        using unique_or_nonowning_ptr_deleter = unique_or_nonowning_ptr<T, deleter<T, Delete>>;

        template<typename T, typename TArg, void(*Delete)(TArg*)>
        struct arg_deleter
        {
            void operator()(TArg* object) const { Delete(object); }
        };

        template<typename T, typename TArg, void(*Delete)(TArg*)>
        using unique_ptr_arg_deleter = std::unique_ptr<T, arg_deleter<T, TArg, Delete>>;

        template<typename T, typename TArg, void(*Delete)(TArg*)>
        using unique_or_nonowning_ptr_arg_deleter = unique_or_nonowning_ptr<T, arg_deleter<T, TArg, Delete>>;

        struct RWops_deleter
        {
            void operator()(SDL_RWops *RWops) const { if (RWops) { SDL_RWclose(RWops); } }
        };
    }

    template<typename T>
    using sdl_ptr = implementation::unique_ptr_arg_deleter<T, void, SDL_free>;

    typedef implementation::unique_ptr_deleter<SDL_Surface, SDL_FreeSurface> surface_ptr;
    typedef implementation::unique_or_nonowning_ptr_deleter<SDL_Surface, SDL_FreeSurface> surface_unique_or_nonowning_ptr;
    typedef implementation::unique_ptr_deleter<SDL_Texture, SDL_DestroyTexture> texture_ptr;
    typedef implementation::unique_or_nonowning_ptr_deleter<SDL_Texture, SDL_DestroyTexture> texture_unique_or_nonowning_ptr;
    typedef implementation::unique_ptr_deleter<SDL_Palette, SDL_FreePalette> palette_ptr;
    typedef implementation::unique_ptr_deleter<SDL_PixelFormat, SDL_FreeFormat> pixel_format_ptr;
    typedef implementation::unique_ptr_deleter<SDL_Renderer, SDL_DestroyRenderer> renderer_ptr;
    typedef std::unique_ptr<SDL_RWops, implementation::RWops_deleter> RWops_ptr;

    typedef implementation::unique_ptr_deleter<Mix_Chunk, Mix_FreeChunk> mix_chunk_ptr;
}

#endif // SDL2PP_H

