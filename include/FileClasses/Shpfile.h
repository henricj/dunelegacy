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

#ifndef SHPFILE_H
#define SHPFILE_H

#include "Animation.h"
#include <misc/SDL2pp.h>

#include <array>
#include <concepts>
#include <span>
#include <tuple>
#include <utility>
#include <vector>

inline constexpr auto TILE_NORMAL = 0x00010000;
inline constexpr auto TILE_FLIPH  = 0x00100000;
inline constexpr auto TILE_FLIPV  = 0x01000000;
inline constexpr auto TILE_ROTATE = 0x10000000;

constexpr auto TILE_GETINDEX(uint32_t x) {
    return x & 0x0000FFFFU;
}

constexpr auto TILE_GETTYPE(uint32_t x) {
    return x & 0xFFFF0000U;
}

/// A class for loading a *.SHP-File.
/**
    This class can read shp-Files and return the contained pictures as a SDL_Surface.
*/
class Shpfile final {
private:
    // Internal structure used for an index of contained files
    struct ShpfileEntry {
        uint32_t startOffset;
        uint32_t endOffset;
    };

public:
    explicit Shpfile(SDL_RWops* rwop);
    Shpfile(const Shpfile& o)          = delete;
    Shpfile(Shpfile&&)                 = delete;
    Shpfile& operator=(const Shpfile&) = delete;
    Shpfile& operator=(Shpfile&&)      = delete;
    virtual ~Shpfile();

    sdl2::surface_ptr getPicture(uint32_t indexOfFile);

    template<typename... Args>
    sdl2::surface_ptr getPictureArray(unsigned int tilesX, unsigned int tilesY, Args&&... args) {
        constexpr auto size = (std::tuple_size_v<std::decay_t<Args>> + ...);
        std::array<int, size> buffer;

        auto it = buffer.begin();

        auto next_out = [](auto&& v) { return v.out; };

        ((it = next_out(std::ranges::copy(args, it))), ...);

        assert(it == buffer.end());

        return getPictureArrayImpl(tilesX, tilesY, buffer);
    }

    sdl2::surface_ptr getPictureArray(unsigned int tilesX, unsigned int tilesY, std::integral auto... tiles) {
        constexpr auto size = sizeof...(tiles);
        std::array<int, size> buffer{tiles...};

        return getPictureArrayImpl(tilesX, tilesY, buffer);
    }

    std::unique_ptr<Animation> getAnimation(unsigned int startindex, unsigned int endindex, bool bDoublePic = true,
                                            bool bSetColorKey = true, bool bLoopRewindBackwards = false);

    /// Returns the number of contained pictures
    /**
        Returns the number of pictures in this SHP-File.
        \return Number of pictures in this SHP-File.
    */
    [[nodiscard]] int getNumFiles() const { return static_cast<int>(shpfileEntries.size()); }

private:
    sdl2::surface_ptr
    getPictureArrayImpl(unsigned int tilesX, unsigned int tilesY, std::span<const int> tile_index) const;
    void readIndex();
    static void shpCorrectLF(const unsigned char* in, unsigned char* out, int size);
    static void applyPalOffsets(const unsigned char* offsets, unsigned char* data, unsigned int length);

    std::vector<ShpfileEntry> shpfileEntries;
    std::unique_ptr<unsigned char[]> pFiledata;
    size_t shpFilesize;
};

#endif // SHPFILE_H
