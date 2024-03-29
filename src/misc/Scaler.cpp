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

#include <misc/Scaler.h>

#include "Definitions.h"
#include "misc/draw_util.h"

#include <algorithm>

DoubleSurfaceFunction* Scaler::defaultDoubleSurface           = doubleSurfaceScale2x;
DoubleTiledSurfaceFunction* Scaler::defaultDoubleTiledSurface = doubleTiledSurfaceScale2x;

DoubleSurfaceFunction* Scaler::defaultTripleSurface           = tripleSurfaceScale3x;
DoubleTiledSurfaceFunction* Scaler::defaultTripleTiledSurface = tripleTiledSurfaceScale3x;

void Scaler::setDefaultScaler(ScalerType scaler) {
    switch (scaler) {
        case ScaleHD: {
            defaultDoubleSurface      = doubleSurfaceScale2x;
            defaultDoubleTiledSurface = doubleTiledSurfaceScale2x;
            defaultTripleSurface      = tripleSurfaceScale3x;
            defaultTripleTiledSurface = tripleTiledSurfaceScale3x;
        } break;

        case Scale2x:
        default: {
            defaultDoubleSurface      = doubleSurfaceScale2x;
            defaultDoubleTiledSurface = doubleTiledSurfaceScale2x;
            defaultTripleSurface      = tripleSurfaceScale3x;
            defaultTripleTiledSurface = tripleTiledSurfaceScale3x;
        } break;

        case ScaleNN: {
            defaultDoubleSurface      = doubleSurfaceNN;
            defaultDoubleTiledSurface = doubleTiledSurfaceNN;
            defaultTripleSurface      = tripleSurfaceNN;
            defaultTripleTiledSurface = tripleTiledSurfaceNN;
        } break;
    }
}

namespace {
template<typename Scale>
sdl2::surface_ptr
scale_surface(SDL_Surface* src, bool freeSrcSurface, int width, int height, bool allow_rle, Scale&& scale) {
    if (src == nullptr)
        return nullptr;

    sdl2::surface_ptr src_handle{freeSrcSurface ? src : nullptr};

    // create new picture surface
    auto returnPic = sdl2::surface_ptr{SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0)};
    if (returnPic == nullptr) {
        return nullptr;
    }

    copySurfaceAttributes(returnPic.get(), src);

    sdl2::surface_lock return_lock{returnPic.get()};
    sdl2::surface_lock src_lock{src};

    // Now we can copy pixel by pixel
    scale(src, returnPic.get());

    return returnPic;
}
} // namespace

/**
    This function doubles a surface by making 4 same-colored pixels out of one.
    \param  src             the source image
    \return the scaled surface
*/
sdl2::surface_ptr Scaler::doubleSurfaceNN(SDL_Surface* src) {
    if (src == nullptr) {
        return nullptr;
    }

    // create new picture surface
    auto returnPic = sdl2::surface_ptr{SDL_CreateRGBSurface(0, src->w * 2, src->h * 2, 8, 0, 0, 0, 0)};
    if (returnPic == nullptr) {
        return nullptr;
    }

    copySurfaceAttributes(returnPic.get(), src);

    sdl2::surface_lock return_lock{returnPic.get()};
    sdl2::surface_lock src_lock{src};

    const auto* RESTRICT source = static_cast<char*>(src->pixels);
    auto* RESTRICT destination  = static_cast<char*>(returnPic->pixels);

    const auto source_pitch      = src->pitch;
    const auto destination_pitch = returnPic->pitch;

    // Now we can copy pixel by pixel
    for (int y = 0; y < src->h; y++) {
        for (int x = 0; x < src->w; x++) {
            const char val = source[y * source_pitch + x];

            destination[2 * y * destination_pitch + 2 * x]           = val;
            destination[2 * y * destination_pitch + 2 * x + 1]       = val;
            destination[(2 * y + 1) * destination_pitch + 2 * x]     = val;
            destination[(2 * y + 1) * destination_pitch + 2 * x + 1] = val;
        }
    }

    return returnPic;
}

/**
    This function is a wrapper around Scaler::doubleSurfaceNN.
    \param  src             the source image
    \param  tilesX          ignored
    \param  tilesY          ignored
    \return the scaled surface
*/
sdl2::surface_ptr
Scaler::doubleTiledSurfaceNN(SDL_Surface* src, [[maybe_unused]] int tilesX, [[maybe_unused]] int tilesY) {
    return doubleSurfaceNN(src);
}

