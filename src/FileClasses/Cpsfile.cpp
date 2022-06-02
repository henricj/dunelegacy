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

#include <FileClasses/Cpsfile.h>

#include <FileClasses/Decode.h>
#include <FileClasses/Palette.h>

#include <misc/exceptions.h>

#include "globals.h"
#include <Definitions.h>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {
inline constexpr auto SIZE_X = 320;
inline constexpr auto SIZE_Y = 200;
} // namespace

sdl2::surface_ptr LoadCPS_RW(SDL_RWops* RWop) {
    if (RWop == nullptr) {
        return nullptr;
    }

    const int64_t endOffset = SDL_RWsize(RWop);
    if (endOffset <= 0) {
        THROW(std::runtime_error, "LoadCPS_RW(): Cannot determine size of this *.cps-File!");
    }

    const auto cpsFilesize = static_cast<size_t>(endOffset);
    const auto pFiledata   = std::make_unique<uint8_t[]>(cpsFilesize);

    if (SDL_RWread(RWop, pFiledata.get(), cpsFilesize, 1) != 1) {
        THROW(std::runtime_error, "LoadCPS_RW(): Reading this *.cps-File failed!");
    }

    const uint16_t format = SDL_SwapLE16(*reinterpret_cast<uint16_t*>(pFiledata.get() + 2));

    if (format != 0x0004) {
        THROW(std::runtime_error, "LoadCPS_RW(): Only Format80 encoded *.cps-Files are supported!");
    }

    unsigned int SizeXTimeSizeY = SDL_SwapLE16(*reinterpret_cast<uint16_t*>(pFiledata.get() + 4));
    SizeXTimeSizeY += SDL_SwapLE16(*reinterpret_cast<uint16_t*>(pFiledata.get() + 6));

    if (SizeXTimeSizeY != SIZE_X * SIZE_Y) {
        THROW(std::runtime_error, "LoadCPS_RW(): Images must be 320x200 pixels big!");
    }

    const uint16_t PaletteSize = SDL_SwapLE16(*reinterpret_cast<uint16_t*>(pFiledata.get() + 8));

    const auto pImageOut = std::make_unique<uint8_t[]>(static_cast<size_t>(SIZE_X) * SIZE_Y);
    memset(pImageOut.get(), 0, static_cast<size_t>(SIZE_X) * SIZE_Y);

    if (decode80(pFiledata.get() + 10 + PaletteSize, pImageOut.get(), 0) == -2) {
        THROW(std::runtime_error, "LoadCPS_RW(): Decoding this *.cps-File failed!");
    }

    // create new picture surface
    auto pic = sdl2::surface_ptr{SDL_CreateRGBSurface(0, SIZE_X, SIZE_Y, 8, 0, 0, 0, 0)};
    if (pic == nullptr) {
        THROW(std::runtime_error, "LoadCPS_RW(): SDL_CreateRGBSurface has failed!");
    }

    dune::globals::palette.applyToSurface(pic.get());
    sdl2::surface_lock lock{pic.get()};

    // Now we can copy line by line
    auto* const RESTRICT p = static_cast<char*>(pic->pixels);
    for (auto y = ptrdiff_t{}; y < static_cast<ptrdiff_t>(SIZE_Y); ++y) {
        memcpy(p + y * pic->pitch, pImageOut.get() + y * SIZE_X, SIZE_X);
    }

    return pic;
}