/**
    This function triples a surface by making 9 same-colored pixels out of one.
    \param  src             the source image
    \return the scaled surface
*/
sdl2::surface_ptr Scaler::tripleSurfaceNN(SDL_Surface* src) {
    if (src == nullptr) {
        return nullptr;
    }

    // create new picture surface
    auto returnPic = sdl2::surface_ptr{SDL_CreateRGBSurface(0, src->w * 3, src->h * 3, 8, 0, 0, 0, 0)};
    if (returnPic == nullptr) {
        return nullptr;
    }

    copySurfaceAttributes(returnPic.get(), src);

    sdl2::surface_lock return_lock{returnPic.get()};
    sdl2::surface_lock src_lock{src};

    const auto* RESTRICT source = static_cast<char*>(src->pixels);
    auto* RESTRICT destination  = static_cast<char*>(returnPic->pixels);

    const auto destination_pitch = returnPic->pitch;
    const auto source_pitch      = src->pitch;

    // Now we can copy pixel by pixel
    for (int y = 0; y < src->h; y++) {
        for (int x = 0; x < src->w; x++) {
            const char val = source[y * source_pitch + x];

            destination[3 * y * destination_pitch + 3 * x]           = val;
            destination[3 * y * destination_pitch + 3 * x + 1]       = val;
            destination[3 * y * destination_pitch + 3 * x + 2]       = val;
            destination[(3 * y + 1) * destination_pitch + 3 * x]     = val;
            destination[(3 * y + 1) * destination_pitch + 3 * x + 1] = val;
            destination[(3 * y + 1) * destination_pitch + 3 * x + 2] = val;
            destination[(3 * y + 2) * destination_pitch + 3 * x]     = val;
            destination[(3 * y + 2) * destination_pitch + 3 * x + 1] = val;
            destination[(3 * y + 2) * destination_pitch + 3 * x + 2] = val;
        }
    }

    return returnPic;
}

/**
    This function is a wrapper around Scaler::tripleSurfaceNN.
    \param  src             the source image
    \param  tilesX          ignored
    \param  tilesY          ignored
    \return the scaled surface
*/
sdl2::surface_ptr
Scaler::tripleTiledSurfaceNN(SDL_Surface* src, [[maybe_unused]] int tilesX, [[maybe_unused]] int tilesY) {
    return tripleSurfaceNN(src);
}

/**
    This function doubles a surface while smoothing edges (see http://scale2x.sourceforge.net/algorithm.html ).
    \param  src             the source image
    \return the scaled surface
*/
sdl2::surface_ptr Scaler::doubleSurfaceScale2x(SDL_Surface* src) {
    return doubleTiledSurfaceScale2x(src, 1, 1);
}

/**
    This function doubles a surface while smoothing edges (see http://scale2x.sourceforge.net/algorithm.html ).
    \param  src             the source image
    \param  tilesX          number of subimages in x direction
    \param  tilesY          number of subimages in y direction
    \return the scaled surface
*/
sdl2::surface_ptr Scaler::doubleTiledSurfaceScale2x(SDL_Surface* src, int tilesX, int tilesY) {
    if (src == nullptr) {
        return nullptr;
    }

    const int srcWidth  = src->w;
    const int srcHeight = src->h;

    // create new picture surface
    auto returnPic = sdl2::surface_ptr{SDL_CreateRGBSurface(0, srcWidth * 2, srcHeight * 2, 8, 0, 0, 0, 0)};
    if (returnPic == nullptr) {
        return nullptr;
    }

    copySurfaceAttributes(returnPic.get(), src);

    const int tileWidth  = srcWidth / tilesX;
    const int tileHeight = srcHeight / tilesY;

    sdl2::surface_lock return_lock{returnPic.get()};
    sdl2::surface_lock src_lock{src};

    const auto* RESTRICT srcPixels = static_cast<uint8_t*>(src->pixels);
    auto* RESTRICT destPixels      = static_cast<uint8_t*>(returnPic->pixels);

    const auto source_pitch      = src->pitch;
    const auto destination_pitch = returnPic->pitch;

    for (int j = 0; j < tilesY; ++j) {
        for (int i = 0; i < tilesX; ++i) {

            /*

            Scale center pixel E into 4 new pixels

                Source            Dest
            +---+---+---+
            | A | B | C |       +--+--+
            +---+---+---+       |E0|E1|
            | D | E | F |   ->  +--+--+
            +---+---+---+       |E2|E3|
            | G | H | I |       +--+--+
            +---+---+---+

            */

            for (int y = 0; y < tileHeight; ++y) {
                for (int x = 0; x < tileWidth; ++x) {
                    uint8_t E = srcPixels[(j * tileHeight + y) * source_pitch + (i * tileWidth + x)];
                    const uint8_t B =
                        srcPixels[(j * tileHeight + std::max(0, y - 1)) * source_pitch + (i * tileWidth + x)];
                    const uint8_t H = srcPixels[(j * tileHeight + std::min(tileHeight - 1, y + 1)) * source_pitch
                                                + (i * tileWidth + x)];
                    uint8_t D = srcPixels[(j * tileHeight + y) * source_pitch + (i * tileWidth + std::max(0, x - 1))];
                    uint8_t F = srcPixels[(j * tileHeight + y) * source_pitch
                                          + (i * tileWidth + std::min(tileWidth - 1, x + 1))];

                    uint8_t E0 = 0;
                    uint8_t E1 = 0;
                    uint8_t E2 = 0;
                    uint8_t E3 = 0;

                    if (B != H && D != F) {
                        E0 = D == B ? D : E;
                        E1 = B == F ? F : E;
                        E2 = D == H ? D : E;
                        E3 = H == F ? F : E;
                    } else {
                        E0 = E;
                        E1 = E;
                        E2 = E;
                        E3 = E;
                    }

                    destPixels[(j * tileHeight + y) * 2 * destination_pitch + (i * tileWidth + x) * 2]           = E0;
                    destPixels[(j * tileHeight + y) * 2 * destination_pitch + (i * tileWidth + x) * 2 + 1]       = E1;
                    destPixels[((j * tileHeight + y) * 2 + 1) * destination_pitch + (i * tileWidth + x) * 2]     = E2;
                    destPixels[((j * tileHeight + y) * 2 + 1) * destination_pitch + (i * tileWidth + x) * 2 + 1] = E3;
                }
            }
        }
    }

    return returnPic;
}

/**
    This function triples a surface while smoothing edges (see http://scale2x.sourceforge.net/algorithm.html ).
    \param  src             the source image
    \return the scaled surface
*/
sdl2::surface_ptr Scaler::tripleSurfaceScale3x(SDL_Surface* src) {
    return tripleTiledSurfaceScale3x(src, 1, 1);
}

/**
    This function triples a surface while smoothing edges (see http://scale2x.sourceforge.net/algorithm.html ).
    \param  src             the source image
    \param  tilesX          number of subimages in x direction
    \param  tilesY          number of subimages in y direction
    \return the scaled surface
*/
sdl2::surface_ptr Scaler::tripleTiledSurfaceScale3x(SDL_Surface* src, int tilesX, int tilesY) {
    if (src == nullptr) {
        return nullptr;
    }

    const int srcWidth  = src->w;
    const int srcHeight = src->h;

    // create new picture surface
    auto returnPic = sdl2::surface_ptr{SDL_CreateRGBSurface(0, srcWidth * 3, srcHeight * 3, 8, 0, 0, 0, 0)};
    if (returnPic == nullptr) {
        return nullptr;
    }

    copySurfaceAttributes(returnPic.get(), src);

    const int tileWidth  = srcWidth / tilesX;
    const int tileHeight = srcHeight / tilesY;

    sdl2::surface_lock return_lock{returnPic.get()};
    sdl2::surface_lock src_lock{src};

    const auto* RESTRICT source = static_cast<uint8_t*>(src->pixels);
    auto* RESTRICT destination  = static_cast<uint8_t*>(returnPic->pixels);

    const auto source_pitch      = src->pitch;
    const auto destination_pitch = returnPic->pitch;

    for (int j = 0; j < tilesY; ++j) {
        for (int i = 0; i < tilesX; ++i) {

            /*

            Scale center pixel E into 4 new pixels

                Source             Dest
            +---+---+---+       +--+--+--+
            | A | B | C |       |E0|E1|E2|
            +---+---+---+       +--+--+--+
            | D | E | F |   ->  |E3|E4|E5|
            +---+---+---+       +--+--+--+
            | G | H | I |       |E6|E7|E8|
            +---+---+---+       +--+--+--+

            */

            for (int y = 0; y < tileHeight; ++y) {
                for (int x = 0; x < tileWidth; ++x) {
                    const uint8_t A = source[(j * tileHeight + std::max(0, y - 1)) * source_pitch
                                             + (i * tileWidth + std::max(0, x - 1))];
                    uint8_t B = source[(j * tileHeight + std::max(0, y - 1)) * source_pitch + (i * tileWidth + x)];
                    const uint8_t C = source[(j * tileHeight + std::max(0, y - 1)) * source_pitch
                                             + (i * tileWidth + std::min(tileWidth - 1, x + 1))];
                    uint8_t D = source[(j * tileHeight + y) * source_pitch + (i * tileWidth + std::max(0, x - 1))];
                    uint8_t E = source[(j * tileHeight + y) * source_pitch + (i * tileWidth + x)];
                    uint8_t F =
                        source[(j * tileHeight + y) * source_pitch + (i * tileWidth + std::min(tileWidth - 1, x + 1))];
                    const uint8_t G = source[(j * tileHeight + std::min(tileHeight - 1, y + 1)) * source_pitch
                                             + (i * tileWidth + std::max(0, x - 1))];
                    uint8_t H =
                        source[(j * tileHeight + std::min(tileHeight - 1, y + 1)) * source_pitch + (i * tileWidth + x)];
                    const uint8_t I = source[(j * tileHeight + std::min(tileHeight - 1, y + 1)) * source_pitch
                                             + (i * tileWidth + std::min(tileWidth - 1, x + 1))];

                    uint8_t E0 = 0;
                    uint8_t E1 = 0;
                    uint8_t E2 = 0;
                    uint8_t E3 = 0;
                    uint8_t E4 = 0;
                    uint8_t E5 = 0;
                    uint8_t E6 = 0;
                    uint8_t E7 = 0;
                    uint8_t E8 = 0;

                    if (B != H && D != F) {
                        E0 = D == B ? D : E;
                        E1 = (D == B && E != C) || (B == F && E != A) ? B : E;
                        E2 = B == F ? F : E;
                        E3 = (D == B && E != G) || (D == H && E != A) ? D : E;
                        E4 = E;
                        E5 = (B == F && E != I) || (H == F && E != C) ? F : E;
                        E6 = D == H ? D : E;
                        E7 = (D == H && E != I) || (H == F && E != G) ? H : E;
                        E8 = H == F ? F : E;
                    } else {
                        E0 = E;
                        E1 = E;
                        E2 = E;
                        E3 = E;
                        E4 = E;
                        E5 = E;
                        E6 = E;
                        E7 = E;
                        E8 = E;
                    }

                    destination[(j * tileHeight + y) * 3 * destination_pitch + (i * tileWidth + x) * 3]           = E0;
                    destination[(j * tileHeight + y) * 3 * destination_pitch + (i * tileWidth + x) * 3 + 1]       = E1;
                    destination[(j * tileHeight + y) * 3 * destination_pitch + (i * tileWidth + x) * 3 + 2]       = E2;
                    destination[((j * tileHeight + y) * 3 + 1) * destination_pitch + (i * tileWidth + x) * 3]     = E3;
                    destination[((j * tileHeight + y) * 3 + 1) * destination_pitch + (i * tileWidth + x) * 3 + 1] = E4;
                    destination[((j * tileHeight + y) * 3 + 1) * destination_pitch + (i * tileWidth + x) * 3 + 2] = E5;
                    destination[((j * tileHeight + y) * 3 + 2) * destination_pitch + (i * tileWidth + x) * 3]     = E6;
                    destination[((j * tileHeight + y) * 3 + 2) * destination_pitch + (i * tileWidth + x) * 3 + 1] = E7;
                    destination[((j * tileHeight + y) * 3 + 2) * destination_pitch + (i * tileWidth + x) * 3 + 2] = E8;
                }
            }
        }
    }

    return returnPic;
}
